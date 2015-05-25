/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/

#include "vtable.h"
#include "RTTIBaseClassDescriptor.h"
#include "RTTIClassHierarchyDescriptor.h"
#include "CompleteObjectLocator.h"

int vtableCount = 0;

/*
* @brief : Get a vtable size
* @param curAddress : The vtable address. Can point to any address.
* @return 0 if no vtable is at \address, or the number of methods of the vtable if detected
*/
size_t 
get_vtable_methods_count (
    ea_t curAddress
) {
    ea_t startTable = BADADDR;
    ea_t curEntry = 0;

    // Iterate until we find a result
    for (; ; curAddress += 4) 
    {
        flags_t flags = IDAUtils::GetFlags (curAddress);

        // First iteration
        if (startTable == BADADDR) {
            startTable = curAddress;
            if (!(hasRef (flags) && (has_name (flags) || (flags & FF_LABL)))) {
                // Start of vtable should have a xref and a name (auto or manual)
                return 0;
            }
        }
        else if (hasRef (flags)) {
            // Might mean start of next vtable
            break;
        }
        
        if (!hasValue (flags) || !isData (flags)) {
            break;
        }
        
        if ((curEntry = get_long (curAddress))) {
            flags = IDAUtils::GetFlags (curEntry);
            
            if (!hasValue (flags) || !isCode (flags) || get_long (curEntry) == 0) {
                break;
            }
        }
    }

    
    if (startTable != BADADDR) {
        return (curAddress - startTable) / 4;
    }
    
    else {
        // No vtable at this EA
        return 0;
    }
}


/*
 * @brief : Check if get_long (vtable-4) points to typeinfo record and extract the type name from it
 * @return The type name, or NULL if an error occured
 */
char *
get_type_name (
    ea_t vtable,
    char *buffer,
    size_t bufferSize
) {
    if (vtable == BADADDR) {
        return NULL;
    }

    ea_t x = get_long (vtable - 4);

    if (!x || (x == BADADDR)) {
        return NULL;
    }

    x = get_long (x + 12);

    if (!x || (x == BADADDR)) {
        return NULL;
    }

    x = x + 8;

    char curByte;
    size_t bufferPos = 0;

    while (curByte = get_byte (x)) {
        buffer [bufferPos++] = curByte;
        x++;
        if (bufferPos > bufferSize) {
            return NULL;
        }
    }

    return buffer;
}



// Get class name for this vtable instance
char * 
get_vtable_class_name (
    ea_t address,
    char *buffer,
    size_t bufferSize
) {
    ea_t offset = get_long (address + 4);
    address = get_long (address + 16); // Class Hierarchy Descriptor

    ea_t a = get_long (address + 12); // pBaseClassArray
    size_t numBaseClasses = get_long (address + 8);  //numBaseClasses
    size_t i = 0;
    buffer[0] = '\0';
    
    while (i < numBaseClasses) 
    {
        ea_t p = get_long (a);
        // BaseClass[%02d]:  %08.8Xh\n", i, p
        if (get_long (p + 8) == offset) {
            // Found it
            return IDAUtils::getAsciizStr(get_long (p) + 8, buffer, bufferSize);
        }

        i++;
        a += 4;
    }

    // Didn't find matching one, let's get the first vbase
    i = 0;
    a = get_long (address + 12);

    while (i < numBaseClasses) 
    {
        ea_t p = get_long (a);
        // BaseClass[%02d]:  %08.8Xh\n", i, p
        if (get_long (p + 12) != -1)  {
            return IDAUtils::getAsciizStr(get_long (p) + 8, buffer, bufferSize);
        }

        i++;
        a += 4;
    }

    return buffer;
}

/*
* @brief : Check if there's a vtable at address, and dump into to output
* @param address : The vtable address. Can point to any address.
* @param output : An opened FILE
* @return position after the end of vtable
*/
void 
parse_vtable (
    ea_t address
) {
    char buffer2[2048] = {0};
    char *typeName = get_type_name (address, buffer2, sizeof (buffer2));
    char buffer[2048] = {0};

    if (strncmp (typeName, ".?A", 3) == 0)
    {
        IDAUtils::Unknown (address - 4, 4);
        IDAUtils::SoftOff (address - 4);
        ea_t i = get_long (address - 4);  // COL
        ea_t s2 = get_long (i + 4); // offset
        i = get_long (i + 16); // CHD
        i = get_long (i + 4);  // Attributes

        if ((i & 3) == 0 && s2 == 0) {
            // Single inheritance, so we don't need to worry about duplicate names (several vtables)
            sprintf_s (buffer, sizeof (buffer), "??_7%s6B@", &typeName[4]);

            // Set the VFTable name
            IDAUtils::MakeName (address, buffer);
            vtableCount++;
            
            sprintf_s (buffer, sizeof (buffer), "??_R4%s6B@", &typeName[4]);
            // Set the RTTI Complete Object Locator name
            IDAUtils::MakeName (get_long (address - 4), buffer);
        }

        else { 
            // Multiple inheritance
            char vtableClassName[2048] = {0};
            get_vtable_class_name (get_long (address - 4), vtableClassName, sizeof (vtableClassName));
            char vtableName[2048] = {0};
            char COLName[2048] = {0};
            sprintf_s (buffer, sizeof (buffer), "%s6B%s@", &typeName[4], &vtableClassName[4]);
            sprintf_s (vtableName, sizeof (vtableName),  "??_7%s", buffer);
            sprintf_s (COLName, sizeof (COLName), "??_R4%s", buffer);
            
            // Set the VFTable name
            IDAUtils::MakeName (address, vtableName);
            vtableCount++;

            // Set the RTTI Complete Object Locator name
            IDAUtils::MakeName (get_long (address - 4), COLName);
        }
    }
}


char *
get_vtable_class_name_2 (
    ea_t colAddress,
    char *buffer,
    size_t bufferSize
) {
    char vtableType [2048] = {0};

    CompleteObjectLocator::get_type_name_by_col (colAddress, vtableType, sizeof (vtableType));
    ea_t i = get_long (colAddress + 16); // CHD
    i = get_long (i+4);  // Attributes

    if ((i & 3) == 0 && get_long (colAddress + 4) == 0) { 
        //Single inheritance, so we don't need to worry about duplicate names (several vtables)
        sprintf_s (buffer, bufferSize, "??_7%s6B@", &vtableType[4]);
        return buffer;
    }

    else {
        // Multiple inheritance 
        char buffer3[2048] = {0};
        char *s2 = get_vtable_class_name (colAddress, buffer3, sizeof (buffer3));
        sprintf_s (buffer, bufferSize, "??_7%s6B%s@", &vtableType[4], &s2[4]);
        return buffer;
    }

    return NULL;
}


ea_t 
get_func_start (
    ea_t address
) {
    if (IDAUtils::GetFunctionFlags (address) == -1) {
        return -1;
    }

    if ((IDAUtils::GetFlags (address) & FF_FUNC) != 0) {
        return address;
    }

    else {
        return IDAUtils::PrevFunction(address);
    }
}


//check for `scalar deleting destructor'
ea_t
checkSDD (
    ea_t address,
    char *name,
    ea_t vtable,
    ea_t gate
) {
    ea_t a = BADADDR, t = 0;
    char buffer[2048];
    char *varName = IDAUtils::Name (address, buffer, sizeof (buffer));

    if ((name != NULL) 
    && (strncmp (varName, "??_", 3) == 0)
    && (str_pos (varName, &name[4]) == 4)
    ) {
        // It's already named
        name = NULL; 
    }

    if ((IDAUtils::Byte (address) == 0xE9)
    ||  (IDAUtils::Byte (address) == 0xEB)
    ) {
        // E9 xx xx xx xx   jmp   xxxxxxx
        return checkSDD (IDAUtils::getRelJmpTarget (address), name, vtable, 1);
    }

    else if (IDAUtils::matchBytes(address,"83E9??E9")) {
        //thunk
        //83 E9 xx        sub     ecx, xx
        //E9 xx xx xx xx  jmp     class::`scalar deleting destructor'(uint)
        a = IDAUtils::getRelJmpTarget (address + 3);
        t = checkSDD (a, name, vtable, 0);
        if (t && name != NULL) {
            char buffer[2048] = {0};
            //rename this function as a thunk
            IDAUtils::MakeName (address, IDAUtils::MakeSpecialName (name, t, IDAUtils::Byte (address + 2), buffer, sizeof (buffer)));
        }

        return t;
    }

    else if (IDAUtils::matchBytes (address,"81E9????????E9")) {
        // thunk
        // 81 E9 xx xx xx xx        sub     ecx, xxxxxxxx
        // E9 xx xx xx xx           jmp     class::`scalar deleting destructor'(uint)
        a = IDAUtils::getRelJmpTarget (address+6);
        t = checkSDD (a, name, vtable, 0);

        if (t && name != NULL) {
            char buffer[2048] = {0};
            //rename this function as a thunk
            char *specialName = IDAUtils::MakeSpecialName (name,t, IDAUtils::Dword (address+2), buffer, sizeof (buffer));
            IDAUtils::MakeName (address, specialName);
        }

        return t;
    }

    else if (IDAUtils::matchBytes (address, "568BF1E8????????F64424080174") 
    &&       IDAUtils::matchBytes (address + 15 + IDAUtils::Byte (address + 14), "8BC65EC20400")
    ) {
        //56                             push    esi
        //8B F1                          mov     esi, ecx
        //E8 xx xx xx xx                 call    class::~class()
        //F6 44 24 08 01                 test    [esp+arg_0], 1
        //74 07                          jz      short @@no_free
        //56                             push    esi
        //                               
        //                           call operator delete();
    
        //   @@no_free:
        //8B C6                          mov     eax, esi
        //5E                             pop     esi
        //C2 04 00                       retn    4

        t = SN_scalardtr;
        a = IDAUtils::getRelCallTarget (address+3);
        if (gate && IDAUtils::Byte(a) == 0xE9)
        {
            //E9 xx xx xx xx   jmp   xxxxxxx
            a = IDAUtils::getRelJmpTarget(a);
        }
    }
    else if (IDAUtils::matchBytes (address,"568BF1FF15????????F64424080174") 
    &&       IDAUtils::matchBytes (address + 16 + IDAUtils::Byte (address + 15), "8BC65EC20400")
    ) {
        //56                             push    esi
        //8B F1                          mov     esi, ecx
        //FF 15 xx xx xx xx              call    class::~class() //dllimport
        //F6 44 24 08 01                 test    [esp+arg_0], 1
        //74 07                          jz      short @@no_free
        //56                             push    esi
        //                               
        //                           call operator delete();
    
        //   @@no_free:
        //8B C6                          mov     eax, esi
        //5E                             pop     esi
        //C2 04 00                       retn    4

        t = SN_scalardtr;
        /*a = getRelCallTarget (address+3);
        if (gate && Byte(a)==0xE9)
        {
            //E9 xx xx xx xx   jmp   xxxxxxx
            a = getRelJmpTarget(a);
        }*/
    }
    else if (IDAUtils::matchBytes (address,"558BEC51894DFC8B4DFCE8????????8B450883E00185C0740C8B4DFC51E8????????83C4048B45FC8BE55DC20400") 
    ||       IDAUtils::matchBytes (address,"558BEC51894DFC8B4DFCE8????????8B450883E00185C074098B4DFC51E8????????8B45FC8BE55DC20400"))
    {
        //55                             push    ebp
        //8B EC                          mov     ebp, esp
        //51                             push    ecx
        //89 4D FC                       mov     [ebp+var_4], ecx
        //8B 4D FC                       mov     ecx, [ebp+var_4]
        //E8 xx xx xx xx                 call    sub_10001099
        //8B 45 08                       mov     eax, [ebp+arg_0]
        //83 E0 01                       and     eax, 1
        //85 C0                          test    eax, eax
        //74 0C                          jz      short skip
        //8B 4D FC                       mov     ecx, [ebp+var_4]
        //51                             push    ecx
        //E8 F0 56 05 00                 call    operator delete(void *)
        //83 C4 04                       add     esp, 4
        //
        //               skip:
        //8B 45 FC                       mov     eax, [ebp+var_4]
        //8B E5                          mov     esp, ebp
        //5D                             pop     ebp
        //C2 04 00                       retn    4

        t = SN_scalardtr;
        a = IDAUtils::getRelCallTarget (address+10);
        if (gate && IDAUtils::Byte(a)==0xE9)
        {
            //E9 xx xx xx xx   jmp   xxxxxxx
            a = IDAUtils::getRelJmpTarget(a);
        }
    }
    else if (IDAUtils::matchBytes (address,"568D71??578D7E??8BCFE8????????F644240C01"))
    {
        //56                             push    esi
        //8D 71 xx                       lea     esi, [ecx-XX]
        //57                             push    edi
        //8D 7E xx                       lea     edi, [esi+XX]
        //8B CF                          mov     ecx, edi
        //E8 xx xx xx xx                 call    class::~class()
        //F6 44 24 0C 01                 test    [esp+4+arg_0], 1
        a = IDAUtils::getRelCallTarget (address+10);
        if (gate && IDAUtils::Byte(a)==0xE9)  {
            a = IDAUtils::getRelJmpTarget(a);
        }

        t = SN_scalardtr;
    }

    else if (IDAUtils::matchBytes (address,"568DB1????????578DBE????????8BCFE8????????F644240C01")) {
        //56                             push    esi
        //8D B1 xx xx xx xx              lea     esi, [ecx-XX]
        //57                             push    edi
        //8D BE xx xx xx xx              lea     edi, [esi+XX]
        //8B CF                          mov     ecx, edi
        //E8 xx xx xx xx                 call    class::~class()
        //F6 44 24 0C 01                 test    [esp+4+arg_0], 1
        a = IDAUtils::getRelCallTarget (address+16);
        if (gate && IDAUtils::Byte(a)==0xE9) {
            a = IDAUtils::getRelJmpTarget(a);
        }

        t = SN_scalardtr;
    }
    else if ((IDAUtils::matchBytes (address, "F644240401568BF1C706") /*&& Dword (address+10)==vtable*/) 
    ||       (IDAUtils::matchBytes (address, "8A442404568BF1A801C706") /*&& Dword (address+11)==vtable */) 
    ||       (IDAUtils::matchBytes (address, "568BF1C706????????E8????????F64424080174") 
              && IDAUtils::matchBytes (address + 21 + IDAUtils::Byte (address + 20), "8BC65EC20400"))
    ) {
        //F6 44 24 04 01                 test    [esp+arg_0], 1
        //56                             push    esi
        //8B F1                          mov     esi, ecx
        //  OR
        //8A 44 24 04                    mov     al, [esp+arg_0]
        //56                             push    esi
        //8B F1                          mov     esi, ecx
        //A8 01                          test    al, 1

        //C7 06 xx xx xx xx              mov     dword ptr [esi], xxxxxxx //offset vtable
        //                           <inlined destructor>
        //74 07                          jz      short @@no_free
        //56                             push    esi
        //E8 CA 2D 0D 00                 call    operator delete(void *)
        //59                             pop     ecx
        //   @@no_free:
        //8B C6                          mov     eax, esi
        //5E                             pop     esi
        //C2 04 00                       retn    4  
        t = SN_scalardtr;
    }
    else if (IDAUtils::matchBytes (address,"538A5C2408568BF1F6C302742B8B46FC578D7EFC68????????506A??56E8") || 
            IDAUtils::matchBytes (address,"538A5C2408F6C302568BF1742E8B46FC5768????????8D7EFC5068????????56E8"))
    {
        //53                            push    ebx
        //8A 5C 24 08                   mov     bl, [esp+arg_0]
        //56                            push    esi
        //8B F1                         mov     esi, ecx
        //F6 C3 02                      test    bl, 2
        //74 2B                         jz      short loc_100037F8
        //8B 46 FC                      mov     eax, [esi-4]
        //57                            push    edi
        //8D 7E FC                      lea     edi, [esi-4]
        //68 xx xx xx xx                push    offset class::~class(void)
        //50                            push    eax
        //6A xx                         push    xxh
        //56                            push    esi
        //E8 xx xx xx xx                call    `eh vector destructor iterator'(void *,uint,int,void (*)(void *))
        t = SN_vectordtr;
        if (name != 0) {
            a = IDAUtils::Dword (address+21);
        }
        if (gate && IDAUtils::Byte(a)==0xE9) {
            a = IDAUtils::getRelJmpTarget(a);
        }
    }

    if (t > 0) {
        if (name != NULL) {
            char buffer[2048] = {0};
            IDAUtils::MakeName (address, IDAUtils::MakeSpecialName (name, t, 0, buffer, sizeof (buffer)));
        }

        if (a != BADADDR) {
            if (name != NULL) {
                char buffer[2048] = {0};
                IDAUtils::MakeName(a, IDAUtils::MakeSpecialName(name, SN_vdestructor, 0, buffer, sizeof (buffer)));
            }
        }

        IDAUtils::CommentStack (address, 4, "__flags$", -1);
    }

    return t;
}


/*
* @brief : Check if there's a vtable at address, and dump into to output
* @param address : The vtable address. Can point to any address.
* @param output : An opened FILE
* @return position after the end of vtable
*/
ea_t 
do_vtable (
    ea_t address
) {
    size_t vtableMethodsCount = get_vtable_methods_count (address);
    ea_t endTable;
    ea_t p;
    char buffer[2048] = {0};

    if (!vtableMethodsCount) {
        // No vtable found at this address, return the next address
        return address + 4;
    }
    
    //check if it's named as a vtable
    char bufName[2048];
    char *name = IDAUtils::Name (address, bufName, sizeof (bufName));
    if (strncmp (name, "??_7", 4) != 0) {
        name = NULL;
    }
    
    endTable = get_long (address - 4);

    if (CompleteObjectLocator::isValid (endTable)) {
        parse_vtable (address);
        if (name == NULL) {
            name = get_vtable_class_name_2 (endTable, buffer, sizeof (buffer));
        }
        // only output object tree for main vtable
        if (get_long (endTable + 4) == 0) {
            CRTTIClassHierarchyDescriptor::parse2 (get_long (endTable + 16));
        }

        IDAUtils::MakeName(address, name);
    }

    if (name != NULL) {
        int typeinfoPos = str_pos (name, "@@6B");
        name[typeinfoPos+2] = '\0';
        char buffer2[2048];
        //convert vtable name into typeinfo name
        sprintf_s (buffer2, sizeof (buffer2), ".?AV%s", &name[4]);
        name = buffer2;
    }
    
    IDAUtils::DeleteArray (IDAUtils::GetArrayId ("AddrList"));
    int q = 0; int i = 1;

    for (endTable = IDAUtils::DfirstB (address); endTable != BADADDR; endTable = IDAUtils::DnextB (address, endTable))
    {
        p = get_func_start (endTable);

        if (p != BADADDR)
        {
            if (q == p) {
                i++;
            }

            else {           
                if (q) {
                    IDAUtils::AddAddr(q);
                }
                i = 1;
                q = p;
            }
        }
    }
    if (q) {
        IDAUtils::AddAddr(q);
    }
    
    endTable = address;

    while (vtableMethodsCount > 0)
    {
        p = get_long (endTable);
        if (IDAUtils::GetFunctionFlags(p) == -1) {
            IDAUtils::MakeCode(p);
            IDAUtils::MakeFunction (p, BADADDR);
        }
        
        checkSDD (p, name, address, 0);
        vtableMethodsCount--;
        endTable += 4;
    }

    IDAUtils::doAddrList (name);
    
    return endTable;
}

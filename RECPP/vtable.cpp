/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/

// ---------- Includes ------------
#include "Vtable.h"
#include "IDAUtils.h"
#include "VirtualMethod.h"
#include "CompleteObjectLocator.h"

Vtable::Vtable (
    ea_t address, 
    char *className, 
    size_t virtualMethodsCount
)  {
    char *forClass;
    filterClassName (&className, &forClass);

    this->address = address;
    this->className = strdup (className);
    this->virtualMethodsCount = virtualMethodsCount;
    
    // msg ("=== Analyzing Vtable for '%s' ===\n", className);

    for (size_t methodIndex = 0; methodIndex < virtualMethodsCount; methodIndex++) {
        VirtualMethod *m = new VirtualMethod (className, address, methodIndex, forClass);
        m->explore ();
        this->virtualMethods.push_back (m);
    }
}

Vtable::~Vtable () 
{
}

void
Vtable::filterClassName (
    char **_className,
    char **forClass
) {
    char *className = *_className;

    if (forClass != NULL) {
        *forClass = NULL;
    }

    // Remove const_ prefix
    if ((strncmp (className, "const ", strlen ("const ")) == 0)
    ||  (strncmp (className, "const_", strlen ("const_")) == 0)
    ) {
        className += strlen ("const ");
    }

    char *forPos = strstr (className, "{for `");
    char *vftablePos = strstr (className, "`vftable'");
    if (forPos) {
        forPos += strlen ("{for `");
        char *endSubClass = strstr (forPos, "'");
        if (endSubClass) {
            *endSubClass = '\0';
            if (forClass != NULL) {
                *forClass = forPos;
            }
        }
    }
    if (vftablePos) {
        *vftablePos = '\0';
    }

    // Remove ending ::
    size_t classNameLen = strlen (className);
    if (classNameLen >= 2) {
        char *endingColons = &className[classNameLen - 2];
        if (strncmp (endingColons, "::", 2) == 0) {
            *endingColons = '\0';
        }
    }

    // Remove forbidden characters
    char *forbidden;
    while (forbidden = strstr (className, "<")) {
        *forbidden = '_';
    }
    while (forbidden = strstr (className, ">")) {
        *forbidden = '_';
    }
    while (forbidden = strstr (className, " ")) {
        *forbidden = '_';
    }
    while (forbidden = strstr (className, ",")) {
        *forbidden = '_';
    }
    while (forbidden = strstr (className, "*")) {
        *forbidden = '_';
    }
    while (forbidden = strstr (className, "`")) {
        *forbidden = '_';
    }
    while (forbidden = strstr (className, "'")) {
        *forbidden = '_';
    }
    
    *_className = className;
}



/*
 * @brief : Check if get_long (vtable-4) points to typeinfo record and extract the type name from it
 * @return The type name, or NULL if an error occured
 */
char *
Vtable::getTypeName (
    ea_t vtable,
    char *result,
    size_t resultSize
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
        result [bufferPos++] = curByte;
        x++;
        if (bufferPos > resultSize) {
            return NULL;
        }
    }

    return result;
}

// Get class name for this vtable instance based on the COL
char * 
Vtable::getClassName2 (
    ea_t colAddress,
    char *result,
    size_t resultSize
) {
    char vtableType [4096] = {0};

    CompleteObjectLocator::get_type_name_by_col (colAddress, vtableType, sizeof (vtableType));
    ea_t i = get_long (colAddress + 16); // CHD
    i = get_long (i+4);  // Attributes

    if ((i & 3) == 0 && get_long (colAddress + 4) == 0) { 
        //Single inheritance, so we don't need to worry about duplicate names (several vtables)
        sprintf_s (result, resultSize, "??_7%s6B@", &vtableType[4]);
        return result;
    }

    else {
        // Multiple inheritance 
        char classNameBuffer [4096] = {0};
        char *s2 = getClassName (colAddress, classNameBuffer, sizeof (classNameBuffer));
        sprintf_s (result, resultSize, "??_7%s6B%s@", &vtableType[4], &s2[4]);
        return result;
    }

    return NULL;
}

ea_t
Vtable::getFuncStart (
    ea_t address
) {
    if (IDAUtils::GetFunctionFlags (address) == -1) {
        return -1;
    }

    if ((IDAUtils::GetFlags (address) & FF_FUNC) != 0) {
        return address;
    }

    else {
        return IDAUtils::PrevFunction (address);
    }
}


// Get class name for this vtable instance
char * 
Vtable::getClassName (
    ea_t address,
    char *result,
    size_t resultSize
) {
    ea_t offset = get_long (address + 4);
    address = get_long (address + 16); // Class Hierarchy Descriptor

    ea_t a = get_long (address + 12); // pBaseClassArray
    size_t numBaseClasses = get_long (address + 8);  //numBaseClasses
    size_t i = 0;
    result[0] = '\0';
    
    while (i < numBaseClasses) 
    {
        ea_t p = get_long (a);

        if (get_long (p + 8) == offset) {
            // Found it
            return IDAUtils::GetAsciizStr (get_long (p) + 8, result, resultSize);
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

        if (get_long (p + 12) != -1)  {
            return IDAUtils::GetAsciizStr (get_long (p) + 8, result, resultSize);
        }

        i++;
        a += 4;
    }

    return result;
}

void
Vtable::createStruct (
    ea_t vtableAddress,
    size_t methodsCount,
    char *className  
) {
    if (strlen (className) == 0) {
        return;
    }

    char structName [4096] = {0};
    sprintf_s (structName, sizeof (structName), "%s_vtable", className);

    tid_t struct_id = IDAUtils::GetStrucIdByName (structName);

    if (struct_id != -1) {
        return;
    }

    struct_id = IDAUtils::AddStrucEx (-1, structName, 0);

    if (struct_id == -1) {
        msg ("Could not create the vtable structure %s !\n", structName);
        return;
    }

    for (size_t i = 0 ; i < methodsCount ; i++)
    {
        char methodName[4096] = {0};
        ea_t methodAddress = get_long (vtableAddress + i * 4);
        if (!methodAddress) {
            continue;
        }

        IDAUtils::Name (methodAddress, methodName, sizeof (methodName));

        if (strlen (methodName) == 0) {
            continue;
        }

        IDAUtils::ForceMethodMember (struct_id, i * 4, methodName, sizeof (methodName));
    }
}

Vtable *
Vtable::parse (
    ea_t address,
    size_t methodsCount
) {
    char typeName [4096] = {0};
    char className [4096] = {0};
    char classNameMangled [4096] = {0};
    char COLName[4096] = {0};

    getTypeName (address, typeName, sizeof (typeName));
    Vtable *result = NULL;

    if (strncmp (typeName, ".?A", 3) == 0)
    {
        IDAUtils::Unknown (address - 4, 4);
        IDAUtils::SoftOff (address - 4);
        ea_t i = get_long (address - 4);  // COL
        ea_t s2 = get_long (i + 4); // offset
        i = get_long (i + 16); // CHD
        i = get_long (i + 4);  // Attributes

        char className [4096] = {0};

        if ((i & 3) == 0 && s2 == 0) {
            // Single inheritance, so we don't need to worry about duplicate names (several vtables)
            sprintf_s (classNameMangled, sizeof (classNameMangled), "??_7%s6B@", &typeName[4]);

            // Set the VFTable name
            IDAUtils::MakeName (address, classNameMangled);
            
            // Get the demangled name
            get_short_name (BADADDR, address, className, sizeof (className));

            result = new Vtable (address, className, methodsCount);

            // Set the RTTI Complete Object Locator name
            sprintf_s (COLName, sizeof (COLName), "??_R4%s6B@", &typeName[4]);
            IDAUtils::MakeName (get_long (address - 4), COLName);
        }

        else {
            // Multiple inheritance
            char buffer [4096] = {0};
            char vtableName [4096] = {0};
            getClassName (get_long (address - 4), vtableName, sizeof (vtableName));
            sprintf_s (buffer, sizeof (buffer), "%s6B%s@", &typeName[4], &vtableName[4]);
            sprintf_s (vtableName, sizeof (vtableName),  "??_7%s", buffer);
            sprintf_s (COLName, sizeof (COLName), "??_R4%s", buffer);

            // Set the VFTable name
            IDAUtils::MakeName (address, vtableName);

            // Get the demangled name
            get_short_name (BADADDR, address, className, sizeof (className));
            
            // Filter the class name
            result = new Vtable (address, className, methodsCount);
            
            // Set the RTTI Complete Object Locator name
            IDAUtils::MakeName (get_long (address - 4), COLName);
        }

        if (result != NULL) {
            createStruct (address, methodsCount, result->className);
        }
    }

    return result;
}


//check for `scalar deleting destructor'
ea_t
Vtable::checkSDD (
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
            IDAUtils::MakeName (address, IDAUtils::MakeSpecialName (name, t, 0, buffer, sizeof (buffer)));
        }

        if (a != BADADDR) {
            if (name != NULL) {
                IDAUtils::MakeName(a, IDAUtils::MakeSpecialName(name, SN_vdestructor, 0, buffer, sizeof (buffer)));
            }
        }

        IDAUtils::CommentStack (address, 4, "__flags$", -1);
    }

    return t;
}


/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/

#include "RECPP.h"
#include "CompleteObjectLocator.h"

// Hex-Rays API pointer
hexdsp_t *hexdsp = NULL;
static bool inited = false;

/*
* @brief : Get a vtable size
* @param curAddress : The vtable address. Can point to any address.
* @return 0 if no vtable is at \address, or the number of methods of the vtable if detected
*/
static size_t 
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
static char *
get_type_name (
    ea_t vtable
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
    char buffer[1000] = {0};
    size_t bufferPos = 0;

    while (curByte = get_byte (x)) {
        buffer [bufferPos++] = curByte;
        x++;
        if (bufferPos > sizeof (buffer)) {
            msg ("Type name too long, cannot continue.\n");
            return NULL;
        }
    }

    return _strdup (buffer);
}



// Get class name for this vtable instance
char * 
get_vtable_class_name (
    ea_t address
) {
    ea_t offset = get_long (address + 4);
    address = get_long (address + 16); // Class Hierarchy Descriptor

    ea_t a = get_long (address + 12); // pBaseClassArray
    size_t numBaseClasses = get_long (address + 8);  //numBaseClasses
    size_t i = 0;
    char *s = "";
    
    while (i < numBaseClasses) 
    {
        ea_t p = get_long (a);
        // BaseClass[%02d]:  %08.8Xh\n", i, p
        if (get_long (p + 8) == offset) {
            // Found it
            return IDAUtils::getAsciizStr(get_long (p) + 8);
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
            return IDAUtils::getAsciizStr(get_long (p) + 8);
        }

        i++;
        a += 4;
    }

    return s;
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
    char *typeName = get_type_name (address);

    if (strncmp (typeName, ".?A", 3) == 0)
    {
        msg ("RTTICompleteObjectLocator:      %08.8Xh\n", get_long (address - 4));
        CompleteObjectLocator *col = new CompleteObjectLocator (get_long (address - 4));
        IDAUtils::Unknown (address - 4, 4);
        IDAUtils::SoftOff (address - 4);
        ea_t i = get_long (address - 4);  // COL
        ea_t s2 = get_long (i + 4); // offset
        i = get_long (i + 16); // CHD
        i = get_long (i + 4);  // Attributes

        if ((i & 3) == 0 && s2 == 0) {
            // Single inheritance, so we don't need to worry about duplicate names (several vtables)
            char *completeName;
            
            completeName = asprintf ("??_7%s6B@", &typeName[4]);
            IDAUtils::MakeName (address, completeName);
            free (completeName);
            
            completeName = asprintf ("??_R4%s6B@", &typeName[4]);
            IDAUtils::MakeName (get_long (address - 4), completeName);
            free (completeName);
        }

        else { 
            // Message ("Multiple inheritance\n");
            char *s2 = get_vtable_class_name (get_long (address - 4));
            char *newName = asprintf ("%s6B%s@", &typeName[4], &s2[4]);
            char *newName2 = asprintf ("??_7%s", newName);
            char *newName3 = asprintf ("??_R4%s", newName);
            IDAUtils::MakeName (address, newName2);
            IDAUtils::MakeName (get_long (address - 4), newName3);
        }
    }
}

char *
get_type_name_by_col (
    ea_t colAddress
) {
    ea_t x = get_long (colAddress + 12);
    
    if (x == BADADDR || !x) {
        return NULL;
    }

    return IDAUtils::getAsciizStr (x + 8);
}

char *
get_vtable_class_name_2 (
    ea_t colAddress
) {
    char *s = get_type_name_by_col (colAddress);
    ea_t i = get_long (colAddress + 16); // CHD
    i = get_long (i+4);  // Attributes

    if ((i & 3) == 0 && get_long (colAddress + 4) == 0) { 
        //Single inheritance, so we don't need to worry about duplicate names (several vtables)
        return asprintf ("??_7%s6B@", &s[4]);
    }

    else {
        // Multiple inheritance 
        char *s2 = get_vtable_class_name (colAddress);
        return asprintf ("??_7%s6B%s@", &s[4], &s2[4]);
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
    char *s;
    //Message("checking function at %a\n",x);

    if ((name != NULL) 
    && (strncmp (IDAUtils::Name (address), "??_", 3) == 0)
    && (str_pos (IDAUtils::Name (address), &name[4]) == 4)
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
        msg ("  %a: thunk to %a\n", address, a);
        t = checkSDD (a, name, vtable, 0);
        if (t && name != NULL) {
            //rename this function as a thunk
            IDAUtils::MakeName (address, IDAUtils::MakeSpecialName (name, t, IDAUtils::Byte (address + 2)));
        }

        return t;
    }

    else if (IDAUtils::matchBytes (address,"81E9????????E9")) {
        // thunk
        // 81 E9 xx xx xx xx        sub     ecx, xxxxxxxx
        // E9 xx xx xx xx           jmp     class::`scalar deleting destructor'(uint)
        a = IDAUtils::getRelJmpTarget (address+6);
        msg("  %a: thunk to %a\n", address, a);
        t = checkSDD (a, name, vtable, 0);

        if (t && name != NULL) {
            //rename this function as a thunk
            IDAUtils::MakeName (address, IDAUtils::MakeSpecialName (name,t, IDAUtils::Dword (address+2)));
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
        if (gate && IDAUtils::Byte(a)==0xE9)
        {
            a = IDAUtils::getRelJmpTarget(a);
        }
        t=SN_scalardtr;
        }
        else if (IDAUtils::matchBytes (address,"568DB1????????578DBE????????8BCFE8????????F644240C01"))
        {
        //56                             push    esi
        //8D B1 xx xx xx xx              lea     esi, [ecx-XX]
        //57                             push    edi
        //8D BE xx xx xx xx              lea     edi, [esi+XX]
        //8B CF                          mov     ecx, edi
        //E8 xx xx xx xx                 call    class::~class()
        //F6 44 24 0C 01                 test    [esp+4+arg_0], 1
        a = IDAUtils::getRelCallTarget (address+16);
        if (gate && IDAUtils::Byte(a)==0xE9)
        {
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
        msg ("  vector deleting destructor at %a\n", address);
        if (name != 0) {
            a = IDAUtils::Dword (address+21);
        }
        if (gate && IDAUtils::Byte(a)==0xE9) {
            a = IDAUtils::getRelJmpTarget(a);
        }
    }

    if (t > 0) {
        if (t == SN_vectordtr) {
            s = "vector";
        }
        else {
            s = "scalar";
        }
        msg ("  %s deleting destructor at %a\n", s, address);
        
        if (name != 0) {
            IDAUtils::MakeName (address, IDAUtils::MakeSpecialName (name, t, 0));
        }

        if (a != BADADDR) {
            msg ("  virtual destructor at %a\n",a);
            if (name != 0) {
                IDAUtils::MakeName(a, IDAUtils::MakeSpecialName(name, SN_vdestructor,0));
            }
        }

        IDAUtils::CommentStack (address, 4, "__flags$",-1);
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

    if (!vtableMethodsCount) {
        // No vtable found at this address, return the next address
        return address + 4;
    }

    msg ("%08.8Xh: possible vtable (%d methods)\n", address, vtableMethodsCount);
    
    //check if it's named as a vtable
    char *name = IDAUtils::Name (address);
    if (strncmp (name, "??_7", 4) != 0) {
        name = NULL;
    }
    
    endTable = get_long (address - 4);

    if (CompleteObjectLocator::isValid (endTable)) {
        parse_vtable (address);
        if (name == NULL) {
            name = get_vtable_class_name_2 (endTable);
        }
        // only output object tree for main vtable
        if (get_long (endTable + 4) == 0) {
            CRTTIClassHierarchyDescriptor::parse2 (get_long (endTable + 16));
        }

        IDAUtils::MakeName(address, name);
    }

    if (name != NULL) {
        char *s = IDAUtils::Demangle (name, 0x00004006);
        msg ("> %s\n",s);
        int typeinfoPos = str_pos (name, "@@6B");
        if (typeinfoPos == -1) {
            msg ("typeinfoPos error.");
        }
        char *nameDup = strdup (name);
        nameDup[typeinfoPos+2] = '\0';
        //convert vtable name into typeinfo name
        name = asprintf (".?AV%s", &nameDup[4]);
    }
    
    IDAUtils::DeleteArray (IDAUtils::GetArrayId ("AddrList"));
    msg ("  referencing functions: \n");
    int q = 0; int i = 1;

    char *s = "";
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
                    if (i > 1) {
                        s = asprintf("  %x (%d times)", q, i);
                    }
                    else {
                        s = asprintf("  %x",q);
                    }
    
                    if (IDAUtils::hasName (IDAUtils::GetFlags (q))) {
                        s = asprintf ("%s (%s)", s, IDAUtils::Demangle (IDAUtils::Name (q), 8));
                    }
                    s = asprintf ("%s\n", s);
                    msg (s);
                    IDAUtils::AddAddr(q);
                }
                i = 1;
                q = p;
            }
        }
    }
    if (q)
    {           
        if (i > 1) {
            s = asprintf("  %a (%d times)", q, i);
        }
    
        else {
            s = asprintf("  %x", q);
        }
        
        if (IDAUtils::hasName (IDAUtils::GetFlags (q))) {
            s = asprintf ("%s (%s)", s, IDAUtils::Demangle (IDAUtils::Name (q), 8));
        }
        
        s = asprintf ("%s\n", s);
        msg (s);
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
    msg ("\n");
    
    return endTable;
}

static bool idaapi 
user_menu_scan_vftable (
    void *__unused
) {
    // Get .text and .rdata segments boundaries
    segment_t *textSeg  = get_segm_by_name (".text");
    segment_t *rdataSeg = get_segm_by_name (".rdata");

    if (!textSeg || !rdataSeg) {
        msg ("Error : Cannot find the .text or .rdata segment.");
        return false;
    }

    ea_t rMin = rdataSeg->startEA, 
         rMax = rdataSeg->endEA, 
         cMin = textSeg->startEA, 
         cMax = textSeg->endEA;
    
    if (rMin == 0) {
        rMin = cMin; 
        rMax = cMax;
    }

    msg (".rdata: %08.8Xh - %08.8Xh, .text %08.8Xh - %08.8Xh\n", rMin, rMax, cMin, cMax);

    ea_t curAddress = rMin;
    ea_t x;
    
    while (curAddress < rMax) {
        x = get_long (curAddress);

        // Methods should reside in .text
        if (x >= cMin && x < cMax) {
            curAddress = do_vtable (curAddress);
        }
        else {
            curAddress += 4;
        }
    }

    msg ("Finished !\n");

    return true;
}

static int idaapi 
ui_callback (
    void * __unused, 
    int notification_code, 
    va_list va
) {
    TCustomControl *view = va_arg (va, TCustomControl *);

    switch (notification_code)
    {
        case view_popup:
            add_custom_viewer_popup_item(view, "Scan vftables", "Ctrl-5", user_menu_scan_vftable, NULL);
        break;
    }

    return 0;
}

/*
 * @brief Initialize the RECPP plugin 
 */
int idaapi 
init (
    void
) {
    if (!init_hexrays_plugin ()) {
	    return PLUGIN_SKIP;
    }

    msg ("=====================[ RE CPP ]=====================\n");
    hook_to_notification_point (HT_VIEW, ui_callback, NULL); 
    // install_hexrays_callback (callback, NULL);
    inited = true;

    return PLUGIN_KEEP;
}

/*
 * @brief Terminate the RECPP plugin 
 */
void idaapi 
term (
    void
) {
    if (inited) {
        // remove_hexrays_callback (callback, NULL);
        term_hexrays_plugin ();
    }
}

/*
 * @brief Run the RECPP plugin 
 */
void idaapi 
run (
    int __unused
) {
}


/*
 * Register the RECPP plugin
 */
plugin_t PLUGIN = {
	IDP_INTERFACE_VERSION,
    PLUGIN_HIDE,          // plugin flags
    init,                 // initialize
    term,                 // terminate. this pointer may be NULL.
    run,                  // invoke plugin
    "RECPP IDA Plugin",   // long comment about the plugin
                          // it could appear in the status line or as a hint
    "",                   // multiline help about the plugin
    "RECPP",              // the preferred short name of the plugin
    "Alt-D"               // the preferred hotkey to run the plugin
};
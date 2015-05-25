/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/
#include "RTTIClassHierarchyDescriptor.h"
#include "RTTIBaseClassDescriptor.h"
#include "IDAUtils.h"


void
CRTTIClassHierarchyDescriptor::parse (
    ea_t address
) {
    if (address == BADADDR || !address) {
        return;
    }

    char buffer[2048] = {0};

    ea_t a = get_long (address + 4);

    IDAUtils::DwordCmt(address, "signature");
    IDAUtils::DwordCmt(address + 4, "attributes");
    IDAUtils::DwordCmt(address + 8, "numBaseClasses");
    IDAUtils::OffCmt(address + 12, "pBaseClassArray");

    a = get_long (address + 12);
    ea_t n = get_long (address + 8);
    ea_t i = 0;
    
    // IDAUtils::DumpNestedClass (a, indent, n);

    while (i < n) 
    {
        ea_t p = get_long (a);

        sprintf_s (buffer, sizeof (buffer), "BaseClass[%02d]", i);
        IDAUtils::OffCmt(a, buffer);
        
        if (i == 0) {
            char buffer2[2048] = {0};
            char *s = CRTTIBaseClassDescriptor::parse (p, buffer2, sizeof (buffer2));
            //??_R2A@@8 = A::`RTTI Base Class Array'
            sprintf_s (buffer, sizeof (buffer), "??_R2%s8", &s[4]);
            IDAUtils::MakeName(a, buffer);

            //??_R3A@@8 = A::`RTTI Class Hierarchy Descriptor'
            sprintf_s (buffer, sizeof (buffer), "??_R3%s8", &s[4]);
            IDAUtils::MakeName (address, buffer);
        }
        else {
            char buffer2[2048] = {0};
            CRTTIBaseClassDescriptor::parse (p, buffer2, sizeof (buffer2));
        }

        i++;
        a += 4;
    }
}


void
CRTTIClassHierarchyDescriptor::parse2 (
    ea_t address
) {
    return;
}
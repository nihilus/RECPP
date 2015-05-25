/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/
#include "RTTIBaseClassDescriptor.h"
#include "TypeDescriptor.h"
#include "IDAUtils.h"


char *
CRTTIBaseClassDescriptor::parse (
    ea_t address
) {
    if (address == BADADDR || !address) {
        return NULL;
    }

    IDAUtils::OffCmt(address, "pTypeDescriptor");
    IDAUtils::DwordCmt(address + 4, "numContainedBases");
    IDAUtils::DwordArrayCmt(address + 8, 3, "PMD where");
    IDAUtils::DwordCmt(address + 20, "attributes");

    char *s = CTypeDescriptor::parse (get_long (address));
    //??_R1A@?0A@A@B@@8 = B::`RTTI Base Class Descriptor at (0,-1,0,0)'
    char *m1 = IDAUtils::MangleNumber (get_long (address + 8));
    char *m2 = IDAUtils::MangleNumber (get_long (address + 12));
    char *m3 = IDAUtils::MangleNumber (get_long (address + 16));
    char *m4 = IDAUtils::MangleNumber (get_long (address + 20));

    char *newName = asprintf ("??_R1%s%s%s%s%s8", m1, m2, m3, m4, &s[4]);
    IDAUtils::MakeName (address, newName);
    
    free (m1);
    free (m2);
    free (m3);
    free (m4);
    free (newName);

    return s;
}

#include "TypeDescriptor.h"
#include "IDAUtils.h"

char *
CTypeDescriptor::parse (
    ea_t address
) {
    if (address == BADADDR || !address) {
        return false;
    }

    char buffer[2048] = {0};
    char buffer2[2048] = {0};

    char *a = IDAUtils::getAsciizStr (address + 8, buffer2, sizeof (buffer2));

    msg ("    pVFTable:          %08.8Xh\n", get_long (address));
    msg ("    spare:             %08.8Xh\n", get_long (address + 4));
    msg ("    name:              '%s'\n", a);

    IDAUtils::OffCmt (address, "pVFTable");
    IDAUtils::OffCmt (address + 4, "spare");
    IDAUtils::StrCmt (address + 8, "name");

    //??_R0?AVA@@@8 = A `RTTI Type Descriptor'
    asprintf (buffer, "??_R0%s@8", &a[1]);
    IDAUtils::MakeName (address, buffer);
    
    return a;
}


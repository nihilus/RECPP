/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/

#include "TypeDescriptor.h"
#include "IDAUtils.h"

char *
CTypeDescriptor::parse (
    ea_t address,
    char *buffer,
    size_t bufferSize
) {
    if (address == BADADDR || !address) {
        return false;
    }

    char buffer2[2048] = {0};

    char *a = IDAUtils::GetAsciizStr (address + 8, buffer, sizeof (buffer));

    IDAUtils::OffCmt (address, "pVFTable");
    IDAUtils::OffCmt (address + 4, "spare");
    IDAUtils::StrCmt (address + 8, "name");

    //??_R0?AVA@@@8 = A `RTTI Type Descriptor'
    sprintf_s (buffer2, sizeof (buffer2), "??_R0%s@8", &a[1]);
    IDAUtils::MakeName (address, buffer2);
    
    return a;
}


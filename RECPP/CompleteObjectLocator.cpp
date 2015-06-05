/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/

#include "CompleteObjectLocator.h"
#include "IDAUtils.h"

void
CompleteObjectLocator::parse (
    ea_t address
) {
    if (address == BADADDR || !address) {
        return;
    }
    
    char buffer2[2048] = {0};
    char *cloName = IDAUtils::GetAsciizStr (get_long (address + 12) + 8, buffer2, sizeof (buffer2));

    IDAUtils::DwordCmt (address, "signature");
    IDAUtils::DwordCmt (address + 4, "offset");
    IDAUtils::DwordCmt (address + 8, "cdOffset");
    IDAUtils::OffCmt (address + 12, "pTypeDescriptor");
    IDAUtils::OffCmt (address + 16, "pClassDescriptor");

    CRTTIClassHierarchyDescriptor::parse (get_long (address + 16));
}

bool
CompleteObjectLocator::isValid (
    ea_t address
) {
    ea_t x = get_long (address + 12);

    if (!x || (x == BADADDR)) {
        return 0;
    }

    x = get_long (x + 8);

                          // .?A
    if ((x & 0xFFFFFF) == 0x413F2E) {
        return true;
    }

    return false;
}


char *
CompleteObjectLocator::get_type_name_by_col (
    ea_t colAddress,
    char *buffer,
    size_t bufferSize
) {
    ea_t x = get_long (colAddress + 12);
    
    if (x == BADADDR || !x) {
        return NULL;
    }

    return IDAUtils::GetAsciizStr (x + 8, buffer, bufferSize);
};
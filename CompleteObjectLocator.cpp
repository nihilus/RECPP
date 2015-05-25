#include "CompleteObjectLocator.h"
#include "IDAUtils.h"


CompleteObjectLocator::CompleteObjectLocator (
    ea_t address
) {
    this->signature = address;
    this->offset = address + 4;
    this->cdOffset = address + 8;
    this->pTypeDescriptor = NULL; // new TypeDescriptor (address + 12);
    this->pClassDescriptor = NULL; // new RTTIClassHierarchyDescriptor (address + 16);
}


CompleteObjectLocator::~CompleteObjectLocator () {
}



void
CompleteObjectLocator::parse (
    ea_t address
) {
    if (address == BADADDR || !address) {
        return;
    }
    
    char buffer[2048] = {0};
    char buffer2[2048] = {0};
    char *cloName = IDAUtils::getAsciizStr (get_long (address + 12) + 8, buffer2, sizeof (buffer2));

    msg ("    signature:         %08.8Xh\n", get_long (address));
    msg ("    offset:            %08.8Xh\n", get_long (address + 4));
    msg ("    cdOffset:          %08.8Xh\n", get_long (address + 8));
    msg ("    pTypeDescriptor:   %08.8Xh (%s)\n", get_long (address + 12), IDAUtils::DemangleTIName (cloName, buffer, sizeof (buffer)));
    msg ("    pClassDescriptor:  %08.8Xh\n", get_long (address + 16));

    IDAUtils::DwordCmt (address, "signature");
    IDAUtils::DwordCmt (address + 4, "offset");
    IDAUtils::DwordCmt (address + 8, "cdOffset");
    IDAUtils::OffCmt (address + 12, "pTypeDescriptor");
    IDAUtils::OffCmt (address + 16, "pClassDescriptor");

    //
    // Parse_CHD(get_long (address + 16));
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

    return IDAUtils::getAsciizStr (x + 8, buffer, bufferSize);
};
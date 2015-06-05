/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/

#pragma once

// ---------- Includes ------------
#include "VtableScanner.h"
#include "CompleteObjectLocator.h"

VtableScanner::VtableScanner (DecMap *decMap) {
    this->decMap = decMap;
}

VtableScanner::~VtableScanner () {
}

size_t 
VtableScanner::getVtableMethodsCount (
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

ea_t
VtableScanner::checkVtable (
    ea_t address
) {
    size_t vtableMethodsCount = getVtableMethodsCount (address);
    ea_t endTable;
    ea_t p;

    if (!vtableMethodsCount) {
        // No vtable found at this address, return the next address
        return address + 4;
    }
    
    // Check if it's named as a vtable
    char bufName [4096];
    char *name = IDAUtils::Name (address, bufName, sizeof (bufName));
    if (strncmp (name, "??_7", 4) != 0) {
        name = NULL;
    }
    
    endTable = get_long (address - 4);
    
    char buffer[4096] = {0};
    if (CompleteObjectLocator::isValid (endTable)) {
        Vtable *vtable = Vtable::parse (address, vtableMethodsCount);
        if (vtable) {
            this->vtables.push_back (vtable);
        }
        
        if (name == NULL) {
            name = Vtable::getClassName2 (endTable, buffer, sizeof (buffer));
        }

        // only output object tree for main vtable
        if (get_long (endTable + 4) == 0) {
            CRTTIClassHierarchyDescriptor::parse2 (get_long (endTable + 16));
        }

        IDAUtils::MakeName (address, name);
    }
    
    char buffer2 [4096];
    if (name != NULL) {
        int typeinfoPos = str_pos (name, "@@6B");
        name[typeinfoPos+2] = '\0';
        //convert vtable name into typeinfo name
        sprintf_s (buffer2, sizeof (buffer2), ".?AV%s", &name[4]);
        name = buffer2;
    }
    
    IDAUtils::DeleteArray (IDAUtils::GetArrayId ("AddrList"));
    int q = 0; int i = 1;

    for (endTable = IDAUtils::DfirstB (address); endTable != BADADDR; endTable = IDAUtils::DnextB (address, endTable))
    {
        p = Vtable::getFuncStart (endTable);

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
        
        Vtable::checkSDD (p, name, address, 0);
        vtableMethodsCount--;
        endTable += 4;
    }

    IDAUtils::doAddrList (name);
    
    return endTable;
}

bool
VtableScanner::scan (
    void
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

    ea_t curAddress = rMin;
    ea_t curDword;

    while (curAddress < rMax) {
        curDword = get_long (curAddress);

        // Methods should reside in .text
        if (curDword >= cMin && curDword < cMax) {
            curAddress = this->checkVtable (curAddress);
        }
        else {
            curAddress += 4;
        }
    }

    msg ("Finished !\n");
    msg ("Vtable count = %d", this->vtables.size());

    return true;
}
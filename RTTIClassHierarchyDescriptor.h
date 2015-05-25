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
#include "RECPP.h"
#include "RTTIBaseClassDescriptor.h"

// ---------- Defines -------------


// ------ Class declaration -------
struct _RTTIClassHierarchyDescriptor {
	uint32 signature;
	uint32 attributes;	// bit 0 multiple inheritance, bit 1 virtual inheritance
	size_t numBaseClasses; // at least 1 (all base classes, including itself)
	struct _RTTIBaseClassDescriptor **pBaseClassArray;
};

class CRTTIClassHierarchyDescriptor {
public:
    static void
    parse (
        ea_t address
    );

    static void
    parse2 (
        ea_t address
    );
};
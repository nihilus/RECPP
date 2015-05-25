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
#include "TypeDescriptor.h"
#include "RTTIClassHierarchyDescriptor.h"

// ---------- Defines -------------

// ------ Class declaration -------
typedef struct _PMD2 {
	ptrdiff_t	mdisp;		// vftable offset
	ptrdiff_t	pdisp;		// vbtable offset
	ptrdiff_t	vdisp;		// vftable offset(for virtual base class)
} PMD2;

struct _RTTIBaseClassDescriptor {
	TypeDescriptor*	pTypeDescriptor;
	size_t	numBaseClasses;	// direct base classes
	PMD2	pmd;	// len = 0xC
	uint32	attributes;
	struct _RTTIClassHierarchyDescriptor *pClassHierarchyDescriptor;
};

class CRTTIBaseClassDescriptor {

public:
    static char *
    CRTTIBaseClassDescriptor::parse (
        ea_t address,
        char *buffer3,
        size_t bufferSize
    );
};
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

// ---------- Defines -------------


// ------ Structure declaration -------
struct TypeDescriptor {

    // vtable of type_info class
    const void *pVFTable;

    // used to keep the demangled name returned by type_info::name()
    void *spare;

    // mangled type name, e.g. ".H" = "int", ".?AUA@@" = "struct A", ".?AVA@@" = "class A"
    char name[0];
};

class CTypeDescriptor {
public: 
    static char *
    CTypeDescriptor::parse (
        ea_t address,
        char *buffer,
        size_t bufferSize
    );
};

// ----------- Functions ------------
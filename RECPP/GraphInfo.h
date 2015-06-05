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
#include "CallGraph.h"

// ---------- Defines -------------


// ------ Class definition --------

class GraphInfo
{
// Actual context variables
public:
    CallGraph fg; // associated call graph maker
    ea_t func_ea; // function ea in question
    func_t *function; // Function pointer

// Instance management
private:

    typedef qlist<GraphInfo *> graphinfo_list_t;
    typedef graphinfo_list_t::iterator iterator;
    static graphinfo_list_t instances;

    static bool
    find (
        ea_t func_ea, 
        iterator *out
    );

    GraphInfo (
        ea_t address
    );

public:
    ~GraphInfo (
        void
    );

    static GraphInfo *
    create (
        ea_t func_ea
    );
    
    static GraphInfo *
    find (
        ea_t func_ea
    );
};

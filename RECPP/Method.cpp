/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/

#include "Method.h"
#include "IDAUtils.h"
#include <iostream>
#include <cstdarg>

Method::Method (
    char *className, 
    ea_t methodAddress,
    bool makeName
) {
    if (makeName) {
        this->methodName = IDAUtils::string_sprintf ("%s::sub_%x", className, methodAddress);
        IDAUtils::MakeName (get_long (methodAddress), (char *) this->methodName.c_str());
    }

    this->methodAddress = methodAddress;
    this->function = get_func (this->methodAddress);
    this->graphInfo = GraphInfo::create (this->methodAddress);
}

Method::~Method () 
{
}

void
Method::explore (
    void
) {
    if (!this->function) {
        return;
    }
    
    if (!this->graphInfo) {
        msg ("graphInfo is NULL.\n");
        return;
    }

    CallGraph *fg = &this->graphInfo->fg;

    // Start analysing here
    msg ("[%x] Analyzing %s callgraph...\n", this->methodAddress, this->methodName);

    static funcs_walk_options_t fg_opts = {
        FWO_VERSION,       // version
        FWO_RECURSE_UNLIM, // flags
        0                  // max recursion
    };

    fg->walk_func (this->function, &fg_opts, 2);
}
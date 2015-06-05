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
#include "GraphInfo.h"

// ---------- Defines -------------


// ------ Class definition --------
class Method 
{
    public:
        Method (char *className, ea_t functionAddress, bool makeName);
        virtual ~Method ();
        
        virtual void
        Method::explore (
            void
        );
        
    protected:
        GraphInfo *graphInfo;
        std::string methodName;
        func_t *function;
        ea_t methodAddress;
        ea_t methodStart;

    private:
};
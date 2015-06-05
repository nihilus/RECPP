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

#include "RECPP.h"
#include "Method.h"

class VirtualMethod : public Method 
{
public:
    VirtualMethod::VirtualMethod (
        char *methodName, 
        ea_t functionAddress, 
        ea_t vftableAddress,
        char *forClass
    );

    ~VirtualMethod ();

    static char *
    VirtualMethod::getMethodName (
        char *className
    );

private:
    ea_t vftableAddress;
};
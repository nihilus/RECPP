/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/

#include "VirtualMethod.h"

VirtualMethod::VirtualMethod (
    char *className,
    ea_t vftableAddress,
    size_t methodIndex,
    char *forClass
)
    : Method (className, vftableAddress + methodIndex * 4, false),
    vftableAddress (vftableAddress)
{
    this->methodAddress = vftableAddress + methodIndex * 4;
    this->methodStart = get_long (methodAddress);

    // Check current method name
    char methodName[4096];
    IDAUtils::Name (methodStart, methodName, sizeof (methodName));

    if (strncmp (methodName, "sub_", 4) == 0) {
        if (forClass) {
            this->methodName = IDAUtils::string_sprintf ("%s::virt%d_for_%s", className, methodIndex + 1, forClass);
        }
        else {
            this->methodName = IDAUtils::string_sprintf ("%s::virt%d", className, methodIndex + 1);
        }
    
        IDAUtils::MakeName (methodStart, (char *) this->methodName.c_str ());
    }
}

VirtualMethod::~VirtualMethod ()
{
}
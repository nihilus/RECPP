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
#include "VirtualMethod.h"

// ---------- Defines -------------


// ------ Class definition --------
class Vtable 
{

public:
    Vtable (ea_t address, char *className, size_t virtualMethodsCount);
    ~Vtable ();

    static Vtable *
    Vtable::parse (
        ea_t address,
        size_t methodsCount
    );
    
    /*
     * @brief : Get class name for this vtable instance
     * @return The class name, or NULL if an error occured
     */
    static char * 
    Vtable::getClassName (
        ea_t address,
        char *result,
        size_t resultSize
    );
    

    static ea_t
    Vtable::getFuncStart (
        ea_t address
    );

    /*
     * @brief : Get class name for this vtable instance based on the COL
     * @return The class name, or NULL if an error occured
     */
    static char * 
    Vtable::getClassName2 (
        ea_t colAddress,
        char *result,
        size_t resultSize
    );

    /*
     * @brief : Check if get_long (vtable-4) points to typeinfo record and extract the type name from it
     * @return The type name, or NULL if an error occured
     */
    static char *
    Vtable::getTypeName (
        ea_t vtable,
        char *result,
        size_t resultSize
    );

    static void
    Vtable::createStruct (
        ea_t vtableAddress,
        size_t methodsCount,
        char *className  
    );
    
    /*
     * @brief : check for `scalar deleting destructor'
     * @return The type name, or NULL if an error occured
     */
    static ea_t
    Vtable::checkSDD (
        ea_t address,
        char *name,
        ea_t vtable,
        ea_t gate
    );

private:
    ea_t address;
    char *className;
    std::vector<VirtualMethod *> virtualMethods;
    size_t virtualMethodsCount;
    
    static void
    Vtable::filterClassName (
        char **className,
        char **forClass
    );
};


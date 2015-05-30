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
#include "DecMap.h"
#include <frame.hpp>

// ---------- Defines -------------
extern int vtableCount;


// ------ Class declaration -------


/*
* @brief : Get a vtable size
* @param curAddress : The vtable address. Can point to any address.
* @return 0 if no vtable is at \address, or the number of methods of the vtable if detected
*/
static size_t 
get_vtable_methods_count (
    ea_t curAddress
);


/*
 * @brief : Check if get_long (vtable-4) points to typeinfo record and extract the type name from it
 * @return The type name, or NULL if an error occured
 */
char *
get_type_name (
    ea_t vtable,
    char *buffer,
    size_t bufferSize
);


// Get class name for this vtable instance
char * 
get_vtable_class_name (
    ea_t address,
    char *buffer,
    size_t bufferSize
);


/*
* @brief : Check if there's a vtable at address, and dump into to output
* @param address : The vtable address. Can point to any address.
* @param methodsCount : Number of methods in the current vtable
* @return position after the end of vtable
*/
void 
parse_vtable (
    ea_t address,
    size_t methodsCount
);


char *
get_vtable_class_name_2 (
    ea_t colAddress,
    char *buffer,
    size_t bufferSize
);


ea_t 
get_func_start (
    ea_t address
);


//check for `scalar deleting destructor'
ea_t
checkSDD (
    ea_t address,
    char *name,
    ea_t vtable,
    ea_t gate
);


/*
* @brief : Check if there's a vtable at address, and dump into to output
* @param decMap : An allocated DecMap
* @param address : The vtable address. Can point to any address.
* @return position after the end of vtable
*/
ea_t 
do_vtable (
    DecMap *decMap,
    ea_t address
);

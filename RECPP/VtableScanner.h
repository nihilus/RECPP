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
#include "Vtable.h"
#include "DecMap.h"

// ---------- Defines -------------


// ------ Class definition --------
class VtableScanner {
    public:
    VtableScanner (DecMap *decMap);
    ~VtableScanner ();
    
    bool
    VtableScanner::scan (
        void
    );
    

    ea_t
    VtableScanner::checkVtable (
        ea_t address
    );

    private:
        std::vector <Vtable *> vtables;
        DecMap *decMap;
        
        /*
        * @brief : Get a vtable size
        * @param curAddress : The vtable address. Can point to any address.
        * @return 0 if no vtable is at \address, or the number of methods of the vtable if detected
        */
        static size_t 
        VtableScanner::getVtableMethodsCount (
            ea_t curAddress
        );
};


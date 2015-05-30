#include "DecMap.h"


DecMap::DecMap () {
}


DecMap::~DecMap () {
}


//--------------------------------------------------------------------------
void DecMap::process (cfunc_t *cfunc)
{
    ea_cf_map_t::const_iterator it = ea2cf.find (cfunc->entry_ea);
    if (it != ea2cf.end ()) {
        // Already exists
        return;
    }

    ea_gi_map_t::const_iterator it2 = ea2gi.find (cfunc->entry_ea);
    if (it2 == ea2gi.end ()) {
        // Don't exist, but it should do!
        return;
    }

    ea2cf [cfunc->entry_ea] = cfunc;
}

//--------------------------------------------------------------------------
void DecMap::decompile_function (graph_info_t *gi, ea_t func_ea)
{
    ea_gi_map_t::const_iterator it = ea2gi.find (func_ea);
    if (it != ea2gi.end ()) {
        // Already exists
        return;
    }

    // Get function
    func_t *pFn = get_func (func_ea);
    if (!pFn) {
        return;
    }
    
    // Everything is ok from here, add the graph info to the map
    ea2gi [func_ea] = gi;

    // Request to decompile function
    hexrays_failure_t hf;
    cfunc_t *cfunc = decompile (pFn, &hf);
    if (!cfunc) {
        return;
    }
}

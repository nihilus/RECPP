/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/


#include "GraphInfo.h"

GraphInfo::graphinfo_list_t GraphInfo::instances;

bool GraphInfo::find (
    ea_t func_ea, 
    iterator *out
) {
    iterator end = instances.end();

    for (iterator it = instances.begin(); it != end; it++)
    {
        if ((*it)->func_ea == func_ea)
        {
            if (out != NULL) {
                *out = it;
            }

            return true;
        }
    }

    return false;
}

GraphInfo *GraphInfo::find (
    ea_t func_ea
) {
    iterator it;

    if (!find (func_ea, &it)) {
        return NULL;
    }

    return *it;
}

GraphInfo *
GraphInfo::create (
    ea_t func_ea
) {
    GraphInfo *r = find (func_ea);

    // Not there ? create it
    if (r == NULL) 
    {
        r = new GraphInfo (func_ea);
        if (!r->function) {
            return NULL;
        }

        instances.push_back (r);
    }

    return r;
}

GraphInfo::GraphInfo (
    ea_t address
)  {
    if (!(this->function = get_func (address))) {
        return;
    }

    this->func_ea = this->function->startEA;

    static funcs_walk_options_t fg_opts = {
        FWO_VERSION,       // version
        FWO_RECURSE_UNLIM, // flags
        0                  // max recursion
    };

    this->fg.walk_func (this->function, &fg_opts, 2);
}

GraphInfo::~GraphInfo () 
{
}
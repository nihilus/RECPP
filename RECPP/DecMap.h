#pragma once

#include <ida.hpp>
#include <idp.hpp>
#include <graph.hpp>
#include <hexrays.hpp>
#include <loader.hpp>
#include <kernwin.hpp>
#include "callgraph.h"

class graph_info_t;

class DecMap {
public:
    DecMap ();
    ~DecMap ();

    int node_count;
    
    // Map address -> cfunc_t
    typedef std::map<ea_t, cfunc_t *> ea_cf_map_t;
    typedef std::map<ea_t, graph_info_t *> ea_gi_map_t;
    ea_cf_map_t ea2cf;
    ea_gi_map_t ea2gi;

    void DecMap::decompile_function (graph_info_t *gi, ea_t func_ea);
    void DecMap::process (cfunc_t *cfunc);
};


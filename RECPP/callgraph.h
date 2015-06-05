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

// ---------- Defines -------------


// ------ Class definition --------
// function call graph creator class
struct funcs_walk_options_t
{
    #define FWO_VERSION 1 // current version of options block
    int32 version;

    #define FWO_SKIPLIB       0x0001 // skip library functions
    #define FWO_RECURSE_UNLIM 0x0002 // unlimited recursion
    int32 flags;

    int32 recurse_limit; // how deep to recurse (0 = unlimited)
};

class CallGraph
{
public:
    CallGraph::CallGraph ();
    int node_count;

    // current node search ptr
    int  cur_node;
    char cur_text[MAXSTR];

    bool visited(ea_t func_ea, int *nid);
    int  add(ea_t func_ea);

    // edge structure
    struct edge_t
    {
        int id1;
        int id2;
        edge_t(int i1, int i2): id1(i1), id2(i2) { }
        edge_t(): id1(0), id2(0) { }
    };
    typedef qlist<edge_t> edges_t;

    // edge manipulation
    typedef edges_t::iterator edge_iterator;
    void create_edge(int id1, int id2);
    edge_iterator begin_edges() { return edges.begin(); }
    edge_iterator end_edges() { return edges.end(); }
    void clear_edges();

    // find nodes by text
    int find_first(const char *text);
    int find_next();
    const char *get_findtext() { return cur_text; }
    const int count() const { return node_count; }
    void reset();

    // node / func info
    struct funcinfo_t
    {
        func_t *func;
        qstring name;
        bgcolor_t color;
        ea_t ea;
    };
    typedef std::map<int, funcinfo_t> int_funcinfo_map_t;
    int_funcinfo_map_t cached_funcs;

    funcinfo_t *get_info (int nid);

    // function name manipulation
    const ea_t get_addr(int nid);
    const char *get_name(int nid);
    func_t *CallGraph::get_function(int nid);

    int walk_func (func_t *func, funcs_walk_options_t *o = NULL, int level = 1);

private:
    edges_t edges;

    // node id to func addr and reverse lookup
    typedef std::map<ea_t, int> ea_int_map_t;
    typedef std::map<int, ea_t> int_ea_map_t;
    ea_int_map_t ea2node;
    int_ea_map_t node2ea;
};
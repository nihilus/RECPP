#pragma warning(disable: 4800 4018)
/*
*  This is a sample plugin module.
*  It demonstrates how to create build a function call graph
*
*  It can be compiled by the following compilers:
*
*      - Borland C++, CBuilder, free C++
*
*/

#include <windows.h>

#include <list>
#include <map>

#include "callgraph.h"
#ifdef __BORLANDC__
  #include "../../module/memredir.cpp"
#endif
//--------------------------------------------------------------------------
bool callgraph_t::visited(ea_t func_ea, int *nid)
{
  ea_int_map_t::const_iterator it = ea2node.find(func_ea);
  if ( it != ea2node.end() )
  {
    if ( nid != NULL )
      *nid = it->second;
    return true;
  }
  return false;
}

//--------------------------------------------------------------------------
int callgraph_t::walk_func(func_t *func, funcs_walk_options_t *opt, int level)
{
  // add a node for this function
  int id = add(func->startEA);

  func_item_iterator_t fii;
  for ( bool fi_ok=fii.set(func); fi_ok; fi_ok=fii.next_code() )
  {
    xrefblk_t xb;
    for ( bool xb_ok = xb.first_from(fii.current(), XREF_FAR);
      xb_ok && xb.iscode;
      xb_ok = xb.next_from() )
    {
      int id2;
      if ( !visited(xb.to, &id2) )
      {
        func_t *f = get_func(xb.to);
        if ( f == NULL || func_contains(func, xb.to) )
          continue;

        bool skip = false;

        if ( opt != NULL )
        {
          skip =
            // skip lib funcs?
            (((f->flags & FUNC_LIB) != 0) && ((opt->flags & FWO_SKIPLIB) != 0))
            // max recursion is off, and limit is reached?
            || (  ((opt->flags & FWO_RECURSE_UNLIM) == 0)
            && (level > opt->recurse_limit) );
        }

        if ( skip )
          id2 = add(f->startEA);
        else
          id2 = walk_func(f, opt, level+1);
      }
      create_edge(id, id2);
    }
  }
  return id;
}

//--------------------------------------------------------------------------
int callgraph_t::find_first(const char *text)
{
  if ( text == NULL || text[0] == '\0' )
    return -1;
  qstrncpy(cur_text, text, sizeof(cur_text));
  cur_node = 0;
  return find_next();
}

//--------------------------------------------------------------------------
int callgraph_t::find_next()
{
  for ( int i = cur_node; i < node_count; i++ )
  {
    const char *s = get_name(i);
    if ( stristr(s, cur_text) != NULL )
    {
      cur_node = i + 1;
      return i;
    }
  }
  // reset search
  cur_node = 0;
  // nothing is found
  return -1;
}

//--------------------------------------------------------------------------
inline void callgraph_t::create_edge(int id1, int id2)
{
  edges.push_back(edge_t(id1, id2));
}

//--------------------------------------------------------------------------
void callgraph_t::reset()
{
  node_count = 0;
  cur_node = 0;
  cur_text[0] = '\0';
  ea2node.clear();
  node2ea.clear();
  cached_funcs.clear();
  edges.clear();
}

//--------------------------------------------------------------------------
const ea_t callgraph_t::get_addr(int nid)
{
  int_ea_map_t::const_iterator it = node2ea.find(nid);
  return it == node2ea.end() ? BADADDR : it->second;
}

//--------------------------------------------------------------------------
callgraph_t::funcinfo_t *callgraph_t::get_info(int nid)
{
  funcinfo_t *ret = NULL;

  do
  {
    // returned cached name
    int_funcinfo_map_t::iterator it = cached_funcs.find(nid);
    if ( it != cached_funcs.end() )
    {
      ret = &it->second;
      break;
    }

    // node does not exist?
    int_ea_map_t::const_iterator it_ea = node2ea.find(nid);
    if ( it_ea == node2ea.end() )
      break;

    func_t *pfn = get_func(it_ea->second);
    if ( pfn == NULL )
      break;

    funcinfo_t fi;

    // get name
    char buf[MAXSTR];
    if ( get_func_name(it_ea->second, buf, sizeof(buf)) == NULL )
      fi.name = "?";
    else
      fi.name = buf;

    // get color
    fi.color = calc_bg_color(pfn->startEA);

    fi.ea = pfn->startEA;

    // Get function pointers
    fi.func = pfn;

    // Decompile it
    DecMap *decMap = this->graphInfo->decMap;
    decMap->decompile_function (this->graphInfo, pfn->startEA);

    it = cached_funcs.insert(cached_funcs.end(), std::make_pair(nid, fi));
    ret = &it->second;
  } while ( false );

  return ret;
}

//--------------------------------------------------------------------------
const char *callgraph_t::get_name(int nid)
{
  funcinfo_t *fi = get_info(nid);
  if ( fi == NULL )
    return "?";
  else
    return fi->name.c_str();
}

//--------------------------------------------------------------------------
void callgraph_t::setGraphInfo(graph_info_t *graphInfo)
{
    this->graphInfo = graphInfo;
}

//--------------------------------------------------------------------------
func_t *callgraph_t::get_function(int nid)
{
    funcinfo_t *fi = get_info (nid);

    if (fi == NULL) {
        return NULL;
    }

    return fi->func;
}

//--------------------------------------------------------------------------
int callgraph_t::add(ea_t func_ea)
{
  ea_int_map_t::const_iterator it = ea2node.find(func_ea);
  if ( it != ea2node.end() )
    return it->second;

  ea2node[func_ea]    = node_count;
  node2ea[node_count] = func_ea;
  return node_count++;
}

//--------------------------------------------------------------------------
callgraph_t::callgraph_t ()
{
    node_count = 0;
    cur_text[0] = '\0';
}

//--------------------------------------------------------------------------
void callgraph_t::clear_edges()
{
  edges.clear();
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
graph_info_t::graphinfo_list_t graph_info_t::instances;

//--------------------------------------------------------------------------
graph_info_t::graph_info_t (DecMap *decMap)
{
    refresh_needed = true;
    form = NULL;
    gv = NULL;
    fg.setGraphInfo (this);
}

//--------------------------------------------------------------------------
bool graph_info_t::find(const ea_t func_ea, iterator *out)
{
  iterator end = instances.end();
  for ( iterator it = instances.begin(); it != end; ++it )
  {
    if ( (*it)->func_ea == func_ea )
    {
      if ( out != NULL )
        *out = it;
      return true;
    }
  }
  return false;
}

//--------------------------------------------------------------------------
graph_info_t *graph_info_t::find(const ea_t func_ea)
{
  iterator it;
  if ( !find(func_ea, &it) )
    return NULL;
  return *it;
}

//--------------------------------------------------------------------------
graph_info_t *graph_info_t::find(const char *title)
{
  iterator it, end = instances.end();
  for ( it = instances.begin(); it != end; ++it )
  {
    graph_info_t *gi = *it;
    if ( strcmp(gi->title.c_str(), title) == 0 )
      return gi;
  }
  return NULL;
}

//--------------------------------------------------------------------------
graph_info_t * graph_info_t::create(DecMap *decMap, ea_t func_ea)
{
  graph_info_t *r = find(func_ea);
  // not there? create it
  if ( r == NULL )
  {
    // we need a function!
    func_t *pfn = get_func(func_ea);
    if ( pfn == NULL )
      return NULL;

    r = new graph_info_t (decMap);
    get_title(func_ea, &r->title);
    r->func_ea = pfn->startEA;
    instances.push_back(r);
  }
  
  r->decMap = decMap;
  return r;
}

//--------------------------------------------------------------------------
void graph_info_t::destroy(graph_info_t *gi)
{
  iterator it;
  if ( !find(gi->func_ea, &it) )
    return;
  delete gi;
  instances.erase(it);
}

//--------------------------------------------------------------------------
bool graph_info_t::get_title(ea_t func_ea, qstring *out)
{
  // we should succeed in getting the name
  char func_name[MAXSTR];
  if ( get_func_name(func_ea, func_name, sizeof(func_name)) == NULL )
    return false;
  out->sprnt("Call graph of: %s", func_name);
  return true;
}

//--------------------------------------------------------------------------
void graph_info_t::mark_for_refresh()
{
  refresh_needed = true;
}

//--------------------------------------------------------------------------
void graph_info_t::mark_as_refreshed()
{
  refresh_needed = false;
}

//--------------------------------------------------------------------------
void graph_info_t::refresh()
{
  mark_for_refresh();
  refresh_viewer(gv);
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
static funcs_walk_options_t fg_opts =
{
  FWO_VERSION,     // version
  FWO_RECURSE_UNLIM, // flags
  0                // max recursion
};

#define DECLARE_GI_VAR \
  graph_info_t *gi = (graph_info_t *) ud

#define DECLARE_GI_VARS \
  DECLARE_GI_VAR;       \
  callgraph_t *fg = &gi->fg

//--------------------------------------------------------------------------
static int idaapi gr_callback(void *ud, int code, va_list va)
{
  bool result = false;
  switch ( code )
  {
    // a graph node has been double clicked
  case grcode_dblclicked:
    // in:  graph_viewer_t *gv
    //      selection_item_t *current_item
    // out: 0-ok, 1-ignore click
    {
      DECLARE_GI_VARS;
      va_arg(va, graph_viewer_t *);
      selection_item_t *s = va_arg(va, selection_item_t *);
      if ( s != NULL && s->is_node )
        jumpto(fg->get_addr(s->node));
    }
    break;

    // refresh user-defined graph nodes and edges
  case grcode_user_refresh:
    // in:  mutable_graph_t *g
    // out: success
    {
      DECLARE_GI_VARS;
      if ( !gi->is_refresh_needed() )
        break;

      gi->mark_as_refreshed();
      fg->reset();
      func_t *f = get_func(gi->func_ea);
      if (f == NULL)
        break;

      fg->walk_func(f, &fg_opts, 2);

      mutable_graph_t *mg = va_arg(va, mutable_graph_t *);

      // we have to resize
      mg->reset();
      mg->resize(fg->count());

      callgraph_t::edge_iterator end = fg->end_edges();
      for ( callgraph_t::edge_iterator it=fg->begin_edges();
        it != end;
        ++it )
      {
        mg->add_edge(it->id1, it->id2, NULL);
      }
      fg->clear_edges();
      result = true;
    }
    break;

    // retrieve text for user-defined graph node
  case grcode_user_text:
    // in:  mutable_graph_t *g
    //      int node
    //      const char **result
    //      bgcolor_t *bg_color (maybe NULL)
    // out: must return 0, result must be filled
    // NB: do not use anything calling GDI!
    {
      DECLARE_GI_VARS;
      va_arg(va, mutable_graph_t *);
      int node           = va_arg(va, int);
      const char **text  = va_arg(va, const char **);
      bgcolor_t *bgcolor = va_arg(va, bgcolor_t *);

      callgraph_t::funcinfo_t *fi = fg->get_info(node);
      result = fi != NULL;
      if ( result )
      {
        *text = fi->name.c_str();
        if ( bgcolor != NULL )
          *bgcolor = fi->color;
      }
    }
    break;

    // retrieve hint for the user-defined graph
  case grcode_user_hint:
    // in:  mutable_graph_t *g
    //      int mousenode
    //      int mouseedge_src
    //      int mouseedge_dst
    //      char **hint
    // 'hint' must be allocated by qalloc() or qstrdup()
    // out: 0-use default hint, 1-use proposed hint
    {
      DECLARE_GI_VARS;
      va_arg(va, mutable_graph_t *);
      int mousenode = va_argi(va, int);
      va_argi(va, int);
      va_argi(va, int);
      char **hint = va_arg(va, char **);
      ea_t addr;
      if ( mousenode != -1 && (addr = fg->get_addr(mousenode)) != BADADDR )
      {
        char *lines[50];
        int nl = generate_disassembly(addr, lines, qnumber(lines), NULL, false);
        qstring all_lines;
        for ( int i = 0; i < nl; i++)
        {
          if ( i != 0 )
            all_lines += "\n";
          all_lines += lines[i];
          qfree(lines[i]);
        }
        *hint = all_lines.extract();
      }
      result = true; // use our hint
    }
    break;

    // graph is being destroyed
  case grcode_destroyed:
    {
      DECLARE_GI_VAR;
      graph_info_t::destroy(gi);
    }
    break;
  }
  return (int)result;
}

//--------------------------------------------------------------------------
static const char *NODE_NAME = "$ callgraph sample";

static bool load_options()
{
  funcs_walk_options_t opt;
  netnode n(NODE_NAME);
  if ( !exist(n) )
    return false;

  n.supval(1, &opt, sizeof(opt));

  if ( opt.version != FWO_VERSION )
    return false;

  fg_opts = opt;
  return true;
}

//--------------------------------------------------------------------------
static void jump_to_node(graph_info_t *gi, const int nid)
{
  viewer_center_on(gi->gv, nid);
  int x, y;

  // will return a place only when a node was previously selected
  place_t *old_pl = get_custom_viewer_place(gi->gv, false, &x, &y);
  if ( old_pl != NULL )
  {
#ifdef __BORLANDC__
    user_graph_place_t *new_pl = (user_graph_place_t *) old_pl->clone();
#else
    // although this works, it may not work in the future
    user_graph_place_t *new_pl = (user_graph_place_t *) qalloc(sizeof(user_graph_place_t));
    memcpy(new_pl, old_pl, sizeof(user_graph_place_t));
#endif
    new_pl->node = nid;
    jumpto(gi->gv, new_pl, x, y);
#ifdef __BORLANDC__
    delete new_pl;
#else
    qfree(new_pl);
#endif
  }
}

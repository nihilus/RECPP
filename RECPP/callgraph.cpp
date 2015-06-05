#include "CallGraph.h"

CallGraph::CallGraph () {
    this->node_count = 0;
    this->cur_text[0] = '\0';
}

bool
CallGraph::visited (
    ea_t func_ea, 
    int *nid
) {
    ea_int_map_t::const_iterator it = this->ea2node.find (func_ea);
    
    if (it != this->ea2node.end ()) {
        if (nid != NULL) {
            *nid = it->second;
        }
        return true;
    }
    
    return false;
}


int
CallGraph::walk_func (
    func_t *func, 
    funcs_walk_options_t *opt, 
    int level
) {
    // add a node for this function
    int id = add (func->startEA);

    func_item_iterator_t fii;

    for (bool fi_ok = fii.set (func); fi_ok; fi_ok = fii.next_code ()) 
    {
        xrefblk_t xb;

        for (bool xb_ok = xb.first_from (fii.current (), XREF_FAR);
            xb_ok && xb.iscode;
            xb_ok = xb.next_from ()
        ) {
            int id2;
            if (!visited (xb.to, &id2)) 
            {
                func_t *f = get_func (xb.to);
                
                if (f == NULL || func_contains (func, xb.to)) {
                    continue;
                }

                bool skip = false;

                if (opt != NULL) {
                    // skip lib funcs?
                    skip = (  ((f->flags & FUNC_LIB) != 0) 
                           && ((opt->flags & FWO_SKIPLIB) != 0))
                         || ( ((opt->flags & FWO_RECURSE_UNLIM) == 0)
                           && (level > opt->recurse_limit));
                }

                id2 = (skip) ? add (f->startEA) 
                             : walk_func (f, opt, level + 1);
            }

            create_edge (id, id2);
        }
    }

    return id;
}

int
CallGraph::find_first (
    const char *text
) {
    if (text == NULL || text[0] == '\0') {
        return -1;
    }

    qstrncpy (this->cur_text, text, sizeof (this->cur_text));
    this->cur_node = 0;

    return find_next ();
}

int
CallGraph::find_next (
    void
) {
    for (int i = this->cur_node; i < node_count; i++) {
        const char *s = get_name (i);
        if (stristr (s, this->cur_text) != NULL) {
            this->cur_node = i + 1;
            return i;
        }
    }

    // reset search
    this->cur_node = 0;

    // nothing is found
    return -1;
}


inline void
CallGraph::create_edge (
    int id1, int id2
) {
    edges.push_back (edge_t (id1, id2));
}

void
CallGraph::reset (
    void
) {
    this->node_count = 0;
    this->cur_node = 0;
    this->cur_text[0] = '\0';
    this->ea2node.clear ();
    this->node2ea.clear ();
    this->cached_funcs.clear ();
    this->edges.clear ();
}

const ea_t
CallGraph::get_addr (
    int nid
) {
    int_ea_map_t::const_iterator it = this->node2ea.find (nid);
    return it == this->node2ea.end () ? BADADDR : it->second;
}

CallGraph::funcinfo_t *
CallGraph::get_info (
    int nid
) {
    funcinfo_t *ret = NULL;

    do {
        // returned cached name
        int_funcinfo_map_t::iterator it = cached_funcs.find (nid);
        if (it != cached_funcs.end ()) {
            ret = &it->second;
            break;
        }

        // node does not exist?
        int_ea_map_t::const_iterator it_ea = this->node2ea.find (nid);
        if (it_ea == this->node2ea.end ()) {
            break;
        }

        func_t *pfn = get_func (it_ea->second);
        if (pfn == NULL) {
            break;
        }

        funcinfo_t fi;

        // get name
        char buf [MAXSTR];
        if (get_func_name (it_ea->second, buf, sizeof (buf)) == NULL) {
            fi.name = "?";
        } 
        else {
            fi.name = buf;
        }
        // get color
        fi.color = calc_bg_color (pfn->startEA);

        fi.ea = pfn->startEA;

        // Get function pointers
        fi.func = pfn;

        it = cached_funcs.insert (cached_funcs.end (), std::make_pair (nid, fi));
        ret = &it->second;
    } while (false);

    return ret;
}


const char *CallGraph::get_name (int nid) {
    funcinfo_t *fi = get_info (nid);
    if (fi == NULL)
      return "?";
    else
      return fi->name.c_str ();
}


func_t *CallGraph::get_function (int nid) {
    funcinfo_t *fi = get_info (nid);

    if (fi == NULL) {
        return NULL;
    }

    return fi->func;
}


int CallGraph::add (ea_t func_ea) {
    ea_int_map_t::const_iterator it = this->ea2node.find (func_ea);
    if (it != this->ea2node.end ())
      return it->second;

    this->ea2node[func_ea] = node_count;
    this->node2ea[node_count] = func_ea;
    return node_count++;
}



void CallGraph::clear_edges () {
    edges.clear ();
}

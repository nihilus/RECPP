/*
    ██████╗ ███████╗ ██████╗██████╗ ██████╗ 
    ██╔══██╗██╔════╝██╔════╝██╔══██╗██╔══██╗
    ██████╔╝█████╗  ██║     ██████╔╝██████╔╝
    ██╔══██╗██╔══╝  ██║     ██╔═══╝ ██╔═══╝ 
    ██║  ██║███████╗╚██████╗██║     ██║     
    ╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝     ╚═╝     
* @license : <license placeholder>
*/

#include "RECPP.h"
#include "VtableScanner.h"
#include "DecMap.h"

// Hex-Rays API pointer
hexdsp_t *hexdsp = NULL;
static bool inited = false;


// UI callbacks

static bool idaapi 
user_menu_scan_vftable (
    void *ud
) {
    DecMap *decompilationMap = (DecMap *) ud;
    VtableScanner *vScanner = new VtableScanner (decompilationMap);
    
    if (!(vScanner->scan ())) {
        msg ("Cannot scan the virtual function tables.");
        return false;
    }

    return true;
}

// Callbacks

static int idaapi 
ui_callback (
    void *ud, 
    int notification_code, 
    va_list va
) {
    TCustomControl *view = va_arg (va, TCustomControl *);

    switch (notification_code)
    {
        case view_popup:
            add_custom_viewer_popup_item (view, "Scan and rename vftables", "", user_menu_scan_vftable, ud);
        break;
    }

    return 0;
}

static int idaapi hx_callback (void *ud, hexrays_event_t event, va_list va)
{
    if (event == hxe_maturity)
    {
        cfunc_t *cfunc = va_arg(va, cfunc_t*);
        ctree_maturity_t mat = va_argi(va, ctree_maturity_t);
        if (mat == CMAT_FINAL) {
            DecMap *decompilationMap = (DecMap *) ud;
            msg ("Function %x decompiled, start processing...\n", cfunc->entry_ea);
            decompilationMap->process (cfunc);
        }
    }

    return 0;
}

/*
 * @brief Initialize the RECPP plugin 
 */
int idaapi 
init (
    void
) {
    if (!init_hexrays_plugin ()) {
	    return PLUGIN_SKIP;
    }
    
    msg (" |||||.  |||||||` ||||||` |||||.  |||||.  \n"
         " ||+--|| ||+----`||+----` ||+--|| ||+--|| \n"
         " ||||||+ |||||`  |||      ||||||' ||||||' \n"
         " ||+--|| ||+--`  |||      ||+---` ||+---` \n"
         " |||  || |||||||`+||||||` |||     |||     \n"
         " +-`  +-` ------` +-----` +-`     +-`     \n");

    DecMap *decompilationMap = new DecMap ();
    hook_to_notification_point (HT_VIEW, ui_callback, decompilationMap); 
    install_hexrays_callback (hx_callback, decompilationMap);
    inited = true;

    return PLUGIN_KEEP;
}

/*
 * @brief Terminate the RECPP plugin 
 */
void idaapi 
term (
    void
) {
    if (inited) {
        // remove_hexrays_callback (callback, NULL);
        term_hexrays_plugin ();
    }
}

/*
 * @brief Run the RECPP plugin 
 */
void idaapi 
run (
    int __unused
) {
}


/*
 * Register the RECPP plugin
 */
plugin_t PLUGIN = {
	IDP_INTERFACE_VERSION,
    PLUGIN_HIDE,          // plugin flags
    init,                 // initialize
    term,                 // terminate. this pointer may be NULL.
    run,                  // invoke plugin
    "RECPP IDA Plugin",   // long comment about the plugin
                          // it could appear in the status line or as a hint
    "",                   // multiline help about the plugin
    "RECPP",              // the preferred short name of the plugin
    "Alt-D"               // the preferred hotkey to run the plugin
};
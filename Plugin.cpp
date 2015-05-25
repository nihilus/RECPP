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
#include "vtable.h"

// Hex-Rays API pointer
hexdsp_t *hexdsp = NULL;
static bool inited = false;

static bool idaapi 
user_menu_scan_vftable (
    void *__unused
) {
    // Get .text and .rdata segments boundaries
    segment_t *textSeg  = get_segm_by_name (".text");
    segment_t *rdataSeg = get_segm_by_name (".rdata");

    if (!textSeg || !rdataSeg) {
        msg ("Error : Cannot find the .text or .rdata segment.");
        return false;
    }

    ea_t rMin = rdataSeg->startEA, 
         rMax = rdataSeg->endEA, 
         cMin = textSeg->startEA, 
         cMax = textSeg->endEA;
    
    if (rMin == 0) {
        rMin = cMin; 
        rMax = cMax;
    }

    ea_t curAddress = rMin;
    ea_t x;
    
    while (curAddress < rMax) {
        x = get_long (curAddress);

        // Methods should reside in .text
        if (x >= cMin && x < cMax) {
            curAddress = do_vtable (curAddress);
        }
        else {
            curAddress += 4;
        }
    }

    msg ("Finished !\n");
    msg ("Vtable count = %d", vtableCount);

    return true;
}

static int idaapi 
ui_callback (
    void * __unused, 
    int notification_code, 
    va_list va
) {
    TCustomControl *view = va_arg (va, TCustomControl *);

    switch (notification_code)
    {
        case view_popup:
            // clear the current user-defined menu (may clear items added by other plugins, so not recommended)
            set_custom_viewer_popup_menu(view, NULL);

            add_custom_viewer_popup_item(view, "Scan vftables", "Ctrl-5", user_menu_scan_vftable, NULL);
        break;
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

    msg ("=====================[ RE CPP ]=====================\n");
    hook_to_notification_point (HT_VIEW, ui_callback, NULL); 
    // install_hexrays_callback (callback, NULL);
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
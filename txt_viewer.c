#include <furi.h>
#include <gui/gui.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_box.h>
#include <stdlib.h>
#include <string.h>

// Enumeration for different views
typedef enum {
    TxtViewerMainMenu,
    TxtViewerBrowse,
    TxtViewerRecents,
    TxtViewerFavorites,
    TxtViewerInfo
} TxtViewerView;

// Application state structure
typedef struct {
    ViewDispatcher* view_dispatcher;
    Submenu* menu;
    Submenu* list_view;
    TextBox* text_box;
    Storage* storage;
    DialogsApp* dialogs;
} TxtViewerApp;

// Callback for main menu selections
static void txt_viewer_menu_callback(void* ctx, uint32_t index) {
    TxtViewerApp* app = ctx;
    switch(index) {
        case 0: { // Browse
            submenu_reset(app->list_view);
            StorageDir* dir = storage_dir_alloc();
            if(storage_dir_open(app->storage, dir, "/ext/txt_viewer")) {
                FuriString* fname = furi_string_alloc();
                while(storage_dir_read(dir, fname)) {
                    const char* name = furi_string_get_cstr(fname);
                    if(strstr(name, ".txt") != NULL) {
                        submenu_add_item(app->list_view, name, 0, NULL, NULL);
                    }
                }
                furi_string_free(fname);
            }
            storage_dir_close(dir);
            storage_dir_free(dir);
            submenu_set_header(app->list_view, "Browse Files");
            view_dispatcher_switch_to_view(app->view_dispatcher, TxtViewerBrowse);
            break;
        }
        case 1: { // Recents
            submenu_reset(app->list_view);
            submenu_add_item(app->list_view, "No recent files", 0, NULL, NULL);
            submenu_set_header(app->list_view, "Recent Files");
            view_dispatcher_switch_to_view(app->view_dispatcher, TxtViewerRecents);
            break;
        }
        case 2: { // Favorites
            submenu_reset(app->list_view);
            submenu_add_item(app->list_view, "No favorites", 0, NULL, NULL);
            submenu_set_header(app->list_view, "Favorite Files");
            view_dispatcher_switch_to_view(app->view_dispatcher, TxtViewerFavorites);
            break;
        }
        case 3: { // Info
            text_box_reset(app->text_box);
            text_box_set_text(app->text_box, "TXT Viewer\n\nThis app allows you to view prayers stored in .txt files.\nAdd files to /ext/txt_viewer using QFlipper.\n\nCreated as a simple way to view prayers.");
            view_dispatcher_switch_to_view(app->view_dispatcher, TxtViewerInfo);
            break;
        }
        default:
            break;
    }
}

int32_t txt_viewer_app(void* p) {
    UNUSED(p);
    // Allocate application state
    TxtViewerApp* app = malloc(sizeof(TxtViewerApp));
    app->storage = furi_record_open("storage");
    app->dialogs = furi_record_open("dialogs");
    // Ensure directory exists
    storage_common_mkdir(app->storage, "/ext/txt_viewer");
    // Create components
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    app->menu = submenu_alloc();
    // Add menu items
    submenu_add_item(app->menu, "Browse", 0, txt_viewer_menu_callback, app);
    submenu_add_item(app->menu, "Recents", 1, txt_viewer_menu_callback, app);
    submenu_add_item(app->menu, "Favorites", 2, txt_viewer_menu_callback, app);
    submenu_add_item(app->menu, "Info", 3, txt_viewer_menu_callback, app);
    // List view for Browse/Recents/Favorites
    app->list_view = submenu_alloc();
    // Text box for Info
    app->text_box = text_box_alloc();
    text_box_set_scrollbar_enabled(app->text_box, true);
    // Register views
    view_dispatcher_add_view(app->view_dispatcher, TxtViewerMainMenu, submenu_get_view(app->menu));
    view_dispatcher_add_view(app->view_dispatcher, TxtViewerBrowse, submenu_get_view(app->list_view));
    view_dispatcher_add_view(app->view_dispatcher, TxtViewerRecents, submenu_get_view(app->list_view));
    view_dispatcher_add_view(app->view_dispatcher, TxtViewerFavorites, submenu_get_view(app->list_view));
    view_dispatcher_add_view(app->view_dispatcher, TxtViewerInfo, text_box_get_view(app->text_box));
    // Show main menu
    view_dispatcher_switch_to_view(app->view_dispatcher, TxtViewerMainMenu);
    // Run dispatcher
    view_dispatcher_run(app->view_dispatcher);
    // Cleanup
    view_dispatcher_remove_view(app->view_dispatcher, TxtViewerInfo);
    text_box_free(app->text_box);
    submenu_free(app->list_view);
    submenu_free(app->menu);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close("storage");
    furi_record_close("dialogs");
    free(app);
    return 0;
}

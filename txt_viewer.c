#include <furi.h>
#include <gui/gui.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/text_box.h>
#include <stdlib.h>
#include <string.h>

// Enumeration for views
typedef enum {
    TxtViewerMainMenu,
    TxtViewerList,
    TxtViewerContent,
    TxtViewerInfo,
} TxtViewerView;

// Application state structure
typedef struct {
    ViewDispatcher* view_dispatcher;
    Submenu* main_menu;
    Submenu* list_view;
    TextBox* text_box;
    Storage* storage;
    DialogsApp* dialogs;
    FuriString** file_names;
    size_t file_count;
    FuriString** recents;
    size_t recents_count;
    FuriString** favorites;
    size_t favorites_count;
} TxtViewerApp;

static void txt_viewer_list_callback(void* ctx, uint32_t index);
static void txt_viewer_main_menu_callback(void* ctx, uint32_t index);

// Load directory listing into list view and store file names
static void load_directory(TxtViewerApp* app, const char* dir_path) {
    // Free previous file names
    for(size_t i = 0; i < app->file_count; i++) {
        furi_string_free(app->file_names[i]);
    }
    free(app->file_names);
    app->file_names = NULL;
    app->file_count = 0;
    submenu_reset(app->list_view);
    
    // Allocate directory handle
    File* dir = storage_file_alloc(app->storage);
    if(storage_dir_open(dir, dir_path)) {
        FileInfo file_info;
        char name[128];
        while(storage_dir_read(dir, &file_info, name, sizeof(name))) {
            if(file_info.type == FileTypeFile) {
                // store file name
                FuriString* fname = furi_string_alloc();
                furi_string_set(fname, name);
                app->file_names = realloc(app->file_names, (app->file_count + 1) * sizeof(FuriString*));
                app->file_names[app->file_count] = fname;
                // add to list view
                submenu_add_item(app->list_view, name, app->file_count, txt_viewer_list_callback, app);
                app->file_count++;
            }
        }
        storage_dir_close(dir);
    }
    storage_file_free(dir);
}

// Add file name to recents list if not already present
static void txt_viewer_add_recent(TxtViewerApp* app, const char* name) {
    for(size_t i = 0; i < app->recents_count; i++) {
        if(strcmp(furi_string_get_cstr(app->recents[i]), name) == 0) {
            return;
        }
    }
    FuriString* fname = furi_string_alloc();
    furi_string_set(fname, name);
    app->recents = realloc(app->recents, (app->recents_count + 1) * sizeof(FuriString*));
    app->recents[app->recents_count] = fname;
    app->recents_count++;
}

// Callback when a file is selected from list
static void txt_viewer_list_callback(void* ctx, uint32_t index) {
    TxtViewerApp* app = ctx;
    if(index >= app->file_count) return;
    const char* name = furi_string_get_cstr(app->file_names[index]);
    char path[256];
    snprintf(path, sizeof(path), "/ext/txt_viewer/%s", name);
    File* file = storage_file_alloc(app->storage);
    if(storage_file_open(file, path, FS_READ, FS_OPEN_EXISTING)) {
        size_t total_size = 0;
        char* content = NULL;
        char buf[64];
        size_t read;
        while((read = storage_file_read(file, buf, sizeof(buf))) > 0) {
            content = realloc(content, total_size + read + 1);
            memcpy(content + total_size, buf, read);
            total_size += read;
        }
        if(content) {
            content[total_size] = '\0';
            text_box_reset(app->text_box);
            text_box_set_text(app->text_box, content, false);
            free(content);
        }
        storage_file_close(file);
    }
    storage_file_free(file);
    txt_viewer_add_recent(app, name);
    view_dispatcher_switch_to_view(app->view_dispatcher, TxtViewerContent);
}

// Callback for main menu selections
static void txt_viewer_main_menu_callback(void* ctx, uint32_t index) {
    TxtViewerApp* app = ctx;
    switch(index) {
        case 0: // Browse
            load_directory(app, "/ext/txt_viewer");
            view_dispatcher_switch_to_view(app->view_dispatcher, TxtViewerList);
            break;
        case 1: // Recents
            submenu_reset(app->list_view);
            for(size_t i = 0; i < app->recents_count; i++) {
                const char* name = furi_string_get_cstr(app->recents[i]);
                submenu_add_item(app->list_view, name, i, txt_viewer_list_callback, app);
            }
            view_dispatcher_switch_to_view(app->view_dispatcher, TxtViewerList);
            break;
        case 2: // Favorites
            submenu_reset(app->list_view);
            for(size_t i = 0; i < app->favorites_count; i++) {
                const char* name = furi_string_get_cstr(app->favorites[i]);
                submenu_add_item(app->list_view, name, i, txt_viewer_list_callback, app);
            }
            view_dispatcher_switch_to_view(app->view_dispatcher, TxtViewerList);
            break;
        case 3: // Info
        default:
            text_box_reset(app->text_box);
            text_box_set_text(app->text_box, "TXT Viewer\n\nThis app allows you to view prayers stored in .txt files.\nAdd files to /ext/txt_viewer using QFlipper.\nUse 'Browse' to open files, and mark favorites by pressing OK.", false);
            view_dispatcher_switch_to_view(app->view_dispatcher, TxtViewerInfo);
            break;
    }
}

// Application entry point
int32_t txt_viewer_app(void* p) {
    UNUSED(p);
    TxtViewerApp* app = malloc(sizeof(TxtViewerApp));
    memset(app, 0, sizeof(TxtViewerApp));

    app->storage = furi_record_open("storage");
    app->dialogs = furi_record_open("dialogs");
    storage_common_mkdir(app->storage, "/ext/txt_viewer");

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    app->main_menu = submenu_alloc();
    app->list_view = submenu_alloc();
    app->text_box = text_box_alloc();

    submenu_add_item(app->main_menu, "Browse", 0, txt_viewer_main_menu_callback, app);
    submenu_add_item(app->main_menu, "Recents", 1, txt_viewer_main_menu_callback, app);
    submenu_add_item(app->main_menu, "Favorites", 2, txt_viewer_main_menu_callback, app);
    submenu_add_item(app->main_menu, "Info", 3, txt_viewer_main_menu_callback, app);

    view_dispatcher_add_view(app->view_dispatcher, TxtViewerMainMenu, submenu_get_view(app->main_menu));
    view_dispatcher_add_view(app->view_dispatcher, TxtViewerList, submenu_get_view(app->list_view));
    view_dispatcher_add_view(app->view_dispatcher, TxtViewerContent, text_box_get_view(app->text_box));
    view_dispatcher_add_view(app->view_dispatcher, TxtViewerInfo, text_box_get_view(app->text_box));

    view_dispatcher_switch_to_view(app->view_dispatcher, TxtViewerMainMenu);
    view_dispatcher_run(app->view_dispatcher);

    // Free file names arrays
    for(size_t i = 0; i < app->file_count; i++) {
        furi_string_free(app->file_names[i]);
    }
    free(app->file_names);
    for(size_t i = 0; i < app->recents_count; i++) {
        furi_string_free(app->recents[i]);
    }
    free(app->recents);
    for(size_t i = 0; i < app->favorites_count; i++) {
        furi_string_free(app->favorites[i]);
    }
    free(app->favorites);

    text_box_free(app->text_box);
    submenu_free(app->list_view);
    submenu_free(app->main_menu);
    view_dispatcher_free(app->view_dispatcher);
    furi_record_close("storage");
    furi_record_close("dialogs");
    free(app);
    return 0;
}

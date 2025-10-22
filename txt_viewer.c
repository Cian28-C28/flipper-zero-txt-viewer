#include <furi.h>
#include <gui/gui.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

int32_t txt_viewer_app(void* p) {
    UNUSED(p);

    // Open required records
    Gui* gui = furi_record_open("gui");
    DialogsApp* dialogs = furi_record_open("dialogs");
    Storage* storage = furi_record_open("storage");

    // Create directory for text viewer files
    storage_common_mkdir(storage, "/ext/txt_viewer");

    // Show informational message to user
    dialogs_message_show(dialogs, "TXT Viewer", "Folder /ext/txt_viewer created.\nAdd text files there using QFlipper.", NULL, false);

    // Close records
    furi_record_close("storage");
    furi_record_close("dialogs");
    furi_record_close("gui");

    return 0;
}

#include <furi.h>
#include <gui/gui.h>
#include <dialogs/dialogs.h>
#include <storage/storage.h>

int32_t txt_viewer_app(void* p) {
    UNUSED(p);
    // Open required records
    DialogsApp* dialogs = furi_record_open("dialogs");
    Storage* storage = furi_record_open("storage");

    // Create directory for text viewer files
    storage_common_mkdir(storage, "/ext/txt_viewer");

    // Allocate dialog message
    DialogMessage* message = dialog_message_alloc();
    // Set header and text
    dialog_message_set_header(message, "TXT Viewer", 0, 0, AlignCenter, AlignTop);
    dialog_message_set_text(message, "Folder /ext/txt_viewer created. Add text files there using QFlipper.", 0, 12, AlignLeft, AlignTop);
    // Show message
    dialog_message_show(dialogs, message);
    // Free message
    dialog_message_free(message);

    // Close records
    furi_record_close("storage");
    furi_record_close("dialogs");
    return 0;
}

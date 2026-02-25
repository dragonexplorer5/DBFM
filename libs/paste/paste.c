#include "paste.h"
#include <stdio.h>
#include "../clipboard/clipboard.h"
#include "../copy/copy.h"
#include "../move/move.h"

int paste_item(const char* dest_dir)
{
    if (dest_dir == NULL) return -1;

    if (!clipboard_has_item()) return -1;

    const char* path = clipboard_get_path();
    if (path == NULL) return -1;

    ClipboardOp op = clipboard_get_operation();
    int rc = -1;
    if (op == CLIPBOARD_COPY) {
        rc = copy_item(path, dest_dir);
    } else if (op == CLIPBOARD_MOVE) {
        rc = move_file(path, dest_dir);
    }

    if (rc == 0) {
        /* Clear clipboard after successful paste for MOVE; keep for COPY */
        if (op == CLIPBOARD_MOVE) clipboard_clear();
        return 0;
    }

    return -1;
}

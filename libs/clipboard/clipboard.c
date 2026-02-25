#include "clipboard.h"
#include <string.h>

static char g_clipboard_path[512];
static ClipboardOp g_clipboard_op = CLIPBOARD_NONE;

void clipboard_init(void)
{
    g_clipboard_path[0] = '\0';
    g_clipboard_op = CLIPBOARD_NONE;
}

int clipboard_set(const char* path, ClipboardOp op)
{
    if (path == NULL) return -1;
    strncpy(g_clipboard_path, path, sizeof(g_clipboard_path)-1);
    g_clipboard_path[sizeof(g_clipboard_path)-1] = '\0';
    g_clipboard_op = op;
    return 0;
}

const char* clipboard_get_path(void)
{
    if (g_clipboard_op == CLIPBOARD_NONE) return NULL;
    return g_clipboard_path;
}

ClipboardOp clipboard_get_operation(void)
{
    return g_clipboard_op;
}

void clipboard_clear(void)
{
    g_clipboard_path[0] = '\0';
    g_clipboard_op = CLIPBOARD_NONE;
}

int clipboard_has_item(void)
{
    return (g_clipboard_op != CLIPBOARD_NONE && g_clipboard_path[0] != '\0');
}

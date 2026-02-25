#ifndef CLIPBOARD_H
#define CLIPBOARD_H

/** Clipboard operation types */
typedef enum {
    CLIPBOARD_NONE = 0,
    CLIPBOARD_COPY = 1,
    CLIPBOARD_MOVE = 2
} ClipboardOp;

/* Initialize clipboard system */
void clipboard_init(void);

/* Set clipboard item and operation */
int clipboard_set(const char* path, ClipboardOp op);

/* Get clipboard path (returns pointer to internal buffer, NULL if none) */
const char* clipboard_get_path(void);

/* Get clipboard operation (CLIPBOARD_NONE/COPY/MOVE) */
ClipboardOp clipboard_get_operation(void);

/* Clear clipboard */
void clipboard_clear(void);

/* Check if clipboard has an item */
int clipboard_has_item(void);

#endif

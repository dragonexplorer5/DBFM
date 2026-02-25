#ifndef UI_H
#define UI_H

#include "fs.h"

/* popup type constants (match values used internally in ui.c) */
#define POPUP_NONE    0
#define POPUP_MESSAGE 1

/* overlay operation codes (used in dynamic menus) */
#define UI_OP_COPY    0
#define UI_OP_PASTE   1
#define UI_OP_MOVE    2
#define UI_OP_DELETE  3
#define UI_OP_RENAME  4
#define UI_OP_LAUNCH  5
#define UI_OP_INSTALL 6

/**
 * UI Module
 * 
 * Handles rendering of the file browser interface.
 * Manages display of directory listings, selection state, and scrolling.
 * Maximum of 20 entries visible at once (scrolls for larger directories).
 */

/**
 * UIState - Complete state of the UI
 */
typedef struct {
    FsDirectory* current_dir;      // Current directory listing (malloc'd)
    int selected_index;            // Index of selected entry (0-based)
    int scroll_offset;             // First visible entry index for scrolling
    char current_path[512];        // Full path of current directory
    
    // Overlay menu state
    int overlay_active;            // 1 if overlay menu is open
    int overlay_selected;          // Currently selected menu option (index into dynamic list)
    int overlay_count;             // number of items currently in overlay
    int overlay_codes[8];          // operation codes for each slot
    char overlay_labels[8][32];    // label text for each slot

    // Popup notification state
    int popup_active;              // 1 if a popup is currently visible
    int popup_type;                // 0=none,1=message
    char popup_message[256];       // message text for simple popups
    int popup_timer;               // frames remaining before auto-dismiss (for message)
} UIState;

/**
 * ui_init(ui_state)
 * Initialize the UI system and load the starting directory (sdmc:/).
 * Must be called once before rendering or handling input.
 */
void ui_init(UIState* ui_state);

/**
 * ui_render(ui_state)
 * Render current directory listing to screen.
 * Shows selected entry highlighted with inverse video.
 * Should be called once per frame after input updates.
 */
void ui_render(UIState* ui_state);

/**
 * ui_select_next(ui_state)
 * Move selection down to next entry. Auto-scrolls if needed.
 */
void ui_select_next(UIState* ui_state);

/**
 * ui_select_prev(ui_state)
 * Move selection up to previous entry. Auto-scrolls if needed.
 */
void ui_select_prev(UIState* ui_state);

/**
 * ui_get_selected_entry(ui_state)
 * Get pointer to currently selected directory entry.
 * Returns NULL if no valid selection.
 */
FsEntry* ui_get_selected_entry(UIState* ui_state);

/**
 * ui_get_selected_path(ui_state, dest)
 * Get full path of selected entry.
 * dest must be at least 512 bytes.
 */
void ui_get_selected_path(UIState* ui_state, char* dest);

/**
 * ui_enter_directory(ui_state)
 * Change to selected directory if it's a folder.
 * Resets selection to top of new directory.
 * Returns 0 on success, -1 if entry is not a directory or access fails.
 */
int ui_enter_directory(UIState* ui_state);

/**
 * ui_go_back(ui_state)
 * Navigate to parent directory.
 * Resets selection to top of parent directory.
 * Returns 0 on success, -1 if already at root.
 */
int ui_go_back(UIState* ui_state);

/**
 * ui_cleanup(ui_state)
 * Free UI resources. Call before application exit.
 * Safe to call with uninitialized or already-cleaned state.
 */
void ui_cleanup(UIState* ui_state);

/**
 * Overlay Menu Controls
 */

/**
 * ui_open_overlay(ui_state)
 * Open the file operations overlay menu.
 */
void ui_open_overlay(UIState* ui_state);

/**
 * ui_close_overlay(ui_state)
 * Close the file operations overlay menu.
 */
void ui_close_overlay(UIState* ui_state);

/**
 * ui_overlay_select_next(ui_state)
 * Move selection down in overlay menu.
 */
void ui_overlay_select_next(UIState* ui_state);

/**
 * ui_overlay_select_prev(ui_state)
 * Move selection up in overlay menu.
 */
void ui_overlay_select_prev(UIState* ui_state);

/**
 * ui_overlay_get_selected(ui_state)
 * Get currently selected menu option (0=Copy, 1=Paste, 2=Move, 3=Delete, 4=Rename).
 */
int ui_overlay_get_selected(UIState* ui_state);

/* Popup system helpers */

/**
 * ui_show_message(ui_state, msg, duration)
 * Display a transient message popup for 'duration' frames.
 * If duration is 0 the popup will wait for button press.
 */
void ui_show_message(UIState* ui_state, const char* msg, int duration);

/**

/**
 * ui_process_popup_input(ui_state)
 * Should be called each frame when a popup is active. Handles
 * navigation, editing, and dismiss logic. Return codes:
 *   0 = popup was dismissed this frame
 *   1 = popup still active, no special event
 *   2 = rename dialog confirmed (buffer contains new name)
 */
int ui_process_popup_input(UIState* ui_state);

#endif

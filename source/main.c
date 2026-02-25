#include <string.h>
#include <stdio.h>
#include <switch.h>
#include <switch/applets/swkbd.h>

#include "input.h"
#include "fs.h"
#include "ui.h"
#include "text.h"
#include "clipboard.h"
#include "paste.h"
#include "delete.h"
#include "../libs/rename/rename.h"  // rename support
#include "../libs/utils/utils.h"  // path helpers
#include "../libs/launch/launch.h"  // nro launching
#include "../libs/install/install.h"  // package installation

/**
 * Nintendo Switch File Browser - Main Entry Point
 * 
 * A modular file browser application for the Nintendo Switch.
 * Uses modules:
 *  - input.c/h: Controller input handling
 *  - fs.c/h: Filesystem operations (directory listing)
 *  - ui.c/h: User interface and rendering
 *  - libs/text/text.c/h: Text rendering library
 *  - libs/utils/utils.c/h: Utility functions
 */

int main(int argc, char **argv)
{
    // Initialize all subsystems
    text_init();
    input_init();
    fs_init();
    clipboard_init();

    // Initialize UI with starting state
    UIState ui_state;
    ui_init(&ui_state);

    // Check if we could read the root directory
    if (ui_state.current_dir == NULL) {
        text_clear();
        text_draw(0, 5, "Error: Cannot read filesystem");
        text_draw(0, 8, "Close this app to continue");
        text_update();
        while(appletMainLoop()) {
            // Wait for user to close app
        }
        ui_cleanup(&ui_state);
        fs_cleanup();
        text_exit();
        return 1;
    }

    // Initial render
    ui_render(&ui_state);

    // Main application loop
    while(appletMainLoop())
    {
        // Check for power button to exit
        if (input_power_pressed()) {
            break;
        }

        // Update input state
        input_update();

        // If a popup is visible, let it consume input first
        if (ui_state.popup_active) {
            int code = ui_process_popup_input(&ui_state);
            (void)code;  // just dismiss on input
            // always render and skip other input handling
            ui_render(&ui_state);
            continue;
        }

        if (ui_state.overlay_active) {
            // Handle overlay menu input
            if (input_down()) {
                ui_overlay_select_next(&ui_state);
            }

            if (input_up()) {
                ui_overlay_select_prev(&ui_state);
            }

            // Handle overlay selection (confirm operation)
            if (input_select()) {
                int idx = ui_overlay_get_selected(&ui_state);
                int selected_op = (idx >= 0 && idx < ui_state.overlay_count) ?
                                  ui_state.overlay_codes[idx] : -1;
                FsEntry* sel_entry = ui_get_selected_entry(&ui_state);
                
                if (sel_entry != NULL && selected_op != -1) {
                    char selected_path[512];
                    ui_get_selected_path(&ui_state, selected_path);

                    switch (selected_op) {
                        case UI_OP_COPY:  // Copy -> set clipboard
                            clipboard_set(selected_path, CLIPBOARD_COPY);
                            {
                                char msg[256];
                                snprintf(msg, sizeof(msg), "Copied: %s", sel_entry->name);
                                ui_show_message(&ui_state, msg, 120);
                            }
                            break;
                        case UI_OP_PASTE:  // Paste into selected directory
                            if (sel_entry != NULL && sel_entry->is_dir) {
                                if (paste_item(selected_path) == 0) {
                                    // determine which operation occurred
                                    const char* clipPath = clipboard_get_path();
                                    ClipboardOp op = clipboard_get_operation();
                                    const char* name = path_get_filename(clipPath);
                                    if (op == CLIPBOARD_COPY) {
                                        char msg[256];
                                        snprintf(msg, sizeof(msg), "Pasted: %s", name);
                                        ui_show_message(&ui_state, msg, 120);
                                    } else if (op == CLIPBOARD_MOVE) {
                                        char msg[256];
                                        snprintf(msg, sizeof(msg), "Moved: %s", name);
                                        ui_show_message(&ui_state, msg, 120);
                                    }
                                } else {
                                    ui_show_message(&ui_state, "Paste failed", 120);
                                }
                                /* reload current directory */
                                FsDirectory* new_dir = fs_list_directory(ui_state.current_path);
                                if (new_dir != NULL) {
                                    if (ui_state.current_dir != NULL)
                                        fs_free_directory(ui_state.current_dir);
                                    ui_state.current_dir = new_dir;
                                }
                            }
                            break;
                        case UI_OP_MOVE:  // Move -> set clipboard to move
                            clipboard_set(selected_path, CLIPBOARD_MOVE);
                            {
                                char msg[256];
                                snprintf(msg, sizeof(msg), "Marked to move: %s", sel_entry->name);
                                ui_show_message(&ui_state, msg, 120);
                            }
                            break;
                        case UI_OP_DELETE:  // Delete
                            if (delete_item(selected_path) == 0) {
                                char msg[256];
                                snprintf(msg, sizeof(msg), "Deleted: %s", sel_entry->name);
                                ui_show_message(&ui_state, msg, 120);
                            } else {
                                ui_show_message(&ui_state, "Delete failed", 120);
                            }
                            /* reload current directory */
                            {
                                FsDirectory* new_dir = fs_list_directory(ui_state.current_path);
                                if (new_dir != NULL) {
                                    if (ui_state.current_dir != NULL)
                                        fs_free_directory(ui_state.current_dir);
                                    ui_state.current_dir = new_dir;
                                    if (ui_state.selected_index >= ui_state.current_dir->count && ui_state.current_dir->count > 0)
                                        ui_state.selected_index = ui_state.current_dir->count - 1;
                                    else if (ui_state.current_dir->count == 0)
                                        ui_state.selected_index = 0;
                                }
                            }
                            break;
                        case UI_OP_RENAME:  // Rename (use software keyboard)
                            {
                                // prepare keyboard
                                SwkbdConfig kbd;
                                char result[256];
                                swkbdCreate(&kbd, 0);
                                swkbdConfigMakePresetDefault(&kbd);
                                swkbdConfigSetInitialText(&kbd, sel_entry->name);
                                swkbdConfigSetGuideText(&kbd, "New name");
                                swkbdConfigSetOkButtonText(&kbd, "Rename");
                                swkbdShow(&kbd, result, sizeof(result));
                                swkbdClose(&kbd);
                                if (result[0] != '\0') {
                                    // perform rename
                                    char oldpath[512];
                                    ui_get_selected_path(&ui_state, oldpath);
                                    if (rename_item(oldpath, result) == 0) {
                                        char msg2[256];
                                        snprintf(msg2, sizeof(msg2), "Renamed to: %s", result);
                                        ui_show_message(&ui_state, msg2, 120);
                                        FsDirectory* new_dir = fs_list_directory(ui_state.current_path);
                                        if (new_dir != NULL) {
                                            if (ui_state.current_dir != NULL)
                                                fs_free_directory(ui_state.current_dir);
                                            ui_state.current_dir = new_dir;
                                        }
                                    } else {
                                        ui_show_message(&ui_state, "Rename failed", 120);
                                    }
                                }
                            }
                            break;
                        case UI_OP_LAUNCH:
                            if (!sel_entry->is_dir) {
                                if (launch_nro(selected_path) == 0) {
                                    char msg3[256];
                                    snprintf(msg3, sizeof(msg3), "Launched: %s", sel_entry->name);
                                    ui_show_message(&ui_state, msg3, 120);
                                } else {
                                    ui_show_message(&ui_state, "Launch failed", 120);
                                }
                            }
                            break;
                        case UI_OP_INSTALL:
                            if (!sel_entry->is_dir) {
                                if (install_package(selected_path) == 0) {
                                    char msg4[256];
                                    snprintf(msg4, sizeof(msg4), "Installed: %s", sel_entry->name);
                                    ui_show_message(&ui_state, msg4, 120);
                                } else {
                                    ui_show_message(&ui_state, "Install failed", 120);
                                }
                            }
                            break;
                    }
                }
                ui_close_overlay(&ui_state);
            }

            // Handle back button to close overlay
            if (input_back()) {
                ui_close_overlay(&ui_state);
            }
        } else {
            // Handle normal directory navigation
            if (input_down()) {
                ui_select_next(&ui_state);
            }

            if (input_up()) {
                ui_select_prev(&ui_state);
            }

            // Handle selection (A button)
            if (input_select()) {
                FsEntry* selected = ui_get_selected_entry(&ui_state);
                if (selected != NULL) {
                    if (selected->is_dir) {
                        // A on folder: enter directory
                        ui_enter_directory(&ui_state);
                    } else {
                        // A on file: open overlay for file
                        ui_open_overlay(&ui_state);
                    }
                }
            }

            // Handle back button (go to parent directory)
            if (input_back()) {
                // Ignore error if already at root
                ui_go_back(&ui_state);
            }

            // Handle file ops button (X)
            if (input_fileops()) {
                FsEntry* selected = ui_get_selected_entry(&ui_state);
                if (selected != NULL && selected->is_dir) {
                    ui_open_overlay(&ui_state);
                }
            }

            // Handle exit button (return to hbmenu)
            if (input_exit()) {
                break;
            }
        }

        // Render current state
        ui_render(&ui_state);
    }

    // Cleanup
    clipboard_clear();
    ui_cleanup(&ui_state);
    fs_cleanup();
    text_exit();

    return 0;
}

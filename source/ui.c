#include "ui.h"
#include "fs.h"
#include "utils.h"
#include "text.h"
#include "input.h"        // needed for popup input handling
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * UI Module Implementation
 * 
 * Renders the file browser interface with selection highlighting.
 * Handles directory navigation and display of file listings.
 */

#define MAX_VISIBLE_ENTRIES 20

// Popup types
#define POPUP_NONE   0
#define POPUP_MESSAGE 1
#define POPUP_RENAME  2

// Forward declaration
static void ui_render_overlay(UIState* ui_state);
static void ui_render_popup(UIState* ui_state);

void ui_init(UIState* ui_state)
{
    if (ui_state == NULL)
        return;

    ui_state->selected_index = 0;
    ui_state->scroll_offset = 0;
    ui_state->current_dir = NULL;
    ui_state->overlay_active = 0;
    ui_state->overlay_selected = 0;

    // initialize popup state
    ui_state->popup_active = 0;
    ui_state->popup_type = POPUP_NONE;
    ui_state->popup_message[0] = '\0';
    ui_state->popup_timer = 0;

    // Start at root - libnx automatically handles sdmc:/ redirection
    strcpy(ui_state->current_path, "/");

    // Load initial directory
    ui_state->current_dir = fs_list_directory(ui_state->current_path);
}

void ui_render(UIState* ui_state)
{
    if (ui_state == NULL || ui_state->current_dir == NULL)
        return;

    // Clear screen
    text_clear();

    // Draw header
    text_draw(0, 0, "=== FILE BROWSER ===");
    text_draw(0, 1, ui_state->current_path);

    // Draw separator
    text_draw(0, 2, "====================");

    // Draw entries
    int display_start = ui_state->scroll_offset;
    int display_count = (ui_state->current_dir->count < MAX_VISIBLE_ENTRIES) ?
                        ui_state->current_dir->count : MAX_VISIBLE_ENTRIES;

    for (int i = 0; i < display_count; i++) {
        int entry_idx = display_start + i;
        if (entry_idx >= ui_state->current_dir->count)
            break;

        FsEntry* entry = &ui_state->current_dir->entries[entry_idx];

        // Prepare display string
        char display[512];
        if (entry->is_dir) {
            snprintf(display, sizeof(display), "[%s]", entry->name);
        } else {
            // Format file size
            uint64_t size = entry->size;
            const char* unit = "B";
            int display_size = size;

            if (size > 1024 * 1024) {
                display_size = size / (1024 * 1024);
                unit = "MB";
            } else if (size > 1024) {
                display_size = size / 1024;
                unit = "KB";
            }

            snprintf(display, sizeof(display), "%s (%d%s)", entry->name, display_size, unit);
        }

        // Highlight selected entry
        int y = 3 + i;
        if (entry_idx == ui_state->selected_index) {
            text_draw_formatted(0, y, "i", display);
        } else {
            text_draw(0, y, display);
        }
    }

    // Draw footer with controls
    int footer_y = 24;
    if (ui_state->overlay_active) {
        text_draw(0, footer_y, "Controls: UP/DOWN=Select, A=Confirm, B=Cancel");
    } else if (ui_state->popup_active && ui_state->popup_type == POPUP_RENAME) {
        text_draw(0, footer_y, "Controls: A=OK B=Cancel U/D=Char L/R=Move");
    } else {
        text_draw(0, footer_y, "Controls: UP/DOWN=Navigate, A=Select, B=Back, X=FileOps, Plus=Exit");
    }

    // Draw current selection info
    if (!ui_state->overlay_active && ui_state->selected_index < ui_state->current_dir->count) {
        FsEntry* sel = &ui_state->current_dir->entries[ui_state->selected_index];
        char info[512];
        snprintf(info, sizeof(info), "Selected: %s (%s)",
                 sel->name, sel->is_dir ? "DIR" : "FILE");
        text_draw(0, 25, info);
    }

    // Draw overlay if active
    if (ui_state->overlay_active) {
        ui_render_overlay(ui_state);
    }

    // Draw popup on top if active
    if (ui_state->popup_active) {
        ui_render_popup(ui_state);
    }

    // Update display
    text_update();
}

void ui_select_next(UIState* ui_state)
{
    if (ui_state == NULL || ui_state->current_dir == NULL)
        return;

    if (ui_state->selected_index < ui_state->current_dir->count - 1) {
        ui_state->selected_index++;

        // Adjust scroll if needed
        if (ui_state->selected_index >= ui_state->scroll_offset + MAX_VISIBLE_ENTRIES) {
            ui_state->scroll_offset++;
        }
    }
}

void ui_select_prev(UIState* ui_state)
{
    if (ui_state == NULL || ui_state->current_dir == NULL)
        return;

    if (ui_state->selected_index > 0) {
        ui_state->selected_index--;

        // Adjust scroll if needed
        if (ui_state->selected_index < ui_state->scroll_offset) {
            ui_state->scroll_offset--;
        }
    }
}

FsEntry* ui_get_selected_entry(UIState* ui_state)
{
    if (ui_state == NULL || ui_state->current_dir == NULL)
        return NULL;

    if (ui_state->selected_index >= ui_state->current_dir->count)
        return NULL;

    return &ui_state->current_dir->entries[ui_state->selected_index];
}

void ui_get_selected_path(UIState* ui_state, char* dest)
{
    if (ui_state == NULL || dest == NULL)
        return;

    FsEntry* entry = ui_get_selected_entry(ui_state);
    if (entry == NULL) {
        dest[0] = '\0';
        return;
    }

    fs_build_path(ui_state->current_path, entry->name, dest);
}

int ui_enter_directory(UIState* ui_state)
{
    if (ui_state == NULL)
        return -1;

    FsEntry* entry = ui_get_selected_entry(ui_state);
    if (entry == NULL || !entry->is_dir)
        return -1;

    // Build new path
    char new_path[512];
    ui_get_selected_path(ui_state, new_path);

    // Check if valid
    if (!fs_is_valid_path(new_path))
        return -1;

    // Load new directory
    FsDirectory* new_dir = fs_list_directory(new_path);
    if (new_dir == NULL)
        return -1;

    // Clean up old directory
    if (ui_state->current_dir != NULL)
        fs_free_directory(ui_state->current_dir);

    // Update state
    strcpy(ui_state->current_path, new_path);
    ui_state->current_dir = new_dir;
    ui_state->selected_index = 0;
    ui_state->scroll_offset = 0;

    return 0;
}

int ui_go_back(UIState* ui_state)
{
    if (ui_state == NULL)
        return -1;

    // Get parent path
    char parent_path[512];
    if (path_get_parent(ui_state->current_path, parent_path) != 0)
        return -1;  // Already at root

    // Load parent directory
    FsDirectory* parent_dir = fs_list_directory(parent_path);
    if (parent_dir == NULL)
        return -1;

    // Clean up current directory
    if (ui_state->current_dir != NULL)
        fs_free_directory(ui_state->current_dir);

    // Update state
    strcpy(ui_state->current_path, parent_path);
    ui_state->current_dir = parent_dir;
    ui_state->selected_index = 0;
    ui_state->scroll_offset = 0;

    return 0;
}

void ui_cleanup(UIState* ui_state)
{
    if (ui_state == NULL)
        return;

    if (ui_state->current_dir != NULL) {
        fs_free_directory(ui_state->current_dir);
        ui_state->current_dir = NULL;
    }
}

/**
 * Overlay Menu Implementation
 */

// helper: file extension checks
static int has_extension(const char* name, const char* ext)
{
    if (!name || !ext) return 0;
    int len = strlen(name);
    int elen = strlen(ext);
    if (len < elen) return 0;
    return strcasecmp(&name[len-elen], ext) == 0;
}

static int is_nro_file(const char* name)
{
    return has_extension(name, ".nro");
}

static int is_installer_file(const char* name)
{
    return has_extension(name, ".nsp") || has_extension(name, ".nsz") ||
           has_extension(name, ".xci") || has_extension(name, ".xcz");
}

void ui_open_overlay(UIState* ui_state)
{
    if (ui_state == NULL)
        return;

    ui_state->overlay_active = 1;
    ui_state->overlay_selected = 0;
    ui_state->overlay_count = 0;

    FsEntry* sel = ui_get_selected_entry(ui_state);
    if (sel == NULL)
        return;

    // always include basic operations
    const char* basic_labels[] = {"Copy", "Paste", "Move", "Delete", "Rename"};
    int basic_codes[] = {UI_OP_COPY, UI_OP_PASTE, UI_OP_MOVE, UI_OP_DELETE, UI_OP_RENAME};
    for (int i = 0; i < 5; i++) {
        strncpy(ui_state->overlay_labels[ui_state->overlay_count], basic_labels[i], 31);
        ui_state->overlay_labels[ui_state->overlay_count][31] = '\0';
        ui_state->overlay_codes[ui_state->overlay_count] = basic_codes[i];
        ui_state->overlay_count++;
    }

    // additional options for files
    if (!sel->is_dir) {
        if (is_nro_file(sel->name)) {
            strncpy(ui_state->overlay_labels[ui_state->overlay_count], "Launch", 31);
            ui_state->overlay_labels[ui_state->overlay_count][31] = '\0';
            ui_state->overlay_codes[ui_state->overlay_count] = UI_OP_LAUNCH;
            ui_state->overlay_count++;
        }
        if (is_installer_file(sel->name)) {
            strncpy(ui_state->overlay_labels[ui_state->overlay_count], "Install", 31);
            ui_state->overlay_labels[ui_state->overlay_count][31] = '\0';
            ui_state->overlay_codes[ui_state->overlay_count] = UI_OP_INSTALL;
            ui_state->overlay_count++;
        }
    }
}

void ui_close_overlay(UIState* ui_state)
{
    if (ui_state == NULL)
        return;

    ui_state->overlay_active = 0;
}

void ui_overlay_select_next(UIState* ui_state)
{
    if (ui_state == NULL || !ui_state->overlay_active)
        return;

    if (ui_state->overlay_selected < ui_state->overlay_count - 1) {
        ui_state->overlay_selected++;
    }
}

void ui_overlay_select_prev(UIState* ui_state)
{
    if (ui_state == NULL || !ui_state->overlay_active)
        return;

    if (ui_state->overlay_selected > 0) {
        ui_state->overlay_selected--;
    }
}

int ui_overlay_get_selected(UIState* ui_state)
{
    if (ui_state == NULL || !ui_state->overlay_active)
        return -1;

    return ui_state->overlay_selected;
}

/** Helper function to render the overlay menu */
static void ui_render_overlay(UIState* ui_state)
{
    if (ui_state == NULL || !ui_state->overlay_active)
        return;

    // Draw semi-transparent background (black box covering middle of screen)
    int overlay_top = 8;
    int overlay_height = 12;
    int overlay_left = 15;

    // Draw background box using spaces with inverse video
    for (int y = overlay_top; y < overlay_top + overlay_height; y++) {
        char line[80];
        memset(line, ' ', sizeof(line));
        line[79] = '\0';
        text_draw_formatted(overlay_left - 2, y, "i", "                                              ");
    }

    // Draw title
    text_draw_formatted(overlay_left + 8, overlay_top + 1, "i", "FILE OPS");

    // Draw menu options dynamically
    for (int i = 0; i < ui_state->overlay_count; i++) {
        int y = overlay_top + 3 + i;
        char menu_item[48];
        snprintf(menu_item, sizeof(menu_item), "  %s", ui_state->overlay_labels[i]);

        if (i == ui_state->overlay_selected) {
            text_draw_formatted(overlay_left, y, "i", menu_item);
        } else {
            text_draw(overlay_left, y, menu_item);
        }
    }

    // Draw instructions
    text_draw(overlay_left, overlay_top + overlay_height - 2, "A=Select  B=Cancel");
}


/**
 * Popup rendering and control helpers
 */
static void ui_render_popup(UIState* ui_state)
{
    if (ui_state == NULL || !ui_state->popup_active)
        return;

    // dim entire screen by drawing inverse-space lines
    for (int y = 0; y < 27; y++) {
        text_draw_formatted(0, y, "i", "                                                                                ");
    }

    int box_left = 10;
    int box_top = 8;

    if (ui_state->popup_type == POPUP_MESSAGE) {
        // center the message vertically
        int msg_y = box_top + 4;
        text_draw(box_left + 2, msg_y, ui_state->popup_message);
    }
}

void ui_show_message(UIState* ui_state, const char* msg, int duration)
{
    if (ui_state == NULL || msg == NULL)
        return;
    ui_state->popup_active = 1;
    ui_state->popup_type = POPUP_MESSAGE;
    strncpy(ui_state->popup_message, msg, sizeof(ui_state->popup_message)-1);
    ui_state->popup_message[sizeof(ui_state->popup_message)-1] = '\0';
    ui_state->popup_timer = duration;
}


int ui_process_popup_input(UIState* ui_state)
{
    if (ui_state == NULL || !ui_state->popup_active)
        return 0;

    if (ui_state->popup_type == POPUP_MESSAGE) {
        // dismiss on any button or when timer expires
        if (ui_state->popup_timer > 0) {
            ui_state->popup_timer--;
            if (ui_state->popup_timer <= 0) {
                ui_state->popup_active = 0;
                ui_state->popup_type = POPUP_NONE;
                return 0;
            }
        }
        // check input
        if (input_up() || input_down() || input_left() || input_right() ||
            input_select() || input_back() || input_fileops() || input_exit()) {
            ui_state->popup_active = 0;
            ui_state->popup_type = POPUP_NONE;
            return 0;
        }
        return 1;
    }

    return 0;
}


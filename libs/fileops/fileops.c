#include "fileops.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../utils/utils.h"

/**
 * File Operations Module Implementation
 * 
 * Provides file copy, paste, move, and delete operations.
 * Uses a clipboard mechanism for copy/paste workflow.
 */

// Global file operations state
static FileOpsState g_fileops_state;

void fileops_init(void)
{
    // Initialize clipboard as empty
    g_fileops_state.clipboard_path[0] = '\0';
    g_fileops_state.clipboard_has_data = 0;
    g_fileops_state.clipboard_is_cut = 0;
}

void fileops_cleanup(void)
{
    // Clear clipboard
    memset(&g_fileops_state, 0, sizeof(FileOpsState));
}

int fileops_copy(const char* source_path)
{
    if (source_path == NULL)
        return -1;

    // Copy path to clipboard
    str_copy(g_fileops_state.clipboard_path, source_path, sizeof(g_fileops_state.clipboard_path));
    g_fileops_state.clipboard_has_data = 1;
    g_fileops_state.clipboard_is_cut = 0;  // Copy, not cut
    return 0;
}

int fileops_paste(const char* dest_dir)
{
    if (dest_dir == NULL || !g_fileops_state.clipboard_has_data)
        return -1;

    // Extract filename from clipboard path
    const char* filename_ptr = path_get_filename(g_fileops_state.clipboard_path);
    if (filename_ptr == NULL)
        return -1;
    
    char filename[256];
    strncpy(filename, filename_ptr, sizeof(filename) - 1);
    filename[sizeof(filename) - 1] = '\0';

    // Build destination path
    char dest_path[512];
    snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, filename);

    // Copy file from clipboard to destination
    FILE* src = fopen(g_fileops_state.clipboard_path, "rb");
    if (src == NULL)
        return -1;

    FILE* dst = fopen(dest_path, "wb");
    if (dst == NULL) {
        fclose(src);
        return -1;
    }

    // Copy file data in chunks
    char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        if (fwrite(buffer, 1, bytes_read, dst) != bytes_read) {
            fclose(src);
            fclose(dst);
            return -1;
        }
    }

    fclose(src);
    fclose(dst);

    // If this was a move (cut), delete the original
    if (g_fileops_state.clipboard_is_cut) {
        remove(g_fileops_state.clipboard_path);
        g_fileops_state.clipboard_has_data = 0;
        g_fileops_state.clipboard_path[0] = '\0';
    }

    return 0;
}

int fileops_move(const char* source_path, const char* dest_dir)
{
    if (source_path == NULL || dest_dir == NULL)
        return -1;

    // Set clipboard to cut mode (not copy)
    str_copy(g_fileops_state.clipboard_path, source_path, sizeof(g_fileops_state.clipboard_path));
    g_fileops_state.clipboard_has_data = 1;
    g_fileops_state.clipboard_is_cut = 1;  // Move, not copy

    // Execute the move
    return fileops_paste(dest_dir);
}

int fileops_delete(const char* path)
{
    if (path == NULL)
        return -1;

    // Try to delete file using remove()
    if (remove(path) == 0) {
        // If we deleted the clipboard item, clear it
        if (strcmp(path, g_fileops_state.clipboard_path) == 0) {
            g_fileops_state.clipboard_has_data = 0;
            g_fileops_state.clipboard_path[0] = '\0';
        }
        return 0;
    }

    return -1;
}

int fileops_has_clipboard(void)
{
    return g_fileops_state.clipboard_has_data;
}

int fileops_get_clipboard_name(char* dest)
{
    if (dest == NULL || !g_fileops_state.clipboard_has_data)
        return -1;

    // Extract filename from clipboard path
    const char* filename_ptr = path_get_filename(g_fileops_state.clipboard_path);
    if (filename_ptr == NULL)
        return -1;
    
    strncpy(dest, filename_ptr, 255);
    dest[255] = '\0';
    return 0;
}

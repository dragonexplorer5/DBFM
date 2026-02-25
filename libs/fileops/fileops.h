#ifndef FILEOPS_H
#define FILEOPS_H

/**
 * File Operations Module
 * 
 * Provides core file operations: copy, paste, move, delete.
 * Maintains a clipboard state for copy/paste operations.
 */

/**
 * FileOpsState - Global clipboard and operation state
 */
typedef struct {
    char clipboard_path[512];  // Path of copied/cut file
    int clipboard_has_data;    // 1 if clipboard contains data
    int clipboard_is_cut;      // 1 if cut (move), 0 if copy
} FileOpsState;

/**
 * fileops_init()
 * Initialize the file operations system.
 * Must be called once at application startup.
 */
void fileops_init(void);

/**
 * fileops_cleanup()
 * Clean up file operations resources.
 * Call before application exit.
 */
void fileops_cleanup(void);

/**
 * fileops_copy(source_path)
 * Copy a file to the clipboard (non-destructive).
 * Returns 0 on success, -1 on error.
 */
int fileops_copy(const char* source_path);

/**
 * fileops_paste(dest_dir)
 * Paste the file from clipboard to destination directory.
 * Preserves original if clipboard was copied, removes if clipboard was cut.
 * Returns 0 on success, -1 on error.
 */
int fileops_paste(const char* dest_dir);

/**
 * fileops_move(source_path, dest_dir)
 * Move a file from source to destination directory.
 * Equivalent to copy + delete.
 * Returns 0 on success, -1 on error.
 */
int fileops_move(const char* source_path, const char* dest_dir);

/**
 * fileops_delete(path)
 * Delete a file (does not delete directories).
 * Returns 0 on success, -1 on error.
 */
int fileops_delete(const char* path);

/**
 * fileops_has_clipboard()
 * Check if clipboard contains data.
 * Returns 1 if clipboard has data, 0 otherwise.
 */
int fileops_has_clipboard(void);

/**
 * fileops_get_clipboard_name(dest)
 * Get the filename of the clipboard item (just the name, not full path).
 * dest must be at least 256 bytes.
 * Returns 0 on success, -1 if clipboard empty.
 */
int fileops_get_clipboard_name(char* dest);

#endif

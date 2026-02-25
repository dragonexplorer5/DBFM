#ifndef FS_H
#define FS_H

#include <switch.h>

/**
 * Filesystem Module
 * 
 * Provides directory listing and navigation functionality.
 * Works with the SD card via both "/" (default) and "sdmc:/" paths.
 * Uses standard POSIX directory operations (opendir, readdir, closedir).
 */

/**
 * FsEntry - Represents a single file or directory entry
 */
typedef struct {
    char name[256];      // File/folder name
    int is_dir;          // 1 if directory, 0 if file
    uint64_t size;       // File size in bytes (0 for directories)
} FsEntry;

/**
 * FsDirectory - Contains directory listing with dynamic array
 */
typedef struct {
    FsEntry* entries;    // Array of directory entries (malloc'd)
    int count;           // Number of entries in array
    int capacity;        // Allocated capacity (grows as needed)
} FsDirectory;

/**
 * fs_init()
 * Initialize the filesystem module.
 * No special initialization needed for libnx.
 */
void fs_init(void);

/**
 * fs_cleanup()
 * Cleanup filesystem resources.
 * No special cleanup needed at module level.
 */
void fs_cleanup(void);

/**
 * fs_list_directory(path)
 * Read directory contents and return allocated FsDirectory structure.
 * Returns NULL on failure (path not found or unable to open).
 * Caller must free result with fs_free_directory().
 */
FsDirectory* fs_list_directory(const char* path);

/**
 * fs_free_directory(dir)
 * Free memory allocated by fs_list_directory().
 * Safe to call with NULL pointer.
 */
void fs_free_directory(FsDirectory* dir);

/**
 * fs_build_path(current_path, entry_name, dest)
 * Construct full path by combining directory path with entry name.
 * dest must be at least 512 bytes.
 */
void fs_build_path(const char* current_path, const char* entry_name, char* dest);

/**
 * fs_is_valid_path(path)
 * Check if path is valid and accessible (can open directory).
 * Returns 1 if valid, 0 if invalid.
 */
int fs_is_valid_path(const char* path);

/**
 * fs_is_directory(entry)
 * Check if entry represents a directory.
 * Returns 1 if directory, 0 if file or NULL entry.
 */
int fs_is_directory(const FsEntry* entry);

#endif

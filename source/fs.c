#include "fs.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

/**
 * Filesystem Module Implementation
 * 
 * Provides directory listing and navigation.
 * Works with SD card using standard POSIX operations.
 * Automatically supports both "/" and "sdmc:/" paths via libnx.
 */

// Keep handle for mounted SD card filesystem so POSIX calls work on Switch
static FsFileSystem g_sd_fs;
static int g_sd_mounted = 0;

void fs_init(void)
{
    // Try to mount the SD card filesystem so POSIX APIs (opendir/stat/fopen)
    // operate correctly on Switch. This uses libnx helper to open the sdcard
    // filesystem and mounts it under the "sdmc" device via fsdev.
    Result rc = fsOpenSdCardFileSystem(&g_sd_fs);
    if (R_SUCCEEDED(rc)) {
        rc = fsdevMountDevice("sdmc", g_sd_fs);
        if (R_SUCCEEDED(rc)) {
            g_sd_mounted = 1;
        } else {
            // Failed to mount; close fs handle
            fsFsClose(&g_sd_fs);
            g_sd_mounted = 0;
        }
    }
    // Even if mount failed, POSIX calls may still operate on some paths.
}

void fs_cleanup(void)
{
    // Unmount sdmc if we mounted it
    if (g_sd_mounted) {
        fsdevUnmountDevice("sdmc");
        fsFsClose(&g_sd_fs);
        g_sd_mounted = 0;
    }
}

FsDirectory* fs_list_directory(const char* path)
{
    if (path == NULL)
        return NULL;

    // Open directory using standard POSIX (libnx handles path resolution)
    DIR* dir = opendir(path);
    if (dir == NULL)
        return NULL;

    // Allocate directory structure
    FsDirectory* fs_dir = (FsDirectory*)malloc(sizeof(FsDirectory));
    if (fs_dir == NULL) {
        closedir(dir);
        return NULL;
    }

    // Initialize
    fs_dir->capacity = 32;
    fs_dir->count = 0;
    fs_dir->entries = (FsEntry*)malloc(sizeof(FsEntry) * fs_dir->capacity);

    if (fs_dir->entries == NULL) {
        free(fs_dir);
        closedir(dir);
        return NULL;
    }

    // Read directory entries
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Resize array if needed
        if (fs_dir->count >= fs_dir->capacity) {
            fs_dir->capacity *= 2;
            FsEntry* new_entries = (FsEntry*)realloc(fs_dir->entries,
                                                      sizeof(FsEntry) * fs_dir->capacity);
            if (new_entries == NULL) {
                closedir(dir);
                return fs_dir;  // Return partial results
            }
            fs_dir->entries = new_entries;
        }

        // Copy entry data
        FsEntry* cur_entry = &fs_dir->entries[fs_dir->count];
        strncpy(cur_entry->name, entry->d_name, sizeof(cur_entry->name) - 1);
        cur_entry->name[sizeof(cur_entry->name) - 1] = '\0';

        // Determine if directory
        cur_entry->is_dir = (entry->d_type == DT_DIR);

        // Get file size
        cur_entry->size = 0;
        if (!cur_entry->is_dir) {
            // Build full path for stat
            char full_path[512];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

            struct stat file_stat;
            if (stat(full_path, &file_stat) == 0) {
                cur_entry->size = file_stat.st_size;
            }
        }

        fs_dir->count++;
    }

    closedir(dir);
    return fs_dir;
}

void fs_free_directory(FsDirectory* dir)
{
    if (dir == NULL)
        return;

    if (dir->entries != NULL)
        free(dir->entries);

    free(dir);
}

void fs_build_path(const char* current_path, const char* entry_name, char* dest)
{
    if (dest == NULL)
        return;

    if (current_path == NULL || entry_name == NULL) {
        dest[0] = '\0';
        return;
    }

    snprintf(dest, 512, "%s/%s", current_path, entry_name);
}

int fs_is_valid_path(const char* path)
{
    if (path == NULL)
        return 0;

    DIR* dir = opendir(path);
    if (dir == NULL)
        return 0;

    closedir(dir);
    return 1;
}

int fs_is_directory(const FsEntry* entry)
{
    if (entry == NULL)
        return 0;

    return entry->is_dir;
}

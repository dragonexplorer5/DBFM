#include "delete.h"
#include <switch.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Normalize sd path helper
static void normalize_sd_path(const char* in, char* out, size_t outlen)
{
    if (!in || !out || outlen == 0) return;
    const char* p = in;
    if (strncmp(p, "sdmc:/", 6) == 0) p += 6;
    if (*p == '/') p++;
    strncpy(out, p, outlen - 1);
    out[outlen - 1] = '\0';
}

static int delete_recursive_libnx(const char* path)
{
    if (!path) return -1;

    Result rc;
    FsFileSystem fs;
    rc = fsOpenSdCardFileSystem(&fs);
    if (R_FAILED(rc)) return -1;

    char path_n[512];
    normalize_sd_path(path, path_n, sizeof(path_n));

    // Try to open as directory
    FsDir dir;
    rc = fsFsOpenDirectory(&fs, path_n, FsDirOpenMode_ReadDirs | FsDirOpenMode_ReadFiles, &dir);
    if (R_SUCCEEDED(rc)) {
        // Directory: iterate and delete children
        while (1) {
            s64 entries = 0;
            FsDirectoryEntry entry;
            rc = fsDirRead(&dir, &entries, 1, &entry);
            if (R_FAILED(rc)) { fsDirClose(&dir); fsFsClose(&fs); return -1; }
            if (entries == 0) break;
            if (strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0) continue;

            char child[512];
            snprintf(child, sizeof(child), "%s/%s", path, entry.name);
            if (delete_recursive_libnx(child) != 0) { fsDirClose(&dir); fsFsClose(&fs); return -1; }
        }
        fsDirClose(&dir);
        // Remove directory
        rc = fsFsDeleteDirectory(&fs, path_n);
        fsFsClose(&fs);
        if (R_FAILED(rc)) return -1;
        return 0;
    }

    // Not a directory -> delete file
    rc = fsFsDeleteFile(&fs, path_n);
    fsFsClose(&fs);
    if (R_FAILED(rc)) return -1;
    return 0;
}

int delete_item(const char* path)
{
    if (path == NULL) return -1;
    return delete_recursive_libnx(path);
}

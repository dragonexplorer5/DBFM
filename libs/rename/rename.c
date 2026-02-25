#include "rename.h"
#include <switch.h>
#include <string.h>
#include <stdio.h>

#include "../utils/utils.h"

// Normalize sd path helper (same as copy.c/delete.c)
static void normalize_sd_path(const char* in, char* out, size_t outlen)
{
    if (!in || !out || outlen == 0) return;
    const char* p = in;
    if (strncmp(p, "sdmc:/", 6) == 0) p += 6;
    if (*p == '/') p++;
    strncpy(out, p, outlen - 1);
    out[outlen - 1] = '\0';
}

int rename_item(const char* path, const char* newname)
{
    if (path == NULL || newname == NULL)
        return -1;

    // compute parent directory
    char parent[512];
    if (path_get_parent(path, parent) != 0)
        return -1;

    // build destination full path
    char dest_full[512];
    snprintf(dest_full, sizeof(dest_full), "%s/%s", parent, newname);

    // open filesystem
    Result rc;
    FsFileSystem fs;
    rc = fsOpenSdCardFileSystem(&fs);
    if (R_FAILED(rc))
        return -1;

    // normalize both paths for libnx operations
    char src_n[512];
    char dest_n[512];
    normalize_sd_path(path, src_n, sizeof(src_n));
    normalize_sd_path(dest_full, dest_n, sizeof(dest_n));

    // determine if source is directory
    int is_dir = 0;
    FsDir dir;
    rc = fsFsOpenDirectory(&fs, src_n, FsDirOpenMode_ReadDirs, &dir);
    if (R_SUCCEEDED(rc)) {
        is_dir = 1;
        fsDirClose(&dir);
    }

    // perform rename by type
    if (is_dir) {
        rc = fsFsRenameDirectory(&fs, src_n, dest_n);
    } else {
        rc = fsFsRenameFile(&fs, src_n, dest_n);
    }

    fsFsClose(&fs);

    if (R_FAILED(rc))
        return -1;
    return 0;
}

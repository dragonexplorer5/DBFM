#include "move.h"
#include <switch.h>
#include <string.h>
#include <stdio.h>
#include "../copy/copy.h"
#include "../utils/utils.h"
#include "../delete/delete.h"

int move_file(const char* src, const char* dest_dir)
{
    if (src == NULL || dest_dir == NULL) return -1;

    const char* filename = path_get_filename(src);
    if (!filename) return -1;
    char dest_path[512];
    snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, filename);

    // Try libnx rename first
    Result rc;
    FsFileSystem fs;
    rc = fsOpenSdCardFileSystem(&fs);
    if (R_SUCCEEDED(rc)) {
        char src_n[512];
        char dest_n[512];
        // normalize
        const char* p = src;
        if (strncmp(p, "sdmc:/", 6) == 0) p += 6;
        if (*p == '/') p++;
        strncpy(src_n, p, sizeof(src_n)-1); src_n[sizeof(src_n)-1] = '\0';
        p = dest_path;
        if (strncmp(p, "sdmc:/", 6) == 0) p += 6;
        if (*p == '/') p++;
        strncpy(dest_n, p, sizeof(dest_n)-1); dest_n[sizeof(dest_n)-1] = '\0';

        rc = fsFsRenameFile(&fs, src_n, dest_n);
        fsFsClose(&fs);
        if (R_SUCCEEDED(rc)) return 0;
    }

    // Fallback: copy then delete
    if (copy_item(src, dest_dir) != 0) return -1;
    if (delete_item(src) != 0) return -1;
    return 0;
}

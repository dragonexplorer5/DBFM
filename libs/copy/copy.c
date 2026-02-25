#include "copy.h"
#include <switch.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../utils/utils.h"

// Helper: normalize incoming paths to be relative to the SdCard FsFileSystem.
// Accepts paths like "/switch/foo" or "sdmc:/switch/foo" and returns
// "switch/foo" in out (no leading slash).
static void normalize_sd_path(const char* in, char* out, size_t outlen)
{
    if (!in || !out || outlen == 0) return;
    const char* p = in;
    // strip leading "sdmc:/"
    if (strncmp(p, "sdmc:/", 6) == 0) p += 6;
    // strip single leading '/'
    if (*p == '/') p++;
    // copy remaining
    strncpy(out, p, outlen - 1);
    out[outlen - 1] = '\0';
}

static int copy_file_contents_libnx(const char* src, const char* dest)
{
    if (!src || !dest) return -1;

    Result rc;
    FsFileSystem fs;
    rc = fsOpenSdCardFileSystem(&fs);
    if (R_FAILED(rc)) return -1;

    char src_n[512];
    char dest_n[512];
    normalize_sd_path(src, src_n, sizeof(src_n));
    normalize_sd_path(dest, dest_n, sizeof(dest_n));

    FsFile inFile, outFile;
    rc = fsFsOpenFile(&fs, src_n, FsOpenMode_Read, &inFile);
    if (R_FAILED(rc)) { fsFsClose(&fs); return -1; }

    // Ensure destination exists (create empty file if needed)
    fsFsCreateFile(&fs, dest_n, 0, 0);
    rc = fsFsOpenFile(&fs, dest_n, FsOpenMode_Write, &outFile);
    if (R_FAILED(rc)) { fsFileClose(&inFile); fsFsClose(&fs); return -1; }

    char buf[4096];
    s64 off_in = 0;
    s64 off_out = 0;
    while (1) {
        u64 bytesRead = 0;
        rc = fsFileRead(&inFile, off_in, buf, sizeof(buf), FsReadOption_None, &bytesRead);
        if (R_FAILED(rc)) { fsFileClose(&inFile); fsFileClose(&outFile); fsFsClose(&fs); return -1; }
        if (bytesRead == 0) break;
        rc = fsFileWrite(&outFile, off_out, buf, bytesRead, FsWriteOption_None);
        if (R_FAILED(rc)) { fsFileClose(&inFile); fsFileClose(&outFile); fsFsClose(&fs); return -1; }
        off_in += (s64)bytesRead;
        off_out += (s64)bytesRead;
    }

    fsFileClose(&inFile);
    fsFileClose(&outFile);
    fsFsClose(&fs);
    return 0;
}

static int copy_dir_recursive_libnx(const char* src_dir, const char* dest_dir)
{
    if (!src_dir || !dest_dir) return -1;

    Result rc;
    FsFileSystem fs;
    rc = fsOpenSdCardFileSystem(&fs);
    if (R_FAILED(rc)) return -1;

    char src_n[512];
    char dest_n[512];
    normalize_sd_path(src_dir, src_n, sizeof(src_n));
    normalize_sd_path(dest_dir, dest_n, sizeof(dest_n));

    // Ensure dest exists
    rc = fsFsCreateDirectory(&fs, dest_n);
    if (R_FAILED(rc) && rc != (Result)0x402) { /* ignore already exists (converted below) */ }

    FsDir dir;
    rc = fsFsOpenDirectory(&fs, src_n, FsDirOpenMode_ReadDirs | FsDirOpenMode_ReadFiles, &dir);
    if (R_FAILED(rc)) { fsFsClose(&fs); return -1; }

    while (1) {
        s64 entries = 0;
        FsDirectoryEntry entry;
        rc = fsDirRead(&dir, &entries, 1, &entry);
        if (R_FAILED(rc)) { fsDirClose(&dir); fsFsClose(&fs); return -1; }
        if (entries == 0) break;

        // entry.name is a UTF-8 string
        if (strcmp(entry.name, ".") == 0 || strcmp(entry.name, "..") == 0) continue;

        char child_src[512];
        char child_dest[512];
        snprintf(child_src, sizeof(child_src), "%s/%s", src_n, entry.name);
        snprintf(child_dest, sizeof(child_dest), "%s/%s", dest_n, entry.name);

        if (entry.type == FsDirEntryType_Dir) {
            if (copy_dir_recursive_libnx(child_src, child_dest) != 0) {
                fsDirClose(&dir);
                fsFsClose(&fs);
                return -1;
            }
        } else {
            if (copy_file_contents_libnx(child_src, child_dest) != 0) {
                fsDirClose(&dir);
                fsFsClose(&fs);
                return -1;
            }
        }
    }

    fsDirClose(&dir);
    fsFsClose(&fs);
    return 0;
}

int copy_item(const char* src, const char* dest_dir)
{
    if (src == NULL || dest_dir == NULL) return -1;

    // Determine name
    const char* name = path_get_filename(src);
    if (!name) return -1;

    char dest_path[512];
    snprintf(dest_path, sizeof(dest_path), "%s/%s", dest_dir, name);

    // Try open src as directory using libnx to detect type
    Result rc;
    FsFileSystem fs;
    rc = fsOpenSdCardFileSystem(&fs);
    if (R_FAILED(rc)) return -1;

    char src_n[512];
    normalize_sd_path(src, src_n, sizeof(src_n));

    FsDir dir;
    rc = fsFsOpenDirectory(&fs, src_n, FsDirOpenMode_ReadDirs | FsDirOpenMode_ReadFiles, &dir);
    if (R_SUCCEEDED(rc)) {
        // It's a directory
        fsDirClose(&dir);
        fsFsClose(&fs);
        return copy_dir_recursive_libnx(src, dest_path);
    }

    // Not a directory -> copy file
    fsFsClose(&fs);
    return copy_file_contents_libnx(src, dest_path);
}

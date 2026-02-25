#ifndef RENAME_H
#define RENAME_H

/**
 * Rename module
 *
 * Provides cross-type rename operations using libnx filesystem APIs.
 */

/**
 * rename_item(path, newname)
 * Rename the file or directory specified by 'path' to the base name
 * 'newname'.  The parent directory is preserved.  Returns 0 on success
 * or -1 on failure.
 */
int rename_item(const char* path, const char* newname);

#endif

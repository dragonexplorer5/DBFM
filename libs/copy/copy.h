#ifndef COPY_H
#define COPY_H

/* Copy an item (file or directory) from src into dest_dir.
 * If src is a directory, copy recursively.
 * Returns 0 on success, -1 on error.
 */
int copy_item(const char* src, const char* dest_dir);

#endif

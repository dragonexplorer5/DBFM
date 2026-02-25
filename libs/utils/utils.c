#include "utils.h"
#include <string.h>
#include <stdlib.h>

/**
 * String Utilities Implementation
 */

int str_len(const char* str)
{
    if (str == NULL) return 0;
    int len = 0;
    while (str[len] != '\0') len++;
    return len;
}

int str_copy(char* dest, const char* src, int dest_size)
{
    if (dest == NULL || src == NULL || dest_size <= 0)
        return -1;

    int i = 0;
    while (src[i] != '\0' && i < dest_size - 1) {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';

    // Return -1 if truncated
    if (src[i] != '\0')
        return -1;

    return 0;
}

int str_concat(char* dest, const char* src, int dest_size)
{
    if (dest == NULL || src == NULL || dest_size <= 0)
        return -1;

    // Find end of dest string
    int dest_len = 0;
    while (dest[dest_len] != '\0' && dest_len < dest_size - 1)
        dest_len++;

    // Copy src to end of dest
    int i = 0;
    while (src[i] != '\0' && (dest_len + i) < dest_size - 1) {
        dest[dest_len + i] = src[i];
        i++;
    }

    dest[dest_len + i] = '\0';

    // Return -1 if truncated
    if (src[i] != '\0')
        return -1;

    return 0;
}

/**
 * Path Utilities Implementation
 */

void path_normalize(char* path)
{
    if (path == NULL)
        return;

    int len = str_len(path);

    // Remove trailing slashes (but keep root "/")
    while (len > 1 && path[len - 1] == '/') {
        path[len - 1] = '\0';
        len--;
    }
}

int path_get_parent(const char* path, char* dest)
{
    if (path == NULL || dest == NULL)
        return -1;

    // Copy path to dest
    str_copy(dest, path, 512);

    // Remove trailing slashes
    path_normalize(dest);

    // Find last slash
    int len = str_len(dest);
    int last_slash = -1;

    for (int i = len - 1; i >= 0; i--) {
        if (dest[i] == '/') {
            last_slash = i;
            break;
        }
    }

    // If no slash found or only root, fail
    if (last_slash <= 0)
        return -1;

    // Truncate at last slash (keep the slash for root)
    dest[last_slash] = '\0';

    return 0;
}

const char* path_get_filename(const char* path)
{
    if (path == NULL)
        return "";

    int len = str_len(path);

    // Find last slash
    for (int i = len - 1; i >= 0; i--) {
        if (path[i] == '/') {
            return &path[i + 1];
        }
    }

    return path;
}

int path_ends_with_separator(const char* path)
{
    if (path == NULL)
        return 0;

    int len = str_len(path);
    return (len > 0 && path[len - 1] == '/');
}

void path_ensure_separator(char* path, int max_len)
{
    if (path == NULL || max_len <= 0)
        return;

    int len = str_len(path);

    // Don't add if already has separator or is empty
    if (len == 0 || path[len - 1] == '/')
        return;

    // Add separator if space available
    if (len + 1 < max_len) {
        path[len] = '/';
        path[len + 1] = '\0';
    }
}

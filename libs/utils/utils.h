#ifndef UTILS_H
#define UTILS_H

/**
 * Utilities Library
 * 
 * Provides reusable helper functions for string manipulation,
 * path handling, and common utility operations.
 * All functions include null-pointer checks.
 */

/**
 * STRING UTILITIES
 */

/**
 * str_len(str)
 * Get length of null-terminated string (safe version).
 * Returns 0 if str is NULL.
 */
int str_len(const char* str);

/**
 * str_copy(dest, src, dest_size)
 * Safely copy string, always null-terminating.
 * Returns 0 on success, -1 if string was truncated.
 */
int str_copy(char* dest, const char* src, int dest_size);

/**
 * str_concat(dest, src, dest_size)
 * Safely concatenate string to dest, always null-terminating.
 * Returns 0 on success, -1 if string was truncated.
 */
int str_concat(char* dest, const char* src, int dest_size);

/**
 * PATH UTILITIES
 */

/**
 * path_normalize(path)
 * Normalize path by removing trailing slashes (except for root "/").
 * Modifies path in-place.
 */
void path_normalize(char* path);

/**
 * path_get_parent(path, dest)
 * Get parent directory path.
 * dest must be at least 512 bytes.
 * Returns 0 on success, -1 if already at root.
 */
int path_get_parent(const char* path, char* dest);

/**
 * path_get_filename(path)
 * Extract filename from full path.
 * Returns pointer to filename portion (pointer into original path).
 */
const char* path_get_filename(const char* path);

/**
 * path_ends_with_separator(path)
 * Check if path ends with forward slash.
 * Returns 1 if ends with "/", 0 otherwise.
 */
int path_ends_with_separator(const char* path);

/**
 * path_ensure_separator(path, max_len)
 * Add forward slash to end of path if not already present.
 * Modifies path in-place. max_len is buffer size.
 */
void path_ensure_separator(char* path, int max_len);

#endif

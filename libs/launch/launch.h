#ifndef LAUNCH_H
#define LAUNCH_H

/**
 * Launch module
 *
 * Provides NRO launching support using libnx applet launcher APIs.
 */

/**
 * launch_nro(path)
 * Launch an NRO (homebrew executable) file by path.
 * Returns 0 on success, -1 on failure.
 * Note: This will start a new process and may exit the current application.
 */
int launch_nro(const char* path);

#endif

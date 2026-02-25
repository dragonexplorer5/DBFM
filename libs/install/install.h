#ifndef INSTALL_H
#define INSTALL_H

/**
 * Install module
 *
 * Provides game package installation support for NSP/NSZ/XCI/XCZ files.
 */

/**
 * install_package(path)
 * Install a game package file (NSP/NSZ/XCI/XCZ) by path.
 * Returns 0 on success, -1 on failure or unsupported in current context.
 * 
 * Note: Package installation requires ES (Ticket) service permissions and
 * typically only works in contexts with sufficient privilege. Homebrew environments
 * have limited access to these services. A real implementation would need
 * integration with system installers or specialized tools.
 */
int install_package(const char* path);

#endif

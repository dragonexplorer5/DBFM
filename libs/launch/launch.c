#include "launch.h"
#include <switch.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Attempt to call hbloader's launch function if it's linked into the
// final binary. We declare it as a weak symbol so the code builds even
// when hbloader is not present. Common hbloader frontends expose a
// function similar to: int hbloaderLaunch(const char* path, int argc, char* const argv[]);
// Marking it weak avoids unresolved symbol errors when hbloader isn't
// linked; we check for NULL before calling.
int hbloaderLaunch(const char* path, int argc, char* const argv[]) __attribute__((weak));

/**
 * launch_nro(path)
 *
 * Launch an NRO file using hbloader when available. This implementation:
 * - Verifies the path argument
 * - Builds a minimal argv array containing the selected path
 * - Attempts to invoke hbloader's launch function (if present)
 * - Exits the current process cleanly when handoff succeeds
 *
 * Returns 0 on success (handed off to hbloader and exiting), or -1 on failure.
 */
int launch_nro(const char* path)
{
    if (path == NULL)
        return -1;

    // Ensure we're not running in a highly restricted applet context
    AppletType applet_type = appletGetAppletType();
    if (applet_type == AppletType_LibraryApplet || applet_type == AppletType_SystemApplet)
    {
        // Cannot safely launch NRO from restricted applet contexts
        return -1;
    }

    // If hbloader isn't linked into this build, we cannot proceed
    if (hbloaderLaunch == NULL) {
        return -1;
    }

    // Prepare argv: conventionally the launched homebrew expects argv[0]
    // to be the path to the executable. We pass a single argument (the path).
    char* argv[2];
    argv[0] = strdup(path);
    if (argv[0] == NULL) return -1;
    argv[1] = NULL;

    // Attempt to launch via hbloader. Many hbloader implementations take
    // ownership of the environment for the new process; we rely on that
    // behaviour and exit this app after a successful handoff.
    int rc = hbloaderLaunch(path, 1, (char* const*)argv);

    // free our duplicated argument
    free(argv[0]);

    if (rc == 0) {
        // Handoff succeeded. Perform minimal cleanup and exit so hbloader
        // can start the target homebrew with a clean process state.
        // Note: do not attempt to call into UI subsystems here; higher-level
        // callers should ensure they close any open resources as needed.
        // Call svcExitProcess to terminate immediately.
        svcExitProcess();
        // svcExitProcess does not return
    }

    return -1;
}

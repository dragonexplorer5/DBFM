#include "text.h"
#include <switch.h>
#include <stdio.h>

/**
 * Text Library Implementation
 * 
 * Uses libnx console functions to render text to the screen.
 * Provides a simple interface for drawing and updating the display.
 */

void text_init(void)
{
    // Initialize the console with default settings
    // Uses default internal console structure managed by libnx
    consoleInit(NULL);
}

void text_clear(void)
{
    // Clear screen using ANSI escape code
    printf("\x1b[2J");
    // Move cursor to top-left
    printf("\x1b[0;0H");
}

void text_draw(int x, int y, const char* msg)
{
    if (msg == NULL)
        return;

    // Move cursor to position using ANSI escape sequence
    // Format: \x1b[y;xH
    printf("\x1b[%d;%dH%s", y, x, msg);
}

void text_draw_formatted(int x, int y, const char* format, const char* msg)
{
    if (format == NULL || msg == NULL)
        return;

    // Move cursor to position
    printf("\x1b[%d;%dH", y, x);

    // Apply format (could be extended for colors, bold, etc.)
    // Example format codes could be: "bold", "inverse", etc.
    if (format[0] == 'i' || format[0] == 'I') {
        // Inverse video (highlight)
        printf("\x1b[7m");
    }

    printf("%s", msg);

    // Reset formatting
    printf("\x1b[0m");
}

void text_update(void)
{
    // Update the console display
    consoleUpdate(NULL);
}

void text_exit(void)
{
    // Clean up console
    consoleExit(NULL);
}

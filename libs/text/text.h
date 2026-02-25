#ifndef TEXT_H
#define TEXT_H

/**
 * Text Library
 * 
 * Provides simple text rendering functions using libnx console.
 * Wraps console initialization and text output with ANSI escape sequences.
 * 
 * Coordinate system: x=column (0-79), y=row (0-29)
 * Uses ANSI escape codes for cursor positioning and formatting.
 */

/**
 * text_init()
 * Initialize the text/console system. Call once at startup.
 * Uses default internal console structure managed by libnx.
 */
void text_init(void);

/**
 * text_clear()
 * Clear entire screen and move cursor to top-left (0,0).
 */
void text_clear(void);

/**
 * text_draw(x, y, msg)
 * Draw text at specific position.
 * x: column (0-79)
 * y: row (0-29)
 * msg: null-terminated string to display
 */
void text_draw(int x, int y, const char* msg);

/**
 * text_draw_formatted(x, y, format, msg)
 * Draw text at position with formatting.
 * format: "i" for inverse video (highlight), other values for plain text
 * Applied as ANSI escape codes.
 */
void text_draw_formatted(int x, int y, const char* format, const char* msg);

/**
 * text_update()
 * Refresh/update the console display. Call once per frame after drawing.
 */
void text_update(void);

/**
 * text_exit()
 * Clean up console resources. Call before application exit.
 */
void text_exit(void);

#endif

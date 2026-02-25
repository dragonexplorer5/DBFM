#ifndef INPUT_H
#define INPUT_H

#include <switch.h>

/**
 * Input Module
 * 
 * Provides a clean interface for handling Nintendo Switch controller input.
 * Wraps libnx PAD functions for easier use in the file browser.
 * 
 * All functions return newly pressed buttons only (not held buttons).
 * Call input_update() once per frame before checking button states.
 */

/**
 * input_init()
 * Initialize the input system with standard controller configuration.
 * Must be called once at application startup.
 */
void input_init(void);

/**
 * input_update()
 * Update the controller state. Call once per frame, before checking button inputs.
 */
void input_update(void);

/**
 * Directional input checks - return 1 if button newly pressed, 0 otherwise
 */
int input_up(void);
int input_down(void);
int input_left(void);
int input_right(void);

/**
 * Action button input checks - return 1 if button newly pressed, 0 otherwise
 */
int input_select(void);   // A button (confirm/select)
int input_back(void);     // B button (cancel/back)
int input_exit(void);     // Plus button (exit application)
int input_fileops(void);  // X button (open file operations overlay)

/**
 * input_power_pressed()
 * Check if Home button was newly pressed in this frame.
 * Used for safe exit in error conditions (applet mode).
 */
int input_power_pressed(void);

#endif

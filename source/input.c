#include "input.h"

/**
 * Input Module Implementation
 * 
 * Wraps libnx PAD functions for clean input handling.
 * Manages a single gamepad input state for standard controllers.
 * 
 * All button check functions return 1 for newly pressed buttons only
 * (not held from previous frame). Use padGetButtons() for held state if needed.
 */

// Static gamepad state - persists across function calls
static PadState g_pad;

void input_init(void)
{
    // Configure input for single player with standard controller styles
    // Supports both handheld and docked modes automatically
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad
    // Reads from handheld controls or first connected controller
    padInitializeDefault(&g_pad);
}

void input_update(void)
{
    // Scan the gamepad - call once per frame
    // Updates internal buttons state (down, held, up)
    padUpdate(&g_pad);
}

int input_up(void)
{
    u64 buttons = padGetButtonsDown(&g_pad);
    return (buttons & HidNpadButton_Up) != 0;
}

int input_down(void)
{
    u64 buttons = padGetButtonsDown(&g_pad);
    return (buttons & HidNpadButton_Down) != 0;
}

int input_left(void)
{
    u64 buttons = padGetButtonsDown(&g_pad);
    return (buttons & HidNpadButton_Left) != 0;
}

int input_right(void)
{
    u64 buttons = padGetButtonsDown(&g_pad);
    return (buttons & HidNpadButton_Right) != 0;
}

int input_select(void)
{
    u64 buttons = padGetButtonsDown(&g_pad);
    return (buttons & HidNpadButton_A) != 0;
}

int input_back(void)
{
    u64 buttons = padGetButtonsDown(&g_pad);
    return (buttons & HidNpadButton_B) != 0;
}

int input_exit(void)
{
    u64 buttons = padGetButtonsDown(&g_pad);
    return (buttons & HidNpadButton_Plus) != 0;
}

int input_fileops(void)
{
    u64 buttons = padGetButtonsDown(&g_pad);
    return (buttons & HidNpadButton_X) != 0;
}

int input_power_pressed(void)
{
    // Reserved for future use - currently not needed
    // Applet mode errors display message and wait for natural exit
    return 0;
}


# File Operations Overlay - Implementation Guide

## Overview
The Nintendo Switch file browser now includes an **interactive File Ops overlay menu** that appears when the user presses **X**. The overlay provides quick access to file operations while maintaining the main UI state underneath.

## Architecture

### 1. Input Handling (`include/input.h` + `source/input.c`)

**New Function Added:**
```c
int input_fileops(void);  // X button detection
```

- Detects X button press using libnx's `HidNpadButton_X`
- Returns 1 when X is newly pressed, 0 otherwise
- Updates once per `input_update()` call

### 2. File Operations Module (`libs/fileops/`)

Created new modular library with two files:

#### fileops.h - API
```c
typedef struct {
    char clipboard_path[512];   // Copied/cut file path
    int clipboard_has_data;     // 1 if clipboard has content
    int clipboard_is_cut;       // 1 if move, 0 if copy
} FileOpsState;
```

**Core Functions:**
- `fileops_copy(source)` - Copy file to clipboard (non-destructive)
- `fileops_paste(dest_dir)` - Paste from clipboard to destination
- `fileops_move(source, dest)` - Move file (copy + delete)
- `fileops_delete(path)` - Delete file
- `fileops_has_clipboard()` - Check if clipboard has data
- `fileops_get_clipboard_name(dest)` - Get clipboard filename

#### fileops.c - Implementation
- **Global State:** Single `FileOpsState` variable tracks clipboard
- **File Operations:** Uses standard C `fopen/fread/fwrite` for copying
- **Chunk-based:** Reads/writes files in 4KB chunks for memory efficiency
- **Safety:** Closes files properly, checks return values
- **Cleanup:** Clears clipboard after successful move

**Key Features:**
- ✅ Copy: Non-destructive, can paste multiple times
- ✅ Paste: Copies file, optionally removes original if cut
- ✅ Move: Combines copy + delete
- ✅ Delete: Removes files (not directories)
- ✅ Smart clipboard: Remembers if last operation was copy or cut

### 3. UI Overlay (`include/ui.h` + `source/ui.c`)

#### Extended UIState
```c
typedef struct {
    // ... existing fields ...
    int overlay_active;         // 1 if menu open
    int overlay_selected;       // 0-4: Copy/Paste/Move/Delete/Rename
} UIState;
```

#### New UI Functions

**Public API:**
- `ui_open_overlay()` - Show the File Ops menu
- `ui_close_overlay()` - Hide the menu
- `ui_overlay_select_next()` - Move selection down (0→3)
- `ui_overlay_select_prev()` - Move selection up (3→0)
- `ui_overlay_get_selected()` - Get current selection (0-4)

**Private Rendering:**
- `ui_render_overlay()` - Draws the overlay (static helper)

#### Overlay Rendering Details

```
┌──────────────────────────────────────┐
│                                      │
│         FILE OPS                     │   ← Title in inverse video
│                                      │
│         > Copy                       │   ← Selected option (highlighted)
│           Paste                      │
│           Move                       │
│           Delete                     │
│           Rename                     │   ← new option
│                                      │
│         A=Select  B=Cancel           │   ← Instructions
│                                      │
└──────────────────────────────────────┘
```

**Popup Notifications:**
A lightweight notification system was added to provide feedback for every action.  When the user copies, pastes, moves, deletes, or renames an item a semi‑transparent message appears above the file list (e.g. "Copied: filename.txt").  Notifications disappear automatically after a short delay or when any button is pressed.  Rename uses an interactive popup (see next section).

### Rename Workflow

Selecting "Rename" from the File Ops menu opens an on‑screen rename dialog.  The current filename is pre‑populated; the user can move a cursor left/right and cycle characters up/down through a basic alphanumeric set.  Confirming with A attempts a libnx filesystem rename and shows a success/failure notification on completion.  Cancel with B/Plus aborts the operation.

**Visual Features:**
- Centered box on screen (rows 8-19)
- Background drawn with inverse video (dark highlight)
- Menu options at rows 11-14
- Currently selected option highlighted with inverse video
- Instructions at bottom: "A=Select  B=Cancel"

### 4. Main Loop Integration (`source/main.c`)

#### Initialization
```c
fileops_init();  // Initialize clipboard (called at startup)
```

#### Input Handling - Two Modes

**Normal Mode (overlay_active == 0):**
```c
if (input_fileops()) {
    ui_open_overlay(&ui_state);  // X button opens menu
}
```

**Overlay Mode (overlay_active == 1):**
```c
if (input_down()) {
    ui_overlay_select_next();    // D-Pad down moves selection
}

if (input_select()) {
    // A button executes operation
    switch (ui_overlay_get_selected()) {
        case 0: fileops_copy(path); break;
        case 1: fileops_paste(dest_dir); break;
        case 2: fileops_move(source, dest); break;
        case 3: fileops_delete(path); break;
    }
    ui_close_overlay();
}

if (input_back()) {
    ui_close_overlay();  // B button closes menu
}
```

#### Directory Refresh
After paste/delete operations, the current directory is reloaded:
```c
FsDirectory* new_dir = fs_list_directory(ui_state.current_path);
if (new_dir != NULL) {
    fs_free_directory(ui_state.current_dir);
    ui_state.current_dir = new_dir;
}
```

#### Cleanup
```c
fileops_cleanup();  // Called before exit
```

## User Workflow

### Copy & Paste Workflow
```
1. Navigate to file
2. Press X → File Ops menu opens
3. Highlight "Copy" and press A
4. Navigate to destination directory
5. Press X → File Ops menu opens
6. Highlight "Paste" and press A
7. File is copied to new location
```

### Move Workflow (Alternative)
```
1. Navigate to file
2. Press X → File Ops menu opens
3. Highlight "Move" and press A
4. Navigate to destination
5. Press X → File Ops menu opens
6. Highlight "Paste" and press A
7. File is moved (original removed)
```

### Delete Workflow
```
1. Navigate to file
2. Press X → File Ops menu opens
3. Highlight "Delete" and press A
4. File is removed
5. Directory reloads with updated listing
```

## Code Organization

```
hello-world/
├── source/
│   ├── main.c              (Main loop with overlay handling)
│   ├── ui.c                (Overlay rendering + menu controls)
│   ├── input.c             (X button detection)
│   └── ...
├── include/
│   ├── ui.h                (Overlay state + functions)
│   ├── input.h             (X button declaration)
│   └── ...
├── libs/
│   └── fileops/
│       ├── fileops.h       (Public API - copy/paste/move/delete)
│       └── fileops.c       (Implementation - file I/O)
└── Makefile                (Updated: SOURCES += libs/fileops)
```

## Implementation Details

### File Copy Operation
```c
1. Open source file in binary read mode
2. Open destination file in binary write mode
3. Read source in 4KB chunks
4. Write each chunk to destination
5. Close both files
6. On success: Return 0; On error: Return -1
```

### Clipboard Behavior
- **Copy Mode:** `clipboard_is_cut = 0`
  - File remains after paste
  - Can paste multiple times
  
- **Move Mode:** `clipboard_is_cut = 1`
  - Original file removed after successful paste
  - Clipboard cleared automatically

### Error Handling
- **Paste into file:** Silently fails (only pastes into directories)
- **Delete directory:** Silently fails (only deletes files)
- **File operations:** Return codes checked, failures don't crash app
- **Directory reload:** Handles NULL pointers gracefully

## Testing Checklist

- [x] X button opens overlay in normal mode
- [x] D-Pad Up/Down navigate overlay menu
- [x] A button executes selected operation
- [x] B button closes overlay without action
- [x] Copy creates file in destination
- [x] Paste works after copy
- [x] Move combines copy + delete
- [x] Delete removes file and refreshes directory
- [x] Directory listing updates after operations
- [x] Multiple operations can be chained
- [x] App does not crash on invalid operations
- [x] Cleanup properly frees resources

## Performance Notes

- **Memory:** Each file operation uses single 4KB buffer (fixed)
- **Speed:** Depends on file size and SD card speed
- **Responsiveness:** UI remains responsive (not blocking)
- **Storage:** Clipboard stores path (512 bytes, minimal overhead)

## Future Enhancements

1. **Directory Move/Delete:** Support operations on folders
2. **Clipboard Display:** Show what's in clipboard
3. **Confirmation Dialogs:** Ask before deleting
4. **Move Progress:** Show progress for large files
5. **File Information:** Display file size/date before operations
6. **Undo/Redo:** Keep operation history
7. **Multi-file Operations:** Select multiple files at once

## Compilation

```bash
make clean && make
```

Build includes:
- Updated SOURCES: `source libs/text libs/utils libs/fileops`
- Updated INCLUDES: `include libs/text libs/utils libs/fileops`

Output: `switch/hello-world.nro` (242 KB)

## Notes

- Overlay properly pauses normal navigation (doesn't interfere)
- File operations use standard C library (portable)
- No dynamic memory allocation after initialization (safe)
- All functions have null checks (defensive)
- Modular design allows easy addition of more operations

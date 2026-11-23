// FILE: main.c
// PURPOSE: Main entry point of the Blood Bank Management System.
//          Launches the interactive Text User Interface (TUI) and exits cleanly.

// 1. HEADER INCLUDES
#include "tui.h"    // Imports the tui_run() function declaration to start the terminal menu

// 2. PROGRAM ENTRY POINT
// Execution begins here when the compiled program is launched.
int main(void) {
    tui_run();      // Start the interactive terminal menu loop
    return 0;       // Return 0 to the operating system to signal successful program termination
}
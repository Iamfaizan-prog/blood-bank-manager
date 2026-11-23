// FILE: tui.h
// PURPOSE: Public API for the Text User Interface (TUI).
//          Provides an interactive, menu-driven terminal experience for the user.

#ifndef TUI_H
#define TUI_H

// 1. TUI ENTRY POINT
// Launches the interactive visual menu loop in the terminal, guiding the user
// step-by-step through on-screen prompts instead of requiring upfront CLI flags.
//
// Parameters:
//   void - Takes no arguments (no argc/argv needed since input is gathered interactively).
//
// Returns:
//   void - Runs continuously in a menu loop until the user chooses to exit.
void tui_run(void);

#endif // End of TUI_H header guard
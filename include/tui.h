#ifndef TUI_H 

// --- Text User Interface (TUI) Blueprint ---
// This function launches an interactive, visual menu in the terminal.
// Instead of requiring the user to type complex commands upfront, 
// it will guide them step-by-step using on-screen prompts.
//
// Inputs: (void) - Takes no arguments. It doesn't need argc/argv because 
//                  it asks for user input AFTER the menu is already on screen.
// Output: void   - Returns nothing. The function runs in a loop until the 
//                  user chooses to exit the program.
void tui_run(void);

#endif 
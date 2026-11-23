#ifndef CLI_H 
#define CLI_H 
// --- Command Line Interface (CLI) Blueprint ---
// This function acts as the "waiter" for your program. It catches the raw 
// commands typed in the terminal, translates them, and tells the backend (core) what to do.
// We put this in its own file to keep the main() function clean and organized.
// Inputs:
// - int argc (Argument Count): Tells the program EXACTLY how many words the user typed.
// - char **argv (Argument Vector): An array holding the actual text strings the user typed.
//
// Output:
// - int: Returns 0 to the operating system if successful, or another number (like 1) if there was an error.
int cli_run(int argc, char **argv);
//So *argv is the first string (argv[0]), and **argv is the first character of that string ('.').
//We use argv to receive command-line input. It's a char ** because it's an array of strings of different lengths, 
// so each string is a char * and the array of them is a pointer to those."
#endif 

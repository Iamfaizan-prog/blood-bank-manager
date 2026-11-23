// FILE: cli.h
// PURPOSE: Public API for the Command Line Interface (CLI).
//          Acts as the entry point for parsing and executing terminal commands.

#ifndef CLI_H
#define CLI_H

// 1. CLI ENTRY POINT
// Runs the command-line interface using arguments passed from main().
//
// Parameters:
//   argc (Argument Count)  - Total number of words/arguments typed in the terminal (including the program name).
//   argv (Argument Vector) - Array of strings (char *) holding the actual text arguments typed by the user.
//                            *argv (argv[0]) points to the first string, while **argv is its first character.
//
// Returns:
//   0 if the command succeeded, or a non-zero error code (e.g., 1) if something went wrong.
int cli_run(int argc, char **argv);

#endif // End of CLI_H header guard
// FILE: cli.c
// PURPOSE: Implements the Command Line Interface (CLI) router and JSON output formatter.
//          Translates terminal arguments (argc/argv) into backend core API calls and
//          prints structured JSON responses so external scripts or UIs can parse results.

// 1. HEADER INCLUDES
#include <stdio.h>    // Provides standard output functions (printf, putchar, puts)
#include <stdlib.h>   // Provides string-to-number conversion tools (atoi, atof, strtoul)
#include <string.h>   // Provides string comparison and safe copying (strcmp, strncpy, memset)
#include <stdint.h>   // Provides fixed-width integer types (uint32_t)
#include "core.h"     // Connects this CLI to backend database functions, structs, and constants

// 2. JSON FORMATTING & ERROR HELPERS

// Safely prints a C string formatted as valid JSON text by escaping quotes and backslashes.
// Prevents special characters inside names or addresses from breaking the JSON structure.
static void json_string(const char *s) {
    // Point to the string (or "" if NULL); unsigned char prevents negative ASCII sign bugs
    const unsigned char *p = (const unsigned char *)(s ? s : "");

    putchar('"');                                           // Print opening JSON double quote
    while (*p) {                                            // Loop character-by-character until null terminator ('\0')
        if (*p == '"' || *p == '\\') {
            putchar('\\');                                  // Escape double quotes (") and backslashes (\) with a backslash
            putchar(*p);
        } else if (*p < 32) {
            printf(" ");                                    // Replace invisible control characters (newlines, tabs, ASCII < 32) with spaces
        } else {
            putchar(*p);                                    // Print normal characters directly
        }
        ++p;                                                // Advance pointer to the next character
    }
    putchar('"');                                           // Print closing JSON double quote
}

// Formats and prints a single Donor struct as a valid JSON object (e.g., {"id":1001,"name":"Ali",...}).
static void print_donor(const Donor *d) {
    char gender_str[2] = {d->gender, '\0'};                 // Convert single char 'gender' into a 2-byte string for json_string()

    printf("{\"id\":%u,\"name\":", d->id);
    json_string(d->name);

    printf(",\"age\":%d,\"gender\":", d->age);
    json_string(gender_str);

    printf(",\"weight\":%.1f,\"blood_group\":", d->weight);
    json_string(d->blood_group);

    printf(",\"phone\":");
    json_string(d->phone);

    // Print boolean as JSON true/false and nest last_donation fields inside its own JSON object
    printf(",\"has_donated_before\":%s,\"last_donation\":{\"day\":%d,\"month\":%d,\"year\":%d}}",
           d->has_donated_before ? "true" : "false",
           d->last_donation.day,
           d->last_donation.month,
           d->last_donation.year);
}

// Prints a standardized JSON error payload and returns exit code 1 to indicate failure.
static int fail(const char *message) {
    printf("{\"ok\":false,\"error\":");
    json_string(message);
    puts("}");                                              // Print closing brace followed by newline
    return 1;                                               // Return non-zero status code to the OS/caller
}

// 3. MAIN CLI COMMAND ROUTER

// Parses command-line arguments (argc/argv) and executes the matching backend operation.
int cli_run(int argc, char **argv) {
    BloodInventory inv;                                     // Holds loaded blood bank inventory data
    Donor d;                                                // Temporary Donor struct for add/update/get operations
    char *end;                                              // Pointer used by strtoul() to detect invalid trailing characters
    unsigned long number;                                   // Temporary variable for parsed numeric arguments

    // Step 1: Initialize backend files and load saved inventory before processing any command
    if (!core_init(&inv)) return fail(core_last_error);

    // Step 2: Verify the user supplied at least one command name after the executable name
    if (argc < 2) return fail("Missing command");

    // COMMAND 1: "list" -> Fetch all donor records and print them as a JSON array
    if (strcmp(argv[1], "list") == 0) {
        Donor records[10000];
        size_t count = 0, i;

        if (!core_get_all_donors(records, 10000, &count)) return fail(core_last_error);

        printf("{\"ok\":true,\"donors\":[");
        for (i = 0; i < count; ++i) {
            if (i > 0) putchar(',');                        // Print a comma separator before every item except the first
            print_donor(&records[i]);
        }
        puts("]}");
        return 0;                                           // Return 0 to signal success
    }

    // COMMAND 2: "get <id>" -> Fetch a single donor by ID (expects 3 words: [program, "get", id])
    if (strcmp(argv[1], "get") == 0 && argc == 3) {
        number = strtoul(argv[2], &end, 10);                // Convert ID string to base-10 unsigned integer
        if (*end != '\0') return fail("Invalid ID");        // Reject if non-numeric characters remain (e.g., "1001abc")
        if (!core_get_donor((uint32_t)number, &d)) return fail(core_last_error);

        printf("{\"ok\":true,\"donor\":");
        print_donor(&d);
        puts("}");
        return 0;
    }

    // COMMAND 3: "delete <id>" -> Remove a donor by ID (expects 3 words: [program, "delete", id])
    if (strcmp(argv[1], "delete") == 0 && argc == 3) {
        number = strtoul(argv[2], &end, 10);
        if (*end != '\0') return fail("Invalid ID");
        if (!core_delete_donor((uint32_t)number)) return fail(core_last_error);

        puts("{\"ok\":true}");
        return 0;
    }

    // COMMAND 4: "add <name> <age> <gender> <weight> <group> <phone> <donated> <day> <month> <year>"
    // Expects 12 words total (program + "add" + 10 donor fields)
    if (strcmp(argv[1], "add") == 0 && argc == 12) {
        memset(&d, 0, sizeof(d));                           // Zero out struct memory to clear leftover RAM garbage

        strncpy(d.name, argv[2], sizeof(d.name) - 1);       // Copy name safely (-1 preserves space for '\0')
        d.age    = atoi(argv[3]);                           // Convert age string to integer
        d.gender = argv[4][0];                              // Extract first character of gender string
        d.weight = atof(argv[5]);                           // Convert weight string to floating-point double
        strncpy(d.blood_group, argv[6], sizeof(d.blood_group) - 1);
        strncpy(d.phone, argv[7], sizeof(d.phone) - 1);
        d.has_donated_before = (atoi(argv[8]) != 0);        // "1" becomes true, "0" becomes false
        d.last_donation.day   = atoi(argv[9]);
        d.last_donation.month = atoi(argv[10]);
        d.last_donation.year  = atoi(argv[11]);

        if (!core_add_donor(&d)) return fail(core_last_error);
        puts("{\"ok\":true}");
        return 0;
    }

    // COMMAND 5: "update <id> <name> <age> <gender> <weight> <group> <phone> <donated> <day> <month> <year>"
    // Expects 13 words total (same as "add", with existing donor ID inserted at argv[2])
    if (strcmp(argv[1], "update") == 0 && argc == 13) {
        memset(&d, 0, sizeof(d));                           // Zero out struct memory before populating fields

        d.id     = (uint32_t)strtoul(argv[2], NULL, 10);    // Parse existing donor ID
        strncpy(d.name, argv[3], sizeof(d.name) - 1);
        d.age    = atoi(argv[4]);
        d.gender = argv[5][0];
        d.weight = atof(argv[6]);
        strncpy(d.blood_group, argv[7], sizeof(d.blood_group) - 1);
        strncpy(d.phone, argv[8], sizeof(d.phone) - 1);
        d.has_donated_before = (atoi(argv[9]) != 0);
        d.last_donation.day   = atoi(argv[10]);
        d.last_donation.month = atoi(argv[11]);
        d.last_donation.year  = atoi(argv[12]);

        if (!core_update_donor(&d)) return fail(core_last_error);
        puts("{\"ok\":true}");
        return 0;
    }

    // COMMAND 6: "inventory" -> Print total blood units and breakdown across all 8 blood groups
    if (strcmp(argv[1], "inventory") == 0) {
        int i;

        printf("{\"ok\":true,\"total_units\":%u,\"stocks\":[", inv.total_units);
        for (i = 0; i < NUM_BLOOD_GROUPS; ++i) {
            if (i > 0) putchar(',');                        // Separate JSON objects with commas
            printf("{\"blood_group\":\"%s\",\"units\":%u}",
                   inv.stocks[i].blood_group,
                   inv.stocks[i].units);
        }
        puts("]}");
        return 0;
    }

    // COMMAND 7: "stock-add <group> <units>" OR "stock-deduct <group> <units>" (expects 4 words total)
    if ((strcmp(argv[1], "stock-add") == 0 || strcmp(argv[1], "stock-deduct") == 0) && argc == 4) {
        number = strtoul(argv[3], &end, 10);                // Parse unit quantity as unsigned integer
        if (*end != '\0' || number == 0) return fail("Invalid quantity");

        // Call core_add_units for "stock-add", or core_deduct_units for "stock-deduct"
        bool ok = (strcmp(argv[1], "stock-add") == 0)
                  ? core_add_units(&inv, argv[2], (uint32_t)number)
                  : core_deduct_units(&inv, argv[2], (uint32_t)number);

        if (!ok) return fail(core_last_error);
        puts("{\"ok\":true}");
        return 0;
    }

    // Fallback: Reject unrecognized commands or mismatched argument counts
    return fail("Unknown command or incorrect arguments");
}

// 4. CLI EXECUTABLE ENTRY POINT

// Entry point when compiled as standalone CLI binary (bb_cli.exe): forwards terminal arguments to cli_run().
int main(int argc, char **argv) {
    return cli_run(argc, argv);
}
#include <stdio.h>   // Standard input/output functions (printf, putchar, puts)
#include <stdlib.h>  // Number conversion tools (atoi, atof, strtoul)
#include <string.h>  // String tools (strcmp for comparing words, strncpy for safe copying)
#include <stdint.h>  // Fixed-size integer types like uint32_t
#include "core.h"    // Connects this CLI to the backend database functions and structs[cite: 1]

// Safely prints a C string formatted as valid JSON text (escapes quotes and backslashes)
static void json_string(const char *s) {
    // Point to the string (or "" if NULL); unsigned char prevents negative ASCII bugs
    const unsigned char *p = (const unsigned char *)(s ? s : "");
    putchar('"'); // Print opening double quote
    while (*p) { // Loop character by character until hitting '\0' (end of string)
        // If text has a quote (") or backslash (\), print a backslash first so JSON doesn't break
        if (*p == '"' || *p == '\\') { putchar('\\'); putchar(*p); }
        // Replace invisible control characters (like Enter/newlines, ASCII < 32) with a normal space
        else if (*p < 32) printf(" ");
        // Otherwise, print the character normally
        else putchar(*p);
        ++p; // Advance pointer to the next character
    }
    putchar('"'); // Print closing double quote
}

// Formats and prints one Donor struct as a JSON object (e.g., {"id":1001,"name":"Ali",...})
static void print_donor(const Donor *d) {
    printf("{\"id\":%u,\"name\":", d->id); json_string(d->name);
    // Turn single char 'gender' into a tiny 2-byte string so json_string can wrap it in quotes
    printf(",\"age\":%d,\"gender\":", d->age); { char g[2] = {d->gender, 0}; json_string(g); }
    printf(",\"weight\":%.1f,\"blood_group\":", d->weight); json_string(d->blood_group);
    printf(",\"phone\":"); json_string(d->phone);
    // Print boolean as JSON "true"/"false" and nest the last_donation date fields inside braces
    printf(",\"has_donated_before\":%s,\"last_donation\":{\"day\":%d,\"month\":%d,\"year\":%d}}",
           d->has_donated_before ? "true" : "false", d->last_donation.day, d->last_donation.month, d->last_donation.year);
}

// Prints a standardized JSON error message and returns 1 (tells OS/Python that command failed)
static int fail(const char *message) {
    printf("{\"ok\":false,\"error\":");
    json_string(message);
    puts("}"); // Print closing brace + newline
    return 1;
}

// Main CLI router: reads terminal words (argc/argv) and executes the matching backend command
int cli_run(int argc, char **argv) {
    BloodInventory inv; Donor d; char *end; unsigned long number;
    
    // Start up backend and load saved inventory.dat before running any command[cite: 1]
    if (!core_init(&inv)) return fail(core_last_error);
    
    // Safety check: make sure user typed at least 1 command after the program name
    if (argc < 2) return fail("Missing command");

    // COMMAND 1: "list" -> Fetch all donors and print them as a JSON list
    if (strcmp(argv[1], "list") == 0) {
        Donor records[10000]; size_t count = 0, i;
        if (!core_get_all_donors(records, 10000, &count)) return fail(core_last_error);
        printf("{\"ok\":true,\"donors\":[");
        for (i = 0; i < count; ++i) {
            if (i) putchar(','); // Print a comma BEFORE every donor except the very first one (i == 0)
            print_donor(&records[i]);
        }
        puts("]}"); return 0; // Return 0 to signal success
    }

    // COMMAND 2: "get <id>" -> Requires exactly 3 words: [program, "get", id]
    if (strcmp(argv[1], "get") == 0 && argc == 3) {
        // strtoul converts text ID to base-10 number; 'end' points to any leftover non-digit letters
        number = strtoul(argv[2], &end, 10);
        if (*end) return fail("Invalid ID"); // Reject if user typed letters like "1001abc"
        if (!core_get_donor((uint32_t)number, &d)) return fail(core_last_error);
        printf("{\"ok\":true,\"donor\":"); print_donor(&d); puts("}"); return 0;
    }

    // COMMAND 3: "delete <id>" -> Requires exactly 3 words: [program, "delete", id]
    if (strcmp(argv[1], "delete") == 0 && argc == 3) {
        number = strtoul(argv[2], &end, 10);
        if (*end) return fail("Invalid ID");
        if (!core_delete_donor((uint32_t)number)) return fail(core_last_error);
        puts("{\"ok\":true}"); return 0;
    }

    // COMMAND 4: "add" -> Expects 12 words total (program + "add" + 10 donor fields)
    if (strcmp(argv[1], "add") == 0 && argc == 12) {
        memset(&d, 0, sizeof(d)); // Wipe struct memory with 0s to clear leftover RAM garbage
        // strncpy safely copies text without overflowing array bounds (-1 keeps space for '\0')
        strncpy(d.name, argv[2], sizeof(d.name)-1);
        d.age = atoi(argv[3]);      // Convert age text to integer
        d.gender = argv[4][0];      // Take only the first character of gender
        d.weight = atof(argv[5]);   // Convert weight text to decimal (double)
        strncpy(d.blood_group, argv[6], sizeof(d.blood_group)-1);
        strncpy(d.phone, argv[7], sizeof(d.phone)-1);
        d.has_donated_before = atoi(argv[8]) != 0; // "1" becomes true, "0" becomes false
        d.last_donation.day = atoi(argv[9]);
        d.last_donation.month = atoi(argv[10]);
        d.last_donation.year = atoi(argv[11]);
        if (!core_add_donor(&d)) return fail(core_last_error);
        puts("{\"ok\":true}"); return 0;
    }

    // COMMAND 5: "update" -> Expects 13 words total (same as "add", plus existing ID in argv[2])
    if (strcmp(argv[1], "update") == 0 && argc == 13) {
        memset(&d, 0, sizeof(d));
        d.id = (uint32_t)strtoul(argv[2], NULL, 10); // Convert existing ID text to uint32_t
        strncpy(d.name, argv[3], sizeof(d.name)-1);
        d.age = atoi(argv[4]); d.gender = argv[5][0]; d.weight = atof(argv[6]);
        strncpy(d.blood_group, argv[7], sizeof(d.blood_group)-1);
        strncpy(d.phone, argv[8], sizeof(d.phone)-1);
        d.has_donated_before = atoi(argv[9]) != 0;
        d.last_donation.day = atoi(argv[10]); d.last_donation.month = atoi(argv[11]); d.last_donation.year = atoi(argv[12]);
        if (!core_update_donor(&d)) return fail(core_last_error);
        puts("{\"ok\":true}"); return 0;
    }

    // COMMAND 6: "inventory" -> Prints total units and all 8 blood groups as a JSON array
    if (strcmp(argv[1], "inventory") == 0) {
        int i; printf("{\"ok\":true,\"total_units\":%u,\"stocks\":[", inv.total_units);
        for (i = 0; i < NUM_BLOOD_GROUPS; ++i) {
            if (i) putchar(','); // Add comma between JSON objects
            printf("{\"blood_group\":\"%s\",\"units\":%u}", inv.stocks[i].blood_group, inv.stocks[i].units);
        }
        puts("]}"); return 0;
    }

    // COMMAND 7: "stock-add <group> <units>" OR "stock-deduct <group> <units>" (4 words total)
    if ((strcmp(argv[1], "stock-add") == 0 || strcmp(argv[1], "stock-deduct") == 0) && argc == 4) {
        number = strtoul(argv[3], &end, 10);
        if (*end || number == 0) return fail("Invalid quantity"); // Must be a clean number > 0
        // Ternary operator (? :) calls core_add_units for "stock-add", or core_deduct_units for "stock-deduct"
        if (strcmp(argv[1], "stock-add") == 0 ? !core_add_units(&inv, argv[2], (uint32_t)number) : !core_deduct_units(&inv, argv[2], (uint32_t)number)) {
            return fail(core_last_error);
        }
        puts("{\"ok\":true}"); return 0;
    }

    // Fallback error if user typed an unknown command or wrong number of arguments
    return fail("Unknown command or incorrect arguments");
}

// Entry point for bb_cli.exe: forwards terminal arguments directly to cli_run[cite: 1]
int main(int argc, char **argv) { return cli_run(argc, argv); }
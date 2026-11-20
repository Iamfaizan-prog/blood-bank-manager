#include <stdio.h>  
#include <stdlib.h>  // Standard library utilities
#include <string.h>  // String helpers like memset and strcspn
#include <stdint.h>  // Fixed-width integers like uint32_t
#include "core.h"    // Connects this UI to the backend database & rules

// Safely reads a line of text from the keyboard without overflowing memory
static void read_line(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt); // Display prompt to user
    // Read up to 'size' characters from keyboard (stdin); if it fails, empty the string
    if (!fgets(buffer, (int)size, stdin)) { buffer[0] = '\0'; return; }
    buffer[strcspn(buffer, "\r\n")] = '\0'; // strcspn find the Enter key (\r or \n) at the end and replace it with '\0' (end of string)
}

// Keeps asking until the user enters a valid whole number
static int read_int(const char *prompt) {
    char line[64], extra; int value;
    for (;;) { // Infinite loop until valid input is returned
        read_line(prompt, line, sizeof(line));
        // sscanf returns 1 ONLY if it reads a clean integer and NO extra trailing characters
        if (sscanf(line, "%d %c", &value, &extra) == 1) return value;
        puts("Please enter a whole number.");
    }
}

// Keeps asking until the user enters a valid decimal number (e.g., weight)
static double read_double(const char *prompt) {
    char line[64], extra; double value;
    for (;;) {
        read_line(prompt, line, sizeof(line));
        // %lf reads a double; checks that no leftover text exists after the number
        if (sscanf(line, "%lf %c", &value, &extra) == 1) return value;
        puts("Please enter a number.");
    }
}

// Fetches all donors from storage and prints them as an aligned table
static void show_donors(void) {
    Donor donors[1000]; // Temporary array in RAM capable of holding up to 1000 donors
    size_t count = 0, i;
    // Ask backend to fill the array and update 'count' with exact number found
    if (!core_get_all_donors(donors, 1000, &count)) { printf("Error: %s\n", core_last_error); return; }
    // Print table headers; '-' left-aligns each column to a fixed width
    printf("\n%-6s %-24s %-4s %-5s %-7s %-5s %-16s\n", "ID", "NAME", "AGE", "SEX", "WEIGHT", "GROUP", "PHONE");
    // Loop through every donor and print formatted columns (.24 truncates overly long names)
    for (i = 0; i < count; ++i) printf("%-6u %-24.24s %-4d %-5c %-7.1f %-5s %-16.16s\n", donors[i].id, donors[i].name, donors[i].age, donors[i].gender, donors[i].weight, donors[i].blood_group, donors[i].phone);
    printf("%lu donor(s)\n", (unsigned long)count);
}

// Collects new donor details from user and passes them to backend
static void add_donor(void) {
    Donor d; char temp[32];
    memset(&d, 0, sizeof(d)); // Wipe the entire Donor struct with 0s to clear garbage RAM
    read_line("Full name: ", d.name, sizeof(d.name));
    d.age = read_int("Age: ");
    read_line("Gender (M/F/O): ", temp, sizeof(temp)); d.gender = temp[0]; // Grab just the first letter
    d.weight = read_double("Weight in kg: ");
    read_line("Blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ", d.blood_group, sizeof(d.blood_group));
    read_line("Phone: ", d.phone, sizeof(d.phone));
    read_line("Donated before? (y/n): ", temp, sizeof(temp));
    // Convert 'y' or 'Y' into boolean true, anything else into false
    d.has_donated_before = (temp[0] == 'y' || temp[0] == 'Y');
    // Only ask for last donation date if they have donated before
    if (d.has_donated_before) { d.last_donation.day = read_int("Last donation day: "); d.last_donation.month = read_int("Last donation month: "); d.last_donation.year = read_int("Last donation year: "); }
    // Send struct address (&d) to backend for validation and saving
    if (core_add_donor(&d)) puts("Donor added successfully."); else printf("Could not add donor: %s\n", core_last_error);
}

// Looks up a single donor by ID and prints their full profile
static void search_donor(void) {
    Donor d;
    uint32_t id = (uint32_t)read_int("Donor ID: "); // Cast int input to unsigned 32-bit ID
    if (!core_get_donor(id, &d)) { printf("Not found: %s\n", core_last_error); return; }
    printf("ID: %u\nName: %s\nAge: %d\nGender: %c\nWeight: %.1f\nBlood group: %s\nPhone: %s\n", d.id, d.name, d.age, d.gender, d.weight, d.blood_group, d.phone);
}

// Deletes a donor record using their ID
static void delete_donor(void) {
    uint32_t id = (uint32_t)read_int("Donor ID to delete: ");
    if (core_delete_donor(id)) puts("Donor deleted."); else printf("Delete failed: %s\n", core_last_error);
}

// Loads an existing donor, lets user overwrite fields, and saves changes
static void update_donor(void) {
    Donor d; char temp[32];
    uint32_t id = (uint32_t)read_int("Donor ID to update: ");
    // First check if the donor exists and load their current data into 'd'
    if (!core_get_donor(id, &d)) { puts("Donor not found."); return; }
    printf("Current name: %s\n", d.name);
    read_line("New name: ", d.name, sizeof(d.name));
    d.age = read_int("New age: "); d.weight = read_double("New weight: ");
    read_line("New phone: ", d.phone, sizeof(d.phone));
    read_line("New blood group: ", d.blood_group, sizeof(d.blood_group));
    read_line("Has donated before? (y/n): ", temp, sizeof(temp)); d.has_donated_before = (temp[0] == 'y' || temp[0] == 'Y');
    if (d.has_donated_before) { d.last_donation.day = read_int("Last donation day: "); d.last_donation.month = read_int("Last donation month: "); d.last_donation.year = read_int("Last donation year: "); }
    // If they changed answer to 'no', zero out the old date struct
    else memset(&d.last_donation, 0, sizeof(d.last_donation));
    if (core_update_donor(&d)) puts("Donor updated."); else printf("Update failed: %s\n", core_last_error);
}

// Displays current blood bag stock and allows adding or removing units
static void inventory_menu(BloodInventory *inv) {
    int i, choice; char group[8]; uint32_t units;
    puts("\nBlood inventory:");
    // Loop through all 8 blood groups using '->' because 'inv' is a pointer
    for (i = 0; i < NUM_BLOOD_GROUPS; ++i) printf("  %-3s : %u\n", inv->stocks[i].blood_group, inv->stocks[i].units);
    printf("Total units: %u\n", inv->total_units);
    puts("1. Restock  2. Dispense  0. Back"); choice = read_int("Choice: ");
    if (choice == 1 || choice == 2) {
        read_line("Blood group: ", group, sizeof(group)); units = (uint32_t)read_int("Units: ");
        // Ternary (? :) runs core_add_units if choice is 1, otherwise runs core_deduct_units
        if (choice == 1 ? core_add_units(inv, group, units) : core_deduct_units(inv, group, units)) puts("Inventory updated.");
        else printf("Operation failed: %s\n", core_last_error);
    }
}

// Main entry point for the interactive terminal menu (declared in tui.h)
void tui_run(void) {
    BloodInventory inventory; int choice;
    // Boot up the backend and load saved inventory from disk before showing menu
    if (!core_init(&inventory)) { printf("Startup error: %s\n", core_last_error); return; }
    do { // Run menu at least once, repeating until user enters 0
        puts("\n========== BLOOD BANK MANAGEMENT ==========");
        puts("1. Add donor\n2. Display all donors\n3. Search donor by ID\n4. Update donor\n5. Delete donor\n6. Blood inventory\n0. Exit");
        choice = read_int("Select: ");
        switch (choice) { // Route user's numeric selection to the matching helper function
            case 1: add_donor(); break;
            case 2: show_donors(); break;
            case 3: search_donor(); break;
            case 4: update_donor(); break;
            case 5: delete_donor(); break;
            case 6: inventory_menu(&inventory); break;
            case 0: puts("Goodbye."); break;
            default: puts("Invalid choice.");
        }
    } while (choice != 0);
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "core.h"

static void read_line(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (!fgets(buffer, (int)size, stdin)) { buffer[0] = '\0'; return; }
    buffer[strcspn(buffer, "\r\n")] = '\0';
}
static int read_int(const char *prompt) {
    char line[64], extra; int value;
    for (;;) { read_line(prompt, line, sizeof(line)); if (sscanf(line, "%d %c", &value, &extra) == 1) return value; puts("Please enter a whole number."); }
}
static double read_double(const char *prompt) {
    char line[64], extra; double value;
    for (;;) { read_line(prompt, line, sizeof(line)); if (sscanf(line, "%lf %c", &value, &extra) == 1) return value; puts("Please enter a number."); }
}
static void show_donors(void) {
    Donor donors[1000]; size_t count = 0, i;
    if (!core_get_all_donors(donors, 1000, &count)) { printf("Error: %s\n", core_last_error); return; }
    printf("\n%-6s %-24s %-4s %-5s %-7s %-5s %-16s\n", "ID", "NAME", "AGE", "SEX", "WEIGHT", "GROUP", "PHONE");
    for (i = 0; i < count; ++i) printf("%-6u %-24.24s %-4d %-5c %-7.1f %-5s %-16.16s\n", donors[i].id, donors[i].name, donors[i].age, donors[i].gender, donors[i].weight, donors[i].blood_group, donors[i].phone);
    printf("%lu donor(s)\n", (unsigned long)count);
}
static void add_donor(void) {
    Donor d; char temp[32];
    memset(&d, 0, sizeof(d));
    read_line("Full name: ", d.name, sizeof(d.name));
    d.age = read_int("Age: ");
    read_line("Gender (M/F/O): ", temp, sizeof(temp)); d.gender = temp[0];
    d.weight = read_double("Weight in kg: ");
    read_line("Blood group (A+, A-, B+, B-, AB+, AB-, O+, O-): ", d.blood_group, sizeof(d.blood_group));
    read_line("Phone: ", d.phone, sizeof(d.phone));
    read_line("Donated before? (y/n): ", temp, sizeof(temp));
    d.has_donated_before = (temp[0] == 'y' || temp[0] == 'Y');
    if (d.has_donated_before) { d.last_donation.day = read_int("Last donation day: "); d.last_donation.month = read_int("Last donation month: "); d.last_donation.year = read_int("Last donation year: "); }
    if (core_add_donor(&d)) puts("Donor added successfully."); else printf("Could not add donor: %s\n", core_last_error);
}
static void search_donor(void) {
    Donor d; uint32_t id = (uint32_t)read_int("Donor ID: ");
    if (!core_get_donor(id, &d)) { printf("Not found: %s\n", core_last_error); return; }
    printf("ID: %u\nName: %s\nAge: %d\nGender: %c\nWeight: %.1f\nBlood group: %s\nPhone: %s\n", d.id, d.name, d.age, d.gender, d.weight, d.blood_group, d.phone);
}
static void delete_donor(void) {
    uint32_t id = (uint32_t)read_int("Donor ID to delete: ");
    if (core_delete_donor(id)) puts("Donor deleted."); else printf("Delete failed: %s\n", core_last_error);
}
static void update_donor(void) {
    Donor d; char temp[32]; uint32_t id = (uint32_t)read_int("Donor ID to update: ");
    if (!core_get_donor(id, &d)) { puts("Donor not found."); return; }
    printf("Current name: %s\n", d.name);
    read_line("New name: ", d.name, sizeof(d.name));
    d.age = read_int("New age: "); d.weight = read_double("New weight: ");
    read_line("New phone: ", d.phone, sizeof(d.phone));
    read_line("New blood group: ", d.blood_group, sizeof(d.blood_group));
    read_line("Has donated before? (y/n): ", temp, sizeof(temp)); d.has_donated_before = (temp[0] == 'y' || temp[0] == 'Y');
    if (d.has_donated_before) { d.last_donation.day = read_int("Last donation day: "); d.last_donation.month = read_int("Last donation month: "); d.last_donation.year = read_int("Last donation year: "); }
    else memset(&d.last_donation, 0, sizeof(d.last_donation));
    if (core_update_donor(&d)) puts("Donor updated."); else printf("Update failed: %s\n", core_last_error);
}
static void inventory_menu(BloodInventory *inv) {
    int i, choice; char group[8]; uint32_t units;
    puts("\nBlood inventory:");
    for (i = 0; i < NUM_BLOOD_GROUPS; ++i) printf("  %-3s : %u\n", inv->stocks[i].blood_group, inv->stocks[i].units);
    printf("Total units: %u\n", inv->total_units);
    puts("1. Restock  2. Dispense  0. Back"); choice = read_int("Choice: ");
    if (choice == 1 || choice == 2) {
        read_line("Blood group: ", group, sizeof(group)); units = (uint32_t)read_int("Units: ");
        if (choice == 1 ? core_add_units(inv, group, units) : core_deduct_units(inv, group, units)) puts("Inventory updated.");
        else printf("Operation failed: %s\n", core_last_error);
    }
}
void tui_run(void) {
    BloodInventory inventory; int choice;
    if (!core_init(&inventory)) { printf("Startup error: %s\n", core_last_error); return; }
    do {
        puts("\n========== BLOOD BANK MANAGEMENT ==========");
        puts("1. Add donor\n2. Display all donors\n3. Search donor by ID\n4. Update donor\n5. Delete donor\n6. Blood inventory\n0. Exit");
        choice = read_int("Select: ");
        switch (choice) {
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

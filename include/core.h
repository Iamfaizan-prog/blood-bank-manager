#ifndef CORE_H // If CORE_H is not defined yet...
#define CORE_H // ...define it now prevents double including

#include "models.h" // Blueprints for custom types like BloodInventory and Donor
#include <stddef.h> // Standard C library providing size_t for memory sizes

extern char core_last_error[256]; // Global string to store the latest error message

// --- System & Utilities ---
bool core_init(BloodInventory *inventory); //its job is to prepare the main_inventory so the rest of the program doesn't crash.
int core_find_blood_group(const char *group); // Returns the system index for a group (e.g., "O+")
void core_get_today(Date *today); // Fetches current date and stores it in 'today'

// --- Eligibility & Lookup ---
bool core_check_eligibility(int age, double weight, bool has_donated, const Date *last, const Date *today); // Validates medical/legal rules to donate
bool core_get_all_donors(Donor *records, size_t capacity, size_t *count); // Gets donor list up to capacity updates exact count
bool core_get_donor(uint32_t id, Donor *result); // Finds one donor by ID saves data into result

// --- Database Operations (CRUD) ---
bool core_add_donor(const Donor *donor); // Registers a new donor
bool core_update_donor(const Donor *donor); // Updates an existing donor's details
bool core_delete_donor(uint32_t id); // Removes a donor using their ID

// --- Inventory Management ---
bool core_add_units(BloodInventory *inventory, const char *group, uint32_t units); // Adds donated blood units to a specific group
bool core_deduct_units(BloodInventory *inventory, const char *group, uint32_t units); // Removes blood units for hospital requests

// --- Data Persistence ---
bool core_save_inventory(const BloodInventory *inventory); // Writes current memory data to a file on the hard drive
bool core_load_inventory(BloodInventory *inventory); // Reads saved file from hard drive back into memory

#endif // End of CORE_H include guard

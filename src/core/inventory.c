// FILE: core_inventory.c (or blood inventory module)
// PURPOSE: Manages adding and deducting blood units in memory (RAM) and ensures
//          changes are safely persisted to disk with automatic rollback on failure.

// 1. HEADER INCLUDES
#include <stdio.h>    // Provides snprintf() for formatting error messages
#include "core.h"     // Imports BloodInventory struct, function declarations, and error buffer

// 2. ADD BLOOD UNITS TO INVENTORY

// Adds newly donated blood units to the matching blood group and saves the updated inventory to disk.
// Includes a rollback mechanism so RAM never goes out of sync with the saved file if disk write fails.
bool core_add_units(BloodInventory *inventory, const char *group, uint32_t units) {
    int index;                                              // Holds the array index (0-7) of the target blood group

    // Step 1: Validate that inventory pointer exists, units > 0, and blood group string is valid
    if (!inventory || units == 0 || (index = core_find_blood_group(group)) < 0) {
        snprintf(core_last_error, sizeof(core_last_error), "Invalid blood group or quantity");
        return false;                                       // Reject invalid input immediately
    }

    // Step 2: Update the stock counts in temporary memory (RAM)
    inventory->stocks[index].units += units;                // Increase units for the specific blood group (e.g., "A+")
    inventory->total_units += units;                        // Increase the overall total blood units in the bank

    // Step 3: Save the updated inventory permanently to the hard drive
    if (!core_save_inventory(inventory)) {
        // FAIL-SAFE (Rollback): If saving to disk fails, undo the changes made in RAM
        // so that in-memory data stays consistent with what is stored on disk.
        inventory->stocks[index].units -= units;            // Revert the specific blood group addition
        inventory->total_units -= units;                    // Revert the grand total addition
        return false;                                       // Report failure to the caller
    }

    return true;                                            // Successfully added in RAM and saved to disk
}

// 3. DEDUCT BLOOD UNITS FROM INVENTORY

// Removes blood units from a specific group (e.g., when fulfilling a hospital request) and saves to disk.
// Includes stock availability checks and automatic rollback if saving fails.
bool core_deduct_units(BloodInventory *inventory, const char *group, uint32_t units) {
    int index;                                              // Holds the array index (0-7) of the target blood group

    // Step 1: Validate that inventory pointer exists, units > 0, and blood group string is valid
    if (!inventory || units == 0 || (index = core_find_blood_group(group)) < 0) {
        snprintf(core_last_error, sizeof(core_last_error), "Invalid blood group or quantity");
        return false;                                       // Reject invalid input immediately
    }

    // Step 2: Ensure the blood bank has enough units available before deducting
    if (inventory->stocks[index].units < units) {
        snprintf(core_last_error, sizeof(core_last_error), "Not enough units in stock");
        return false;                                       // Reject request due to insufficient stock
    }

    // Step 3: Subtract the requested units in temporary memory (RAM)
    inventory->stocks[index].units -= units;                // Decrease units for the specific blood group
    inventory->total_units -= units;                        // Decrease the overall total blood units in the bank

    // Step 4: Save the updated inventory permanently to the hard drive
    if (!core_save_inventory(inventory)) {
        // FAIL-SAFE (Rollback): If saving to disk fails, restore the deducted units in RAM
        // so stock records are not accidentally lost.
        inventory->stocks[index].units += units;            // Restore the specific blood group count
        inventory->total_units += units;                    // Restore the grand total count
        return false;                                       // Report failure to the caller
    }

    return true;                                            // Successfully deducted in RAM and saved to disk
}
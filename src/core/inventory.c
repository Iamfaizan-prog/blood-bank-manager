#include <stdio.h> 
#include "core.h" // Include our main system definitions and structures

// --- Add Blood to Inventory ---
bool core_add_units(BloodInventory *inventory, const char *group, uint32_t units) {
    int index; // Variable to hold the array index of the requested blood group
    
    // Validate: Check if inventory exists, units are greater than 0, and if the blood group text is valid
    if (!inventory || units == 0 || (index = core_find_blood_group(group)) < 0) {
        snprintf(core_last_error, sizeof(core_last_error), "Invalid blood group or quantity"); // Set error message
        return false; // Reject the operation
    }
    
    // Update the numbers in the computer's temporary RAM
    inventory->stocks[index].units += units; // Add units to the specific blood group (e.g., A+)
    inventory->total_units += units; // Add units to the grand total of all blood in the bank
    
    // Attempt to save these new numbers to the hard drive permanently
    if (!core_save_inventory(inventory)) { 
        // FAIL-SAFE (Rollback): If the hard drive fails to save, we must undo the changes we just made in RAM
        // Otherwise, our RAM and our hard drive will be out of sync.
        inventory->stocks[index].units -= units; // Undo the specific group addition
        inventory->total_units -= units; // Undo the grand total addition
        return false; // Report failure
    }
    return true; // Successfully added to memory and saved to disk
}

// --- Remove Blood from Inventory (e.g., for hospital use) ---
bool core_deduct_units(BloodInventory *inventory, const char *group, uint32_t units) {
    int index; // Variable to hold the array index of the requested blood group
    
    // Validate: Check if inventory exists, units are greater than 0, and if the blood group is valid
    if (!inventory || units == 0 || (index = core_find_blood_group(group)) < 0) {
        snprintf(core_last_error, sizeof(core_last_error), "Invalid blood group or quantity"); // Set error message
        return false; // Reject the operation
    }
    
    // Extra Validation: Check if we actually have enough blood to give out
    if (inventory->stocks[index].units < units) {
        snprintf(core_last_error, sizeof(core_last_error), "Not enough units in stock"); // Set error message
        return false; // Reject because of insufficient stock
    }
    
    // Update the numbers in the computer's temporary RAM
    inventory->stocks[index].units -= units; // Subtract units from the specific blood group
    inventory->total_units -= units; // Subtract units from the grand total
    
    // Attempt to save these new numbers to the hard drive permanently
    if (!core_save_inventory(inventory)) { 
        // FAIL-SAFE (Rollback): If the save fails, put the blood back into the RAM inventory so data isn't lost
        inventory->stocks[index].units += units; // Undo the specific group deduction
        inventory->total_units += units; // Undo the grand total deduction
        return false; // Report failure
    }
    return true; // Successfully deducted from memory and saved to disk
}
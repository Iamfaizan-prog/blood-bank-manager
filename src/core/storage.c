#include <stdio.h> 
#include <stdlib.h> // Include standard library for memory allocation and basic utilities
#include <string.h> 
#include "core.h" // Include our main system definitions and structures

bool core_load_inventory(BloodInventory *inventory) { // Function to read the blood inventory from the hard drive into memory
    FILE *fp; // Declare a file pointer
    if (!inventory) return false; // Safety check: if the inventory pointer is NULL, return false
    fp = fopen(INVENTORY_FILE, "rb"); // Open the inventory data file in "read binary" mode
    if (!fp) return false; // If the file does not exist, return false so the system knows to create a new one
    if (fread(inventory, sizeof(*inventory), 1, fp) != 1) { fclose(fp); return false; } // Read the entire inventory block into memory at once; if it fails, close and return false
    fclose(fp); // Close the file after a successful read
    return true; // Return true indicating inventory was loaded successfully
}

bool core_save_inventory(const BloodInventory *inventory) { // Function to write the current memory inventory to the hard drive
    FILE *fp; // Declare a file pointer
    if (!inventory) return false; // Safety check: if pointer is NULL, return false
    fp = fopen(INVENTORY_FILE, "wb"); // Open the inventory file in "write binary" mode (overwrites existing file)
    if (!fp) { snprintf(core_last_error, sizeof(core_last_error), "Cannot write inventory file"); return false; } // If file cannot be created, write error and return false
    if (fwrite(inventory, sizeof(*inventory), 1, fp) != 1) { fclose(fp); return false; } // Write the entire memory block to the file; if it fails, close and return false
    if (fclose(fp) != 0) return false; // Safely close the file and flush data to disk; return false if closing fails
    return true; // Return true indicating successful save
}

bool core_init(BloodInventory *inventory) { // Core startup function to initialize files and memory before the program runs
    FILE *fp; // Declare a file pointer for initial checks
    int i; // Loop counter variable
    if (!inventory) return false; // Safety check: if main memory pointer is missing, fail startup
    fp = fopen(DONOR_FILE, "ab"); // Try to open the donor file in "append binary" mode (creates it if it doesn't exist)
    if (!fp) { snprintf(core_last_error, sizeof(core_last_error), "Cannot open donor data file; run from project folder"); return false; } // If we can't create/open it, log an error about the working directory and fail
    fclose(fp); // Immediately close it; we just needed to guarantee the file exists
    
    if (!core_load_inventory(inventory)) { // Attempt to load existing inventory data. If it fails (e.g., first time running the app)...
        memset(inventory, 0, sizeof(*inventory)); // ...wipe the entire inventory memory block clean with zeros
        for (i = 0; i < NUM_BLOOD_GROUPS; ++i) { // Loop through all 8 standard blood groups
            strncpy(inventory->stocks[i].blood_group, VALID_BLOOD_GROUPS[i], BLOOD_GROUP_LEN - 1); // Copy the text names ("A+", "O-", etc.) into the blank inventory slots
        }
        if (!core_save_inventory(inventory)) return false; // Save this freshly created, zeroed-out inventory to the hard drive
    }
    return true; // Startup successful, return true
}
// FILE: core_storage.c (or persistence/initialization module)
// PURPOSE: Handles loading and saving the BloodInventory to disk, as well as
//          initializing data files and default inventory structures at startup.

// 1. HEADER INCLUDES
#include <stdio.h>    // Provides file I/O functions (fopen, fread, fwrite, fclose, snprintf)
#include <stdlib.h>   // Provides standard utility functions
#include <string.h>   // Provides memory and string functions (memset, strncpy)
#include "core.h"     // Imports BloodInventory struct, constants, and core function declarations

// 2. LOAD INVENTORY FROM DISK

// Reads the saved BloodInventory structure from the binary file on disk into RAM.
bool core_load_inventory(BloodInventory *inventory) {
    FILE *fp;                                               // File pointer for reading the inventory file

    if (!inventory) return false;                           // Safety check: inventory pointer must not be NULL

    fp = fopen(INVENTORY_FILE, "rb");                       // Open the inventory file in read-binary ("rb") mode
    if (!fp) return false;                                  // Return false if file does not exist yet (triggers first-time setup)

    // Read the entire BloodInventory struct directly from disk into memory in one block
    if (fread(inventory, sizeof(*inventory), 1, fp) != 1) {
        fclose(fp);                                         // Close file before returning on a corrupted or incomplete read
        return false;
    }

    fclose(fp);                                             // Close the file to release system resources
    return true;                                            // Inventory loaded into RAM successfully
}

// 3. SAVE INVENTORY TO DISK

// Writes the current in-memory BloodInventory structure to the binary file on disk.
bool core_save_inventory(const BloodInventory *inventory) {
    FILE *fp;                                               // File pointer for writing the inventory file

    if (!inventory) return false;                           // Safety check: inventory pointer must not be NULL

    fp = fopen(INVENTORY_FILE, "wb");                       // Open in write-binary ("wb") mode (creates or overwrites file)
    if (!fp) {
        snprintf(core_last_error, sizeof(core_last_error), "Cannot write inventory file");
        return false;                                       // Fail if file cannot be opened or created
    }

    // Write the entire BloodInventory struct from RAM to disk in one block
    if (fwrite(inventory, sizeof(*inventory), 1, fp) != 1) {
        fclose(fp);                                         // Close file before returning on write failure
        return false;
    }

    // Close the file and verify all buffered bytes flushed cleanly to the hard drive
    if (fclose(fp) != 0) return false;

    return true;                                            // Inventory saved to disk successfully
}

// 4. SYSTEM INITIALIZATION (STARTUP)

// Ensures required database files exist and loads (or initializes) the blood inventory at startup.
bool core_init(BloodInventory *inventory) {
    FILE *fp;                                               // Temporary file pointer to verify file creation
    int i;                                                  // Loop counter for initializing the 8 blood groups

    if (!inventory) return false;                           // Safety check: main inventory pointer must not be NULL

    // Step 1: Guarantee that the donor database file exists on disk
    // Opening in append-binary ("ab") creates an empty file if missing without erasing existing data
    fp = fopen(DONOR_FILE, "ab");
    if (!fp) {
        snprintf(core_last_error, sizeof(core_last_error), "Cannot open donor data file; run from project folder");
        return false;                                       // Fail if 'data/' folder is missing or inaccessible
    }
    fclose(fp);                                             // Close immediately; file existence is now confirmed

    // Step 2: Load existing inventory from disk, or build a fresh one on first run
    if (!core_load_inventory(inventory)) {
        memset(inventory, 0, sizeof(*inventory));           // Zero out all bytes in the struct (sets all units & totals to 0)

        // Populate each of the 8 stock slots with its standard blood group label ("A+", "A-", etc.)
        for (i = 0; i < NUM_BLOOD_GROUPS; ++i) {
            strncpy(inventory->stocks[i].blood_group, VALID_BLOOD_GROUPS[i], BLOOD_GROUP_LEN - 1);
        }

        // Persist the newly initialized zero-stock inventory to disk
        if (!core_save_inventory(inventory)) return false;
    }

    return true;                                            // System initialized and ready for use
}
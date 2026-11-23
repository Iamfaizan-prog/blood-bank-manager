// FILE: core_crud.c (or donor CRUD module)
// PURPOSE: Implements Create, Read, Update, and Delete (CRUD) file operations
//          for managing donor records stored in the binary database file.

// 1. HEADER INCLUDES
#include <stdio.h>    // Provides file I/O functions (fopen, fread, fwrite, fclose, remove, rename)
#include <string.h>   // Provides string manipulation utilities
#include "core.h"     // Imports core function declarations, Donor struct, and error buffer

// 2. INTERNAL HELPER FUNCTIONS

// Scans the donor file to find the highest existing ID and returns the next available ID.
// 'static' means this helper function is private to this .c file and cannot be called from outside.
static uint32_t next_id(void) {
    FILE *fp = fopen(DONOR_FILE, "rb"); // Open the binary donor file for reading
    Donor d;                            // Temporary struct to hold each record read from the file
    uint32_t max_id = 1000;             // Base ID number; the first donor registered will receive 1001

    if (!fp) return 1001;               // If no file exists yet, start ID numbering at 1001

    // Loop through every record in the file and track the highest ID seen so far
    while (fread(&d, sizeof(d), 1, fp) == 1) {
        if (d.id > max_id) {
            max_id = d.id;              // Update max_id whenever a larger ID is found
        }
    }

    fclose(fp);                         // Close the file to release system resources
    return max_id + 1;                  // Return the next sequential ID
}

// 3. READ OPERATIONS (FETCHING DONORS)

// Loads all donor records into the caller's 'records' array (up to 'capacity')
// and writes the total number of donors found into 'count'.
bool core_get_all_donors(Donor *records, size_t capacity, size_t *count) {
    FILE *fp;                           // File pointer for reading the database
    Donor d;                            // Temporary struct to store one donor at a time
    size_t n = 0;                       // Counter for the total number of records read

    if (!count) return false;           // Safety check: 'count' pointer must not be NULL
    *count = 0;                         // Initialize output count to 0

    fp = fopen(DONOR_FILE, "rb");       // Open the donor file in read-binary ("rb") mode
    if (!fp) return true;               // If the file doesn't exist yet, 0 donors exist (not an error)

    // Read one Donor struct at a time until fread reaches the end of the file
    while (fread(&d, sizeof(d), 1, fp) == 1) {
        // Only copy the record into the array if a buffer was provided and hasn't reached capacity
        if (records && n < capacity) {
            records[n] = d;
        }
        ++n;                            // Always increment total record count, even if array is full
    }

    fclose(fp);                         // Close the file after reading all records
    *count = n;                         // Store the final count in the caller's variable

    // If the caller provided an array, verify it was large enough to hold every record
    if (records && n > capacity) {
        snprintf(core_last_error, sizeof(core_last_error), "Donor buffer too small");
        return false;                   // Return false to warn that some records were truncated
    }

    return true;                        // Successfully read all donor records
}

// Searches the binary file for a single donor matching 'id' and copies their data into 'result'.
bool core_get_donor(uint32_t id, Donor *result) {
    FILE *fp;                           // File pointer for reading the database
    Donor d;                            // Temporary struct for scanning records

    if (!result) return false;          // Safety check: 'result' pointer must not be NULL

    fp = fopen(DONOR_FILE, "rb");       // Open the donor file in read-binary ("rb") mode
    if (!fp) return false;              // Fail if the file does not exist or cannot be opened

    // Scan through the file record by record
    while (fread(&d, sizeof(d), 1, fp) == 1) {
        if (d.id == id) {               // Check if the current record's ID matches the target ID
            *result = d;                // Copy the matching donor struct into the output pointer
            fclose(fp);                 // Close the file immediately once found
            return true;                // Return true indicating the donor was found
        }
    }

    fclose(fp);                         // Close the file after reaching the end with no match
    snprintf(core_last_error, sizeof(core_last_error), "Donor not found");
    return false;                       // Return false indicating no donor has that ID
}

// 4. CREATE OPERATION (ADDING A NEW DONOR)

// Validates a new donor's details and eligibility, assigns a unique ID, and appends them to the file.
bool core_add_donor(const Donor *donor) {
    FILE *fp;                           // File pointer for appending to the database
    Donor d;                            // Local copy of the donor struct so we can assign an ID
    Date today;                         // Holds today's date for checking donation intervals

    // Step 1: Validate that the pointer is valid, name is non-empty, and blood group is recognized
    if (!donor || !donor->name[0] || core_find_blood_group(donor->blood_group) < 0) {
        snprintf(core_last_error, sizeof(core_last_error), "Invalid donor details or blood group");
        return false;
    }

    // Step 2: Check medical eligibility rules (age, weight, and 90-day waiting period)
    core_get_today(&today);
    if (!core_check_eligibility(donor->age, donor->weight, donor->has_donated_before, &donor->last_donation, &today)) {
        return false;                   // core_check_eligibility automatically sets core_last_error
    }

    // Step 3: Copy donor data into a writable struct and generate a new unique ID
    d = *donor;
    d.id = next_id();

    // Step 4: Open the file in append-binary ("ab") mode to add the new record at the end
    fp = fopen(DONOR_FILE, "ab");
    if (!fp) {
        snprintf(core_last_error, sizeof(core_last_error), "Cannot open donor file");
        return false;
    }

    // Write the exact bytes of the Donor struct to disk; fwrite returns 1 if 1 item was written
    if (fwrite(&d, sizeof(d), 1, fp) != 1) {
        fclose(fp);                     // Always close the file even if writing fails
        return false;
    }

    fclose(fp);                         // Flush buffer and close the file safely
    core_last_error[0] = '\0';          // Clear any old error messages by setting first char to null
    return true;                        // Donor successfully registered
}

// 5. UPDATE OPERATION (MODIFYING AN EXISTING DONOR)

// Rewrites the donor file using a temporary file to safely replace a matching donor's record.
// Note: Binary files cannot easily overwrite variable-sized or middle records safely without a temp file.
bool core_update_donor(const Donor *donor) {
    FILE *in, *out;                     // 'in' reads the original file; 'out' writes the updated copy
    Donor d;                            // Temporary struct to hold each record during transfer
    Date today;                         // Holds today's date for eligibility verification
    bool found = false;                 // Tracks whether a donor with the given ID actually existed

    // Step 1: Validate ID, name, blood group, and medical eligibility before touching any files
    if (!donor || donor->id == 0 || !donor->name[0] || core_find_blood_group(donor->blood_group) < 0) {
        snprintf(core_last_error, sizeof(core_last_error), "Invalid donor details or blood group");
        return false;
    }

    core_get_today(&today);
    if (!core_check_eligibility(donor->age, donor->weight, donor->has_donated_before, &donor->last_donation, &today)) {
        return false;
    }

    // Step 2: Open the existing database for reading and create a temporary file for writing
    in = fopen(DONOR_FILE, "rb");
    if (!in) return false;

    out = fopen("data/donors.tmp", "wb");
    if (!out) {
        fclose(in);                     // Clean up 'in' if 'out' fails to open
        return false;
    }

    // Step 3: Copy every record from 'in' to 'out', swapping in the updated record when ID matches
    while (fread(&d, sizeof(d), 1, in) == 1) {
        if (d.id == donor->id) {
            d = *donor;                 // Replace old record data with the new updated struct
            found = true;               // Mark that we successfully located the target donor
        }
        if (fwrite(&d, sizeof(d), 1, out) != 1) {
            fclose(in);                 // On write failure, close both files...
            fclose(out);
            remove("data/donors.tmp");  // ...and delete the incomplete temporary file
            return false;
        }
    }

    fclose(in);                         // Close both files before renaming or deleting them
    fclose(out);

    // Step 4: If the ID was never found, discard the temp file and report an error
    if (!found) {
        remove("data/donors.tmp");
        snprintf(core_last_error, sizeof(core_last_error), "Donor not found");
        return false;
    }

    // Step 5: Delete the old database file and rename the temporary file to replace it
    if (remove(DONOR_FILE) != 0 || rename("data/donors.tmp", DONOR_FILE) != 0) {
        return false;
    }

    return true;                        // Donor record successfully updated
}

// 6. DELETE OPERATION (REMOVING A DONOR)

// Rewrites the donor file using a temporary file while skipping the record matching 'id'.
bool core_delete_donor(uint32_t id) {
    FILE *in, *out;                     // 'in' reads the original file; 'out' writes the filtered copy
    Donor d;                            // Temporary struct to hold each record during transfer
    bool found = false;                 // Tracks whether the target ID was found and removed

    // Step 1: Open the existing database for reading and create a temporary file for writing
    in = fopen(DONOR_FILE, "rb");
    if (!in) return false;

    out = fopen("data/donors.tmp", "wb");
    if (!out) {
        fclose(in);                     // Clean up 'in' if 'out' fails to open
        return false;
    }

    // Step 2: Copy all records to the temp file EXCEPT the one matching 'id'
    while (fread(&d, sizeof(d), 1, in) == 1) {
        if (d.id == id) {
            found = true;               // Mark that the target donor was found
            continue;                   // Skip fwrite() so this record is left out of the new file
        }
        if (fwrite(&d, sizeof(d), 1, out) != 1) {
            fclose(in);                 // On write failure, close both files...
            fclose(out);
            remove("data/donors.tmp");  // ...and clean up the incomplete temporary file
            return false;
        }
    }

    fclose(in);                         // Close both files before renaming or deleting them
    fclose(out);

    // Step 3: If no donor had that ID, delete the temp file and report an error
    if (!found) {
        remove("data/donors.tmp");
        snprintf(core_last_error, sizeof(core_last_error), "Donor not found");
        return false;
    }

    // Step 4: Replace the old database file with the new filtered temporary file
    if (remove(DONOR_FILE) != 0 || rename("data/donors.tmp", DONOR_FILE) != 0) {
        return false;
    }

    return true;                        // Donor record successfully deleted
}
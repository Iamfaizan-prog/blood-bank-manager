//CRUD (Create, Read, Update, Delete) operations
#include <stdio.h> 
#include <string.h> 
#include "core.h" // Include the core header file 

bool core_get_all_donors(Donor *records, size_t capacity, size_t *count) { // Function to fetch all donors
    FILE *fp; // Declare a file pointer for file operations
    Donor d; // Declare a temporary Donor structure to hold read data
    size_t n = 0; // Initialize a counter for the number of donors read
    if (!count) return false; // If the count pointer is NULL, return false which is error
    *count = 0; // Set the initial value at the count pointer to 0
    fp = fopen(DONOR_FILE, "rb"); // Open the donor file in read-binary mode
    if (!fp) return true; // If file doesn't exist, assume 0 donors and return true (success)
    while (fread(&d, sizeof(d), 1, fp) == 1) { // Read one Donor record at a time until end of file
        if (records && n < capacity) records[n] = d; // If array exists and has space, store the donor
        ++n; // Increment the total donor count
    } 
    fclose(fp); // Close the file to free system resources
    *count = n; // Update the external count with the total number of records found
    if (records && n > capacity) { snprintf(core_last_error, sizeof(core_last_error), "Donor buffer too small"); return false; } // Error if records exceed array size
    return true; // Return true indicating successful execution
} // End of core_get_all_donors function

bool core_get_donor(uint32_t id, Donor *result) { // Function to fetch a single donor by ID
    FILE *fp; // Declare a file pointer
    Donor d; // Declare a temporary Donor structure
    if (!result) return false; // If the result pointer is NULL, return false (error)
    fp = fopen(DONOR_FILE, "rb"); // Open the donor file in read-binary mode
    if (!fp) return false; // If the file cannot be opened, return false
    while (fread(&d, sizeof(d), 1, fp) == 1) { // Read one Donor record at a time
        if (d.id == id) { *result = d; fclose(fp); return true; } // If IDs match, copy data to result, close file, and return true
    } // End of reading loop
    fclose(fp); // Close the file if no match was found
    snprintf(core_last_error, sizeof(core_last_error), "Donor not found"); // Set the global error message
    return false; // Return false indicating the donor was not found
} 

static uint32_t next_id(void) { // Internal helper function to generate the next unique donor ID
    FILE *fp = fopen(DONOR_FILE, "rb"); // Open the donor file in read-binary mode
    Donor d; // Declare a temporary Donor structure
    uint32_t max_id = 1000; // Initialize the starting max ID to 1000
    if (!fp) return 1001; // If file doesn't exist, return 1001 as the first ID
    while (fread(&d, sizeof(d), 1, fp) == 1) if (d.id > max_id) max_id = d.id; // Read all records and find the highest ID
    fclose(fp); // Close the file
    return max_id + 1; // Return the highest found ID plus one
} 

bool core_add_donor(const Donor *donor) { // Function to add a new donor to the system
    FILE *fp; // Declare a file pointer
    Donor d; // Declare a temporary Donor structure
    Date today; // Declare a Date structure for the current date
    if (!donor || !donor->name[0] || core_find_blood_group(donor->blood_group) < 0) { // Validate pointer, name, and blood group
        snprintf(core_last_error, sizeof(core_last_error), "Invalid donor details or blood group"); return false; // Set error and return false if invalid
    } // End of initial validation
    core_get_today(&today); // Fetch the current date and store it in 'today'
    if (!core_check_eligibility(donor->age, donor->weight, donor->has_donated_before, &donor->last_donation, &today)) return false; // Check medical eligibility rules
    d = *donor; // Copy the provided donor data into the temporary structure
    d.id = next_id(); // Assign a newly generated unique ID to this donor
    fp = fopen(DONOR_FILE, "ab"); // Open the donor file in append-binary mode
    if (!fp) { snprintf(core_last_error, sizeof(core_last_error), "Cannot open donor file"); return false; } // Set error and return if file won't open
    if (fwrite(&d, sizeof(d), 1, fp) != 1) { fclose(fp); return false; } // Write the new record to the file; if it fails, close and return false
    fclose(fp); // Close the file successfully
    core_last_error[0] = '\0'; // Clear any lingering error messages (set string to empty)
    return true; // Return true indicating successful addition
} 

bool core_update_donor(const Donor *donor) { // Function to modify an existing donor's data
    FILE *in, *out; // Declare two file pointers: one for reading, one for writing
    Donor d; // Declare a temporary Donor structure
    Date today; // Declare a Date structure for the current date
    bool found = false; // Flag to track if the target donor was found
    if (!donor || donor->id == 0 || !donor->name[0] || core_find_blood_group(donor->blood_group) < 0) { // Validate ID, name, and blood group
        snprintf(core_last_error, sizeof(core_last_error), "Invalid donor details or blood group"); // Set error message
        return false; // Return false due to invalid input
    } // End of initial validation
    core_get_today(&today); // Fetch the current date
    if (!core_check_eligibility(donor->age, donor->weight, donor->has_donated_before, &donor->last_donation, &today)) return false; // Check medical eligibility
    in = fopen(DONOR_FILE, "rb"); // Open the main donor file for reading
    if (!in) return false; // Return false if the main file cannot be opened
    out = fopen("data/donors.tmp", "wb"); // Open a new temporary file for writing
    if (!out) { fclose(in); return false; } // If temp file fails to open, close main file and return false
    while (fread(&d, sizeof(d), 1, in) == 1) { // Read records one by one from the main file
        if (d.id == donor->id) { d = *donor; found = true; } // If IDs match, overwrite 'd' with new data and set found flag
        if (fwrite(&d, sizeof(d), 1, out) != 1) { fclose(in); fclose(out); remove("data/donors.tmp"); return false; } // Write 'd' to temp file; handle write errors
    } // End of read/write loop
    fclose(in); fclose(out); // Close both files
    if (!found) { remove("data/donors.tmp"); snprintf(core_last_error, sizeof(core_last_error), "Donor not found"); return false; } // If not found, delete temp file and return false
    if (remove(DONOR_FILE) != 0 || rename("data/donors.tmp", DONOR_FILE) != 0) return false; // Delete old file and rename temp file to main file name
    return true; // Return true indicating successful update
} 

bool core_delete_donor(uint32_t id) { // Function to remove a donor from the system
    FILE *in, *out; // Declare two file pointers (read and write)
    Donor d; // Declare a temporary Donor structure
    bool found = false; // Flag to track if the target donor was found
    in = fopen(DONOR_FILE, "rb"); // Open the main donor file for reading
    if (!in) return false; // Return false if the main file cannot be opened
    out = fopen("data/donors.tmp", "wb"); // Open a new temporary file for writing
    if (!out) { fclose(in); return false; } // If temp file fails, close main file and return false
    while (fread(&d, sizeof(d), 1, in) == 1) { // Read records one by one from the main file
        if (d.id == id) { found = true; continue; } // If IDs match, set flag and SKIP writing this record (effectively deleting it)
        if (fwrite(&d, sizeof(d), 1, out) != 1) { fclose(in); fclose(out); remove("data/donors.tmp"); return false; } // Write non-matching records to temp file
    } // End of read/write loop
    fclose(in); fclose(out); // Close both files
    if (!found) { remove("data/donors.tmp"); snprintf(core_last_error, sizeof(core_last_error), "Donor not found"); return false; } // If not found, delete temp file and return false
    if (remove(DONOR_FILE) != 0 || rename("data/donors.tmp", DONOR_FILE) != 0) return false; // Delete old file and rename temp file to main file name
    return true; // Return true indicating successful deletion
} 
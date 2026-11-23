// FILE: models.h
// PURPOSE: Blueprint and core data rules for the Blood Bank Management System.
//          Defines all constants, file paths, and data structures (Date, Donor,
//          BloodStock, and BloodInventory) shared across the program.

#ifndef MODELS_H
#define MODELS_H

// 1. STANDARD LIBRARY INCLUDES
#include <stdbool.h>  // Enables standard 'true' and 'false' boolean values instead of 1 and 0
#include <stdint.h>   // Provides fixed-width integers (e.g., uint32_t) for consistent memory size across platforms

// 2. ARRAY SIZE & BUFFER LENGTH CONSTANTS
#define NUM_BLOOD_GROUPS 8      // Total number of recognized blood types (A+, A-, B+, B-, AB+, AB-, O+, O-)
#define BLOOD_GROUP_LEN  4      // Max 3 chars (e.g., "AB-") + 1 null terminator ('\0')
#define NAME_LEN         64     // Maximum characters allowed for a donor's name
#define PHONE_LEN        24     // Maximum characters allowed for a phone number

// 3. FILE STORAGE PATHS
#define DATA_DIR         "data"                // Folder where binary data files are stored
#define DONOR_FILE       "data/donors.dat"     // Binary file storing donor records
#define INVENTORY_FILE   "data/inventory.dat"  // Binary file storing blood stock levels

// 4. DONOR ELIGIBILITY RULES
#define MIN_DONOR_AGE         18    // Minimum allowed age in years
#define MAX_DONOR_AGE         65    // Maximum allowed age in years
#define MIN_DONOR_WEIGHT      50.0  // Minimum required weight in kilograms (kg)
#define MIN_DONATION_INTERVAL 90    // Minimum waiting period (in days) between donations

// 5. DATA STRUCTURES (STRUCTS)

// Stores calendar dates (used for tracking a donor's last donation)
typedef struct Date {
    int day;
    int month;
    int year;
} Date;

// Stores complete personal and medical eligibility details for a single donor
typedef struct Donor {
    uint32_t id;                         // Unique 4-byte positive ID number
    char name[NAME_LEN];                 // Donor's full name
    int age;                             // Donor's age in years
    char gender;                         // Single character ('M', 'F', 'O')
    double weight;                       // Donor's weight in kg (8-byte floating point)
    char blood_group[BLOOD_GROUP_LEN];   // Blood type string (e.g., "O+")
    char phone[PHONE_LEN];               // Contact number
    Date last_donation;                  // Nested Date struct for last donation date
    bool has_donated_before;             // Flag indicating if the donor has a previous record
} Donor;

// Tracks available units for one specific blood group
typedef struct BloodStock {
    char blood_group[BLOOD_GROUP_LEN];   // Blood type identifier (e.g., "A+")
    uint32_t units;                      // Number of available blood bags/units
} BloodStock;

// Master inventory holding stock levels for all 8 blood groups
typedef struct BloodInventory {
    BloodStock stocks[NUM_BLOOD_GROUPS]; // Array holding stock info for each of the 8 blood types
    uint32_t total_units;                // Combined total of all blood units in the bank
} BloodInventory;

// 6. EXTERNAL GLOBAL DECLARATIONS
// Array of read-only strings listing valid blood types.
// 'extern' tells the compiler this array is declared here, while its actual values are defined in a .c file.
extern const char *VALID_BLOOD_GROUPS[NUM_BLOOD_GROUPS];

#endif // End of MODELS_H header guard
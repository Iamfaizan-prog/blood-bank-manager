// FILE: core.c (or core validation & utilities module)
// PURPOSE: Defines global blood group constants, error buffers, system clock utilities,
//          date arithmetic, and donor medical eligibility rules.

// 1. HEADER INCLUDES
#include <stdio.h>    // Provides snprintf() for formatting error messages
#include <string.h>   // Provides strcmp() for exact string comparison
#include <time.h>     // Provides time() and localtime() to fetch the computer's real-world clock
#include "core.h"     // Imports core function declarations, Date struct, and system constants

// 2. GLOBAL VARIABLE DEFINITIONS

// Master lookup table of all 8 valid blood groups (memory is allocated here; declared 'extern' in models.h)
const char *VALID_BLOOD_GROUPS[NUM_BLOOD_GROUPS] = {
    "A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"
};

// Global 256-byte buffer storing the most recent error message (declared 'extern' in core.h)
char core_last_error[256] = "";

// 3. BLOOD GROUP LOOKUP

// Searches VALID_BLOOD_GROUPS for an exact match and returns its array index (0-7), or -1 if invalid.
int core_find_blood_group(const char *group) {
    if (!group) return -1;                                  // Safety check: reject NULL pointers immediately

    for (int i = 0; i < NUM_BLOOD_GROUPS; ++i) {
        if (strcmp(group, VALID_BLOOD_GROUPS[i]) == 0) {    // strcmp() returns 0 when two strings are identical
            return i;                                       // Return the matching index (0 through 7)
        }
    }

    return -1;                                              // Loop finished with no match; blood group is invalid
}

// 4. SYSTEM DATE & CALENDAR UTILITIES

// Fetches the computer's current local date and stores day, month, and year inside 'today'.
void core_get_today(Date *today) {
    time_t now;                                             // Holds raw system time (seconds since Jan 1, 1970)
    struct tm *value;                                       // Pointer to broken-down calendar date/time struct

    if (!today) return;                                     // Safety check: exit if destination pointer is NULL

    now = time(NULL);                                       // Read current raw timestamp from the OS clock
    value = localtime(&now);                                // Convert raw timestamp into local calendar fields

    // Failsafe: if the system clock fails, default to a safe baseline date
    if (!value) {
        today->day = 1;
        today->month = 1;
        today->year = 2026;
        return;
    }

    today->day   = value->tm_mday;                          // Day of the month (1-31)
    today->month = value->tm_mon + 1;                       // Months since January (0-11), so add 1 for (1-12)
    today->year  = value->tm_year + 1900;                   // Years since 1900, so add 1900 for full 4-digit year
}

// Internal helper: converts a calendar Date into a single continuous day count (Howard Hinnant's algorithm).
// This allows exact day-difference subtraction across months, years, and leap years.
static long date_number(const Date *d) {
    int y, m;                                               // Adjusted year and month values
    long era, yoe, doy, doe;                                // 400-year era, year-of-era, day-of-year, day-of-era

    // Reject NULL pointers or physically impossible calendar values
    if (!d || d->month < 1 || d->month > 12 || d->day < 1 || d->day > 31) {
        return -1;
    }

    y = d->year;
    m = d->month;
    y -= (m <= 2);                                          // Treat Jan/Feb as months 13/14 of the previous year so leap day falls at year-end
    era = (y >= 0 ? y : y - 399) / 400;                     // Determine which 400-year Gregorian cycle the year falls into
    yoe = y - era * 400;                                    // Year within the current 400-year era (0-399)
    doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d->day - 1; // Day within the shifted year (0-365)
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;            // Days elapsed in era (accounts for 4-year and 100-year leap rules)

    return era * 146097 + doe;                              // 146,097 is the exact number of days in a 400-year Gregorian cycle
}

// 5. DONOR ELIGIBILITY VALIDATION

// Checks whether a donor meets age, weight, and minimum 90-day recovery interval requirements.
bool core_check_eligibility(int age, double weight, bool has_donated, const Date *last, const Date *today) {
    // Step 1: Verify age (18-65) and minimum body weight (50.0 kg)
    if (age < MIN_DONOR_AGE || age > MAX_DONOR_AGE || weight < MIN_DONOR_WEIGHT) {
        snprintf(core_last_error, sizeof(core_last_error), "Age must be 18-65 and weight at least 50 kg");
        return false;                                       // Reject donor due to age or weight violation
    }

    // Step 2: If the donor has donated previously, verify at least 90 days have passed
    if (has_donated) {
        long a = date_number(last);                         // Convert last donation date to continuous day number
        long b = date_number(today);                        // Convert today's date to continuous day number

        // Reject if either date is invalid, last donation is in the future, or gap is less than 90 days
        if (a < 0 || b < 0 || b < a || b - a < MIN_DONATION_INTERVAL) {
            snprintf(core_last_error, sizeof(core_last_error), "At least 90 days must pass since the last donation");
            return false;                                   // Reject donor due to insufficient recovery interval
        }
    }

    core_last_error[0] = '\0';                              // Clear any previous error message on success
    return true;                                            // Donor meets all medical eligibility criteria
}
#include <stdio.h> 
#include <string.h> // String manipulation library for functions like strcmp
#include <time.h> // C time library Used to fetch the computer's real-world clock data
#include "core.h" // Includes our system's main header definitions

const char *VALID_BLOOD_GROUPS[NUM_BLOOD_GROUPS] = {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"}; // Allocates memory for the 8 standard blood group strings
char core_last_error[256] = ""; // Allocates a 256-byte global string to store the most recent error message

int core_find_blood_group(const char *group) { // Function to find the index number of a blood group string
    if (!group) return -1; // Safety check: if input string is empty, return -1 (invalid)
    for (int i = 0; i < NUM_BLOOD_GROUPS; ++i) 
        if (strcmp(group, VALID_BLOOD_GROUPS[i]) == 0) return i; // If exact text match is found, return its index number
    return -1; // If loop finishes without a match, return -1 (invalid)
}

void core_get_today(Date *today) { // Function to fetch current date and save it in 'today'
    time_t now; // Variable to hold raw system time 
    struct tm *value; // Pointer to a structure that holds human-readable time (Y/M/D)
    if (!today) return; // Safety check: if pointer is missing, exit function
    now = time(NULL); // Get current raw time from the system clock
    value = localtime(&now); // Convert raw time to local calendar time
    if (!value) { today->day = 1; today->month = 1; today->year = 2026; return; } // Failsafe: if clock fails, default to Jan 1, 2026
    today->day = value->tm_mday; // Copy the day of the month (1-31)
    today->month = value->tm_mon + 1; // Copy month (add 1 because C counts months from 0-11)
    today->year = value->tm_year + 1900; // Copy year (add 1900 because C counts years since 1900)
}

static long date_number(const Date *d) { // Helper function to convert a Date into total days since year zero
    int y, m; // Temporary variables for year and month
    long era, yoe, doy, doe; // Variables for eras and leap-year calculations
    if (!d || d->month < 1 || d->month > 12 || d->day < 1 || d->day > 31) return -1; // Return error if date is physically impossible
    y = d->year; m = d->month; // Copy year and month for calculation
    y -= (m <= 2); // Shift year back by 1 for Jan/Feb (standard chronological math trick)
    era = (y >= 0 ? y : y - 399) / 400; // Calculate 400-year cycle era
    yoe = y - era * 400; // Calculate year of the current era
    doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d->day - 1; // Calculate day of the year
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy; // Calculate days since start of era accounting for leap years
    return era * 146097 + doe; // Return total continuous days
}

bool core_check_eligibility(int age, double weight, bool has_donated, const Date *last, const Date *today) { // Function to check if donor is legally/medically allowed
    if (age < MIN_DONOR_AGE || age > MAX_DONOR_AGE || weight < MIN_DONOR_WEIGHT) { // Check if age and weight are outside allowed constants
        snprintf(core_last_error, sizeof(core_last_error), "Age must be 18-65 and weight at least 50 kg"); // Write error message
        return false; // Reject donor
    }
    if (has_donated) { // If the donor has donated before, we must check dates
        long a = date_number(last), b = date_number(today); // Convert both dates to total days for easy subtraction
        if (a < 0 || b < 0 || b < a || b - a < MIN_DONATION_INTERVAL) { // If dates invalid, last is in future, or wait time is too short...
            snprintf(core_last_error, sizeof(core_last_error), "At least 90 days must pass since the last donation"); // Write error message
            return false; // Reject donor
        }
    }
    core_last_error[0] = '\0'; // If all checks pass, clear the global error string
    return true; // Approve donor
}
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "core.h"

const char *VALID_BLOOD_GROUPS[NUM_BLOOD_GROUPS] = {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"};
char core_last_error[256] = "";

int core_find_blood_group(const char *group) {
    int i;
    if (!group) return -1;
    for (i = 0; i < NUM_BLOOD_GROUPS; ++i)
        if (strcmp(group, VALID_BLOOD_GROUPS[i]) == 0) return i;
    return -1;
}

void core_get_today(Date *today) {
    time_t now;
    struct tm *value;
    if (!today) return;
    now = time(NULL);
    value = localtime(&now);
    if (!value) { today->day = 1; today->month = 1; today->year = 2026; return; }
    today->day = value->tm_mday;
    today->month = value->tm_mon + 1;
    today->year = value->tm_year + 1900;
}

static long date_number(const Date *d) {
    int y, m;
    long era, yoe, doy, doe;
    if (!d || d->month < 1 || d->month > 12 || d->day < 1 || d->day > 31) return -1;
    y = d->year; m = d->month;
    y -= (m <= 2);
    era = (y >= 0 ? y : y - 399) / 400;
    yoe = y - era * 400;
    doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d->day - 1;
    doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe;
}

bool core_check_eligibility(int age, double weight, bool has_donated, const Date *last, const Date *today) {
    if (age < MIN_DONOR_AGE || age > MAX_DONOR_AGE || weight < MIN_DONOR_WEIGHT) {
        snprintf(core_last_error, sizeof(core_last_error), "Age must be 18-65 and weight at least 50 kg");
        return false;
    }
    if (has_donated) {
        long a = date_number(last), b = date_number(today);
        if (a < 0 || b < 0 || b < a || b - a < MIN_DONATION_INTERVAL) {
            snprintf(core_last_error, sizeof(core_last_error), "At least 90 days must pass since the last donation");
            return false;
        }
    }
    core_last_error[0] = '\0';
    return true;
}

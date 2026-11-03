#include <stdio.h>
#include <string.h>
#include "core.h"

bool core_get_all_donors(Donor *records, size_t capacity, size_t *count) {
    FILE *fp;
    Donor d;
    size_t n = 0;
    if (!count) return false;
    *count = 0;
    fp = fopen(DONOR_FILE, "rb");
    if (!fp) return true;
    while (fread(&d, sizeof(d), 1, fp) == 1) {
        if (records && n < capacity) records[n] = d;
        ++n;
    }
    fclose(fp);
    *count = n;
    if (records && n > capacity) { snprintf(core_last_error, sizeof(core_last_error), "Donor buffer too small"); return false; }
    return true;
}

bool core_get_donor(uint32_t id, Donor *result) {
    FILE *fp;
    Donor d;
    if (!result) return false;
    fp = fopen(DONOR_FILE, "rb");
    if (!fp) return false;
    while (fread(&d, sizeof(d), 1, fp) == 1) {
        if (d.id == id) { *result = d; fclose(fp); return true; }
    }
    fclose(fp);
    snprintf(core_last_error, sizeof(core_last_error), "Donor not found");
    return false;
}

static uint32_t next_id(void) {
    FILE *fp = fopen(DONOR_FILE, "rb");
    Donor d;
    uint32_t max_id = 1000;
    if (!fp) return 1001;
    while (fread(&d, sizeof(d), 1, fp) == 1) if (d.id > max_id) max_id = d.id;
    fclose(fp);
    return max_id + 1;
}

bool core_add_donor(const Donor *donor) {
    FILE *fp;
    Donor d;
    Date today;
    if (!donor || !donor->name[0] || core_find_blood_group(donor->blood_group) < 0) {
        snprintf(core_last_error, sizeof(core_last_error), "Invalid donor details or blood group"); return false;
    }
    core_get_today(&today);
    if (!core_check_eligibility(donor->age, donor->weight, donor->has_donated_before, &donor->last_donation, &today)) return false;
    d = *donor;
    d.id = next_id();
    fp = fopen(DONOR_FILE, "ab");
    if (!fp) { snprintf(core_last_error, sizeof(core_last_error), "Cannot open donor file"); return false; }
    if (fwrite(&d, sizeof(d), 1, fp) != 1) { fclose(fp); return false; }
    fclose(fp);
    core_last_error[0] = '\0';
    return true;
}

bool core_update_donor(const Donor *donor) {
    FILE *in, *out;
    Donor d;
    Date today;
    bool found = false;
    if (!donor || donor->id == 0 || !donor->name[0] || core_find_blood_group(donor->blood_group) < 0) {
        snprintf(core_last_error, sizeof(core_last_error), "Invalid donor details or blood group");
        return false;
    }
    core_get_today(&today);
    if (!core_check_eligibility(donor->age, donor->weight, donor->has_donated_before, &donor->last_donation, &today)) return false;
    in = fopen(DONOR_FILE, "rb");
    if (!in) return false;
    out = fopen("data/donors.tmp", "wb");
    if (!out) { fclose(in); return false; }
    while (fread(&d, sizeof(d), 1, in) == 1) {
        if (d.id == donor->id) { d = *donor; found = true; }
        if (fwrite(&d, sizeof(d), 1, out) != 1) { fclose(in); fclose(out); remove("data/donors.tmp"); return false; }
    }
    fclose(in); fclose(out);
    if (!found) { remove("data/donors.tmp"); snprintf(core_last_error, sizeof(core_last_error), "Donor not found"); return false; }
    if (remove(DONOR_FILE) != 0 || rename("data/donors.tmp", DONOR_FILE) != 0) return false;
    return true;
}

bool core_delete_donor(uint32_t id) {
    FILE *in, *out;
    Donor d;
    bool found = false;
    in = fopen(DONOR_FILE, "rb");
    if (!in) return false;
    out = fopen("data/donors.tmp", "wb");
    if (!out) { fclose(in); return false; }
    while (fread(&d, sizeof(d), 1, in) == 1) {
        if (d.id == id) { found = true; continue; }
        if (fwrite(&d, sizeof(d), 1, out) != 1) { fclose(in); fclose(out); remove("data/donors.tmp"); return false; }
    }
    fclose(in); fclose(out);
    if (!found) { remove("data/donors.tmp"); snprintf(core_last_error, sizeof(core_last_error), "Donor not found"); return false; }
    if (remove(DONOR_FILE) != 0 || rename("data/donors.tmp", DONOR_FILE) != 0) return false;
    return true;
}

#ifndef CORE_H
#define CORE_H
#include "models.h"
#include <stddef.h>
extern char core_last_error[256];
bool core_init(BloodInventory *inventory);
int core_find_blood_group(const char *group);
void core_get_today(Date *today);
bool core_check_eligibility(int age, double weight, bool has_donated, const Date *last, const Date *today);
bool core_get_all_donors(Donor *records, size_t capacity, size_t *count);
bool core_get_donor(uint32_t id, Donor *result);
bool core_add_donor(const Donor *donor);
bool core_update_donor(const Donor *donor);
bool core_delete_donor(uint32_t id);
bool core_add_units(BloodInventory *inventory, const char *group, uint32_t units);
bool core_deduct_units(BloodInventory *inventory, const char *group, uint32_t units);
bool core_save_inventory(const BloodInventory *inventory);
bool core_load_inventory(BloodInventory *inventory);
#endif

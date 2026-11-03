#include <stdio.h>
#include "core.h"

bool core_add_units(BloodInventory *inventory, const char *group, uint32_t units) {
    int index;
    if (!inventory || units == 0 || (index = core_find_blood_group(group)) < 0) {
        snprintf(core_last_error, sizeof(core_last_error), "Invalid blood group or quantity"); return false;
    }
    inventory->stocks[index].units += units;
    inventory->total_units += units;
    if (!core_save_inventory(inventory)) { inventory->stocks[index].units -= units; inventory->total_units -= units; return false; }
    return true;
}

bool core_deduct_units(BloodInventory *inventory, const char *group, uint32_t units) {
    int index;
    if (!inventory || units == 0 || (index = core_find_blood_group(group)) < 0) {
        snprintf(core_last_error, sizeof(core_last_error), "Invalid blood group or quantity"); return false;
    }
    if (inventory->stocks[index].units < units) {
        snprintf(core_last_error, sizeof(core_last_error), "Not enough units in stock"); return false;
    }
    inventory->stocks[index].units -= units;
    inventory->total_units -= units;
    if (!core_save_inventory(inventory)) { inventory->stocks[index].units += units; inventory->total_units += units; return false; }
    return true;
}

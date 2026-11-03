#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core.h"

bool core_load_inventory(BloodInventory *inventory) {
    FILE *fp;
    if (!inventory) return false;
    fp = fopen(INVENTORY_FILE, "rb");
    if (!fp) return false;
    if (fread(inventory, sizeof(*inventory), 1, fp) != 1) { fclose(fp); return false; }
    fclose(fp);
    return true;
}

bool core_save_inventory(const BloodInventory *inventory) {
    FILE *fp;
    if (!inventory) return false;
    fp = fopen(INVENTORY_FILE, "wb");
    if (!fp) { snprintf(core_last_error, sizeof(core_last_error), "Cannot write inventory file"); return false; }
    if (fwrite(inventory, sizeof(*inventory), 1, fp) != 1) { fclose(fp); return false; }
    if (fclose(fp) != 0) return false;
    return true;
}

bool core_init(BloodInventory *inventory) {
    FILE *fp;
    int i;
    if (!inventory) return false;
    fp = fopen(DONOR_FILE, "ab");
    if (!fp) { snprintf(core_last_error, sizeof(core_last_error), "Cannot open donor data file; run from project folder"); return false; }
    fclose(fp);
    if (!core_load_inventory(inventory)) {
        memset(inventory, 0, sizeof(*inventory));
        for (i = 0; i < NUM_BLOOD_GROUPS; ++i) {
            strncpy(inventory->stocks[i].blood_group, VALID_BLOOD_GROUPS[i], BLOOD_GROUP_LEN - 1);
        }
        if (!core_save_inventory(inventory)) return false;
    }
    return true;
}

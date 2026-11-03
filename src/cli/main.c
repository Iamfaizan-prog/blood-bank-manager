#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "core.h"

static void json_string(const char *s) {
    const unsigned char *p = (const unsigned char *)(s ? s : "");
    putchar('"');
    while (*p) { if (*p == '"' || *p == '\\') { putchar('\\'); putchar(*p); } else if (*p < 32) printf(" "); else putchar(*p); ++p; }
    putchar('"');
}
static void print_donor(const Donor *d) {
    printf("{\"id\":%u,\"name\":", d->id); json_string(d->name);
    printf(",\"age\":%d,\"gender\":", d->age); { char g[2] = {d->gender, 0}; json_string(g); }
    printf(",\"weight\":%.1f,\"blood_group\":", d->weight); json_string(d->blood_group);
    printf(",\"phone\":"); json_string(d->phone);
    printf(",\"has_donated_before\":%s,\"last_donation\":{\"day\":%d,\"month\":%d,\"year\":%d}}", d->has_donated_before ? "true" : "false", d->last_donation.day, d->last_donation.month, d->last_donation.year);
}
static int fail(const char *message) { printf("{\"ok\":false,\"error\":"); json_string(message); puts("}"); return 1; }
int cli_run(int argc, char **argv) {
    BloodInventory inv; Donor d; char *end; unsigned long number;
    if (!core_init(&inv)) return fail(core_last_error);
    if (argc < 2) return fail("Missing command");
    if (strcmp(argv[1], "list") == 0) {
        Donor records[10000]; size_t count = 0, i;
        if (!core_get_all_donors(records, 10000, &count)) return fail(core_last_error);
        printf("{\"ok\":true,\"donors\":[");
        for (i = 0; i < count; ++i) { if (i) putchar(','); print_donor(&records[i]); }
        puts("]}"); return 0;
    }
    if (strcmp(argv[1], "get") == 0 && argc == 3) {
        number = strtoul(argv[2], &end, 10); if (*end) return fail("Invalid ID");
        if (!core_get_donor((uint32_t)number, &d)) return fail(core_last_error);
        printf("{\"ok\":true,\"donor\":"); print_donor(&d); puts("}"); return 0;
    }
    if (strcmp(argv[1], "delete") == 0 && argc == 3) {
        number = strtoul(argv[2], &end, 10); if (*end) return fail("Invalid ID");
        if (!core_delete_donor((uint32_t)number)) return fail(core_last_error);
        puts("{\"ok\":true}"); return 0;
    }
    if (strcmp(argv[1], "add") == 0 && argc == 12) {
        memset(&d, 0, sizeof(d));
        strncpy(d.name, argv[2], sizeof(d.name)-1); d.age = atoi(argv[3]); d.gender = argv[4][0]; d.weight = atof(argv[5]);
        strncpy(d.blood_group, argv[6], sizeof(d.blood_group)-1); strncpy(d.phone, argv[7], sizeof(d.phone)-1);
        d.has_donated_before = atoi(argv[8]) != 0; d.last_donation.day = atoi(argv[9]); d.last_donation.month = atoi(argv[10]); d.last_donation.year = atoi(argv[11]);
        if (!core_add_donor(&d)) return fail(core_last_error);
        puts("{\"ok\":true}"); return 0;
    }
    if (strcmp(argv[1], "update") == 0 && argc == 13) {
        memset(&d, 0, sizeof(d)); d.id = (uint32_t)strtoul(argv[2], NULL, 10);
        strncpy(d.name, argv[3], sizeof(d.name)-1); d.age = atoi(argv[4]); d.gender = argv[5][0]; d.weight = atof(argv[6]);
        strncpy(d.blood_group, argv[7], sizeof(d.blood_group)-1); strncpy(d.phone, argv[8], sizeof(d.phone)-1);
        d.has_donated_before = atoi(argv[9]) != 0; d.last_donation.day = atoi(argv[10]); d.last_donation.month = atoi(argv[11]); d.last_donation.year = atoi(argv[12]);
        if (!core_update_donor(&d)) return fail(core_last_error);
        puts("{\"ok\":true}"); return 0;
    }
    if (strcmp(argv[1], "inventory") == 0) {
        int i; printf("{\"ok\":true,\"total_units\":%u,\"stocks\":[", inv.total_units);
        for (i = 0; i < NUM_BLOOD_GROUPS; ++i) { if (i) putchar(','); printf("{\"blood_group\":\"%s\",\"units\":%u}", inv.stocks[i].blood_group, inv.stocks[i].units); }
        puts("]}"); return 0;
    }
    if ((strcmp(argv[1], "stock-add") == 0 || strcmp(argv[1], "stock-deduct") == 0) && argc == 4) {
        number = strtoul(argv[3], &end, 10); if (*end || number == 0) return fail("Invalid quantity");
        if (strcmp(argv[1], "stock-add") == 0 ? !core_add_units(&inv, argv[2], (uint32_t)number) : !core_deduct_units(&inv, argv[2], (uint32_t)number)) return fail(core_last_error);
        puts("{\"ok\":true}"); return 0;
    }
    return fail("Unknown command or incorrect arguments");
}
int main(int argc, char **argv) { return cli_run(argc, argv); }

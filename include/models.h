#ifndef MODELS_H
#define MODELS_H
#include <stdbool.h>
#include <stdint.h>
#define NUM_BLOOD_GROUPS 8
#define BLOOD_GROUP_LEN 4
#define NAME_LEN 64
#define PHONE_LEN 24
#define DATA_DIR "data"
#define DONOR_FILE "data/donors.dat"
#define INVENTORY_FILE "data/inventory.dat"
#define MIN_DONOR_AGE 18
#define MAX_DONOR_AGE 65
#define MIN_DONOR_WEIGHT 50.0
#define MIN_DONATION_INTERVAL 90

typedef struct { int day, month, year; } Date;
typedef struct {
    uint32_t id;
    char name[NAME_LEN];
    int age;
    char gender;
    double weight;
    char blood_group[BLOOD_GROUP_LEN];
    char phone[PHONE_LEN];
    Date last_donation;
    bool has_donated_before;
} Donor;
typedef struct { char blood_group[BLOOD_GROUP_LEN]; uint32_t units; } BloodStock;
typedef struct { BloodStock stocks[NUM_BLOOD_GROUPS]; uint32_t total_units; } BloodInventory;
extern const char *VALID_BLOOD_GROUPS[NUM_BLOOD_GROUPS];
#endif

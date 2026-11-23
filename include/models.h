//this file is a blue print and set of rules that the entire programme has to follow
 //two main structs defined here are donor , bloodstock,BloodInventory and date that will be used later

#ifndef MODELS_H
#define MODELS_H
#include <stdbool.h>  //gives us proper true and false boolean conditions instead of using wierd 0 and 1
#include <stdint.h>  // provides us with precision integer types like uint32_t so the precison remains same all across the system

#define NUM_BLOOD_GROUPS 8
#define BLOOD_GROUP_LEN 4 // it tells the blood group will require length four like ab- plus the null charaacter which c uses to know where the string ended
#define NAME_LEN 64
#define PHONE_LEN 24
// below are file paths hardcoded to tell where the data gets saved
#define DATA_DIR "data"
#define DONOR_FILE "data/donors.dat"
#define INVENTORY_FILE "data/inventory.dat"
#define MIN_DONOR_AGE 18
#define MAX_DONOR_AGE 65
#define MIN_DONOR_WEIGHT 50.0
#define MIN_DONATION_INTERVAL 90

typedef struct Date{
    int day, month, year;
} Date;
typedef struct Donor{
    uint32_t id; //unsigned integer 4 bytes only positive no and zero are stored not negative
    char name[NAME_LEN]; //name[64] defined above
    int age; //stored in 4 byte
    char gender; //stored in 1 byte
    double weight; //stored in 8 bytes
    char blood_group[BLOOD_GROUP_LEN]; //blood_group[4] defined above
    char phone[PHONE_LEN]; //phone[24]
    Date last_donation; //nested structure 
    bool has_donated_before;
} Donor;
typedef struct BloodStock{ 
char blood_group[BLOOD_GROUP_LEN];  
uint32_t units; 
} BloodStock;
typedef struct BloodInventory{
 BloodStock stocks[NUM_BLOOD_GROUPS];  //stocks[8]
uint32_t total_units;
 } BloodInventory;
extern const char *VALID_BLOOD_GROUPS[NUM_BLOOD_GROUPS]; //extern means this variable is only declared here and its real value is in other c file
#endif  //endif: Closes the header guard statement opened on line 1.
//line 44-> const char *VALID_BLOOD_GROUPS[NUM_BLOOD_GROUPS]; is a  pointer pointing towards a string which we cant change bcz blood groups cant be changed
//this file is a blue print and set of rules that the entire programme has to follow i.e the minimum age to donate blood etc it uses prerprocessor so declar all the things before anything starts (#) if anything goes against they replace/override it with what is already set
// //two main structs defined here are donor and date that will be used later

#ifndef MODELS_H  // #ifndef checks if the called header is only used once in the compilation process if it is already used it stops it so we dont get dupication errors
#define MODELS_H // define continues if the indef is right so it starts the furhter stages
#include <stdbool.h> //gives us proper true and false boolean conditions instead of using wierd 0 and 1
#include <stdint.h> // provides us with precision integer types like uint32_t so the precison remains same all across the system

//below are all the preprocessor defined which replace all there respective conditions and parts they need to be there we used this so if someone cahnges something we wont have to mannualy replace 1000s repetitions of same rule all across the program

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

//below are usual structs defined that will be used further in the program
// using type def lets you call the struct without typing "struct"
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
typedef struct { char blood_group[BLOOD_GROUP_LEN]; uint32_t units; } BloodStock; // Pairs a blood type string ("A-") with an integer count of how many bags (units) are available.

typedef struct { BloodStock stocks[NUM_BLOOD_GROUPS]; uint32_t total_units; } BloodInventory; // It creates an array of 8 BloodStock rows (stocks[8]), and adds a total_units counter at the bottom so you don't have to manually sum up all 8 types every time you want the grand total.

//below extern: Tells the compiler, "Hey, an array of strings called VALID_BLOOD_GROUPS exists somewhere else in another file. Don't build it here, just trust that it's out there."

extern const char *VALID_BLOOD_GROUPS[NUM_BLOOD_GROUPS];
#endif //endif: Closes the header guard statement opened on line 1.

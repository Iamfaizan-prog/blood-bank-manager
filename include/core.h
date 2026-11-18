// Stops core.h from being pasted twice and causing a compiler stroke. it checks if it has read this file already?
#ifndef WATERMELON // we have used watermelon here cuz it can be anything it is just a flag it checks for it sees if it is there if yes so skip the file if not put a flag/stickynote so next time the compiler reads it can see the flag and skip the file 
#define WATERMELON

#include "models.h" //includes the models.h file we made for structs to be used
#include <stddef.h> // //ts is a standard c file used for size Imports size_t, a specialized data type used for counting things safely (like array sizes and loop counters) without risking memory overflows.
extern char core_last_error[256]; //searches enitre codebase for error and if found prints it

// all the stuff below are function declarations that are defined in core.c file and are used in main.c file to perform various operations related to blood inventory and donor management.
// Using bool for explicit pass/fail readability and strict type safety to catch invalid return values at compile time.
bool core_init(BloodInventory *inventory); // a boolean to see if everything boots up and works or not 
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

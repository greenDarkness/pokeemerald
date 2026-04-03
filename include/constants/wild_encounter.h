#ifndef GUARD_CONSTANTS_WILD_ENCOUNTER_H
#define GUARD_CONSTANTS_WILD_ENCOUNTER_H

#define LAND_WILD_COUNT     12
#define WATER_WILD_COUNT    5
#define ROCK_WILD_COUNT     5
#define FISH_WILD_COUNT     10

#define NUM_ALTERING_CAVE_TABLES 9

// Time-of-day encounter availability bitmask.
// 0 = available at all times, otherwise a bitmask of allowed times.
#define TIME_OF_DAY_ENCOUNTER_ANY       0
#define TIME_OF_DAY_ENCOUNTER_NIGHT     (1 << 0) // TIME_OF_DAY_NIGHT
#define TIME_OF_DAY_ENCOUNTER_MORNING   (1 << 1) // TIME_OF_DAY_MORNING
#define TIME_OF_DAY_ENCOUNTER_DAY       (1 << 2) // TIME_OF_DAY_DAY

#endif // GUARD_CONSTANTS_WILD_ENCOUNTER_H

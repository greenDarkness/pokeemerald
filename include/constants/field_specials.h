#ifndef GUARD_CONSTANTS_FIELD_SPECIALS_H
#define GUARD_CONSTANTS_FIELD_SPECIALS_H

// Results of the specials that give a Pokemon a new PID (ChangePokemonNature, ChangePokemonGender,
// SwapPokemonAbility, ChangePokemonIV), see TryChangeMonPid in src/overrides.c.
#define PID_CHANGE_DONE           0 // Changed, color variation kept
#define PID_CHANGE_DONE_NEW_COLOR 1 // Changed, but no legal PID could keep its color variation
#define PID_CHANGE_NO_LEGAL_PID   2 // Its PID is tied to its IVs and no legal PID allows the change
#define PID_CHANGE_NOT_POSSIBLE   3 // Its species can't have the change (one gender or one ability)

// What those specials do, set in VAR_0x8007 before calling them
#define PID_CHANGE_MODE_CHECK       0 // Make the change if a legal PID allows it
#define PID_CHANGE_MODE_FORCE       1 // Make the change even if no legal PID allows it
#define PID_CHANGE_MODE_SUGGEST     2 // Change nothing; buffer in gStringVar4 other changes that would make it
                                      // legal, VAR_RESULT = PID_SUGGEST_*
#define PID_CHANGE_MODE_WITH_IV_FIX 3 // Make the change PID_SUGGEST_IV_FIX offered instead

// VAR_RESULT after PID_CHANGE_MODE_SUGGEST
#define PID_SUGGEST_NONE   0 // Nothing would make it legal
#define PID_SUGGEST_SHOWN  1 // Suggestions are in gStringVar4
#define PID_SUGGEST_IV_FIX 2 // Same, and the IV changer can make it legal himself (another value for the IV,
                             // or changing other IVs too): gStringVar2 asks whether to (then use
                             // PID_CHANGE_MODE_WITH_IV_FIX)

// PC Locations
#define PC_LOCATION_OTHER           0
#define PC_LOCATION_BRENDANS_HOUSE  1
#define PC_LOCATION_MAYS_HOUSE      2

// SS Tidal Locations
#define SS_TIDAL_LOCATION_CURRENTS  0
#define SS_TIDAL_LOCATION_SLATEPORT 1
#define SS_TIDAL_LOCATION_LILYCOVE  2
#define SS_TIDAL_LOCATION_ROUTE124  3
#define SS_TIDAL_LOCATION_ROUTE131  4

#define SS_TIDAL_BOARD_SLATEPORT     1
#define SS_TIDAL_DEPART_SLATEPORT    2
#define SS_TIDAL_HALFWAY_LILYCOVE    3
#define SS_TIDAL_LAND_LILYCOVE       4
#define SS_TIDAL_BOARD_LILYCOVE      5
#define SS_TIDAL_DEPART_LILYCOVE     6
#define SS_TIDAL_HALFWAY_SLATEPORT   7
#define SS_TIDAL_LAND_SLATEPORT      8
#define SS_TIDAL_EXIT_CURRENTS_RIGHT 9
#define SS_TIDAL_EXIT_CURRENTS_LEFT  10

#define SS_TIDAL_MAX_STEPS 205

// Scrollable Multichoice Menus
#define SCROLL_MULTI_NONE                                  0
#define SCROLL_MULTI_GLASS_WORKSHOP_VENDOR                 1
#define SCROLL_MULTI_POKEMON_FAN_CLUB_RATER                2
#define SCROLL_MULTI_BF_EXCHANGE_CORNER_DECOR_VENDOR_1     3
#define SCROLL_MULTI_BF_EXCHANGE_CORNER_DECOR_VENDOR_2     4
#define SCROLL_MULTI_BF_EXCHANGE_CORNER_VITAMIN_VENDOR     5
#define SCROLL_MULTI_BF_EXCHANGE_CORNER_HOLD_ITEM_VENDOR   6
#define SCROLL_MULTI_BERRY_POWDER_VENDOR                   7
#define SCROLL_MULTI_BF_RECEPTIONIST                       8
#define SCROLL_MULTI_BF_MOVE_TUTOR_1                       9
#define SCROLL_MULTI_BF_MOVE_TUTOR_2                      10
#define SCROLL_MULTI_SS_TIDAL_DESTINATION                 11
#define SCROLL_MULTI_BATTLE_TENT_RULES                    12
#define SCROLL_MULTI_NATURES                              13

#define MAX_SCROLL_MULTI_ON_SCREEN 6
#define MAX_SCROLL_MULTI_LENGTH 26

// Dept Store Floor Numbers
#define DEPT_STORE_FLOORNUM_B4F       0
#define DEPT_STORE_FLOORNUM_B3F       1
#define DEPT_STORE_FLOORNUM_B2F       2
#define DEPT_STORE_FLOORNUM_B1F       3
#define DEPT_STORE_FLOORNUM_1F        4
#define DEPT_STORE_FLOORNUM_2F        5
#define DEPT_STORE_FLOORNUM_3F        6
#define DEPT_STORE_FLOORNUM_4F        7
#define DEPT_STORE_FLOORNUM_5F        8
#define DEPT_STORE_FLOORNUM_6F        9
#define DEPT_STORE_FLOORNUM_7F       10
#define DEPT_STORE_FLOORNUM_8F       11
#define DEPT_STORE_FLOORNUM_9F       12
#define DEPT_STORE_FLOORNUM_10F      13
#define DEPT_STORE_FLOORNUM_11F      14
#define DEPT_STORE_FLOORNUM_ROOFTOP  15

// Lilycove Pokémon Trainer Fan Club
#define NUM_TRAINER_FAN_CLUB_MEMBERS  8

#define FANCLUB_GOT_FIRST_FANS 7
#define FANCLUB_MEMBER1        8
#define FANCLUB_MEMBER2        9
#define FANCLUB_MEMBER3        10
#define FANCLUB_MEMBER4        11
#define FANCLUB_MEMBER5        12
#define FANCLUB_MEMBER6        13
#define FANCLUB_MEMBER7        14
#define FANCLUB_MEMBER8        15

#define FANCOUNTER_DEFEATED_DRAKE    0
#define FANCOUNTER_BATTLED_AT_BASE   1
#define FANCOUNTER_FINISHED_CONTEST  2
#define FANCOUNTER_USED_BATTLE_TOWER 3

// Return values for DoDeoxysRockInteraction
#define DEOXYS_ROCK_FAILED     0
#define DEOXYS_ROCK_PROGRESSED 1
#define DEOXYS_ROCK_SOLVED     2
#define DEOXYS_ROCK_COMPLETE   3

#endif // GUARD_CONSTANTS_FIELD_SPECIALS_H

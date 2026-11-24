#ifndef GUARD_CONSTANTS_HERB_SHOP_H
#define GUARD_CONSTANTS_HERB_SHOP_H

// What the Petalburg Herb Shop owner's mission is about (special HerbShop_GetMission / HerbShop_GiveMission)
#define HERB_MISSION_NONE    0
#define HERB_MISSION_ESCORT  1 // lead her to a route's Hidden Grotto
#define HERB_MISSION_CATCH   2 // bring her a Pokemon for the shop
#define HERB_MISSION_FETCH   3 // bring her the ingredients of a remedy

// special HerbShop_GiveChosenMon
#define HERB_GIVE_MON_OK         0
#define HERB_GIVE_MON_WRONG      1 // an Egg or another species
#define HERB_GIVE_MON_LAST       2 // the player's only Pokemon that can battle

// Temp flags hiding the shop's objects, set by HerbShop_SetupShop on entering
#define FLAG_HIDE_HERB_SHOP_STAFF      FLAG_TEMP_1 // the clerk and the customers, while the shop is closed
#define FLAG_HIDE_HERB_SHOP_OWNER      FLAG_TEMP_2 // she is out on an escort mission
#define FLAG_HIDE_HERB_SHOP_SHROOMISH  FLAG_TEMP_3
#define FLAG_HIDE_HERB_SHOP_PARAS      FLAG_TEMP_4
// The shop's on-frame script thanks the player for the escort when this is 1
#define VAR_HERB_SHOP_ESCORT_RETURNED  VAR_TEMP_1

// In the Hidden Grotto room (data/maps/HiddenGrotto): the owner is hidden unless she led the
// player in, which also starts HerbShop_EventScript_EscortInGrotto
#define FLAG_HIDE_GROTTO_HERB_SHOP_OWNER  FLAG_TEMP_5
#define VAR_GROTTO_HERB_SHOP_ESCORT       VAR_TEMP_6

// special HerbShop_EscortMove (VAR_0x8004)
#define HERB_ESCORT_MOVE_WALK         0 // she walks to the grotto with the player a step behind
#define HERB_ESCORT_MOVE_OWNER_ENTER  1 // she steps into the grotto
#define HERB_ESCORT_MOVE_PLAYER_ENTER 2 // the player steps up to the grotto and faces it

#endif // GUARD_CONSTANTS_HERB_SHOP_H

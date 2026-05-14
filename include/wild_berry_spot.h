#ifndef GUARD_WILD_BERRY_SPOT_H
#define GUARD_WILD_BERRY_SPOT_H

#include "global.h"

// Number of globally-indexed wild berry spots. Each map object event
// that uses MOVEMENT_TYPE_WILD_BERRY_SPOT stores its spot ID (0-based,
// < WILD_BERRY_SPOT_COUNT) in the trainerRange_berryTreeId field.
#define WILD_BERRY_SPOT_COUNT 64

// Called from DoTimeBasedEvents() in clock.c on every time-based update.
// Advances flowering spots to grown on the MORNING->DAY transition (50% chance each).
void WildBerrySpotTimeUpdate(void);

// Returns the hardcoded berry type number (1-based, ITEM_TO_BERRY scale) for a spot.
u8 GetWildBerrySpotBerryType(u8 spotId);

// Returns TRUE if spot is fully grown (ready to pick).
bool8 IsWildBerrySpotGrown(u8 spotId);

// Resets spot to the flowering stage (called after picking).
void SetWildBerrySpotFlowering(u8 spotId);

// Script specials registered in data/specials.inc.
// WildBerrySpotInteraction: sets gSpecialVar_0x8004 to 0 (flowering) or 1 (grown)
//   and copies the berry name into gStringVar1.
void WildBerrySpotInteraction(void);

// WildBerrySpotPick: adds one berry item to the bag; sets gSpecialVar_0x8004 to
//   TRUE if the item was successfully added, FALSE if the bag is full.
//   Resets the spot to flowering on success.
void WildBerrySpotPick(void);

// Movement-type callback. Implemented in event_object_movement.c because it
// needs access to the file-private SetBerryTreeGraphics helper.
void MovementType_WildBerrySpot(struct Sprite *sprite);

#endif // GUARD_WILD_BERRY_SPOT_H

#ifndef GUARD_DAILY_HIDDEN_ITEMS_H
#define GUARD_DAILY_HIDDEN_ITEMS_H

#include "constants/flags.h"

// Check if a hidden item ID represents a daily hidden item
bool8 IsDailyHiddenItemId(u16 hiddenItemId);

// Check if a specific daily hidden item spot is active today
// Returns TRUE if this is the active spot for its group today AND hasn't been collected
bool8 IsDailyHiddenSpotActive(u16 hiddenItemId);

// Get the random item for a daily hidden item group today
u16 GetDailyHiddenItem(u8 groupIndex);

// Get the daily flag for a group (to set when item is collected)
u16 GetDailyHiddenFlag(u8 groupIndex);

// Extract group index from hidden item ID
u8 GetDailyHiddenGroupIndex(u16 hiddenItemId);

// Extract spot index from hidden item ID
u8 GetDailyHiddenSpotIndex(u16 hiddenItemId);

#endif // GUARD_DAILY_HIDDEN_ITEMS_H

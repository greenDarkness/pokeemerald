#include "global.h"
#include "berry.h"
#include "event_data.h"
#include "global.fieldmap.h"
#include "item.h"
#include "overworld.h"
#include "random.h"
#include "rtc.h"
#include "string_util.h"
#include "wild_berry_spot.h"
#include "constants/berry.h"
#include "constants/items.h"

// ---------------------------------------------------------------------------
// ROM: hardcoded berry type per spot (1-based ITEM_TO_BERRY index).
// Edit this table to assign a berry type to each spot ID used on your maps.
// ---------------------------------------------------------------------------
static const u8 sWildBerrySpotBerryTypes[WILD_BERRY_SPOT_COUNT] = {
    [0]  = ITEM_TO_BERRY(ITEM_ORAN_BERRY),
    [1]  = ITEM_TO_BERRY(ITEM_PECHA_BERRY),
    [2]  = ITEM_TO_BERRY(ITEM_CHERI_BERRY),
    [3]  = ITEM_TO_BERRY(ITEM_RAWST_BERRY),
    [4]  = ITEM_TO_BERRY(ITEM_ASPEAR_BERRY),
    [5]  = ITEM_TO_BERRY(ITEM_LEPPA_BERRY),
    [6]  = ITEM_TO_BERRY(ITEM_SITRUS_BERRY),
    [7]  = ITEM_TO_BERRY(ITEM_PERSIM_BERRY),
    // Remaining spots default to CHERI_BERRY (value 0 in the array maps to
    // ITEM_TO_BERRY(ITEM_CHERI_BERRY) via the else branch in BerryTypeToItemId,
    // but we treat type=0 as "no berry defined" — fall back to type 1).
};

// ---------------------------------------------------------------------------
// EWRAM: volatile state — zeroed on cold boot, not saved.
// sWildBerrySpotGrown: non-zero means the spot is fully grown (ready to pick).
// sWildBerryLastTimeBand: last observed gTimeOfDay value; 0 = NIGHT on boot.
// ---------------------------------------------------------------------------
EWRAM_DATA static u8 sWildBerrySpotGrown[WILD_BERRY_SPOT_COUNT] = {0};
EWRAM_DATA static u8 sWildBerryLastTimeBand = 0;

// ---------------------------------------------------------------------------
// Public accessors
// ---------------------------------------------------------------------------

u8 GetWildBerrySpotBerryType(u8 spotId)
{
    u8 berryType;

    if (spotId >= WILD_BERRY_SPOT_COUNT)
        return ITEM_TO_BERRY(ITEM_ORAN_BERRY);

    berryType = sWildBerrySpotBerryTypes[spotId];
    // A zero entry means the developer left the slot empty; default to Oran Berry.
    if (berryType == 0)
        return ITEM_TO_BERRY(ITEM_ORAN_BERRY);

    return berryType;
}

bool8 IsWildBerrySpotGrown(u8 spotId)
{
    if (spotId >= WILD_BERRY_SPOT_COUNT)
        return FALSE;
    return sWildBerrySpotGrown[spotId] != 0;
}

void SetWildBerrySpotFlowering(u8 spotId)
{
    if (spotId < WILD_BERRY_SPOT_COUNT)
        sWildBerrySpotGrown[spotId] = 0;
}

// ---------------------------------------------------------------------------
// Time-based update (called from DoTimeBasedEvents in clock.c)
// ---------------------------------------------------------------------------

void WildBerrySpotTimeUpdate(void)
{
    u8 currentBand;
    u8 i;

    currentBand = gTimeOfDay;

    // On MORNING -> DAY transition, each flowering spot has a 50% chance to grow.
    if (sWildBerryLastTimeBand == TIME_OF_DAY_MORNING && currentBand == TIME_OF_DAY_DAY)
    {
        for (i = 0; i < WILD_BERRY_SPOT_COUNT; i++)
        {
            if (sWildBerrySpotGrown[i] == 0 && (Random() & 1) == 0)
                sWildBerrySpotGrown[i] = 1;
        }
    }

    sWildBerryLastTimeBand = currentBand;
}

// ---------------------------------------------------------------------------
// Script specials
// ---------------------------------------------------------------------------

// WildBerrySpotInteraction:
//   gSpecialVar_0x8004 = 0 (flowering) or 1 (fully grown)
//   gStringVar1        = berry name (always set so scripts can display it)
void WildBerrySpotInteraction(void)
{
    u8 spotId    = gObjectEvents[gSelectedObjectEvent].trainerRange_berryTreeId;
    u8 berryType = GetWildBerrySpotBerryType(spotId);
    u16 itemId   = berryType + FIRST_BERRY_INDEX - 1;

    gSpecialVar_0x8004 = IsWildBerrySpotGrown(spotId) ? 1 : 0;
    gSpecialVar_0x8005 = itemId;
    GetBerryNameByBerryType(berryType, gStringVar1);
}

// WildBerrySpotPick:
//   Attempts to add one berry item to the bag.
//   gSpecialVar_0x8004 = TRUE on success, FALSE if bag is full.
//   On success the spot reverts to flowering.
void WildBerrySpotPick(void)
{
    u8  spotId    = gObjectEvents[gSelectedObjectEvent].trainerRange_berryTreeId;
    u8  berryType = GetWildBerrySpotBerryType(spotId);
    u16 itemId    = berryType + FIRST_BERRY_INDEX - 1;

    if (AddBagItem(itemId, 1) == TRUE)
    {
        SetWildBerrySpotFlowering(spotId);
        gSpecialVar_0x8004 = TRUE;
    }
    else
    {
        gSpecialVar_0x8004 = FALSE;
    }
}

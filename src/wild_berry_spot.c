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

    // HOENN
    [0]  = ITEM_TO_BERRY(ITEM_CHERI_BERRY),
    [1]  = ITEM_TO_BERRY(ITEM_CHERI_BERRY),
    [2]  = ITEM_TO_BERRY(ITEM_CHESTO_BERRY),
    [3]  = ITEM_TO_BERRY(ITEM_CHESTO_BERRY),
    [4]  = ITEM_TO_BERRY(ITEM_PECHA_BERRY),
    [5]  = ITEM_TO_BERRY(ITEM_PECHA_BERRY),
    [6]  = ITEM_TO_BERRY(ITEM_RAWST_BERRY),
    [7]  = ITEM_TO_BERRY(ITEM_RAWST_BERRY),
    [8]  = ITEM_TO_BERRY(ITEM_ASPEAR_BERRY),
    [9]  = ITEM_TO_BERRY(ITEM_ASPEAR_BERRY),
    [10]  = ITEM_TO_BERRY(ITEM_LEPPA_BERRY),
    [11]  = ITEM_TO_BERRY(ITEM_LEPPA_BERRY),
    [12]  = ITEM_TO_BERRY(ITEM_ORAN_BERRY),
    [13]  = ITEM_TO_BERRY(ITEM_ORAN_BERRY),
    [14]  = ITEM_TO_BERRY(ITEM_PERSIM_BERRY),
    [15]  = ITEM_TO_BERRY(ITEM_PERSIM_BERRY),
    [16]  = ITEM_TO_BERRY(ITEM_LUM_BERRY),
    [17]  = ITEM_TO_BERRY(ITEM_LUM_BERRY),
    [18]  = ITEM_TO_BERRY(ITEM_SITRUS_BERRY),
    [19]  = ITEM_TO_BERRY(ITEM_SITRUS_BERRY),
    [20]  = ITEM_TO_BERRY(ITEM_MAGO_BERRY),
    [21]  = ITEM_TO_BERRY(ITEM_MAGO_BERRY),
    [22]  = ITEM_TO_BERRY(ITEM_MAGO_BERRY),
    [23]  = ITEM_TO_BERRY(ITEM_MAGO_BERRY),
    [24]  = ITEM_TO_BERRY(ITEM_RAZZ_BERRY),
    [25]  = ITEM_TO_BERRY(ITEM_RAZZ_BERRY),
    [26]  = ITEM_TO_BERRY(ITEM_BLUK_BERRY),
    [27]  = ITEM_TO_BERRY(ITEM_BLUK_BERRY),
    [28]  = ITEM_TO_BERRY(ITEM_NANAB_BERRY),
    [29]  = ITEM_TO_BERRY(ITEM_NANAB_BERRY),
    [30]  = ITEM_TO_BERRY(ITEM_WEPEAR_BERRY),
    [31]  = ITEM_TO_BERRY(ITEM_WEPEAR_BERRY),
    [32]  = ITEM_TO_BERRY(ITEM_PINAP_BERRY),
    [33]  = ITEM_TO_BERRY(ITEM_PINAP_BERRY),
    [34]  = ITEM_TO_BERRY(ITEM_POMEG_BERRY),
    [35]  = ITEM_TO_BERRY(ITEM_POMEG_BERRY),
    [36]  = ITEM_TO_BERRY(ITEM_TAMATO_BERRY),
    [37]  = ITEM_TO_BERRY(ITEM_TAMATO_BERRY),
    [38]  = ITEM_TO_BERRY(ITEM_SALAC_BERRY),
    [39]  = ITEM_TO_BERRY(ITEM_SALAC_BERRY),
    [40]  = ITEM_TO_BERRY(ITEM_LANSAT_BERRY),
    [41]  = ITEM_TO_BERRY(ITEM_LANSAT_BERRY),

    // KANTO
    [42]  = ITEM_TO_BERRY(ITEM_CHERI_BERRY),
    [43]  = ITEM_TO_BERRY(ITEM_CHERI_BERRY),
    [44]  = ITEM_TO_BERRY(ITEM_CHESTO_BERRY),
    [45]  = ITEM_TO_BERRY(ITEM_CHESTO_BERRY),
    [46]  = ITEM_TO_BERRY(ITEM_PECHA_BERRY),
    [47]  = ITEM_TO_BERRY(ITEM_PECHA_BERRY),
    [48]  = ITEM_TO_BERRY(ITEM_RAWST_BERRY),
    [49]  = ITEM_TO_BERRY(ITEM_RAWST_BERRY),
    [50]  = ITEM_TO_BERRY(ITEM_ASPEAR_BERRY),
    [51]  = ITEM_TO_BERRY(ITEM_ASPEAR_BERRY),
    [52]  = ITEM_TO_BERRY(ITEM_LEPPA_BERRY),
    [53]  = ITEM_TO_BERRY(ITEM_LEPPA_BERRY),
    [54]  = ITEM_TO_BERRY(ITEM_ORAN_BERRY),
    [55]  = ITEM_TO_BERRY(ITEM_ORAN_BERRY),
    [56]  = ITEM_TO_BERRY(ITEM_PERSIM_BERRY),
    [57]  = ITEM_TO_BERRY(ITEM_PERSIM_BERRY),
    [58]  = ITEM_TO_BERRY(ITEM_LUM_BERRY),
    [59]  = ITEM_TO_BERRY(ITEM_LUM_BERRY),
    [60]  = ITEM_TO_BERRY(ITEM_SITRUS_BERRY),
    [61]  = ITEM_TO_BERRY(ITEM_SITRUS_BERRY),
    [62]  = ITEM_TO_BERRY(ITEM_FIGY_BERRY),
    [63]  = ITEM_TO_BERRY(ITEM_FIGY_BERRY),
    [64]  = ITEM_TO_BERRY(ITEM_IAPAPA_BERRY),
    [65]  = ITEM_TO_BERRY(ITEM_IAPAPA_BERRY),
    [66]  = ITEM_TO_BERRY(ITEM_CORNN_BERRY),
    [67]  = ITEM_TO_BERRY(ITEM_CORNN_BERRY),
    [68]  = ITEM_TO_BERRY(ITEM_MAGOST_BERRY),
    [69]  = ITEM_TO_BERRY(ITEM_MAGOST_BERRY),
    [70]  = ITEM_TO_BERRY(ITEM_RABUTA_BERRY),
    [71]  = ITEM_TO_BERRY(ITEM_RABUTA_BERRY),
    [72]  = ITEM_TO_BERRY(ITEM_NOMEL_BERRY),
    [73]  = ITEM_TO_BERRY(ITEM_NOMEL_BERRY),
    [74]  = ITEM_TO_BERRY(ITEM_SPELON_BERRY),
    [75]  = ITEM_TO_BERRY(ITEM_SPELON_BERRY),
    [76]  = ITEM_TO_BERRY(ITEM_HONDEW_BERRY),
    [77]  = ITEM_TO_BERRY(ITEM_HONDEW_BERRY),
    [78]  = ITEM_TO_BERRY(ITEM_GREPA_BERRY),
    [79]  = ITEM_TO_BERRY(ITEM_GREPA_BERRY),
    [80]  = ITEM_TO_BERRY(ITEM_LIECHI_BERRY),
    [81]  = ITEM_TO_BERRY(ITEM_LIECHI_BERRY),
    [82]  = ITEM_TO_BERRY(ITEM_GANLON_BERRY),
    [83]  = ITEM_TO_BERRY(ITEM_GANLON_BERRY),

    // JOHTO
    [84]  = ITEM_TO_BERRY(ITEM_CHERI_BERRY),
    [85]  = ITEM_TO_BERRY(ITEM_CHERI_BERRY),
    [86]  = ITEM_TO_BERRY(ITEM_CHESTO_BERRY),
    [87]  = ITEM_TO_BERRY(ITEM_CHESTO_BERRY),
    [88]  = ITEM_TO_BERRY(ITEM_PECHA_BERRY),
    [89]  = ITEM_TO_BERRY(ITEM_PECHA_BERRY),
    [90]  = ITEM_TO_BERRY(ITEM_RAWST_BERRY),
    [91]  = ITEM_TO_BERRY(ITEM_RAWST_BERRY),
    [92]  = ITEM_TO_BERRY(ITEM_ASPEAR_BERRY),
    [93]  = ITEM_TO_BERRY(ITEM_ASPEAR_BERRY),
    [94]  = ITEM_TO_BERRY(ITEM_LEPPA_BERRY),
    [95]  = ITEM_TO_BERRY(ITEM_LEPPA_BERRY),
    [96]  = ITEM_TO_BERRY(ITEM_ORAN_BERRY),
    [97]  = ITEM_TO_BERRY(ITEM_ORAN_BERRY),
    [98]  = ITEM_TO_BERRY(ITEM_PERSIM_BERRY),
    [99]  = ITEM_TO_BERRY(ITEM_PERSIM_BERRY),
    [100]  = ITEM_TO_BERRY(ITEM_LUM_BERRY),
    [101]  = ITEM_TO_BERRY(ITEM_LUM_BERRY),
    [102]  = ITEM_TO_BERRY(ITEM_SITRUS_BERRY),
    [103]  = ITEM_TO_BERRY(ITEM_SITRUS_BERRY),
    [104]  = ITEM_TO_BERRY(ITEM_WIKI_BERRY),
    [105]  = ITEM_TO_BERRY(ITEM_WIKI_BERRY),
    [106]  = ITEM_TO_BERRY(ITEM_AGUAV_BERRY),
    [107]  = ITEM_TO_BERRY(ITEM_AGUAV_BERRY),
    [108]  = ITEM_TO_BERRY(ITEM_SPELON_BERRY),
    [109]  = ITEM_TO_BERRY(ITEM_SPELON_BERRY),
    [110]  = ITEM_TO_BERRY(ITEM_PAMTRE_BERRY),
    [111]  = ITEM_TO_BERRY(ITEM_PAMTRE_BERRY),
    [112]  = ITEM_TO_BERRY(ITEM_WATMEL_BERRY),
    [113]  = ITEM_TO_BERRY(ITEM_WATMEL_BERRY),
    [114]  = ITEM_TO_BERRY(ITEM_DURIN_BERRY),
    [115]  = ITEM_TO_BERRY(ITEM_DURIN_BERRY),
    [116]  = ITEM_TO_BERRY(ITEM_BELUE_BERRY),
    [117]  = ITEM_TO_BERRY(ITEM_BELUE_BERRY),
    [118]  = ITEM_TO_BERRY(ITEM_KELPSY_BERRY),
    [119]  = ITEM_TO_BERRY(ITEM_KELPSY_BERRY),
    [120]  = ITEM_TO_BERRY(ITEM_QUALOT_BERRY),
    [121]  = ITEM_TO_BERRY(ITEM_QUALOT_BERRY),
    [122]  = ITEM_TO_BERRY(ITEM_PETAYA_BERRY),
    [123]  = ITEM_TO_BERRY(ITEM_PETAYA_BERRY),
    [124]  = ITEM_TO_BERRY(ITEM_APICOT_BERRY),
    [125]  = ITEM_TO_BERRY(ITEM_APICOT_BERRY),

    // OTHER
    [126]  = ITEM_TO_BERRY(ITEM_STARF_BERRY),
    [127]  = ITEM_TO_BERRY(ITEM_STARF_BERRY),
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

    RtcCalcLocalTime();
    if (gLocalTime.hours >= 4 && gLocalTime.hours <= 9)
        currentBand = TIME_OF_DAY_MORNING;
    else if (gLocalTime.hours >= 10 && gLocalTime.hours <= 19)
        currentBand = TIME_OF_DAY_DAY;
    else
        currentBand = TIME_OF_DAY_NIGHT;

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
//   gStringVar2        = flowering adverb ("prettily" / "cutely" /
//                        "very beautifully"), chosen deterministically per spot
//                        so each spot uses the same wording every time.
void WildBerrySpotInteraction(void)
{
    static const u8 sAdverbPrettily[] = _("prettily");
    static const u8 sAdverbCutely[] = _("cutely");
    static const u8 sAdverbBeautifully[] = _("very beautifully");
    static const u8 *const sAdverbs[] = {
        sAdverbPrettily,
        sAdverbCutely,
        sAdverbBeautifully,
    };

    u8 spotId    = gObjectEvents[gSelectedObjectEvent].trainerRange_berryTreeId;
    u8 berryType = GetWildBerrySpotBerryType(spotId);
    u16 itemId   = berryType + FIRST_BERRY_INDEX - 1;

    gSpecialVar_0x8004 = IsWildBerrySpotGrown(spotId) ? 1 : 0;
    gSpecialVar_0x8005 = itemId;
    GetBerryNameByBerryType(berryType, gStringVar1);
    StringCopy(gStringVar2, sAdverbs[spotId % ARRAY_COUNT(sAdverbs)]);
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

#include "global.h"
#include "battle.h"
#include "event_data.h"
#include "pokemon.h"
#include "random.h"
#include "roamer.h"
#include "constants/battle.h"

// Latias or Latios roams Hoenn; Raikou and Entei roam Johto once the Burned Tower has released them
// (InitBeastRoamers, FLAG_RELEASED_BEASTS).
enum
{
    ROAMER_LATI,
    ROAMER_RAIKOU,
    ROAMER_ENTEI,
    ROAMER_COUNT,
};

// Latias and Latios are Lv50 rather than the original Lv40: when caught, they get the met data of
// the stationary Latios/Latias on Southern Island in Ruby/Sapphire, which is Lv50. Raikou and
// Entei get that of Colosseum's, which are Lv40 (see src/data/overrides.h).
#define ROAMER_LEVEL 50
#define BEAST_ROAMER_LEVEL 40

enum
{
    MAP_GRP, // map group
    MAP_NUM, // map number
};

EWRAM_DATA static u8 sLocationHistory[3][2] = {0};
EWRAM_DATA static u8 sRoamerLocation[ROAMER_COUNT][2] = {0};
// The roamer of the current or last roamer battle
EWRAM_DATA static u8 sCurrentRoamer = 0;

#define ___ MAP_NUM(MAP_UNDEFINED) // For empty spots in the location table

// Note: There are two potential softlocks that can occur with this table if its maps are
//       changed in particular ways. They can be avoided by ensuring the following:
//       - There must be at least 2 location sets that start with a different map,
//         i.e. every location set cannot start with the same map. This is because of
//         the while loop in RoamerMoveToOtherLocationSet.
//       - Each location set must have at least 3 unique maps. This is because of
//         the while loop in RoamerMove. In this loop the first map in the set is
//         ignored, and an additional map is ignored if the roamer was there recently.
//       - Additionally, while not a softlock, it's worth noting that if for any
//         map in the location table there is not a location set that starts with
//         that map then the roamer will be significantly less likely to move away
//         from that map when it lands there.
#define NUM_LOCATIONS_PER_SET 6

static const u8 sRoamerLocations[][NUM_LOCATIONS_PER_SET] =
{
    { MAP_NUM(MAP_ROUTE110), MAP_NUM(MAP_ROUTE111), MAP_NUM(MAP_ROUTE117), MAP_NUM(MAP_ROUTE118), MAP_NUM(MAP_ROUTE134), ___ },
    { MAP_NUM(MAP_ROUTE111), MAP_NUM(MAP_ROUTE110), MAP_NUM(MAP_ROUTE117), MAP_NUM(MAP_ROUTE118), ___, ___ },
    { MAP_NUM(MAP_ROUTE117), MAP_NUM(MAP_ROUTE111), MAP_NUM(MAP_ROUTE110), MAP_NUM(MAP_ROUTE118), ___, ___ },
    { MAP_NUM(MAP_ROUTE118), MAP_NUM(MAP_ROUTE117), MAP_NUM(MAP_ROUTE110), MAP_NUM(MAP_ROUTE111), MAP_NUM(MAP_ROUTE119), MAP_NUM(MAP_ROUTE123) },
    { MAP_NUM(MAP_ROUTE119), MAP_NUM(MAP_ROUTE118), MAP_NUM(MAP_ROUTE120), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE120), MAP_NUM(MAP_ROUTE119), MAP_NUM(MAP_ROUTE121), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE121), MAP_NUM(MAP_ROUTE120), MAP_NUM(MAP_ROUTE122), MAP_NUM(MAP_ROUTE123), ___, ___ },
    { MAP_NUM(MAP_ROUTE122), MAP_NUM(MAP_ROUTE121), MAP_NUM(MAP_ROUTE123), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE123), MAP_NUM(MAP_ROUTE122), MAP_NUM(MAP_ROUTE118), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE124), MAP_NUM(MAP_ROUTE121), MAP_NUM(MAP_ROUTE125), MAP_NUM(MAP_ROUTE126), ___, ___ },
    { MAP_NUM(MAP_ROUTE125), MAP_NUM(MAP_ROUTE124), MAP_NUM(MAP_ROUTE127), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE126), MAP_NUM(MAP_ROUTE124), MAP_NUM(MAP_ROUTE127), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE127), MAP_NUM(MAP_ROUTE125), MAP_NUM(MAP_ROUTE126), MAP_NUM(MAP_ROUTE128), ___, ___ },
    { MAP_NUM(MAP_ROUTE128), MAP_NUM(MAP_ROUTE127), MAP_NUM(MAP_ROUTE129), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE129), MAP_NUM(MAP_ROUTE128), MAP_NUM(MAP_ROUTE130), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE130), MAP_NUM(MAP_ROUTE129), MAP_NUM(MAP_ROUTE131), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE131), MAP_NUM(MAP_ROUTE130), MAP_NUM(MAP_ROUTE132), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE132), MAP_NUM(MAP_ROUTE131), MAP_NUM(MAP_ROUTE133), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE133), MAP_NUM(MAP_ROUTE132), MAP_NUM(MAP_ROUTE134), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE134), MAP_NUM(MAP_ROUTE133), MAP_NUM(MAP_ROUTE110), ___, ___, ___ },
    { ___, ___, ___, ___, ___, ___ },
};

// Johto: the routes the roamers use in HeartGold and SoulSilver, and which of them border each.
static const u8 sJohtoRoamerLocations[][NUM_LOCATIONS_PER_SET] =
{
    { MAP_NUM(MAP_ROUTE29), MAP_NUM(MAP_ROUTE30), MAP_NUM(MAP_ROUTE46), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE30), MAP_NUM(MAP_ROUTE29), MAP_NUM(MAP_ROUTE31), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE31), MAP_NUM(MAP_ROUTE30), MAP_NUM(MAP_ROUTE32), MAP_NUM(MAP_ROUTE36), ___, ___ },
    { MAP_NUM(MAP_ROUTE32), MAP_NUM(MAP_ROUTE31), MAP_NUM(MAP_ROUTE33), MAP_NUM(MAP_ROUTE36), ___, ___ },
    { MAP_NUM(MAP_ROUTE33), MAP_NUM(MAP_ROUTE32), MAP_NUM(MAP_ROUTE34), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE34), MAP_NUM(MAP_ROUTE33), MAP_NUM(MAP_ROUTE35), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE35), MAP_NUM(MAP_ROUTE34), MAP_NUM(MAP_ROUTE36), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE36), MAP_NUM(MAP_ROUTE31), MAP_NUM(MAP_ROUTE32), MAP_NUM(MAP_ROUTE37), ___, ___ },
    { MAP_NUM(MAP_ROUTE37), MAP_NUM(MAP_ROUTE36), MAP_NUM(MAP_ROUTE38), MAP_NUM(MAP_ROUTE42), ___, ___ },
    { MAP_NUM(MAP_ROUTE38), MAP_NUM(MAP_ROUTE37), MAP_NUM(MAP_ROUTE39), MAP_NUM(MAP_ROUTE42), ___, ___ },
    { MAP_NUM(MAP_ROUTE39), MAP_NUM(MAP_ROUTE38), ___, ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE42), MAP_NUM(MAP_ROUTE37), MAP_NUM(MAP_ROUTE38), MAP_NUM(MAP_ROUTE43), MAP_NUM(MAP_ROUTE44), ___ },
    { MAP_NUM(MAP_ROUTE43), MAP_NUM(MAP_ROUTE42), MAP_NUM(MAP_ROUTE44), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE44), MAP_NUM(MAP_ROUTE42), MAP_NUM(MAP_ROUTE43), MAP_NUM(MAP_ROUTE46), ___, ___ },
    { MAP_NUM(MAP_ROUTE45), MAP_NUM(MAP_ROUTE44), MAP_NUM(MAP_ROUTE46), ___, ___, ___ },
    { MAP_NUM(MAP_ROUTE46), MAP_NUM(MAP_ROUTE29), MAP_NUM(MAP_ROUTE45), ___, ___, ___ },
};

#undef ___
#define NO_LOCATION MAP_NUM(MAP_UNDEFINED)

struct RoamerInfo
{
    u16 species;   // SPECIES_NONE: the species is chosen when the roamer is created
    u16 caughtFlag;
    u8 level;
    u8 mapGroup;
    u8 numLocationSets;
    const u8 (*locations)[NUM_LOCATIONS_PER_SET];
};

static const struct RoamerInfo sRoamerInfo[ROAMER_COUNT] =
{
    [ROAMER_LATI] =
    {
        .level = ROAMER_LEVEL,
        .mapGroup = MAP_GROUP(MAP_ROUTE110),
        .numLocationSets = ARRAY_COUNT(sRoamerLocations) - 1, // the last one is empty
        .locations = sRoamerLocations,
    },
    [ROAMER_RAIKOU] =
    {
        .species = SPECIES_RAIKOU,
        .caughtFlag = FLAG_CAUGHT_RAIKOU,
        .level = BEAST_ROAMER_LEVEL,
        .mapGroup = MAP_GROUP(MAP_ROUTE29),
        .numLocationSets = ARRAY_COUNT(sJohtoRoamerLocations),
        .locations = sJohtoRoamerLocations,
    },
    [ROAMER_ENTEI] =
    {
        .species = SPECIES_ENTEI,
        .caughtFlag = FLAG_CAUGHT_ENTEI,
        .level = BEAST_ROAMER_LEVEL,
        .mapGroup = MAP_GROUP(MAP_ROUTE29),
        .numLocationSets = ARRAY_COUNT(sJohtoRoamerLocations),
        .locations = sJohtoRoamerLocations,
    },
};

static struct Roamer *GetRoamer(u32 id)
{
    if (id == ROAMER_LATI)
        return &gSaveBlock1Ptr->roamer;
    return &gSaveBlock1Ptr->beastRoamers[id - ROAMER_RAIKOU];
}

// Raikou's and Entei's data was free space before, which can hold anything on a save from then:
// it only means something once they have been released and it has been filled in.
static bool32 IsBeastRoamerCreated(u32 id)
{
    struct Roamer *roamer = GetRoamer(id);

    return FlagGet(FLAG_RELEASED_BEASTS)
        && roamer->species == sRoamerInfo[id].species
        && roamer->level == sRoamerInfo[id].level
        && roamer->active <= TRUE;
}

static bool32 IsRoamerActive(u32 id)
{
    if (id != ROAMER_LATI && !IsBeastRoamerCreated(id))
        return FALSE;
    return GetRoamer(id)->active == TRUE;
}

static void ClearRoamer(u32 id)
{
    memset(GetRoamer(id), 0, sizeof(struct Roamer));
    sRoamerLocation[id][MAP_GRP] = 0;
    sRoamerLocation[id][MAP_NUM] = 0;
}

void ClearRoamerData(void)
{
    u32 i;

    for (i = 0; i < ROAMER_COUNT; i++)
        ClearRoamer(i);
    GetRoamer(ROAMER_LATI)->species = SPECIES_LATIAS;
}

void ClearRoamerLocationData(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sLocationHistory); i++)
    {
        sLocationHistory[i][MAP_GRP] = 0;
        sLocationHistory[i][MAP_NUM] = 0;
    }

    for (i = 0; i < ROAMER_COUNT; i++)
    {
        sRoamerLocation[i][MAP_GRP] = 0;
        sRoamerLocation[i][MAP_NUM] = 0;
    }
}

static void CreateInitialRoamerMon(u32 id, u16 species)
{
    const struct RoamerInfo *info = &sRoamerInfo[id];
    struct Roamer *roamer = GetRoamer(id);

    ClearRoamer(id);
    roamer->species = species;
    CreateMon(&gEnemyParty[0], species, info->level, USE_RANDOM_IVS, FALSE, 0, OT_ID_PLAYER_ID, 0);
    roamer->level = info->level;
    roamer->status = 0;
    roamer->active = TRUE;
    roamer->ivs = GetMonData(&gEnemyParty[0], MON_DATA_IVS);
    roamer->personality = GetMonData(&gEnemyParty[0], MON_DATA_PERSONALITY);
    roamer->hp = GetMonData(&gEnemyParty[0], MON_DATA_MAX_HP);
    roamer->cool = GetMonData(&gEnemyParty[0], MON_DATA_COOL);
    roamer->beauty = GetMonData(&gEnemyParty[0], MON_DATA_BEAUTY);
    roamer->cute = GetMonData(&gEnemyParty[0], MON_DATA_CUTE);
    roamer->smart = GetMonData(&gEnemyParty[0], MON_DATA_SMART);
    roamer->tough = GetMonData(&gEnemyParty[0], MON_DATA_TOUGH);
    sRoamerLocation[id][MAP_GRP] = info->mapGroup;
    sRoamerLocation[id][MAP_NUM] = info->locations[Random() % info->numLocationSets][0];
}

// gSpecialVar_0x8004 here corresponds to the options in the multichoice MULTI_TV_LATI (0 for 'Red', 1 for 'Blue')
void InitRoamer(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sLocationHistory); i++)
    {
        sLocationHistory[i][MAP_GRP] = 0;
        sLocationHistory[i][MAP_NUM] = 0;
    }
    CreateInitialRoamerMon(ROAMER_LATI, gSpecialVar_0x8004 ? SPECIES_LATIOS : SPECIES_LATIAS);
}

// Raikou and Entei start roaming Johto. The script sets FLAG_RELEASED_BEASTS.
void InitBeastRoamers(void)
{
    CreateInitialRoamerMon(ROAMER_RAIKOU, SPECIES_RAIKOU);
    CreateInitialRoamerMon(ROAMER_ENTEI, SPECIES_ENTEI);
}

// On a save where the beasts were released before they roamed, they start now.
static void TryCreateMissingBeastRoamers(void)
{
    u32 id;

    if (!FlagGet(FLAG_RELEASED_BEASTS))
        return;
    for (id = ROAMER_RAIKOU; id < ROAMER_COUNT; id++)
    {
        if (!IsBeastRoamerCreated(id))
            CreateInitialRoamerMon(id, sRoamerInfo[id].species);
    }
}

void UpdateLocationHistoryForRoamer(void)
{
    TryCreateMissingBeastRoamers();

    sLocationHistory[2][MAP_GRP] = sLocationHistory[1][MAP_GRP];
    sLocationHistory[2][MAP_NUM] = sLocationHistory[1][MAP_NUM];

    sLocationHistory[1][MAP_GRP] = sLocationHistory[0][MAP_GRP];
    sLocationHistory[1][MAP_NUM] = sLocationHistory[0][MAP_NUM];

    sLocationHistory[0][MAP_GRP] = gSaveBlock1Ptr->location.mapGroup;
    sLocationHistory[0][MAP_NUM] = gSaveBlock1Ptr->location.mapNum;
}

static void MoveRoamerToOtherLocationSet(u32 id)
{
    const struct RoamerInfo *info = &sRoamerInfo[id];
    u8 mapNum = 0;

    if (!IsRoamerActive(id))
        return;

    sRoamerLocation[id][MAP_GRP] = info->mapGroup;

    // Choose a location set that starts with a map
    // different from the roamer's current map
    while (1)
    {
        mapNum = info->locations[Random() % info->numLocationSets][0];
        if (sRoamerLocation[id][MAP_NUM] != mapNum)
        {
            sRoamerLocation[id][MAP_NUM] = mapNum;
            return;
        }
    }
}

void RoamerMoveToOtherLocationSet(void)
{
    u32 id;

    for (id = 0; id < ROAMER_COUNT; id++)
        MoveRoamerToOtherLocationSet(id);
}

static void MoveRoamer(u32 id)
{
    const struct RoamerInfo *info = &sRoamerInfo[id];
    u8 choices[NUM_LOCATIONS_PER_SET - 1];
    u32 locSet, i, numChoices, numPreferred;

    if ((Random() % 16) == 0)
    {
        MoveRoamerToOtherLocationSet(id);
        return;
    }
    if (!IsRoamerActive(id))
        return;

    for (locSet = 0; locSet < info->numLocationSets; locSet++)
    {
        // Find the location set that starts with the roamer's current map
        if (sRoamerLocation[id][MAP_NUM] != info->locations[locSet][0])
            continue;

        // Choose a new map (excluding the first) within this set. Maps where the
        // player was 2 moves ago are only used if there is no other.
        numChoices = 0;
        numPreferred = 0;
        for (i = 1; i < NUM_LOCATIONS_PER_SET; i++)
        {
            u8 mapNum = info->locations[locSet][i];

            if (mapNum == NO_LOCATION)
                continue;
            if (sLocationHistory[2][MAP_GRP] == info->mapGroup && sLocationHistory[2][MAP_NUM] == mapNum)
            {
                choices[numChoices++] = mapNum;
            }
            else
            {
                // Keep the preferred ones at the front
                choices[numChoices++] = choices[numPreferred];
                choices[numPreferred++] = mapNum;
            }
        }
        if (numPreferred != 0)
            numChoices = numPreferred;
        if (numChoices != 0)
            sRoamerLocation[id][MAP_NUM] = choices[Random() % numChoices];
        return;
    }
}

void RoamerMove(void)
{
    u32 id;

    for (id = 0; id < ROAMER_COUNT; id++)
        MoveRoamer(id);
}

static bool32 IsRoamerIdAt(u32 id, u8 mapGroup, u8 mapNum)
{
    return IsRoamerActive(id) && mapGroup == sRoamerLocation[id][MAP_GRP] && mapNum == sRoamerLocation[id][MAP_NUM];
}

bool8 IsRoamerAt(u8 mapGroup, u8 mapNum)
{
    u32 id;

    for (id = 0; id < ROAMER_COUNT; id++)
    {
        if (IsRoamerIdAt(id, mapGroup, mapNum))
            return TRUE;
    }
    return FALSE;
}

// Raises a roamer from a save made when roamers were a lower level, keeping the damage it has taken.
static void UpdateRoamerLevel(u32 id)
{
    struct Roamer *roamer = GetRoamer(id);
    struct Pokemon *mon = &gEnemyParty[0];
    u8 level = sRoamerInfo[id].level;
    u16 oldMaxHp;

    if (roamer->level >= level)
        return;

    CreateMonWithIVsPersonality(mon, roamer->species, roamer->level, roamer->ivs, roamer->personality);
    oldMaxHp = GetMonData(mon, MON_DATA_MAX_HP);
    CreateMonWithIVsPersonality(mon, roamer->species, level, roamer->ivs, roamer->personality);
    roamer->level = level;
    roamer->hp = GetMonData(mon, MON_DATA_MAX_HP) - (oldMaxHp - min(roamer->hp, oldMaxHp));
}

// Creates the wild Pokemon for a battle with the current roamer.
void CreateRoamerMonInstance(void)
{
    struct Roamer *roamer = GetRoamer(sCurrentRoamer);
    u32 status;
    struct Pokemon *mon = &gEnemyParty[0];
    ZeroEnemyPartyMons();
    UpdateRoamerLevel(sCurrentRoamer);
    CreateMonWithIVsPersonality(mon, roamer->species, roamer->level, roamer->ivs, roamer->personality);
    // The roamer's status field is u8, but SetMonData expects status to be u32.
    status = roamer->status;
    SetMonData(mon, MON_DATA_STATUS, &status);
    SetMonData(mon, MON_DATA_HP, &roamer->hp);
    SetMonData(mon, MON_DATA_COOL, &roamer->cool);
    SetMonData(mon, MON_DATA_BEAUTY, &roamer->beauty);
    SetMonData(mon, MON_DATA_CUTE, &roamer->cute);
    SetMonData(mon, MON_DATA_SMART, &roamer->smart);
    SetMonData(mon, MON_DATA_TOUGH, &roamer->tough);
}

// Each roamer on the player's map has a 1 in 4 chance to be the wild encounter.
bool8 TryStartRoamerEncounter(void)
{
    u32 id;

    for (id = 0; id < ROAMER_COUNT; id++)
    {
        if (IsRoamerIdAt(id, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum) && (Random() % 4) == 0)
        {
            sCurrentRoamer = id;
            CreateRoamerMonInstance();
            return TRUE;
        }
    }
    return FALSE;
}

u8 GetCurrentRoamerLevel(void)
{
    return GetRoamer(sCurrentRoamer)->level;
}

void UpdateRoamerHPStatus(struct Pokemon *mon)
{
    struct Roamer *roamer = GetRoamer(sCurrentRoamer);

    roamer->hp = GetMonData(mon, MON_DATA_HP);
    roamer->status = GetMonData(mon, MON_DATA_STATUS);

    MoveRoamerToOtherLocationSet(sCurrentRoamer);
}

// The current roamer was caught or knocked out.
void SetRoamerInactive(void)
{
    GetRoamer(sCurrentRoamer)->active = FALSE;
    if (gBattleOutcome == B_OUTCOME_CAUGHT && sRoamerInfo[sCurrentRoamer].caughtFlag != 0)
        FlagSet(sRoamerInfo[sCurrentRoamer].caughtFlag);
}

// Whether the species is a roamer's (for Latias and Latios: of the one that roams or roamed). If so,
// also whether it is roaming, and where.
bool8 GetRoamerLocation(u16 species, bool8 *active, u8 *mapGroup, u8 *mapNum)
{
    u32 id;

    for (id = 0; id < ROAMER_COUNT; id++)
    {
        if (species != (id == ROAMER_LATI ? GetRoamer(id)->species : sRoamerInfo[id].species))
            continue;
        *active = IsRoamerActive(id);
        *mapGroup = sRoamerLocation[id][MAP_GRP];
        *mapNum = sRoamerLocation[id][MAP_NUM];
        return TRUE;
    }
    return FALSE;
}

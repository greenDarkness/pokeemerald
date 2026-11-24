// Hidden Grottos, ported from Black 2 / White 2 (https://github.com/fuddlesworth/pokebw2).
// The grotto list, pools and odds live in src/data/hidden_grottos.h.
#include "global.h"
#include "hidden_grotto.h"
#include "daycare.h"
#include "egg_hatch.h"
#include "event_data.h"
#include "event_scripts.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_specials.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "main.h"
#include "overworld.h"
#include "overrides.h"
#include "palette.h"
#include "random.h"
#include "script.h"
#include "script_pokemon_util.h"
#include "sound.h"
#include "task.h"
#include "time_events.h"
#include "constants/event_objects.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/moves.h"
#include "constants/songs.h"
#include "constants/species.h"
#include "wild_encounter.h"

// The pools a grotto rolls from, in roll order.
enum {
    GROTTO_POOL_MON1,
    GROTTO_POOL_MON2,
    GROTTO_POOL_ITEM1,
    GROTTO_POOL_ITEM2,
    GROTTO_POOL_EGG,
    GROTTO_POOL_COUNT,
};

// Entrance of a grotto: map coordinates of the tile the player faces, the percent chance of
// each pool, and the seasons it's there (SEASON_TAG_*, 0 = all; see src/data/hidden_grottos.h).
struct HiddenGrotto
{
    u8 location;
    u8 mapGroup;
    u8 x;
    u8 y;
    u8 mon1Chance;
    u8 mon2Chance;
    u8 item1Chance;
    u8 item2Chance;
    u8 eggChance;
    u8 seasons;
};

// Like the mass outbreak (swarm) list in src/tv.c, but on a single map. .level is a plain level
// or LEVEL_RANGE(min, max) (see include/wild_encounter.h).
struct HiddenGrottoMon
{
    u16 species;
    u16 moves[MAX_MON_MOVES];
    u16 level;
    u8 location;
    u8 mapGroup;
    u8 seasons;
};

// .quantity is how many the player picks up: a plain number, or QUANTITY_RANGE(min, max) for a
// random amount rolled when it's picked up. Left out (0) it's one.
struct HiddenGrottoItem
{
    u16 item;
    u16 quantity;
    u8 location;
    u8 mapGroup;
    u8 seasons;
};

#define QUANTITY_RANGE(min, max) LEVEL_RANGE(min, max)

// Items shown in the grotto as themselves instead of as an item ball.
struct HiddenGrottoItemGraphics
{
    u16 item;
    u16 graphicsId;
};

// The egg hatches into the species' base form, like a Day Care egg.
// GROTTO_EGG_ANY_GRASS_MON instead picks a breedable Pokemon from the map's grass encounters.
struct HiddenGrottoEgg
{
    u16 species;
    u8 location;
    u8 mapGroup;
    u8 seasons;
};

#define GROTTO_EGG_ANY_GRASS_MON 0xFFFF

#include "data/hidden_grottos.h"

enum {
    GROTTO_REGION_KANTO,
    GROTTO_REGION_JOHTO,
    GROTTO_REGION_HOENN,
};

// What a grotto holds is saved in one byte, gSaveBlock2Ptr->hiddenGrottos[slot]:
// 0 = not rolled yet, 1 = empty, otherwise ((pool + 1) << 5) | n, the n-th entry of the
// pool on the grotto's map. For GROTTO_POOL_GRASS_EGG (an egg from a GROTTO_EGG_ANY_GRASS_MON
// entry) n is the slot of the map's grass encounter table.
#define GROTTO_STATE_UNROLLED 0
#define GROTTO_STATE_EMPTY    1
#define GROTTO_POOL_SHIFT     5
#define GROTTO_INDEX_MASK     ((1 << GROTTO_POOL_SHIFT) - 1)
#define GROTTO_MAX_POOL_ENTRIES_PER_MAP (GROTTO_INDEX_MASK + 1)
#define GROTTO_NO_POOL        0xFF
#define GROTTO_POOL_GRASS_EGG GROTTO_POOL_COUNT

#define GROTTO_STATE(pool, n) ((((pool) + 1) << GROTTO_POOL_SHIFT) | (n))
#define GROTTO_POOL(state)    (((state) >> GROTTO_POOL_SHIFT) == 0 ? GROTTO_NO_POOL : ((state) >> GROTTO_POOL_SHIFT) - 1)
#define GROTTO_INDEX(state)   ((state) & GROTTO_INDEX_MASK)
#define IS_MON_POOL(pool)     ((pool) == GROTTO_POOL_MON1 || (pool) == GROTTO_POOL_MON2)
#define IS_ITEM_POOL(pool)    ((pool) == GROTTO_POOL_ITEM1 || (pool) == GROTTO_POOL_ITEM2)

// Room state; the temp flags/vars are cleared on every warp and kept in saves.
#define VAR_GROTTO_SLOT       VAR_TEMP_5 // slot + 1 of the grotto the room shows
#define VAR_GROTTO_MON_SPECIES VAR_TEMP_4 // read by OBJ_EVENT_GFX_OW_MON objects using a VAR gfx id
#define FLAG_HIDE_GROTTO_MON  FLAG_TEMP_1
#define FLAG_HIDE_GROTTO_ITEM FLAG_TEMP_2
#define FLAG_HIDE_GROTTO_EGG  FLAG_TEMP_4

struct HiddenGrottoRegion
{
    const struct HiddenGrotto *grottos;
    const struct HiddenGrottoMon *mons[2];
    const struct HiddenGrottoItem *items[2];
    const struct HiddenGrottoEgg *eggs;
    u16 grottoCount;
    u16 poolSizes[GROTTO_POOL_COUNT];
};

#define GROTTO_REGION(region, Region)                                                         \
[GROTTO_REGION_##region] =                                                                    \
{                                                                                             \
    .grottos = sHiddenGrottos##Region,                                                        \
    .mons = {sHiddenGrottoMons1##Region, sHiddenGrottoMons2##Region},                         \
    .items = {sHiddenGrottoItems1##Region, sHiddenGrottoItems2##Region},                      \
    .eggs = sHiddenGrottoEggs##Region,                                                        \
    .grottoCount = ARRAY_COUNT(sHiddenGrottos##Region),                                       \
    .poolSizes = {                                                                            \
        ARRAY_COUNT(sHiddenGrottoMons1##Region), ARRAY_COUNT(sHiddenGrottoMons2##Region),     \
        ARRAY_COUNT(sHiddenGrottoItems1##Region), ARRAY_COUNT(sHiddenGrottoItems2##Region),   \
        ARRAY_COUNT(sHiddenGrottoEggs##Region),                                               \
    },                                                                                        \
}

static const struct HiddenGrottoRegion sHiddenGrottoRegions[HIDDEN_GROTTO_REGION_COUNT] =
{
    GROTTO_REGION(KANTO, Kanto),
    GROTTO_REGION(JOHTO, Johto),
    GROTTO_REGION(HOENN, Hoenn),
};

STATIC_ASSERT(ARRAY_COUNT(sHiddenGrottosKanto) <= HIDDEN_GROTTOS_PER_REGION, TooManyKantoHiddenGrottos);
STATIC_ASSERT(ARRAY_COUNT(sHiddenGrottosJohto) <= HIDDEN_GROTTOS_PER_REGION, TooManyJohtoHiddenGrottos);
STATIC_ASSERT(ARRAY_COUNT(sHiddenGrottosHoenn) <= HIDDEN_GROTTOS_PER_REGION, TooManyHoennHiddenGrottos);

// Slot + 1 of the grotto being entered (0 = none, EWRAM starts zeroed).
static EWRAM_DATA u8 sEnteredGrottoSlot = 0;
// Song playing on the route when the grotto was entered (0 = use the route's default).
static EWRAM_DATA u16 sEntranceMusic = 0;

static const struct HiddenGrottoRegion *GetSlotRegion(u8 slot)
{
    return &sHiddenGrottoRegions[slot / HIDDEN_GROTTOS_PER_REGION];
}

static const struct HiddenGrotto *GetSlotGrotto(u8 slot)
{
    const struct HiddenGrottoRegion *region;
    u8 index = slot % HIDDEN_GROTTOS_PER_REGION;

    if (slot >= HIDDEN_GROTTO_REGION_COUNT * HIDDEN_GROTTOS_PER_REGION)
        return NULL;
    region = GetSlotRegion(slot);
    if (index >= region->grottoCount)
        return NULL;
    return &region->grottos[index];
}

static bool8 IsPoolEntryOnGrottoMap(const struct HiddenGrottoRegion *region, u8 pool, u16 i, const struct HiddenGrotto *grotto)
{
    u8 mapGroup, mapNum;

    if (IS_MON_POOL(pool))
    {
        mapGroup = region->mons[pool - GROTTO_POOL_MON1][i].mapGroup;
        mapNum = region->mons[pool - GROTTO_POOL_MON1][i].location;
    }
    else if (IS_ITEM_POOL(pool))
    {
        mapGroup = region->items[pool - GROTTO_POOL_ITEM1][i].mapGroup;
        mapNum = region->items[pool - GROTTO_POOL_ITEM1][i].location;
    }
    else
    {
        mapGroup = region->eggs[i].mapGroup;
        mapNum = region->eggs[i].location;
    }
    return mapGroup == grotto->mapGroup && mapNum == grotto->location;
}

// Index in the region's pool array of the n-th entry for the grotto's map, or -1.
static s32 FindPoolEntry(const struct HiddenGrottoRegion *region, u8 pool, const struct HiddenGrotto *grotto, u8 n)
{
    s32 i;

    if (pool >= GROTTO_POOL_COUNT)
        return -1;
    for (i = 0; i < region->poolSizes[pool]; i++)
    {
        if (IsPoolEntryOnGrottoMap(region, pool, i, grotto) && n-- == 0)
            return i;
    }
    return -1;
}

static u8 GetPoolEntrySeasons(const struct HiddenGrottoRegion *region, u8 pool, u16 i)
{
    if (IS_MON_POOL(pool))
        return region->mons[pool - GROTTO_POOL_MON1][i].seasons;
    if (IS_ITEM_POOL(pool))
        return region->items[pool - GROTTO_POOL_ITEM1][i].seasons;
    return region->eggs[i].seasons;
}

// Chance of each grass encounter slot, out of ENCOUNTER_CHANCE_LAND_MONS_TOTAL.
static const u8 sGrassSlotChances[NUM_LAND_MONS_ENCOUNTER_SLOTS] =
{
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_0,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_1 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_0,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_2 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_1,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_3 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_2,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_4 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_3,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_5 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_4,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_6 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_5,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_7 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_6,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_8 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_7,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_9 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_8,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_10 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_9,
    ENCOUNTER_CHANCE_LAND_MONS_SLOT_11 - ENCOUNTER_CHANCE_LAND_MONS_SLOT_10,
};

static const struct WildPokemon *GetGrottoGrassMons(const struct HiddenGrotto *grotto)
{
    u16 i;

    for (i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        if (gWildMonHeaders[i].mapGroup == grotto->mapGroup && gWildMonHeaders[i].mapNum == grotto->location)
            return gWildMonHeaders[i].landMonsInfo != NULL ? gWildMonHeaders[i].landMonsInfo->wildPokemon : NULL;
    }
    return NULL;
}

// Same rule as the Day Care: anything but Ditto and the Undiscovered egg group (whose babies,
// like Pichu, are covered through their evolved forms' egg groups).
static bool8 CanGrassMonBeEgg(u16 species)
{
    return species != SPECIES_NONE && species < NUM_SPECIES && species != SPECIES_DITTO
        && gSpeciesInfo[species].eggGroups[0] != EGG_GROUP_NO_EGGS_DISCOVERED;
}

// A grass encounter slot picked with the slots' encounter odds, or 0xFF if none can be an egg.
static u8 RollGrassEggSlot(const struct HiddenGrotto *grotto)
{
    const struct WildPokemon *mons = GetGrottoGrassMons(grotto);
    u16 total = 0, roll;
    u8 i;

    if (mons == NULL)
        return 0xFF;
    for (i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
    {
        if (CanGrassMonBeEgg(mons[i].species))
            total += sGrassSlotChances[i];
    }
    if (total == 0)
        return 0xFF;
    roll = Random() % total;
    for (i = 0; i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
    {
        if (!CanGrassMonBeEgg(mons[i].species))
            continue;
        if (roll < sGrassSlotChances[i])
            return i;
        roll -= sGrassSlotChances[i];
    }
    return 0xFF;
}

static bool8 HasGrassEggMon(const struct HiddenGrotto *grotto)
{
    const struct WildPokemon *mons = GetGrottoGrassMons(grotto);
    u8 i;

    for (i = 0; mons != NULL && i < NUM_LAND_MONS_ENCOUNTER_SLOTS; i++)
    {
        if (CanGrassMonBeEgg(mons[i].species))
            return TRUE;
    }
    return FALSE;
}

static bool8 HasInSeasonGrassEggEntry(const struct HiddenGrottoRegion *region, const struct HiddenGrotto *grotto)
{
    u16 i;

    for (i = 0; i < region->poolSizes[GROTTO_POOL_EGG]; i++)
    {
        if (region->eggs[i].species == GROTTO_EGG_ANY_GRASS_MON && IsInSeason(region->eggs[i].seasons)
         && IsPoolEntryOnGrottoMap(region, GROTTO_POOL_EGG, i, grotto))
            return TRUE;
    }
    return FALSE;
}

// Fills entries with the n (as used by FindPoolEntry) of every entry for the grotto's map
// that is in season, and returns how many there are.
static u8 GetInSeasonPoolEntries(const struct HiddenGrottoRegion *region, u8 pool, const struct HiddenGrotto *grotto, u8 *entries)
{
    u16 i;
    u8 n = 0, count = 0;

    for (i = 0; i < region->poolSizes[pool] && n < GROTTO_MAX_POOL_ENTRIES_PER_MAP; i++)
    {
        if (!IsPoolEntryOnGrottoMap(region, pool, i, grotto))
            continue;
        if (IsInSeason(GetPoolEntrySeasons(region, pool, i))
         && !(pool == GROTTO_POOL_EGG && region->eggs[i].species == GROTTO_EGG_ANY_GRASS_MON && !HasGrassEggMon(grotto)))
            entries[count++] = n;
        n++;
    }
    return count;
}

static u8 GetPoolChance(const struct HiddenGrotto *grotto, u8 pool)
{
    switch (pool)
    {
    case GROTTO_POOL_MON1:  return grotto->mon1Chance;
    case GROTTO_POOL_MON2:  return grotto->mon2Chance;
    case GROTTO_POOL_ITEM1: return grotto->item1Chance;
    case GROTTO_POOL_ITEM2: return grotto->item2Chance;
    case GROTTO_POOL_EGG:   return grotto->eggChance;
    }
    return 0;
}

// Pokédex: the index-th Pokémon pool entry of any region. Returns FALSE once past the last entry.
// *species is SPECIES_NONE unless a grotto on the entry's map can hold it right now, i.e. the entry
// and an entrance on that map are in season and the entrance has a chance of rolling that pool.
bool8 GetPokedexGrottoMonCandidate(u16 index, u16 *species, u8 *minLevel, u8 *maxLevel, u8 *mapGroup, u8 *mapNum)
{
    u8 r, pool;
    u16 g;

    for (r = 0; r < HIDDEN_GROTTO_REGION_COUNT; r++)
    {
        const struct HiddenGrottoRegion *region = &sHiddenGrottoRegions[r];

        for (pool = GROTTO_POOL_MON1; pool <= GROTTO_POOL_MON2; pool++)
        {
            const struct HiddenGrottoMon *mon;

            if (index >= region->poolSizes[pool])
            {
                index -= region->poolSizes[pool];
                continue;
            }
            mon = &region->mons[pool - GROTTO_POOL_MON1][index];
            *species = SPECIES_NONE;
            GetLevelRange(mon->level, minLevel, maxLevel);
            *mapGroup = mon->mapGroup;
            *mapNum = mon->location;
            if (!IsInSeason(mon->seasons))
                return TRUE;
            for (g = 0; g < region->grottoCount; g++)
            {
                const struct HiddenGrotto *grotto = &region->grottos[g];

                if (grotto->mapGroup == mon->mapGroup && grotto->location == mon->location
                 && IsInSeason(grotto->seasons) && GetPoolChance(grotto, pool) != 0)
                {
                    *species = mon->species;
                    break;
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}

static u8 RollGrottoContents(const struct HiddenGrottoRegion *region, const struct HiddenGrotto *grotto)
{
    u16 roll = Random() % 100;
    u16 total = 0;
    u8 pool, count;
    u8 entries[GROTTO_MAX_POOL_ENTRIES_PER_MAP];

    if (!IsInSeason(grotto->seasons))
        return GROTTO_STATE_EMPTY;
    for (pool = 0; pool < GROTTO_POOL_COUNT; pool++)
    {
        total += GetPoolChance(grotto, pool);
        if (roll < total)
            break;
    }
    if (pool == GROTTO_POOL_COUNT)
        return GROTTO_STATE_EMPTY;

    count = GetInSeasonPoolEntries(region, pool, grotto, entries);
    if (count == 0)
        return GROTTO_STATE_EMPTY;
    count = entries[Random() % count];
    if (pool == GROTTO_POOL_EGG && region->eggs[FindPoolEntry(region, pool, grotto, count)].species == GROTTO_EGG_ANY_GRASS_MON)
    {
        u8 grassSlot = RollGrassEggSlot(grotto);

        return grassSlot == 0xFF ? GROTTO_STATE_EMPTY : GROTTO_STATE(GROTTO_POOL_GRASS_EGG, grassSlot);
    }
    return GROTTO_STATE(pool, count);
}

static bool8 IsGrottoStateValid(const struct HiddenGrottoRegion *region, const struct HiddenGrotto *grotto, u8 state)
{
    s32 i;

    if (state == GROTTO_STATE_EMPTY)
        return TRUE;
    // The pools or encounter tables may have been edited since this was rolled, or the
    // grotto's or entry's season ended.
    if (GROTTO_POOL(state) == GROTTO_POOL_GRASS_EGG)
    {
        const struct WildPokemon *mons = GetGrottoGrassMons(grotto);

        return IsInSeason(grotto->seasons) && HasInSeasonGrassEggEntry(region, grotto) && mons != NULL
            && GROTTO_INDEX(state) < NUM_LAND_MONS_ENCOUNTER_SLOTS && CanGrassMonBeEgg(mons[GROTTO_INDEX(state)].species);
    }
    i = FindPoolEntry(region, GROTTO_POOL(state), grotto, GROTTO_INDEX(state));
    if (i >= 0 && GROTTO_POOL(state) == GROTTO_POOL_EGG && region->eggs[i].species == GROTTO_EGG_ANY_GRASS_MON)
        return FALSE;
    return i >= 0 && IsInSeason(grotto->seasons) && IsInSeason(GetPoolEntrySeasons(region, GROTTO_POOL(state), i));
}

// What the grotto holds, rolling it first if it never has been.
static u8 GetGrottoState(u8 slot)
{
    const struct HiddenGrotto *grotto = GetSlotGrotto(slot);
    const struct HiddenGrottoRegion *region;
    u8 *state;

    if (grotto == NULL)
        return GROTTO_STATE_EMPTY;
    region = GetSlotRegion(slot);
    state = &gSaveBlock2Ptr->hiddenGrottos[slot];
    if (!IsGrottoStateValid(region, grotto, *state))
        *state = RollGrottoContents(region, grotto);
    return *state;
}

static void SetGrottoEmpty(u8 slot)
{
    if (GetSlotGrotto(slot) != NULL)
        gSaveBlock2Ptr->hiddenGrottos[slot] = GROTTO_STATE_EMPTY;
}

// Grottos that were taken or rolled empty get a new roll each day; full ones keep their contents.
void UpdateHiddenGrottosPerDay(void)
{
    u8 slot;

    for (slot = 0; slot < HIDDEN_GROTTO_REGION_COUNT * HIDDEN_GROTTOS_PER_REGION; slot++)
    {
        const struct HiddenGrotto *grotto = GetSlotGrotto(slot);
        const struct HiddenGrottoRegion *region = GetSlotRegion(slot);
        u8 *state = &gSaveBlock2Ptr->hiddenGrottos[slot];

        if (grotto == NULL)
            *state = GROTTO_STATE_UNROLLED;
        else if (*state == GROTTO_STATE_EMPTY || !IsGrottoStateValid(region, grotto, *state))
            *state = RollGrottoContents(region, grotto);
    }
}

static s16 FindGrottoSlot(u8 mapGroup, u8 mapNum, s16 x, s16 y)
{
    u8 r, i;

    for (r = 0; r < HIDDEN_GROTTO_REGION_COUNT; r++)
    {
        const struct HiddenGrottoRegion *region = &sHiddenGrottoRegions[r];

        for (i = 0; i < region->grottoCount; i++)
        {
            const struct HiddenGrotto *grotto = &region->grottos[i];

            if (grotto->mapGroup == mapGroup && grotto->location == mapNum && grotto->x == x && grotto->y == y
             && IsInSeason(grotto->seasons))
                return r * HIDDEN_GROTTOS_PER_REGION + i;
        }
    }
    return -1;
}

// x, y are map coordinates (without MAP_OFFSET) of the tile the player is facing.
const u8 *GetHiddenGrottoEntranceScript(s16 x, s16 y)
{
    s16 slot = FindGrottoSlot(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, x, y);

    if (slot < 0)
        return NULL;
    gSpecialVar_0x8004 = slot;
    return HiddenGrotto_EventScript_Entrance;
}

// For scripted entrances (special EnterHiddenGrotto): VAR_0x8004 = slot of the grotto the
// player is facing, VAR_RESULT = whether there is one.
void GetFacingHiddenGrotto(void)
{
    s16 x, y;

    GetXYCoordsOneStepInFrontOfPlayer(&x, &y);
    gSpecialVar_Result = (GetHiddenGrottoEntranceScript(x - MAP_OFFSET, y - MAP_OFFSET) != NULL);
    if (!gSpecialVar_Result)
        gSpecialVar_0x8004 = 0xFF;
}

bool8 IsHiddenGrottoMap(u8 mapGroup, u8 mapNum)
{
    return mapGroup == MAP_GROUP(MAP_HIDDEN_GROTTO) && mapNum == MAP_NUM(MAP_HIDDEN_GROTTO);
}

bool8 IsPlayerInHiddenGrotto(void)
{
    return IsHiddenGrottoMap(gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum);
}

// The grotto's exit is a dynamic warp back to where the player stood at the entrance.
mapsec_u16_t GetHiddenGrottoEntranceMapSec(void)
{
    return Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->dynamicWarp.mapGroup, gSaveBlock1Ptr->dynamicWarp.mapNum)->regionMapSectionId;
}

u16 GetHiddenGrottoMusic(void)
{
    if (sEntranceMusic != 0)
        return sEntranceMusic;
    if (IsHiddenGrottoMap(gSaveBlock1Ptr->dynamicWarp.mapGroup, gSaveBlock1Ptr->dynamicWarp.mapNum))
        return MUS_NONE;
    return GetLocationMusic(&gSaveBlock1Ptr->dynamicWarp);
}

static void Task_EnterHiddenGrotto(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        SetWarpDestinationToMapWarp(MAP_GROUP(MAP_HIDDEN_GROTTO), MAP_NUM(MAP_HIDDEN_GROTTO), 0);
        WarpIntoMap();
        gFieldCallback = FieldCB_ContinueScriptHandleMusic;
        SetMainCallback2(CB2_LoadMap);
        DestroyTask(taskId);
    }
}

// Same as EnterSecretBase: the grotto's exit returns to the player's current position.
void EnterHiddenGrotto(void)
{
    u16 music = GetCurrentMapMusic();

    if (music == MUS_NONE || music == MUS_SURF || music == MUS_UNDERWATER)
        music = 0;
    sEntranceMusic = music;
    sEnteredGrottoSlot = gSpecialVar_0x8004 + 1;
    GetGrottoState(gSpecialVar_0x8004);

    CreateTask(Task_EnterHiddenGrotto, 0);
    FadeScreen(FADE_TO_BLACK, 0);
    SetDynamicWarp(0, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE);
}

// Fallback for when the room loads without EnterHiddenGrotto having run this session:
// the grotto next to the spot the exit returns to.
static u8 FindEnteredGrottoFromDynamicWarp(void)
{
    const struct WarpData *warp = &gSaveBlock1Ptr->dynamicWarp;
    u8 slot;

    for (slot = 0; slot < HIDDEN_GROTTO_REGION_COUNT * HIDDEN_GROTTOS_PER_REGION; slot++)
    {
        const struct HiddenGrotto *grotto = GetSlotGrotto(slot);

        if (grotto != NULL && grotto->mapGroup == warp->mapGroup && grotto->location == warp->mapNum
         && abs(grotto->x - warp->x) + abs(grotto->y - warp->y) == 1)
            return slot + 1;
    }
    return 0;
}

static const struct HiddenGrottoMon *GetGrottoMon(u8 slot, u8 state)
{
    const struct HiddenGrottoRegion *region = GetSlotRegion(slot);
    u8 pool = GROTTO_POOL(state);
    s32 i;

    if (!IS_MON_POOL(pool))
        return NULL;
    i = FindPoolEntry(region, pool, GetSlotGrotto(slot), GROTTO_INDEX(state));
    return i < 0 ? NULL : &region->mons[pool - GROTTO_POOL_MON1][i];
}

static const struct HiddenGrottoItem *GetGrottoItem(u8 slot, u8 state)
{
    const struct HiddenGrottoRegion *region = GetSlotRegion(slot);
    u8 pool = GROTTO_POOL(state);
    s32 i;

    if (!IS_ITEM_POOL(pool))
        return NULL;
    i = FindPoolEntry(region, pool, GetSlotGrotto(slot), GROTTO_INDEX(state));
    if (i < 0 || region->items[pool - GROTTO_POOL_ITEM1][i].item == ITEM_NONE)
        return NULL;
    return &region->items[pool - GROTTO_POOL_ITEM1][i];
}

static u16 GetGrottoItemGraphicsId(u16 item)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sHiddenGrottoItemGraphics); i++)
    {
        if (sHiddenGrottoItemGraphics[i].item == item)
            return sHiddenGrottoItemGraphics[i].graphicsId;
    }
    return OBJ_EVENT_GFX_ITEM_BALL;
}

// Species the grotto's egg hatches into (the base form), or SPECIES_NONE if it holds no egg.
static u16 GetGrottoEggSpecies(u8 slot, u8 state)
{
    const struct HiddenGrottoRegion *region = GetSlotRegion(slot);
    const struct HiddenGrotto *grotto = GetSlotGrotto(slot);
    u16 species = SPECIES_NONE;
    s32 i;

    if (grotto == NULL)
        return SPECIES_NONE;
    if (GROTTO_POOL(state) == GROTTO_POOL_EGG)
    {
        i = FindPoolEntry(region, GROTTO_POOL_EGG, grotto, GROTTO_INDEX(state));
        if (i >= 0 && region->eggs[i].species != GROTTO_EGG_ANY_GRASS_MON)
            species = region->eggs[i].species;
    }
    else if (GROTTO_POOL(state) == GROTTO_POOL_GRASS_EGG)
    {
        const struct WildPokemon *mons = GetGrottoGrassMons(grotto);

        if (mons != NULL && GROTTO_INDEX(state) < NUM_LAND_MONS_ENCOUNTER_SLOTS)
            species = mons[GROTTO_INDEX(state)].species;
    }
    if (species == SPECIES_NONE || species >= NUM_SPECIES)
        return SPECIES_NONE;
    return GetEggSpecies(species);
}

// Object palette of the room's egg (OBJ_EVENT_PAL_TAG_GROTTO_EGG), colored by the egg's types like
// the egg hatching and party egg icons. Filled before the room's objects are spawned.
EWRAM_DATA u16 gHiddenGrottoEggPalette[16] = {0};
extern const u16 gObjectEventPal_GrottoEgg[];

static void UpdateHiddenGrottoEggPalette(u16 eggSpecies)
{
    CpuCopy16(gObjectEventPal_GrottoEgg, gHiddenGrottoEggPalette, sizeof(gHiddenGrottoEggPalette));
    if (eggSpecies != SPECIES_NONE)
        ApplyOverworldEggTypePalette(gHiddenGrottoEggPalette, eggSpecies);
}

static u8 GetRoomGrottoSlot(void);

// Continuing a game saved in the grotto room: its objects spawn without the on-transition script.
void RefreshHiddenGrottoRoomEggPalette(void)
{
    u8 slot = GetRoomGrottoSlot();

    UpdateHiddenGrottoEggPalette(slot == 0xFF ? SPECIES_NONE : GetGrottoEggSpecies(slot, gSaveBlock2Ptr->hiddenGrottos[slot]));
}

// The room's on-transition script: show the Pokemon, item or egg the grotto holds. The Pokemon
// and the item both take their graphics from VAR_OBJ_GFX_ID_0, as only one of them is there.
void SetupHiddenGrottoRoom(void)
{
    u8 slotId = sEnteredGrottoSlot;
    u8 state = GROTTO_STATE_EMPTY;
    const struct HiddenGrottoMon *mon = NULL;
    const struct HiddenGrottoItem *item = NULL;
    u16 eggSpecies = SPECIES_NONE;

    if (slotId == 0)
        slotId = FindEnteredGrottoFromDynamicWarp();
    VarSet(VAR_GROTTO_SLOT, slotId);
    if (slotId != 0)
    {
        state = GetGrottoState(slotId - 1);
        mon = GetGrottoMon(slotId - 1, state);
        item = GetGrottoItem(slotId - 1, state);
        eggSpecies = GetGrottoEggSpecies(slotId - 1, state);
    }
    UpdateHiddenGrottoEggPalette(eggSpecies);

    VarSet(VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_ITEM_BALL);
    FlagSet(FLAG_HIDE_GROTTO_MON);
    FlagSet(FLAG_HIDE_GROTTO_ITEM);
    FlagSet(FLAG_HIDE_GROTTO_EGG);
    if (mon != NULL)
    {
        VarSet(VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_OW_MON);
        VarSet(VAR_GROTTO_MON_SPECIES, mon->species);
        FlagClear(FLAG_HIDE_GROTTO_MON);
    }
    else if (item != NULL)
    {
        VarSet(VAR_OBJ_GFX_ID_0, GetGrottoItemGraphicsId(item->item));
        FlagClear(FLAG_HIDE_GROTTO_ITEM);
    }
    else if (eggSpecies != SPECIES_NONE)
    {
        FlagClear(FLAG_HIDE_GROTTO_EGG);
    }
}

static u8 GetRoomGrottoSlot(void)
{
    u16 slotId = VarGet(VAR_GROTTO_SLOT);

    if (slotId == 0 || GetSlotGrotto(slotId - 1) == NULL)
        return 0xFF;
    return slotId - 1;
}

// VAR_0x8004 = species, VAR_0x8005 = level (the lowest, for ranges), VAR_RESULT = whether the
// grotto still holds it.
void GetHiddenGrottoPokemon(void)
{
    u8 slot = GetRoomGrottoSlot();
    u8 minLevel, maxLevel;
    const struct HiddenGrottoMon *mon = NULL;

    if (slot != 0xFF)
        mon = GetGrottoMon(slot, GetGrottoState(slot));
    gSpecialVar_Result = (mon != NULL);
    if (mon != NULL)
    {
        GetLevelRange(mon->level, &minLevel, &maxLevel);
        gSpecialVar_0x8004 = mon->species;
        gSpecialVar_0x8005 = minLevel;
    }
}

// Sets the moves like the mass outbreak encounter does: MOVE_LEVEL takes the newest level-up
// move the Pokemon doesn't already know. Without any moves set, it keeps CreateMon's moveset.
// Returns the slot of its oldest level-up move, the one the chain's egg move may replace
// (MAX_MON_MOVES if every move is set by hand).
static u8 SetGrottoMonMoves(struct Pokemon *mon, const struct HiddenGrottoMon *grottoMon, u8 level)
{
    u16 moves[MAX_MON_MOVES];
    bool8 custom = FALSE;
    u8 oldestLevelMoveSlot = MAX_MON_MOVES;
    u8 i;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        moves[i] = grottoMon->moves[i];
        if (moves[i] != MOVE_LEVEL && moves[i] != MOVE_NONE)
            custom = TRUE;
    }
    if (!custom)
        return 0;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (moves[i] == MOVE_LEVEL)
        {
            moves[i] = GetMoveLevelMove_Wild(grottoMon->species, level, moves);
            oldestLevelMoveSlot = i;
        }
        SetMonMoveSlot(mon, moves[i], i);
    }
    return oldestLevelMoveSlot;
}

// Same IV rules as regular wild encounters: perfect IVs, then the species' custom IV sets.
static void SetGrottoMonIVs(struct Pokemon *mon, u16 species)
{
    u8 iv = MAX_PER_STAT_IVS;
    u8 stat;

    for (stat = 0; stat < NUM_STATS; stat++)
        SetMonData(mon, MON_DATA_HP_IV + stat, &iv);
    if (!TryApplyCustomWildMonIVs(species, mon))
        CalculateMonStats(mon);
}

// The chain's bonuses come last, so its egg move isn't replaced by the grotto's moveset.
static void CreateGrottoMon(u16 species, u8 level, const struct HiddenGrottoMon *grottoMon)
{
    u8 eggMoveSlot = 0;

    ZeroEnemyPartyMons();
    CreateMon(&gEnemyParty[0], species, level, USE_RANDOM_IVS, 0, 0, OT_ID_PLAYER_ID, 0);
    SetGrottoMonIVs(&gEnemyParty[0], species);
    if (grottoMon != NULL)
        eggMoveSlot = SetGrottoMonMoves(&gEnemyParty[0], grottoMon, level);
    ApplyChainBonuses(&gEnemyParty[0], eggMoveSlot);
}

// Like BW2, the Pokemon leaves the grotto once the battle starts, whatever its outcome.
void StartHiddenGrottoBattle(void)
{
    u8 slot = GetRoomGrottoSlot();
    const struct HiddenGrottoMon *mon = NULL;

    if (slot != 0xFF)
        mon = GetGrottoMon(slot, GetGrottoState(slot));
    if (mon != NULL)
    {
        u8 minLevel, maxLevel;

        GetLevelRange(mon->level, &minLevel, &maxLevel);
        CreateGrottoMon(mon->species, ChooseLevelInRange(minLevel, maxLevel), mon);
    }
    else
    {
        CreateGrottoMon(gSpecialVar_0x8004, gSpecialVar_0x8005, NULL);
    }
    SetGrottoEmpty(slot);
    FlagSet(FLAG_HIDE_GROTTO_MON);
}

// VAR_0x8000 = item, VAR_0x8001 = how many, VAR_RESULT = whether the grotto still holds it.
void GetHiddenGrottoItem(void)
{
    u8 slot = GetRoomGrottoSlot();
    const struct HiddenGrottoItem *item = NULL;

    if (slot != 0xFF)
        item = GetGrottoItem(slot, GetGrottoState(slot));
    gSpecialVar_Result = (item != NULL);
    if (item != NULL)
    {
        u8 minQuantity, maxQuantity;

        GetLevelRange(item->quantity, &minQuantity, &maxQuantity);
        gSpecialVar_0x8000 = item->item;
        gSpecialVar_0x8001 = ChooseLevelInRange(minQuantity, maxQuantity);
    }
}

void ClearHiddenGrottoContents(void)
{
    SetGrottoEmpty(GetRoomGrottoSlot());
}

// VAR_RESULT = whether the grotto still holds an egg.
void GetHiddenGrottoEgg(void)
{
    u8 slot = GetRoomGrottoSlot();

    gSpecialVar_Result = (slot != 0xFF && GetGrottoEggSpecies(slot, GetGrottoState(slot)) != SPECIES_NONE);
}

// Gives the grotto's egg (to the egg slot if it's free, else the party); VAR_RESULT = whether it
// was given. The egg records the grotto's route until it hatches, gets the same IVs as grotto
// Pokemon, and random egg moves like GiveRandomPerfectIVEgg. Unlike every other gift Pokemon
// and egg, it doesn't count towards the chain.
void GiveHiddenGrottoEgg(void)
{
    u8 slot = GetRoomGrottoSlot();
    struct Pokemon mon;
    metloc_u8_t metLocation;
    u16 species = SPECIES_NONE;

    gSpecialVar_Result = FALSE;
    if (slot != 0xFF)
        species = GetGrottoEggSpecies(slot, GetGrottoState(slot));
    if (species == SPECIES_NONE || !CanAcceptEgg())
        return;

    CreateEgg(&mon, species, FALSE);
    metLocation = GetCurrentRegionMapSectionId();
    SetMonData(&mon, MON_DATA_MET_LOCATION, &metLocation);
    SetGrottoMonIVs(&mon, species);
    GiveMonRandomEggMoves(&mon, species);

    if (GetMonData(&gEggSlot, MON_DATA_SPECIES) == SPECIES_NONE)
        gEggSlot = mon;
    else if (GiveMonToPlayer(&mon) == MON_CANT_GIVE)
        return;

    gSpecialVar_Result = TRUE;
    SetGrottoEmpty(slot);
    FlagSet(FLAG_HIDE_GROTTO_EGG);
}

// Hidden Grottos, ported from Black 2 / White 2 (https://github.com/fuddlesworth/pokebw2).
// The grotto list, pools and odds live in src/data/hidden_grottos.h.
#include "global.h"
#include "hidden_grotto.h"
#include "event_data.h"
#include "event_scripts.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "fieldmap.h"
#include "main.h"
#include "overworld.h"
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

// The four pools a grotto rolls from, in roll order.
enum {
    GROTTO_POOL_MON1,
    GROTTO_POOL_MON2,
    GROTTO_POOL_ITEM1,
    GROTTO_POOL_ITEM2,
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
    u8 seasons;
};

// Same layout as the mass outbreak (swarm) list in src/tv.c.
struct HiddenGrottoMon
{
    u16 species;
    u16 moves[MAX_MON_MOVES];
    u8 level;
    u8 location;
    u8 mapGroup;
    u8 seasons;
};

struct HiddenGrottoItem
{
    u16 item;
    u8 location;
    u8 mapGroup;
    u8 seasons;
};

#include "data/hidden_grottos.h"

enum {
    GROTTO_REGION_KANTO,
    GROTTO_REGION_JOHTO,
    GROTTO_REGION_HOENN,
};

// What a grotto holds is saved in one byte, gSaveBlock2Ptr->hiddenGrottos[slot]:
// 0 = not rolled yet, 1 = empty, otherwise ((pool + 1) << 5) | n, the n-th entry of the
// pool on the grotto's map.
#define GROTTO_STATE_UNROLLED 0
#define GROTTO_STATE_EMPTY    1
#define GROTTO_POOL_SHIFT     5
#define GROTTO_INDEX_MASK     ((1 << GROTTO_POOL_SHIFT) - 1)
#define GROTTO_MAX_POOL_ENTRIES_PER_MAP (GROTTO_INDEX_MASK + 1)
#define GROTTO_NO_POOL        0xFF

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

struct HiddenGrottoRegion
{
    const struct HiddenGrotto *grottos;
    const struct HiddenGrottoMon *mons[2];
    const struct HiddenGrottoItem *items[2];
    u16 grottoCount;
    u16 poolSizes[GROTTO_POOL_COUNT];
};

#define GROTTO_REGION(region, Region)                                                         \
[GROTTO_REGION_##region] =                                                                    \
{                                                                                             \
    .grottos = sHiddenGrottos##Region,                                                        \
    .mons = {sHiddenGrottoMons1##Region, sHiddenGrottoMons2##Region},                         \
    .items = {sHiddenGrottoItems1##Region, sHiddenGrottoItems2##Region},                      \
    .grottoCount = ARRAY_COUNT(sHiddenGrottos##Region),                                       \
    .poolSizes = {                                                                            \
        ARRAY_COUNT(sHiddenGrottoMons1##Region), ARRAY_COUNT(sHiddenGrottoMons2##Region),     \
        ARRAY_COUNT(sHiddenGrottoItems1##Region), ARRAY_COUNT(sHiddenGrottoItems2##Region),   \
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
    else
    {
        mapGroup = region->items[pool - GROTTO_POOL_ITEM1][i].mapGroup;
        mapNum = region->items[pool - GROTTO_POOL_ITEM1][i].location;
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
    return region->items[pool - GROTTO_POOL_ITEM1][i].seasons;
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
        if (IsInSeason(GetPoolEntrySeasons(region, pool, i)))
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
    }
    return 0;
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
    return GROTTO_STATE(pool, entries[Random() % count]);
}

static bool8 IsGrottoStateValid(const struct HiddenGrottoRegion *region, const struct HiddenGrotto *grotto, u8 state)
{
    s32 i;

    if (state == GROTTO_STATE_EMPTY)
        return TRUE;
    // The pools may have been edited since this was rolled, or the grotto's or entry's season ended.
    i = FindPoolEntry(region, GROTTO_POOL(state), grotto, GROTTO_INDEX(state));
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

static u16 GetGrottoItem(u8 slot, u8 state)
{
    const struct HiddenGrottoRegion *region = GetSlotRegion(slot);
    u8 pool = GROTTO_POOL(state);
    s32 i;

    if (!IS_ITEM_POOL(pool))
        return ITEM_NONE;
    i = FindPoolEntry(region, pool, GetSlotGrotto(slot), GROTTO_INDEX(state));
    return i < 0 ? ITEM_NONE : region->items[pool - GROTTO_POOL_ITEM1][i].item;
}

// The room's on-transition script: show the Pokemon or item ball the grotto holds.
void SetupHiddenGrottoRoom(void)
{
    u8 slotId = sEnteredGrottoSlot;
    u8 state = GROTTO_STATE_EMPTY;
    const struct HiddenGrottoMon *mon = NULL;

    if (slotId == 0)
        slotId = FindEnteredGrottoFromDynamicWarp();
    VarSet(VAR_GROTTO_SLOT, slotId);
    if (slotId != 0)
    {
        state = GetGrottoState(slotId - 1);
        mon = GetGrottoMon(slotId - 1, state);
    }

    VarSet(VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_ITEM_BALL);
    FlagSet(FLAG_HIDE_GROTTO_MON);
    FlagSet(FLAG_HIDE_GROTTO_ITEM);
    if (mon != NULL)
    {
        VarSet(VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_OW_MON);
        VarSet(VAR_GROTTO_MON_SPECIES, mon->species);
        FlagClear(FLAG_HIDE_GROTTO_MON);
    }
    else if (slotId != 0 && GetGrottoItem(slotId - 1, state) != ITEM_NONE)
    {
        FlagClear(FLAG_HIDE_GROTTO_ITEM);
    }
}

static u8 GetRoomGrottoSlot(void)
{
    u16 slotId = VarGet(VAR_GROTTO_SLOT);

    if (slotId == 0 || GetSlotGrotto(slotId - 1) == NULL)
        return 0xFF;
    return slotId - 1;
}

// VAR_0x8004 = species, VAR_0x8005 = level, VAR_RESULT = whether the grotto still holds it.
void GetHiddenGrottoPokemon(void)
{
    u8 slot = GetRoomGrottoSlot();
    const struct HiddenGrottoMon *mon = NULL;

    if (slot != 0xFF)
        mon = GetGrottoMon(slot, GetGrottoState(slot));
    gSpecialVar_Result = (mon != NULL);
    if (mon != NULL)
    {
        gSpecialVar_0x8004 = mon->species;
        gSpecialVar_0x8005 = mon->level;
    }
}

// Sets the moves like the mass outbreak encounter does: MOVE_LEVEL takes the newest level-up
// move the Pokemon doesn't already know. Without any moves set, it keeps CreateMon's moveset.
static void SetGrottoMonMoves(struct Pokemon *mon, const struct HiddenGrottoMon *grottoMon)
{
    u16 moves[MAX_MON_MOVES];
    bool8 custom = FALSE;
    u8 i;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        moves[i] = grottoMon->moves[i];
        if (moves[i] != MOVE_LEVEL && moves[i] != MOVE_NONE)
            custom = TRUE;
    }
    if (!custom)
        return;

    for (i = 0; i < MAX_MON_MOVES; i++)
    {
        if (moves[i] == MOVE_LEVEL)
            moves[i] = GetMoveLevelMove_Wild(grottoMon->species, grottoMon->level, moves);
        SetMonMoveSlot(mon, moves[i], i);
    }
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

// Like BW2, the Pokemon leaves the grotto once the battle starts, whatever its outcome.
void StartHiddenGrottoBattle(void)
{
    u8 slot = GetRoomGrottoSlot();
    const struct HiddenGrottoMon *mon = NULL;

    if (slot != 0xFF)
        mon = GetGrottoMon(slot, GetGrottoState(slot));
    if (mon != NULL)
    {
        CreateScriptedWildMon(mon->species, mon->level, ITEM_NONE);
        SetGrottoMonIVs(&gEnemyParty[0], mon->species);
        SetGrottoMonMoves(&gEnemyParty[0], mon);
    }
    else
    {
        CreateScriptedWildMon(gSpecialVar_0x8004, gSpecialVar_0x8005, ITEM_NONE);
        SetGrottoMonIVs(&gEnemyParty[0], gSpecialVar_0x8004);
    }
    SetGrottoEmpty(slot);
    FlagSet(FLAG_HIDE_GROTTO_MON);
}

// VAR_0x8000 = item, VAR_RESULT = whether the grotto still holds it.
void GetHiddenGrottoItem(void)
{
    u8 slot = GetRoomGrottoSlot();
    u16 item = ITEM_NONE;

    if (slot != 0xFF)
        item = GetGrottoItem(slot, GetGrottoState(slot));
    gSpecialVar_Result = (item != ITEM_NONE);
    gSpecialVar_0x8000 = item;
}

void ClearHiddenGrottoContents(void)
{
    SetGrottoEmpty(GetRoomGrottoSlot());
}

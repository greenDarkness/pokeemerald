// Missions of the Petalburg Herb Shop's owner. The missions and how to add one are in
// src/data/herb_shop.h; her scripts are in data/maps/PetalburgCity_HerbShop/scripts.inc (the
// shop) and data/scripts/herb_shop.inc (the escort to a Hidden Grotto).
#include "global.h"
#include "herb_shop.h"
#include "data.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "fieldmap.h"
#include "hidden_grotto.h"
#include "item.h"
#include "overworld.h"
#include "pokemon.h"
#include "random.h"
#include "region_map.h"
#include "script_movement.h"
#include "string_util.h"
#include "constants/event_objects.h"
#include "constants/event_object_movement.h"
#include "constants/herb_shop.h"
#include "constants/items.h"
#include "constants/maps.h"
#include "constants/species.h"

enum {
    HERB_UNLOCK_START,
    HERB_UNLOCK_GRUNT,
    HERB_UNLOCK_GYM,
};

struct HerbShopEscort
{
    u8 mapGroup;
    u8 mapNum;
    u8 x;
    u8 y;
    const char *path;
    const char *altPath;
    u16 item;
    u8 unlock;
    const u8 *where;
};

struct HerbShopCatch
{
    u16 species;
    u16 flag;
    u16 hideFlag;
    u8 localId;
    u8 unlock;
    const u8 *hint;
};

struct HerbShopIngredient
{
    u16 item;
    u8 ip;
};

#include "data/herb_shop.h"

// VAR_HERB_SHOP_MISSION: bits 0-1 are what the mission is about (HERB_MISSION_*), bits 2-3 how
// far an escort has come (ESCORT_*), and bits 4-15 which one it is: the index of the escort or
// the catch, or the indices of a fetch mission's three ingredients (4 bits each).
// The mission is to be done while FLAG_HERB_SHOP_MISSION_ACTIVE is set. The var keeps it after
// that, so the next mission can be a different one.
#define MISSION_KIND_MASK       0x0003
#define MISSION_PHASE_SHIFT     2
#define MISSION_PHASE_MASK      0x000C
#define MISSION_ID_SHIFT        4
#define INGREDIENT_BITS         4
#define INGREDIENTS_PER_RECIPE  3

#define MISSION(kind, id)       ((kind) | ((id) << MISSION_ID_SHIFT))
#define MISSION_KIND(mission)   ((mission) & MISSION_KIND_MASK)
#define MISSION_PHASE(mission)  (((mission) & MISSION_PHASE_MASK) >> MISSION_PHASE_SHIFT)
#define MISSION_ID(mission)     ((mission) >> MISSION_ID_SHIFT)

enum {
    ESCORT_WAITING,   // she stands on the route
    ESCORT_LEADING,   // from the walk to the grotto until she has found the item inside
    ESCORT_RETURNING, // warping back: the shop's on-frame script rewards the player
};

#define FIRST_ESCORT 0 // the mission that opens the shop

#define MAX_CANDIDATES (ARRAY_COUNT(sHerbShopEscorts) + ARRAY_COUNT(sHerbShopCatches) + 1)
#define ESCORT_MAX_STEPS 60

STATIC_ASSERT(ARRAY_COUNT(sHerbShopEscorts) <= (1 << (16 - MISSION_ID_SHIFT)), TooManyHerbShopEscorts);
STATIC_ASSERT(ARRAY_COUNT(sHerbShopCatches) <= (1 << (16 - MISSION_ID_SHIFT)), TooManyHerbShopCatches);
STATIC_ASSERT(ARRAY_COUNT(sHerbShopIngredients) <= (1 << INGREDIENT_BITS), TooManyHerbShopIngredients);
STATIC_ASSERT(ARRAY_COUNT(sHerbShopIngredients) >= INGREDIENTS_PER_RECIPE, TooFewHerbShopIngredients);
STATIC_ASSERT(MISSION_ID_SHIFT + INGREDIENT_BITS * INGREDIENTS_PER_RECIPE <= 16, HerbShopRecipeDoesntFitInVar);

// Scripted movement is read while the object moves, so it's kept here until the walk is over.
static EWRAM_DATA u8 sOwnerMovement[ESCORT_MAX_STEPS + 4] = {0};
static EWRAM_DATA u8 sPlayerMovement[ESCORT_MAX_STEPS + 4] = {0};
static EWRAM_DATA const char *sEscortPath = NULL;

static const u8 sWalkActions[] =
{
    [DIR_SOUTH] = MOVEMENT_ACTION_WALK_NORMAL_DOWN,
    [DIR_NORTH] = MOVEMENT_ACTION_WALK_NORMAL_UP,
    [DIR_WEST]  = MOVEMENT_ACTION_WALK_NORMAL_LEFT,
    [DIR_EAST]  = MOVEMENT_ACTION_WALK_NORMAL_RIGHT,
};

static const u8 sFaceActions[] =
{
    [DIR_SOUTH] = MOVEMENT_ACTION_FACE_DOWN,
    [DIR_NORTH] = MOVEMENT_ACTION_FACE_UP,
    [DIR_WEST]  = MOVEMENT_ACTION_FACE_LEFT,
    [DIR_EAST]  = MOVEMENT_ACTION_FACE_RIGHT,
};

static const u8 sTurnActions[] =
{
    [DIR_SOUTH] = MOVEMENT_ACTION_WALK_IN_PLACE_FASTER_DOWN,
    [DIR_NORTH] = MOVEMENT_ACTION_WALK_IN_PLACE_FASTER_UP,
    [DIR_WEST]  = MOVEMENT_ACTION_WALK_IN_PLACE_FASTER_LEFT,
    [DIR_EAST]  = MOVEMENT_ACTION_WALK_IN_PLACE_FASTER_RIGHT,
};

static const s8 sDirectionSteps[][2] =
{
    [DIR_SOUTH] = { 0,  1},
    [DIR_NORTH] = { 0, -1},
    [DIR_WEST]  = {-1,  0},
    [DIR_EAST]  = { 1,  0},
};

static const struct HerbShopEscort *GetEscort(u16 mission)
{
    u16 id = MISSION_ID(mission);

    return &sHerbShopEscorts[id < ARRAY_COUNT(sHerbShopEscorts) ? id : FIRST_ESCORT];
}

static const struct HerbShopCatch *GetCatch(u16 mission)
{
    u16 id = MISSION_ID(mission);

    return &sHerbShopCatches[id < ARRAY_COUNT(sHerbShopCatches) ? id : 0];
}

static const struct HerbShopIngredient *GetIngredient(u16 mission, u8 n)
{
    u16 id = (MISSION_ID(mission) >> (INGREDIENT_BITS * n)) & ((1 << INGREDIENT_BITS) - 1);

    return &sHerbShopIngredients[id < ARRAY_COUNT(sHerbShopIngredients) ? id : 0];
}

// The mission to be done, or 0. Missions last for the day they were given, except the escort
// that opens the shop and an escort she has already set off on with the player.
static u16 GetActiveMission(void)
{
    u16 mission = VarGet(VAR_HERB_SHOP_MISSION);

    if (!FlagGet(FLAG_HERB_SHOP_MISSION_ACTIVE))
        return 0;
    if (FlagGet(FLAG_HERB_SHOP_OPEN) && !FlagGet(FLAG_DAILY_HERB_SHOP_MISSION)
     && !(MISSION_KIND(mission) == HERB_MISSION_ESCORT && MISSION_PHASE(mission) != ESCORT_WAITING))
    {
        FlagClear(FLAG_HERB_SHOP_MISSION_ACTIVE);
        return 0;
    }
    return mission;
}

static void SetEscortPhase(u8 phase)
{
    u16 mission = VarGet(VAR_HERB_SHOP_MISSION);

    VarSet(VAR_HERB_SHOP_MISSION, (mission & ~MISSION_PHASE_MASK) | (phase << MISSION_PHASE_SHIFT));
}

// The escort she is waiting on a route for, or NULL. Called while loading a map other than the
// grotto room, where an escort can't be under way: one that is never reached the grotto.
static const struct HerbShopEscort *GetWaitingEscort(void)
{
    u16 mission = GetActiveMission();

    if (MISSION_KIND(mission) != HERB_MISSION_ESCORT)
        return NULL;
    if (MISSION_PHASE(mission) == ESCORT_LEADING)
    {
        SetEscortPhase(ESCORT_WAITING);
        mission = GetActiveMission();
        if (mission == 0)
            return NULL;
    }
    if (MISSION_PHASE(mission) != ESCORT_WAITING)
        return NULL;
    return GetEscort(mission);
}

// STR_VAR_1 to 3 for the mission's texts. Escort: the route, where on it she waits, the item she
// finds. Catch: the Pokemon, where to find it. Fetch: the three ingredients.
static void BufferMission(u16 mission)
{
    switch (MISSION_KIND(mission))
    {
    case HERB_MISSION_ESCORT:
    {
        const struct HerbShopEscort *escort = GetEscort(mission);

        GetMapName(gStringVar1, Overworld_GetMapHeaderByGroupAndId(escort->mapGroup, escort->mapNum)->regionMapSectionId, 0);
        StringCopy(gStringVar2, escort->where);
        CopyItemName(escort->item, gStringVar3);
        break;
    }
    case HERB_MISSION_CATCH:
    {
        const struct HerbShopCatch *catch = GetCatch(mission);

        StringCopy(gStringVar1, gSpeciesNames[catch->species]);
        StringCopy(gStringVar2, catch->hint);
        break;
    }
    case HERB_MISSION_FETCH:
        CopyItemName(GetIngredient(mission, 0)->item, gStringVar1);
        CopyItemName(GetIngredient(mission, 1)->item, gStringVar2);
        CopyItemName(GetIngredient(mission, 2)->item, gStringVar3);
        break;
    }
}

static u8 GetUnlockLevel(void)
{
    if (FlagGet(FLAG_DEFEATED_RUSTBORO_GYM))
        return HERB_UNLOCK_GYM;
    if (VarGet(VAR_PETALBURG_WOODS_STATE) != 0)
        return HERB_UNLOCK_GRUNT;
    return HERB_UNLOCK_START;
}

static u16 RollRecipe(void)
{
    u8 pool[ARRAY_COUNT(sHerbShopIngredients)];
    u16 recipe = 0;
    u8 i, j, picked;

    for (i = 0; i < ARRAY_COUNT(sHerbShopIngredients); i++)
        pool[i] = i;
    for (i = 0; i < INGREDIENTS_PER_RECIPE; i++)
    {
        j = i + Random() % (ARRAY_COUNT(sHerbShopIngredients) - i);
        picked = pool[j];
        pool[j] = pool[i];
        recipe |= picked << (INGREDIENT_BITS * i);
    }
    return recipe;
}

// Picks from every unlocked mission by weight, leaving out the one she gave last.
static u16 RollMission(void)
{
    u16 missions[MAX_CANDIDATES];
    u8 weights[MAX_CANDIDATES];
    u16 last = VarGet(VAR_HERB_SHOP_MISSION) & ~MISSION_PHASE_MASK;
    u8 level = GetUnlockLevel();
    u16 total = 0, roll;
    u8 count = 0, i;

    for (i = 0; i < ARRAY_COUNT(sHerbShopEscorts); i++)
    {
        if (sHerbShopEscorts[i].unlock <= level)
        {
            missions[count] = MISSION(HERB_MISSION_ESCORT, i);
            weights[count++] = HERB_ESCORT_WEIGHT;
        }
    }
    for (i = 0; i < ARRAY_COUNT(sHerbShopCatches); i++)
    {
        if (sHerbShopCatches[i].unlock <= level && !FlagGet(sHerbShopCatches[i].flag))
        {
            missions[count] = MISSION(HERB_MISSION_CATCH, i);
            weights[count++] = HERB_CATCH_WEIGHT;
        }
    }
    if (HERB_FETCH_UNLOCK <= level)
    {
        missions[count] = MISSION(HERB_MISSION_FETCH, 0);
        weights[count++] = HERB_FETCH_WEIGHT;
    }

    for (i = 0; i < count; i++)
    {
        if (missions[i] == last && count > 1)
            weights[i] = 0;
        total += weights[i];
    }
    roll = Random() % total;
    for (i = 0; i < count - 1; i++)
    {
        if (roll < weights[i])
            break;
        roll -= weights[i];
    }
    if (MISSION_KIND(missions[i]) == HERB_MISSION_FETCH)
        return MISSION(HERB_MISSION_FETCH, RollRecipe());
    return missions[i];
}

static void StartMission(u16 mission)
{
    VarSet(VAR_HERB_SHOP_MISSION, mission);
    FlagSet(FLAG_HERB_SHOP_MISSION_ACTIVE);
    BufferMission(mission);
}

// ---------------------------------------------------------------- the shop

// On-transition: hides the shop's objects with its temp flags.
void HerbShop_SetupShop(void)
{
    u16 mission;
    u8 i;

    GetWaitingEscort();
    mission = GetActiveMission();
    if (!FlagGet(FLAG_HERB_SHOP_OPEN))
        FlagSet(FLAG_HIDE_HERB_SHOP_STAFF);
    if (MISSION_KIND(mission) == HERB_MISSION_ESCORT)
    {
        if (MISSION_PHASE(mission) == ESCORT_RETURNING)
        {
            // The player arrives on her left (see HerbShop_EventScript_EscortInGrotto)
            SetObjEventTemplateMovementType(LOCALID_HERB_SHOP_OWNER, MOVEMENT_TYPE_FACE_LEFT);
            VarSet(VAR_HERB_SHOP_ESCORT_RETURNED, 1);
        }
        else
        {
            FlagSet(FLAG_HIDE_HERB_SHOP_OWNER);
        }
    }
    for (i = 0; i < ARRAY_COUNT(sHerbShopCatches); i++)
    {
        if (!FlagGet(sHerbShopCatches[i].flag))
            FlagSet(sHerbShopCatches[i].hideFlag);
    }
}

// VAR_RESULT = HERB_MISSION_* of the mission to be done, with its texts in STR_VAR_1 to 3.
u16 HerbShop_GetMission(void)
{
    u16 mission = GetActiveMission();

    BufferMission(mission);
    return MISSION_KIND(mission);
}

// Today's mission, returned like HerbShop_GetMission.
u16 HerbShop_GiveMission(void)
{
    u16 mission = RollMission();

    StartMission(mission);
    FlagSet(FLAG_DAILY_HERB_SHOP_MISSION);
    return MISSION_KIND(mission);
}

void HerbShop_StartFirstEscort(void)
{
    StartMission(MISSION(HERB_MISSION_ESCORT, FIRST_ESCORT));
}

// VAR_RESULT = whether the player has the Pokemon or all the ingredients she asked for.
u16 HerbShop_CanCompleteMission(void)
{
    u16 mission = GetActiveMission();
    u8 i;

    switch (MISSION_KIND(mission))
    {
    case HERB_MISSION_CATCH:
        for (i = 0; i < PARTY_SIZE; i++)
        {
            if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG) == GetCatch(mission)->species)
                return TRUE;
        }
        return FALSE;
    case HERB_MISSION_FETCH:
        for (i = 0; i < INGREDIENTS_PER_RECIPE; i++)
        {
            if (!CheckBagHasItem(GetIngredient(mission, i)->item, 1))
                return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

// Takes the party Pokemon in slot VAR_0x8004 if it's the one she asked for and the player can
// still battle without it. VAR_RESULT = HERB_GIVE_MON_*.
u16 HerbShop_GiveChosenMon(void)
{
    u8 slot = gSpecialVar_0x8004;
    u8 i;

    if (slot >= PARTY_SIZE
     || GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES_OR_EGG) != GetCatch(VarGet(VAR_HERB_SHOP_MISSION))->species)
        return HERB_GIVE_MON_WRONG;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES_OR_EGG);

        if (i != slot && species != SPECIES_NONE && species != SPECIES_EGG && GetMonData(&gPlayerParty[i], MON_DATA_HP) != 0)
            break;
    }
    if (i == PARTY_SIZE)
        return HERB_GIVE_MON_LAST;

    gSpecialVar_0x8005 = slot;
    ReleaseMonFromParty();
    return HERB_GIVE_MON_OK;
}

void HerbShop_GiveIngredients(void)
{
    u16 mission = VarGet(VAR_HERB_SHOP_MISSION);
    u8 i;

    for (i = 0; i < INGREDIENTS_PER_RECIPE; i++)
        RemoveBagItem(GetIngredient(mission, i)->item, 1);
}

// Ends the mission. VAR_RESULT = the IP it is worth. For a catch mission the Pokemon moves into
// the shop: VAR_0x8004 = its object to add, VAR_0x8005 = its species.
u16 HerbShop_CompleteMission(void)
{
    u16 mission = VarGet(VAR_HERB_SHOP_MISSION);
    u16 reward = 0;
    u8 i;

    switch (MISSION_KIND(mission))
    {
    case HERB_MISSION_ESCORT:
        reward = HERB_ESCORT_IP;
        break;
    case HERB_MISSION_CATCH:
    {
        const struct HerbShopCatch *catch = GetCatch(mission);

        FlagSet(catch->flag);
        FlagClear(catch->hideFlag);
        gSpecialVar_0x8004 = catch->localId;
        gSpecialVar_0x8005 = catch->species;
        reward = HERB_CATCH_IP;
        break;
    }
    case HERB_MISSION_FETCH:
        for (i = 0; i < INGREDIENTS_PER_RECIPE; i++)
            reward += GetIngredient(mission, i)->ip;
        break;
    }
    VarSet(VAR_HERB_SHOP_MISSION, mission & ~MISSION_PHASE_MASK);
    FlagClear(FLAG_HERB_SHOP_MISSION_ACTIVE);
    return reward;
}

// ---------------------------------------------------------------- the escort

// Shows her on this map if she is waiting here (FLAG_HIDE_HERB_SHOP_ESCORT), from the map's
// on-transition script and again when she is talked to. VAR_RESULT = whether she is.
u16 HerbShop_UpdateEscortNpc(void)
{
    const struct HerbShopEscort *escort = GetWaitingEscort();

    if (escort != NULL && escort->mapGroup == gSaveBlock1Ptr->location.mapGroup && escort->mapNum == gSaveBlock1Ptr->location.mapNum)
    {
        FlagClear(FLAG_HIDE_HERB_SHOP_ESCORT);
        return TRUE;
    }
    FlagSet(FLAG_HIDE_HERB_SHOP_ESCORT);
    return FALSE;
}

static u8 StepDirection(char step)
{
    switch (step)
    {
    case 'U': return DIR_NORTH;
    case 'D': return DIR_SOUTH;
    case 'L': return DIR_WEST;
    default:  return DIR_EAST;
    }
}

static u8 GetPathLength(const char *path)
{
    u8 length = 0;

    while (path[length] != '\0' && length < ESCORT_MAX_STEPS)
        length++;
    return length;
}

static void MoveObject(u8 localId, const u8 *movement)
{
    ScriptMovement_StartObjectMovementScript(localId, gSaveBlock1Ptr->location.mapNum, gSaveBlock1Ptr->location.mapGroup, movement);
}

// She walks her path up to the grotto. The player steps onto the tile she stood on and stays one
// step behind her, like a follower Pokemon, and ends up facing her.
static void StartEscortWalk(const struct HerbShopEscort *escort)
{
    s16 playerX, playerY;
    u8 dir, length, i;
    u8 playerDir = DIR_NONE;
    u8 n = 0;

    PlayerGetDestCoords(&playerX, &playerY);
    playerX -= MAP_OFFSET;
    playerY -= MAP_OFFSET;

    sEscortPath = escort->path;
    dir = StepDirection(sEscortPath[0]);
    if (playerX == escort->x + sDirectionSteps[dir][0] && playerY == escort->y + sDirectionSteps[dir][1])
        sEscortPath = escort->altPath;
    length = GetPathLength(sEscortPath);
    if (length < 2)
        return;

    for (i = 0; i + 1 < length; i++)
        sOwnerMovement[i] = sWalkActions[StepDirection(sEscortPath[i])];
    sOwnerMovement[i] = MOVEMENT_ACTION_STEP_END;

    for (dir = DIR_SOUTH; dir <= DIR_EAST; dir++)
    {
        if (playerX + sDirectionSteps[dir][0] == escort->x && playerY + sDirectionSteps[dir][1] == escort->y)
            playerDir = dir;
    }
    if (playerDir != DIR_NONE)
        sPlayerMovement[n++] = sWalkActions[playerDir];
    for (i = 0; i + 2 < length; i++)
    {
        playerDir = StepDirection(sEscortPath[i]);
        sPlayerMovement[n++] = sWalkActions[playerDir];
    }
    dir = StepDirection(sEscortPath[length - 2]);
    if (playerDir != dir)
        sPlayerMovement[n++] = sTurnActions[dir];
    sPlayerMovement[n] = MOVEMENT_ACTION_STEP_END;

    MoveObject(gSpecialVar_LastTalked, sOwnerMovement);
    MoveObject(LOCALID_PLAYER, sPlayerMovement);
    SetEscortPhase(ESCORT_LEADING);
}

// VAR_0x8004 = HERB_ESCORT_MOVE_*. Starts the movement of her (VAR_LAST_TALKED) or the player
// for the script to wait on: the directions depend on the route and on where the player stood.
void HerbShop_EscortMove(void)
{
    const struct HerbShopEscort *escort = GetEscort(VarGet(VAR_HERB_SHOP_MISSION));
    u8 length, intoGrotto, lastStep;

    if (gSpecialVar_0x8004 == HERB_ESCORT_MOVE_WALK)
    {
        StartEscortWalk(escort);
        return;
    }

    if (sEscortPath == NULL)
        sEscortPath = escort->path;
    length = GetPathLength(sEscortPath);
    if (length < 2)
        return;
    intoGrotto = StepDirection(sEscortPath[length - 1]);
    lastStep = StepDirection(sEscortPath[length - 2]);

    if (gSpecialVar_0x8004 == HERB_ESCORT_MOVE_OWNER_ENTER)
    {
        sOwnerMovement[0] = sFaceActions[intoGrotto];
        sOwnerMovement[1] = MOVEMENT_ACTION_DELAY_8;
        sOwnerMovement[2] = sWalkActions[intoGrotto];
        sOwnerMovement[3] = MOVEMENT_ACTION_SET_INVISIBLE;
        sOwnerMovement[4] = MOVEMENT_ACTION_STEP_END;
        MoveObject(gSpecialVar_LastTalked, sOwnerMovement);
    }
    else
    {
        u8 n = 0;

        sPlayerMovement[n++] = sWalkActions[lastStep];
        if (lastStep != intoGrotto)
            sPlayerMovement[n++] = sTurnActions[intoGrotto];
        sPlayerMovement[n] = MOVEMENT_ACTION_STEP_END;
        MoveObject(LOCALID_PLAYER, sPlayerMovement);
    }
}

// The grotto room's on-transition, after SetupHiddenGrottoRoom: if she led the player here, she
// is in the room with the item she came for, and the room's on-frame script takes over.
void HerbShop_SetupGrottoEscort(void)
{
    u16 mission = GetActiveMission();

    if (MISSION_KIND(mission) != HERB_MISSION_ESCORT || MISSION_PHASE(mission) != ESCORT_LEADING)
        return;
    ShowItemInHiddenGrottoRoom(GetEscort(mission)->item);
    FlagClear(FLAG_HIDE_GROTTO_HERB_SHOP_OWNER);
    VarSet(VAR_GROTTO_HERB_SHOP_ESCORT, 1);
}

// She has her item: next is the warp to the shop (HerbShop_SetupShop).
void HerbShop_FinishGrottoEscort(void)
{
    SetEscortPhase(ESCORT_RETURNING);
}

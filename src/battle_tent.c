#include "global.h"
#include "battle_tent.h"
#include "overworld.h"
#include "event_data.h"
#include "region_map.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_tower.h"
#include "random.h"
#include "item.h"
#include "battle_factory_screen.h"
#include "frontier_util.h"
#include "string_util.h"
#include "constants/battle_frontier.h"
#include "constants/battle_tent.h"
#include "constants/battle_tent_trainers.h"
#include "constants/battle_tent_mons.h"
#include "constants/items.h"
#include "constants/layouts.h"
#include "constants/region_map_sections.h"
#include "constants/trainers.h"

// This file's functions.
static void InitVerdanturfTentChallenge(void);
static void GetVerdanturfTentPrize(void);
static void SetVerdanturfTentPrize(void);
static void SetVerdanturfTentTrainerGfx(void);
static void BufferVerdanturfTentTrainerIntro(void);
static void SaveVerdanturfTentChallenge(void);
static void SetRandomVerdanturfTentPrize(void);
static void GiveVerdanturfTentPrize(void);
static void InitFallarborTentChallenge(void);
static void GetFallarborTentPrize(void);
static void SetFallarborTentPrize(void);
static void SaveFallarborTentChallenge(void);
static void SetRandomFallarborTentPrize(void);
static void GiveFallarborTentPrize(void);
static void BufferFallarborTentTrainerName(void);
static void InitSlateportTentChallenge(void);
static void GetSlateportTentPrize(void);
static void SetSlateportTentPrize(void);
static void SaveSlateportTentChallenge(void);
static void SetRandomSlateportTentPrize(void);
static void GiveSlateportTentPrize(void);
static void SelectInitialRentalMons(void);
static void SwapRentalMons(void);
static void GenerateOpponentMons(void);
static void GenerateInitialRentalMons(void);

/*
 * Battle Tents are mini versions of particular Battle Frontier facilities
 * As such they each share some scripts and functions with their counterpart
 *
 * Verdanturf Battle Tent: Battle Palace
 * Fallarbor Battle Tent:  Battle Arena
 * Slateport Battle Tent:  Battle Factory
 *
 */

static u16 sRandMonId;

void static (*const sVerdanturfTentFuncs[])(void) =
{
    [VERDANTURF_TENT_FUNC_INIT]               = InitVerdanturfTentChallenge,
    [VERDANTURF_TENT_FUNC_GET_PRIZE]          = GetVerdanturfTentPrize,
    [VERDANTURF_TENT_FUNC_SET_PRIZE]          = SetVerdanturfTentPrize,
    [VERDANTURF_TENT_FUNC_SET_OPPONENT_GFX]   = SetVerdanturfTentTrainerGfx,
    [VERDANTURF_TENT_FUNC_GET_OPPONENT_INTRO] = BufferVerdanturfTentTrainerIntro,
    [VERDANTURF_TENT_FUNC_SAVE]               = SaveVerdanturfTentChallenge,
    [VERDANTURF_TENT_FUNC_SET_RANDOM_PRIZE]   = SetRandomVerdanturfTentPrize,
    [VERDANTURF_TENT_FUNC_GIVE_PRIZE]         = GiveVerdanturfTentPrize
};

void static (*const sFallarborTentFuncs[])(void) =
{
    [FALLARBOR_TENT_FUNC_INIT]              = InitFallarborTentChallenge,
    [FALLARBOR_TENT_FUNC_GET_PRIZE]         = GetFallarborTentPrize,
    [FALLARBOR_TENT_FUNC_SET_PRIZE]         = SetFallarborTentPrize,
    [FALLARBOR_TENT_FUNC_SAVE]              = SaveFallarborTentChallenge,
    [FALLARBOR_TENT_FUNC_SET_RANDOM_PRIZE]  = SetRandomFallarborTentPrize,
    [FALLARBOR_TENT_FUNC_GIVE_PRIZE]        = GiveFallarborTentPrize,
    [FALLARBOR_TENT_FUNC_GET_OPPONENT_NAME] = BufferFallarborTentTrainerName
};

void static (*const sSlateportTentFuncs[])(void) =
{
    [SLATEPORT_TENT_FUNC_INIT]                   = InitSlateportTentChallenge,
    [SLATEPORT_TENT_FUNC_GET_PRIZE]              = GetSlateportTentPrize,
    [SLATEPORT_TENT_FUNC_SET_PRIZE]              = SetSlateportTentPrize,
    [SLATEPORT_TENT_FUNC_SAVE]                   = SaveSlateportTentChallenge,
    [SLATEPORT_TENT_FUNC_SET_RANDOM_PRIZE]       = SetRandomSlateportTentPrize,
    [SLATEPORT_TENT_FUNC_GIVE_PRIZE]             = GiveSlateportTentPrize,
    [SLATEPORT_TENT_FUNC_SELECT_RENT_MONS]       = SelectInitialRentalMons,
    [SLATEPORT_TENT_FUNC_SWAP_RENT_MONS]         = SwapRentalMons,
    [SLATEPORT_TENT_FUNC_GENERATE_OPPONENT_MONS] = GenerateOpponentMons,
    [SLATEPORT_TENT_FUNC_GENERATE_RENTAL_MONS]   = GenerateInitialRentalMons
};

// The prize for winning at a Battle Tent: TENT_PRIZE_ITEMS items, each any of these (the same one
// can come up more than once).
static const u16 sTentPrizeItems[] =
{
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CARBOS,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_PP_UP,
    ITEM_PP_MAX,
    ITEM_RARE_CANDY,
};

// A prize waits in its tent's save field until the Bag has room for it: F_TENT_PRIZE, and an
// index into sTentPrizeItems in every TENT_PRIZE_ITEM_BITS bits.
#define TENT_PRIZE_ITEMS     3
#define TENT_PRIZE_ITEM_BITS 4
#define F_TENT_PRIZE         (1 << 15)

STATIC_ASSERT(ARRAY_COUNT(sTentPrizeItems) <= (1 << TENT_PRIZE_ITEM_BITS), TentPrizeItemsFitTheirBits)

// code
static u16 GetRandomTentPrize(void)
{
    u32 i;
    u16 prize = F_TENT_PRIZE;

    for (i = 0; i < TENT_PRIZE_ITEMS; i++)
        prize |= (Random() % ARRAY_COUNT(sTentPrizeItems)) << (i * TENT_PRIZE_ITEM_BITS);
    return prize;
}

static u16 GetTentPrizeItem(u16 prize, u32 i)
{
    u32 id = (prize >> (i * TENT_PRIZE_ITEM_BITS)) & ((1 << TENT_PRIZE_ITEM_BITS) - 1);

    return sTentPrizeItems[id % ARRAY_COUNT(sTentPrizeItems)];
}

// Gives the prize waiting in *prize: all of its items, or none of them if the Bag has no room.
// Buffers the items' names in gStringVar1 to gStringVar3.
static void GiveTentPrize(u16 *prize)
{
    static u8 *const sNameBuffers[TENT_PRIZE_ITEMS] = {gStringVar1, gStringVar2, gStringVar3};
    u32 i;

    // A prize won before the prizes changed was a single item: it becomes a new prize
    if (!(*prize & F_TENT_PRIZE))
        *prize = GetRandomTentPrize();

    for (i = 0; i < TENT_PRIZE_ITEMS; i++)
    {
        if (!AddBagItem(GetTentPrizeItem(*prize, i), 1))
            break;
    }
    if (i != TENT_PRIZE_ITEMS)
    {
        while (i-- != 0)
            RemoveBagItem(GetTentPrizeItem(*prize, i), 1);
        gSpecialVar_Result = FALSE;
        return;
    }

    for (i = 0; i < TENT_PRIZE_ITEMS; i++)
        CopyItemName(GetTentPrizeItem(*prize, i), sNameBuffers[i]);
    *prize = ITEM_NONE;
    gSpecialVar_Result = TRUE;
}

void CallVerdanturfTentFunction(void)
{
    sVerdanturfTentFuncs[gSpecialVar_0x8004]();
}

static void InitVerdanturfTentChallenge(void)
{
    gSaveBlock2Ptr->frontier.challengeStatus = 0;
    gSaveBlock2Ptr->frontier.curChallengeBattleNum = 0;
    gSaveBlock2Ptr->frontier.challengePaused = FALSE;
    SetDynamicWarp(0, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE);
}

static void GetVerdanturfTentPrize(void)
{
    gSpecialVar_Result = gSaveBlock2Ptr->frontier.verdanturfTentPrize;
}

static void SetVerdanturfTentPrize(void)
{
    gSaveBlock2Ptr->frontier.verdanturfTentPrize = gSpecialVar_0x8006;
}

static void SetVerdanturfTentTrainerGfx(void)
{
    gTrainerBattleOpponent_A = (u32)((Random() % 255) * 5) / 64;
    SetBattleFacilityTrainerGfxId(gTrainerBattleOpponent_A, 0);
}

static void BufferVerdanturfTentTrainerIntro(void)
{
    if (gTrainerBattleOpponent_A < FRONTIER_TRAINERS_COUNT)
        FrontierSpeechToString(gFacilityTrainers[gTrainerBattleOpponent_A].speechBefore);
}

static void SaveVerdanturfTentChallenge(void)
{
    gSaveBlock2Ptr->frontier.challengeStatus = gSpecialVar_0x8005;
    VarSet(VAR_TEMP_CHALLENGE_STATUS, 0);
    gSaveBlock2Ptr->frontier.challengePaused = TRUE;
    SaveGameFrontier();
}

static void SetRandomVerdanturfTentPrize(void)
{
    gSaveBlock2Ptr->frontier.verdanturfTentPrize = GetRandomTentPrize();
}

static void GiveVerdanturfTentPrize(void)
{
    GiveTentPrize(&gSaveBlock2Ptr->frontier.verdanturfTentPrize);
}

void CallFallarborTentFunction(void)
{
    sFallarborTentFuncs[gSpecialVar_0x8004]();
}

static void InitFallarborTentChallenge(void)
{
    gSaveBlock2Ptr->frontier.challengeStatus = 0;
    gSaveBlock2Ptr->frontier.curChallengeBattleNum = 0;
    gSaveBlock2Ptr->frontier.challengePaused = FALSE;
    SetDynamicWarp(0, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE);
}

static void GetFallarborTentPrize(void)
{
    gSpecialVar_Result = gSaveBlock2Ptr->frontier.fallarborTentPrize;
}

static void SetFallarborTentPrize(void)
{
    gSaveBlock2Ptr->frontier.fallarborTentPrize = gSpecialVar_0x8006;
}

static void SaveFallarborTentChallenge(void)
{
    gSaveBlock2Ptr->frontier.challengeStatus = gSpecialVar_0x8005;
    VarSet(VAR_TEMP_CHALLENGE_STATUS, 0);
    gSaveBlock2Ptr->frontier.challengePaused = TRUE;
    SaveGameFrontier();
}

static void SetRandomFallarborTentPrize(void)
{
    gSaveBlock2Ptr->frontier.fallarborTentPrize = GetRandomTentPrize();
}

static void GiveFallarborTentPrize(void)
{
    GiveTentPrize(&gSaveBlock2Ptr->frontier.fallarborTentPrize);
}

static void BufferFallarborTentTrainerName(void)
{
    GetFrontierTrainerName(gStringVar1, gTrainerBattleOpponent_A);
}

void CallSlateportTentFunction(void)
{
    sSlateportTentFuncs[gSpecialVar_0x8004]();
}

static void InitSlateportTentChallenge(void)
{
    gSaveBlock2Ptr->frontier.challengeStatus = 0;
    gSaveBlock2Ptr->frontier.curChallengeBattleNum = 0;
    gSaveBlock2Ptr->frontier.challengePaused = FALSE;
    SetDynamicWarp(0, gSaveBlock1Ptr->location.mapGroup, gSaveBlock1Ptr->location.mapNum, WARP_ID_NONE);
}

static void GetSlateportTentPrize(void)
{
    gSpecialVar_Result = gSaveBlock2Ptr->frontier.slateportTentPrize;
}

static void SetSlateportTentPrize(void)
{
    gSaveBlock2Ptr->frontier.slateportTentPrize = gSpecialVar_0x8006;
}

static void SaveSlateportTentChallenge(void)
{
    gSaveBlock2Ptr->frontier.challengeStatus = gSpecialVar_0x8005;
    VarSet(VAR_TEMP_CHALLENGE_STATUS, 0);
    gSaveBlock2Ptr->frontier.challengePaused = TRUE;
    SaveGameFrontier();
}

static void SetRandomSlateportTentPrize(void)
{
    gSaveBlock2Ptr->frontier.slateportTentPrize = GetRandomTentPrize();
}

static void GiveSlateportTentPrize(void)
{
    GiveTentPrize(&gSaveBlock2Ptr->frontier.slateportTentPrize);
}

static void SelectInitialRentalMons(void)
{
    ZeroPlayerPartyMons();
    DoBattleFactorySelectScreen();
}

static void SwapRentalMons(void)
{
    DoBattleFactorySwapScreen();
}

bool8 InSlateportBattleTent(void)
{
    return gMapHeader.regionMapSectionId == MAPSEC_SLATEPORT_CITY
           && (gMapHeader.mapLayoutId == LAYOUT_BATTLE_TENT_CORRIDOR || gMapHeader.mapLayoutId == LAYOUT_BATTLE_TENT_BATTLE_ROOM);
}

static void GenerateInitialRentalMons(void)
{
    s32 i, j;
    u8 firstMonId;
    u16 monSetId;
    u16 currSpecies;
    u16 species[PARTY_SIZE];
    u16 monIds[PARTY_SIZE];
    u16 heldItems[PARTY_SIZE];
    u32 draws = 0;

    firstMonId = 0;
    gFacilityTrainers = gSlateportBattleTentTrainers;
    for (i = 0; i < PARTY_SIZE; i++)
    {
        species[i] = 0;
        monIds[i] = 0;
        heldItems[i] = 0;
    }
    gFacilityTrainerMons = gSlateportBattleTentMons;
    currSpecies = SPECIES_NONE;
    i = 0;
    while (i != PARTY_SIZE)
    {
        // Cannot have two Pokémon of the same species.
        monSetId = Random() % NUM_SLATEPORT_TENT_MONS;
        draws++;
        for (j = firstMonId; j < firstMonId + i; j++)
        {
            if (monIds[j] == monSetId)
                break;
            if (species[j] == gFacilityTrainerMons[monSetId].species)
            {
                if (currSpecies == SPECIES_NONE)
                    currSpecies = gFacilityTrainerMons[monSetId].species;
                else
                    break;
            }
        }
        if (j != i + firstMonId)
            continue;

        // Cannot have two same held items.
        for (j = firstMonId; j < i + firstMonId; j++)
        {
            if (draws < FRONTIER_ITEM_CLAUSE_DRAWS
             && heldItems[j] != 0 && heldItems[j] == gBattleFrontierHeldItems[gFacilityTrainerMons[monSetId].itemTableId])
            {
                if (gFacilityTrainerMons[monSetId].species == currSpecies)
                    currSpecies = SPECIES_NONE;
                break;
            }
        }
        if (j != i + firstMonId)
            continue;

        gSaveBlock2Ptr->frontier.rentalMons[i].monId = monSetId;
        species[i] = gFacilityTrainerMons[monSetId].species;
        heldItems[i] = gBattleFrontierHeldItems[gFacilityTrainerMons[monSetId].itemTableId];
        monIds[i] = monSetId;
        i++;
    }
}

static void GenerateOpponentMons(void)
{
    u16 trainerId;
    s32 i, j, k;
    const u16 *monSet;
    u16 species[FRONTIER_PARTY_SIZE];
    u16 heldItems[FRONTIER_PARTY_SIZE];
    s32 numMons = 0;
    u32 draws = 0;

    gFacilityTrainers = gSlateportBattleTentTrainers;
    gFacilityTrainerMons = gSlateportBattleTentMons;

    while (1)
    {
        do
        {
            // Choose a random trainer, ensuring no repeats in this challenge
            trainerId = Random() % NUM_BATTLE_TENT_TRAINERS;
            for (i = 0; i < gSaveBlock2Ptr->frontier.curChallengeBattleNum; i++)
            {
                if (gSaveBlock2Ptr->frontier.trainerIds[i] == trainerId)
                    break;
            }
        } while (i != gSaveBlock2Ptr->frontier.curChallengeBattleNum);

        gTrainerBattleOpponent_A = trainerId;
        monSet = gFacilityTrainers[gTrainerBattleOpponent_A].monSet;
        while (monSet[numMons] != 0xFFFF)
            numMons++;
        if (numMons > 8)
            break;
        numMons = 0;
    }

    if (gSaveBlock2Ptr->frontier.curChallengeBattleNum < TENT_STAGES_PER_CHALLENGE - 1)
        gSaveBlock2Ptr->frontier.trainerIds[gSaveBlock2Ptr->frontier.curChallengeBattleNum] = gTrainerBattleOpponent_A;

    monSet = gFacilityTrainers[gTrainerBattleOpponent_A].monSet;
    i = 0;
    while (i != FRONTIER_PARTY_SIZE)
    {
        sRandMonId = monSet[Random() % numMons];
        draws++;

        // Ensure none of the opponent's Pokémon are the same as the potential rental Pokémon for the player
        for (j = 0; j < (int)ARRAY_COUNT(gSaveBlock2Ptr->frontier.rentalMons); j++)
        {
            if (gFacilityTrainerMons[sRandMonId].species == gFacilityTrainerMons[gSaveBlock2Ptr->frontier.rentalMons[j].monId].species)
                break;
        }
        if (j != (int)ARRAY_COUNT(gSaveBlock2Ptr->frontier.rentalMons))
            continue;

        // Ensure this species hasn't already been chosen for the opponent
        for (k = 0; k < i; k++)
        {
            if (species[k] == gFacilityTrainerMons[sRandMonId].species)
                break;
        }
        if (k != i)
            continue;

        // Ensure held items don't repeat on the opponent's team
        for (k = 0; k < i; k++)
        {
            if (draws < FRONTIER_ITEM_CLAUSE_DRAWS
             && heldItems[k] != ITEM_NONE && heldItems[k] == gBattleFrontierHeldItems[gFacilityTrainerMons[sRandMonId].itemTableId])
                break;
        }
        if (k != i)
            continue;

        // Successful selection
        species[i] = gFacilityTrainerMons[sRandMonId].species;
        heldItems[i] = gBattleFrontierHeldItems[gFacilityTrainerMons[sRandMonId].itemTableId];
        gFrontierTempParty[i] = sRandMonId;
        i++;
    }
}

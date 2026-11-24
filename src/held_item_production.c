#include "global.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "event_data.h"
#include "field_notification.h"
#include "held_item_production.h"
#include "pokemon.h"
#include "random.h"
#include "constants/abilities.h"
#include "constants/battle_pyramid.h"
#include "constants/items.h"
#include "constants/species.h"

// Items that party Pokemon come to hold on their own: Pickup's finds, Effect Spore's mushrooms, pearls,
// stardust and Shuckle's Berry Juice. Who makes what and how often is all set in the tables of this first
// part of the file. The code that rolls for them is at the bottom and isn't needed to adjust any of it.

// When party Pokemon get to roll. The Battle Pike never gives anything after a battle.
enum
{
    AFTER_BATTLE_WON,           // battle won or wild Pokemon caught
    AFTER_BATTLE_FLED,          // player ran, or the wild Pokemon did
    WHILE_WALKING,              // every STEPS_PER_WALKING_ROLL steps, anywhere
    AFTER_PYRAMID_BATTLE_WON,   // the two battle events, in the Battle Pyramid
    AFTER_PYRAMID_BATTLE_FLED,
    NUM_PRODUCTION_EVENTS
};

#define STEPS_PER_WALKING_ROLL 255

#define MAX_PRODUCER_SPECIES 4
#define MAX_PRODUCER_FROM_ITEMS 8

// Chances are kept as how many of Random()'s 65536 values hit
#define PERCENT(n) ((u32)((n) * 65536 / 100.0 + 0.5))
#define ODDS(itemPercent, rareItemPercent) {PERCENT(itemPercent), PERCENT(rareItemPercent)}

struct ItemProducer
{
    u16 species[MAX_PRODUCER_SPECIES];      // Made by any of these species...
    u8 ability;                             // ...and by any Pokemon with this ability
    bool8 usesPickupTables;                 // item is taken from the Pickup tables below instead
    u16 fromItems[MAX_PRODUCER_FROM_ITEMS]; // Held item that gets turned into it. None: made when holding nothing
    u16 item;
    u16 rareItem;
    struct
    {
        u32 item;
        u32 rareItem;
    } odds[NUM_PRODUCTION_EVENTS];
};

// On each event, every party Pokemon that isn't an Egg goes down this list and gets one roll for each entry
// that fits it: its species or ability, and what it is holding. An item it gets is its held item for the
// entries after, which is how Shuckle's Berry Juice can go on to become a Rare Candy on the same event.
//
// ODDS(item, rareItem) is the chance in percent of getting each on that roll, so the chance of getting
// anything is the two added up (the rare one isn't a share of the other). Decimals work: 6.25 is 1 in 16.
// An event that an entry leaves out never produces it.
static const struct ItemProducer sItemProducers[] =
{
    // Clamperl, Shellder and Cloyster grow pearls
    {
        .species = {SPECIES_CLAMPERL, SPECIES_SHELLDER, SPECIES_CLOYSTER},
        .item = ITEM_PEARL,
        .rareItem = ITEM_BIG_PEARL,
        .odds =
        {
            [AFTER_BATTLE_WON]          = ODDS(10, 5),
            [AFTER_BATTLE_FLED]         = ODDS(5, 1),
            [WHILE_WALKING]             = ODDS(5, 1),
            [AFTER_PYRAMID_BATTLE_WON]  = ODDS(10, 5),
            [AFTER_PYRAMID_BATTLE_FLED] = ODDS(5, 1),
        },
    },

    // Staryu and Starmie shed stardust
    {
        .species = {SPECIES_STARYU, SPECIES_STARMIE},
        .item = ITEM_STARDUST,
        .rareItem = ITEM_STAR_PIECE,
        .odds =
        {
            [AFTER_BATTLE_WON]          = ODDS(10, 5),
            [AFTER_BATTLE_FLED]         = ODDS(5, 1),
            [WHILE_WALKING]             = ODDS(5, 1),
            [AFTER_PYRAMID_BATTLE_WON]  = ODDS(10, 5),
            [AFTER_PYRAMID_BATTLE_FLED] = ODDS(5, 1),
        },
    },

    // Shuckle ferments the Berry it holds into Berry Juice...
    {
        .species = {SPECIES_SHUCKLE},
        .fromItems = {ITEM_ORAN_BERRY, ITEM_SITRUS_BERRY, ITEM_FIGY_BERRY, ITEM_WIKI_BERRY, ITEM_MAGO_BERRY,
                      ITEM_AGUAV_BERRY, ITEM_IAPAPA_BERRY},
        .item = ITEM_BERRY_JUICE,
        .odds =
        {
            [AFTER_BATTLE_WON]          = ODDS(6.25, 0),
            [AFTER_BATTLE_FLED]         = ODDS(6.25, 0),
            [WHILE_WALKING]             = ODDS(6.25, 0),
            [AFTER_PYRAMID_BATTLE_FLED] = ODDS(6.25, 0),
        },
    },

    // ...and its Berry Juice into a Rare Candy
    {
        .species = {SPECIES_SHUCKLE},
        .fromItems = {ITEM_BERRY_JUICE},
        .item = ITEM_RARE_CANDY,
        .odds =
        {
            [AFTER_BATTLE_WON]          = ODDS(1.5625, 0),
            [AFTER_BATTLE_FLED]         = ODDS(1.5625, 0),
            [WHILE_WALKING]             = ODDS(1.5625, 0),
            [AFTER_PYRAMID_BATTLE_FLED] = ODDS(1.5625, 0),
        },
    },

    // Pickup. What it finds is set at sPickupItems
    {
        .ability = ABILITY_PICKUP,
        .usesPickupTables = TRUE,
        .odds =
        {
            [AFTER_BATTLE_WON]          = ODDS(10, 0),
            [AFTER_BATTLE_FLED]         = ODDS(5, 0),
            [WHILE_WALKING]             = ODDS(1, 0),
            [AFTER_PYRAMID_BATTLE_WON]  = ODDS(10, 0),
            [AFTER_PYRAMID_BATTLE_FLED] = ODDS(5, 0),
        },
    },

    // Effect Spore grows mushrooms
    {
        .ability = ABILITY_EFFECT_SPORE,
        .item = ITEM_TINY_MUSHROOM,
        .rareItem = ITEM_BIG_MUSHROOM,
        .odds =
        {
            [AFTER_BATTLE_WON]          = ODDS(10, 5),
            [AFTER_BATTLE_FLED]         = ODDS(5, 1),
            [WHILE_WALKING]             = ODDS(5, 1),
            [AFTER_PYRAMID_BATTLE_WON]  = ODDS(10, 5),
            [AFTER_PYRAMID_BATTLE_FLED] = ODDS(5, 1),
        },
    },
};

// What Pickup finds once its roll in sItemProducers succeeds. After a battle in the Battle Pyramid it is
// GetBattlePyramidPickupItemId that chooses instead. Otherwise the item depends on the Pokemon's level:
//   offset = (level - 1) / 10      Lv 1-10 = 0, Lv 11-20 = 1, ... Lv 91-100 = 9
//   rand   = Random() % 100
//   rand  0-29   sPickupItems[offset + 0]       30%
//   rand 30-89   sPickupItems[offset + 1..6]    10% each
//   rand 90-97   sPickupItems[offset + 7..8]     4% each
//   rand 98      sRarePickupItems[offset + 1]    1%
//   rand 99      sRarePickupItems[offset + 0]    1%
// So a Pokemon sees a window of 9 common and 2 rare items, and the window slides down one entry every 10 levels.
//
// To adjust:
//   - What is found: edit the lists. Keep sPickupItems at 18 entries (offset 9 + slot 8 is the last index read)
//     and sRarePickupItems at 11 (offset 9 + 1).
//   - Odds of each slot: edit sPickupProbabilities. Each value is the first rand that no longer hits that slot,
//     so a slot's chance is its value minus the previous one. Keep them ascending and the last one at 98:
//     rand 98 and 99 are always the rare rolls, and anything left between the last value and 98 gives no item
//     at all. A slot added here needs one more entry in sPickupItems.
static const u16 sPickupItems[] =
{
    ITEM_HYPER_POTION,  // Lv  1-10
    ITEM_PREMIER_BALL,  // Lv  1-20
    ITEM_REPEL,         // Lv  1-30
    ITEM_POKE_DOLL,     // Lv  1-40
    ITEM_FULL_HEAL,     // Lv  1-50
    ITEM_ULTRA_BALL,    // Lv  1-60
    ITEM_MAX_POTION,    // Lv  1-70
    ITEM_RARE_CANDY,    // Lv  1-80
    ITEM_HP_UP,         // Lv  1-90
    ITEM_PROTEIN,       // Lv 11-100
    ITEM_IRON,          // Lv 21-100
    ITEM_CALCIUM,       // Lv 31-100
    ITEM_ZINC,          // Lv 41-100
    ITEM_CARBOS,        // Lv 51-100
    ITEM_FULL_RESTORE,  // Lv 61-100
    ITEM_MAX_REVIVE,    // Lv 71-100
    ITEM_PP_UP,         // Lv 81-100
    ITEM_MAX_ELIXIR,    // Lv 91-100
};

// Each entry is available for 20 levels: first as the rand 98 item, then as the rand 99 item.
static const u16 sRarePickupItems[] =
{
    ITEM_PP_MAX,        // Lv  1-10
    ITEM_NUGGET,        // Lv  1-20
    ITEM_KINGS_ROCK,    // Lv 11-30
    ITEM_FULL_RESTORE,  // Lv 21-40
    ITEM_SHELL_BELL,    // Lv 31-50
    ITEM_WHITE_HERB,    // Lv 41-60
    ITEM_METAL_COAT,    // Lv 51-70
    ITEM_SCOPE_LENS,    // Lv 61-80
    ITEM_DRAGON_SCALE,  // Lv 71-90
    ITEM_LEFTOVERS,     // Lv 81-100
    ITEM_UP_GRADE,      // Lv 91-100
};

// Cumulative thresholds for the 9 slots of sPickupItems: 30%, 10% x6, 4% x2.
static const u8 sPickupProbabilities[] =
{
    30, 40, 50, 60, 70, 80, 90, 94, 98
};

// End of the settings. Below is the code that applies them.

static u16 ChoosePickupItem(struct Pokemon *mon)
{
    u32 i;
    u32 rand = Random() % 100;
    u32 offset = (GetMonData(mon, MON_DATA_LEVEL) - 1) / 10;

    if (offset > 9)
        offset = 9;

    if (rand >= 98)
        return sRarePickupItems[offset + (99 - rand)];

    for (i = 0; i < ARRAY_COUNT(sPickupProbabilities); i++)
    {
        if (sPickupProbabilities[i] > rand)
            return sPickupItems[offset + i];
    }
    return ITEM_NONE;
}

static bool32 IsProducer(const struct ItemProducer *producer, u16 species, u8 ability)
{
    u32 i;

    if (producer->ability != ABILITY_NONE && producer->ability == ability)
        return TRUE;

    for (i = 0; i < ARRAY_COUNT(producer->species) && producer->species[i] != SPECIES_NONE; i++)
    {
        if (producer->species[i] == species)
            return TRUE;
    }
    return FALSE;
}

static bool32 CanProduceWhileHolding(const struct ItemProducer *producer, u16 heldItem)
{
    u32 i;

    if (producer->fromItems[0] == ITEM_NONE)
        return heldItem == ITEM_NONE;

    for (i = 0; i < ARRAY_COUNT(producer->fromItems) && producer->fromItems[i] != ITEM_NONE; i++)
    {
        if (producer->fromItems[i] == heldItem)
            return TRUE;
    }
    return FALSE;
}

static u16 RollProducedItem(const struct ItemProducer *producer, struct Pokemon *mon, u32 event)
{
    u32 rareOdds = producer->odds[event].rareItem;
    u32 odds = rareOdds + producer->odds[event].item;
    u32 roll;

    if (odds == 0)
        return ITEM_NONE;

    roll = Random();
    if (roll < rareOdds)
        return producer->rareItem;
    if (roll >= odds)
        return ITEM_NONE;
    if (!producer->usesPickupTables)
        return producer->item;
    if (event == AFTER_PYRAMID_BATTLE_WON || event == AFTER_PYRAMID_BATTLE_FLED)
        return GetBattlePyramidPickupItemId();
    return ChoosePickupItem(mon);
}

static void ProduceHeldItems(u32 event)
{
    u32 i, j;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        struct Pokemon *mon = &gPlayerParty[i];
        u16 species = GetMonData(mon, MON_DATA_SPECIES_OR_EGG);
        u16 heldItem, item;
        u8 ability;

        if (species == SPECIES_NONE || species == SPECIES_EGG)
            continue;

        heldItem = GetMonData(mon, MON_DATA_HELD_ITEM);
        ability = gSpeciesInfo[species].abilities[GetMonData(mon, MON_DATA_ABILITY_NUM) ? 1 : 0];

        for (j = 0; j < ARRAY_COUNT(sItemProducers); j++)
        {
            const struct ItemProducer *producer = &sItemProducers[j];

            if (!IsProducer(producer, species, ability) || !CanProduceWhileHolding(producer, heldItem))
                continue;

            item = RollProducedItem(producer, mon, event);
            if (item != ITEM_NONE)
            {
                SetMonData(mon, MON_DATA_HELD_ITEM, &item);
                QueuePickupItemNotification(mon);
                heldItem = item;
            }
        }
    }
}

void TryProduceHeldItemsAfterBattle(bool32 fled)
{
    if (InBattlePike())
        return;

    if (CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE)
        ProduceHeldItems(fled ? AFTER_PYRAMID_BATTLE_FLED : AFTER_PYRAMID_BATTLE_WON);
    else
        ProduceHeldItems(fled ? AFTER_BATTLE_FLED : AFTER_BATTLE_WON);
}

// Called on every step
void TryProduceHeldItemsOnStep(void)
{
    u16 steps = VarGet(VAR_PICKUP_STEP_COUNTER) + 1;

    if (steps >= STEPS_PER_WALKING_ROLL)
    {
        VarSet(VAR_PICKUP_STEP_COUNTER, 0);
        ProduceHeldItems(WHILE_WALKING);
    }
    else
    {
        VarSet(VAR_PICKUP_STEP_COUNTER, steps);
    }
}

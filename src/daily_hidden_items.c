#include "global.h"
#include "daily_hidden_items.h"
#include "event_data.h"
#include "lottery_corner.h"
#include "random.h"
#include "constants/items.h"

// Helper to get trainer ID as a u32 for hash seed
static u32 GetTrainerIdSeed(void)
{
    return T1_READ_32(gSaveBlock2Ptr->playerTrainerId);
}

// Town and City Item Pool
static const u16 sTownAndCityItemPool[] = {
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_HYPER_POTION,
    ITEM_MAX_POTION,
    ITEM_ANTIDOTE,
    ITEM_AWAKENING,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_PARALYZE_HEAL,
    ITEM_FULL_HEAL,
    ITEM_REVIVE,
    ITEM_MAX_REVIVE,
    ITEM_FULL_RESTORE,
    ITEM_ENERGY_POWDER,
    ITEM_ENERGY_ROOT,
    ITEM_HEAL_POWDER,
    ITEM_REVIVAL_HERB,
    ITEM_ETHER,
    ITEM_MAX_ETHER,
    ITEM_ELIXIR,
    ITEM_MAX_ELIXIR,
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_ULTRA_BALL,
    ITEM_PREMIER_BALL,
    ITEM_NEST_BALL,
    ITEM_NET_BALL,
    ITEM_REPEAT_BALL,
    ITEM_TIMER_BALL,
    ITEM_FLUFFY_TAIL,
    ITEM_NUGGET,
    ITEM_STARDUST,
    ITEM_STAR_PIECE,
    ITEM_PP_UP,
    ITEM_PP_MAX,
    ITEM_RARE_CANDY,
    ITEM_BRIGHT_POWDER,
    ITEM_HARD_STONE,
    ITEM_MENTAL_HERB,
    ITEM_MIRACLE_SEED,
    ITEM_POISON_BARB,
    ITEM_QUICK_CLAW,
    ITEM_SHARP_BEAK,
    ITEM_SILVER_POWDER,
    ITEM_STICK,
    ITEM_WHITE_HERB,
    ITEM_LUCKY_EGG,
};

// Cave Item Pool
static const u16 sCaveItemPool[] = {
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_HYPER_POTION,
    ITEM_MAX_POTION,
    ITEM_ANTIDOTE,
    ITEM_AWAKENING,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_PARALYZE_HEAL,
    ITEM_FULL_HEAL,
    ITEM_REVIVE,
    ITEM_MAX_REVIVE,
    ITEM_FULL_RESTORE,
    ITEM_ETHER,
    ITEM_MAX_ETHER,
    ITEM_ELIXIR,
    ITEM_MAX_ELIXIR,
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_ULTRA_BALL,
    ITEM_PREMIER_BALL,
    ITEM_REPEAT_BALL,
    ITEM_TIMER_BALL,
    ITEM_NUGGET,
    ITEM_TINY_MUSHROOM,
    ITEM_BIG_MUSHROOM,
    ITEM_PP_UP,
    ITEM_PP_MAX,
    ITEM_HP_UP,
    ITEM_PROTEIN,
    ITEM_IRON,
    ITEM_CALCIUM,
    ITEM_ZINC,
    ITEM_CARBOS,
    ITEM_PP_UP,
    ITEM_PP_MAX,
    ITEM_RARE_CANDY,
    ITEM_ESCAPE_ROPE,
    ITEM_BRIGHT_POWDER,
    ITEM_HARD_STONE,
    ITEM_DRAGON_FANG,
    ITEM_EVERSTONE,
    ITEM_KINGS_ROCK,
    ITEM_MAGNET,
    ITEM_METAL_COAT,
    ITEM_METAL_POWDER,
    ITEM_SOFT_SAND,
    ITEM_THICK_CLUB,
};

// Ocean Item Pool
static const u16 sOceanItemPool[] = {
    ITEM_POTION,
    ITEM_SUPER_POTION,
    ITEM_HYPER_POTION,
    ITEM_MAX_POTION,
    ITEM_ANTIDOTE,
    ITEM_AWAKENING,
    ITEM_BURN_HEAL,
    ITEM_ICE_HEAL,
    ITEM_PARALYZE_HEAL,
    ITEM_FULL_HEAL,
    ITEM_REVIVE,
    ITEM_MAX_REVIVE,
    ITEM_FULL_RESTORE,
    ITEM_ETHER,
    ITEM_MAX_ETHER,
    ITEM_ELIXIR,
    ITEM_MAX_ELIXIR,
    ITEM_POKE_BALL,
    ITEM_GREAT_BALL,
    ITEM_ULTRA_BALL,
    ITEM_PREMIER_BALL,
    ITEM_REPEAT_BALL,
    ITEM_TIMER_BALL,
    ITEM_NET_BALL,
    ITEM_DIVE_BALL,
    ITEM_PEARL,
    ITEM_BIG_PEARL,
    ITEM_STARDUST,
    ITEM_STAR_PIECE,
    ITEM_HEART_SCALE,
    ITEM_BLUE_SHARD,
    ITEM_GREEN_SHARD,
    ITEM_RED_SHARD,
    ITEM_YELLOW_SHARD,
    ITEM_PP_UP,
    ITEM_PP_MAX,
    ITEM_RARE_CANDY,
    ITEM_DEEP_SEA_TOOTH,
    ITEM_DEEP_SEA_SCALE,
    ITEM_DRAGON_SCALE,
    ITEM_KINGS_ROCK,
    ITEM_MYSTIC_WATER,
    ITEM_NEVER_MELT_ICE,
    ITEM_SEA_INCENSE,
    ITEM_SOFT_SAND,
};

// Item pool types
enum {
    POOL_TOWN_AND_CITY,
    POOL_CAVE,
    POOL_OCEAN,
};

// Configuration for daily hidden item groups
// Each group has multiple spots, but only one is active per day
struct DailyHiddenGroup {
    u8 numSpots;    // Number of possible spots in this group
    u8 seed;        // Unique seed for randomization
    u8 poolType;    // Which item pool to use
};

// Define your daily hidden item groups here
// Group 0 is the first group, add more as needed
static const struct DailyHiddenGroup sDailyHiddenGroups[] = {
    [0] = { .numSpots = 16, .seed = 137, .poolType = POOL_TOWN_AND_CITY },
    [1] = { .numSpots = 16, .seed = 139, .poolType = POOL_TOWN_AND_CITY },
    [2] = { .numSpots = 16, .seed = 149, .poolType = POOL_TOWN_AND_CITY },
    [3] = { .numSpots = 16, .seed = 151, .poolType = POOL_TOWN_AND_CITY },
    [4] = { .numSpots = 16, .seed = 157, .poolType = POOL_TOWN_AND_CITY },
    [5] = { .numSpots = 16, .seed = 163, .poolType = POOL_TOWN_AND_CITY },
    [6] = { .numSpots = 16, .seed = 167, .poolType = POOL_TOWN_AND_CITY },
    [7] = { .numSpots = 16, .seed = 173, .poolType = POOL_TOWN_AND_CITY },
    [8] = { .numSpots = 16, .seed = 179, .poolType = POOL_TOWN_AND_CITY },
    [9] = { .numSpots = 16, .seed = 181, .poolType = POOL_TOWN_AND_CITY },
    [10] = { .numSpots = 16, .seed = 191, .poolType = POOL_CAVE },
    [11] = { .numSpots = 16, .seed = 193, .poolType = POOL_CAVE },
    [12] = { .numSpots = 16, .seed = 197, .poolType = POOL_CAVE },
    [13] = { .numSpots = 16, .seed = 199, .poolType = POOL_CAVE },
    [14] = { .numSpots = 16, .seed = 211, .poolType = POOL_CAVE },
    [15] = { .numSpots = 16, .seed = 223, .poolType = POOL_CAVE },
    [16] = { .numSpots = 16, .seed = 227, .poolType = POOL_CAVE },
    [17] = { .numSpots = 16, .seed = 229, .poolType = POOL_CAVE },
    [18] = { .numSpots = 16, .seed = 233, .poolType = POOL_OCEAN },
    [19] = { .numSpots = 16, .seed = 239, .poolType = POOL_OCEAN },
    [20] = { .numSpots = 16, .seed = 241, .poolType = POOL_OCEAN },
    [21] = { .numSpots = 16, .seed = 251, .poolType = POOL_OCEAN },
    [22] = { .numSpots = 16, .seed = 2, .poolType = POOL_OCEAN },
    [23] = { .numSpots = 16, .seed = 3, .poolType = POOL_OCEAN },
    [24] = { .numSpots = 16, .seed = 5, .poolType = POOL_OCEAN },
    [25] = { .numSpots = 16, .seed = 7, .poolType = POOL_OCEAN },
    // Add more groups here as needed:
    // [26] = { .numSpots = 5, .seed = 11, .poolType = POOL_CAVE },
};

// Daily flags for each group (slot 1) - must match sDailyHiddenGroups entries
static const u16 sDailyHiddenFlags[] = {
    FLAG_DAILY_HIDDEN_GROUP_A,
    FLAG_DAILY_HIDDEN_GROUP_B,
    FLAG_DAILY_HIDDEN_GROUP_C,
    FLAG_DAILY_HIDDEN_GROUP_D,
    FLAG_DAILY_HIDDEN_GROUP_E,
    FLAG_DAILY_HIDDEN_GROUP_F,
    FLAG_DAILY_HIDDEN_GROUP_G,
    FLAG_DAILY_HIDDEN_GROUP_H,
    FLAG_DAILY_HIDDEN_GROUP_I,
    FLAG_DAILY_HIDDEN_GROUP_J,
    FLAG_DAILY_HIDDEN_GROUP_K,
    FLAG_DAILY_HIDDEN_GROUP_L,
    FLAG_DAILY_HIDDEN_GROUP_M,
    FLAG_DAILY_HIDDEN_GROUP_N,
    FLAG_DAILY_HIDDEN_GROUP_O,
    FLAG_DAILY_HIDDEN_GROUP_P,
    FLAG_DAILY_HIDDEN_GROUP_Q,
    FLAG_DAILY_HIDDEN_GROUP_R,
    FLAG_DAILY_HIDDEN_GROUP_S,
    FLAG_DAILY_HIDDEN_GROUP_T,
    FLAG_DAILY_HIDDEN_GROUP_U,
    FLAG_DAILY_HIDDEN_GROUP_V,
    FLAG_DAILY_HIDDEN_GROUP_W,
    FLAG_DAILY_HIDDEN_GROUP_X,
    FLAG_DAILY_HIDDEN_GROUP_Y,
    FLAG_DAILY_HIDDEN_GROUP_Z,
};

#define NUM_DAILY_HIDDEN_GROUPS ARRAY_COUNT(sDailyHiddenGroups)

bool8 IsDailyHiddenItemId(u16 hiddenItemId)
{
    return hiddenItemId >= DAILY_HIDDEN_ITEM_ID_START;
}

u8 GetDailyHiddenGroupIndex(u16 hiddenItemId)
{
    u16 offset;
    
    if (!IsDailyHiddenItemId(hiddenItemId))
        return 0xFF;
    
    offset = hiddenItemId - DAILY_HIDDEN_ITEM_ID_START;
    return offset / DAILY_HIDDEN_MAX_SPOTS;
}

u8 GetDailyHiddenSpotIndex(u16 hiddenItemId)
{
    u16 offset;
    
    if (!IsDailyHiddenItemId(hiddenItemId))
        return 0xFF;
    
    offset = hiddenItemId - DAILY_HIDDEN_ITEM_ID_START;
    return offset % DAILY_HIDDEN_MAX_SPOTS;
}

u16 GetDailyHiddenFlag(u8 groupIndex)
{
    if (groupIndex >= ARRAY_COUNT(sDailyHiddenFlags))
        return 0;
    return sDailyHiddenFlags[groupIndex];
}

// Calculate which spot is active today for a given group (slot 1)
static u8 GetActiveSpotForGroup(u8 groupIndex)
{
    const struct DailyHiddenGroup *group;
    u32 hash;
    
    if (groupIndex >= NUM_DAILY_HIDDEN_GROUPS)
        return 0xFF;
    
    group = &sDailyHiddenGroups[groupIndex];
    
    // Use lottery number + trainer ID + group seed to determine active spot
    // Lottery number changes only on day transitions and is saved
    // Trainer ID ensures different results per save file
    hash = ISO_RANDOMIZE2(GetLotteryNumber() + GetTrainerIdSeed() + group->seed);
    return hash % group->numSpots;
}

// Calculate second active spot for a given group (slot 2)
// Uses a different hash to get a potentially different spot
static u8 GetSecondActiveSpotForGroup(u8 groupIndex)
{
    const struct DailyHiddenGroup *group;
    u32 hash;
    
    if (groupIndex >= NUM_DAILY_HIDDEN_GROUPS)
        return 0xFF;
    
    group = &sDailyHiddenGroups[groupIndex];
    
    // Use a different hash formula to get potentially different spot
    hash = ISO_RANDOMIZE2(GetLotteryNumber() * 7 + GetTrainerIdSeed() + group->seed * 13);
    return hash % group->numSpots;
}

// Calculate third active spot for a given group (slot 3)
static u8 GetThirdActiveSpotForGroup(u8 groupIndex)
{
    const struct DailyHiddenGroup *group;
    u32 hash;
    
    if (groupIndex >= NUM_DAILY_HIDDEN_GROUPS)
        return 0xFF;
    
    group = &sDailyHiddenGroups[groupIndex];
    
    // Use yet another hash formula for the third spot
    hash = ISO_RANDOMIZE2(GetLotteryNumber() * 11 + GetTrainerIdSeed() + group->seed * 19);
    return hash % group->numSpots;
}

// Calculate fourth active spot for a given group (slot 4)
static u8 GetFourthActiveSpotForGroup(u8 groupIndex)
{
    const struct DailyHiddenGroup *group;
    u32 hash;
    
    if (groupIndex >= NUM_DAILY_HIDDEN_GROUPS)
        return 0xFF;
    
    group = &sDailyHiddenGroups[groupIndex];
    
    // Use yet another hash formula for the fourth spot
    hash = ISO_RANDOMIZE2(GetLotteryNumber() * 17 + GetTrainerIdSeed() + group->seed * 23);
    return hash % group->numSpots;
}

bool8 IsDailyHiddenSpotActive(u16 hiddenItemId)
{
    u8 groupIndex;
    u8 spotIndex;
    u8 activeSpot1;
    u8 activeSpot2;
    u8 activeSpot3;
    u8 activeSpot4;
    u16 dailyFlag;
    
    if (!IsDailyHiddenItemId(hiddenItemId))
        return FALSE;
    
    groupIndex = GetDailyHiddenGroupIndex(hiddenItemId);
    spotIndex = GetDailyHiddenSpotIndex(hiddenItemId);
    
    if (groupIndex >= NUM_DAILY_HIDDEN_GROUPS)
        return FALSE;
    
    activeSpot1 = GetActiveSpotForGroup(groupIndex);
    activeSpot2 = GetSecondActiveSpotForGroup(groupIndex);
    activeSpot3 = GetThirdActiveSpotForGroup(groupIndex);
    activeSpot4 = GetFourthActiveSpotForGroup(groupIndex);
    
    // Check if this spot matches any of the four active spots
    if (spotIndex != activeSpot1 && spotIndex != activeSpot2 && spotIndex != activeSpot3 && spotIndex != activeSpot4)
        return FALSE;
    
    // Check if item already collected today (single flag blocks all spots)
    dailyFlag = GetDailyHiddenFlag(groupIndex);
    if (FlagGet(dailyFlag))
        return FALSE;
    
    return TRUE;
}

u16 GetDailyHiddenItem(u8 groupIndex)
{
    const struct DailyHiddenGroup *group;
    u32 hash;
    
    if (groupIndex >= NUM_DAILY_HIDDEN_GROUPS)
        return ITEM_NONE;
    
    group = &sDailyHiddenGroups[groupIndex];
    
    // Use lottery number + trainer ID for item selection
    // Lottery number changes only on day transitions and is saved
    // Trainer ID ensures different items per save file
    hash = ISO_RANDOMIZE2(GetLotteryNumber() * 31 + GetTrainerIdSeed() + group->seed * 17);
    
    // Select item from the appropriate pool
    switch (group->poolType)
    {
    case POOL_CAVE:
        return sCaveItemPool[hash % ARRAY_COUNT(sCaveItemPool)];
    case POOL_OCEAN:
        return sOceanItemPool[hash % ARRAY_COUNT(sOceanItemPool)];
    case POOL_TOWN_AND_CITY:
    default:
        return sTownAndCityItemPool[hash % ARRAY_COUNT(sTownAndCityItemPool)];
    }
}

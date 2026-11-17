// Overrides: custom met data and PID/IV pools for Pokemon the player gets (logic in src/overrides.c).
//
// Caught Pokemon
//   A wild Pokemon caught in battle gets "hatched at" met data (met level 0, friendship 120) at the
//   place it was caught, like the egg it could have come from. Pokemon in the Undiscovered or Ditto
//   egg group can't come from an egg, so they get normal "met at" data instead (met level = their
//   level), unless they are listed in sCaughtAsHatchedSpecies. Legendary battles always get "met at".
//
// Met data overrides (sMetDataOverrides)
//   A "met at" Pokemon caught where it isn't legally found gets its met data moved to a map where
//   it is. Each entry is:
//
//     .species  the Pokemon
//     .region   FLYREGION_* the override applies in (where it was caught)
//     .map      MAP_* to use as met location, where the species is legally caught
//     .ivPool   optional IV_POOL_* (see below); leave out for none
//     .metGame  optional VERSION_* to set as the game of origin (e.g. VERSION_FIRE_RED for a
//               Kanto map); leave out to keep Emerald
//
//   Without an IV pool the met location is moved to .map (unless already caught there) and the met
//   level is a random level the map's encounter table has for the species, no higher than the
//   Pokemon's level.
//
//   With an IV pool the Pokemon is also given one of the pool's PID/IV pairs, even when caught on
//   .map itself. Gen 3 legality checkers (PKHeX) walk the RNG back from the PID/IVs to the nature,
//   level and encounter slot rolls, so the met level has to match the pair: only the pair/met level
//   combinations PKHeX reports as legal for the species on .map are used (sLegalIvTables). One is
//   picked at random among those with a met level no higher than the Pokemon's level that keep its
//   shininess and gender, preferring its own level as met level (a Pokemon caught above its met
//   level looks leveled up without battling to PKHeX until it gains EXP), then ones that also keep
//   its color variation. The PID sets nature and ability. If none fit (Pokemon below every legal
//   met level, or shiny), the no-pool behavior is used instead.
//
//   After adding or changing an override or a pool, run `python tech/override_ivs.py` to regenerate
//   sLegalIvTables (needs the .NET SDK; it runs PKHeX through tech/pkhex_legality). The build fails
//   if a legal table no longer matches its pool's size. `python tech/override_ivs.py --check` tells
//   whether anything is out of date.
//
// IV pools (sIvPools)
//   A pool is a list of PID/IV pairs { pid, iv1, iv2 }, with iv1 = HP | Atk << 5 | Def << 10 and
//   iv2 = Speed | SpAtk << 5 | SpDef << 10. To add a pool, add an IV_POOL_* id to the enum below,
//   its array and an IV_POOL() entry in sIvPools.
//
//   IV_POOL_WILD (sIvPoolWild) is generated: every pair the Gen 3 wild RNG (Method H-1, H-2 and H-4,
//   see PokeFinder) can give with all six IVs 30 or 31, best IV total first. It's meant for
//   Pokemon that can't be bred.
//
// Custom IVs (sSpeciesCustomIVs)
//   A wild Pokemon of a listed species gets one of its IV sets (picked at random) instead of the
//   usual IVs (TryApplyCustomWildMonIVs).
//
// Wish eggs
//   GiveOddEgg (the PCNY Wish Egg) gives one of sWishEggSpecies with that species' sWishEggMoves and
//   a PID/IV pair from the first WISH_EGG_PID_IV_EGG_COUNT entries of sWishEggPidIvs. When a hatched
//   wish egg Pokemon's nature, ability or gender is changed, it's given a pair from the whole list
//   with the new nature, ability and gender, so it stays legal.

enum
{
    IV_POOL_NONE,
    IV_POOL_WILD,
};

static const struct MetDataOverride sMetDataOverrides[] =
{
    {
        .species = SPECIES_DITTO,
        .region = FLYREGION_HOENN,
        .map = MAP_DESERT_UNDERPASS,
        .ivPool = IV_POOL_WILD,
    },
};

// Undiscovered egg group Pokemon that still get "hatched at" met data when caught.
static const u16 sCaughtAsHatchedSpecies[] =
{
    SPECIES_PICHU,
    SPECIES_CLEFFA,
    SPECIES_IGGLYBUFF,
    SPECIES_TOGEPI,
    SPECIES_TYROGUE,
    SPECIES_SMOOCHUM,
    SPECIES_ELEKID,
    SPECIES_MAGBY,
    SPECIES_AZURILL,
    SPECIES_WYNAUT,
    SPECIES_NIDORINA,
    SPECIES_NIDOQUEEN,
};

// Custom IVs: a wild Pokemon of one of these species gets one of its IV sets (picked at random)
// instead of the usual IVs. Format: HP, ATK, DEF, SPATK, SPDEF, SPD; up to 5 sets.
static const struct SpeciesCustomIVs sSpeciesCustomIVs[] =
{

    {
        .species = SPECIES_VULPIX,
        .ivSetCount = 1,
        .ivSets = {
            {31, 30, 31, 30, 31, 31},
        }
    },

    {
        .species = SPECIES_NINETALES,
        .ivSetCount = 1,
        .ivSets = {
            {31, 30, 31, 30, 31, 31},
        }
    },

    {
        .species = SPECIES_PARAS,
        .ivSetCount = 2,
        .ivSets = {
            {31, 31, 31, 31, 30, 30},
            {31, 30, 30, 31, 30, 31},
        }
    },

    {
        .species = SPECIES_PARASECT,
        .ivSetCount = 2,
        .ivSets = {
            {31, 31, 31, 31, 30, 30},
            {31, 30, 30, 31, 30, 31},
        }
    },

     {
        .species = SPECIES_MAGNEMITE,
        .ivSetCount = 3,
        .ivSets = {
            {31, 30, 31, 30, 31, 31},
            {31, 30, 31, 30, 31, 30},
            {31, 31, 31, 31, 30, 31},
        }
    },

    {
        .species = SPECIES_MAGNETON,
        .ivSetCount = 3,
        .ivSets = {
            {31, 30, 31, 30, 31, 31},
            {31, 30, 31, 30, 31, 30},
            {31, 31, 31, 31, 30, 31},
        }
    },

    {
        .species = SPECIES_HITMONLEE,
        .ivSetCount = 1,
        .ivSets = {
            {31, 31, 30, 31, 30, 31},
        }
    },

    {
        .species = SPECIES_HITMONCHAN,
        .ivSetCount = 1,
        .ivSets = {
            {31, 31, 30, 31, 30, 31},
        }
    },

    {
        .species = SPECIES_PINSIR,
        .ivSetCount = 2,
        .ivSets = {
            {31, 31, 31, 31, 30, 30},
            {31, 30, 30, 31, 30, 31},
        }
    },

    {
        .species = SPECIES_MAGIKARP,
        .ivSetCount = 2,
        .ivSets = {
            {30, 30, 30, 30, 30, 31},
            {31, 31, 31, 30, 30, 30},
        }
    },

    {
        .species = SPECIES_GYARADOS,
        .ivSetCount = 2,
        .ivSets = {
            {30, 30, 30, 30, 30, 31},
            {31, 31, 31, 30, 30, 30},
        }
    },

    {
        .species = SPECIES_TYROGUE,
        .ivSetCount = 1,
        .ivSets = {
            {31, 31, 30, 31, 30, 31},
        }
    },

    {
        .species = SPECIES_HITMONTOP,
        .ivSetCount = 1,
        .ivSets = {
            {31, 31, 30, 31, 30, 31},
        }
    },

    {
        .species = SPECIES_SURSKIT,
        .ivSetCount = 2,
        .ivSets = {
            {31, 30, 30, 31, 30, 31},
            {30, 30, 30, 31, 30, 31},
        }
    },

    {
        .species = SPECIES_MAWILE,
        .ivSetCount = 1,
        .ivSets = {
            {31, 31, 31, 31, 30, 31},
        }
    },
    
    {
        .species = SPECIES_ANORITH,
        .ivSetCount = 2,
        .ivSets = {
            {31, 31, 31, 31, 30, 30},
            {31, 30, 30, 31, 30, 31},
        }
    },

    {
        .species = SPECIES_ARMALDO,
        .ivSetCount = 2,
        .ivSets = {
            {31, 31, 31, 31, 30, 30},
            {31, 30, 30, 31, 30, 31},
        }
    },
   
};

static const u16 sWishEggSpecies[] =
{
    SPECIES_FARFETCHD,
    SPECIES_DROWZEE,
    SPECIES_EXEGGCUTE,
    SPECIES_LICKITUNG,
    SPECIES_CHANSEY,
    SPECIES_KANGASKHAN,
};

// PCNY Wish Egg event moves (slot 0 = special move, slot 1 = Wish)
static const u16 sWishEggMoves[][2] =
{
    [0] = {MOVE_YAWN,        MOVE_WISH}, // Farfetch'd
    [1] = {MOVE_BELLY_DRUM,  MOVE_WISH}, // Drowzee
    [2] = {MOVE_SWEET_SCENT, MOVE_WISH}, // Exeggcute
    [3] = {MOVE_HEAL_BELL,   MOVE_WISH}, // Lickitung
    [4] = {MOVE_SWEET_SCENT, MOVE_WISH}, // Chansey
    [5] = {MOVE_YAWN,        MOVE_WISH}, // Kangaskhan
};

// BEGIN GENERATED sIvPoolWild (python tech/override_ivs.py pool)
// Every Wild Method 1/2/4 PID/IV pair with all IVs 30 or 31, best IV total first.
// 772 pairs: 260 Method 1, 260 Method 2, 252 Method 4
// Comments: method, nature, HP/Atk/Def/SpA/SpD/Spe (total).
static const struct PidIvPair sIvPoolWild[] =
{
    { 0x685011A9, 0x7FFF, 0x7FFF }, // M1 Modest  31/31/31/31/31/31 (186)
    { 0x6937DA48, 0x7FFF, 0x7FFF }, // M1 Modest  31/31/31/31/31/31 (186)
    { 0x7942EF72, 0x7FFF, 0x7FFF }, // M1 Timid   31/31/31/31/31/31 (186)
    { 0xE85091A9, 0x7FFF, 0x7FFF }, // M1 Docile  31/31/31/31/31/31 (186)
    { 0xE9375A48, 0x7FFF, 0x7FFF }, // M1 Calm    31/31/31/31/31/31 (186)
    { 0xF9426F72, 0x7FFF, 0x7FFF }, // M1 Modest  31/31/31/31/31/31 (186)
    { 0x11A97B04, 0x7FFF, 0x7FFF }, // M2 Careful 31/31/31/31/31/31 (186)
    { 0x5A48D694, 0x7FFF, 0x7FFF }, // M2 Naive   31/31/31/31/31/31 (186)
    { 0x6F72469F, 0x7FFF, 0x7FFF }, // M2 Lax     31/31/31/31/31/31 (186)
    { 0x91A9FB04, 0x7FFF, 0x7FFF }, // M2 Naive   31/31/31/31/31/31 (186)
    { 0xDA485694, 0x7FFF, 0x7FFF }, // M2 Rash    31/31/31/31/31/31 (186)
    { 0xEF72C69F, 0x7FFF, 0x7FFF }, // M2 Hardy   31/31/31/31/31/31 (186)
    { 0x195A3C94, 0x7FFF, 0x7FFF }, // M4 Careful 31/31/31/31/31/31 (186)
    { 0x3886AC85, 0x7FFF, 0x7FFF }, // M4 Timid   31/31/31/31/31/31 (186)
    { 0x995ABC94, 0x7FFF, 0x7FFF }, // M4 Naive   31/31/31/31/31/31 (186)
    { 0xB8862C85, 0x7FFF, 0x7FFF }, // M4 Modest  31/31/31/31/31/31 (186)
    { 0x2B3B4352, 0x7FDF, 0x7FFF }, // M1 Gentle  31/30/31/31/31/31 (185)
    { 0x2C230BF2, 0x7FDF, 0x7FFF }, // M1 Sassy   31/30/31/31/31/31 (185)
    { 0x30742070, 0x7FFF, 0x7FDF }, // M1 Careful 31/31/31/30/31/31 (185)
    { 0x3EB4F7E3, 0x7BFF, 0x7FFF }, // M1 Docile  31/31/30/31/31/31 (185)
    { 0x3F9CC083, 0x7BFF, 0x7FFF }, // M1 Relaxed 31/31/30/31/31/31 (185)
    { 0x4167FE3A, 0x7FFF, 0x7FDF }, // M1 Bold    31/31/31/30/31/31 (185)
    { 0x47521EB4, 0x7FFF, 0x7FFE }, // M1 Careful 31/31/31/31/31/30 (185)
    { 0x5845FC7E, 0x7FFF, 0x7FFE }, // M1 Bold    31/31/31/31/31/30 (185)
    { 0x6463B3AD, 0x7FFF, 0x7BFF }, // M1 Hasty   31/31/31/31/30/31 (185)
    { 0x67684909, 0x7FFE, 0x7FFF }, // M1 Naive   30/31/31/31/31/31 (185)
    { 0xAB3BC352, 0x7FDF, 0x7FFF }, // M1 Serious 31/30/31/31/31/31 (185)
    { 0xAC238BF2, 0x7FDF, 0x7FFF }, // M1 Jolly   31/30/31/31/31/31 (185)
    { 0xB074A070, 0x7FFF, 0x7FDF }, // M1 Naive   31/31/31/30/31/31 (185)
    { 0xBEB477E3, 0x7BFF, 0x7FFF }, // M1 Hasty   31/31/30/31/31/31 (185)
    { 0xBF9C4083, 0x7BFF, 0x7FFF }, // M1 Serious 31/31/30/31/31/31 (185)
    { 0xC1677E3A, 0x7FFF, 0x7FDF }, // M1 Timid   31/31/31/30/31/31 (185)
    { 0xC7529EB4, 0x7FFF, 0x7FFE }, // M1 Naive   31/31/31/31/31/30 (185)
    { 0xD8457C7E, 0x7FFF, 0x7FFE }, // M1 Timid   31/31/31/31/31/30 (185)
    { 0xE46333AD, 0x7FFF, 0x7BFF }, // M1 Mild    31/31/31/31/30/31 (185)
    { 0xE768C909, 0x7FFE, 0x7FFF }, // M1 Bold    30/31/31/31/31/31 (185)
    { 0x0BF2DF32, 0x7FDF, 0x7FFF }, // M2 Lonely  31/30/31/31/31/31 (185)
    { 0x1EB4BF5D, 0x7FFF, 0x7FFE }, // M2 Quirky  31/31/31/31/31/30 (185)
    { 0x20700355, 0x7FFF, 0x7FDF }, // M2 Sassy   31/31/31/30/31/31 (185)
    { 0x33AD4CC0, 0x7FFF, 0x7BFF }, // M2 Quiet   31/31/31/31/30/31 (185)
    { 0x408368A4, 0x7BFF, 0x7FFF }, // M2 Adamant 31/31/30/31/31/31 (185)
    { 0x435203A1, 0x7FDF, 0x7FFF }, // M2 Adamant 31/30/31/31/31/31 (185)
    { 0x49099F73, 0x7FFE, 0x7FFF }, // M2 Hasty   30/31/31/31/31/31 (185)
    { 0x77E38D13, 0x7BFF, 0x7FFF }, // M2 Mild    31/31/30/31/31/31 (185)
    { 0x7C7E8AF9, 0x7FFF, 0x7FFE }, // M2 Sassy   31/31/31/31/31/30 (185)
    { 0x7E3ACEF0, 0x7FFF, 0x7FDF }, // M2 Bold    31/31/31/30/31/31 (185)
    { 0x8BF25F32, 0x7FDF, 0x7FFF }, // M2 Docile  31/30/31/31/31/31 (185)
    { 0x9EB43F5D, 0x7FFF, 0x7FFE }, // M2 Naughty 31/31/31/31/31/30 (185)
    { 0xA0708355, 0x7FFF, 0x7FDF }, // M2 Jolly   31/31/31/30/31/31 (185)
    { 0xB3ADCCC0, 0x7FFF, 0x7BFF }, // M2 Impish  31/31/31/31/30/31 (185)
    { 0xC083E8A4, 0x7BFF, 0x7FFF }, // M2 Rash    31/31/30/31/31/31 (185)
    { 0xC35283A1, 0x7FDF, 0x7FFF }, // M2 Rash    31/30/31/31/31/31 (185)
    { 0xC9091F73, 0x7FFE, 0x7FFF }, // M2 Mild    30/31/31/31/31/31 (185)
    { 0xF7E30D13, 0x7BFF, 0x7FFF }, // M2 Gentle  31/31/30/31/31/31 (185)
    { 0xFC7E0AF9, 0x7FFF, 0x7FFE }, // M2 Brave   31/31/31/31/31/30 (185)
    { 0xFE3A4EF0, 0x7FFF, 0x7FDF }, // M2 Timid   31/31/31/30/31/31 (185)
    { 0x174EC5BD, 0x7FFE, 0x7FFF }, // M4 Sassy   30/31/31/31/31/31 (185)
    { 0x225DA645, 0x7FDF, 0x7FFF }, // M4 Relaxed 31/30/31/31/31/31 (185)
    { 0x26F30D49, 0x7FFF, 0x7FDF }, // M4 Relaxed 31/31/31/30/31/31 (185)
    { 0x2DC2BB17, 0x7BFF, 0x7FFF }, // M4 Quirky  31/31/30/31/31/31 (185)
    { 0x3742631E, 0x7FFF, 0x7FFE }, // M4 Bold    31/31/31/31/31/30 (185)
    { 0x41891635, 0x7FDF, 0x7FFF }, // M4 Relaxed 31/30/31/31/31/31 (185)
    { 0x4E31746E, 0x7BFF, 0x7FFF }, // M4 Bashful 31/31/30/31/31/31 (185)
    { 0x57B21C75, 0x7FFF, 0x7FFE }, // M4 Timid   31/31/31/31/31/30 (185)
    { 0x586C83B4, 0x7FFF, 0x7BFF }, // M4 Serious 31/31/31/31/30/31 (185)
    { 0x58F565DC, 0x7FFE, 0x7FFF }, // M4 Naughty 30/31/31/31/31/31 (185)
    { 0x7798F3A4, 0x7FFF, 0x7BFF }, // M4 Careful 31/31/31/31/30/31 (185)
    { 0x7822D5CD, 0x7FFE, 0x7FFF }, // M4 Brave   30/31/31/31/31/31 (185)
    { 0x974E45BD, 0x7FFE, 0x7FFF }, // M4 Brave   30/31/31/31/31/31 (185)
    { 0xA25D2645, 0x7FDF, 0x7FFF }, // M4 Serious 31/30/31/31/31/31 (185)
    { 0xA6F38D49, 0x7FFF, 0x7FDF }, // M4 Careful 31/31/31/30/31/31 (185)
    { 0xADC23B17, 0x7BFF, 0x7FFF }, // M4 Naughty 31/31/30/31/31/31 (185)
    { 0xB742E31E, 0x7FFF, 0x7FFE }, // M4 Gentle  31/31/31/31/31/30 (185)
    { 0xC1899635, 0x7FDF, 0x7FFF }, // M4 Careful 31/30/31/31/31/31 (185)
    { 0xCE31F46E, 0x7BFF, 0x7FFF }, // M4 Lax     31/31/30/31/31/31 (185)
    { 0xD7B29C75, 0x7FFF, 0x7FFE }, // M4 Lonely  31/31/31/31/31/30 (185)
    { 0xD86C03B4, 0x7FFF, 0x7BFF }, // M4 Quiet   31/31/31/31/30/31 (185)
    { 0xD8F5E5DC, 0x7FFE, 0x7FFF }, // M4 Calm    30/31/31/31/31/31 (185)
    { 0xF79873A4, 0x7FFF, 0x7BFF }, // M4 Adamant 31/31/31/31/30/31 (185)
    { 0xF82255CD, 0x7FFE, 0x7FFF }, // M4 Relaxed 30/31/31/31/31/31 (185)
    { 0x01A0298D, 0x7BDF, 0x7FFF }, // M1 Jolly   31/30/30/31/31/31 (184)
    { 0x0287F22D, 0x7BDF, 0x7FFF }, // M1 Naive   31/30/30/31/31/31 (184)
    { 0x06D906AB, 0x7BFF, 0x7FDF }, // M1 Modest  31/31/30/30/31/31 (184)
    { 0x07C1CF4A, 0x7BFF, 0x7FDF }, // M1 Lonely  31/31/30/30/31/31 (184)
    { 0x0A3D505E, 0x7FDF, 0x7FFE }, // M1 Bold    31/30/31/31/31/30 (184)
    { 0x0B2518FE, 0x7FDF, 0x7FFE }, // M1 Docile  31/30/31/31/31/30 (184)
    { 0x0F772D7C, 0x7FFF, 0x7FDE }, // M1 Bashful 31/31/31/30/31/30 (184)
    { 0x165B078E, 0x7FDF, 0x7BFF }, // M1 Serious 31/30/31/31/30/31 (184)
    { 0x1A486589, 0x7FDE, 0x7FFF }, // M1 Lonely  30/30/31/31/31/31 (184)
    { 0x1B302E28, 0x7FDF, 0x7FFE }, // M1 Lonely  31/30/31/31/31/30 (184)
    { 0x1B95E4AB, 0x7FFF, 0x7BDF }, // M1 Timid   31/31/31/30/30/31 (184)
    { 0x1DB704EF, 0x7BFF, 0x7FFE }, // M1 Modest  31/31/30/31/31/30 (184)
    { 0x1E9A7A07, 0x7FFE, 0x7FDF }, // M1 Jolly   30/31/31/30/31/31 (184)
    { 0x1E9ECD8F, 0x7BFF, 0x7FFE }, // M1 Mild    31/31/30/31/31/30 (184)
    { 0x1F8142A6, 0x7FFE, 0x7FDF }, // M1 Brave   30/31/31/30/31/31 (184)
    { 0x20690B46, 0x7FFF, 0x7FDE }, // M1 Adamant 31/31/31/30/31/30 (184)
    { 0x274EE557, 0x7FDF, 0x7BFF }, // M1 Bashful 31/30/31/31/30/31 (184)
    { 0x29D5BC1E, 0x7BFF, 0x7BFF }, // M1 Relaxed 31/31/30/31/30/31 (184)
    { 0x2A537AB3, 0x7FDE, 0x7FFF }, // M1 Gentle  30/30/31/31/31/31 (184)
    { 0x2C87C275, 0x7FFF, 0x7BDF }, // M1 Calm    31/31/31/30/30/31 (184)
    { 0x2DC21A19, 0x7BFE, 0x7FFF }, // M1 Timid   30/31/30/31/31/31 (184)
    { 0x2EA9E2B9, 0x7BFF, 0x7FFE }, // M1 Hasty   31/31/30/31/31/30 (184)
    { 0x2F8C57D1, 0x7FFE, 0x7FDF }, // M1 Careful 30/31/31/30/31/31 (184)
    { 0x3AC899E8, 0x7BFF, 0x7BFF }, // M1 Adamant 31/31/30/31/30/31 (184)
    { 0x3DCD2F44, 0x7BFE, 0x7FFF }, // M1 Docile  30/31/30/31/31/31 (184)
    { 0x4365C0B9, 0x7FFF, 0x7BFE }, // M1 Calm    31/31/31/31/30/30 (184)
    { 0x466A5615, 0x7FFE, 0x7FFE }, // M1 Careful 30/31/31/31/31/30 (184)
    { 0x52880D44, 0x7FFE, 0x7BFF }, // M1 Naughty 30/31/31/31/30/31 (184)
    { 0x5370D5E4, 0x7FFF, 0x7BFE }, // M1 Mild    31/31/31/31/30/30 (184)
    { 0x54589E83, 0x7FFF, 0x7BFE }, // M1 Mild    31/31/31/31/30/30 (184)
    { 0x575D33DF, 0x7FFE, 0x7FFE }, // M1 Rash    30/31/31/31/31/30 (184)
    { 0x6293226F, 0x7FFE, 0x7BFF }, // M1 Hardy   30/31/31/31/30/31 (184)
    { 0x6354BCF0, 0x7FDF, 0x7FDF }, // M1 Lonely  31/30/31/30/31/31 (184)
    { 0x637BEB0E, 0x7FFE, 0x7BFF }, // M1 Hasty   30/31/31/31/30/31 (184)
    { 0x71959463, 0x7BDF, 0x7FFF }, // M1 Lax     31/30/30/31/31/31 (184)
    { 0x735FD21A, 0x7FDF, 0x7FDF }, // M1 Gentle  31/30/31/30/31/31 (184)
    { 0x81A0A98D, 0x7BDF, 0x7FFF }, // M1 Naughty 31/30/30/31/31/31 (184)
    { 0x8287722D, 0x7BDF, 0x7FFF }, // M1 Rash    31/30/30/31/31/31 (184)
    { 0x86D986AB, 0x7BFF, 0x7FDF }, // M1 Docile  31/31/30/30/31/31 (184)
    { 0x87C14F4A, 0x7BFF, 0x7FDF }, // M1 Docile  31/31/30/30/31/31 (184)
    { 0x8A3DD05E, 0x7FDF, 0x7FFE }, // M1 Gentle  31/30/31/31/31/30 (184)
    { 0x8B2598FE, 0x7FDF, 0x7FFE }, // M1 Sassy   31/30/31/31/31/30 (184)
    { 0x8F77AD7C, 0x7FFF, 0x7FDE }, // M1 Lax     31/31/31/30/31/30 (184)
    { 0x965B878E, 0x7FDF, 0x7BFF }, // M1 Adamant 31/30/31/31/30/31 (184)
    { 0x9A48E589, 0x7FDE, 0x7FFF }, // M1 Quiet   30/30/31/31/31/31 (184)
    { 0x9B30AE28, 0x7FDF, 0x7FFE }, // M1 Quiet   31/30/31/31/31/30 (184)
    { 0x9B9564AB, 0x7FFF, 0x7BDF }, // M1 Modest  31/31/31/30/30/31 (184)
    { 0x9DB784EF, 0x7BFF, 0x7FFE }, // M1 Docile  31/31/30/31/31/30 (184)
    { 0x9E9AFA07, 0x7FFE, 0x7FDF }, // M1 Naughty 30/31/31/30/31/31 (184)
    { 0x9E9E4D8F, 0x7BFF, 0x7FFE }, // M1 Gentle  31/31/30/31/31/30 (184)
    { 0x9F81C2A6, 0x7FFE, 0x7FDF }, // M1 Bashful 30/31/31/30/31/31 (184)
    { 0xA0698B46, 0x7FFF, 0x7FDE }, // M1 Rash    31/31/31/30/31/30 (184)
    { 0xA74E6557, 0x7FDF, 0x7BFF }, // M1 Careful 31/30/31/31/30/31 (184)
    { 0xA9D53C1E, 0x7BFF, 0x7BFF }, // M1 Serious 31/31/30/31/30/31 (184)
    { 0xAA53FAB3, 0x7FDE, 0x7FFF }, // M1 Serious 30/30/31/31/31/31 (184)
    { 0xAC874275, 0x7FFF, 0x7BDF }, // M1 Hardy   31/31/31/30/30/31 (184)
    { 0xADC29A19, 0x7BFE, 0x7FFF }, // M1 Lonely  30/31/30/31/31/31 (184)
    { 0xAEA962B9, 0x7BFF, 0x7FFE }, // M1 Mild    31/31/30/31/31/30 (184)
    { 0xAF8CD7D1, 0x7FFE, 0x7FDF }, // M1 Naive   30/31/31/30/31/31 (184)
    { 0xBAC819E8, 0x7BFF, 0x7BFF }, // M1 Impish  31/31/30/31/30/31 (184)
    { 0xBDCDAF44, 0x7BFE, 0x7FFF }, // M1 Sassy   30/31/30/31/31/31 (184)
    { 0xC36540B9, 0x7FFF, 0x7BFE }, // M1 Hardy   31/31/31/31/30/30 (184)
    { 0xC66AD615, 0x7FFE, 0x7FFE }, // M1 Naive   30/31/31/31/31/30 (184)
    { 0xD2888D44, 0x7FFE, 0x7BFF }, // M1 Calm    30/31/31/31/30/31 (184)
    { 0xD37055E4, 0x7FFF, 0x7BFE }, // M1 Gentle  31/31/31/31/30/30 (184)
    { 0xD4581E83, 0x7FFF, 0x7BFE }, // M1 Gentle  31/31/31/31/30/30 (184)
    { 0xD75DB3DF, 0x7FFE, 0x7FFE }, // M1 Timid   30/31/31/31/31/30 (184)
    { 0xE293A26F, 0x7FFE, 0x7BFF }, // M1 Mild    30/31/31/31/30/31 (184)
    { 0xE3543CF0, 0x7FDF, 0x7FDF }, // M1 Docile  31/30/31/30/31/31 (184)
    { 0xE37B6B0E, 0x7FFE, 0x7BFF }, // M1 Mild    30/31/31/31/30/31 (184)
    { 0xF1951463, 0x7BDF, 0x7FFF }, // M1 Naive   31/30/30/31/31/31 (184)
    { 0xF35F521A, 0x7FDF, 0x7FDF }, // M1 Lonely  31/30/31/30/31/31 (184)
    { 0x04EF516D, 0x7BFF, 0x7FFE }, // M2 Jolly   31/31/30/31/31/30 (184)
    { 0x06AB9565, 0x7BFF, 0x7FDF }, // M2 Sassy   31/31/30/30/31/31 (184)
    { 0x078E09C3, 0x7FDF, 0x7BFF }, // M2 Careful 31/30/31/31/30/31 (184)
    { 0x0B46934A, 0x7FFF, 0x7FDE }, // M2 Brave   31/31/31/30/31/30 (184)
    { 0x0D44A595, 0x7FFE, 0x7BFF }, // M2 Calm    30/31/31/31/30/31 (184)
    { 0x146325A6, 0x7BDF, 0x7FFF }, // M2 Sassy   31/30/30/31/31/31 (184)
    { 0x18FE238B, 0x7FDF, 0x7FFE }, // M2 Brave   31/30/31/31/31/30 (184)
    { 0x19E8DED0, 0x7BFF, 0x7BFF }, // M2 Quiet   31/31/30/31/30/31 (184)
    { 0x1A19C178, 0x7BFE, 0x7FFF }, // M2 Rash    30/31/30/31/31/31 (184)
    { 0x1E83DCB6, 0x7FFF, 0x7BFE }, // M2 Careful 31/31/31/31/30/30 (184)
    { 0x226F159F, 0x7FFE, 0x7BFF }, // M2 Hardy   30/31/31/31/30/31 (184)
    { 0x298D95B1, 0x7BDF, 0x7FFF }, // M2 Adamant 31/30/30/31/31/31 (184)
    { 0x2D7C47AF, 0x7FFF, 0x7FDE }, // M2 Timid   31/31/31/30/31/30 (184)
    { 0x2E289396, 0x7FDF, 0x7FFE }, // M2 Impish  31/30/31/31/31/30 (184)
    { 0x2F443183, 0x7BFE, 0x7FFF }, // M2 Hardy   30/31/30/31/31/31 (184)
    { 0x33DF2F68, 0x7FFE, 0x7FFE }, // M2 Bold    30/31/31/31/31/30 (184)
    { 0x3C1E9335, 0x7BFF, 0x7BFF }, // M2 Hardy   31/31/30/31/30/31 (184)
    { 0x3CF01BE8, 0x7FDF, 0x7FDF }, // M2 Rash    31/30/31/30/31/31 (184)
    { 0x40B9911A, 0x7FFF, 0x7BFE }, // M2 Bold    31/31/31/31/30/30 (184)
    { 0x4275D512, 0x7FFF, 0x7BDF }, // M2 Naive   31/31/31/30/30/31 (184)
    { 0x42A6B7BA, 0x7FFE, 0x7FDF }, // M2 Mild    30/31/31/30/31/31 (184)
    { 0x4D8FACFE, 0x7BFF, 0x7FFE }, // M2 Mild    31/31/30/31/31/30 (184)
    { 0x4F4AF0F5, 0x7BFF, 0x7FDF }, // M2 Jolly   31/31/30/30/31/31 (184)
    { 0x505E47FB, 0x7FDF, 0x7FFE }, // M2 Mild    31/30/31/31/31/30 (184)
    { 0x521A8BF3, 0x7FDF, 0x7FDF }, // M2 Hardy   31/30/31/30/31/31 (184)
    { 0x55E40125, 0x7FFF, 0x7BFE }, // M2 Hasty   31/31/31/31/30/30 (184)
    { 0x5615E3CD, 0x7FFE, 0x7FFE }, // M2 Quirky  30/31/31/31/31/30 (184)
    { 0x57D127C5, 0x7FFE, 0x7FDF }, // M2 Sassy   30/31/31/30/31/31 (184)
    { 0x62B91D08, 0x7BFF, 0x7FFE }, // M2 Timid   31/31/30/31/31/30 (184)
    { 0x64AB8976, 0x7FFF, 0x7BDF }, // M2 Gentle  31/31/31/30/30/31 (184)
    { 0x6557D55E, 0x7FDF, 0x7BFF }, // M2 Calm    31/30/31/31/30/31 (184)
    { 0x6589B806, 0x7FDE, 0x7FFF }, // M2 Impish  30/30/31/31/31/31 (184)
    { 0x6B0E7130, 0x7FFE, 0x7BFF }, // M2 Quiet   30/31/31/31/30/31 (184)
    { 0x722DF141, 0x7BDF, 0x7FFF }, // M2 Bold    31/30/30/31/31/31 (184)
    { 0x7A07DC29, 0x7FFE, 0x7FDF }, // M2 Modest  30/31/31/30/31/31 (184)
    { 0x7AB32811, 0x7FDE, 0x7FFF }, // M2 Adamant 30/30/31/31/31/31 (184)
    { 0x84EFD16D, 0x7BFF, 0x7FFE }, // M2 Naughty 31/31/30/31/31/30 (184)
    { 0x86AB1565, 0x7BFF, 0x7FDF }, // M2 Brave   31/31/30/30/31/31 (184)
    { 0x878E89C3, 0x7FDF, 0x7BFF }, // M2 Naive   31/30/31/31/30/31 (184)
    { 0x8B46134A, 0x7FFF, 0x7FDE }, // M2 Relaxed 31/31/31/30/31/30 (184)
    { 0x8D442595, 0x7FFE, 0x7BFF }, // M2 Hardy   30/31/31/31/30/31 (184)
    { 0x9463A5A6, 0x7BDF, 0x7FFF }, // M2 Jolly   31/30/30/31/31/31 (184)
    { 0x98FEA38B, 0x7FDF, 0x7FFE }, // M2 Bashful 31/30/31/31/31/30 (184)
    { 0x99E85ED0, 0x7BFF, 0x7BFF }, // M2 Sassy   31/31/30/31/30/31 (184)
    { 0x9A194178, 0x7BFE, 0x7FFF }, // M2 Quirky  30/31/30/31/31/31 (184)
    { 0x9E835CB6, 0x7FFF, 0x7BFE }, // M2 Adamant 31/31/31/31/30/30 (184)
    { 0xA26F959F, 0x7FFE, 0x7BFF }, // M2 Mild    30/31/31/31/30/31 (184)
    { 0xA98D15B1, 0x7BDF, 0x7FFF }, // M2 Impish  31/30/30/31/31/31 (184)
    { 0xAD7CC7AF, 0x7FFF, 0x7FDE }, // M2 Lonely  31/31/31/30/31/30 (184)
    { 0xAE281396, 0x7FDF, 0x7FFE }, // M2 Jolly   31/30/31/31/31/30 (184)
    { 0xAF44B183, 0x7BFE, 0x7FFF }, // M2 Mild    30/31/30/31/31/31 (184)
    { 0xB3DFAF68, 0x7FFE, 0x7FFE }, // M2 Gentle  30/31/31/31/31/30 (184)
    { 0xBC1E1335, 0x7BFF, 0x7BFF }, // M2 Bold    31/31/30/31/30/31 (184)
    { 0xBCF09BE8, 0x7FDF, 0x7FDF }, // M2 Timid   31/30/31/30/31/31 (184)
    { 0xC0B9111A, 0x7FFF, 0x7BFE }, // M2 Timid   31/31/31/31/30/30 (184)
    { 0xC2755512, 0x7FFF, 0x7BDF }, // M2 Rash    31/31/31/30/30/31 (184)
    { 0xC2A637BA, 0x7FFE, 0x7FDF }, // M2 Gentle  30/31/31/30/31/31 (184)
    { 0xCD8F2CFE, 0x7BFF, 0x7FFE }, // M2 Gentle  31/31/30/31/31/30 (184)
    { 0xCF4A70F5, 0x7BFF, 0x7FDF }, // M2 Bashful 31/31/30/30/31/31 (184)
    { 0xD05EC7FB, 0x7FDF, 0x7FFE }, // M2 Relaxed 31/30/31/31/31/30 (184)
    { 0xD21A0BF3, 0x7FDF, 0x7FDF }, // M2 Bold    31/30/31/30/31/31 (184)
    { 0xD5E48125, 0x7FFF, 0x7BFE }, // M2 Brave   31/31/31/31/30/30 (184)
    { 0xD61563CD, 0x7FFE, 0x7FFE }, // M2 Naughty 30/31/31/31/31/30 (184)
    { 0xD7D1A7C5, 0x7FFE, 0x7FDF }, // M2 Jolly   30/31/31/30/31/31 (184)
    { 0xE2B99D08, 0x7BFF, 0x7FFE }, // M2 Lonely  31/31/30/31/31/30 (184)
    { 0xE4AB0976, 0x7FFF, 0x7BDF }, // M2 Lonely  31/31/31/30/30/31 (184)
    { 0xE557555E, 0x7FDF, 0x7BFF }, // M2 Hardy   31/30/31/31/30/31 (184)
    { 0xE5893806, 0x7FDE, 0x7FFF }, // M2 Jolly   30/30/31/31/31/31 (184)
    { 0xEB0EF130, 0x7FFE, 0x7BFF }, // M2 Impish  30/31/31/31/30/31 (184)
    { 0xF22D7141, 0x7BDF, 0x7FFF }, // M2 Timid   31/30/30/31/31/31 (184)
    { 0xFA075C29, 0x7FFE, 0x7FDF }, // M2 Calm    30/31/31/30/31/31 (184)
    { 0xFAB3A811, 0x7FDE, 0x7FFF }, // M2 Rash    30/30/31/31/31/31 (184)
    { 0x009BDD55, 0x7FDF, 0x7BFF }, // M4 Mild    31/30/31/31/30/31 (184)
    { 0x0125BF7D, 0x7FDE, 0x7FFF }, // M4 Rash    30/30/31/31/31/31 (184)
    { 0x0478DD1A, 0x7FFF, 0x7FDE }, // M4 Hasty   31/31/31/30/31/30 (184)
    { 0x05324459, 0x7FFF, 0x7BDF }, // M4 Brave   31/31/31/30/30/31 (184)
    { 0x05BB2681, 0x7FFE, 0x7FDF }, // M4 Rash    30/31/31/30/31/31 (184)
    { 0x20512F6E, 0x7FDE, 0x7FFF }, // M4 Calm    30/30/31/31/31/31 (184)
    { 0x2FF676F9, 0x7FDF, 0x7FDF }, // M4 Modest  31/30/31/30/31/31 (184)
    { 0x37349CEC, 0x7FFE, 0x7BFF }, // M4 Quirky  30/31/31/31/30/31 (184)
    { 0x3C9FD532, 0x7BFF, 0x7FDF }, // M4 Serious 31/31/30/30/31/31 (184)
    { 0x46207D39, 0x7FFF, 0x7FDE }, // M4 Naughty 31/31/31/30/31/30 (184)
    { 0x46D9E477, 0x7FFF, 0x7BDF }, // M4 Rash    31/31/31/30/30/31 (184)
    { 0x4763C6A0, 0x7FFE, 0x7FDF }, // M4 Careful 30/31/31/30/31/31 (184)
    { 0x4CEE2B08, 0x7BFF, 0x7FFE }, // M4 Hardy   31/31/30/31/31/30 (184)
    { 0x4DA89246, 0x7BFF, 0x7BFF }, // M4 Lonely  31/31/30/31/30/31 (184)
    { 0x4F23E6EA, 0x7FDF, 0x7FDF }, // M4 Jolly   31/30/31/30/31/31 (184)
    { 0x57283A4D, 0x7FFF, 0x7BFE }, // M4 Relaxed 31/31/31/31/30/30 (184)
    { 0x5734DE1F, 0x7BDF, 0x7FFF }, // M4 Brave   31/30/30/31/31/31 (184)
    { 0x5BCB4523, 0x7BFF, 0x7FDF }, // M4 Jolly   31/31/30/30/31/31 (184)
    { 0x60B58626, 0x7FDF, 0x7FFE }, // M4 Rash    31/30/31/31/31/30 (184)
    { 0x616FED64, 0x7FDF, 0x7BFF }, // M4 Calm    31/30/31/31/30/31 (184)
    { 0x654CED2A, 0x7FFF, 0x7FDE }, // M4 Mild    31/31/31/30/31/30 (184)
    { 0x66065468, 0x7FFF, 0x7BDF }, // M4 Docile  31/31/31/30/30/31 (184)
    { 0x668F3690, 0x7FFE, 0x7FDF }, // M4 Careful 30/31/31/30/31/31 (184)
    { 0x6C1A9AF8, 0x7BFF, 0x7FFE }, // M4 Hasty   31/31/30/31/31/30 (184)
    { 0x6CD40237, 0x7BFF, 0x7BFF }, // M4 Brave   31/31/30/31/30/31 (184)
    { 0x6D5DE45F, 0x7BFE, 0x7FFF }, // M4 Bold    30/31/30/31/31/31 (184)
    { 0x7654AA3E, 0x7FFF, 0x7BFE }, // M4 Rash    31/31/31/31/30/30 (184)
    { 0x76DE8C66, 0x7FFE, 0x7FFE }, // M4 Sassy   30/31/31/31/31/30 (184)
    { 0x7AF7B513, 0x7BFF, 0x7FDF }, // M4 Quirky  31/31/30/30/31/31 (184)
    { 0x7FE1F617, 0x7FDF, 0x7FFE }, // M4 Docile  31/30/31/31/31/30 (184)
    { 0x809B5D55, 0x7FDF, 0x7BFF }, // M4 Gentle  31/30/31/31/30/31 (184)
    { 0x81253F7D, 0x7FDE, 0x7FFF }, // M4 Quirky  30/30/31/31/31/31 (184)
    { 0x84785D1A, 0x7FFF, 0x7FDE }, // M4 Mild    31/31/31/30/31/30 (184)
    { 0x8532C459, 0x7FFF, 0x7BDF }, // M4 Bashful 31/31/31/30/30/31 (184)
    { 0x85BBA681, 0x7FFE, 0x7FDF }, // M4 Timid   30/31/31/30/31/31 (184)
    { 0xA051AF6E, 0x7FDE, 0x7FFF }, // M4 Hasty   30/30/31/31/31/31 (184)
    { 0xAFF6F6F9, 0x7FDF, 0x7FDF }, // M4 Docile  31/30/31/30/31/31 (184)
    { 0xB7341CEC, 0x7FFE, 0x7BFF }, // M4 Naughty 30/31/31/31/30/31 (184)
    { 0xBC9F5532, 0x7BFF, 0x7FDF }, // M4 Quiet   31/31/30/30/31/31 (184)
    { 0xC620FD39, 0x7FFF, 0x7FDE }, // M4 Calm    31/31/31/30/31/30 (184)
    { 0xC6D96477, 0x7FFF, 0x7BDF }, // M4 Quirky  31/31/31/30/30/31 (184)
    { 0xC76346A0, 0x7FFE, 0x7FDF }, // M4 Adamant 30/31/31/30/31/31 (184)
    { 0xCCEEAB08, 0x7BFF, 0x7FFE }, // M4 Mild    31/31/30/31/31/30 (184)
    { 0xCDA81246, 0x7BFF, 0x7BFF }, // M4 Docile  31/31/30/31/30/31 (184)
    { 0xCF2366EA, 0x7FDF, 0x7FDF }, // M4 Bashful 31/30/31/30/31/31 (184)
    { 0xD728BA4D, 0x7FFF, 0x7BFE }, // M4 Careful 31/31/31/31/30/30 (184)
    { 0xD7345E1F, 0x7BDF, 0x7FFF }, // M4 Relaxed 31/30/30/31/31/31 (184)
    { 0xDBCBC523, 0x7BFF, 0x7FDF }, // M4 Naughty 31/31/30/30/31/31 (184)
    { 0xE0B50626, 0x7FDF, 0x7FFE }, // M4 Quirky  31/30/31/31/31/30 (184)
    { 0xE16F6D64, 0x7FDF, 0x7BFF }, // M4 Hardy   31/30/31/31/30/31 (184)
    { 0xE54C6D2A, 0x7FFF, 0x7FDE }, // M4 Gentle  31/31/31/30/31/30 (184)
    { 0xE606D468, 0x7FFF, 0x7BDF }, // M4 Sassy   31/31/31/30/30/31 (184)
    { 0xE68FB690, 0x7FFE, 0x7FDF }, // M4 Naive   30/31/31/30/31/31 (184)
    { 0xEC1A1AF8, 0x7BFF, 0x7FFE }, // M4 Mild    31/31/30/31/31/30 (184)
    { 0xECD48237, 0x7BFF, 0x7BFF }, // M4 Bashful 31/31/30/31/30/31 (184)
    { 0xED5D645F, 0x7BFE, 0x7FFF }, // M4 Timid   30/31/30/31/31/31 (184)
    { 0xF6542A3E, 0x7FFF, 0x7BFE }, // M4 Quirky  31/31/31/31/30/30 (184)
    { 0xF6DE0C66, 0x7FFE, 0x7FFE }, // M4 Brave   30/31/31/31/31/30 (184)
    { 0xFAF73513, 0x7BFF, 0x7FDF }, // M4 Naughty 31/31/30/30/31/31 (184)
    { 0xFFE17617, 0x7FDF, 0x7FFE }, // M4 Hasty   31/30/31/31/31/30 (184)
    { 0x02ECA8B0, 0x7BFF, 0x7BDF }, // M1 Serious 31/31/30/30/30/31 (183)
    { 0x05F13E0B, 0x7BFE, 0x7FDF }, // M1 Naive   30/31/30/30/31/31 (183)
    { 0x0651F263, 0x7FDF, 0x7BFE }, // M1 Jolly   31/30/31/31/30/30 (183)
    { 0x095687BF, 0x7FDE, 0x7FFE }, // M1 Mild    30/30/31/31/31/30 (183)
    { 0x0B8ACF81, 0x7FFF, 0x7BDE }, // M1 Modest  31/31/31/30/30/30 (183)
    { 0x0E8F64DD, 0x7FFE, 0x7FDE }, // M1 Bashful 30/31/31/30/31/30 (183)
    { 0x15743EEE, 0x7FDE, 0x7BFF }, // M1 Sassy   30/30/31/31/30/31 (183)
    { 0x1743D02D, 0x7FDF, 0x7BFE }, // M1 Careful 31/30/31/31/30/30 (183)
    { 0x19CAA6F4, 0x7BFF, 0x7BFE }, // M1 Serious 31/31/30/31/30/30 (183)
    { 0x1AAD1C0C, 0x7FFE, 0x7BDF }, // M1 Quirky  30/31/31/30/30/31 (183)
    { 0x1CCF3C50, 0x7BFE, 0x7FFE }, // M1 Modest  30/31/30/31/31/30 (183)
    { 0x26661CB8, 0x7FDE, 0x7BFF }, // M1 Relaxed 30/30/31/31/30/31 (183)
    { 0x28EDF37F, 0x7BFE, 0x7BFF }, // M1 Relaxed 30/31/30/31/30/31 (183)
    { 0x2ABD84BE, 0x7BFF, 0x7BFE }, // M1 Impish  31/31/30/31/30/30 (183)
    { 0x2BA0F9D6, 0x7FFE, 0x7BDF }, // M1 Docile  30/31/31/30/30/31 (183)
    { 0x39B9A32A, 0x7BDF, 0x7FDF }, // M1 Quiet   31/30/30/30/31/31 (183)
    { 0x39E0D149, 0x7BFE, 0x7BFF }, // M1 Adamant 30/31/30/31/30/31 (183)
    { 0x41962F7A, 0x7FFE, 0x7BFE }, // M1 Rash    30/31/31/31/30/30 (183)
    { 0x427DF81A, 0x7FFE, 0x7BFE }, // M1 Calm    30/31/31/31/30/30 (183)
    { 0x49C4B855, 0x7BDF, 0x7FDF }, // M1 Jolly   31/30/30/30/31/31 (183)
    { 0x4AAC80F4, 0x7BDF, 0x7FDF }, // M1 Jolly   31/30/30/30/31/31 (183)
    { 0x5262DF26, 0x7FDF, 0x7FDE }, // M1 Mild    31/30/31/30/31/30 (183)
    { 0x5E809655, 0x7FDF, 0x7BDF }, // M1 Sassy   31/30/31/30/30/31 (183)
    { 0x61852BB1, 0x7FDE, 0x7FDF }, // M1 Hardy   30/30/31/30/31/31 (183)
    { 0x618A7F38, 0x7BDF, 0x7FFE }, // M1 Jolly   31/30/30/31/31/30 (183)
    { 0x626DF450, 0x7FDE, 0x7FDF }, // M1 Hasty   30/30/31/30/31/31 (183)
    { 0x65DB93B7, 0x7BFF, 0x7FDE }, // M1 Modest  31/31/30/30/31/30 (183)
    { 0x66C35C56, 0x7BFF, 0x7FDE }, // M1 Modest  31/31/30/30/31/30 (183)
    { 0x6CC06DC8, 0x7BDF, 0x7BFF }, // M1 Rash    31/30/30/31/30/31 (183)
    { 0x6DA83668, 0x7BDF, 0x7BFF }, // M1 Calm    31/30/30/31/30/31 (183)
    { 0x6F73741F, 0x7FDF, 0x7BDF }, // M1 Bashful 31/30/31/30/30/31 (183)
    { 0x70ADCBC3, 0x7BDE, 0x7FFF }, // M1 Impish  30/30/30/31/31/31 (183)
    { 0x71F94AE6, 0x7BFF, 0x7BDF }, // M1 Gentle  31/31/30/30/30/31 (183)
    { 0x75E6A8E1, 0x7BFE, 0x7FDF }, // M1 Timid   30/31/30/30/31/31 (183)
    { 0x76CE7180, 0x7BFF, 0x7FDE }, // M1 Timid   31/31/30/30/31/30 (183)
    { 0x7D9C0713, 0x7FFE, 0x7FDE }, // M1 Brave   30/31/31/30/31/30 (183)
    { 0x7DB34B92, 0x7BDF, 0x7BFF }, // M1 Modest  31/30/30/31/30/31 (183)
    { 0x82EC28B0, 0x7BFF, 0x7BDF }, // M1 Quiet   31/31/30/30/30/31 (183)
    { 0x85F1BE0B, 0x7BFE, 0x7FDF }, // M1 Bold    30/31/30/30/31/31 (183)
    { 0x86517263, 0x7FDF, 0x7BFE }, // M1 Bashful 31/30/31/31/30/30 (183)
    { 0x895607BF, 0x7FDE, 0x7FFE }, // M1 Gentle  30/30/31/31/31/30 (183)
    { 0x8B8A4F81, 0x7FFF, 0x7BDE }, // M1 Calm    31/31/31/30/30/30 (183)
    { 0x8E8FE4DD, 0x7FFE, 0x7FDE }, // M1 Lax     30/31/31/30/31/30 (183)
    { 0x9574BEEE, 0x7FDE, 0x7BFF }, // M1 Jolly   30/30/31/31/30/31 (183)
    { 0x9743502D, 0x7FDF, 0x7BFE }, // M1 Adamant 31/30/31/31/30/30 (183)
    { 0x99CA26F4, 0x7BFF, 0x7BFE }, // M1 Quiet   31/31/30/31/30/30 (183)
    { 0x9AAD9C0C, 0x7FFE, 0x7BDF }, // M1 Modest  30/31/31/30/30/31 (183)
    { 0x9CCFBC50, 0x7BFE, 0x7FFE }, // M1 Docile  30/31/30/31/31/30 (183)
    { 0xA6669CB8, 0x7FDE, 0x7BFF }, // M1 Careful 30/30/31/31/30/31 (183)
    { 0xA8ED737F, 0x7BFE, 0x7BFF }, // M1 Serious 30/31/30/31/30/31 (183)
    { 0xAABD04BE, 0x7BFF, 0x7BFE }, // M1 Jolly   31/31/30/31/30/30 (183)
    { 0xABA079D6, 0x7FFE, 0x7BDF }, // M1 Hasty   30/31/31/30/30/31 (183)
    { 0xB9B9232A, 0x7BDF, 0x7FDF }, // M1 Sassy   31/30/30/30/31/31 (183)
    { 0xB9E05149, 0x7BFE, 0x7BFF }, // M1 Impish  30/31/30/31/30/31 (183)
    { 0xC196AF7A, 0x7FFE, 0x7BFE }, // M1 Timid   30/31/31/31/30/30 (183)
    { 0xC27D781A, 0x7FFE, 0x7BFE }, // M1 Hardy   30/31/31/31/30/30 (183)
    { 0xC9C43855, 0x7BDF, 0x7FDF }, // M1 Bashful 31/30/30/30/31/31 (183)
    { 0xCAAC00F4, 0x7BDF, 0x7FDF }, // M1 Bashful 31/30/30/30/31/31 (183)
    { 0xD2625F26, 0x7FDF, 0x7FDE }, // M1 Gentle  31/30/31/30/31/30 (183)
    { 0xDE801655, 0x7FDF, 0x7BDF }, // M1 Brave   31/30/31/30/30/31 (183)
    { 0xE185ABB1, 0x7FDE, 0x7FDF }, // M1 Mild    30/30/31/30/31/31 (183)
    { 0xE18AFF38, 0x7BDF, 0x7FFE }, // M1 Naughty 31/30/30/31/31/30 (183)
    { 0xE26D7450, 0x7FDE, 0x7FDF }, // M1 Mild    30/30/31/30/31/31 (183)
    { 0xE5DB13B7, 0x7BFF, 0x7FDE }, // M1 Calm    31/31/30/30/31/30 (183)
    { 0xE6C3DC56, 0x7BFF, 0x7FDE }, // M1 Docile  31/31/30/30/31/30 (183)
    { 0xECC0EDC8, 0x7BDF, 0x7BFF }, // M1 Timid   31/30/30/31/30/31 (183)
    { 0xEDA8B668, 0x7BDF, 0x7BFF }, // M1 Hasty   31/30/30/31/30/31 (183)
    { 0xEF73F41F, 0x7FDF, 0x7BDF }, // M1 Lax     31/30/31/30/30/31 (183)
    { 0xF0AD4BC3, 0x7BDE, 0x7FFF }, // M1 Jolly   30/30/30/31/31/31 (183)
    { 0xF1F9CAE6, 0x7BFF, 0x7BDF }, // M1 Serious 31/31/30/30/30/31 (183)
    { 0xF5E628E1, 0x7BFE, 0x7FDF }, // M1 Modest  30/31/30/30/31/31 (183)
    { 0xF6CEF180, 0x7BFF, 0x7FDE }, // M1 Lonely  31/31/30/30/31/30 (183)
    { 0xFD9C8713, 0x7FFE, 0x7FDE }, // M1 Bashful 30/31/31/30/31/30 (183)
    { 0xFDB3CB92, 0x7BDF, 0x7BFF }, // M1 Docile  31/30/30/31/30/31 (183)
    { 0x00F4F993, 0x7BDF, 0x7FDF }, // M2 Hardy   31/30/30/30/31/31 (183)
    { 0x04BE6EC5, 0x7BFF, 0x7BFE }, // M2 Hasty   31/31/30/31/30/30 (183)
    { 0x0713A083, 0x7FFE, 0x7FDE }, // M2 Serious 30/31/31/30/31/30 (183)
    { 0x07BFEC6A, 0x7FDE, 0x7FFE }, // M2 Timid   30/30/31/31/31/30 (183)
    { 0x13B7D9BE, 0x7BFF, 0x7FDE }, // M2 Lax     31/31/30/30/31/30 (183)
    { 0x16559214, 0x7FDF, 0x7BDF }, // M2 Impish  31/30/31/30/30/31 (183)
    { 0x1C0C2DE6, 0x7FFE, 0x7BDF }, // M2 Bold    30/31/31/30/30/31 (183)
    { 0x1CB879CE, 0x7FDE, 0x7BFF }, // M2 Naughty 30/30/31/31/30/31 (183)
    { 0x232AADF7, 0x7BDF, 0x7FDF }, // M2 Relaxed 31/30/30/30/31/31 (183)
    { 0x26F4232A, 0x7BFF, 0x7BFE }, // M2 Rash    31/31/30/31/30/30 (183)
    { 0x28B06721, 0x7BFF, 0x7BDF }, // M2 Brave   31/31/30/30/30/31 (183)
    { 0x28E149C9, 0x7BFE, 0x7FDF }, // M2 Naughty 30/31/30/30/31/31 (183)
    { 0x2BB1E4C7, 0x7FDE, 0x7FDF }, // M2 Brave   30/30/31/30/31/31 (183)
    { 0x2F7A59F9, 0x7FFE, 0x7BFE }, // M2 Brave   30/31/31/31/30/30 (183)
    { 0x3668F763, 0x7BDF, 0x7BFF }, // M2 Naive   31/30/30/31/30/31 (183)
    { 0x38551E02, 0x7BDF, 0x7FDF }, // M2 Jolly   31/30/30/30/31/31 (183)
    { 0x3C5075DD, 0x7BFE, 0x7FFE }, // M2 Jolly   30/31/30/31/31/30 (183)
    { 0x3E0BB9D4, 0x7BFE, 0x7FDF }, // M2 Timid   30/31/30/30/31/31 (183)
    { 0x3EEE2E32, 0x7FDE, 0x7BFF }, // M2 Hasty   30/30/31/31/30/31 (183)
    { 0x4AE61B86, 0x7BFF, 0x7BDF }, // M2 Timid   31/31/30/30/30/31 (183)
    { 0x4B92676E, 0x7BDF, 0x7BFF }, // M2 Lax     31/30/30/31/30/31 (183)
    { 0x4BC34A16, 0x7BDE, 0x7FFF }, // M2 Hasty   30/30/30/31/31/31 (183)
    { 0x4F81196B, 0x7FFF, 0x7BDE }, // M2 Modest  31/31/31/30/30/30 (183)
    { 0x502D6553, 0x7FDF, 0x7BFE }, // M2 Naive   31/30/31/31/30/30 (183)
    { 0x51490340, 0x7BFE, 0x7BFF }, // M2 Docile  30/31/30/31/30/31 (183)
    { 0x5C56354F, 0x7BFF, 0x7FDE }, // M2 Modest  31/31/30/30/31/30 (183)
    { 0x5F26D04C, 0x7FDF, 0x7FDE }, // M2 Serious 31/30/31/30/31/30 (183)
    { 0x64DD6C1E, 0x7FFE, 0x7FDE }, // M2 Lax     30/31/31/30/31/30 (183)
    { 0x6DC81BD2, 0x7BDF, 0x7BFF }, // M2 Mild    31/30/30/31/30/31 (183)
    { 0x7180A55A, 0x7BFF, 0x7FDE }, // M2 Gentle  31/31/30/30/31/30 (183)
    { 0x726319B8, 0x7FDF, 0x7BFE }, // M2 Sassy   31/30/31/31/30/30 (183)
    { 0x737FB7A4, 0x7BFE, 0x7BFF }, // M2 Quirky  30/31/30/31/30/31 (183)
    { 0x741F5DAF, 0x7FDF, 0x7BDF }, // M2 Bold    31/30/31/30/30/31 (183)
    { 0x74504057, 0x7FDE, 0x7FDF }, // M2 Relaxed 30/30/31/30/31/31 (183)
    { 0x781AB58A, 0x7FFE, 0x7BFE }, // M2 Bold    30/31/31/31/30/30 (183)
    { 0x79D6F981, 0x7FFE, 0x7BDF }, // M2 Jolly   30/31/31/30/30/31 (183)
    { 0x7F38359B, 0x7BDF, 0x7FFE }, // M2 Gentle  31/30/30/31/31/30 (183)
    { 0x80F47993, 0x7BDF, 0x7FDF }, // M2 Bold    31/30/30/30/31/31 (183)
    { 0x84BEEEC5, 0x7BFF, 0x7BFE }, // M2 Brave   31/31/30/31/30/30 (183)
    { 0x87132083, 0x7FFE, 0x7FDE }, // M2 Quiet   30/31/31/30/31/30 (183)
    { 0x87BF6C6A, 0x7FDE, 0x7FFE }, // M2 Modest  30/30/31/31/31/30 (183)
    { 0x93B759BE, 0x7BFF, 0x7FDE }, // M2 Naive   31/31/30/30/31/30 (183)
    { 0x96551214, 0x7FDF, 0x7BDF }, // M2 Jolly   31/30/31/30/30/31 (183)
    { 0x9C0CADE6, 0x7FFE, 0x7BDF }, // M2 Gentle  30/31/31/30/30/31 (183)
    { 0x9CB8F9CE, 0x7FDE, 0x7BFF }, // M2 Calm    30/30/31/31/30/31 (183)
    { 0xA32A2DF7, 0x7BDF, 0x7FDF }, // M2 Serious 31/30/30/30/31/31 (183)
    { 0xA6F4A32A, 0x7BFF, 0x7BFE }, // M2 Timid   31/31/30/31/30/30 (183)
    { 0xA8B0E721, 0x7BFF, 0x7BDF }, // M2 Bashful 31/31/30/30/30/31 (183)
    { 0xA8E1C9C9, 0x7BFE, 0x7FDF }, // M2 Calm    30/31/30/30/31/31 (183)
    { 0xABB164C7, 0x7FDE, 0x7FDF }, // M2 Relaxed 30/30/31/30/31/31 (183)
    { 0xAF7AD9F9, 0x7FFE, 0x7BFE }, // M2 Bashful 30/31/31/31/30/30 (183)
    { 0xB6687763, 0x7BDF, 0x7BFF }, // M2 Rash    31/30/30/31/30/31 (183)
    { 0xB8559E02, 0x7BDF, 0x7FDF }, // M2 Naughty 31/30/30/30/31/31 (183)
    { 0xBC50F5DD, 0x7BFE, 0x7FFE }, // M2 Naughty 30/31/30/31/31/30 (183)
    { 0xBE0B39D4, 0x7BFE, 0x7FDF }, // M2 Modest  30/31/30/30/31/31 (183)
    { 0xBEEEAE32, 0x7FDE, 0x7BFF }, // M2 Brave   30/30/31/31/30/31 (183)
    { 0xCAE69B86, 0x7BFF, 0x7BDF }, // M2 Lonely  31/31/30/30/30/31 (183)
    { 0xCB92E76E, 0x7BDF, 0x7BFF }, // M2 Hardy   31/30/30/31/30/31 (183)
    { 0xCBC3CA16, 0x7BDE, 0x7FFF }, // M2 Brave   30/30/30/31/31/31 (183)
    { 0xCF81996B, 0x7FFF, 0x7BDE }, // M2 Docile  31/31/31/30/30/30 (183)
    { 0xD02DE553, 0x7FDF, 0x7BFE }, // M2 Bold    31/30/31/31/30/30 (183)
    { 0xD1498340, 0x7BFE, 0x7BFF }, // M2 Sassy   30/31/30/31/30/31 (183)
    { 0xDC56B54F, 0x7BFF, 0x7FDE }, // M2 Docile  31/31/30/30/31/30 (183)
    { 0xDF26504C, 0x7FDF, 0x7FDE }, // M2 Quiet   31/30/31/30/31/30 (183)
    { 0xE4DDEC1E, 0x7FFE, 0x7FDE }, // M2 Hardy   30/31/31/30/31/30 (183)
    { 0xEDC89BD2, 0x7BDF, 0x7BFF }, // M2 Relaxed 31/30/30/31/30/31 (183)
    { 0xF180255A, 0x7BFF, 0x7FDE }, // M2 Lonely  31/31/30/30/31/30 (183)
    { 0xF26399B8, 0x7FDF, 0x7BFE }, // M2 Jolly   31/30/31/31/30/30 (183)
    { 0xF37F37A4, 0x7BFE, 0x7BFF }, // M2 Naughty 30/31/30/31/30/31 (183)
    { 0xF41FDDAF, 0x7FDF, 0x7BDF }, // M2 Gentle  31/30/31/30/30/31 (183)
    { 0xF450C057, 0x7FDE, 0x7FDF }, // M2 Careful 30/30/31/30/31/31 (183)
    { 0xF81A358A, 0x7FFE, 0x7BFE }, // M2 Timid   30/31/31/31/30/30 (183)
    { 0xF9D67981, 0x7FFE, 0x7BDF }, // M2 Bashful 30/31/31/30/30/31 (183)
    { 0xFF38B59B, 0x7BDF, 0x7FFE }, // M2 Serious 31/30/30/31/31/30 (183)
    { 0x03FA9EC4, 0x7BDF, 0x7FDF }, // M4 Quiet   31/30/30/30/31/31 (183)
    { 0x0C00F227, 0x7BFF, 0x7BFE }, // M4 Impish  31/31/30/31/30/30 (183)
    { 0x0C8AD450, 0x7BFE, 0x7FFE }, // M4 Serious 30/31/30/31/31/30 (183)
    { 0x0D433B8E, 0x7BFE, 0x7BFF }, // M4 Mild    30/31/30/31/30/31 (183)
    { 0x0D7B46CB, 0x7FDF, 0x7FDE }, // M4 Lax     31/30/31/30/31/30 (183)
    { 0x0E35AE09, 0x7FDF, 0x7BDF }, // M4 Timid   31/30/31/30/30/31 (183)
    { 0x0EBE9032, 0x7FDE, 0x7FDF }, // M4 Adamant 30/30/31/30/31/31 (183)
    { 0x1449F499, 0x7BDF, 0x7FFE }, // M4 Modest  31/30/30/31/31/30 (183)
    { 0x15035BD8, 0x7BDF, 0x7BFF }, // M4 Docile  31/30/30/31/30/31 (183)
    { 0x158D3E00, 0x7BDE, 0x7FFF }, // M4 Lax     30/30/30/31/31/31 (183)
    { 0x16C4E395, 0x7FFE, 0x7BFE }, // M4 Rash    30/31/31/31/30/30 (183)
    { 0x1ADD0C42, 0x7BFF, 0x7BDF }, // M4 Timid   31/31/30/30/30/31 (183)
    { 0x1B67EE6B, 0x7BFE, 0x7FDF }, // M4 Hardy   30/31/30/30/31/31 (183)
    { 0x1E8403DF, 0x7FDF, 0x7BFE }, // M4 Careful 31/30/31/31/30/30 (183)
    { 0x1F0DE607, 0x7FDE, 0x7FFE }, // M4 Lonely  30/30/31/31/31/30 (183)
    { 0x1FC74D46, 0x7FDE, 0x7BFF }, // M4 Quiet   30/30/31/31/30/31 (183)
    { 0x245EB449, 0x7FFF, 0x7BDE }, // M4 Jolly   31/31/31/30/30/30 (183)
    { 0x24E79672, 0x7FFE, 0x7FDE }, // M4 Docile  30/31/31/30/31/30 (183)
    { 0x25A1FDB0, 0x7FFE, 0x7BDF }, // M4 Relaxed 30/31/31/30/30/31 (183)
    { 0x2B2C6218, 0x7BFF, 0x7BFE }, // M4 Lax     31/31/30/31/30/30 (183)
    { 0x2BB64440, 0x7BFE, 0x7FFE }, // M4 Serious 30/31/30/31/31/30 (183)
    { 0x2C70AB7F, 0x7BFE, 0x7BFF }, // M4 Naive   30/31/30/31/30/31 (183)
    { 0x35F05386, 0x7FFE, 0x7BFE }, // M4 Calm    30/31/31/31/30/30 (183)
    { 0x3A935E5B, 0x7BFE, 0x7FDF }, // M4 Hardy   30/31/30/30/31/31 (183)
    { 0x4037069D, 0x7FDE, 0x7BFF }, // M4 Sassy   30/30/31/31/30/31 (183)
    { 0x44140662, 0x7FFE, 0x7FDE }, // M4 Quiet   30/31/31/30/31/30 (183)
    { 0x44CD6DA1, 0x7FFE, 0x7BDF }, // M4 Impish  30/31/31/30/30/31 (183)
    { 0x4AE2B431, 0x7BFE, 0x7FFE }, // M4 Quirky  30/31/30/31/31/30 (183)
    { 0x4B9C1B6F, 0x7BFE, 0x7BFF }, // M4 Naive   30/31/30/31/30/31 (183)
    { 0x551CC376, 0x7FFE, 0x7BFE }, // M4 Docile  30/31/31/31/30/30 (183)
    { 0x55F194B8, 0x7BDF, 0x7FFE }, // M4 Impish  31/30/30/31/31/30 (183)
    { 0x5A88FBBC, 0x7BFF, 0x7FDE }, // M4 Bold    31/31/30/30/31/30 (183)
    { 0x64CEAED3, 0x7BDF, 0x7FDF }, // M4 Gentle  31/30/30/30/31/31 (183)
    { 0x6E4F56DA, 0x7FDF, 0x7FDE }, // M4 Jolly   31/30/31/30/31/30 (183)
    { 0x6F09BE19, 0x7FDF, 0x7BDF }, // M4 Modest  31/30/31/30/30/31 (183)
    { 0x6F92A041, 0x7FDE, 0x7FDF }, // M4 Relaxed 30/30/31/30/31/31 (183)
    { 0x751D04A9, 0x7BDF, 0x7FFE }, // M4 Lax     31/30/30/31/31/30 (183)
    { 0x75D76BE7, 0x7BDF, 0x7BFF }, // M4 Timid   31/30/30/31/30/31 (183)
    { 0x76604E10, 0x7BDE, 0x7FFF }, // M4 Adamant 30/30/30/31/31/31 (183)
    { 0x7F5813EE, 0x7FDF, 0x7BFE }, // M4 Brave   31/30/31/31/30/30 (183)
    { 0x83FA1EC4, 0x7BDF, 0x7FDF }, // M4 Sassy   31/30/30/30/31/31 (183)
    { 0x8C007227, 0x7BFF, 0x7BFE }, // M4 Jolly   31/31/30/31/30/30 (183)
    { 0x8C8A5450, 0x7BFE, 0x7FFE }, // M4 Quiet   30/31/30/31/31/30 (183)
    { 0x8D43BB8E, 0x7BFE, 0x7BFF }, // M4 Relaxed 30/31/30/31/30/31 (183)
    { 0x8D7BC6CB, 0x7FDF, 0x7FDE }, // M4 Hardy   31/30/31/30/31/30 (183)
    { 0x8E352E09, 0x7FDF, 0x7BDF }, // M4 Modest  31/30/31/30/30/31 (183)
    { 0x8EBE1032, 0x7FDE, 0x7FDF }, // M4 Impish  30/30/31/30/31/31 (183)
    { 0x94497499, 0x7BDF, 0x7FFE }, // M4 Calm    31/30/30/31/31/30 (183)
    { 0x9503DBD8, 0x7BDF, 0x7BFF }, // M4 Sassy   31/30/30/31/30/31 (183)
    { 0x958DBE00, 0x7BDE, 0x7FFF }, // M4 Hardy   30/30/30/31/31/31 (183)
    { 0x96C46395, 0x7FFE, 0x7BFE }, // M4 Quirky  30/31/31/31/30/30 (183)
    { 0x9ADD8C42, 0x7BFF, 0x7BDF }, // M4 Lonely  31/31/30/30/30/31 (183)
    { 0x9B676E6B, 0x7BFE, 0x7FDF }, // M4 Bold    30/31/30/30/31/31 (183)
    { 0x9E8483DF, 0x7FDF, 0x7BFE }, // M4 Naive   31/30/31/31/30/30 (183)
    { 0x9F0D6607, 0x7FDE, 0x7FFE }, // M4 Docile  30/30/31/31/31/30 (183)
    { 0x9FC7CD46, 0x7FDE, 0x7BFF }, // M4 Impish  30/30/31/31/30/31 (183)
    { 0xA45E3449, 0x7FFF, 0x7BDE }, // M4 Bashful 31/31/31/30/30/30 (183)
    { 0xA4E71672, 0x7FFE, 0x7FDE }, // M4 Hasty   30/31/31/30/31/30 (183)
    { 0xA5A17DB0, 0x7FFE, 0x7BDF }, // M4 Serious 30/31/31/30/30/31 (183)
    { 0xAB2CE218, 0x7BFF, 0x7BFE }, // M4 Hardy   31/31/30/31/30/30 (183)
    { 0xABB6C440, 0x7BFE, 0x7FFE }, // M4 Adamant 30/31/30/31/31/30 (183)
    { 0xAC702B7F, 0x7BFE, 0x7BFF }, // M4 Rash    30/31/30/31/30/31 (183)
    { 0xB5F0D386, 0x7FFE, 0x7BFE }, // M4 Hasty   30/31/31/31/30/30 (183)
    { 0xBA93DE5B, 0x7BFE, 0x7FDF }, // M4 Mild    30/31/30/30/31/31 (183)
    { 0xC037869D, 0x7FDE, 0x7BFF }, // M4 Jolly   30/30/31/31/30/31 (183)
    { 0xC4148662, 0x7FFE, 0x7FDE }, // M4 Impish  30/31/31/30/31/30 (183)
    { 0xC4CDEDA1, 0x7FFE, 0x7BDF }, // M4 Quirky  30/31/31/30/30/31 (183)
    { 0xCAE23431, 0x7BFE, 0x7FFE }, // M4 Naughty 30/31/30/31/31/30 (183)
    { 0xCB9C9B6F, 0x7BFE, 0x7BFF }, // M4 Bold    30/31/30/31/30/31 (183)
    { 0xD51C4376, 0x7FFE, 0x7BFE }, // M4 Hasty   30/31/31/31/30/30 (183)
    { 0xD5F114B8, 0x7BDF, 0x7FFE }, // M4 Jolly   31/30/30/31/31/30 (183)
    { 0xDA887BBC, 0x7BFF, 0x7FDE }, // M4 Timid   31/31/30/30/31/30 (183)
    { 0xE4CE2ED3, 0x7BDF, 0x7FDF }, // M4 Lonely  31/30/30/30/31/31 (183)
    { 0xEE4FD6DA, 0x7FDF, 0x7FDE }, // M4 Naughty 31/30/31/30/31/30 (183)
    { 0xEF093E19, 0x7FDF, 0x7BDF }, // M4 Calm    31/30/31/30/30/31 (183)
    { 0xEF922041, 0x7FDE, 0x7FDF }, // M4 Serious 30/30/31/30/31/31 (183)
    { 0xF51D84A9, 0x7BDF, 0x7FFE }, // M4 Hardy   31/30/30/31/31/30 (183)
    { 0xF5D7EBE7, 0x7BDF, 0x7BFF }, // M4 Lonely  31/30/30/31/30/31 (183)
    { 0xF660CE10, 0x7BDE, 0x7FFF }, // M4 Rash    30/30/30/31/31/31 (183)
    { 0xFF5893EE, 0x7FDF, 0x7BFE }, // M4 Bashful 31/30/31/31/30/30 (183)
    { 0x0204E010, 0x7BFE, 0x7BDF }, // M1 Hasty   30/31/30/30/30/31 (182)
    { 0x056929C4, 0x7FDE, 0x7BFE }, // M1 Brave   30/30/31/31/30/30 (182)
    { 0x07F0008B, 0x7BFE, 0x7BFE }, // M1 Mild    30/31/30/31/30/30 (182)
    { 0x09BA3E42, 0x7FFE, 0x7BDE }, // M1 Adamant 30/31/31/30/30/30 (182)
    { 0x0AA206E1, 0x7FFE, 0x7BDE }, // M1 Adamant 30/31/31/30/30/30 (182)
    { 0x18E2DE55, 0x7BFE, 0x7BFE }, // M1 Serious 30/31/30/31/30/30 (182)
    { 0x28C7C561, 0x7BDF, 0x7FDE }, // M1 Impish  31/30/30/30/31/30 (182)
    { 0x29AE8E00, 0x7BDF, 0x7FDE }, // M1 Sassy   31/30/30/30/31/30 (182)
    { 0x34E57C90, 0x7BDF, 0x7BDF }, // M1 Naive   31/30/30/30/30/31 (182)
    { 0x35CC452F, 0x7BDF, 0x7BDF }, // M1 Adamant 31/30/30/30/30/31 (182)
    { 0x38D1DA8B, 0x7BDE, 0x7FDF }, // M1 Quiet   30/30/30/30/31/31 (182)
    { 0x3D82A361, 0x7FDF, 0x7BDE }, // M1 Docile  31/30/31/30/30/30 (182)
    { 0x408738BD, 0x7FDE, 0x7FDE }, // M1 Lax     30/30/31/30/31/30 (182)
    { 0x416F015C, 0x7FDE, 0x7FDE }, // M1 Lax     30/30/31/30/31/30 (182)
    { 0x45D75A5A, 0x7BDF, 0x7BDF }, // M1 Quirky  31/30/30/30/30/31 (182)
    { 0x4CA6EFEC, 0x7FDE, 0x7BDF }, // M1 Serious 30/30/31/30/30/31 (182)
    { 0x4E75812B, 0x7FDF, 0x7BDE }, // M1 Brave   31/30/31/30/30/30 (182)
    { 0x4FAFD8CF, 0x7BDE, 0x7FFE }, // M1 Quiet   30/30/30/31/31/30 (182)
    { 0x517A1686, 0x7FDE, 0x7FDE }, // M1 Naughty 30/30/31/30/31/30 (182)
    { 0x5CB5589E, 0x7BDF, 0x7BFE }, // M1 Quirky  31/30/30/31/30/30 (182)
    { 0x5D98CDB6, 0x7FDE, 0x7BDF }, // M1 Sassy   30/30/31/30/30/31 (182)
    { 0x5FBAEDF9, 0x7BDE, 0x7FFE }, // M1 Serious 30/30/30/31/31/30 (182)
    { 0x60A2B699, 0x7BDE, 0x7FFE }, // M1 Jolly   30/30/30/31/31/30 (182)
    { 0x61EE35BC, 0x7BFF, 0x7BDE }, // M1 Lonely  31/31/30/30/30/30 (182)
    { 0x64F4CB17, 0x7BFE, 0x7FDE }, // M1 Hardy   30/31/30/30/31/30 (182)
    { 0x6BD8A529, 0x7BDE, 0x7BFF }, // M1 Rash    30/30/30/31/30/31 (182)
    { 0x71128246, 0x7BFE, 0x7BDF }, // M1 Docile  30/31/30/30/30/31 (182)
    { 0x72E11385, 0x7BFF, 0x7BDE }, // M1 Gentle  31/31/30/30/30/30 (182)
    { 0x7476CBFA, 0x7FDE, 0x7BFE }, // M1 Sassy   30/30/31/31/30/30 (182)
    { 0x79AFA918, 0x7FFE, 0x7BDE }, // M1 Quirky  30/31/31/30/30/30 (182)
    { 0x82046010, 0x7BFE, 0x7BDF }, // M1 Mild    30/31/30/30/30/31 (182)
    { 0x8569A9C4, 0x7FDE, 0x7BFE }, // M1 Bashful 30/30/31/31/30/30 (182)
    { 0x87F0808B, 0x7BFE, 0x7BFE }, // M1 Relaxed 30/31/30/31/30/30 (182)
    { 0x89BABE42, 0x7FFE, 0x7BDE }, // M1 Rash    30/31/31/30/30/30 (182)
    { 0x8AA286E1, 0x7FFE, 0x7BDE }, // M1 Rash    30/31/31/30/30/30 (182)
    { 0x98E25E55, 0x7BFE, 0x7BFE }, // M1 Quiet   30/31/30/31/30/30 (182)
    { 0xA8C74561, 0x7BDF, 0x7FDE }, // M1 Jolly   31/30/30/30/31/30 (182)
    { 0xA9AE0E00, 0x7BDF, 0x7FDE }, // M1 Brave   31/30/30/30/31/30 (182)
    { 0xB4E5FC90, 0x7BDF, 0x7BDF }, // M1 Bold    31/30/30/30/30/31 (182)
    { 0xB5CCC52F, 0x7BDF, 0x7BDF }, // M1 Rash    31/30/30/30/30/31 (182)
    { 0xB8D15A8B, 0x7BDE, 0x7FDF }, // M1 Sassy   30/30/30/30/31/31 (182)
    { 0xBD822361, 0x7FDF, 0x7BDE }, // M1 Hasty   31/30/31/30/30/30 (182)
    { 0xC087B8BD, 0x7FDE, 0x7FDE }, // M1 Hardy   30/30/31/30/31/30 (182)
    { 0xC16F815C, 0x7FDE, 0x7FDE }, // M1 Hardy   30/30/31/30/31/30 (182)
    { 0xC5D7DA5A, 0x7BDF, 0x7BDF }, // M1 Modest  31/30/30/30/30/31 (182)
    { 0xCCA66FEC, 0x7FDE, 0x7BDF }, // M1 Quiet   30/30/31/30/30/31 (182)
    { 0xCE75012B, 0x7FDF, 0x7BDE }, // M1 Relaxed 31/30/31/30/30/30 (182)
    { 0xCFAF58CF, 0x7BDE, 0x7FFE }, // M1 Sassy   30/30/30/31/31/30 (182)
    { 0xD17A9686, 0x7FDE, 0x7FDE }, // M1 Calm    30/30/31/30/31/30 (182)
    { 0xDCB5D89E, 0x7BDF, 0x7BFE }, // M1 Modest  31/30/30/31/30/30 (182)
    { 0xDD984DB6, 0x7FDE, 0x7BDF }, // M1 Brave   30/30/31/30/30/31 (182)
    { 0xDFBA6DF9, 0x7BDE, 0x7FFE }, // M1 Quiet   30/30/30/31/31/30 (182)
    { 0xE0A23699, 0x7BDE, 0x7FFE }, // M1 Bashful 30/30/30/31/31/30 (182)
    { 0xE1EEB5BC, 0x7BFF, 0x7BDE }, // M1 Quiet   31/31/30/30/30/30 (182)
    { 0xE4F44B17, 0x7BFE, 0x7FDE }, // M1 Bold    30/31/30/30/31/30 (182)
    { 0xEBD82529, 0x7BDE, 0x7BFF }, // M1 Quirky  30/30/30/31/30/31 (182)
    { 0xF1120246, 0x7BFE, 0x7BDF }, // M1 Hasty   30/31/30/30/30/31 (182)
    { 0xF2E19385, 0x7BFF, 0x7BDE }, // M1 Serious 31/31/30/30/30/30 (182)
    { 0xF4764BFA, 0x7FDE, 0x7BFE }, // M1 Brave   30/30/31/31/30/30 (182)
    { 0xF9AF2918, 0x7FFE, 0x7BDE }, // M1 Naughty 30/31/31/30/30/30 (182)
    { 0x008B7BFE, 0x7BFE, 0x7BFE }, // M2 Gentle  30/31/30/31/30/30 (182)
    { 0x012B2209, 0x7FDF, 0x7BDE }, // M2 Brave   31/30/31/30/30/30 (182)
    { 0x015C04B1, 0x7FDE, 0x7FDE }, // M2 Naughty 30/30/31/30/31/30 (182)
    { 0x0246BFF5, 0x7BFE, 0x7BDF }, // M2 Bashful 30/31/30/30/30/31 (182)
    { 0x06E1BDDB, 0x7FFE, 0x7BDE }, // M2 Quirky  30/31/31/30/30/30 (182)
    { 0x0E003DEC, 0x7BDF, 0x7FDE }, // M2 Lonely  31/30/30/30/31/30 (182)
    { 0x1385F716, 0x7BFF, 0x7BDE }, // M2 Gentle  31/31/30/30/30/30 (182)
    { 0x168674BC, 0x7FDE, 0x7FDE }, // M2 Timid   30/30/31/30/31/30 (182)
    { 0x2361D66E, 0x7FDF, 0x7BDE }, // M2 Gentle  31/30/31/30/30/30 (182)
    { 0x2529C042, 0x7BDE, 0x7BFF }, // M2 Hasty   30/30/30/31/30/31 (182)
    { 0x2918723F, 0x7FFE, 0x7BDE }, // M2 Quiet   30/31/31/30/30/30 (182)
    { 0x29C4BE27, 0x7FDE, 0x7BFE }, // M2 Mild    30/30/31/31/30/30 (182)
    { 0x35BCAB7B, 0x7BFF, 0x7BDE }, // M2 Modest  31/31/30/30/30/30 (182)
    { 0x3699DA0B, 0x7BDE, 0x7FFE }, // M2 Mild    30/30/30/31/31/30 (182)
    { 0x38BD2920, 0x7FDE, 0x7FDE }, // M2 Adamant 30/30/31/30/31/30 (182)
    { 0x3E42E24A, 0x7FFE, 0x7BDE }, // M2 Careful 30/31/31/30/30/30 (182)
    { 0x452F7FB4, 0x7BDF, 0x7BDF }, // M2 Jolly   31/30/30/30/30/31 (182)
    { 0x4561625C, 0x7BDF, 0x7FDE }, // M2 Lonely  31/30/30/30/31/30 (182)
    { 0x4B17FE2E, 0x7BFE, 0x7FDE }, // M2 Careful 30/31/30/30/31/30 (182)
    { 0x4BFA728C, 0x7FDE, 0x7BFE }, // M2 Quirky  30/30/31/31/30/30 (182)
    { 0x4DB6B683, 0x7FDE, 0x7BDF }, // M2 Relaxed 30/30/31/30/30/31 (182)
    { 0x589EABC7, 0x7BDF, 0x7BFE }, // M2 Gentle  31/30/30/31/30/30 (182)
    { 0x58CF8E6F, 0x7BDE, 0x7FFE }, // M2 Careful 30/30/30/31/31/30 (182)
    { 0x5A5AEFBF, 0x7BDF, 0x7BDF }, // M2 Bold    31/30/30/30/30/31 (182)
    { 0x5A8BD267, 0x7BDE, 0x7FDF }, // M2 Relaxed 30/30/30/30/31/31 (182)
    { 0x5E554799, 0x7BFE, 0x7BFE }, // M2 Bashful 30/31/30/31/30/30 (182)
    { 0x60108B91, 0x7BFE, 0x7BDF }, // M2 Mild    30/31/30/30/30/31 (182)
    { 0x6DF9FE7A, 0x7BDE, 0x7FFE }, // M2 Naughty 30/30/30/31/31/30 (182)
    { 0x6FEC6AE8, 0x7FDE, 0x7BDF }, // M2 Modest  30/30/31/30/30/31 (182)
    { 0x7C90A424, 0x7BDF, 0x7BDF }, // M2 Jolly   31/30/30/30/30/31 (182)
    { 0x808BFBFE, 0x7BFE, 0x7BFE }, // M2 Serious 30/31/30/31/30/30 (182)
    { 0x812BA209, 0x7FDF, 0x7BDE }, // M2 Bashful 31/30/31/30/30/30 (182)
    { 0x815C84B1, 0x7FDE, 0x7FDE }, // M2 Calm    30/30/31/30/31/30 (182)
    { 0x82463FF5, 0x7BFE, 0x7BDF }, // M2 Careful 30/31/30/30/30/31 (182)
    { 0x86E13DDB, 0x7FFE, 0x7BDE }, // M2 Naughty 30/31/31/30/30/30 (182)
    { 0x8E00BDEC, 0x7BDF, 0x7FDE }, // M2 Quiet   31/30/30/30/31/30 (182)
    { 0x93857716, 0x7BFF, 0x7BDE }, // M2 Lonely  31/31/30/30/30/30 (182)
    { 0x9686F4BC, 0x7FDE, 0x7FDE }, // M2 Lonely  30/30/31/30/31/30 (182)
    { 0xA361566E, 0x7FDF, 0x7BDE }, // M2 Lonely  31/30/31/30/30/30 (182)
    { 0xA5294042, 0x7BDE, 0x7BFF }, // M2 Mild    30/30/30/31/30/31 (182)
    { 0xA918F23F, 0x7FFE, 0x7BDE }, // M2 Impish  30/31/31/30/30/30 (182)
    { 0xA9C43E27, 0x7FDE, 0x7BFE }, // M2 Gentle  30/30/31/31/30/30 (182)
    { 0xB5BC2B7B, 0x7BFF, 0x7BDE }, // M2 Calm    31/31/30/30/30/30 (182)
    { 0xB6995A0B, 0x7BDE, 0x7FFE }, // M2 Gentle  30/30/30/31/31/30 (182)
    { 0xB8BDA920, 0x7FDE, 0x7FDE }, // M2 Rash    30/30/31/30/31/30 (182)
    { 0xBE42624A, 0x7FFE, 0x7BDE }, // M2 Adamant 30/31/31/30/30/30 (182)
    { 0xC52FFFB4, 0x7BDF, 0x7BDF }, // M2 Naughty 31/30/30/30/30/31 (182)
    { 0xC561E25C, 0x7BDF, 0x7FDE }, // M2 Quiet   31/30/30/30/31/30 (182)
    { 0xCB177E2E, 0x7BFE, 0x7FDE }, // M2 Adamant 30/31/30/30/31/30 (182)
    { 0xCBFAF28C, 0x7FDE, 0x7BFE }, // M2 Modest  30/30/31/31/30/30 (182)
    { 0xCDB63683, 0x7FDE, 0x7BDF }, // M2 Serious 30/30/31/30/30/31 (182)
    { 0xD89E2BC7, 0x7BDF, 0x7BFE }, // M2 Lonely  31/30/30/31/30/30 (182)
    { 0xD8CF0E6F, 0x7BDE, 0x7FFE }, // M2 Adamant 30/30/30/31/31/30 (182)
    { 0xDA5A6FBF, 0x7BDF, 0x7BDF }, // M2 Timid   31/30/30/30/30/31 (182)
    { 0xDA8B5267, 0x7BDE, 0x7FDF }, // M2 Serious 30/30/30/30/31/31 (182)
    { 0xDE55C799, 0x7BFE, 0x7BFE }, // M2 Lax     30/31/30/31/30/30 (182)
    { 0xE0100B91, 0x7BFE, 0x7BDF }, // M2 Gentle  30/31/30/30/30/31 (182)
    { 0xEDF97E7A, 0x7BDE, 0x7FFE }, // M2 Lax     30/30/30/31/31/30 (182)
    { 0xEFECEAE8, 0x7FDE, 0x7BDF }, // M2 Docile  30/30/31/30/30/31 (182)
    { 0xFC902424, 0x7BDF, 0x7BDF }, // M2 Bashful 31/30/30/30/30/31 (182)
    { 0x02B7555D, 0x7BDF, 0x7FDE }, // M4 Careful 31/30/30/30/31/30 (182)
    { 0x199AC2DC, 0x7BFF, 0x7BDE }, // M4 Adamant 31/31/30/30/30/30 (182)
    { 0x1A23A504, 0x7BFE, 0x7FDE }, // M4 Calm    30/31/30/30/31/30 (182)
    { 0x23E075F3, 0x7BDF, 0x7BDF }, // M4 Rash    31/30/30/30/30/31 (182)
    { 0x246A581B, 0x7BDE, 0x7FDF }, // M4 Sassy   30/30/30/30/31/31 (182)
    { 0x2D611DFA, 0x7FDF, 0x7BDE }, // M4 Hasty   31/30/31/30/30/30 (182)
    { 0x2DEB0022, 0x7FDE, 0x7FDE }, // M4 Naive   30/30/31/30/31/30 (182)
    { 0x2EA46761, 0x7FDE, 0x7BDF }, // M4 Bold    30/30/31/30/30/31 (182)
    { 0x342FCBC8, 0x7BDF, 0x7BFE }, // M4 Quiet   31/30/30/31/30/30 (182)
    { 0x34B9ADF1, 0x7BDE, 0x7FFE }, // M4 Gentle  30/30/30/31/31/30 (182)
    { 0x3573152F, 0x7BDE, 0x7BFF }, // M4 Hasty   30/30/30/31/30/31 (182)
    { 0x38C632CC, 0x7BFF, 0x7BDE }, // M4 Adamant 31/31/30/30/30/30 (182)
    { 0x394F14F5, 0x7BFE, 0x7FDE }, // M4 Gentle  30/31/30/30/31/30 (182)
    { 0x3A097C33, 0x7BFE, 0x7BDF }, // M4 Sassy   30/31/30/30/30/31 (182)
    { 0x3EF3BD36, 0x7FDE, 0x7BFE }, // M4 Adamant 30/30/31/31/30/30 (182)
    { 0x438A243A, 0x7FFE, 0x7BDE }, // M4 Naive   30/31/31/30/30/30 (182)
    { 0x4396C80C, 0x7BDE, 0x7FDF }, // M4 Lax     30/30/30/30/31/31 (182)
    { 0x4A58D209, 0x7BFE, 0x7BFE }, // M4 Gentle  30/31/30/31/30/30 (182)
    { 0x4C8D8DEB, 0x7FDF, 0x7BDE }, // M4 Careful 31/30/31/30/30/30 (182)
    { 0x4D177013, 0x7FDE, 0x7FDE }, // M4 Lonely  30/30/31/30/31/30 (182)
    { 0x4DD1D751, 0x7FDE, 0x7BDF }, // M4 Brave   30/30/31/30/30/31 (182)
    { 0x535B3BB9, 0x7BDF, 0x7BFE }, // M4 Bashful 31/30/30/31/30/30 (182)
    { 0x53E51DE1, 0x7BDE, 0x7FFE }, // M4 Gentle  30/30/30/31/31/30 (182)
    { 0x549F8520, 0x7BDE, 0x7BFF }, // M4 Careful 30/30/30/31/30/31 (182)
    { 0x5A79358A, 0x7BFE, 0x7BDF }, // M4 Brave   30/31/30/30/30/31 (182)
    { 0x5E1F2D27, 0x7FDE, 0x7BFE }, // M4 Naughty 30/30/31/31/30/30 (182)
    { 0x63FADD91, 0x7FFE, 0x7BDE }, // M4 Bold    30/31/31/30/30/30 (182)
    { 0x6AC88B60, 0x7BFE, 0x7BFE }, // M4 Lonely  30/31/30/31/30/30 (182)
    { 0x6CFD4742, 0x7FDE, 0x7BDF }, // M4 Adamant 30/30/31/30/30/31 (182)
    { 0x7A6ED2EB, 0x7BFF, 0x7BDE }, // M4 Relaxed 31/31/30/30/30/30 (182)
    { 0x82B7D55D, 0x7BDF, 0x7FDE }, // M4 Naive   31/30/30/30/31/30 (182)
    { 0x999A42DC, 0x7BFF, 0x7BDE }, // M4 Impish  31/31/30/30/30/30 (182)
    { 0x9A232504, 0x7BFE, 0x7FDE }, // M4 Hardy   30/31/30/30/31/30 (182)
    { 0xA3E0F5F3, 0x7BDF, 0x7BDF }, // M4 Timid   31/30/30/30/30/31 (182)
    { 0xA46AD81B, 0x7BDE, 0x7FDF }, // M4 Jolly   30/30/30/30/31/31 (182)
    { 0xAD619DFA, 0x7FDF, 0x7BDE }, // M4 Brave   31/30/31/30/30/30 (182)
    { 0xADEB8022, 0x7FDE, 0x7FDE }, // M4 Bold    30/30/31/30/31/30 (182)
    { 0xAEA4E761, 0x7FDE, 0x7BDF }, // M4 Gentle  30/30/31/30/30/31 (182)
    { 0xB42F4BC8, 0x7BDF, 0x7BFE }, // M4 Sassy   31/30/30/31/30/30 (182)
    { 0xB4B92DF1, 0x7BDE, 0x7FFE }, // M4 Lonely  30/30/30/31/31/30 (182)
    { 0xB573952F, 0x7BDE, 0x7BFF }, // M4 Brave   30/30/30/31/30/31 (182)
    { 0xB8C6B2CC, 0x7BFF, 0x7BDE }, // M4 Rash    31/31/30/30/30/30 (182)
    { 0xB94F94F5, 0x7BFE, 0x7FDE }, // M4 Serious 30/31/30/30/31/30 (182)
    { 0xBA09FC33, 0x7BFE, 0x7BDF }, // M4 Jolly   30/31/30/30/30/31 (182)
    { 0xBEF33D36, 0x7FDE, 0x7BFE }, // M4 Impish  30/30/31/31/30/30 (182)
    { 0xC38AA43A, 0x7FFE, 0x7BDE }, // M4 Bold    30/31/31/30/30/30 (182)
    { 0xC396480C, 0x7BDE, 0x7FDF }, // M4 Naive   30/30/30/30/31/31 (182)
    { 0xCA585209, 0x7BFE, 0x7BFE }, // M4 Lonely  30/31/30/31/30/30 (182)
    { 0xCC8D0DEB, 0x7FDF, 0x7BDE }, // M4 Adamant 31/30/31/30/30/30 (182)
    { 0xCD17F013, 0x7FDE, 0x7FDE }, // M4 Quiet   30/30/31/30/31/30 (182)
    { 0xCDD15751, 0x7FDE, 0x7BDF }, // M4 Relaxed 30/30/31/30/30/31 (182)
    { 0xD35BBBB9, 0x7BDF, 0x7BFE }, // M4 Lax     31/30/30/31/30/30 (182)
    { 0xD3E59DE1, 0x7BDE, 0x7FFE }, // M4 Serious 30/30/30/31/31/30 (182)
    { 0xD49F0520, 0x7BDE, 0x7BFF }, // M4 Adamant 30/30/30/31/30/31 (182)
    { 0xDA79B58A, 0x7BFE, 0x7BDF }, // M4 Bashful 30/31/30/30/30/31 (182)
    { 0xDE1FAD27, 0x7FDE, 0x7BFE }, // M4 Calm    30/30/31/31/30/30 (182)
    { 0xE3FA5D91, 0x7FFE, 0x7BDE }, // M4 Timid   30/31/31/30/30/30 (182)
    { 0xEAC80B60, 0x7BFE, 0x7BFE }, // M4 Docile  30/31/30/31/30/30 (182)
    { 0xECFDC742, 0x7FDE, 0x7BDF }, // M4 Rash    30/30/31/30/30/31 (182)
    { 0xFA6E52EB, 0x7BFF, 0x7BDE }, // M4 Serious 31/31/30/30/30/30 (182)
    { 0x13E7899C, 0x7BDF, 0x7BDE }, // M1 Careful 31/30/30/30/30/30 (181)
    { 0x17D4E797, 0x7BDE, 0x7FDE }, // M1 Serious 30/30/30/30/31/30 (181)
    { 0x24DA6766, 0x7BDF, 0x7BDE }, // M1 Rash    31/30/30/30/30/30 (181)
    { 0x27DFFCC1, 0x7BDE, 0x7FDE }, // M1 Relaxed 30/30/30/30/31/30 (181)
    { 0x33FDB3F0, 0x7BDE, 0x7BDF }, // M1 Jolly   30/30/30/30/30/31 (181)
    { 0x3C9BDAC2, 0x7FDE, 0x7BDE }, // M1 Quiet   30/30/31/30/30/30 (181)
    { 0x4ADBB235, 0x7BDE, 0x7BFE }, // M1 Naive   30/30/30/31/30/30 (181)
    { 0x4BC37AD4, 0x7BDE, 0x7BFE }, // M1 Naive   30/30/30/31/30/30 (181)
    { 0x4D8DB88B, 0x7FDE, 0x7BDE }, // M1 Lonely  30/30/31/30/30/30 (181)
    { 0x50148F52, 0x7BFE, 0x7BDE }, // M1 Modest  30/31/30/30/30/30 (181)
    { 0x5BCE8FFE, 0x7BDE, 0x7BFE }, // M1 Lax     30/30/30/31/30/30 (181)
    { 0x61076D1C, 0x7BFE, 0x7BDE }, // M1 Hasty   30/31/30/30/30/30 (181)
    { 0x93E7099C, 0x7BDF, 0x7BDE }, // M1 Adamant 31/30/30/30/30/30 (181)
    { 0x97D46797, 0x7BDE, 0x7FDE }, // M1 Quiet   30/30/30/30/31/30 (181)
    { 0xA4DAE766, 0x7BDF, 0x7BDE }, // M1 Timid   31/30/30/30/30/30 (181)
    { 0xA7DF7CC1, 0x7BDE, 0x7FDE }, // M1 Serious 30/30/30/30/31/30 (181)
    { 0xB3FD33F0, 0x7BDE, 0x7BDF }, // M1 Bashful 30/30/30/30/30/31 (181)
    { 0xBC9B5AC2, 0x7FDE, 0x7BDE }, // M1 Sassy   30/30/31/30/30/30 (181)
    { 0xCADB3235, 0x7BDE, 0x7BFE }, // M1 Rash    30/30/30/31/30/30 (181)
    { 0xCBC3FAD4, 0x7BDE, 0x7BFE }, // M1 Bold    30/30/30/31/30/30 (181)
    { 0xCD8D388B, 0x7FDE, 0x7BDE }, // M1 Docile  30/30/31/30/30/30 (181)
    { 0xD0140F52, 0x7BFE, 0x7BDE }, // M1 Calm    30/31/30/30/30/30 (181)
    { 0xDBCE0FFE, 0x7BDE, 0x7BFE }, // M1 Naive   30/30/30/31/30/30 (181)
    { 0xE107ED1C, 0x7BFE, 0x7BDE }, // M1 Brave   30/31/30/30/30/30 (181)
    { 0x099C687D, 0x7BDF, 0x7BDE }, // M2 Lax     31/30/30/30/30/30 (181)
    { 0x0F52044F, 0x7BFE, 0x7BDE }, // M2 Calm    30/31/30/30/30/30 (181)
    { 0x0FFE5037, 0x7BDE, 0x7BFE }, // M2 Rash    30/30/30/31/30/30 (181)
    { 0x3235049B, 0x7BDE, 0x7BFE }, // M2 Serious 30/30/30/31/30/30 (181)
    { 0x33F04893, 0x7BDE, 0x7BDF }, // M2 Timid   30/30/30/30/30/31 (181)
    { 0x388B4678, 0x7FDE, 0x7BDE }, // M2 Modest  30/30/31/30/30/30 (181)
    { 0x5AC2FADD, 0x7FDE, 0x7BDE }, // M2 Calm    30/30/31/30/30/30 (181)
    { 0x67663419, 0x7BDF, 0x7BDE }, // M2 Relaxed 31/30/30/30/30/30 (181)
    { 0x679716C0, 0x7BDE, 0x7FDE }, // M2 Impish  30/30/30/30/31/30 (181)
    { 0x6D1CCFEB, 0x7BFE, 0x7BDE }, // M2 Naughty 30/31/30/30/30/30 (181)
    { 0x7AD4602C, 0x7BDE, 0x7BFE }, // M2 Naughty 30/30/30/31/30/30 (181)
    { 0x7CC186CB, 0x7BDE, 0x7FDE }, // M2 Naive   30/30/30/30/31/30 (181)
    { 0x899CE87D, 0x7BDF, 0x7BDE }, // M2 Hardy   31/30/30/30/30/30 (181)
    { 0x8F52844F, 0x7BFE, 0x7BDE }, // M2 Hasty   30/31/30/30/30/30 (181)
    { 0x8FFED037, 0x7BDE, 0x7BFE }, // M2 Timid   30/30/30/31/30/30 (181)
    { 0xB235849B, 0x7BDE, 0x7BFE }, // M2 Adamant 30/30/30/31/30/30 (181)
    { 0xB3F0C893, 0x7BDE, 0x7BDF }, // M2 Lonely  30/30/30/30/30/31 (181)
    { 0xB88BC678, 0x7FDE, 0x7BDE }, // M2 Docile  30/30/31/30/30/30 (181)
    { 0xDAC27ADD, 0x7FDE, 0x7BDE }, // M2 Hardy   30/30/31/30/30/30 (181)
    { 0xE766B419, 0x7BDF, 0x7BDE }, // M2 Careful 31/30/30/30/30/30 (181)
    { 0xE79796C0, 0x7BDE, 0x7FDE }, // M2 Quirky  30/30/30/30/31/30 (181)
    { 0xED1C4FEB, 0x7BFE, 0x7BDE }, // M2 Lax     30/31/30/30/30/30 (181)
    { 0xFAD4E02C, 0x7BDE, 0x7BFE }, // M2 Calm    30/30/30/31/30/30 (181)
    { 0xFCC106CB, 0x7BDE, 0x7FDE }, // M2 Rash    30/30/30/30/31/30 (181)
    { 0x12F7E501, 0x7BDE, 0x7BFE }, // M4 Bold    30/30/30/31/30/30 (181)
    { 0x229D2C8C, 0x7BDF, 0x7BDE }, // M4 Hardy   31/30/30/30/30/30 (181)
    { 0x23260EB5, 0x7BDE, 0x7FDE }, // M4 Bashful 30/30/30/30/31/30 (181)
    { 0x41C99C7D, 0x7BDF, 0x7BDE }, // M4 Serious 31/30/30/30/30/30 (181)
    { 0x42537EA5, 0x7BDE, 0x7FDE }, // M4 Modest  30/30/30/30/31/30 (181)
    { 0x430CE5E4, 0x7BDE, 0x7BDF }, // M4 Docile  30/30/30/30/30/31 (181)
    { 0x5935EC24, 0x7BFE, 0x7BDE }, // M4 Lax     30/31/30/30/30/30 (181)
    { 0x73CBF510, 0x7BDE, 0x7BFE }, // M4 Lax     30/30/30/31/30/30 (181)
    { 0x78625C14, 0x7BFE, 0x7BDE }, // M4 Calm    30/31/30/30/30/30 (181)
    { 0x92F76501, 0x7BDE, 0x7BFE }, // M4 Timid   30/30/30/31/30/30 (181)
    { 0xA29DAC8C, 0x7BDF, 0x7BDE }, // M4 Mild    31/30/30/30/30/30 (181)
    { 0xA3268EB5, 0x7BDE, 0x7FDE }, // M4 Lax     30/30/30/30/31/30 (181)
    { 0xC1C91C7D, 0x7BDF, 0x7BDE }, // M4 Quiet   31/30/30/30/30/30 (181)
    { 0xC253FEA5, 0x7BDE, 0x7FDE }, // M4 Docile  30/30/30/30/31/30 (181)
    { 0xC30C65E4, 0x7BDE, 0x7BDF }, // M4 Hasty   30/30/30/30/30/31 (181)
    { 0xD9356C24, 0x7BFE, 0x7BDE }, // M4 Naive   30/31/30/30/30/30 (181)
    { 0xF3CB7510, 0x7BDE, 0x7BFE }, // M4 Naive   30/30/30/31/30/30 (181)
    { 0xF862DC14, 0x7BFE, 0x7BDE }, // M4 Hasty   30/31/30/30/30/30 (181)
    { 0x12FFC0FC, 0x7BDE, 0x7BDE }, // M1 Sassy   30/30/30/30/30/30 (180)
    { 0x23F29EC6, 0x7BDE, 0x7BDE }, // M1 Bashful 30/30/30/30/30/30 (180)
    { 0x92FF40FC, 0x7BDE, 0x7BDE }, // M1 Brave   30/30/30/30/30/30 (180)
    { 0xA3F21EC6, 0x7BDE, 0x7BDE }, // M1 Careful 30/30/30/30/30/30 (180)
    { 0x1EC6D888, 0x7BDE, 0x7BDE }, // M2 Modest  30/30/30/30/30/30 (180)
    { 0x40FC8CED, 0x7BDE, 0x7BDE }, // M2 Careful 30/30/30/30/30/30 (180)
    { 0x9EC65888, 0x7BDE, 0x7BDE }, // M2 Calm    30/30/30/30/30/30 (180)
    { 0xC0FC0CED, 0x7BDE, 0x7BDE }, // M2 Adamant 30/30/30/30/30/30 (180)
    { 0x016545C5, 0x7BDE, 0x7BDE }, // M4 Jolly   30/30/30/30/30/30 (180)
    { 0x2091B5B5, 0x7BDE, 0x7BDE }, // M4 Quirky  30/30/30/30/30/30 (180)
    { 0x623955D4, 0x7BDE, 0x7BDE }, // M4 Quiet   30/30/30/30/30/30 (180)
    { 0x8165C5C5, 0x7BDE, 0x7BDE }, // M4 Naughty 30/30/30/30/30/30 (180)
    { 0xA09135B5, 0x7BDE, 0x7BDE }, // M4 Naughty 30/30/30/30/30/30 (180)
    { 0xE239D5D4, 0x7BDE, 0x7BDE }, // M4 Impish  30/30/30/30/30/30 (180)
};
// END GENERATED sIvPoolWild

// Method 2 PID/IV pairs for wish eggs: the first WISH_EGG_PID_IV_EGG_COUNT have all IVs 30-31 and
// are what eggs are given; the rest have Atk 0-1 (other IVs 30-31) and are only used when changing
// a hatched wish egg Pokemon's nature, ability or gender.
#define WISH_EGG_PID_IV_EGG_COUNT 260

static const struct PidIvPair sWishEggPidIvs[] =
{
    { 0x008B7BFE, 0x7BFE, 0x7BFE }, // Gentle, 30/31/30/30/31/30
    { 0x00F4F993, 0x7BDF, 0x7FDF }, // Hardy, 31/30/30/31/30/31
    { 0x012B2209, 0x7FDF, 0x7BDE }, // Brave, 31/30/31/30/30/30
    { 0x015C04B1, 0x7FDE, 0x7FDE }, // Naughty, 30/30/31/30/30/31
    { 0x0246BFF5, 0x7BFE, 0x7BDF }, // Bashful, 30/31/30/31/30/30
    { 0x04BE6EC5, 0x7BFF, 0x7BFE }, // Hasty, 31/31/30/30/31/30
    { 0x04EF516D, 0x7BFF, 0x7FFE }, // Jolly, 31/31/30/30/31/31
    { 0x06AB9565, 0x7BFF, 0x7FDF }, // Sassy, 31/31/30/31/30/31
    { 0x06E1BDDB, 0x7FFE, 0x7BDE }, // Quirky, 30/31/31/30/30/30
    { 0x0713A083, 0x7FFE, 0x7FDE }, // Serious, 30/31/31/30/30/31
    { 0x078E09C3, 0x7FDF, 0x7BFF }, // Careful, 31/30/31/31/31/30
    { 0x07BFEC6A, 0x7FDE, 0x7FFE }, // Timid, 30/30/31/30/31/31
    { 0x099C687D, 0x7BDF, 0x7BDE }, // Lax, 31/30/30/30/30/30
    { 0x0B46934A, 0x7FFF, 0x7FDE }, // Brave, 31/31/31/30/30/31
    { 0x0BF2DF32, 0x7FDF, 0x7FFF }, // Lonely, 31/30/31/31/31/31
    { 0x0D44A595, 0x7FFE, 0x7BFF }, // Calm, 30/31/31/31/31/30
    { 0x0E003DEC, 0x7BDF, 0x7FDE }, // Lonely, 31/30/30/30/30/31
    { 0x0F52044F, 0x7BFE, 0x7BDE }, // Calm, 30/31/30/30/30/30
    { 0x0FFE5037, 0x7BDE, 0x7BFE }, // Rash, 30/30/30/30/31/30
    { 0x11A97B04, 0x7FFF, 0x7FFF }, // Careful, 31/31/31/31/31/31
    { 0x1385F716, 0x7BFF, 0x7BDE }, // Gentle, 31/31/30/30/30/30
    { 0x13B7D9BE, 0x7BFF, 0x7FDE }, // Lax, 31/31/30/30/30/31
    { 0x146325A6, 0x7BDF, 0x7FFF }, // Sassy, 31/30/30/31/31/31
    { 0x16559214, 0x7FDF, 0x7BDF }, // Impish, 31/30/31/31/30/30
    { 0x168674BC, 0x7FDE, 0x7FDE }, // Timid, 30/30/31/30/30/31
    { 0x18FE238B, 0x7FDF, 0x7FFE }, // Brave, 31/30/31/30/31/31
    { 0x19E8DED0, 0x7BFF, 0x7BFF }, // Quiet, 31/31/30/31/31/30
    { 0x1A19C178, 0x7BFE, 0x7FFF }, // Rash, 30/31/30/31/31/31
    { 0x1C0C2DE6, 0x7FFE, 0x7BDF }, // Bold, 30/31/31/31/30/30
    { 0x1CB879CE, 0x7FDE, 0x7BFF }, // Naughty, 30/30/31/31/31/30
    { 0x1E83DCB6, 0x7FFF, 0x7BFE }, // Careful, 31/31/31/30/31/30
    { 0x1EB4BF5D, 0x7FFF, 0x7FFE }, // Quirky, 31/31/31/30/31/31
    { 0x1EC6D888, 0x7BDE, 0x7BDE }, // Modest, 30/30/30/30/30/30
    { 0x20700355, 0x7FFF, 0x7FDF }, // Sassy, 31/31/31/31/30/31
    { 0x226F159F, 0x7FFE, 0x7BFF }, // Hardy, 30/31/31/31/31/30
    { 0x232AADF7, 0x7BDF, 0x7FDF }, // Relaxed, 31/30/30/31/30/31
    { 0x2361D66E, 0x7FDF, 0x7BDE }, // Gentle, 31/30/31/30/30/30
    { 0x2529C042, 0x7BDE, 0x7BFF }, // Hasty, 30/30/30/31/31/30
    { 0x26F4232A, 0x7BFF, 0x7BFE }, // Rash, 31/31/30/30/31/30
    { 0x28B06721, 0x7BFF, 0x7BDF }, // Brave, 31/31/30/31/30/30
    { 0x28E149C9, 0x7BFE, 0x7FDF }, // Naughty, 30/31/30/31/30/31
    { 0x2918723F, 0x7FFE, 0x7BDE }, // Quiet, 30/31/31/30/30/30
    { 0x298D95B1, 0x7BDF, 0x7FFF }, // Adamant, 31/30/30/31/31/31
    { 0x29C4BE27, 0x7FDE, 0x7BFE }, // Mild, 30/30/31/30/31/30
    { 0x2BB1E4C7, 0x7FDE, 0x7FDF }, // Brave, 30/30/31/31/30/31
    { 0x2D7C47AF, 0x7FFF, 0x7FDE }, // Timid, 31/31/31/30/30/31
    { 0x2E289396, 0x7FDF, 0x7FFE }, // Impish, 31/30/31/30/31/31
    { 0x2F443183, 0x7BFE, 0x7FFF }, // Hardy, 30/31/30/31/31/31
    { 0x2F7A59F9, 0x7FFE, 0x7BFE }, // Brave, 30/31/31/30/31/30
    { 0x3235049B, 0x7BDE, 0x7BFE }, // Serious, 30/30/30/30/31/30
    { 0x33AD4CC0, 0x7FFF, 0x7BFF }, // Quiet, 31/31/31/31/31/30
    { 0x33DF2F68, 0x7FFE, 0x7FFE }, // Bold, 30/31/31/30/31/31
    { 0x33F04893, 0x7BDE, 0x7BDF }, // Timid, 30/30/30/31/30/30
    { 0x35BCAB7B, 0x7BFF, 0x7BDE }, // Modest, 31/31/30/30/30/30
    { 0x3668F763, 0x7BDF, 0x7BFF }, // Naive, 31/30/30/31/31/30
    { 0x3699DA0B, 0x7BDE, 0x7FFE }, // Mild, 30/30/30/30/31/31
    { 0x38551E02, 0x7BDF, 0x7FDF }, // Jolly, 31/30/30/31/30/31
    { 0x388B4678, 0x7FDE, 0x7BDE }, // Modest, 30/30/31/30/30/30
    { 0x38BD2920, 0x7FDE, 0x7FDE }, // Adamant, 30/30/31/30/30/31
    { 0x3C1E9335, 0x7BFF, 0x7BFF }, // Hardy, 31/31/30/31/31/30
    { 0x3C5075DD, 0x7BFE, 0x7FFE }, // Jolly, 30/31/30/30/31/31
    { 0x3CF01BE8, 0x7FDF, 0x7FDF }, // Rash, 31/30/31/31/30/31
    { 0x3E0BB9D4, 0x7BFE, 0x7FDF }, // Timid, 30/31/30/31/30/31
    { 0x3E42E24A, 0x7FFE, 0x7BDE }, // Careful, 30/31/31/30/30/30
    { 0x3EEE2E32, 0x7FDE, 0x7BFF }, // Hasty, 30/30/31/31/31/30
    { 0x408368A4, 0x7BFF, 0x7FFF }, // Adamant, 31/31/30/31/31/31
    { 0x40B9911A, 0x7FFF, 0x7BFE }, // Bold, 31/31/31/30/31/30
    { 0x40FC8CED, 0x7BDE, 0x7BDE }, // Careful, 30/30/30/30/30/30
    { 0x4275D512, 0x7FFF, 0x7BDF }, // Naive, 31/31/31/31/30/30
    { 0x42A6B7BA, 0x7FFE, 0x7FDF }, // Mild, 30/31/31/31/30/31
    { 0x435203A1, 0x7FDF, 0x7FFF }, // Adamant, 31/30/31/31/31/31
    { 0x452F7FB4, 0x7BDF, 0x7BDF }, // Jolly, 31/30/30/31/30/30
    { 0x4561625C, 0x7BDF, 0x7FDE }, // Lonely, 31/30/30/30/30/31
    { 0x49099F73, 0x7FFE, 0x7FFF }, // Hasty, 30/31/31/31/31/31
    { 0x4AE61B86, 0x7BFF, 0x7BDF }, // Timid, 31/31/30/31/30/30
    { 0x4B17FE2E, 0x7BFE, 0x7FDE }, // Careful, 30/31/30/30/30/31
    { 0x4B92676E, 0x7BDF, 0x7BFF }, // Lax, 31/30/30/31/31/30
    { 0x4BC34A16, 0x7BDE, 0x7FFF }, // Hasty, 30/30/30/31/31/31
    { 0x4BFA728C, 0x7FDE, 0x7BFE }, // Quirky, 30/30/31/30/31/30
    { 0x4D8FACFE, 0x7BFF, 0x7FFE }, // Mild, 31/31/30/30/31/31
    { 0x4DB6B683, 0x7FDE, 0x7BDF }, // Relaxed, 30/30/31/31/30/30
    { 0x4F4AF0F5, 0x7BFF, 0x7FDF }, // Jolly, 31/31/30/31/30/31
    { 0x4F81196B, 0x7FFF, 0x7BDE }, // Modest, 31/31/31/30/30/30
    { 0x502D6553, 0x7FDF, 0x7BFE }, // Naive, 31/30/31/30/31/30
    { 0x505E47FB, 0x7FDF, 0x7FFE }, // Mild, 31/30/31/30/31/31
    { 0x51490340, 0x7BFE, 0x7BFF }, // Docile, 30/31/30/31/31/30
    { 0x521A8BF3, 0x7FDF, 0x7FDF }, // Hardy, 31/30/31/31/30/31
    { 0x55E40125, 0x7FFF, 0x7BFE }, // Hasty, 31/31/31/30/31/30
    { 0x5615E3CD, 0x7FFE, 0x7FFE }, // Quirky, 30/31/31/30/31/31
    { 0x57D127C5, 0x7FFE, 0x7FDF }, // Sassy, 30/31/31/31/30/31
    { 0x589EABC7, 0x7BDF, 0x7BFE }, // Gentle, 31/30/30/30/31/30
    { 0x58CF8E6F, 0x7BDE, 0x7FFE }, // Careful, 30/30/30/30/31/31
    { 0x5A48D694, 0x7FFF, 0x7FFF }, // Naive, 31/31/31/31/31/31
    { 0x5A5AEFBF, 0x7BDF, 0x7BDF }, // Bold, 31/30/30/31/30/30
    { 0x5A8BD267, 0x7BDE, 0x7FDF }, // Relaxed, 30/30/30/31/30/31
    { 0x5AC2FADD, 0x7FDE, 0x7BDE }, // Calm, 30/30/31/30/30/30
    { 0x5C56354F, 0x7BFF, 0x7FDE }, // Modest, 31/31/30/30/30/31
    { 0x5E554799, 0x7BFE, 0x7BFE }, // Bashful, 30/31/30/30/31/30
    { 0x5F26D04C, 0x7FDF, 0x7FDE }, // Serious, 31/30/31/30/30/31
    { 0x60108B91, 0x7BFE, 0x7BDF }, // Mild, 30/31/30/31/30/30
    { 0x62B91D08, 0x7BFF, 0x7FFE }, // Timid, 31/31/30/30/31/31
    { 0x64AB8976, 0x7FFF, 0x7BDF }, // Gentle, 31/31/31/31/30/30
    { 0x64DD6C1E, 0x7FFE, 0x7FDE }, // Lax, 30/31/31/30/30/31
    { 0x6557D55E, 0x7FDF, 0x7BFF }, // Calm, 31/30/31/31/31/30
    { 0x6589B806, 0x7FDE, 0x7FFF }, // Impish, 30/30/31/31/31/31
    { 0x67663419, 0x7BDF, 0x7BDE }, // Relaxed, 31/30/30/30/30/30
    { 0x679716C0, 0x7BDE, 0x7FDE }, // Impish, 30/30/30/30/30/31
    { 0x6B0E7130, 0x7FFE, 0x7BFF }, // Quiet, 30/31/31/31/31/30
    { 0x6D1CCFEB, 0x7BFE, 0x7BDE }, // Naughty, 30/31/30/30/30/30
    { 0x6DC81BD2, 0x7BDF, 0x7BFF }, // Mild, 31/30/30/31/31/30
    { 0x6DF9FE7A, 0x7BDE, 0x7FFE }, // Naughty, 30/30/30/30/31/31
    { 0x6F72469F, 0x7FFF, 0x7FFF }, // Lax, 31/31/31/31/31/31
    { 0x6FEC6AE8, 0x7FDE, 0x7BDF }, // Modest, 30/30/31/31/30/30
    { 0x7180A55A, 0x7BFF, 0x7FDE }, // Gentle, 31/31/30/30/30/31
    { 0x722DF141, 0x7BDF, 0x7FFF }, // Bold, 31/30/30/31/31/31
    { 0x726319B8, 0x7FDF, 0x7BFE }, // Sassy, 31/30/31/30/31/30
    { 0x737FB7A4, 0x7BFE, 0x7BFF }, // Quirky, 30/31/30/31/31/30
    { 0x741F5DAF, 0x7FDF, 0x7BDF }, // Bold, 31/30/31/31/30/30
    { 0x74504057, 0x7FDE, 0x7FDF }, // Relaxed, 30/30/31/31/30/31
    { 0x77E38D13, 0x7BFF, 0x7FFF }, // Mild, 31/31/30/31/31/31
    { 0x781AB58A, 0x7FFE, 0x7BFE }, // Bold, 30/31/31/30/31/30
    { 0x79D6F981, 0x7FFE, 0x7BDF }, // Jolly, 30/31/31/31/30/30
    { 0x7A07DC29, 0x7FFE, 0x7FDF }, // Modest, 30/31/31/31/30/31
    { 0x7AB32811, 0x7FDE, 0x7FFF }, // Adamant, 30/30/31/31/31/31
    { 0x7AD4602C, 0x7BDE, 0x7BFE }, // Naughty, 30/30/30/30/31/30
    { 0x7C7E8AF9, 0x7FFF, 0x7FFE }, // Sassy, 31/31/31/30/31/31
    { 0x7C90A424, 0x7BDF, 0x7BDF }, // Jolly, 31/30/30/31/30/30
    { 0x7CC186CB, 0x7BDE, 0x7FDE }, // Naive, 30/30/30/30/30/31
    { 0x7E3ACEF0, 0x7FFF, 0x7FDF }, // Bold, 31/31/31/31/30/31
    { 0x7F38359B, 0x7BDF, 0x7FFE }, // Gentle, 31/30/30/30/31/31
    { 0x808BFBFE, 0x7BFE, 0x7BFE }, // Serious, 30/31/30/30/31/30
    { 0x80F47993, 0x7BDF, 0x7FDF }, // Bold, 31/30/30/31/30/31
    { 0x812BA209, 0x7FDF, 0x7BDE }, // Bashful, 31/30/31/30/30/30
    { 0x815C84B1, 0x7FDE, 0x7FDE }, // Calm, 30/30/31/30/30/31
    { 0x82463FF5, 0x7BFE, 0x7BDF }, // Careful, 30/31/30/31/30/30
    { 0x84BEEEC5, 0x7BFF, 0x7BFE }, // Brave, 31/31/30/30/31/30
    { 0x84EFD16D, 0x7BFF, 0x7FFE }, // Naughty, 31/31/30/30/31/31
    { 0x86AB1565, 0x7BFF, 0x7FDF }, // Brave, 31/31/30/31/30/31
    { 0x86E13DDB, 0x7FFE, 0x7BDE }, // Naughty, 30/31/31/30/30/30
    { 0x87132083, 0x7FFE, 0x7FDE }, // Quiet, 30/31/31/30/30/31
    { 0x878E89C3, 0x7FDF, 0x7BFF }, // Naive, 31/30/31/31/31/30
    { 0x87BF6C6A, 0x7FDE, 0x7FFE }, // Modest, 30/30/31/30/31/31
    { 0x899CE87D, 0x7BDF, 0x7BDE }, // Hardy, 31/30/30/30/30/30
    { 0x8B46134A, 0x7FFF, 0x7FDE }, // Relaxed, 31/31/31/30/30/31
    { 0x8BF25F32, 0x7FDF, 0x7FFF }, // Docile, 31/30/31/31/31/31
    { 0x8D442595, 0x7FFE, 0x7BFF }, // Hardy, 30/31/31/31/31/30
    { 0x8E00BDEC, 0x7BDF, 0x7FDE }, // Quiet, 31/30/30/30/30/31
    { 0x8F52844F, 0x7BFE, 0x7BDE }, // Hasty, 30/31/30/30/30/30
    { 0x8FFED037, 0x7BDE, 0x7BFE }, // Timid, 30/30/30/30/31/30
    { 0x91A9FB04, 0x7FFF, 0x7FFF }, // Naive, 31/31/31/31/31/31
    { 0x93857716, 0x7BFF, 0x7BDE }, // Lonely, 31/31/30/30/30/30
    { 0x93B759BE, 0x7BFF, 0x7FDE }, // Naive, 31/31/30/30/30/31
    { 0x9463A5A6, 0x7BDF, 0x7FFF }, // Jolly, 31/30/30/31/31/31
    { 0x96551214, 0x7FDF, 0x7BDF }, // Jolly, 31/30/31/31/30/30
    { 0x9686F4BC, 0x7FDE, 0x7FDE }, // Lonely, 30/30/31/30/30/31
    { 0x98FEA38B, 0x7FDF, 0x7FFE }, // Bashful, 31/30/31/30/31/31
    { 0x99E85ED0, 0x7BFF, 0x7BFF }, // Sassy, 31/31/30/31/31/30
    { 0x9A194178, 0x7BFE, 0x7FFF }, // Quirky, 30/31/30/31/31/31
    { 0x9C0CADE6, 0x7FFE, 0x7BDF }, // Gentle, 30/31/31/31/30/30
    { 0x9CB8F9CE, 0x7FDE, 0x7BFF }, // Calm, 30/30/31/31/31/30
    { 0x9E835CB6, 0x7FFF, 0x7BFE }, // Adamant, 31/31/31/30/31/30
    { 0x9EB43F5D, 0x7FFF, 0x7FFE }, // Naughty, 31/31/31/30/31/31
    { 0x9EC65888, 0x7BDE, 0x7BDE }, // Calm, 30/30/30/30/30/30
    { 0xA0708355, 0x7FFF, 0x7FDF }, // Jolly, 31/31/31/31/30/31
    { 0xA26F959F, 0x7FFE, 0x7BFF }, // Mild, 30/31/31/31/31/30
    { 0xA32A2DF7, 0x7BDF, 0x7FDF }, // Serious, 31/30/30/31/30/31
    { 0xA361566E, 0x7FDF, 0x7BDE }, // Lonely, 31/30/31/30/30/30
    { 0xA5294042, 0x7BDE, 0x7BFF }, // Mild, 30/30/30/31/31/30
    { 0xA6F4A32A, 0x7BFF, 0x7BFE }, // Timid, 31/31/30/30/31/30
    { 0xA8B0E721, 0x7BFF, 0x7BDF }, // Bashful, 31/31/30/31/30/30
    { 0xA8E1C9C9, 0x7BFE, 0x7FDF }, // Calm, 30/31/30/31/30/31
    { 0xA918F23F, 0x7FFE, 0x7BDE }, // Impish, 30/31/31/30/30/30
    { 0xA98D15B1, 0x7BDF, 0x7FFF }, // Impish, 31/30/30/31/31/31
    { 0xA9C43E27, 0x7FDE, 0x7BFE }, // Gentle, 30/30/31/30/31/30
    { 0xABB164C7, 0x7FDE, 0x7FDF }, // Relaxed, 30/30/31/31/30/31
    { 0xAD7CC7AF, 0x7FFF, 0x7FDE }, // Lonely, 31/31/31/30/30/31
    { 0xAE281396, 0x7FDF, 0x7FFE }, // Jolly, 31/30/31/30/31/31
    { 0xAF44B183, 0x7BFE, 0x7FFF }, // Mild, 30/31/30/31/31/31
    { 0xAF7AD9F9, 0x7FFE, 0x7BFE }, // Bashful, 30/31/31/30/31/30
    { 0xB235849B, 0x7BDE, 0x7BFE }, // Adamant, 30/30/30/30/31/30
    { 0xB3ADCCC0, 0x7FFF, 0x7BFF }, // Impish, 31/31/31/31/31/30
    { 0xB3DFAF68, 0x7FFE, 0x7FFE }, // Gentle, 30/31/31/30/31/31
    { 0xB3F0C893, 0x7BDE, 0x7BDF }, // Lonely, 30/30/30/31/30/30
    { 0xB5BC2B7B, 0x7BFF, 0x7BDE }, // Calm, 31/31/30/30/30/30
    { 0xB6687763, 0x7BDF, 0x7BFF }, // Rash, 31/30/30/31/31/30
    { 0xB6995A0B, 0x7BDE, 0x7FFE }, // Gentle, 30/30/30/30/31/31
    { 0xB8559E02, 0x7BDF, 0x7FDF }, // Naughty, 31/30/30/31/30/31
    { 0xB88BC678, 0x7FDE, 0x7BDE }, // Docile, 30/30/31/30/30/30
    { 0xB8BDA920, 0x7FDE, 0x7FDE }, // Rash, 30/30/31/30/30/31
    { 0xBC1E1335, 0x7BFF, 0x7BFF }, // Bold, 31/31/30/31/31/30
    { 0xBC50F5DD, 0x7BFE, 0x7FFE }, // Naughty, 30/31/30/30/31/31
    { 0xBCF09BE8, 0x7FDF, 0x7FDF }, // Timid, 31/30/31/31/30/31
    { 0xBE0B39D4, 0x7BFE, 0x7FDF }, // Modest, 30/31/30/31/30/31
    { 0xBE42624A, 0x7FFE, 0x7BDE }, // Adamant, 30/31/31/30/30/30
    { 0xBEEEAE32, 0x7FDE, 0x7BFF }, // Brave, 30/30/31/31/31/30
    { 0xC083E8A4, 0x7BFF, 0x7FFF }, // Rash, 31/31/30/31/31/31
    { 0xC0B9111A, 0x7FFF, 0x7BFE }, // Timid, 31/31/31/30/31/30
    { 0xC0FC0CED, 0x7BDE, 0x7BDE }, // Adamant, 30/30/30/30/30/30
    { 0xC2755512, 0x7FFF, 0x7BDF }, // Rash, 31/31/31/31/30/30
    { 0xC2A637BA, 0x7FFE, 0x7FDF }, // Gentle, 30/31/31/31/30/31
    { 0xC35283A1, 0x7FDF, 0x7FFF }, // Rash, 31/30/31/31/31/31
    { 0xC52FFFB4, 0x7BDF, 0x7BDF }, // Naughty, 31/30/30/31/30/30
    { 0xC561E25C, 0x7BDF, 0x7FDE }, // Quiet, 31/30/30/30/30/31
    { 0xC9091F73, 0x7FFE, 0x7FFF }, // Mild, 30/31/31/31/31/31
    { 0xCAE69B86, 0x7BFF, 0x7BDF }, // Lonely, 31/31/30/31/30/30
    { 0xCB177E2E, 0x7BFE, 0x7FDE }, // Adamant, 30/31/30/30/30/31
    { 0xCB92E76E, 0x7BDF, 0x7BFF }, // Hardy, 31/30/30/31/31/30
    { 0xCBC3CA16, 0x7BDE, 0x7FFF }, // Brave, 30/30/30/31/31/31
    { 0xCBFAF28C, 0x7FDE, 0x7BFE }, // Modest, 30/30/31/30/31/30
    { 0xCD8F2CFE, 0x7BFF, 0x7FFE }, // Gentle, 31/31/30/30/31/31
    { 0xCDB63683, 0x7FDE, 0x7BDF }, // Serious, 30/30/31/31/30/30
    { 0xCF4A70F5, 0x7BFF, 0x7FDF }, // Bashful, 31/31/30/31/30/31
    { 0xCF81996B, 0x7FFF, 0x7BDE }, // Docile, 31/31/31/30/30/30
    { 0xD02DE553, 0x7FDF, 0x7BFE }, // Bold, 31/30/31/30/31/30
    { 0xD05EC7FB, 0x7FDF, 0x7FFE }, // Relaxed, 31/30/31/30/31/31
    { 0xD1498340, 0x7BFE, 0x7BFF }, // Sassy, 30/31/30/31/31/30
    { 0xD21A0BF3, 0x7FDF, 0x7FDF }, // Bold, 31/30/31/31/30/31
    { 0xD5E48125, 0x7FFF, 0x7BFE }, // Brave, 31/31/31/30/31/30
    { 0xD61563CD, 0x7FFE, 0x7FFE }, // Naughty, 30/31/31/30/31/31
    { 0xD7D1A7C5, 0x7FFE, 0x7FDF }, // Jolly, 30/31/31/31/30/31
    { 0xD89E2BC7, 0x7BDF, 0x7BFE }, // Lonely, 31/30/30/30/31/30
    { 0xD8CF0E6F, 0x7BDE, 0x7FFE }, // Adamant, 30/30/30/30/31/31
    { 0xDA485694, 0x7FFF, 0x7FFF }, // Rash, 31/31/31/31/31/31
    { 0xDA5A6FBF, 0x7BDF, 0x7BDF }, // Timid, 31/30/30/31/30/30
    { 0xDA8B5267, 0x7BDE, 0x7FDF }, // Serious, 30/30/30/31/30/31
    { 0xDAC27ADD, 0x7FDE, 0x7BDE }, // Hardy, 30/30/31/30/30/30
    { 0xDC56B54F, 0x7BFF, 0x7FDE }, // Docile, 31/31/30/30/30/31
    { 0xDE55C799, 0x7BFE, 0x7BFE }, // Lax, 30/31/30/30/31/30
    { 0xDF26504C, 0x7FDF, 0x7FDE }, // Quiet, 31/30/31/30/30/31
    { 0xE0100B91, 0x7BFE, 0x7BDF }, // Gentle, 30/31/30/31/30/30
    { 0xE2B99D08, 0x7BFF, 0x7FFE }, // Lonely, 31/31/30/30/31/31
    { 0xE4AB0976, 0x7FFF, 0x7BDF }, // Lonely, 31/31/31/31/30/30
    { 0xE4DDEC1E, 0x7FFE, 0x7FDE }, // Hardy, 30/31/31/30/30/31
    { 0xE557555E, 0x7FDF, 0x7BFF }, // Hardy, 31/30/31/31/31/30
    { 0xE5893806, 0x7FDE, 0x7FFF }, // Jolly, 30/30/31/31/31/31
    { 0xE766B419, 0x7BDF, 0x7BDE }, // Careful, 31/30/30/30/30/30
    { 0xE79796C0, 0x7BDE, 0x7FDE }, // Quirky, 30/30/30/30/30/31
    { 0xEB0EF130, 0x7FFE, 0x7BFF }, // Impish, 30/31/31/31/31/30
    { 0xED1C4FEB, 0x7BFE, 0x7BDE }, // Lax, 30/31/30/30/30/30
    { 0xEDC89BD2, 0x7BDF, 0x7BFF }, // Relaxed, 31/30/30/31/31/30
    { 0xEDF97E7A, 0x7BDE, 0x7FFE }, // Lax, 30/30/30/30/31/31
    { 0xEF72C69F, 0x7FFF, 0x7FFF }, // Hardy, 31/31/31/31/31/31
    { 0xEFECEAE8, 0x7FDE, 0x7BDF }, // Docile, 30/30/31/31/30/30
    { 0xF180255A, 0x7BFF, 0x7FDE }, // Lonely, 31/31/30/30/30/31
    { 0xF22D7141, 0x7BDF, 0x7FFF }, // Timid, 31/30/30/31/31/31
    { 0xF26399B8, 0x7FDF, 0x7BFE }, // Jolly, 31/30/31/30/31/30
    { 0xF37F37A4, 0x7BFE, 0x7BFF }, // Naughty, 30/31/30/31/31/30
    { 0xF41FDDAF, 0x7FDF, 0x7BDF }, // Gentle, 31/30/31/31/30/30
    { 0xF450C057, 0x7FDE, 0x7FDF }, // Careful, 30/30/31/31/30/31
    { 0xF7E30D13, 0x7BFF, 0x7FFF }, // Gentle, 31/31/30/31/31/31
    { 0xF81A358A, 0x7FFE, 0x7BFE }, // Timid, 30/31/31/30/31/30
    { 0xF9D67981, 0x7FFE, 0x7BDF }, // Bashful, 30/31/31/31/30/30
    { 0xFA075C29, 0x7FFE, 0x7FDF }, // Calm, 30/31/31/31/30/31
    { 0xFAB3A811, 0x7FDE, 0x7FFF }, // Rash, 30/30/31/31/31/31
    { 0xFAD4E02C, 0x7BDE, 0x7BFE }, // Calm, 30/30/30/30/31/30
    { 0xFC7E0AF9, 0x7FFF, 0x7FFE }, // Brave, 31/31/31/30/31/31
    { 0xFC902424, 0x7BDF, 0x7BDF }, // Bashful, 31/30/30/31/30/30
    { 0xFCC106CB, 0x7BDE, 0x7FDE }, // Rash, 30/30/30/30/30/31
    { 0xFE3A4EF0, 0x7FFF, 0x7FDF }, // Timid, 31/31/31/31/30/31
    { 0xFF38B59B, 0x7BDF, 0x7FFE }, // Serious, 31/30/30/30/31/31
    { 0x01214FC5, 0x7C3F, 0x7FDF }, // Hardy, 31/1/31/31/30/31 [NPC]
    { 0x031F620F, 0x7C3E, 0x7BFF }, // Quiet, 30/1/31/31/31/30 [NPC]
    { 0x03DBFA67, 0x781F, 0x7FDF }, // Timid, 31/0/30/31/30/31 [NPC]
    { 0x041222DD, 0x7C1F, 0x7BDE }, // Serious, 31/0/31/30/30/30 [NPC]
    { 0x052DC0CA, 0x783E, 0x7BDE }, // Naughty, 30/1/30/30/30/30 [NPC]
    { 0x05D90CB2, 0x781E, 0x7BFF }, // Quiet, 30/0/30/31/31/30 [NPC]
    { 0x07A56F9A, 0x783F, 0x7BFE }, // Sassy, 31/1/30/30/31/30 [NPC]
    { 0x0960B391, 0x783F, 0x7BDF }, // Rash, 31/1/30/31/30/30 [NPC]
    { 0x09929639, 0x783F, 0x7FDF }, // Relaxed, 31/1/30/31/30/31 [NPC]
    { 0x09C8BEAF, 0x7C3E, 0x7BDE }, // Lax, 30/1/31/30/30/30 [NPC]
    { 0x0A3EE221, 0x781F, 0x7FFF }, // Docile, 31/0/30/31/31/31 [NPC]
    { 0x0A740A97, 0x7C1F, 0x7BFE }, // Sassy, 31/0/31/30/31/30 [NPC]
    { 0x0AA6ED3F, 0x7C1E, 0x7FFE }, // Gentle, 30/0/31/30/31/31 [NPC]
    { 0x0C613137, 0x7C1E, 0x7FDF }, // Impish, 30/0/31/31/30/31 [NPC]
    { 0x0E2D941F, 0x7C3F, 0x7FDE }, // Jolly, 31/1/31/30/30/31 [NPC]
    { 0x0ED9E006, 0x7C1F, 0x7FFE }, // Hasty, 31/0/31/30/31/31 [NPC]
    { 0x0FF47DF3, 0x783E, 0x7FFF }, // Quiet, 30/1/30/31/31/31 [NPC]
    { 0x102BA669, 0x7C3E, 0x7BFE }, // Bold, 30/1/31/30/31/30 [NPC]
    { 0x12E5510B, 0x781E, 0x7BFE }, // Naughty, 30/0/30/30/31/30 [NPC]
    { 0x145E9930, 0x7C3F, 0x7BFF }, // Calm, 31/1/31/31/31/30 [NPC]
    { 0x148F7BD8, 0x7C3F, 0x7FFE }, // Sassy, 31/1/31/30/31/31 [NPC]
    { 0x14A19503, 0x781E, 0x7BDF }, // Jolly, 30/0/30/31/30/30 [NPC]
    { 0x166CF7EB, 0x783F, 0x7BDE }, // Relaxed, 31/1/30/30/30/30 [NPC]
    { 0x174A267B, 0x781F, 0x7FFE }, // Impish, 31/0/30/30/31/31 [NPC]
    { 0x19056A72, 0x781F, 0x7FDF }, // Bold, 31/0/30/31/30/31 [NPC]
    { 0x193C92E8, 0x7C1F, 0x7BDE }, // Bashful, 31/0/31/30/30/30 [NPC]
    { 0x196D7590, 0x7C1E, 0x7FDE }, // Calm, 30/0/31/30/30/31 [NPC]
    { 0x1CCFDFA5, 0x783F, 0x7BFF }, // Adamant, 31/1/30/31/31/30 [NPC]
    { 0x1D00C24D, 0x783E, 0x7FFE }, // Bold, 30/1/30/30/31/31 [NPC]
    { 0x1DA06858, 0x7C1F, 0x7FDF }, // Hasty, 31/0/31/31/30/31 [NPC]
    { 0x1EBC0644, 0x783E, 0x7FDF }, // Brave, 30/1/30/31/30/31 [NPC]
    { 0x1EF32EBA, 0x7C3E, 0x7BDE }, // Modest, 30/1/31/30/30/30 [NPC]
    { 0x1F9F7AA2, 0x7C1E, 0x7BFF }, // Naive, 30/0/31/31/31/30 [NPC]
    { 0x216ADD8A, 0x7C3F, 0x7BFE }, // Impish, 31/1/31/30/31/30 [NPC]
    { 0x21ADD95D, 0x781E, 0x7BDE }, // Lonely, 30/0/30/30/30/30 [NPC]
    { 0x2357042A, 0x7C3F, 0x7FDE }, // Impish, 31/1/31/30/30/31 [NPC]
    { 0x24035011, 0x7C1F, 0x7FFF }, // Docile, 31/0/31/31/31/31 [NPC]
    { 0x2611AECC, 0x781F, 0x7FDE }, // Bashful, 31/0/30/30/30/31 [NPC]
    { 0x2810C116, 0x781E, 0x7BFF }, // Gentle, 30/0/30/31/31/30 [NPC]
    { 0x29BAEBE3, 0x7C3E, 0x7FFF }, // Naive, 30/1/31/31/31/31 [NPC]
    { 0x2B9767F6, 0x783F, 0x7BDF }, // Jolly, 31/1/30/31/30/30 [NPC]
    { 0x2BC84A9E, 0x783E, 0x7FDE }, // Modest, 30/1/30/30/30/31 [NPC]
    { 0x2C43B3DE, 0x781F, 0x7BFF }, // Serious, 31/0/30/31/31/30 [NPC]
    { 0x2C749686, 0x781E, 0x7FFF }, // Naive, 30/0/30/31/31/31 [NPC]
    { 0x2CABBEFC, 0x7C1E, 0x7BFE }, // Brave, 30/0/31/30/31/30 [NPC]
    { 0x2E6602F3, 0x7C1E, 0x7BDF }, // Jolly, 30/0/31/31/30/30 [NPC]
    { 0x2E98E59B, 0x7C1E, 0x7FDF }, // Serious, 30/0/31/31/30/31 [NPC]
    { 0x310F946B, 0x7C1F, 0x7FFE }, // Rash, 31/0/31/30/31/31 [NPC]
    { 0x31F94FB0, 0x783E, 0x7BFF }, // Careful, 30/1/30/31/31/30 [NPC]
    { 0x322B3257, 0x783E, 0x7FFF }, // Timid, 30/1/30/31/31/31 [NPC]
    { 0x32CBD863, 0x7C1F, 0x7FDF }, // Adamant, 31/0/31/31/30/31 [NPC]
    { 0x341D9EC5, 0x7C3E, 0x7BDF }, // Gentle, 30/1/31/31/30/30 [NPC]
    { 0x36944D95, 0x7C3F, 0x7BFE }, // Adamant, 31/1/31/30/31/30 [NPC]
    { 0x36C6303D, 0x7C3E, 0x7FFE }, // Mild, 30/1/31/30/31/31 [NPC]
    { 0x36D74968, 0x781E, 0x7BDF }, // Gentle, 30/0/30/31/30/30 [NPC]
    { 0x38817435, 0x7C3E, 0x7FDF }, // Naive, 30/1/31/31/30/31 [NPC]
    { 0x394FF837, 0x781F, 0x7BFE }, // Quirky, 31/0/30/30/31/30 [NPC]
    { 0x3980DADF, 0x781E, 0x7FFE }, // Lonely, 30/0/30/30/31/31 [NPC]
    { 0x3B0A3C2F, 0x781F, 0x7BDF }, // Hasty, 31/0/30/31/30/30 [NPC]
    { 0x3B3C1ED7, 0x781F, 0x7FDF }, // Quirky, 31/0/30/31/30/31 [NPC]
    { 0x3B72474D, 0x7C1E, 0x7BDE }, // Lonely, 30/0/31/30/30/30 [NPC]
    { 0x3F059409, 0x783E, 0x7BFE }, // Timid, 30/1/30/30/31/30 [NPC]
    { 0x3F3776B1, 0x783E, 0x7FFE }, // Careful, 30/1/30/30/31/31 [NPC]
    { 0x3FD71CBC, 0x7C1F, 0x7FDE }, // Naughty, 31/0/31/30/30/31 [NPC]
    { 0x40C1D801, 0x783F, 0x7BDF }, // Rash, 31/1/30/31/30/30 [NPC]
    { 0x40F2BAA9, 0x783E, 0x7FDF }, // Gentle, 30/1/30/31/30/31 [NPC]
    { 0x4129E31F, 0x7C3E, 0x7BDE }, // Lax, 30/1/31/30/30/30 [NPC]
    { 0x41D52F07, 0x7C1E, 0x7BFE }, // Sassy, 30/0/31/30/31/30 [NPC]
    { 0x436A6978, 0x783F, 0x7FFE }, // Jolly, 31/1/30/30/31/31 [NPC]
    { 0x455CD5E6, 0x7C3F, 0x7BDE }, // Quirky, 31/1/31/30/30/30 [NPC]
    { 0x458DB88E, 0x7C3E, 0x7FDE }, // Lonely, 30/1/31/30/30/31 [NPC]
    { 0x460821CE, 0x7C1F, 0x7BFF }, // Serious, 31/0/31/31/31/30 [NPC]
    { 0x46390476, 0x7C1F, 0x7FFF }, // Naive, 31/0/31/31/31/31 [NPC]
    { 0x48168089, 0x781F, 0x7BDE }, // Quirky, 31/0/30/30/30/30 [NPC]
    { 0x48476330, 0x781F, 0x7FDE }, // Hardy, 31/0/30/30/30/31 [NPC]
    { 0x4BBFBDA0, 0x7C3F, 0x7BFF }, // Calm, 31/1/31/31/31/30 [NPC]
    { 0x4BF0A048, 0x7C3E, 0x7FFF }, // Sassy, 30/1/31/31/31/31 [NPC]
    { 0x4DCD1C5A, 0x783F, 0x7BDE }, // Calm, 31/1/30/30/30/30 [NPC]
    { 0x4DFEFF02, 0x783E, 0x7FDE }, // Impish, 30/1/30/30/30/31 [NPC]
    { 0x4E796842, 0x781F, 0x7BFF }, // Rash, 31/0/30/31/31/30 [NPC]
    { 0x4EAA4AEA, 0x781E, 0x7FFE }, // Gentle, 30/0/30/30/31/31 [NPC]
    { 0x5023930F, 0x7C3F, 0x7FFF }, // Serious, 31/1/31/31/31/31 [NPC]
    { 0x509DB758, 0x7C1E, 0x7BDF }, // Bashful, 30/0/31/31/30/30 [NPC]
    { 0x5231F1CA, 0x783F, 0x7FDE }, // Quirky, 31/1/30/30/30/31 [NPC]
    { 0x53146628, 0x7C1F, 0x7BFE }, // Hardy, 31/0/31/30/31/30 [NPC]
    { 0x54300414, 0x783E, 0x7BFF }, // Mild, 30/1/30/31/31/30 [NPC]
    { 0x55018CC7, 0x7C1F, 0x7FDF }, // Timid, 31/0/31/31/30/31 [NPC]
    { 0x5894D983, 0x783F, 0x7FFF }, // Rash, 31/1/30/31/31/31 [NPC]
    { 0x58CB01FA, 0x7C3F, 0x7BFE }, // Sassy, 31/1/31/30/31/30 [NPC]
    { 0x5A8645F1, 0x7C3F, 0x7BDF }, // Rash, 31/1/31/31/30/30 [NPC]
    { 0x5AB82899, 0x7C3E, 0x7FDF }, // Relaxed, 30/1/31/31/30/31 [NPC]
    { 0x5B647481, 0x7C1E, 0x7FFF }, // Docile, 30/0/31/31/31/31 [NPC]
    { 0x5B85AC9C, 0x781F, 0x7BFE }, // Relaxed, 31/0/30/30/31/30 [NPC]
    { 0x5D2FD769, 0x7C3F, 0x7FFE }, // Hardy, 31/1/31/30/31/31 [NPC]
    { 0x5D41F093, 0x781F, 0x7BDF }, // Modest, 31/0/30/31/30/30 [NPC]
    { 0x5D72D33B, 0x781E, 0x7FDE }, // Quiet, 30/0/30/30/30/31 [NPC]
    { 0x613B486E, 0x783E, 0x7BFE }, // Bashful, 30/1/30/30/31/30 [NPC]
    { 0x61DCEE79, 0x7C1F, 0x7BDE }, // Gentle, 31/0/31/30/30/30 [NPC]
    { 0x620DD121, 0x7C1F, 0x7FDE }, // Careful, 31/0/31/30/30/31 [NPC]
    { 0x62F78C65, 0x783E, 0x7BDF }, // Lonely, 30/1/30/31/30/30 [NPC]
    { 0x63D5BAF5, 0x781E, 0x7FFF }, // Jolly, 30/0/30/31/31/31 [NPC]
    { 0x65A01DDD, 0x783F, 0x7FFE }, // Gentle, 31/1/30/30/31/31 [NPC]
    { 0x65C72763, 0x7C1E, 0x7BDF }, // Jolly, 30/0/31/31/30/30 [NPC]
    { 0x675C61D5, 0x783F, 0x7FDF }, // Bold, 31/1/30/31/30/31 [NPC]
    { 0x67928A4B, 0x7C3F, 0x7BDE }, // Relaxed, 31/1/31/30/30/30 [NPC]
    { 0x67C36CF3, 0x7C3E, 0x7FDE }, // Lax, 30/1/31/30/30/31 [NPC]
    { 0x683ED633, 0x7C1F, 0x7BFF }, // Docile, 31/0/31/31/31/30 [NPC]
    { 0x6870B8DA, 0x7C1E, 0x7FFE }, // Bashful, 30/0/31/30/31/31 [NPC]
    { 0x6A4C34ED, 0x781F, 0x7BDE }, // Docile, 31/0/30/30/30/30 [NPC]
    { 0x6DF57205, 0x7C3E, 0x7BFF }, // Adamant, 30/1/31/31/31/30 [NPC]
    { 0x7003D0BF, 0x783E, 0x7BDE }, // Naive, 30/1/30/30/30/30 [NPC]
    { 0x70AF1CA7, 0x781E, 0x7BFE }, // Brave, 30/0/30/30/31/30 [NPC]
    { 0x70E0FF4F, 0x781E, 0x7FFE }, // Modest, 30/0/30/30/31/31 [NPC]
    { 0x72594774, 0x7C3F, 0x7FFF }, // Calm, 31/1/31/31/31/31 [NPC]
    { 0x729C4346, 0x781E, 0x7FDF }, // Serious, 30/0/30/31/30/31 [NPC]
    { 0x72D36BBC, 0x7C1E, 0x7BDE }, // Hardy, 30/0/31/30/30/30 [NPC]
    { 0x7467A62E, 0x783F, 0x7FDE }, // Docile, 31/1/30/30/30/31 [NPC]
    { 0x7514F216, 0x781F, 0x7FFF }, // Mild, 31/0/30/31/31/31 [NPC]
    { 0x7666B879, 0x783E, 0x7BFE }, // Timid, 30/1/30/30/31/30 [NPC]
    { 0x77065E84, 0x7C1F, 0x7BDF }, // Mild, 31/0/31/31/30/30 [NPC]
    { 0x7737412C, 0x7C1E, 0x7FDE }, // Bashful, 30/0/31/30/30/31 [NPC]
    { 0x7A99AB40, 0x783F, 0x7BFF }, // Hardy, 31/1/30/31/31/30 [NPC]
    { 0x7ACA8DE8, 0x783E, 0x7FFF }, // Brave, 30/1/30/31/31/31 [NPC]
    { 0x7CBDFA56, 0x7C3E, 0x7BDF }, // Quirky, 30/1/31/31/30/30 [NPC]
    { 0x7CEEDCFE, 0x7C3E, 0x7FDE }, // Lonely, 30/1/31/30/30/31 [NPC]
    { 0x7D69463E, 0x7C1F, 0x7BFF }, // Serious, 31/0/31/31/31/30 [NPC]
    { 0x7D9A28E5, 0x7C1E, 0x7FFF }, // Jolly, 30/0/31/31/31/31 [NPC]
    { 0x7F658BCD, 0x7C3F, 0x7FFE }, // Relaxed, 31/1/31/30/31/31 [NPC]
    { 0x7F77A4F8, 0x781F, 0x7BDE }, // Careful, 31/0/30/30/30/30 [NPC]
    { 0x7FA887A0, 0x781E, 0x7FDE }, // Hardy, 30/0/30/30/30/31 [NPC]
    { 0x8121CFC5, 0x7C3F, 0x7FDF }, // Mild, 31/1/31/31/30/31 [NPC]
    { 0x831FE20F, 0x7C3E, 0x7BFF }, // Impish, 30/1/31/31/31/30 [NPC]
    { 0x83DB7A67, 0x781F, 0x7FDF }, // Modest, 31/0/30/31/30/31 [NPC]
    { 0x8412A2DD, 0x7C1F, 0x7BDE }, // Adamant, 31/0/31/30/30/30 [NPC]
    { 0x852D40CA, 0x783E, 0x7BDE }, // Lax, 30/1/30/30/30/30 [NPC]
    { 0x85D98CB2, 0x781E, 0x7BFF }, // Impish, 30/0/30/31/31/30 [NPC]
    { 0x87A5EF9A, 0x783F, 0x7BFE }, // Jolly, 31/1/30/30/31/30 [NPC]
    { 0x89603391, 0x783F, 0x7BDF }, // Quirky, 31/1/30/31/30/30 [NPC]
    { 0x89921639, 0x783F, 0x7FDF }, // Serious, 31/1/30/31/30/31 [NPC]
    { 0x89C83EAF, 0x7C3E, 0x7BDE }, // Naive, 30/1/31/30/30/30 [NPC]
    { 0x8A3E6221, 0x781F, 0x7FFF }, // Hasty, 31/0/30/31/31/31 [NPC]
    { 0x8A748A97, 0x7C1F, 0x7BFE }, // Jolly, 31/0/31/30/31/30 [NPC]
    { 0x8AA66D3F, 0x7C1E, 0x7FFE }, // Lonely, 30/0/31/30/31/31 [NPC]
    { 0x8C61B137, 0x7C1E, 0x7FDF }, // Quirky, 30/0/31/31/30/31 [NPC]
    { 0x8E2D141F, 0x7C3F, 0x7FDE }, // Bashful, 31/1/31/30/30/31 [NPC]
    { 0x8ED96006, 0x7C1F, 0x7FFE }, // Mild, 31/0/31/30/31/31 [NPC]
    { 0x8FF4FDF3, 0x783E, 0x7FFF }, // Impish, 30/1/30/31/31/31 [NPC]
    { 0x902B2669, 0x7C3E, 0x7BFE }, // Timid, 30/1/31/30/31/30 [NPC]
    { 0x92E5D10B, 0x781E, 0x7BFE }, // Calm, 30/0/30/30/31/30 [NPC]
    { 0x945E1930, 0x7C3F, 0x7BFF }, // Hardy, 31/1/31/31/31/30 [NPC]
    { 0x948FFBD8, 0x7C3F, 0x7FFE }, // Jolly, 31/1/31/30/31/31 [NPC]
    { 0x94A11503, 0x781E, 0x7BDF }, // Bashful, 30/0/30/31/30/30 [NPC]
    { 0x966C77EB, 0x783F, 0x7BDE }, // Serious, 31/1/30/30/30/30 [NPC]
    { 0x974AA67B, 0x781F, 0x7FFE }, // Quirky, 31/0/30/30/31/31 [NPC]
    { 0x9905EA72, 0x781F, 0x7FDF }, // Gentle, 31/0/30/31/30/31 [NPC]
    { 0x993C12E8, 0x7C1F, 0x7BDE }, // Careful, 31/0/31/30/30/30 [NPC]
    { 0x996DF590, 0x7C1E, 0x7FDE }, // Hasty, 30/0/31/30/30/31 [NPC]
    { 0x9CCF5FA5, 0x783F, 0x7BFF }, // Impish, 31/1/30/31/31/30 [NPC]
    { 0x9D00424D, 0x783E, 0x7FFE }, // Timid, 30/1/30/30/31/31 [NPC]
    { 0x9DA0E858, 0x7C1F, 0x7FDF }, // Brave, 31/0/31/31/30/31 [NPC]
    { 0x9EBC8644, 0x783E, 0x7FDF }, // Bashful, 30/1/30/31/30/31 [NPC]
    { 0x9EF3AEBA, 0x7C3E, 0x7BDE }, // Docile, 30/1/31/30/30/30 [NPC]
    { 0x9F9FFAA2, 0x7C1E, 0x7BFF }, // Bold, 30/0/31/31/31/30 [NPC]
    { 0xA16A5D8A, 0x7C3F, 0x7BFE }, // Jolly, 31/1/31/30/31/30 [NPC]
    { 0xA1AD595D, 0x781E, 0x7BDE }, // Docile, 30/0/30/30/30/30 [NPC]
    { 0xA357842A, 0x7C3F, 0x7FDE }, // Quirky, 31/1/31/30/30/31 [NPC]
    { 0xA403D011, 0x7C1F, 0x7FFF }, // Sassy, 31/0/31/31/31/31 [NPC]
    { 0xA6112ECC, 0x781F, 0x7FDE }, // Careful, 31/0/30/30/30/31 [NPC]
    { 0xA8104116, 0x781E, 0x7BFF }, // Lonely, 30/0/30/31/31/30 [NPC]
    { 0xA9BA6BE3, 0x7C3E, 0x7FFF }, // Rash, 30/1/31/31/31/31 [NPC]
    { 0xAB97E7F6, 0x783F, 0x7BDF }, // Naughty, 31/1/30/31/30/30 [NPC]
    { 0xABC8CA9E, 0x783E, 0x7FDE }, // Docile, 30/1/30/30/30/31 [NPC]
    { 0xAC4333DE, 0x781F, 0x7BFF }, // Quiet, 31/0/30/31/31/30 [NPC]
    { 0xAC741686, 0x781E, 0x7FFF }, // Rash, 30/0/30/31/31/31 [NPC]
    { 0xACAB3EFC, 0x7C1E, 0x7BFE }, // Relaxed, 30/0/31/30/31/30 [NPC]
    { 0xAE6682F3, 0x7C1E, 0x7BDF }, // Naughty, 30/0/31/31/30/30 [NPC]
    { 0xAE98659B, 0x7C1E, 0x7FDF }, // Quiet, 30/0/31/31/30/31 [NPC]
    { 0xB10F146B, 0x7C1F, 0x7FFE }, // Quirky, 31/0/31/30/31/31 [NPC]
    { 0xB1F9CFB0, 0x783E, 0x7BFF }, // Naive, 30/1/30/31/31/30 [NPC]
    { 0xB22BB257, 0x783E, 0x7FFF }, // Lonely, 30/1/30/31/31/31 [NPC]
    { 0xB2CB5863, 0x7C1F, 0x7FDF }, // Impish, 31/0/31/31/30/31 [NPC]
    { 0xB41D1EC5, 0x7C3E, 0x7BDF }, // Lonely, 30/1/31/31/30/30 [NPC]
    { 0xB694CD95, 0x7C3F, 0x7BFE }, // Rash, 31/1/31/30/31/30 [NPC]
    { 0xB6C6B03D, 0x7C3E, 0x7FFE }, // Relaxed, 30/1/31/30/31/31 [NPC]
    { 0xB6D7C968, 0x781E, 0x7BDF }, // Serious, 30/0/30/31/30/30 [NPC]
    { 0xB881F435, 0x7C3E, 0x7FDF }, // Bold, 30/1/31/31/30/31 [NPC]
    { 0xB94F7837, 0x781F, 0x7BFE }, // Naughty, 31/0/30/30/31/30 [NPC]
    { 0xB9805ADF, 0x781E, 0x7FFE }, // Docile, 30/0/30/30/31/31 [NPC]
    { 0xBB0ABC2F, 0x781F, 0x7BDF }, // Brave, 31/0/30/31/30/30 [NPC]
    { 0xBB3C9ED7, 0x781F, 0x7FDF }, // Modest, 31/0/30/31/30/31 [NPC]
    { 0xBB72C74D, 0x7C1E, 0x7BDE }, // Quiet, 30/0/31/30/30/30 [NPC]
    { 0xBF051409, 0x783E, 0x7BFE }, // Modest, 30/1/30/30/31/30 [NPC]
    { 0xBF37F6B1, 0x783E, 0x7FFE }, // Naive, 30/1/30/30/31/31 [NPC]
    { 0xBFD79CBC, 0x7C1F, 0x7FDE }, // Calm, 31/0/31/30/30/31 [NPC]
    { 0xC0C15801, 0x783F, 0x7BDF }, // Quirky, 31/1/30/31/30/30 [NPC]
    { 0xC0F23AA9, 0x783E, 0x7FDF }, // Lonely, 30/1/30/31/30/31 [NPC]
    { 0xC129631F, 0x7C3E, 0x7BDE }, // Naive, 30/1/31/30/30/30 [NPC]
    { 0xC1D5AF07, 0x7C1E, 0x7BFE }, // Jolly, 30/0/31/30/31/30 [NPC]
    { 0xC36AE978, 0x783F, 0x7FFE }, // Naughty, 31/1/30/30/31/31 [NPC]
    { 0xC55C55E6, 0x7C3F, 0x7BDE }, // Naughty, 31/1/31/30/30/30 [NPC]
    { 0xC58D388E, 0x7C3E, 0x7FDE }, // Docile, 30/1/31/30/30/31 [NPC]
    { 0xC608A1CE, 0x7C1F, 0x7BFF }, // Adamant, 31/0/31/31/31/30 [NPC]
    { 0xC6398476, 0x7C1F, 0x7FFF }, // Bold, 31/0/31/31/31/31 [NPC]
    { 0xC8160089, 0x781F, 0x7BDE }, // Naughty, 31/0/30/30/30/30 [NPC]
    { 0xC847E330, 0x781F, 0x7FDE }, // Mild, 31/0/30/30/30/31 [NPC]
    { 0xCBBF3DA0, 0x7C3F, 0x7BFF }, // Hardy, 31/1/31/31/31/30 [NPC]
    { 0xCBF02048, 0x7C3E, 0x7FFF }, // Brave, 30/1/31/31/31/31 [NPC]
    { 0xCDCD9C5A, 0x783F, 0x7BDE }, // Hasty, 31/1/30/30/30/30 [NPC]
    { 0xCDFE7F02, 0x783E, 0x7FDE }, // Jolly, 30/1/30/30/30/31 [NPC]
    { 0xCE79E842, 0x781F, 0x7BFF }, // Timid, 31/0/30/31/31/30 [NPC]
    { 0xCEAACAEA, 0x781E, 0x7FFE }, // Serious, 30/0/30/30/31/31 [NPC]
    { 0xD023130F, 0x7C3F, 0x7FFF }, // Quiet, 31/1/31/31/31/31 [NPC]
    { 0xD09D3758, 0x7C1E, 0x7BDF }, // Careful, 30/0/31/31/30/30 [NPC]
    { 0xD23171CA, 0x783F, 0x7FDE }, // Naughty, 31/1/30/30/30/31 [NPC]
    { 0xD314E628, 0x7C1F, 0x7BFE }, // Mild, 31/0/31/30/31/30 [NPC]
    { 0xD4308414, 0x783E, 0x7BFF }, // Relaxed, 30/1/30/31/31/30 [NPC]
    { 0xD5010CC7, 0x7C1F, 0x7FDF }, // Modest, 31/0/31/31/30/31 [NPC]
    { 0xD8945983, 0x783F, 0x7FFF }, // Quirky, 31/1/30/31/31/31 [NPC]
    { 0xD8CB81FA, 0x7C3F, 0x7BFE }, // Jolly, 31/1/31/30/31/30 [NPC]
    { 0xDA86C5F1, 0x7C3F, 0x7BDF }, // Timid, 31/1/31/31/30/30 [NPC]
    { 0xDAB8A899, 0x7C3E, 0x7FDF }, // Careful, 30/1/31/31/30/31 [NPC]
    { 0xDB64F481, 0x7C1E, 0x7FFF }, // Sassy, 30/0/31/31/31/31 [NPC]
    { 0xDB852C9C, 0x781F, 0x7BFE }, // Serious, 31/0/30/30/31/30 [NPC]
    { 0xDD2F5769, 0x7C3F, 0x7FFE }, // Bold, 31/1/31/30/31/31 [NPC]
    { 0xDD417093, 0x781F, 0x7BDF }, // Calm, 31/0/30/31/30/30 [NPC]
    { 0xDD72533B, 0x781E, 0x7FDE }, // Sassy, 30/0/30/30/30/31 [NPC]
    { 0xE13BC86E, 0x783E, 0x7BFE }, // Lax, 30/1/30/30/31/30 [NPC]
    { 0xE1DC6E79, 0x7C1F, 0x7BDE }, // Lonely, 31/0/31/30/30/30 [NPC]
    { 0xE20D5121, 0x7C1F, 0x7FDE }, // Adamant, 31/0/31/30/30/31 [NPC]
    { 0xE2F70C65, 0x783E, 0x7BDF }, // Docile, 30/1/30/31/30/30 [NPC]
    { 0xE3D53AF5, 0x781E, 0x7FFF }, // Bashful, 30/0/30/31/31/31 [NPC]
    { 0xE5A09DDD, 0x783F, 0x7FFE }, // Serious, 31/1/30/30/31/31 [NPC]
    { 0xE5C7A763, 0x7C1E, 0x7BDF }, // Naughty, 30/0/31/31/30/30 [NPC]
    { 0xE75CE1D5, 0x783F, 0x7FDF }, // Gentle, 31/1/30/31/30/31 [NPC]
    { 0xE7920A4B, 0x7C3F, 0x7BDE }, // Serious, 31/1/31/30/30/30 [NPC]
    { 0xE7C3ECF3, 0x7C3E, 0x7FDE }, // Hardy, 30/1/31/30/30/31 [NPC]
    { 0xE83E5633, 0x7C1F, 0x7BFF }, // Hasty, 31/0/31/31/31/30 [NPC]
    { 0xE87038DA, 0x7C1E, 0x7FFE }, // Careful, 30/0/31/30/31/31 [NPC]
    { 0xEA4CB4ED, 0x781F, 0x7BDE }, // Sassy, 31/0/30/30/30/30 [NPC]
    { 0xEDF5F205, 0x7C3E, 0x7BFF }, // Rash, 30/1/31/31/31/30 [NPC]
    { 0xF00350BF, 0x783E, 0x7BDE }, // Rash, 30/1/30/30/30/30 [NPC]
    { 0xF0AF9CA7, 0x781E, 0x7BFE }, // Bashful, 30/0/30/30/31/30 [NPC]
    { 0xF0E07F4F, 0x781E, 0x7FFE }, // Calm, 30/0/30/30/31/31 [NPC]
    { 0xF259C774, 0x7C3F, 0x7FFF }, // Hasty, 31/1/31/31/31/31 [NPC]
    { 0xF29CC346, 0x781E, 0x7FDF }, // Adamant, 30/0/30/31/30/31 [NPC]
    { 0xF2D3EBBC, 0x7C1E, 0x7BDE }, // Mild, 30/0/31/30/30/30 [NPC]
    { 0xF467262E, 0x783F, 0x7FDE }, // Hasty, 31/1/30/30/30/31 [NPC]
    { 0xF5147216, 0x781F, 0x7FFF }, // Gentle, 31/0/30/31/31/31 [NPC]
    { 0xF6663879, 0x783E, 0x7BFE }, // Modest, 30/1/30/30/31/30 [NPC]
    { 0xF706DE84, 0x7C1F, 0x7BDF }, // Relaxed, 31/0/31/31/30/30 [NPC]
    { 0xF737C12C, 0x7C1E, 0x7FDE }, // Lax, 30/0/31/30/30/31 [NPC]
    { 0xFA992B40, 0x783F, 0x7BFF }, // Bold, 31/1/30/31/31/30 [NPC]
    { 0xFACA0DE8, 0x783E, 0x7FFF }, // Relaxed, 30/1/30/31/31/31 [NPC]
    { 0xFCBD7A56, 0x7C3E, 0x7BDF }, // Naughty, 30/1/31/31/30/30 [NPC]
    { 0xFCEE5CFE, 0x7C3E, 0x7FDE }, // Docile, 30/1/31/30/30/31 [NPC]
    { 0xFD69C63E, 0x7C1F, 0x7BFF }, // Adamant, 31/0/31/31/31/30 [NPC]
    { 0xFD9AA8E5, 0x7C1E, 0x7FFF }, // Naughty, 30/0/31/31/31/31 [NPC]
    { 0xFF650BCD, 0x7C3F, 0x7FFE }, // Serious, 31/1/31/30/31/31 [NPC]
    { 0xFF7724F8, 0x781F, 0x7BDE }, // Adamant, 31/0/30/30/30/30 [NPC]
    { 0xFFA807A0, 0x781E, 0x7FDE } // Bold, 30/0/30/30/30/31 [NPC]
};

static const struct IvPool sIvPools[] =
{
    [IV_POOL_WILD] = IV_POOL(sIvPoolWild),
};

// BEGIN GENERATED sLegalIvTables (python tech/override_ivs.py legal)
// For each met data override with an IV pool: the met levels at which each pool pair is legal
// in PKHeX (bit n of a mask = levels[n]; 0 = never legal there).

// SPECIES_DITTO met at MAP_DESERT_UNDERPASS (MAPSEC_DESERT_UNDERPASS, E), IV_POOL_WILD: 698/772 pairs usable, 2222 pair/level combos
// PKHeX slots: Grass slot 0 Lv38-38, Grass slot 2 Lv40-40, Grass slot 4 Lv41-41, Grass slot 7 Lv42-42, Grass slot 9 Lv43-43, Grass slot 11 Lv45-45
static const u8 sLegalLevels_0[] = {38, 40, 41, 42, 43, 45};
static const u32 sLegalMasks_0[] =
{
    0x1D, 0x1F, 0x7, 0x1F, 0x1F, 0x7, 0x7, 0x17,
    0x3, 0x3, 0x9, 0x2, 0x1F, 0xD, 0x13, 0x2,
    0xB, 0x7, 0x4, 0x6, 0x1, 0x2F, 0x7, 0x39,
    0x13, 0x33, 0x1, 0x37, 0xF, 0xB, 0xB, 0xA,
    0x27, 0x0, 0x1F, 0xD, 0x17, 0x2F, 0xF, 0x1,
    0x0, 0x1F, 0x3, 0x1F, 0x0, 0x7, 0x7, 0x1F,
    0x1, 0x7, 0x5, 0x5, 0x1, 0x1F, 0x3, 0x1F,
    0x4, 0xC, 0x1, 0x1F, 0x1F, 0x0, 0x25, 0xF,
    0x25, 0x1A, 0x18, 0x1, 0xD, 0x0, 0xF, 0x2F,
    0x1F, 0x8, 0x2F, 0x0, 0x37, 0x2F, 0x5, 0x21,
    0xF, 0x0, 0x0, 0x0, 0x37, 0x32, 0xB, 0x29,
    0x9, 0x7, 0xF, 0xF, 0x7, 0xF, 0xA, 0xB,
    0x7, 0x17, 0x17, 0xF, 0x6, 0x17, 0x7, 0x1B,
    0x2, 0x6, 0x1D, 0x3F, 0x1, 0x1D, 0x20, 0x13,
    0x0, 0x17, 0x1, 0xE, 0x1D, 0xB, 0x0, 0x4,
    0x19, 0x1F, 0x1D, 0x1, 0x12, 0x1D, 0x1F, 0x1F,
    0x2F, 0xF, 0x1, 0x1F, 0x10, 0xC, 0x4, 0x1,
    0x1F, 0xC, 0x7, 0x17, 0x0, 0x13, 0x0, 0x0,
    0x13, 0x33, 0x26, 0x3, 0x3F, 0xE, 0x1F, 0x6,
    0x1F, 0x1F, 0x25, 0x3, 0x3F, 0x3F, 0x16, 0xD,
    0x2, 0x23, 0x15, 0x0, 0x4, 0xF, 0x13, 0x3F,
    0xA, 0x0, 0x3, 0x0, 0x27, 0x3, 0x3, 0x2,
    0x15, 0x1F, 0xD, 0xD, 0xB, 0xB, 0x1F, 0xF,
    0x1F, 0x1F, 0xF, 0x7, 0x20, 0xD, 0x37, 0xF,
    0x1, 0x2, 0x4, 0x0, 0x5, 0x5, 0x7, 0x0,
    0xF, 0x11, 0x1F, 0x17, 0x1F, 0x0, 0x7, 0x2F,
    0xD, 0x13, 0x13, 0x12, 0x2E, 0x5, 0x1, 0x19,
    0x0, 0x9, 0x7, 0x1B, 0x2F, 0x9, 0x1F, 0x1F,
    0x0, 0x37, 0x1F, 0x17, 0x1D, 0x4, 0x1, 0x12,
    0x5, 0x1, 0xF, 0x0, 0x13, 0x3F, 0x0, 0xF,
    0xB, 0x2F, 0x5, 0x3, 0x1, 0xF, 0xF, 0x5,
    0xF, 0x1, 0x18, 0x7, 0x0, 0x0, 0x1F, 0xC,
    0xF, 0x7, 0xF, 0x0, 0x0, 0x14, 0x10, 0x1,
    0x1, 0x1, 0xD, 0x1, 0x3, 0xF, 0x1F, 0x0,
    0xA, 0xF, 0x3, 0x1F, 0x0, 0x17, 0x15, 0x1,
    0x14, 0x3F, 0x1F, 0x13, 0x5, 0x11, 0xB, 0xB,
    0x13, 0x0, 0xF, 0x15, 0x3F, 0x1F, 0x0, 0x7,
    0x1F, 0x1F, 0x27, 0xF, 0x2, 0x4, 0x1F, 0x1F,
    0x9, 0x1B, 0x19, 0x33, 0x1F, 0xB, 0x1D, 0x7,
    0x5, 0x2, 0x17, 0x17, 0x2, 0x0, 0x3, 0x2D,
    0xE, 0x1F, 0x7, 0x37, 0x7, 0x7, 0x1F, 0xD,
    0x27, 0x2, 0x12, 0x1F, 0x6, 0x1B, 0x1F, 0x0,
    0x3, 0x9, 0x9, 0x1F, 0x7, 0x6, 0x7, 0x0,
    0x3F, 0x5, 0xB, 0x6, 0x1F, 0x17, 0x2, 0x1F,
    0x7, 0x1, 0x0, 0x12, 0x3F, 0x1F, 0x7, 0x0,
    0xF, 0x0, 0x6, 0x4, 0x7, 0x1, 0x1B, 0x3,
    0x1F, 0x10, 0x3, 0x9, 0x2, 0xB, 0x0, 0x37,
    0x1F, 0xF, 0x16, 0x1F, 0x3F, 0x2, 0x1B, 0x3,
    0x1F, 0x17, 0x1F, 0x2F, 0x27, 0x1A, 0x5, 0x27,
    0x7, 0xF, 0x0, 0xD, 0x8, 0xF, 0x0, 0x11,
    0x20, 0x1B, 0x17, 0x1F, 0x1, 0x7, 0x5, 0x0,
    0x7, 0x15, 0x17, 0x0, 0x7, 0x9, 0x1F, 0x2F,
    0xF, 0xE, 0x5, 0x15, 0x4, 0x37, 0x7, 0x6,
    0x3D, 0xF, 0x5, 0x3F, 0xF, 0x37, 0xF, 0x2,
    0x3B, 0x18, 0x4, 0x0, 0x13, 0xF, 0x17, 0x16,
    0x7, 0x3F, 0x1, 0x7, 0xD, 0x7, 0x13, 0xB,
    0x1F, 0xB, 0x17, 0xF, 0x17, 0xB, 0xC, 0x0,
    0x3, 0x0, 0x3F, 0x3F, 0x33, 0xF, 0xF, 0x0,
    0x1D, 0x6, 0x1F, 0x3F, 0xF, 0x17, 0x7, 0x6,
    0x0, 0x1B, 0x7, 0x5, 0x21, 0x7, 0xF, 0x1F,
    0x3F, 0x1, 0xF, 0x3, 0x2E, 0x17, 0x7, 0x3,
    0x1D, 0xB, 0x15, 0x7, 0x1D, 0x3F, 0xD, 0xF,
    0x27, 0x1F, 0x3F, 0xD, 0x1, 0xF, 0x1, 0x1,
    0x0, 0x0, 0x1F, 0x3F, 0x5, 0x3F, 0x2, 0x17,
    0x0, 0x17, 0x17, 0xD, 0x29, 0x2F, 0x5, 0x22,
    0x17, 0x0, 0x1F, 0x1F, 0x2, 0x0, 0x27, 0x1F,
    0x3F, 0x1F, 0x13, 0xB, 0x13, 0x0, 0x3F, 0x1,
    0x3F, 0x5, 0x0, 0x1, 0x3F, 0x17, 0x0, 0x4,
    0x3F, 0x7, 0x5, 0x1B, 0xF, 0x4, 0x1, 0x17,
    0xF, 0x17, 0x1, 0x17, 0x7, 0xB, 0xC, 0x7,
    0x1F, 0x1, 0xA, 0x7, 0x7, 0x6, 0x1F, 0x37,
    0x15, 0x1F, 0x4, 0x3F, 0x0, 0x37, 0x25, 0xF,
    0x9, 0x17, 0x1F, 0x11, 0x1F, 0x2, 0x3, 0xC,
    0x0, 0x5, 0x1, 0x0, 0xF, 0xB, 0x11, 0x2F,
    0x1F, 0xD, 0x3, 0x5, 0x1, 0x7, 0xD, 0xC,
    0x1F, 0x12, 0x3F, 0x7, 0x0, 0x5, 0x1, 0x1,
    0x4, 0x27, 0x3, 0x1F, 0x3, 0x3, 0x5, 0x6,
    0x0, 0x3F, 0xD, 0x1F, 0x3F, 0x1F, 0x17, 0x7,
    0x1, 0xF, 0x1E, 0x19, 0x7, 0x10, 0x0, 0x37,
    0xF, 0x5, 0x1F, 0x5, 0x17, 0x17, 0x5, 0x0,
    0x7, 0x3B, 0x0, 0x0, 0xF, 0x5, 0x1, 0x4,
    0x1, 0x0, 0xF, 0x1, 0x3, 0x3, 0x11, 0x0,
    0xB, 0x1, 0x1, 0x2F, 0x15, 0x0, 0xD, 0xF,
    0x3F, 0x7, 0x12, 0x21, 0x12, 0x9, 0x4, 0xF,
    0x5, 0xB, 0x1F, 0x1, 0x3, 0xF, 0x4, 0xF,
    0x1, 0x2, 0x1D, 0xE, 0x5, 0x3, 0x1, 0x17,
    0x1F, 0x3, 0x3, 0x5, 0x1F, 0x0, 0x7, 0x12,
    0x2, 0x1F, 0x1B, 0x15, 0x17, 0x3, 0x17, 0xF,
    0x3, 0x1, 0x17, 0xD, 0x6, 0x0, 0x5, 0x37,
    0x5, 0x2B, 0x15, 0x1, 0x7, 0x14, 0x1F, 0x25,
    0x2, 0x13, 0x1, 0xF, 0xD, 0x3F, 0x1F, 0x2,
    0x3F, 0x3F, 0xF, 0x20, 0xF, 0x1, 0x0, 0x1F,
    0x0, 0x2F, 0x1F, 0x1, 0x13, 0x0, 0x1, 0x13,
    0x1D, 0x7, 0x4, 0x5, 0x2D, 0x13, 0x1F, 0x3F,
    0xF, 0x1F, 0x1, 0x0, 0x1, 0x1D, 0x9, 0x15,
    0x7, 0x10, 0x20, 0x3, 0x5, 0x3, 0xF, 0x17,
    0xF, 0xD, 0xD, 0x13,
};
STATIC_ASSERT(ARRAY_COUNT(sLegalMasks_0) == ARRAY_COUNT(sIvPoolWild), sLegalMasks_0_MatchesPool)

static const struct LegalIvTable sLegalIvTables[] =
{
    {SPECIES_DITTO, MAP_DESERT_UNDERPASS, IV_POOL_WILD, 0, sLegalLevels_0, ARRAY_COUNT(sLegalLevels_0), sLegalMasks_0},
};
// END GENERATED sLegalIvTables

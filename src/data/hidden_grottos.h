// Hidden Grottos (ported from Black 2 / White 2, see https://github.com/fuddlesworth/pokebw2).
//
// A grotto is an invisible spot on a map: press A while facing it to be asked whether to go in.
// Inside is the grotto room (MAP_HIDDEN_GROTTO, the Shrub1 secret base clearing) holding one
// Pokemon, one item, or nothing. Whatever it holds stays until it is taken. Every new day each
// grotto that is empty (taken, or rolled empty) rolls again:
//
//     roll 0-99:  [0, mon1)                     Pokemon from the location's Pokemon pool 1
//                 [mon1, + mon2)                Pokemon from the location's Pokemon pool 2
//                 [.., + item1)                 item from the location's item pool 1
//                 [.., + item2)                 item from the location's item pool 2
//                 total or higher               empty grotto
//
// So 10/0/30/30 leaves 30% empty (a roll of 70-99). If the total is 100 or more the grotto is
// never empty. If the rolled pool has no entries for the grotto's map, the grotto stays empty.
// The two Pokemon pools let one location have e.g. common low-level and rare evolved Pokemon.
//
// Pokemon moves work like the swarm list in src/tv.c: MOVE_LEVEL takes the newest level-up move
// the Pokemon doesn't already know, any other move is set as is. All four MOVE_LEVEL gives the
// Pokemon its normal level-up moveset. The met location of a Pokemon caught in a grotto is the
// entrance's location, and the music playing on the route keeps playing inside.
//
// Adding a grotto:
//   1. Add its entrance to the region's grotto list. .x/.y are the map coordinates of the tile
//      the player faces (as shown in Porymap), and .mon1Chance etc. its odds in percent (the
//      GROTTO_<REGION>_*_CHANCE defaults below can be used). Each region holds up to
//      HIDDEN_GROTTOS_PER_REGION (20) grottos; each list index is a save slot, so add new
//      grottos at the end of a list to keep the contents of existing saves in place.
//   2. Give the grotto's map entries in that region's Pokemon pools and item pools. Pool entries
//      apply to every grotto on that map, and one is picked at random (up to 32 per map per pool).
//
// `python tech/night_map.py <region>` marks every grotto entrance with a red square.

// ---------------------------------------------------------------- default odds (percent)

#define GROTTO_DEFAULT_MON1_CHANCE   60
#define GROTTO_DEFAULT_MON2_CHANCE   0
#define GROTTO_DEFAULT_ITEM1_CHANCE  20
#define GROTTO_DEFAULT_ITEM2_CHANCE  20

#define GROTTO_KANTO_MON1_CHANCE     GROTTO_DEFAULT_MON1_CHANCE
#define GROTTO_KANTO_MON2_CHANCE     GROTTO_DEFAULT_MON2_CHANCE
#define GROTTO_KANTO_ITEM1_CHANCE    GROTTO_DEFAULT_ITEM1_CHANCE
#define GROTTO_KANTO_ITEM2_CHANCE    GROTTO_DEFAULT_ITEM2_CHANCE

#define GROTTO_JOHTO_MON1_CHANCE     GROTTO_DEFAULT_MON1_CHANCE
#define GROTTO_JOHTO_MON2_CHANCE     GROTTO_DEFAULT_MON2_CHANCE
#define GROTTO_JOHTO_ITEM1_CHANCE    GROTTO_DEFAULT_ITEM1_CHANCE
#define GROTTO_JOHTO_ITEM2_CHANCE    GROTTO_DEFAULT_ITEM2_CHANCE

#define GROTTO_HOENN_MON1_CHANCE     GROTTO_DEFAULT_MON1_CHANCE
#define GROTTO_HOENN_MON2_CHANCE     GROTTO_DEFAULT_MON2_CHANCE
#define GROTTO_HOENN_ITEM1_CHANCE    GROTTO_DEFAULT_ITEM1_CHANCE
#define GROTTO_HOENN_ITEM2_CHANCE    GROTTO_DEFAULT_ITEM2_CHANCE

// ================================================================ Kanto

static const struct HiddenGrotto sHiddenGrottosKanto[] = {
    // {
    //     .location = MAP_NUM(MAP_ROUTE2),
    //     .mapGroup = MAP_GROUP(MAP_ROUTE2),
    //     .x = 12,
    //     .y = 40,
    //     .mon1Chance = GROTTO_KANTO_MON1_CHANCE,
    //     .mon2Chance = GROTTO_KANTO_MON2_CHANCE,
    //     .item1Chance = GROTTO_KANTO_ITEM1_CHANCE,
    //     .item2Chance = GROTTO_KANTO_ITEM2_CHANCE,
    // },
};

static const struct HiddenGrottoMon sHiddenGrottoMons1Kanto[] = {
    // {
    //     .species = SPECIES_PIKACHU,
    //     .moves = {MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL},
    //     .level = 10,
    //     .location = MAP_NUM(MAP_ROUTE2),
    //     .mapGroup = MAP_GROUP(MAP_ROUTE2),
    // },
};

static const struct HiddenGrottoMon sHiddenGrottoMons2Kanto[] = {
};

static const struct HiddenGrottoItem sHiddenGrottoItems1Kanto[] = {
    // {
    //     .item = ITEM_TINY_MUSHROOM,
    //     .location = MAP_NUM(MAP_ROUTE2),
    //     .mapGroup = MAP_GROUP(MAP_ROUTE2),
    // },
};

static const struct HiddenGrottoItem sHiddenGrottoItems2Kanto[] = {
};

// ================================================================ Johto

static const struct HiddenGrotto sHiddenGrottosJohto[] = {
    // {
    //     .location = MAP_NUM(MAP_ROUTE29),
    //     .mapGroup = MAP_GROUP(MAP_ROUTE29),
    //     .x = 20,
    //     .y = 6,
    //     .mon1Chance = GROTTO_JOHTO_MON1_CHANCE,
    //     .mon2Chance = GROTTO_JOHTO_MON2_CHANCE,
    //     .item1Chance = GROTTO_JOHTO_ITEM1_CHANCE,
    //     .item2Chance = GROTTO_JOHTO_ITEM2_CHANCE,
    // },
};

static const struct HiddenGrottoMon sHiddenGrottoMons1Johto[] = {
};

static const struct HiddenGrottoMon sHiddenGrottoMons2Johto[] = {
};

static const struct HiddenGrottoItem sHiddenGrottoItems1Johto[] = {
};

static const struct HiddenGrottoItem sHiddenGrottoItems2Johto[] = {
};

// ================================================================ Hoenn

static const struct HiddenGrotto sHiddenGrottosHoenn[] = {
    {
        .location = MAP_NUM(MAP_ROUTE102),
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
        .x = 12,
        .y = 15,
        .mon1Chance = 40,
        .mon2Chance = 0,
        .item1Chance = 20,
        .item2Chance = 0,
    },
    {
        .location = MAP_NUM(MAP_ROUTE103),
        .mapGroup = MAP_GROUP(MAP_ROUTE103),
        .x = 10,
        .y = 1,
        .mon1Chance = 50,
        .mon2Chance = 0,
        .item1Chance = 50,
        .item2Chance = 0,
    },
    {
        .location = MAP_NUM(MAP_PETALBURG_WOODS),
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
        .x = 26,
        .y = 5,
        .mon1Chance = 30,
        .mon2Chance = 10,
        .item1Chance = 60,
        .item2Chance = 0,
    },
};

static const struct HiddenGrottoMon sHiddenGrottoMons1Hoenn[] = {
    // Route 102
    {
        .species = SPECIES_SURSKIT,
        .moves = {MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL},
        .level = 10,
        .location = MAP_NUM(MAP_ROUTE102),
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
    },
    {
        .species = SPECIES_RALTS,
        .moves = {MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL},
        .level = 10,
        .location = MAP_NUM(MAP_ROUTE102),
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
    },
    // Route 103
    {
        .species = SPECIES_SEEDOT,
        .moves = {MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL},
        .level = 10,
        .location = MAP_NUM(MAP_ROUTE103),
        .mapGroup = MAP_GROUP(MAP_ROUTE103),
    },
    {
        .species = SPECIES_HOUNDOUR,
        .moves = {MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL},
        .level = 10,
        .location = MAP_NUM(MAP_ROUTE103),
        .mapGroup = MAP_GROUP(MAP_ROUTE103),
    },
    // Petalburg Woods
    {
        .species = SPECIES_SHROOMISH,
        .moves = {MOVE_SPORE, MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL},
        .level = 10,
        .location = MAP_NUM(MAP_PETALBURG_WOODS),
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
    },
    {
        .species = SPECIES_SLAKOTH,
        .moves = {MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL},
        .level = 10,
        .location = MAP_NUM(MAP_PETALBURG_WOODS),
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
    },
    {
        .species = SPECIES_PARAS,
        .moves = {MOVE_SPORE, MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL},
        .level = 10,
        .location = MAP_NUM(MAP_PETALBURG_WOODS),
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
    },
};

static const struct HiddenGrottoMon sHiddenGrottoMons2Hoenn[] = {
    // Petalburg Woods
    {
        .species = SPECIES_BRELOOM,
        .moves = {MOVE_SPORE, MOVE_FOCUS_PUNCH, MOVE_LEVEL, MOVE_LEVEL},
        .level = 30,
        .location = MAP_NUM(MAP_PETALBURG_WOODS),
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
    },
    {
        .species = SPECIES_VIGOROTH,
        .moves = {MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL},
        .level = 30,
        .location = MAP_NUM(MAP_PETALBURG_WOODS),
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
    },
    {
        .species = SPECIES_PARASECT,
        .moves = {MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL, MOVE_LEVEL},
        .level = 30,
        .location = MAP_NUM(MAP_PETALBURG_WOODS),
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
    },
};

static const struct HiddenGrottoItem sHiddenGrottoItems1Hoenn[] = {
    // Route 102
    {
        .item = ITEM_FIRE_STONE,
        .location = MAP_NUM(MAP_ROUTE102),
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
    },
    {
        .item = ITEM_WATER_STONE,
        .location = MAP_NUM(MAP_ROUTE102),
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
    },
    {
        .item = ITEM_THUNDER_STONE,
        .location = MAP_NUM(MAP_ROUTE102),
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
    },
    {
        .item = ITEM_LEAF_STONE,
        .location = MAP_NUM(MAP_ROUTE102),
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
    },
    {
        .item = ITEM_MOON_STONE,
        .location = MAP_NUM(MAP_ROUTE102),
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
    },
    {
        .item = ITEM_SUN_STONE,
        .location = MAP_NUM(MAP_ROUTE102),
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
    },
    // Route 103
    {
        .item = ITEM_ENERGY_ROOT,
        .location = MAP_NUM(MAP_ROUTE103),
        .mapGroup = MAP_GROUP(MAP_ROUTE103),
    },
    {
        .item = ITEM_REVIVAL_HERB,
        .location = MAP_NUM(MAP_ROUTE103),
        .mapGroup = MAP_GROUP(MAP_ROUTE103),
    },
    // Petalburg Woods
    {
        .item = ITEM_TINY_MUSHROOM,
        .location = MAP_NUM(MAP_PETALBURG_WOODS),
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
    },
    {
        .item = ITEM_BIG_MUSHROOM,
        .location = MAP_NUM(MAP_PETALBURG_WOODS),
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
    },
};

static const struct HiddenGrottoItem sHiddenGrottoItems2Hoenn[] = {
};

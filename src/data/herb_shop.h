// Petalburg Herb Shop missions (logic in src/herb_shop.c).
//
// The shop stays closed until the player escorts its owner to the Hidden Grotto on Route 101 (the
// first entry of sHerbShopEscorts). After that she gives one mission a day, picked from everything
// here that is unlocked; a mission that isn't done by the end of the day is dropped.
//
//   Escort   She waits on the route at .x/.y, leads the player along .path to the Hidden Grotto,
//            finds .item inside and both return to the shop. Worth HERB_ESCORT_IP.
//   Catch    She asks for a Pokemon, which then lives in the shop. Each one is asked for until the
//            player has brought it. Worth HERB_CATCH_IP.
//   Fetch    She asks for three different sHerbShopIngredients. Worth the sum of their .ip.
//
// The reward is added to the player's max IP. Missions unlock with the story (.unlock):
//
//   HERB_UNLOCK_START   from the start
//   HERB_UNLOCK_GRUNT   once the Team Aqua Grunt in Petalburg Woods is beaten
//   HERB_UNLOCK_GYM     once the Rustboro Gym is beaten
//
// How often each kind comes up is set by the HERB_*_WEIGHT of every unlocked mission: with the
// weights below, a fetch mission is as likely as three of the escorts together.
//
// Adding an escort:
//   1. Add the route and the tile she waits on to ROUTES in tech/herb_escort_paths.py and run it.
//      It prints the entry with its .path and .altPath: strings of U, D, L and R, one letter per
//      step. The last step is the one into the grotto. .altPath is used when the player talks to
//      her from the first tile of .path, so the two have to start in different directions.
//      `python tech/herb_escort_paths.py --check` checks every path here against the maps.
//   2. Give the route's map an OBJ_EVENT_GFX_WOMAN_2 object on that tile with the script
//      HerbShop_EventScript_EscortOwner and the flag FLAG_HIDE_HERB_SHOP_ESCORT, and add
//      `special HerbShop_UpdateEscortNpc` to the map's on-transition script.
//   3. .where is how she describes the spot: "I'll wait for you on ROUTE 101, by the signpost."
//      .item with an overworld sprite (see sHiddenGrottoItemGraphics) lies in the grotto as
//      itself, anything else is an item ball.
//
// Up to 16 escorts, 16 catches and 16 ingredients fit in the saved mission.

#define HERB_ESCORT_IP      1
#define HERB_CATCH_IP       5

#define HERB_ESCORT_WEIGHT  1
#define HERB_CATCH_WEIGHT   2
#define HERB_FETCH_WEIGHT   3

#define HERB_FETCH_UNLOCK   HERB_UNLOCK_GRUNT

static const u8 sText_ByTheSignpost[] = _("by the signpost");
static const u8 sText_NearPetalburg[] = _("not far from PETALBURG");
static const u8 sText_ByTheFlowerShop[] = _("by the flower shop");
static const u8 sText_NorthOfRustboro[] = _("just north of RUSTBORO");
static const u8 sText_PastTheSmallTrees[] = _("past the small trees up north");

// The first entry is the mission that opens the shop.
static const struct HerbShopEscort sHerbShopEscorts[] = {
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE101),
        .mapNum = MAP_NUM(MAP_ROUTE101),
        .x = 7,
        .y = 9,
        .path = "DDDRRRRRRRDDDD",
        .altPath = "RRRDDDRRRRDDDD",
        .item = ITEM_ENERGY_POWDER,
        .unlock = HERB_UNLOCK_START,
        .where = sText_ByTheSignpost,
    },
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE102),
        .mapNum = MAP_NUM(MAP_ROUTE102),
        .x = 10,
        .y = 9,
        .path = "LLDDDLDDDRRRRR",
        .altPath = "DLLDDLDDDRRRRR",
        .item = ITEM_HEAL_POWDER,
        .unlock = HERB_UNLOCK_START,
        .where = sText_NearPetalburg,
    },
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE103),
        .mapNum = MAP_NUM(MAP_ROUTE103),
        .x = 11,
        .y = 6,
        .path = "LUUUUU",
        .altPath = "DLLUUUUURU",
        .item = ITEM_ENERGY_ROOT,
        .unlock = HERB_UNLOCK_START,
        .where = sText_ByTheSignpost,
    },
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE104),
        .mapNum = MAP_NUM(MAP_ROUTE104),
        .x = 6,
        .y = 20,
        .path = "LLLLUUUUUUUUUUUUUURRRRUUU",
        .altPath = "ULLLLUUUUUUUUUUUUURRRRUUU",
        .item = ITEM_REVIVAL_HERB,
        .unlock = HERB_UNLOCK_GRUNT,
        .where = sText_ByTheFlowerShop,
    },
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE115),
        .mapNum = MAP_NUM(MAP_ROUTE115),
        .x = 21,
        .y = 73,
        .path = "UUUUUUUUUUUUR",
        .altPath = "LUUUUUURUUUUUUR",
        .item = ITEM_WHITE_HERB,
        .unlock = HERB_UNLOCK_GRUNT,
        .where = sText_NorthOfRustboro,
    },
    {
        .mapGroup = MAP_GROUP(MAP_ROUTE116),
        .mapNum = MAP_NUM(MAP_ROUTE116),
        .x = 4,
        .y = 10,
        .path = "UUURRRRRRDDD",
        .altPath = "LUUURRRRRRRDDD",
        .item = ITEM_MENTAL_HERB,
        .unlock = HERB_UNLOCK_GRUNT,
        .where = sText_ByTheSignpost,
    },
    {
        .mapGroup = MAP_GROUP(MAP_PETALBURG_WOODS),
        .mapNum = MAP_NUM(MAP_PETALBURG_WOODS),
        .x = 21,
        .y = 11,
        .path = "RRRRRUUUUUU",
        .altPath = "URRRRRUUUUU",
        .item = ITEM_TINY_MUSHROOM,
        .unlock = HERB_UNLOCK_GYM,
        .where = sText_PastTheSmallTrees,
    },
};

static const u8 sText_ShroomishHint[] = _("They live in PETALBURG WOODS.");
static const u8 sText_ParasHint[] = _("I hear they hide in the hidden\ngrotto of PETALBURG WOODS.");

// .localId is the Pokemon's object in the shop, hidden by .hideFlag until .flag is set.
static const struct HerbShopCatch sHerbShopCatches[] = {
    {
        .species = SPECIES_SHROOMISH,
        .flag = FLAG_HERB_SHOP_HAS_SHROOMISH,
        .hideFlag = FLAG_HIDE_HERB_SHOP_SHROOMISH,
        .localId = LOCALID_HERB_SHOP_SHROOMISH,
        .unlock = HERB_UNLOCK_GRUNT,
        .hint = sText_ShroomishHint,
    },
    {
        .species = SPECIES_PARAS,
        .flag = FLAG_HERB_SHOP_HAS_PARAS,
        .hideFlag = FLAG_HIDE_HERB_SHOP_PARAS,
        .localId = LOCALID_HERB_SHOP_PARAS,
        .unlock = HERB_UNLOCK_GYM,
        .hint = sText_ParasHint,
    },
};

static const struct HerbShopIngredient sHerbShopIngredients[] = {
    {.item = ITEM_BIG_MUSHROOM,  .ip = 3},
    {.item = ITEM_BRIGHT_POWDER, .ip = 2},
    {.item = ITEM_TINY_MUSHROOM, .ip = 1},
    {.item = ITEM_DRAGON_FANG,   .ip = 2},
    {.item = ITEM_DRAGON_SCALE,  .ip = 2},
    {.item = ITEM_MENTAL_HERB,   .ip = 2},
    {.item = ITEM_POISON_BARB,   .ip = 3},
    {.item = ITEM_QUICK_CLAW,    .ip = 1},
    {.item = ITEM_SHARP_BEAK,    .ip = 3},
    {.item = ITEM_SILVER_POWDER, .ip = 3},
    {.item = ITEM_WHITE_HERB,    .ip = 2},
};

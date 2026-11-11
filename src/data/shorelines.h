// Shoreline (wading) encounters.
//
// Walking, running or biking (not surfing) on shallow shoreline water (MB_SHALLOW_WATER, the walkable
// band of the beach shorelines) can start a wild battle, the same way tall grass does. Each step rolls
// the table's encounterRate exactly like grass (early routes use 20 for grass, 4 for surfing), then one
// entry is picked by weight and given a random level in its range.
//
// Which table a map uses:
//   - a map listed in sShorelineMaps uses the table given there (.table = NULL turns wading
//     encounters off on that map). Listing a map also works for caves and other non-outdoor maps.
//   - every other outdoor map (town, city, route, ocean route) uses sShorelineDefault.
//   - every other map has no wading encounters.
//
// Weights are relative: an entry's chance is its weight divided by the total weight of the entries
// that can appear right now. 25/25/25/10 gives about 29% each for the first three and 12% for the last.
//
// Entries follow the same rules as every other wild Pokemon:
//   - a species' time of day (gSpeciesInfo .timeOfDay) applies, e.g. Staryu only at night and Corsola
//     only in the morning and during the day. Entries that can't appear now are left out of the roll.
//   - Repel, Keen Eye/Intimidate, Hustle/Pressure/Vital Spirit levels, Cleanse Tag, flutes, chains,
//     custom IVs and dangerous/severe encounters all work as they do in grass. Sweet Scent works too.
//
// Seasons: an entry can take an optional .seasons, a mask of the season tags from include/time_events.h
// joined with | (SEASON_TAG_SPRING, SEASON_TAG_SUMMER, SEASON_TAG_FALL, SEASON_TAG_WINTER, SEASON_TAG_ALL).
// Leaving it out is the same as SEASON_TAG_ALL.
//
// Adding a map with its own table:
//
// static const struct ShorelineMon sShorelineMons_Route134[] = {
//     { .species = SPECIES_STARYU,   .minLevel = 35, .maxLevel = 40, .weight = 25 },
//     { .species = SPECIES_CORSOLA,  .minLevel = 35, .maxLevel = 40, .weight = 10 },
//     { .species = SPECIES_CLAMPERL, .minLevel = 35, .maxLevel = 40, .weight = 5, .seasons = SEASON_TAG_SUMMER },
// };
// static const struct ShorelineTable sShorelineRoute134 = SHORELINE_TABLE(10, sShorelineMons_Route134);
//
// and in sShorelineMaps:
//     { .map = MAP_ROUTE134, .table = &sShorelineRoute134 },
//     { .map = MAP_LILYCOVE_CITY, .table = NULL }, // no wading encounters in Lilycove

#define SHORELINE_TABLE(rate, monList) { .encounterRate = rate, .count = ARRAY_COUNT(monList), .mons = monList }

static const struct ShorelineMon sShorelineMons_Default[] = {
    { .species = SPECIES_STARYU,   .minLevel = 5, .maxLevel = 10, .weight = 25 },
    { .species = SPECIES_SHELLDER, .minLevel = 5, .maxLevel = 10, .weight = 25 },
    { .species = SPECIES_KRABBY,   .minLevel = 5, .maxLevel = 10, .weight = 25 },
    { .species = SPECIES_CORSOLA,  .minLevel = 5, .maxLevel = 10, .weight = 10 },
};

static const struct ShorelineTable sShorelineDefault = SHORELINE_TABLE(10, sShorelineMons_Default);

static const struct ShorelineMap sShorelineMaps[] = {
    { .map = MAP_UNDEFINED }, // end of list, keep last
};

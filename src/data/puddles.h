// Puddle encounters.
//
// Walking, running or biking through a puddle (MB_PUDDLE) can start a wild battle, the same way the
// shallow water of a shoreline does (src/data/shorelines.h). Each step rolls the table's encounterRate
// exactly like grass (early routes use 20 for grass, 4 for surfing), then one entry is picked by weight
// and given a random level in its range.
//
// Which table a map uses:
//   - a map listed in sPuddleMaps uses the table given there (.table = NULL turns puddle encounters
//     off on that map). Listing a map also works for caves and other non-outdoor maps.
//   - every other outdoor map (town, city, route, ocean route) uses sPuddleDefault.
//   - every other map has no puddle encounters.
//
// Weights are relative: an entry's chance is its weight divided by the total weight of the entries
// that can appear right now. 60/30/10 gives 60%, 30% and 10%.
//
// Entries follow the same rules as every other wild Pokemon:
//   - a species' time of day (gSpeciesInfo .timeOfDay) applies, e.g. Mudkip only in the morning and at
//     night. Entries that can't appear now are left out of the roll.
//   - Repel, Keen Eye/Intimidate, Hustle/Pressure/Vital Spirit levels, Cleanse Tag, flutes, chains,
//     custom IVs and dangerous/severe encounters all work as they do in grass. Sweet Scent works too.
//   - the weather makes some types more or less common (sWeatherEncounterTypes in
//     src/wild_encounter.c), e.g. rain doubles the weight of Water-type entries.
//
// Seasons: an entry can take an optional .seasons, a mask of the season tags from include/time_events.h
// joined with | (SEASON_TAG_SPRING, SEASON_TAG_SUMMER, SEASON_TAG_FALL, SEASON_TAG_WINTER, SEASON_TAG_ALL).
// Leaving it out is the same as SEASON_TAG_ALL.
//
// Adding a map with its own table:
//
// static const struct PuddleMon sPuddleMons_Route120[] = {
//     { .species = SPECIES_LOMBRE,  .minLevel = 25, .maxLevel = 28, .weight = 60 },
//     { .species = SPECIES_SURSKIT, .minLevel = 25, .maxLevel = 28, .weight = 30 },
//     { .species = SPECIES_WOOPER,  .minLevel = 25, .maxLevel = 28, .weight = 10, .seasons = SEASON_TAG_SPRING },
// };
// static const struct PuddleTable sPuddleRoute120 = PUDDLE_TABLE(2, sPuddleMons_Route120);
//
// and in sPuddleMaps:
//     { .map = MAP_ROUTE120, .table = &sPuddleRoute120 },
//     { .map = MAP_RUSTBORO_CITY, .table = NULL }, // no puddle encounters in Rustboro

#define PUDDLE_TABLE(rate, monList) { .encounterRate = rate, .count = ARRAY_COUNT(monList), .mons = monList }

static const struct PuddleMon sPuddleMons_Default[] = {
    { .species = SPECIES_LOTAD,   .minLevel = 5, .maxLevel = 10, .weight = 60 }, // common
    { .species = SPECIES_SURSKIT, .minLevel = 5, .maxLevel = 10, .weight = 30 }, // uncommon
    { .species = SPECIES_MUDKIP,  .minLevel = 5, .maxLevel = 10, .weight = 10 }, // rare
};

static const struct PuddleTable sPuddleDefault = PUDDLE_TABLE(1, sPuddleMons_Default);

static const struct PuddleMap sPuddleMaps[] = {
    { .map = MAP_UNDEFINED }, // end of list, keep last
};

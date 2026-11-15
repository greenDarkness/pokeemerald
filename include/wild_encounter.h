#ifndef GUARD_WILD_ENCOUNTER_H
#define GUARD_WILD_ENCOUNTER_H

#include "constants/wild_encounter.h"

struct WildPokemon
{
    u8 minLevel;
    u8 maxLevel;
    u16 species;
};

struct WildPokemonInfo
{
    u8 encounterRate;
    const struct WildPokemon *wildPokemon;
};

struct WildPokemonHeader
{
    u8 mapGroup;
    u8 mapNum;
    const struct WildPokemonInfo *landMonsInfo;
    const struct WildPokemonInfo *waterMonsInfo;
    const struct WildPokemonInfo *rockSmashMonsInfo;
    const struct WildPokemonInfo *fishingMonsInfo;
};

// Level of a hand-written encounter (swarms, hidden grottos), stored in a u16: a plain number is
// a fixed level, LEVEL_RANGE(min, max) a random level from min to max for each encounter.
#define LEVEL_RANGE(min, max) ((min) | ((max) << 8))

extern const struct WildPokemonHeader gWildMonHeaders[];

// Encounter type: 0 = normal, 1 = dangerous, 2 = severe
extern u8 gDangerousEncounterType;

void DisableWildEncounters(bool8 disabled);
void InitFeebasSeed(void);
void UpdateFeebasSpots(u16 daysSince);
bool8 StandardWildEncounter(u16 curMetatileBehavior, u16 prevMetatileBehavior);
bool8 SweetScentWildEncounter(void);
bool8 DoesCurrentMapHaveFishingMons(void);
void FishingWildEncounter(u8 rod);
u16 GetLocalWildMon(bool8 *isWaterMon);
u16 GetLocalWaterMon(void);
bool8 UpdateRepelCounter(void);
bool8 TryApplyCustomWildMonIVs(u16 species, struct Pokemon *mon);
bool8 WasSweetScentBattle(void);
u16 GetMoveLevelMove_Wild(u16 species, u8 level, const u16 *existingMoves);
void ClearSweetScentBattle(void);
const struct WildPokemonHeader *GetCurrentMapWildMonHeader(void);
bool8 IsWildSpeciesAvailableAtTime(u16 species, u8 timeOfDay);
bool8 IsWildSpeciesAvailableAtTimeOnMap(u16 species, u8 timeOfDay, u8 mapType);
bool8 DoesCurrentMapIgnoreTimeOfDay(void);
const struct WildPokemon *GetCurrentMapFeebas(void);
bool8 GetShorelineSpot(u16 index, u8 *mapGroup, u8 *mapNum, u8 *x, u8 *y);
bool8 DoesMapHaveWadingSpecies(u8 mapGroup, u8 mapNum, u16 species, u8 timeOfDay);
bool8 DoesCurrentMapHaveWading(void);
bool8 GetCurrentMapWadingMon(u16 index, u8 timeOfDay, u16 *species, u8 *minLevel, u8 *maxLevel);
void GetLevelRange(u16 level, u8 *minLevel, u8 *maxLevel);
u8 ChooseLevelInRange(u8 minLevel, u8 maxLevel);

#endif // GUARD_WILD_ENCOUNTER_H

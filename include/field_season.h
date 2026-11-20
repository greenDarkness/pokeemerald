#ifndef GUARD_FIELD_SEASON_H
#define GUARD_FIELD_SEASON_H

#include "sprite.h"
#include "time_events.h"

// A tileset's palettes for one season: the ones that differ from the tileset's own.
struct SeasonalPalettes
{
    u16 slots;                  // Bit n set: palette n of the tileset is replaced
    const u16 (*palettes)[16];  // The replacements, lowest slot first
};

// A tileset's graphics for the seasons its own tiles and palettes (its summer look) don't cover.
// gSeasonalTilesets ends with an entry whose tileset is NULL.
struct SeasonalTileset
{
    const struct Tileset *tileset;
    const u32 *tiles[SEASONS_PER_YEAR]; // NULL: the tileset's own tiles
    struct SeasonalPalettes palettes[SEASONS_PER_YEAR];
};

// A sprite palette with the map's grass colors. gSeasonalSpritePalettes ends with a NULL base.
struct SeasonalSpritePalette
{
    const u16 *base;
    const u16 *seasons[SEASONS_PER_YEAR]; // NULL: base
};

extern const struct SeasonalTileset gSeasonalTilesets[];
extern const struct SeasonalSpritePalette gSeasonalSpritePalettes[];

u8 GetFieldSeason(void);
void UpdateFieldSeason(void);
void InitFieldSeason(void);
const u32 *GetTilesetTiles(const struct Tileset *tileset);
const u16 *GetTilesetPalette(const struct Tileset *tileset, u32 slot);
void LoadSeasonalTilesetPalettes(const struct Tileset *tileset, u32 firstSlot, u32 count, bool32 skipFaded);
u8 LoadSeasonalSpritePalette(const struct SpritePalette *palette);
const u8 *GetTimeOfDayStartHours(void);
u8 GetTimeOfDayForHour(u8 hour);

#endif // GUARD_FIELD_SEASON_H

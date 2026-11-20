#include "global.h"
#include "field_season.h"
#include "fieldmap.h"
#include "overworld.h"
#include "palette.h"
#include "sprite.h"
#include "time_events.h"
#include "constants/map_types.h"

// How the seasons look, after Pokemon Black 2 and White 2 (season.c, area_data.c and rtc.c of
// https://github.com/fuddlesworth/pokebw2).
//
// The calendar's season (GetCurrentSeason, src/time_events.c) changes at midnight. The field
// keeps its own, the season it is drawn in, which only catches up when a warp brings the player
// outdoors: walking between connected maps reloads half of the palettes, so a change there would
// show as a seam, and one in the middle of a map as a flash.
//
// Outdoor tilesets have a look for each season: their own tiles and palettes are summer, and
// gSeasonalTilesets (src/data/tilesets/seasonal.h, written by tech/season_palettes.py) lists the
// palettes, and optionally the tiles, that replace them in the others. Only maps with natural
// light and secret bases (whose walls are the leaves of the tree or shrub outside) are drawn in
// the season; other maps (indoors, caves, underwater) always use the tileset's own.
//
// The seasons also move sunrise and sunset; those follow the calendar, since midnight is night
// in every season.

EWRAM_DATA static u8 sFieldSeason = 0;
EWRAM_DATA static const struct Tileset *sLookupTilesets[2] = {NULL};
EWRAM_DATA static const struct SeasonalTileset *sLookupResults[2] = {NULL};

// The hour each time of day starts at. Its first hour blends in from the one before.
// B2W2's LIGHT_CHANGE_HOURS; its evening is part of the day here.
static const u8 sTimeOfDayStartHours[SEASONS_PER_YEAR][TIME_OF_DAY_MAX + 1] =
{
    [SEASON_SPRING] = {[TIME_OF_DAY_MORNING] = 5, [TIME_OF_DAY_DAY] = 10, [TIME_OF_DAY_NIGHT] = 20},
    [SEASON_SUMMER] = {[TIME_OF_DAY_MORNING] = 4, [TIME_OF_DAY_DAY] = 9,  [TIME_OF_DAY_NIGHT] = 21},
    [SEASON_FALL]   = {[TIME_OF_DAY_MORNING] = 6, [TIME_OF_DAY_DAY] = 10, [TIME_OF_DAY_NIGHT] = 20},
    [SEASON_WINTER] = {[TIME_OF_DAY_MORNING] = 7, [TIME_OF_DAY_DAY] = 11, [TIME_OF_DAY_NIGHT] = 19},
};

// The season the field is drawn in.
u8 GetFieldSeason(void)
{
    return sFieldSeason;
}

// Called when a warp loads a map, before its graphics are.
void UpdateFieldSeason(void)
{
    if (MapHasNaturalLight(gMapHeader.mapType))
        sFieldSeason = GetCurrentSeason();
}

// Called when a game starts or is continued: the field starts in the calendar's season wherever
// the player is.
void InitFieldSeason(void)
{
    sFieldSeason = GetCurrentSeason();
}

// Whether the current map is drawn in the field's season. Entering a secret base doesn't change
// the season (UpdateFieldSeason), so the base matches the route it was entered from.
// tech/season_palettes.py has the same rule (SEASONAL_MAP_TYPES).
static bool32 FieldHasSeasons(void)
{
    return MapHasNaturalLight(gMapHeader.mapType) || gMapHeader.mapType == MAP_TYPE_SECRET_BASE;
}

static const struct SeasonalTileset *GetSeasonalTileset(const struct Tileset *tileset)
{
    const struct SeasonalTileset *seasonal;
    u32 i;

    if (tileset == NULL || !FieldHasSeasons())
        return NULL;

    // The map's two tilesets are looked up over and over, so remember where they were.
    i = (tileset->isSecondary == TRUE);
    if (sLookupTilesets[i] == tileset)
        return sLookupResults[i];

    for (seasonal = gSeasonalTilesets; seasonal->tileset != NULL; seasonal++)
    {
        if (seasonal->tileset == tileset)
            break;
    }
    if (seasonal->tileset == NULL)
        seasonal = NULL;
    sLookupTilesets[i] = tileset;
    sLookupResults[i] = seasonal;
    return seasonal;
}

static const u16 *GetSeasonalPalette(const struct SeasonalTileset *seasonal, u32 slot)
{
    const struct SeasonalPalettes *palettes;
    u32 i, index;

    if (seasonal == NULL)
        return NULL;
    palettes = &seasonal->palettes[sFieldSeason];
    if (!(palettes->slots & (1 << slot)))
        return NULL;
    for (i = 0, index = 0; i < slot; i++)
    {
        if (palettes->slots & (1 << i))
            index++;
    }
    return palettes->palettes[index];
}

// The tileset's tile graphics for the field's season, compressed like the tileset's own.
const u32 *GetTilesetTiles(const struct Tileset *tileset)
{
    const struct SeasonalTileset *seasonal = GetSeasonalTileset(tileset);

    if (seasonal != NULL && seasonal->tiles[sFieldSeason] != NULL)
        return seasonal->tiles[sFieldSeason];
    return tileset->tiles;
}

// Palette slot (0-15) of the tileset for the field's season. Use this instead of
// tileset->palettes[slot] for anything shown on the map.
const u16 *GetTilesetPalette(const struct Tileset *tileset, u32 slot)
{
    const u16 *palette = GetSeasonalPalette(GetSeasonalTileset(tileset), slot);

    if (palette != NULL)
        return palette;
    return tileset->palettes[slot];
}

// Loads the season's versions of the tileset's palettes [firstSlot, firstSlot + count) over
// the tileset's own, which were just loaded to the same BG palette numbers.
void LoadSeasonalTilesetPalettes(const struct Tileset *tileset, u32 firstSlot, u32 count, bool32 skipFaded)
{
    const struct SeasonalTileset *seasonal = GetSeasonalTileset(tileset);
    u32 slot;

    if (seasonal == NULL || seasonal->palettes[sFieldSeason].slots == 0)
        return;
    for (slot = firstSlot; slot < firstSlot + count; slot++)
    {
        const u16 *palette = GetSeasonalPalette(seasonal, slot);

        if (palette == NULL)
            continue;
        CpuCopy16(palette, &gPlttBufferUnfaded[BG_PLTT_ID(slot)], PLTT_SIZE_4BPP);
        if (!skipFaded)
            CpuCopy16(palette, &gPlttBufferFaded[BG_PLTT_ID(slot)], PLTT_SIZE_4BPP);
    }
}

// LoadSpritePalette, with the season's version of a palette that holds the map's grass colors.
u8 LoadSeasonalSpritePalette(const struct SpritePalette *palette)
{
    const struct SeasonalSpritePalette *seasonal;
    struct SpritePalette seasonPalette;

    if (MapHasNaturalLight(gMapHeader.mapType))
    {
        for (seasonal = gSeasonalSpritePalettes; seasonal->base != NULL; seasonal++)
        {
            if (seasonal->base == palette->data && seasonal->seasons[sFieldSeason] != NULL)
            {
                seasonPalette = *palette;
                seasonPalette.data = seasonal->seasons[sFieldSeason];
                return LoadSpritePalette(&seasonPalette);
            }
        }
    }
    return LoadSpritePalette(palette);
}

static u32 GetDaylightSeason(void)
{
    struct CalendarDate date;

    // The date the clock is at, even where a new day hasn't been processed yet (Pokemon Centers).
    GetCalendarDate(&date);
    return date.season;
}

// The hours (0-23) that each time of day starts at in the calendar's season, by TIME_OF_DAY_*.
const u8 *GetTimeOfDayStartHours(void)
{
    return sTimeOfDayStartHours[GetDaylightSeason()];
}

// The time of day (TIME_OF_DAY_*) at an hour (0-23) in the calendar's season.
u8 GetTimeOfDayForHour(u8 hour)
{
    const u8 *starts = GetTimeOfDayStartHours();

    if (hour >= starts[TIME_OF_DAY_NIGHT] || hour < starts[TIME_OF_DAY_MORNING])
        return TIME_OF_DAY_NIGHT;
    if (hour >= starts[TIME_OF_DAY_DAY])
        return TIME_OF_DAY_DAY;
    return TIME_OF_DAY_MORNING;
}

#include "global.h"
#include "battle_pyramid.h"
#include "bg.h"
#include "field_season.h"
#include "fieldmap.h"
#include "fldeff.h"
#include "fldeff_misc.h"
#include "frontier_util.h"
#include "menu.h"
#include "mirage_tower.h"
#include "metatile_behavior.h"
#include "overworld.h"
#include "palette.h"
#include "pokenav.h"
#include "script.h"
#include "secret_base.h"
#include "tv.h"
#include "constants/rgb.h"
#include "constants/metatile_behaviors.h"

struct ConnectionFlags
{
    u8 south:1;
    u8 north:1;
    u8 west:1;
    u8 east:1;
};

EWRAM_DATA static u16 ALIGNED(4) sBackupMapData[MAX_MAP_DATA_SIZE] = {0};
EWRAM_DATA struct MapHeader gMapHeader = {0};
EWRAM_DATA struct Camera gCamera = {0};
EWRAM_DATA static struct ConnectionFlags sMapConnectionFlags = {0};
EWRAM_DATA static u32 UNUSED sFiller = 0; // without this, the next file won't align properly

COMMON_DATA struct BackupMapLayout gBackupMapLayout = {0};

static const struct ConnectionFlags sDummyConnectionFlags = {0};

static void InitMapLayoutData(const struct MapHeader *mapHeader);
static void InitBackupMapLayoutData(const u16 *map, u16 width, u16 height);
static void FillSouthConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset);
static void FillNorthConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset);
static void FillWestConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset);
static void FillEastConnection(struct MapHeader const *mapHeader, struct MapHeader const *connectedMapHeader, s32 offset);
static void InitBackupMapLayoutConnections(const struct MapHeader *mapHeader);
static void LoadSavedMapView(void);
static bool8 SkipCopyingMetatileFromSavedMap(u16 *mapBlock, u16 mapWidth, u8 yMode);
static const struct MapConnection *GetIncomingConnection(u8 direction, s32 x, s32 y);
static bool8 IsPosInIncomingConnectingMap(u8 direction, s32 x, s32 y, const struct MapConnection *connection);
static bool8 IsCoordInIncomingConnectingMap(s32 coord, s32 srcMax, s32 destMax, s32 offset);

#define GetBorderBlockAt(x, y) (gMapHeader.mapLayout->border[((x + 1) & 1) + (((y + 1) & 1) << 1)] | MAPGRID_IMPASSABLE)

#define AreCoordsWithinMapGridBounds(x, y) (x >= 0 && x < gBackupMapLayout.width && y >= 0 && y < gBackupMapLayout.height)

#define GetMapGridBlockAt(x, y) (AreCoordsWithinMapGridBounds(x, y) ? gBackupMapLayout.map[x + gBackupMapLayout.width * y] : GetBorderBlockAt(x, y))

const struct MapHeader *const GetMapHeaderFromConnection(const struct MapConnection *connection)
{
    return Overworld_GetMapHeaderByGroupAndId(connection->mapGroup, connection->mapNum);
}

void InitMap(void)
{
    InitMapLayoutData(&gMapHeader);
    SetOccupiedSecretBaseEntranceMetatiles(gMapHeader.events);
    RunOnLoadMapScript();
}

void InitMapFromSavedGame(void)
{
    InitMapLayoutData(&gMapHeader);
    InitSecretBaseAppearance(FALSE);
    SetOccupiedSecretBaseEntranceMetatiles(gMapHeader.events);
    LoadSavedMapView();
    RunOnLoadMapScript();
    UpdateTVScreensOnMap(gBackupMapLayout.width, gBackupMapLayout.height);
}

void InitBattlePyramidMap(bool8 setPlayerPosition)
{
    CpuFastFill16(MAPGRID_UNDEFINED, sBackupMapData, sizeof(sBackupMapData));
    GenerateBattlePyramidFloorLayout(sBackupMapData, setPlayerPosition);
}

static void InitMapLayoutData(const struct MapHeader *mapHeader)
{
    const struct MapLayout *mapLayout = mapHeader->mapLayout;
    CpuFastFill16(MAPGRID_UNDEFINED, sBackupMapData, sizeof(sBackupMapData));

    gBackupMapLayout.map = sBackupMapData;
    gBackupMapLayout.width = mapLayout->width + MAP_OFFSET_W;
    gBackupMapLayout.height = mapLayout->height + MAP_OFFSET_H;

    if (gBackupMapLayout.width * gBackupMapLayout.height > MAX_MAP_DATA_SIZE)
        return;

    InitBackupMapLayoutData(mapLayout->map, mapLayout->width, mapLayout->height);
    InitBackupMapLayoutConnections(mapHeader);
}

static void InitBackupMapLayoutData(const u16 *map, u16 width, u16 height)
{
    u16 *dest;
    s32 y;
    dest = gBackupMapLayout.map;
    dest += gBackupMapLayout.width * MAP_OFFSET + MAP_OFFSET;
    for (y = 0; y < height; y++)
    {
        CpuCopy16(map, dest, width * 2);
        dest += width + MAP_OFFSET_W;
        map += width;
    }
}

static void InitBackupMapLayoutConnections(const struct MapHeader *mapHeader)
{
    s32 count, i, offset;
    const struct MapConnection *connection;
    const struct MapHeader *cMap;

    if (!mapHeader->connections)
        return;

    count = mapHeader->connections->count;
    connection = mapHeader->connections->connections;
    sMapConnectionFlags = sDummyConnectionFlags;
    for (i = 0; i < count; i++, connection++)
    {
        cMap = GetMapHeaderFromConnection(connection);
        offset = connection->offset;
        switch (connection->direction)
        {
        case CONNECTION_SOUTH:
            FillSouthConnection(mapHeader, cMap, offset);
            sMapConnectionFlags.south = TRUE;
            break;
        case CONNECTION_NORTH:
            FillNorthConnection(mapHeader, cMap, offset);
            sMapConnectionFlags.north = TRUE;
            break;
        case CONNECTION_WEST:
            FillWestConnection(mapHeader, cMap, offset);
            sMapConnectionFlags.west = TRUE;
            break;
        case CONNECTION_EAST:
            FillEastConnection(mapHeader, cMap, offset);
            sMapConnectionFlags.east = TRUE;
            break;
        }
    }
}

static void FillConnection(s32 x, s32 y, const struct MapHeader *connectedMapHeader, s32 x2, s32 y2, s32 width, s32 height)
{
    s32 i;
    const u16 *src;
    u16 *dest;
    s32 mapWidth;

    mapWidth = connectedMapHeader->mapLayout->width;
    src = &connectedMapHeader->mapLayout->map[mapWidth * y2 + x2];
    dest = &gBackupMapLayout.map[gBackupMapLayout.width * y + x];

    for (i = 0; i < height; i++)
    {
        CpuCopy16(src, dest, width * 2);
        dest += gBackupMapLayout.width;
        src += mapWidth;
    }
}

static void FillSouthConnection(const struct MapHeader *mapHeader, const struct MapHeader *connectedMapHeader, s32 offset)
{
    s32 x, y;
    s32 x2;
    s32 width;
    s32 cWidth;

    if (!connectedMapHeader)
        return;

    cWidth = connectedMapHeader->mapLayout->width;
    x = offset + MAP_OFFSET;
    y = mapHeader->mapLayout->height + MAP_OFFSET;
    if (x < 0)
    {
        x2 = -x;
        x += cWidth;
        if (x < gBackupMapLayout.width)
            width = x;
        else
            width = gBackupMapLayout.width;
        x = 0;
    }
    else
    {
        x2 = 0;
        if (x + cWidth < gBackupMapLayout.width)
            width = cWidth;
        else
            width = gBackupMapLayout.width - x;
    }

    FillConnection(x, y, connectedMapHeader, x2, /*y2*/ 0, width, /*height*/ MAP_OFFSET);
}

static void FillNorthConnection(const struct MapHeader *mapHeader, const struct MapHeader *connectedMapHeader, s32 offset)
{
    s32 x;
    s32 x2, y2;
    s32 width;
    s32 cWidth, cHeight;

    if (!connectedMapHeader)
        return;

    cWidth = connectedMapHeader->mapLayout->width;
    cHeight = connectedMapHeader->mapLayout->height;
    x = offset + MAP_OFFSET;
    y2 = cHeight - MAP_OFFSET;
    if (x < 0)
    {
        x2 = -x;
        x += cWidth;
        if (x < gBackupMapLayout.width)
            width = x;
        else
            width = gBackupMapLayout.width;
        x = 0;
    }
    else
    {
        x2 = 0;
        if (x + cWidth < gBackupMapLayout.width)
            width = cWidth;
        else
            width = gBackupMapLayout.width - x;
    }

    FillConnection(x, /*y*/ 0, connectedMapHeader, x2, y2, width, /*height*/ MAP_OFFSET);
}

static void FillWestConnection(const struct MapHeader *mapHeader, const struct MapHeader *connectedMapHeader, s32 offset)
{
    s32 y;
    s32 x2, y2;
    s32 height;
    s32 cWidth, cHeight;

    if (!connectedMapHeader)
        return;

    cWidth = connectedMapHeader->mapLayout->width;
    cHeight = connectedMapHeader->mapLayout->height;
    y = offset + MAP_OFFSET;
    x2 = cWidth - MAP_OFFSET;
    if (y < 0)
    {
        y2 = -y;
        if (y + cHeight < gBackupMapLayout.height)
            height = y + cHeight;
        else
            height = gBackupMapLayout.height;
        y = 0;
    }
    else
    {
        y2 = 0;
        if (y + cHeight < gBackupMapLayout.height)
            height = cHeight;
        else
            height = gBackupMapLayout.height - y;
    }

    FillConnection(/*x*/ 0, y, connectedMapHeader, x2, y2, /*width*/ MAP_OFFSET, height);
}

static void FillEastConnection(const struct MapHeader *mapHeader, const struct MapHeader *connectedMapHeader, s32 offset)
{
    s32 x, y;
    s32 y2;
    s32 height;
    s32 cHeight;
    if (!connectedMapHeader)
        return;

    cHeight = connectedMapHeader->mapLayout->height;
    x = mapHeader->mapLayout->width + MAP_OFFSET;
    y = offset + MAP_OFFSET;
    if (y < 0)
    {
        y2 = -y;
        if (y + cHeight < gBackupMapLayout.height)
            height = y + cHeight;
        else
            height = gBackupMapLayout.height;
        y = 0;
    }
    else
    {
        y2 = 0;
        if (y + cHeight < gBackupMapLayout.height)
            height = cHeight;
        else
            height = gBackupMapLayout.height - y;
    }

    FillConnection(x, y, connectedMapHeader, /*x2*/ 0, y2, /*width*/ MAP_OFFSET + 1, height);
}

u8 MapGridGetElevationAt(s32 x, s32 y)
{
    u16 block = GetMapGridBlockAt(x, y);

    if (block == MAPGRID_UNDEFINED)
        return 0;

    return UNPACK_ELEVATION(block);
}

u8 MapGridGetCollisionAt(s32 x, s32 y)
{
    u16 block = GetMapGridBlockAt(x, y);

    if (block == MAPGRID_UNDEFINED)
        return 1;

    return UNPACK_COLLISION(block);
}

s32 MapGridGetMetatileIdAt(s32 x, s32 y)
{
    s32 block = GetMapGridBlockAt(x, y);

    if (block == MAPGRID_UNDEFINED)
        return UNPACK_METATILE(GetBorderBlockAt(x, y));

    return UNPACK_METATILE(block);
}

s32 MapGridGetMetatileBehaviorAt(s32 x, s32 y)
{
    return UNPACK_BEHAVIOR(GetMetatileAttributesById(MapGridGetMetatileIdAt(x, y)));
}

u8 MapGridGetMetatileLayerTypeAt(s32 x, s32 y)
{
    return UNPACK_LAYER_TYPE(GetMetatileAttributesById(MapGridGetMetatileIdAt(x, y)));
}

static bool32 IsWaterDrawnUnderSprites(u32 behavior)
{
    return MetatileBehavior_IsSurfableWaterOrUnderwater(behavior)
        || MetatileBehavior_IsShallowFlowingWater(behavior)
        || behavior == MB_PUDDLE
        || behavior == MB_REFLECTION_UNDER_BRIDGE
        || behavior == MB_BRIDGE_OVER_OCEAN
        || behavior == MB_BRIDGE_OVER_POND_LOW
        || behavior == MB_BRIDGE_OVER_POND_MED
        || behavior == MB_BRIDGE_OVER_POND_HIGH
        || behavior == MB_BRIDGE_OVER_POND_HIGH_EDGE_1
        || behavior == MB_BRIDGE_OVER_POND_HIGH_EDGE_2;
}

static bool32 IsOpenWater(u32 behavior)
{
    return MetatileBehavior_IsSurfableFishableWater(behavior)
        || behavior == MB_PUDDLE
        || behavior == MB_REFLECTION_UNDER_BRIDGE;
}

#define NUM_BOTTOM_TILES (NUM_TILES_PER_METATILE / 2)
#define MAX_WATER_BOTTOMS 48
#define TILE_ID(entry) ((entry) & (NUM_TILES_TOTAL - 1))
#define TEST_TILE(set, entry) ((set)[TILE_ID(entry) / 32] & (1 << (TILE_ID(entry) % 32)))
#define SET_TILE(set, entry)  ((set)[TILE_ID(entry) / 32] |= 1 << (TILE_ID(entry) % 32))

// What the open water of the current primary tileset is made of. Used to recognize water
// in metatiles that aren't water themselves, like rocks and buildings standing in the sea.
static EWRAM_DATA struct
{
    const struct Tileset *tileset;
    u16 bottoms[MAX_WATER_BOTTOMS][NUM_BOTTOM_TILES]; // Every bottom layer its open water uses
    u32 firstTiles[NUM_TILES_TOTAL / 32];             // The tiles that start one of them
    u32 plainTiles[NUM_TILES_TOTAL / 32];             // The tiles of open water with an empty top layer
    u8 numBottoms;
} sWaterTiles = {0};

static bool32 IsSameBottomLayer(const u16 *a, const u16 *b)
{
    return a[0] == b[0] && a[1] == b[1] && a[2] == b[2] && a[3] == b[3];
}

static void LoadWaterTiles(const struct Tileset *tileset)
{
    u32 i, j, behavior;
    const u16 *tiles;

    sWaterTiles.tileset = tileset;
    sWaterTiles.numBottoms = 0;
    CpuFill32(0, sWaterTiles.firstTiles, sizeof(sWaterTiles.firstTiles));
    CpuFill32(0, sWaterTiles.plainTiles, sizeof(sWaterTiles.plainTiles));
    for (i = 0; i < NUM_METATILES_IN_PRIMARY; i++)
    {
        behavior = UNPACK_BEHAVIOR(tileset->metatileAttributes[i]);
        tiles = &tileset->metatiles[i * NUM_TILES_PER_METATILE];
        if (!IsOpenWater(behavior) || (tiles[0] | tiles[1] | tiles[2] | tiles[3]) == 0)
            continue;

        if (MetatileBehavior_IsSurfableFishableWater(behavior) && (tiles[4] | tiles[5] | tiles[6] | tiles[7]) == 0)
        {
            for (j = 0; j < NUM_BOTTOM_TILES; j++)
            {
                if (TILE_ID(tiles[j]) != 0)
                    SET_TILE(sWaterTiles.plainTiles, tiles[j]);
            }
        }

        for (j = 0; j < sWaterTiles.numBottoms; j++)
        {
            if (IsSameBottomLayer(sWaterTiles.bottoms[j], tiles))
                break;
        }
        if (j == sWaterTiles.numBottoms && j < MAX_WATER_BOTTOMS)
        {
            for (j = 0; j < NUM_BOTTOM_TILES; j++)
                sWaterTiles.bottoms[sWaterTiles.numBottoms][j] = tiles[j];
            SET_TILE(sWaterTiles.firstTiles, tiles[0]);
            sWaterTiles.numBottoms++;
        }
    }
}

// Whether a bottom layer is one that the primary tileset's open water uses.
static bool32 IsWaterBottomLayer(const u16 *tiles)
{
    u32 i;

    if (!TEST_TILE(sWaterTiles.firstTiles, tiles[0]))
        return FALSE;
    for (i = 0; i < sWaterTiles.numBottoms; i++)
    {
        if (IsSameBottomLayer(sWaterTiles.bottoms[i], tiles))
            return TRUE;
    }
    return FALSE;
}

// What GetMetatileDrawTiles has worked out for each metatile, for the tilesets and the type of
// map it was last asked about. It's asked for every metatile that scrolls into view (16 a step).
#define DRAW_LAYER_UNKNOWN    0xFF
#define DRAW_LAYER_MOVE_WATER 0x80 // Besides the layer type: covered water, whose plain water tiles go on the bottom layer
static EWRAM_DATA struct
{
    const struct Tileset *primaryTileset;
    const struct Tileset *secondaryTileset;
    u8 mapType;
    u8 layers[NUM_METATILES_TOTAL];
} sDrawLayers = {0};

// The layer type a metatile is drawn with (and DRAW_LAYER_MOVE_WATER). See GetMetatileDrawTiles.
static u32 GetMetatileDrawLayer(u32 metatileId, const u16 *tiles)
{
    const struct MapLayout *layout = gMapHeader.mapLayout;
    u32 attributes = GetMetatileAttributesById(metatileId);
    u32 layerType = UNPACK_LAYER_TYPE(attributes);
    u32 behavior = UNPACK_BEHAVIOR(attributes);
    u32 i;

    if (!MapHasNaturalLight(gMapHeader.mapType))
        return layerType;
    if (sWaterTiles.tileset != layout->primaryTileset)
        LoadWaterTiles(layout->primaryTileset);

    if (layerType == METATILE_LAYER_TYPE_NORMAL)
    {
        // An empty bottom layer would leave a hole on BG3, which normal metatiles fill.
        if ((tiles[0] | tiles[1] | tiles[2] | tiles[3]) != 0
         && (IsWaterDrawnUnderSprites(behavior) || IsWaterBottomLayer(tiles)))
            layerType = METATILE_LAYER_TYPE_SPLIT;
    }
    else if (layerType == METATILE_LAYER_TYPE_COVERED && IsWaterDrawnUnderSprites(behavior))
    {
        for (i = 0; i < NUM_BOTTOM_TILES; i++)
        {
            if (TEST_TILE(sWaterTiles.plainTiles, tiles[NUM_BOTTOM_TILES + i]))
                return layerType | DRAW_LAYER_MOVE_WATER;
        }
    }
    return layerType;
}

// Copies a metatile's tiles to dest the way they're drawn and returns the layer type they're
// drawn with. Outdoors, water is moved from BG2 down to BG3, under priority 3 sprites (sky
// reflections like WEATHER_SUNNY_CLOUDS' clouds and object reflections):
//  - METATILE_LAYER_TYPE_NORMAL metatiles are drawn as split if they're water, or their bottom
//    layer is one that open water uses (rocks and buildings standing in water).
//  - Covered water metatiles get the plain water tiles of their top layer (the calm patches
//    in currents) drawn on the bottom layer, in place of the tiles they hide.
// Nothing else draws between those layers, so the map looks the same otherwise.
u32 GetMetatileDrawTiles(u32 metatileId, u16 *dest)
{
    const struct MapLayout *layout = gMapHeader.mapLayout;
    u32 layer, i;
    const u16 *tiles;

    if (metatileId >= NUM_METATILES_TOTAL)
        metatileId = 0;
    if (metatileId < NUM_METATILES_IN_PRIMARY)
        tiles = &layout->primaryTileset->metatiles[metatileId * NUM_TILES_PER_METATILE];
    else
        tiles = &layout->secondaryTileset->metatiles[(metatileId - NUM_METATILES_IN_PRIMARY) * NUM_TILES_PER_METATILE];
    for (i = 0; i < NUM_TILES_PER_METATILE; i++)
        dest[i] = tiles[i];

    if (sDrawLayers.primaryTileset != layout->primaryTileset
     || sDrawLayers.secondaryTileset != layout->secondaryTileset
     || sDrawLayers.mapType != gMapHeader.mapType)
    {
        sDrawLayers.primaryTileset = layout->primaryTileset;
        sDrawLayers.secondaryTileset = layout->secondaryTileset;
        sDrawLayers.mapType = gMapHeader.mapType;
        memset(sDrawLayers.layers, DRAW_LAYER_UNKNOWN, sizeof(sDrawLayers.layers));
    }
    layer = sDrawLayers.layers[metatileId];
    if (layer == DRAW_LAYER_UNKNOWN)
    {
        layer = GetMetatileDrawLayer(metatileId, tiles);
        sDrawLayers.layers[metatileId] = layer;
    }

    if (layer & DRAW_LAYER_MOVE_WATER)
    {
        for (i = 0; i < NUM_BOTTOM_TILES; i++)
        {
            if (TEST_TILE(sWaterTiles.plainTiles, tiles[NUM_BOTTOM_TILES + i]))
            {
                dest[i] = tiles[NUM_BOTTOM_TILES + i];
                dest[NUM_BOTTOM_TILES + i] = 0;
            }
        }
        layer &= ~DRAW_LAYER_MOVE_WATER;
    }
    return layer;
}

void MapGridSetMetatileIdAt(s32 x, s32 y, u16 metatile)
{
    if (AreCoordsWithinMapGridBounds(x, y))
    {
        // Elevation is ignored in the argument, but copy metatile ID and collision
        gBackupMapLayout.map[x + y * gBackupMapLayout.width] &= MAPGRID_ELEVATION_MASK;
        gBackupMapLayout.map[x + y * gBackupMapLayout.width] |= metatile & ~MAPGRID_ELEVATION_MASK;
    }
}

void MapGridSetMetatileEntryAt(s32 x, s32 y, u16 metatile)
{
    if (AreCoordsWithinMapGridBounds(x, y))
    {
        gBackupMapLayout.map[x + gBackupMapLayout.width * y] = metatile;
    }
}

u16 GetMetatileAttributesById(u16 metatile)
{
    if (metatile < NUM_METATILES_IN_PRIMARY)
    {
        return gMapHeader.mapLayout->primaryTileset->metatileAttributes[metatile];
    }
    else if (metatile < NUM_METATILES_TOTAL)
    {
        return gMapHeader.mapLayout->secondaryTileset->metatileAttributes[metatile - NUM_METATILES_IN_PRIMARY];
    }
    else
    {
        return MB_INVALID;
    }
}

void SaveMapView(void)
{
    s32 i, j;
    s32 x, y;
    u16 *mapView;
    s32 width;
    mapView = gSaveBlock1Ptr->mapView;
    width = gBackupMapLayout.width;
    x = gSaveBlock1Ptr->pos.x;
    y = gSaveBlock1Ptr->pos.y;
    for (i = y; i < y + MAP_OFFSET_H; i++)
    {
        for (j = x; j < x + MAP_OFFSET_W; j++)
            *mapView++ = sBackupMapData[width * i + j];
    }
}

static bool32 SavedMapViewIsEmpty(void)
{
    u16 i;
    u32 marker = 0;

#ifndef UBFIX
    for (i = 0; i < sizeof(gSaveBlock1Ptr->mapView); i++)
        marker |= gSaveBlock1Ptr->mapView[i];
#else
    for (i = 0; i < ARRAY_COUNT(gSaveBlock1Ptr->mapView); i++)
        marker |= gSaveBlock1Ptr->mapView[i];
#endif


    if (marker == 0)
        return TRUE;
    else
        return FALSE;
}

static void ClearSavedMapView(void)
{
    CpuFill16(0, gSaveBlock1Ptr->mapView, sizeof(gSaveBlock1Ptr->mapView));
}

static void LoadSavedMapView(void)
{
    u8 yMode;
    s32 i, j;
    s32 x, y;
    u16 *mapView;
    s32 width;
    mapView = gSaveBlock1Ptr->mapView;
    if (SavedMapViewIsEmpty())
        return;

    width = gBackupMapLayout.width;
    x = gSaveBlock1Ptr->pos.x;
    y = gSaveBlock1Ptr->pos.y;
    for (i = y; i < y + MAP_OFFSET_H; i++)
    {
        if (i == y && i != 0)
            yMode = 0;
        else if (i == y + MAP_OFFSET_H - 1 && i != gMapHeader.mapLayout->height - 1)
            yMode = 1;
        else
            yMode = 0xFF;

        for (j = x; j < x + MAP_OFFSET_W; j++)
        {
            if (!SkipCopyingMetatileFromSavedMap(&sBackupMapData[j + width * i], width, yMode))
                sBackupMapData[j + width * i] = *mapView;
            mapView++;
        }
    }
    for (j = x; j < x + MAP_OFFSET_W; j++)
    {
        if (y != 0)
            FixLongGrassMetatilesWindowTop(j, y - 1);
        if (i < gMapHeader.mapLayout->height - 1)
            FixLongGrassMetatilesWindowBottom(j, y + MAP_OFFSET_H - 1);
    }
    ClearSavedMapView();
}

static void MoveMapViewToBackup(u8 direction)
{
    s32 width;
    s32 x0, y0;
    s32 x2, y2;
    s32 x, y;
    s32 i, j;

    u16 *mapView = gSaveBlock1Ptr->mapView;
    
    width = gBackupMapLayout.width;
    i = 0;
    j = 0;
    x0 = gSaveBlock1Ptr->pos.x;
    y0 = gSaveBlock1Ptr->pos.y;
    x2 = MAP_OFFSET_W;
    y2 = MAP_OFFSET_H;

    switch (direction)
    {
    case CONNECTION_NORTH:
        y0++;
        y2 = MAP_OFFSET_H - 1;
        break;
    case CONNECTION_SOUTH:
        j = 1;
        y2 = MAP_OFFSET_H - 1;
        break;
    case CONNECTION_WEST:
        x0++;
        x2 = MAP_OFFSET_W - 1;
        break;
    case CONNECTION_EAST:
        i = 1;
        x2 = MAP_OFFSET_W - 1;
        break;
    }

    for (y = 0; y < y2; y++)
    {
        for (x = 0; x < x2; x++)
        {
            sBackupMapData[x + x0 + width * (y + y0)] = mapView[i + x + MAP_OFFSET_W * (j + y)];
        }
    }

    ClearSavedMapView();
}

s32 GetMapBorderIdAt(s32 x, s32 y)
{
    if (GetMapGridBlockAt(x, y) == MAPGRID_UNDEFINED)
        return CONNECTION_INVALID;

    if (x >= (gBackupMapLayout.width - (MAP_OFFSET + 1)))
    {
        if (!sMapConnectionFlags.east)
            return CONNECTION_INVALID;

        return CONNECTION_EAST;
    }
    else if (x < MAP_OFFSET)
    {
        if (!sMapConnectionFlags.west)
            return CONNECTION_INVALID;

        return CONNECTION_WEST;
    }
    else if (y >= (gBackupMapLayout.height - MAP_OFFSET))
    {
        if (!sMapConnectionFlags.south)
            return CONNECTION_INVALID;

        return CONNECTION_SOUTH;
    }
    else if (y < MAP_OFFSET)
    {
        if (!sMapConnectionFlags.north)
            return CONNECTION_INVALID;

        return CONNECTION_NORTH;
    }
    else
    {
        return CONNECTION_NONE;
    }
}

s32 GetPostCameraMoveMapBorderId(s32 x, s32 y)
{
    return GetMapBorderIdAt(gSaveBlock1Ptr->pos.x + MAP_OFFSET + x, gSaveBlock1Ptr->pos.y + MAP_OFFSET + y);
}

bool32 CanCameraMoveInDirection(s32 direction)
{
    s32 x, y;
    x = gSaveBlock1Ptr->pos.x + MAP_OFFSET + gDirectionToVectors[direction].x;
    y = gSaveBlock1Ptr->pos.y + MAP_OFFSET + gDirectionToVectors[direction].y;

    if (GetMapBorderIdAt(x, y) == CONNECTION_INVALID)
        return FALSE;

    return TRUE;
}

static void SetPositionFromConnection(const struct MapConnection *connection, s32 direction, s32 x, s32 y)
{
    struct MapHeader const *mapHeader;
    mapHeader = GetMapHeaderFromConnection(connection);
    switch (direction)
    {
    case CONNECTION_EAST:
        gSaveBlock1Ptr->pos.x = -x;
        gSaveBlock1Ptr->pos.y -= connection->offset;
        break;
    case CONNECTION_WEST:
        gSaveBlock1Ptr->pos.x = mapHeader->mapLayout->width;
        gSaveBlock1Ptr->pos.y -= connection->offset;
        break;
    case CONNECTION_SOUTH:
        gSaveBlock1Ptr->pos.x -= connection->offset;
        gSaveBlock1Ptr->pos.y = -y;
        break;
    case CONNECTION_NORTH:
        gSaveBlock1Ptr->pos.x -= connection->offset;
        gSaveBlock1Ptr->pos.y = mapHeader->mapLayout->height;
        break;
    }
}

bool8 CameraMove(s32 x, s32 y)
{
    s32 direction;
    const struct MapConnection *connection;
    s32 old_x, old_y;
    gCamera.active = FALSE;
    direction = GetPostCameraMoveMapBorderId(x, y);
    if (direction == CONNECTION_NONE || direction == CONNECTION_INVALID)
    {
        gSaveBlock1Ptr->pos.x += x;
        gSaveBlock1Ptr->pos.y += y;
    }
    else
    {
        SaveMapView();
        ClearMirageTowerPulseBlendEffect();
        old_x = gSaveBlock1Ptr->pos.x;
        old_y = gSaveBlock1Ptr->pos.y;
        connection = GetIncomingConnection(direction, gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y);
        SetPositionFromConnection(connection, direction, x, y);
        LoadMapFromCameraTransition(connection->mapGroup, connection->mapNum);
        gCamera.active = TRUE;
        gCamera.x = old_x - gSaveBlock1Ptr->pos.x;
        gCamera.y = old_y - gSaveBlock1Ptr->pos.y;
        gSaveBlock1Ptr->pos.x += x;
        gSaveBlock1Ptr->pos.y += y;
        MoveMapViewToBackup(direction);
    }
    return gCamera.active;
}

static const struct MapConnection *GetIncomingConnection(u8 direction, s32 x, s32 y)
{
    s32 count;
    s32 i;
    const struct MapConnection *connection;
    const struct MapConnections *connections = gMapHeader.connections;

#ifdef UBFIX // UB: Multiple possible null dereferences
    if (connections == NULL || connections->connections == NULL)
        return NULL;
#endif
    count = connections->count;
    connection = connections->connections;
    for (i = 0; i < count; i++, connection++)
    {
        if (connection->direction == direction && IsPosInIncomingConnectingMap(direction, x, y, connection) == TRUE)
            return connection;
    }
    return NULL;
}

static bool8 IsPosInIncomingConnectingMap(u8 direction, s32 x, s32 y, const struct MapConnection *connection)
{
    struct MapHeader const *mapHeader;
    mapHeader = GetMapHeaderFromConnection(connection);
    switch (direction)
    {
    case CONNECTION_SOUTH:
    case CONNECTION_NORTH:
        return IsCoordInIncomingConnectingMap(x, gMapHeader.mapLayout->width, mapHeader->mapLayout->width, connection->offset);
    case CONNECTION_WEST:
    case CONNECTION_EAST:
        return IsCoordInIncomingConnectingMap(y, gMapHeader.mapLayout->height, mapHeader->mapLayout->height, connection->offset);
    }
    return FALSE;
}

static bool8 IsCoordInIncomingConnectingMap(s32 coord, s32 srcMax, s32 destMax, s32 offset)
{
    s32 min, max;

    if (offset < 0)
        min = 0;
    else
        min = offset;

    if (destMax + offset < srcMax)
        max = destMax + offset;
    else
        max = srcMax;

    if (min <= coord && coord <= max)
        return TRUE;

    return FALSE;
}

static s32 IsCoordInConnectingMap(s32 coord, s32 max)
{
    if (coord >= 0 && coord < max)
        return TRUE;

    return FALSE;
}

static s32 IsPosInConnectingMap(const struct MapConnection *connection, s32 x, s32 y)
{
    struct MapHeader const *mapHeader;
    mapHeader = GetMapHeaderFromConnection(connection);
    switch (connection->direction)
    {
    case CONNECTION_SOUTH:
    case CONNECTION_NORTH:
        return IsCoordInConnectingMap(x - connection->offset, mapHeader->mapLayout->width);
    case CONNECTION_WEST:
    case CONNECTION_EAST:
        return IsCoordInConnectingMap(y - connection->offset, mapHeader->mapLayout->height);
    }
    return FALSE;
}

const struct MapConnection *GetMapConnectionAtPos(s16 x, s16 y)
{
    s32 count;
    const struct MapConnection *connection;
    s32 i;
    u8 direction;
    if (!gMapHeader.connections)
    {
        return NULL;
    }

    count = gMapHeader.connections->count;
    connection = gMapHeader.connections->connections;
    for (i = 0; i < count; i++, connection++)
    {
        direction = connection->direction;
        if (direction == CONNECTION_DIVE || direction == CONNECTION_EMERGE)
            continue;
        else if (direction == CONNECTION_NORTH && y > MAP_OFFSET - 1)
            continue;
        else if (direction == CONNECTION_SOUTH && y < gMapHeader.mapLayout->height + MAP_OFFSET)
            continue;
        else if (direction == CONNECTION_WEST && x > MAP_OFFSET - 1)
            continue;
        else if (direction == CONNECTION_EAST && x < gMapHeader.mapLayout->width + MAP_OFFSET)
            continue;

        if (IsPosInConnectingMap(connection, x - MAP_OFFSET, y - MAP_OFFSET) == TRUE)
            return connection;
    }
    return NULL;
}

void SetCameraFocusCoords(u16 x, u16 y)
{
    gSaveBlock1Ptr->pos.x = x - MAP_OFFSET;
    gSaveBlock1Ptr->pos.y = y - MAP_OFFSET;
}

void GetCameraFocusCoords(u16 *x, u16 *y)
{
    *x = gSaveBlock1Ptr->pos.x + MAP_OFFSET;
    *y = gSaveBlock1Ptr->pos.y + MAP_OFFSET;
}

static void UNUSED SetCameraCoords(u16 x, u16 y)
{
    gSaveBlock1Ptr->pos.x = x;
    gSaveBlock1Ptr->pos.y = y;
}

void GetCameraCoords(u16 *x, u16 *y)
{
    *x = gSaveBlock1Ptr->pos.x;
    *y = gSaveBlock1Ptr->pos.y;
}

void MapGridSetMetatileImpassabilityAt(s32 x, s32 y, bool32 impassable)
{
    if (AreCoordsWithinMapGridBounds(x, y))
    {
        if (impassable)
            gBackupMapLayout.map[x + gBackupMapLayout.width * y] |= MAPGRID_COLLISION_MASK;
        else
            gBackupMapLayout.map[x + gBackupMapLayout.width * y] &= ~MAPGRID_COLLISION_MASK;
    }
}

static bool8 SkipCopyingMetatileFromSavedMap(u16 *mapBlock, u16 mapWidth, u8 yMode)
{
    if (yMode == 0xFF)
        return FALSE;

    if (yMode == 0)
        mapBlock -= mapWidth;
    else
        mapBlock += mapWidth;

    if (IsLargeBreakableDecoration(UNPACK_METATILE(*mapBlock), yMode) == TRUE)
        return TRUE;
    return FALSE;
}

static void CopyTilesetToVram(struct Tileset const *tileset, u16 numTiles, u16 offset)
{
    if (tileset)
    {
        if (!tileset->isCompressed)
            LoadBgTiles(2, GetTilesetTiles(tileset), numTiles * 32, offset);
        else
            DecompressAndCopyTileDataToVram(2, GetTilesetTiles(tileset), numTiles * 32, offset, 0);
    }
}

static void CopyTilesetToVramUsingHeap(struct Tileset const *tileset, u16 numTiles, u16 offset)
{
    if (tileset)
    {
        if (!tileset->isCompressed)
            LoadBgTiles(2, GetTilesetTiles(tileset), numTiles * 32, offset);
        else
            DecompressAndLoadBgGfxUsingHeap(2, GetTilesetTiles(tileset), numTiles * 32, offset, 0);
    }
}

// Below two are dummied functions from FRLG, used to tint the overworld palettes for the Quest Log
static void ApplyGlobalTintToPaletteEntries(u16 offset, u16 size)
{

}

static void UNUSED ApplyGlobalTintToPaletteSlot(u8 slot, u8 count)
{

}

static void LoadTilesetPalette(struct Tileset const *tileset, u16 destOffset, u16 size, bool8 skipFaded)
{
    u32 low = 0;
    u32 high = 0;

    if (tileset)
    {
        if (tileset->isSecondary == FALSE)
        {
            // LoadPalette(&black, destOffset, 2);
            if (skipFaded)
                CpuFastCopy(tileset->palettes, &gPlttBufferUnfaded[destOffset], size); // always word-aligned
            else
                LoadPaletteFast(tileset->palettes, destOffset, size);
            LoadSeasonalTilesetPalettes(tileset, 0, NUM_PALS_IN_PRIMARY, skipFaded);
            gPlttBufferFaded[destOffset] = gPlttBufferUnfaded[destOffset] = RGB_BLACK; // why does it have to be black?
            ApplyGlobalTintToPaletteEntries(destOffset + 1, (size - 2) >> 1);
            low = 0;
            high = NUM_PALS_IN_PRIMARY;
        }
        else if (tileset->isSecondary == TRUE)
        {
            // All 'gTilesetPalettes_' arrays should have ALIGNED(4) in them,
            // but we use SmartCopy here just in case they don't
            if (skipFaded)
                #ifdef CpuSmartCopy16
                CpuSmartCopy16(tileset->palettes[NUM_PALS_IN_PRIMARY], &gPlttBufferUnfaded[destOffset], size);
                #else
                CpuCopy16(tileset->palettes[NUM_PALS_IN_PRIMARY], &gPlttBufferUnfaded[destOffset], size);
                #endif
            else
                LoadPaletteFast(tileset->palettes[NUM_PALS_IN_PRIMARY], destOffset, size);
            LoadSeasonalTilesetPalettes(tileset, NUM_PALS_IN_PRIMARY, NUM_PALS_TOTAL - NUM_PALS_IN_PRIMARY, skipFaded);
            low = NUM_PALS_IN_PRIMARY;
            high = NUM_PALS_TOTAL;
        }
        else
        {
            LoadCompressedPalette((const u32 *)tileset->palettes, destOffset, size);
            ApplyGlobalTintToPaletteEntries(destOffset, size >> 1);
        }
        // convert legacy light palette system to current
        if (tileset->lightPalettes) {
            u32 i, j, color;
            for (i = low; i < high; i++) {
                if (tileset->lightPalettes & (1 << (i - low))) { // Mark light colors
                    for (j = 1, color = gPlttBufferUnfaded[PLTT_ID(i)]; j < 16 && color; j++, color >>= 1) {
                        if (color & 1)
                            gPlttBufferFaded[PLTT_ID(i)+j] = gPlttBufferUnfaded[PLTT_ID(i)+j] |= RGB_ALPHA;
                    }
                    if (tileset->customLightColor & (1 << (i - low))) // Copy old custom light color to index 0
                        gPlttBufferFaded[PLTT_ID(i)] = gPlttBufferUnfaded[PLTT_ID(i)] = gPlttBufferUnfaded[PLTT_ID(i)+15] | RGB_ALPHA;
                }
            }
        }
    }
}

void CopyPrimaryTilesetToVram(struct MapLayout const *mapLayout)
{
    CopyTilesetToVram(mapLayout->primaryTileset, NUM_TILES_IN_PRIMARY, 0);
}

void CopySecondaryTilesetToVram(struct MapLayout const *mapLayout)
{
    CopyTilesetToVram(mapLayout->secondaryTileset, NUM_TILES_TOTAL - NUM_TILES_IN_PRIMARY, NUM_TILES_IN_PRIMARY);
}

void CopySecondaryTilesetToVramUsingHeap(struct MapLayout const *mapLayout)
{
    CopyTilesetToVramUsingHeap(mapLayout->secondaryTileset, NUM_TILES_TOTAL - NUM_TILES_IN_PRIMARY, NUM_TILES_IN_PRIMARY);
}

static void LoadPrimaryTilesetPalette(struct MapLayout const *mapLayout)
{
    LoadTilesetPalette(mapLayout->primaryTileset, 0, NUM_PALS_IN_PRIMARY * PLTT_SIZE_4BPP, FALSE);
}

void LoadSecondaryTilesetPalette(struct MapLayout const *mapLayout, bool8 skipFaded)
{
    LoadTilesetPalette(mapLayout->secondaryTileset, NUM_PALS_IN_PRIMARY * 16, (NUM_PALS_TOTAL - NUM_PALS_IN_PRIMARY) * PLTT_SIZE_4BPP, skipFaded);
}

void CopyMapTilesetsToVram(struct MapLayout const *mapLayout)
{
    if (mapLayout)
    {
        CopyTilesetToVramUsingHeap(mapLayout->primaryTileset, NUM_TILES_IN_PRIMARY, 0);
        CopyTilesetToVramUsingHeap(mapLayout->secondaryTileset, NUM_TILES_TOTAL - NUM_TILES_IN_PRIMARY, NUM_TILES_IN_PRIMARY);
    }
}

void LoadMapTilesetPalettes(struct MapLayout const *mapLayout)
{
    if (mapLayout)
    {
        LoadPrimaryTilesetPalette(mapLayout);
        LoadSecondaryTilesetPalette(mapLayout, FALSE);
    }
}

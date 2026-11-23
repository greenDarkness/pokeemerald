#include "global.h"
#include "battle_anim.h"
#include "event_object_movement.h"
#include "field_camera.h"
#include "fieldmap.h"
#include "field_season.h"
#include "field_weather.h"
#include "overworld.h"
#include "random.h"
#include "script.h"
#include "constants/weather.h"
#include "constants/songs.h"
#include "constants/rgb.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "time_events.h"
#include "trig.h"
#include "gpu_regs.h"
#include "palette.h"
#include "dma3.h"
#include "malloc.h"
#include "metatile_behavior.h"
#include "tileset_anims.h"
#include "constants/metatile_behaviors.h"

EWRAM_DATA static u8 sCurrentAbnormalWeather = 0;
EWRAM_DATA static u16 sUnusedWeatherRelated = 0;

const u16 gCloudsWeatherPalette[] = INCGFX_U16("graphics/weather/cloud.png", ".gbapal");
const u16 gSandstormWeatherPalette[] = INCGFX_U16("graphics/weather/sandstorm.png", ".gbapal");
const u8 gWeatherFogDiagonalTiles[] = INCGFX_U8("graphics/weather/fog_diagonal.png", ".4bpp");
const u8 gWeatherFogHorizontalTiles[] = INCGFX_U8("graphics/weather/fog_horizontal.png", ".4bpp");
const u8 ALIGNED(4) gWeatherCloudTiles[] = INCGFX_U8("graphics/weather/cloud.png", ".4bpp");
const u8 gWeatherSnowTiles[] = INCGFX_U8("graphics/weather/snow.png", ".4bpp");
const u8 gWeatherBlizzardTiles[] = INCGFX_U8("graphics/weather/blizzard.png", ".4bpp");
const u8 gWeatherBlizzardOverlayTiles[] = INCGFX_U8("graphics/weather/blizzard_overlay.png", ".4bpp");
const u8 gWeatherDiamondDustTiles[] = INCGFX_U8("graphics/weather/diamond_dust.png", ".4bpp");
const u8 gWeatherLeafTiles[] = INCGFX_U8("graphics/weather/leaf.png", ".4bpp");
const u8 gWeatherBubbleTiles[] = INCGFX_U8("graphics/weather/bubble.png", ".4bpp");
const u8 gWeatherAshTiles[] = INCGFX_U8("graphics/weather/ash.png", ".4bpp");
const u8 gWeatherRainTiles[] = INCGFX_U8("graphics/weather/rain.png", ".4bpp");
const u8 gWeatherSandstormTiles[] = INCGFX_U8("graphics/weather/sandstorm.png", ".4bpp");

//------------------------------------------------------------------------------
// WEATHER_SUNNY_CLOUDS
//------------------------------------------------------------------------------

static void CreateCloudSprites(void);
static void DestroyCloudSprites(void);
static void UpdateCloudSprite(struct Sprite *);

#define CLOUD_SPRITE_DIM    64
#define CLOUD_TILES_PER_ROW (CLOUD_SPRITE_DIM / 8)
#define CLOUD_TILE_WORDS    (CLOUD_SPRITE_DIM * CLOUD_SPRITE_DIM / 8) // 4bpp: 8 pixels per u32
#define CLOUD_CELLS         (CLOUD_SPRITE_DIM / 8 + 1) // 8x8 map tiles a cloud can overlap per axis
#define CLOUD_MASK_CACHE    248 // Leaves room in a u8 for the CLOUD_TILE_* values
// Tiles where fewer pixels than this look like water are treated as dry, so stray
// blue pixels on windows and roofs don't flicker as clouds pass.
#define CLOUD_MIN_WATER_PIXELS 12
#define CLOUD_NEAR_WATER_RANGE 2 // Reaches the middle of rock clusters sitting in water
// Frames to wait after a map change before reading tile graphics from VRAM, so a
// newly connected map's tileset has finished copying.
#define CLOUD_SETUP_DELAY 2
// Scanlines of setup work allowed per frame (a frame is 228). Spreading it out keeps it lagless.
#define CLOUD_SETUP_SCANLINES     12
#define CLOUD_MAX_MASKS_PER_FRAME 2
#define CLOUD_ANIM_QUEUE          8 // Tileset animation frames waiting to be added to the masks
#define CLOUD_MAX_ANIMS           12

// The cloud sprites sit between the map's bottom (BG3) and middle (BG2) layers, so the
// hardware alone shows them on any metatile with a see-through middle layer, including
// grass and ledges. Each cloud gets its own copy of the graphics, cleared everywhere
// the bottom layer isn't water. The bottom layer is the one that's drawn, which has more
// of the map's water on it than the metatiles say (see GetMetatileDrawTiles). Water colors
// are the blue colors used by deep water in the primary tileset or on the map. Bottom-layer
// tiles are then sorted by what they're used for on the map:
//  - Puddles and water under bridges: water as a whole.
//  - Deep water: its water colored pixels, or all of it if they're too few to tell
//    (water that isn't blue). Sand and rocks sharing a tile with the sea stay dry.
//  - Anything else (shallow water, shorelines, rocks in water, waterfalls): its water
//    colored pixels, if there are at least CLOUD_MIN_WATER_PIXELS of them and the
//    metatile is within CLOUD_NEAR_WATER_RANGE metatiles of water.
// Animated tiles (waves, foam around rocks) get the water pixels of every frame of their
// animation, collected as the frames play (Clouds_OnTilesetAnim).
struct CloudSpriteBuffers
{
    u32 tiles[NUM_CLOUD_SPRITES][CLOUD_TILE_WORDS];
    struct SpriteFrameImage images[NUM_CLOUD_SPRITES];
    u32 deepWaterTiles[NUM_TILES_TOTAL / 32];
    u32 alwaysWaterTiles[NUM_TILES_TOTAL / 32];
    u32 colorTilesSeen[NUM_TILES_TOTAL / 32];
    u32 metatilesSeen[NUM_METATILES_TOTAL / 32];
    u16 waterColors[16];                    // Per palette slot: bit n set if color n is water
    u8 tileClass[NUM_TILES_TOTAL];          // CLOUD_TILE_*, or an index into tileMasks
    u8 metatileCells[NUM_METATILES_TOTAL];  // CLOUD_CELL_* of each bottom-layer tile, 2 bits each
    u32 metatileCellsKnown[NUM_METATILES_TOTAL / 32];
    u32 metatileIsWater[NUM_METATILES_TOTAL / 32]; // Valid where metatileCellsKnown is set
    u8 tileMasks[CLOUD_MASK_CACHE][8];      // Per pixel row: bit n set if pixel n is water
    u8 tileMaskPalettes[CLOUD_MASK_CACHE];
    struct {
        const u16 *src;
        u16 firstTile;
        u16 numTiles;
    } animQueue[CLOUD_ANIM_QUEUE];
    struct {
        const u16 *firstSrc;
        u16 firstTile;
        bool8 sawOtherFrame;
        bool8 done; // Every frame has been added
    } anims[CLOUD_MAX_ANIMS];
    u8 animQueueCount;
    u8 animTileCursor; // Progress through the first queued frame
    u8 numAnims;
    // What's under each cloud, per 8x8 map cell. Only changes when a cloud crosses into new cells.
    struct {
        s16 firstCellX;
        s16 firstCellY;
        bool8 valid;
        u8 cellClass[CLOUD_CELLS][CLOUD_CELLS];
        u8 cellMasks[CLOUD_CELLS][CLOUD_CELLS][8];
    } cells[NUM_CLOUD_SPRITES];
    u8 numTileMasks;
    s16 maskedX[NUM_CLOUD_SPRITES];
    s16 maskedY[NUM_CLOUD_SPRITES];
    bool8 maskValid[NUM_CLOUD_SPRITES];
    u8 uploadedMask[NUM_CLOUD_SPRITES]; // CLOUD_MASK_* currently in the sprite's VRAM
    u8 setupStep;
    u8 setupDelay;
    u16 setupCursor;
    s8 mapGroup;
    s8 mapNum;
    const struct Tileset *primaryTileset;
    const struct Tileset *secondaryTileset;
};
static EWRAM_DATA struct CloudSpriteBuffers *sCloudBuffers = NULL;

// The clouds are positioned on the map's grid.
// These coordinates are for the lower half of Route 120.
static const struct Coords16 sCloudSpriteMapCoords[] =
{
    { 0, 66},
    { 5, 73},
    {10, 78},
};

static const struct OamData sCloudSpriteOamData =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_BLEND,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 3,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sCloudSpriteAnimCmd[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_END,
};

static const union AnimCmd *const sCloudSpriteAnimCmds[] =
{
    sCloudSpriteAnimCmd,
};

static const struct SpriteTemplate sCloudSpriteTemplate =
{
    .tileTag = TAG_NONE, // each cloud owns its tiles; .images is set per sprite
    .paletteTag = PALTAG_WEATHER_2,
    .oam = &sCloudSpriteOamData,
    .anims = sCloudSpriteAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateCloudSprite,
};

void Clouds_InitVars(void)
{
    gWeatherPtr->noShadows = FALSE;
    gWeatherPtr->targetColorMapIndex = 0;
    gWeatherPtr->colorMapStepDelay = 20;
    gWeatherPtr->weatherGfxLoaded = FALSE;
    gWeatherPtr->initStep = 0;
    if (gWeatherPtr->cloudSpritesCreated == FALSE)
        Weather_SetBlendCoeffs(0, 16);
}

void Clouds_InitAll(void)
{
    Clouds_InitVars();
    while (gWeatherPtr->weatherGfxLoaded == FALSE)
        Clouds_Main();
}

void Clouds_Main(void)
{
    switch (gWeatherPtr->initStep)
    {
    case 0:
        CreateCloudSprites();
        gWeatherPtr->initStep++;
        break;
    case 1:
        Weather_SetTargetBlendCoeffs(12, 8, 1);
        gWeatherPtr->initStep++;
        break;
    case 2:
        if (Weather_UpdateBlend())
        {
            gWeatherPtr->weatherGfxLoaded = TRUE;
            gWeatherPtr->initStep++;
        }
        break;
    }
}

bool8 Clouds_Finish(void)
{
    switch (gWeatherPtr->finishStep)
    {
    case 0:
        Weather_SetTargetBlendCoeffs(0, 16, 1);
        gWeatherPtr->finishStep++;
        return TRUE;
    case 1:
        if (Weather_UpdateBlend())
        {
            DestroyCloudSprites();
            gWeatherPtr->finishStep++;
        }
        return TRUE;
    }
    return FALSE;
}

void Sunny_InitVars(void)
{
    gWeatherPtr->targetColorMapIndex = 0;
    gWeatherPtr->colorMapStepDelay = 20;
    Weather_SetBlendCoeffs(8, 12);
    gWeatherPtr->noShadows = FALSE;
}

void Sunny_InitAll(void)
{
    Sunny_InitVars();
}

void Sunny_Main(void)
{
}

bool8 Sunny_Finish(void)
{
    return FALSE;
}

static void CreateCloudSprites(void)
{
    u16 i;
    u8 spriteId;
    struct Sprite *sprite;
    struct SpriteTemplate template;

    if (gWeatherPtr->cloudSpritesCreated == TRUE)
        return;

    // Zeroed, so clouds stay blank until Clouds_UpdateWaterMask fills them in.
    sCloudBuffers = AllocZeroed(sizeof(*sCloudBuffers));
    LoadCustomWeatherSpritePalette(gCloudsWeatherPalette);
    template = sCloudSpriteTemplate;
    for (i = 0; i < NUM_CLOUD_SPRITES; i++)
    {
        gWeatherPtr->sprites.s1.cloudSprites[i] = NULL;
        if (sCloudBuffers == NULL)
            continue;

        sCloudBuffers->images[i].data = sCloudBuffers->tiles[i];
        sCloudBuffers->images[i].size = sizeof(sCloudBuffers->tiles[i]);
        template.images = &sCloudBuffers->images[i];
        spriteId = CreateSprite(&template, 0, 0, 0xFF);
        if (spriteId != MAX_SPRITES)
        {
            gWeatherPtr->sprites.s1.cloudSprites[i] = &gSprites[spriteId];
            sprite = gWeatherPtr->sprites.s1.cloudSprites[i];
            SetSpritePosToMapCoords(sCloudSpriteMapCoords[i].x + MAP_OFFSET, sCloudSpriteMapCoords[i].y + MAP_OFFSET, &sprite->x, &sprite->y);
            sprite->coordOffsetEnabled = TRUE;
            // Stagger the 1-pixel steps so the clouds don't all need re-masking on the same frame.
            sprite->data[0] = i & 1;
        }
    }

    gWeatherPtr->cloudSpritesCreated = TRUE;
}

static void DestroyCloudSprites(void)
{
    u16 i;

    if (!gWeatherPtr->cloudSpritesCreated)
        return;

    for (i = 0; i < NUM_CLOUD_SPRITES; i++)
    {
        if (gWeatherPtr->sprites.s1.cloudSprites[i] != NULL)
            DestroySprite(gWeatherPtr->sprites.s1.cloudSprites[i]);
    }

    TRY_FREE_AND_SET_NULL(sCloudBuffers);
    gWeatherPtr->cloudSpritesCreated = FALSE;
}

static void UpdateCloudSprite(struct Sprite *sprite)
{
    // Move 1 pixel left every 2 frames.
    sprite->data[0] = (sprite->data[0] + 1) & 1;
    if (sprite->data[0])
        sprite->x--;
}

// Gets the metatile's tiles as they're drawn. Returns whether the first 4 are on BG3,
// underneath the clouds.
static bool32 GetMetatileBottomLayer(u32 metatileId, u16 *tiles)
{
    u32 layerType = GetMetatileDrawTiles(metatileId, tiles);

    return layerType == METATILE_LAYER_TYPE_COVERED || layerType == METATILE_LAYER_TYPE_SPLIT;
}

static bool32 IsCloudWaterBehavior(u32 behavior)
{
    return MetatileBehavior_IsSurfableWaterOrUnderwater(behavior)
        || MetatileBehavior_IsShallowFlowingWater(behavior)
        || behavior == MB_PUDDLE
        || behavior == MB_REFLECTION_UNDER_BRIDGE;
}

#define TEST_BIT(set, n) ((set)[(n) / 32] & (1 << ((n) % 32)))
#define SET_BIT(set, n)  ((set)[(n) / 32] |= 1 << ((n) % 32))

enum {
    CLOUD_TILE_UNCACHED = CLOUD_MASK_CACHE, // Partly water, mask recomputed on each use
    CLOUD_TILE_FULL,
    CLOUD_TILE_NONE,
    CLOUD_TILE_UNKNOWN = 0xFF,
};

static bool32 IsBluishColor(u32 color)
{
    u32 r = GET_R(color), g = GET_G(color), b = GET_B(color);

    return b >= 15 && b >= r + 4 && b >= g;
}

static const u16 *GetMapPalette(u32 paletteNum)
{
    if (paletteNum < NUM_PALS_IN_PRIMARY)
        return GetTilesetPalette(gMapHeader.mapLayout->primaryTileset, paletteNum);
    return GetTilesetPalette(gMapHeader.mapLayout->secondaryTileset, paletteNum);
}

static void AddCloudWaterColors(u32 entry)
{
    const u32 *tileGfx = (const u32 *)(BG_CHAR_ADDR(0) + (entry & (NUM_TILES_TOTAL - 1)) * TILE_SIZE_4BPP);
    u32 k, pixels, color;

    for (k = 0; k < 8; k++)
    {
        for (pixels = tileGfx[k]; pixels != 0; pixels >>= 4)
        {
            color = pixels & 0xF;
            if (color != 0)
                sCloudBuffers->waterColors[entry >> 12] |= 1 << color;
        }
    }
}

// onMap: the metatile is on the map, rather than just in the primary tileset.
static void AddCloudWaterMetatile(u32 metatileId, bool32 onMap)
{
    u32 behavior = UNPACK_BEHAVIOR(GetMetatileAttributesById(metatileId));
    bool32 isDeep = MetatileBehavior_IsSurfableFishableWater(behavior);
    bool32 isAlwaysWater = (behavior == MB_PUDDLE || behavior == MB_REFLECTION_UNDER_BRIDGE);
    u16 tiles[NUM_TILES_PER_METATILE];
    u32 i, tile;

    if (!(MetatileBehavior_IsSurfableWaterOrUnderwater(behavior) || isAlwaysWater) || !GetMetatileBottomLayer(metatileId, tiles))
        return;

    for (i = 0; i < NUM_TILES_PER_METATILE / 2; i++)
    {
        tile = tiles[i] & (NUM_TILES_TOTAL - 1);
        if (tile == 0)
            continue;
        if (onMap && isDeep)
            SET_BIT(sCloudBuffers->deepWaterTiles, tile);
        if (onMap && isAlwaysWater)
            SET_BIT(sCloudBuffers->alwaysWaterTiles, tile);
        if (!TEST_BIT(sCloudBuffers->colorTilesSeen, tile))
        {
            SET_BIT(sCloudBuffers->colorTilesSeen, tile);
            AddCloudWaterColors(tiles[i]);
        }
    }
}

// Writes the water pixels of 4bpp tile graphics to rows (unflipped) and returns how many there are.
static u32 ComputeWaterMaskFromGfx(const u16 *tileGfx, u32 palette, u8 *rows, u32 *numPixels)
{
    u32 waterColors = sCloudBuffers->waterColors[palette];
    u32 r, px, pixels, color, bits, total = 0, opaque = 0;

    for (r = 0; r < 8; r++)
    {
        pixels = tileGfx[r * 2] | (tileGfx[r * 2 + 1] << 16);
        bits = 0;
        for (px = 0; px < 8; px++, pixels >>= 4)
        {
            color = pixels & 0xF;
            if (color == 0)
                continue;
            opaque++;
            if (waterColors & (1 << color))
            {
                bits |= 1 << px;
                total++;
            }
        }
        rows[r] = bits;
    }
    if (numPixels != NULL)
        *numPixels = opaque;
    return total;
}

static u32 ComputeTileWaterMask(u32 entry, u8 *rows, u32 *numPixels)
{
    const u16 *tileGfx = (const u16 *)(BG_CHAR_ADDR(0) + (entry & (NUM_TILES_TOTAL - 1)) * TILE_SIZE_4BPP);

    return ComputeWaterMaskFromGfx(tileGfx, entry >> 12, rows, numPixels);
}

static u32 ClassifyCloudTile(u32 entry)
{
    u32 tile = entry & (NUM_TILES_TOTAL - 1);
    u32 numWater, numPixels, slot;
    u8 rows[8];

    if (tile == 0)
        return CLOUD_TILE_NONE;
    if (TEST_BIT(sCloudBuffers->alwaysWaterTiles, tile))
        return CLOUD_TILE_FULL;

    numWater = ComputeTileWaterMask(entry, rows, &numPixels);
    if (TEST_BIT(sCloudBuffers->deepWaterTiles, tile))
    {
        if (numWater == numPixels || numWater < CLOUD_MIN_WATER_PIXELS)
            return CLOUD_TILE_FULL;
    }
    else if (numWater < CLOUD_MIN_WATER_PIXELS)
    {
        return CLOUD_TILE_NONE;
    }

    if (sCloudBuffers->numTileMasks >= CLOUD_MASK_CACHE)
        return CLOUD_TILE_UNCACHED;
    slot = sCloudBuffers->numTileMasks++;
    memcpy(sCloudBuffers->tileMasks[slot], rows, sizeof(rows));
    sCloudBuffers->tileMaskPalettes[slot] = entry >> 12;
    return slot;
}

static u32 GetCloudTileClass(u32 entry)
{
    u32 tile = entry & (NUM_TILES_TOTAL - 1);

    if (sCloudBuffers->tileClass[tile] == CLOUD_TILE_UNKNOWN)
        sCloudBuffers->tileClass[tile] = ClassifyCloudTile(entry);
    return sCloudBuffers->tileClass[tile];
}

enum {
    CLOUD_CELL_NONE,
    CLOUD_CELL_FULL,
    CLOUD_CELL_PARTIAL,            // Deep water: its water pixels
    CLOUD_CELL_PARTIAL_NEAR_WATER, // Its water pixels, if the metatile is near water
};

static u32 GetCloudMetatileCells(u32 metatileId)
{
    u32 q, tileClass, cellClass, cells = 0;
    u16 tiles[NUM_TILES_PER_METATILE];

    if (TEST_BIT(sCloudBuffers->metatileCellsKnown, metatileId))
        return sCloudBuffers->metatileCells[metatileId];

    if (GetMetatileBottomLayer(metatileId, tiles))
    {
        for (q = 0; q < NUM_TILES_PER_METATILE / 2; q++)
        {
            tileClass = GetCloudTileClass(tiles[q]);
            if (tileClass == CLOUD_TILE_NONE)
                cellClass = CLOUD_CELL_NONE;
            else if (tileClass == CLOUD_TILE_FULL)
                cellClass = CLOUD_CELL_FULL;
            else if (TEST_BIT(sCloudBuffers->deepWaterTiles, tiles[q] & (NUM_TILES_TOTAL - 1)))
                cellClass = CLOUD_CELL_PARTIAL;
            else
                cellClass = CLOUD_CELL_PARTIAL_NEAR_WATER;
            cells |= cellClass << (q * 2);
        }
    }
    sCloudBuffers->metatileCells[metatileId] = cells;
    if (IsCloudWaterBehavior(UNPACK_BEHAVIOR(GetMetatileAttributesById(metatileId))))
        SET_BIT(sCloudBuffers->metatileIsWater, metatileId);
    SET_BIT(sCloudBuffers->metatileCellsKnown, metatileId);
    return cells;
}

enum {
    CLOUD_SETUP_START,
    CLOUD_SETUP_PRIMARY,
    CLOUD_SETUP_MAP,
    CLOUD_SETUP_COLORS,
    CLOUD_SETUP_CLASSIFY,
    CLOUD_SETUP_DONE,
};

static bool32 CloudSetupHasTime(u32 startLine)
{
    return (REG_VCOUNT + 228 - startLine) % 228 < CLOUD_SETUP_SCANLINES;
}

// Finds the map's water and classifies the tiles it uses, a little each frame.
static void RunCloudSetupStep(void)
{
    struct CloudSpriteBuffers *buf = sCloudBuffers;
    u32 count = gBackupMapLayout.width * gBackupMapLayout.height + 4;
    u32 startLine = REG_VCOUNT;
    u32 i, j, block, metatileId;

    switch (buf->setupStep)
    {
    case CLOUD_SETUP_START:
        if (buf->setupDelay != 0)
        {
            buf->setupDelay--;
            break;
        }
        CpuFill32(0, buf->deepWaterTiles, sizeof(buf->deepWaterTiles));
        CpuFill32(0, buf->alwaysWaterTiles, sizeof(buf->alwaysWaterTiles));
        CpuFill32(0, buf->colorTilesSeen, sizeof(buf->colorTilesSeen));
        CpuFill32(0, buf->metatilesSeen, sizeof(buf->metatilesSeen));
        CpuFill32(0, buf->waterColors, sizeof(buf->waterColors));
        CpuFill32(0xFFFFFFFF, buf->tileClass, sizeof(buf->tileClass));
        CpuFill32(0, buf->metatileCellsKnown, sizeof(buf->metatileCellsKnown));
        CpuFill32(0, buf->metatileIsWater, sizeof(buf->metatileIsWater));
        buf->numTileMasks = 0;
        buf->animQueueCount = 0;
        buf->animTileCursor = 0;
        buf->numAnims = 0;
        buf->mapGroup = gSaveBlock1Ptr->location.mapGroup;
        buf->mapNum = gSaveBlock1Ptr->location.mapNum;
        buf->primaryTileset = gMapHeader.mapLayout->primaryTileset;
        buf->secondaryTileset = gMapHeader.mapLayout->secondaryTileset;
        buf->setupCursor = 0;
        for (i = 0; i < NUM_CLOUD_SPRITES; i++)
            buf->cells[i].valid = FALSE;
        // Outdoor primary tilesets are always full, so all of their water can supply colors;
        // a map's own water may not use every shade that its shorelines and rocks do.
        buf->setupStep = MapHasNaturalLight(gMapHeader.mapType) ? CLOUD_SETUP_PRIMARY : CLOUD_SETUP_MAP;
        break;
    case CLOUD_SETUP_PRIMARY:
        while (buf->setupCursor < NUM_METATILES_IN_PRIMARY && CloudSetupHasTime(startLine))
            AddCloudWaterMetatile(buf->setupCursor++, FALSE);
        if (buf->setupCursor >= NUM_METATILES_IN_PRIMARY)
        {
            buf->setupCursor = 0;
            buf->setupStep = CLOUD_SETUP_MAP;
        }
        break;
    case CLOUD_SETUP_MAP:
        // The backup layout includes the edges of connected maps; the last 4 are the 2x2 border.
        for (; buf->setupCursor < count && CloudSetupHasTime(startLine); buf->setupCursor++)
        {
            i = buf->setupCursor;
            block = (i < count - 4) ? gBackupMapLayout.map[i] : gMapHeader.mapLayout->border[i - (count - 4)];
            if (block == MAPGRID_UNDEFINED)
                continue;
            metatileId = UNPACK_METATILE(block);
            if (TEST_BIT(buf->metatilesSeen, metatileId))
                continue;
            SET_BIT(buf->metatilesSeen, metatileId);
            AddCloudWaterMetatile(metatileId, TRUE);
        }
        if (buf->setupCursor >= count)
            buf->setupStep = CLOUD_SETUP_COLORS;
        break;
    case CLOUD_SETUP_COLORS:
        for (i = 0; i < 16; i++)
        {
            for (j = 1; j < 16; j++)
            {
                if ((buf->waterColors[i] & (1 << j)) && !IsBluishColor(GetMapPalette(i)[j]))
                    buf->waterColors[i] &= ~(1 << j);
            }
        }
        buf->setupCursor = 0;
        buf->setupStep = CLOUD_SETUP_CLASSIFY;
        break;
    case CLOUD_SETUP_CLASSIFY:
        // Classifying now means the masking each frame is only lookups.
        for (; buf->setupCursor < NUM_METATILES_TOTAL && CloudSetupHasTime(startLine); buf->setupCursor++)
        {
            metatileId = buf->setupCursor;
            if (TEST_BIT(buf->metatilesSeen, metatileId))
                GetCloudMetatileCells(metatileId);
        }
        if (buf->setupCursor >= NUM_METATILES_TOTAL)
        {
            for (i = 0; i < NUM_CLOUD_SPRITES; i++)
                buf->maskValid[i] = FALSE;
            buf->setupStep = CLOUD_SETUP_DONE;
        }
        break;
    }
}

static u32 ReverseBits8(u32 x)
{
    x = ((x & 0xF0) >> 4) | ((x & 0x0F) << 4);
    x = ((x & 0xCC) >> 2) | ((x & 0x33) << 2);
    x = ((x & 0xAA) >> 1) | ((x & 0x55) << 1);
    return x;
}

// Fills dest with the water pixels of a bottom-layer tilemap entry, flips applied.
static void FillCloudCellMask(u32 entry, u32 tileClass, u8 *dest)
{
    u8 rows[8];
    const u8 *src;
    u32 r;

    if (tileClass == CLOUD_TILE_NONE)
        return;
    if (tileClass == CLOUD_TILE_FULL)
    {
        memset(dest, 0xFF, 8);
        return;
    }
    if (tileClass == CLOUD_TILE_UNCACHED)
    {
        ComputeTileWaterMask(entry, rows, NULL);
        src = rows;
    }
    else
    {
        src = sCloudBuffers->tileMasks[tileClass];
    }

    for (r = 0; r < 8; r++)
        dest[(entry & 0x800) ? 7 - r : r] = (entry & 0x400) ? ReverseBits8(src[r]) : src[r];
}

// Bit n of an index set: nibble n (pixel n of a 4bpp tile row) set.
#define NIBBLES(b) ((((b) & 0x01) ? 0xFu : 0) | (((b) & 0x02) ? 0xF0u : 0) | (((b) & 0x04) ? 0xF00u : 0) | (((b) & 0x08) ? 0xF000u : 0) \
                  | (((b) & 0x10) ? 0xF0000u : 0) | (((b) & 0x20) ? 0xF00000u : 0) | (((b) & 0x40) ? 0xF000000u : 0) | (((b) & 0x80) ? 0xF0000000u : 0))
#define NIBBLES_4(b)  NIBBLES(b), NIBBLES((b) + 1), NIBBLES((b) + 2), NIBBLES((b) + 3)
#define NIBBLES_16(b) NIBBLES_4(b), NIBBLES_4((b) + 4), NIBBLES_4((b) + 8), NIBBLES_4((b) + 12)
#define NIBBLES_64(b) NIBBLES_16(b), NIBBLES_16((b) + 16), NIBBLES_16((b) + 32), NIBBLES_16((b) + 48)
static const u32 sPixelMaskToNibbles[256] = { NIBBLES_64(0), NIBBLES_64(64), NIBBLES_64(128), NIBBLES_64(192) };

enum {
    CLOUD_MASK_HIDDEN,  // No water under the cloud (also the sprite's initial, zeroed graphics)
    CLOUD_MASK_FULL,    // Only water under the cloud
    CLOUD_MASK_PARTIAL, // Masked copy written to the buffer
};

#define CLOUD_METATILES (CLOUD_CELLS / 2 + 1) // Metatiles a cloud can overlap per axis
#define CLOUD_NEAR_WATER_GRID (CLOUD_METATILES + 2 * CLOUD_NEAR_WATER_RANGE)

#define CLOUD_CELL_CACHE(cloudId) (&sCloudBuffers->cells[cloudId])

// Works out the class (and water pixels) of the 8x8 map cells in columns [colStart, colEnd)
// under a cloud whose top-left cell is (firstCellX, firstCellY).
static void BuildCloudCells(u32 cloudId, s32 firstCellX, s32 firstCellY, s32 colStart, s32 colEnd)
{
    u16 waterRows[CLOUD_NEAR_WATER_GRID]; // Bit n of row m: metatile (n, m) of the grid around the cells is water
    u8 (*cellClass)[CLOUD_CELLS] = CLOUD_CELL_CACHE(cloudId)->cellClass;
    u8 (*cellMasks)[CLOUD_CELLS][8] = CLOUD_CELL_CACHE(cloudId)->cellMasks;
    s32 firstMx = (firstCellX + colStart) >> 1, lastMx = (firstCellX + colEnd - 1) >> 1;
    s32 firstMy = firstCellY >> 1, lastMy = (firstCellY + CLOUD_CELLS - 1) >> 1;
    s32 gridWidth = lastMx - firstMx + 1 + 2 * CLOUD_NEAR_WATER_RANGE;
    s32 gridHeight = lastMy - firstMy + 1 + 2 * CLOUD_NEAR_WATER_RANGE;
    bool32 haveWaterRows = FALSE, haveTiles;
    s32 mx, my, cx, cy, gx, gy;
    u32 q, cells, cls, metatileId, nearMask;
    u16 tiles[NUM_TILES_PER_METATILE];

    for (my = firstMy; my <= lastMy; my++)
    {
        for (mx = firstMx; mx <= lastMx; mx++)
        {
            cells = GetCloudMetatileCells(MapGridGetMetatileIdAt(mx, my));
            haveTiles = FALSE;
            for (q = 0; q < NUM_TILES_PER_METATILE / 2; q++)
            {
                cx = mx * 2 + (q & 1) - firstCellX;
                cy = my * 2 + (q >> 1) - firstCellY;
                if (cx < colStart || cx >= colEnd || cy < 0 || cy >= CLOUD_CELLS)
                    continue;
                cls = (cells >> (q * 2)) & 3;
                if (cls == CLOUD_CELL_PARTIAL_NEAR_WATER)
                {
                    // Blue pixels away from water are windows, signs and the like.
                    if (!haveWaterRows)
                    {
                        for (gy = 0; gy < gridHeight; gy++)
                        {
                            waterRows[gy] = 0;
                            for (gx = 0; gx < gridWidth; gx++)
                            {
                                metatileId = MapGridGetMetatileIdAt(firstMx - CLOUD_NEAR_WATER_RANGE + gx, firstMy - CLOUD_NEAR_WATER_RANGE + gy);
                                GetCloudMetatileCells(metatileId);
                                if (TEST_BIT(sCloudBuffers->metatileIsWater, metatileId))
                                    waterRows[gy] |= 1 << gx;
                            }
                        }
                        haveWaterRows = TRUE;
                    }
                    nearMask = ((1 << (2 * CLOUD_NEAR_WATER_RANGE + 1)) - 1) << (mx - firstMx);
                    cls = CLOUD_CELL_NONE;
                    for (gy = my - firstMy; gy <= my - firstMy + 2 * CLOUD_NEAR_WATER_RANGE; gy++)
                    {
                        if (waterRows[gy] & nearMask)
                        {
                            cls = CLOUD_CELL_PARTIAL;
                            break;
                        }
                    }
                }
                // Every cell gets a mask so the per-row work later is plain lookups.
                if (cls == CLOUD_CELL_PARTIAL)
                {
                    if (!haveTiles)
                    {
                        GetMetatileDrawTiles(MapGridGetMetatileIdAt(mx, my), tiles);
                        haveTiles = TRUE;
                    }
                    FillCloudCellMask(tiles[q], sCloudBuffers->tileClass[tiles[q] & (NUM_TILES_TOTAL - 1)], cellMasks[cy][cx]);
                }
                else
                {
                    memset(cellMasks[cy][cx], cls == CLOUD_CELL_FULL ? 0xFF : 0, 8);
                }
                cellClass[cy][cx] = cls;
            }
        }
    }
    CLOUD_CELL_CACHE(cloudId)->firstCellX = firstCellX;
    CLOUD_CELL_CACHE(cloudId)->firstCellY = firstCellY;
    CLOUD_CELL_CACHE(cloudId)->valid = TRUE;
}

static bool32 CloudCellsNeedRebuild(u32 cloudId, s32 worldX, s32 worldY)
{
    return !CLOUD_CELL_CACHE(cloudId)->valid
        || CLOUD_CELL_CACHE(cloudId)->firstCellX != (worldX >> 3)
        || CLOUD_CELL_CACHE(cloudId)->firstCellY != (worldY >> 3);
}

static void UpdateCloudCells(u32 cloudId, s32 firstCellX, s32 firstCellY)
{
    u32 cy;

    if (CLOUD_CELL_CACHE(cloudId)->valid
     && CLOUD_CELL_CACHE(cloudId)->firstCellX == firstCellX
     && CLOUD_CELL_CACHE(cloudId)->firstCellY == firstCellY)
        return;

    // Clouds drift left, so usually only a new column of cells has come into view.
    if (CLOUD_CELL_CACHE(cloudId)->valid
     && CLOUD_CELL_CACHE(cloudId)->firstCellX == firstCellX + 1
     && CLOUD_CELL_CACHE(cloudId)->firstCellY == firstCellY)
    {
        for (cy = 0; cy < CLOUD_CELLS; cy++)
        {
            memmove(&CLOUD_CELL_CACHE(cloudId)->cellClass[cy][1], &CLOUD_CELL_CACHE(cloudId)->cellClass[cy][0], CLOUD_CELLS - 1);
            memmove(CLOUD_CELL_CACHE(cloudId)->cellMasks[cy][1], CLOUD_CELL_CACHE(cloudId)->cellMasks[cy][0], (CLOUD_CELLS - 1) * 8);
        }
        BuildCloudCells(cloudId, firstCellX, firstCellY, 0, 1);
        return;
    }

    BuildCloudCells(cloudId, firstCellX, firstCellY, 0, CLOUD_CELLS);
}
// worldX/worldY: the cloud's top-left in map pixels (MAP_OFFSET included).
static u32 MaskCloudToWater(u32 cloudId, s32 worldX, s32 worldY)
{
    u32 *dest = sCloudBuffers->tiles[cloudId];
    u8 (*cellClass)[CLOUD_CELLS] = CLOUD_CELL_CACHE(cloudId)->cellClass;
    u8 (*cellMasks)[CLOUD_CELLS][8] = CLOUD_CELL_CACHE(cloudId)->cellMasks;
    s32 firstCellX = worldX >> 3;
    s32 firstCellY = worldY >> 3;
    u32 offsetX = worldX & 7;
    u32 offsetY = worldY & 7;
    u32 usedCols = offsetX ? CLOUD_CELLS : CLOUD_CELLS - 1;
    u32 usedRows = offsetY ? CLOUD_CELLS : CLOUD_CELLS - 1;
    const u32 *src = (const u32 *)gWeatherCloudTiles;
    bool32 anyWater = FALSE, allWater = TRUE;
    u32 r, cx, cy, tileRow, tileCol, index, mapRow, bits, left, right, c0, c1, c2, c3;

    UpdateCloudCells(cloudId, firstCellX, firstCellY);


    for (cy = 0; cy < usedRows; cy++)
    {
        for (cx = 0; cx < usedCols; cx++)
        {
            if (cellClass[cy][cx] != CLOUD_CELL_NONE)
                anyWater = TRUE;
            if (cellClass[cy][cx] != CLOUD_CELL_FULL)
                allWater = FALSE;
        }
    }
    if (!anyWater)
        return CLOUD_MASK_HIDDEN;
    if (allWater)
        return CLOUD_MASK_FULL;

    // Each sprite tile covers up to 4 map cells: the end of cell (tileCol, tileRow), the
    // start of the next one over and the ones below them.
    for (tileRow = 0; tileRow < CLOUD_TILES_PER_ROW; tileRow++)
    {
        for (tileCol = 0; tileCol < CLOUD_TILES_PER_ROW; tileCol++)
        {
            index = (tileRow * CLOUD_TILES_PER_ROW + tileCol) * 8;
            c0 = cellClass[tileRow][tileCol];
            c1 = offsetX ? cellClass[tileRow][tileCol + 1] : c0;
            c2 = offsetY ? cellClass[tileRow + 1][tileCol] : c0;
            c3 = offsetY ? (offsetX ? cellClass[tileRow + 1][tileCol + 1] : c2) : c1;
            if (c0 == CLOUD_CELL_FULL && c1 == CLOUD_CELL_FULL && c2 == CLOUD_CELL_FULL && c3 == CLOUD_CELL_FULL)
            {
                for (r = 0; r < 8; r++)
                    dest[index + r] = src[index + r];
                continue;
            }
            if (c0 == CLOUD_CELL_NONE && c1 == CLOUD_CELL_NONE && c2 == CLOUD_CELL_NONE && c3 == CLOUD_CELL_NONE)
            {
                for (r = 0; r < 8; r++)
                    dest[index + r] = 0;
                continue;
            }
            for (r = 0; r < 8; r++)
            {
                mapRow = offsetY + r;
                cy = tileRow + (mapRow >> 3);
                left = cellMasks[cy][tileCol][mapRow & 7];
                right = cellMasks[cy][tileCol + 1][mapRow & 7]; // Shifted out entirely when offsetX is 0
                bits = ((left >> offsetX) | (right << (8 - offsetX))) & 0xFF;
                dest[index + r] = src[index + r] & sPixelMaskToNibbles[bits];
            }
        }
    }
    return CLOUD_MASK_PARTIAL;
}
// Called by the tileset animations with each frame they copy to VRAM. Only the frame's
// location is noted here; ProcessCloudAnimFrames adds its water pixels later.
void Clouds_OnTilesetAnim(const u16 *src, const u16 *dest, u16 size)
{
    struct CloudSpriteBuffers *buf = sCloudBuffers;
    u32 i, firstTile;

    if (buf == NULL || buf->setupStep != CLOUD_SETUP_DONE)
        return;
    if ((u32)dest < BG_CHAR_ADDR(0) || (u32)dest >= BG_CHAR_ADDR(0) + NUM_TILES_TOTAL * TILE_SIZE_4BPP)
        return;
    firstTile = ((u32)dest - BG_CHAR_ADDR(0)) / TILE_SIZE_4BPP;

    for (i = 0; i < buf->numAnims; i++)
    {
        if (buf->anims[i].firstTile == firstTile)
            break;
    }
    if (i == buf->numAnims)
    {
        if (i >= CLOUD_MAX_ANIMS)
            return;
        buf->anims[i].firstSrc = src;
        buf->anims[i].firstTile = firstTile;
        buf->anims[i].sawOtherFrame = FALSE;
        buf->anims[i].done = FALSE;
        buf->numAnims++;
    }
    else if (buf->anims[i].done)
    {
        return;
    }
    else if (src == buf->anims[i].firstSrc)
    {
        // Back to the first frame: the whole cycle has been seen.
        if (buf->anims[i].sawOtherFrame)
        {
            buf->anims[i].done = TRUE;
            return;
        }
    }
    else
    {
        buf->anims[i].sawOtherFrame = TRUE;
    }

    if (buf->animQueueCount < CLOUD_ANIM_QUEUE)
    {
        buf->animQueue[buf->animQueueCount].src = src;
        buf->animQueue[buf->animQueueCount].firstTile = firstTile;
        buf->animQueue[buf->animQueueCount].numTiles = size / TILE_SIZE_4BPP;
        buf->animQueueCount++;
    }
}

// Adds the water pixels of queued animation frames to the partly-water tiles they cover.
static void ProcessCloudAnimFrames(void)
{
    struct CloudSpriteBuffers *buf = sCloudBuffers;
    u32 startLine = REG_VCOUNT;
    u32 tile, tileClass, r, i;
    bool32 changed = FALSE;
    u8 rows[8];

    while (buf->animQueueCount != 0 && CloudSetupHasTime(startLine))
    {
        tile = buf->animQueue[0].firstTile + buf->animTileCursor;
        tileClass = buf->tileClass[tile];
        if (tileClass < CLOUD_MASK_CACHE)
        {
            ComputeWaterMaskFromGfx(buf->animQueue[0].src + buf->animTileCursor * (TILE_SIZE_4BPP / 2),
                                    buf->tileMaskPalettes[tileClass], rows, NULL);
            for (r = 0; r < 8; r++)
            {
                if (rows[r] & ~buf->tileMasks[tileClass][r])
                {
                    buf->tileMasks[tileClass][r] |= rows[r];
                    changed = TRUE;
                }
            }
        }
        if (++buf->animTileCursor >= buf->animQueue[0].numTiles)
        {
            buf->animTileCursor = 0;
            buf->animQueueCount--;
            for (i = 0; i < buf->animQueueCount; i++)
                buf->animQueue[i] = buf->animQueue[i + 1];
        }
    }

    // The clouds' cached cells hold copies of the masks.
    if (changed)
    {
        for (i = 0; i < NUM_CLOUD_SPRITES; i++)
        {
            buf->cells[i].valid = FALSE;
            buf->maskValid[i] = FALSE;
        }
    }
}

// Called each overworld frame after the camera has moved, so the mask matches the
// positions BuildOamBuffer is about to use.
void Clouds_UpdateWaterMask(void)

{
    u32 i, maskType, numMasked = 0;
    bool32 rebuiltCells = FALSE;
    s16 originX, originY;
    s32 screenX, screenY, worldX, worldY;
    struct Sprite *sprite;
    void *vram;

    if (!gWeatherPtr->cloudSpritesCreated || sCloudBuffers == NULL)
        return;

    // Walking into a connected map keeps the clouds but replaces the map grid. The setup is
    // only redone if the tileset changed; otherwise the old map's water is close enough.
    if (sCloudBuffers->setupStep == CLOUD_SETUP_DONE
     && (sCloudBuffers->mapGroup != gSaveBlock1Ptr->location.mapGroup
      || sCloudBuffers->mapNum != gSaveBlock1Ptr->location.mapNum))
    {
        sCloudBuffers->mapGroup = gSaveBlock1Ptr->location.mapGroup;
        sCloudBuffers->mapNum = gSaveBlock1Ptr->location.mapNum;
        for (i = 0; i < NUM_CLOUD_SPRITES; i++)
        {
            sCloudBuffers->maskValid[i] = FALSE;
            sCloudBuffers->cells[i].valid = FALSE;
        }
        if (sCloudBuffers->primaryTileset != gMapHeader.mapLayout->primaryTileset
         || sCloudBuffers->secondaryTileset != gMapHeader.mapLayout->secondaryTileset)
        {
            sCloudBuffers->setupStep = CLOUD_SETUP_START;
            sCloudBuffers->setupDelay = CLOUD_SETUP_DELAY;
        }
    }
    // Until setup finishes the clouds keep their last graphics (blank when just created).
    if (sCloudBuffers->setupStep != CLOUD_SETUP_DONE)
    {
        RunCloudSetupStep();
        return;
    }
    ProcessCloudAnimFrames();

    // Screen position of the map's top-left metatile (map coords include MAP_OFFSET).
    SetSpritePosToMapCoords(0, 0, &originX, &originY);
    originX += gSpriteCoordOffsetX;
    originY += gSpriteCoordOffsetY;

    for (i = 0; i < NUM_CLOUD_SPRITES; i++)
    {
        sprite = gWeatherPtr->sprites.s1.cloudSprites[i];
        if (sprite == NULL || !sprite->inUse || sprite->callback != UpdateCloudSprite)
            continue;

        // Match the hardware's 9-bit X / 8-bit Y wrapping of OAM coordinates.
        screenX = (sprite->x + sprite->x2 + sprite->centerToCornerVecX + gSpriteCoordOffsetX) & 0x1FF;
        screenY = (sprite->y + sprite->y2 + sprite->centerToCornerVecY + gSpriteCoordOffsetY) & 0xFF;
        if (screenX >= 256)
            screenX -= 512;
        if (screenY >= DISPLAY_HEIGHT)
            screenY -= 256;
        if (screenX <= -CLOUD_SPRITE_DIM || screenX >= DISPLAY_WIDTH
         || screenY <= -CLOUD_SPRITE_DIM || screenY >= DISPLAY_HEIGHT)
        {
            sCloudBuffers->maskValid[i] = FALSE;
            continue;
        }

        worldX = screenX - originX;
        worldY = screenY - originY;
        if (sCloudBuffers->maskValid[i]
         && sCloudBuffers->maskedX[i] == worldX
         && sCloudBuffers->maskedY[i] == worldY)
            continue;
        // Moving clouds need at most 2 updates a frame (they step on alternate frames), and
        // rarely 2 cell rebuilds. Anything more waits a frame, which keeps this lagless.
        if (numMasked >= CLOUD_MAX_MASKS_PER_FRAME
         || (rebuiltCells && CloudCellsNeedRebuild(i, worldX, worldY)))
            continue;
        if (CloudCellsNeedRebuild(i, worldX, worldY))
            rebuiltCells = TRUE;
        numMasked++;

        maskType = MaskCloudToWater(i, worldX, worldY);
        vram = (void *)(OBJ_VRAM0 + sprite->oam.tileNum * TILE_SIZE_4BPP);
        if (maskType == CLOUD_MASK_PARTIAL)
            RequestDma3Copy(sCloudBuffers->tiles[i], vram, sizeof(sCloudBuffers->tiles[i]), 1);
        else if (maskType != sCloudBuffers->uploadedMask[i])
        {
            // Hidden and fully-visible clouds don't change as they move, so only upload on a switch.
            if (maskType == CLOUD_MASK_FULL)
                RequestDma3Copy(gWeatherCloudTiles, vram, sizeof(sCloudBuffers->tiles[i]), 1);
            else
                RequestDma3Fill(0, vram, sizeof(sCloudBuffers->tiles[i]), 1);
        }
        sCloudBuffers->uploadedMask[i] = maskType;
        sCloudBuffers->maskedX[i] = worldX;
        sCloudBuffers->maskedY[i] = worldY;
        sCloudBuffers->maskValid[i] = TRUE;
    }
}
//------------------------------------------------------------------------------
// WEATHER_DROUGHT
//------------------------------------------------------------------------------

static void UpdateDroughtBlend(u8);

void Drought_InitVars(void)
{
    gWeatherPtr->initStep = 0;
    gWeatherPtr->weatherGfxLoaded = FALSE;
    gWeatherPtr->targetColorMapIndex = 0;
    gWeatherPtr->colorMapStepDelay = 0;
    gWeatherPtr->noShadows = FALSE;
}

void Drought_InitAll(void)
{
    Drought_InitVars();
    while (gWeatherPtr->weatherGfxLoaded == FALSE)
        Drought_Main();
}

void Drought_Main(void)
{
    switch (gWeatherPtr->initStep)
    {
    case 0:
        if (gWeatherPtr->palProcessingState != WEATHER_PAL_STATE_CHANGING_WEATHER)
            gWeatherPtr->initStep++;
        break;
    case 1:
        ResetDroughtWeatherPaletteLoading();
        gWeatherPtr->initStep++;
        break;
    case 2:
        if (LoadDroughtWeatherPalettes() == FALSE)
            gWeatherPtr->initStep++;
        break;
    case 3:
        DroughtStateInit();
        gWeatherPtr->initStep++;
        break;
    case 4:
        DroughtStateRun();
        if (gWeatherPtr->droughtBrightnessStage == 6)
        {
            gWeatherPtr->weatherGfxLoaded = TRUE;
            gWeatherPtr->initStep++;
        }
        break;
    default:
        DroughtStateRun();
        break;
    }
}

bool8 Drought_Finish(void)
{
    return FALSE;
}

void StartDroughtWeatherBlend(void)
{
    CreateTask(UpdateDroughtBlend, 80);
}

#define tState      data[0]
#define tBlendY     data[1]
#define tBlendDelay data[2]
#define tWinRange   data[3]

static void UpdateDroughtBlend(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case 0:
        task->tBlendY = 0;
        task->tBlendDelay = 0;
        task->tWinRange = REG_WININ;
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_ALL | WININ_WIN1_ALL);
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_BG1 | BLDCNT_TGT1_BG2 | BLDCNT_TGT1_BG3 | BLDCNT_TGT1_OBJ | BLDCNT_EFFECT_LIGHTEN);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        task->tState++;
        // fall through
    case 1:
        task->tBlendY += 3;
        if (task->tBlendY > 16)
            task->tBlendY = 16;
        SetGpuReg(REG_OFFSET_BLDY, task->tBlendY);
        if (task->tBlendY >= 16)
            task->tState++;
        break;
    case 2:
        task->tBlendDelay++;
        if (task->tBlendDelay > 9)
        {
            task->tBlendDelay = 0;
            task->tBlendY--;
            if (task->tBlendY <= 0)
            {
                task->tBlendY = 0;
                task->tState++;
            }
            SetGpuReg(REG_OFFSET_BLDY, task->tBlendY);
        }
        break;
    case 3:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_WININ, task->tWinRange);
        task->tState++;
        break;
    case 4:
        ScriptContext_Enable();
        DestroyTask(taskId);
        break;
    }
}

#undef tState
#undef tBlendY
#undef tBlendDelay
#undef tWinRange

//------------------------------------------------------------------------------
// WEATHER_RAIN
//------------------------------------------------------------------------------

static void LoadRainSpriteSheet(void);
static bool8 CreateRainSprite(void);
static void UpdateRainSprite(struct Sprite *sprite);
static bool8 UpdateVisibleRainSprites(void);
static void DestroyRainSprites(void);

static const struct Coords16 sRainSpriteCoords[] =
{
    {  0,   0},
    {  0, 160},
    {  0,  64},
    {144, 224},
    {144, 128},
    { 32,  32},
    { 32, 192},
    { 32,  96},
    { 72, 128},
    { 72,  32},
    { 72, 192},
    {216,  96},
    {216,   0},
    {104, 160},
    {104,  64},
    {104, 224},
    {144,   0},
    {144, 160},
    {144,  64},
    { 32, 224},
    { 32, 128},
    { 72,  32},
    { 72, 192},
    { 48,  96},
};

static const struct OamData sRainSpriteOamData =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(16x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(16x32),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 2,
    .affineParam = 0,
};

static const union AnimCmd sRainSpriteFallAnimCmd[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd sRainSpriteSplashAnimCmd[] =
{
    ANIMCMD_FRAME(8, 3),
    ANIMCMD_FRAME(32, 2),
    ANIMCMD_FRAME(40, 2),
    ANIMCMD_END,
};

static const union AnimCmd sRainSpriteHeavySplashAnimCmd[] =
{
    ANIMCMD_FRAME(8, 3),
    ANIMCMD_FRAME(16, 3),
    ANIMCMD_FRAME(24, 4),
    ANIMCMD_END,
};

static const union AnimCmd *const sRainSpriteAnimCmds[] =
{
    sRainSpriteFallAnimCmd,
    sRainSpriteSplashAnimCmd,
    sRainSpriteHeavySplashAnimCmd,
};

static const struct SpriteTemplate sRainSpriteTemplate =
{
    .tileTag = GFXTAG_RAIN,
    .paletteTag = PALTAG_WEATHER,
    .oam = &sRainSpriteOamData,
    .anims = sRainSpriteAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateRainSprite,
};

// Q28.4 fixed-point format values
static const s16 sRainSpriteMovement[][2] =
{
    {-0x68,  0xD0},
    {-0xA0, 0x140},
};

// First byte is the number of frames a raindrop falls before it splashes.
// Second byte is the maximum number of frames a raindrop can "wait" before
// it appears and starts falling. (This is only for the initial raindrop spawn.)
static const u16 sRainSpriteFallingDurations[][2] =
{
    {18, 7},
    {12, 10},
};

static const struct SpriteSheet sRainSpriteSheet =
{
    .data = gWeatherRainTiles,
    .size = sizeof(gWeatherRainTiles),
    .tag = GFXTAG_RAIN,
};

void Rain_InitVars(void)
{
    gWeatherPtr->initStep = 0;
    gWeatherPtr->weatherGfxLoaded = FALSE;
    gWeatherPtr->rainSpriteVisibleCounter = 0;
    gWeatherPtr->rainSpriteVisibleDelay = 8;
    gWeatherPtr->isDownpour = FALSE;
    gWeatherPtr->targetRainSpriteCount = 10;
    gWeatherPtr->targetColorMapIndex = 3;
    gWeatherPtr->colorMapStepDelay = 20;
    SetRainStrengthFromSoundEffect(SE_RAIN);
    Weather_SetBlendCoeffs(8, 12); // preserve shadow darkness
    gWeatherPtr->noShadows = FALSE;
}

void Rain_InitAll(void)
{
    Rain_InitVars();
    while (!gWeatherPtr->weatherGfxLoaded)
        Rain_Main();
}

void Rain_Main(void)
{
    switch (gWeatherPtr->initStep)
    {
    case 0:
        LoadRainSpriteSheet();
        gWeatherPtr->initStep++;
        break;
    case 1:
        if (!CreateRainSprite())
            gWeatherPtr->initStep++;
        break;
    case 2:
        if (!UpdateVisibleRainSprites())
        {
            gWeatherPtr->weatherGfxLoaded = TRUE;
            gWeatherPtr->initStep++;
        }
        break;
    }
}

bool8 Rain_Finish(void)
{
    switch (gWeatherPtr->finishStep)
    {
    case 0:
        if (gWeatherPtr->nextWeather == WEATHER_RAIN
         || gWeatherPtr->nextWeather == WEATHER_RAIN_THUNDERSTORM
         || gWeatherPtr->nextWeather == WEATHER_DOWNPOUR)
        {
            gWeatherPtr->finishStep = 0xFF;
            return FALSE;
        }
        else
        {
            gWeatherPtr->targetRainSpriteCount = 0;
            gWeatherPtr->finishStep++;
        }
        // fall through
    case 1:
        if (!UpdateVisibleRainSprites())
        {
            DestroyRainSprites();
            gWeatherPtr->finishStep++;
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

#define tCounter data[0]
#define tRandom  data[1]
#define tPosX    data[2]
#define tPosY    data[3]
#define tState   data[4]
#define tActive  data[5]
#define tWaiting data[6]

static void StartRainSpriteFall(struct Sprite *sprite)
{
    u32 rand;
    u16 numFallingFrames;
    int tileX;
    int tileY;

    if (sprite->tRandom == 0)
        sprite->tRandom = 361;

    rand = ISO_RANDOMIZE2(sprite->tRandom);
    sprite->tRandom = ((rand & 0x7FFF0000) >> 16) % 600;

    numFallingFrames = sRainSpriteFallingDurations[gWeatherPtr->isDownpour][0];

    tileX = sprite->tRandom % 30;
    sprite->tPosX = tileX * 8; // Useless assignment, leftover from before fixed-point values were used

    tileY = sprite->tRandom / 30;
    sprite->tPosY = tileY * 8; // Useless assignment, leftover from before fixed-point values were used

    sprite->tPosX = tileX;
    sprite->tPosX <<= 7; // This is tileX * 8, using a fixed-point value with 4 decimal places

    sprite->tPosY = tileY;
    sprite->tPosY <<= 7; // This is tileX * 8, using a fixed-point value with 4 decimal places

    // "Rewind" the rain sprites, from their ending position.
    sprite->tPosX -= sRainSpriteMovement[gWeatherPtr->isDownpour][0] * numFallingFrames;
    sprite->tPosY -= sRainSpriteMovement[gWeatherPtr->isDownpour][1] * numFallingFrames;

    StartSpriteAnim(sprite, 0);
    sprite->tState = 0;
    sprite->coordOffsetEnabled = FALSE;
    sprite->tCounter = numFallingFrames;
}

static void UpdateRainSprite(struct Sprite *sprite)
{
    if (sprite->tState == 0)
    {
        // Raindrop is in its "falling" motion.
        sprite->tPosX += sRainSpriteMovement[gWeatherPtr->isDownpour][0];
        sprite->tPosY += sRainSpriteMovement[gWeatherPtr->isDownpour][1];
        sprite->x = sprite->tPosX >> 4;
        sprite->y = sprite->tPosY >> 4;

        if (sprite->tActive
         && (sprite->x >= -8 && sprite->x <= DISPLAY_WIDTH + 8)
         && sprite->y >= -16 && sprite->y <= DISPLAY_HEIGHT + 16)
            sprite->invisible = FALSE;
        else
            sprite->invisible = TRUE;

        if (--sprite->tCounter == 0)
        {
            // Make raindrop splash on the ground
            StartSpriteAnim(sprite, gWeatherPtr->isDownpour + 1);
            sprite->tState = 1;
            sprite->x -= gSpriteCoordOffsetX;
            sprite->y -= gSpriteCoordOffsetY;
            sprite->coordOffsetEnabled = TRUE;
        }
    }
    else if (sprite->animEnded)
    {
        // The splashing animation ended.
        sprite->invisible = TRUE;
        StartRainSpriteFall(sprite);
    }
}

static void WaitRainSprite(struct Sprite *sprite)
{
    if (sprite->tCounter == 0)
    {
        StartRainSpriteFall(sprite);
        sprite->callback = UpdateRainSprite;
    }
    else
    {
        sprite->tCounter--;
    }
}

static void InitRainSpriteMovement(struct Sprite *sprite, u16 val)
{
    u16 numFallingFrames = sRainSpriteFallingDurations[gWeatherPtr->isDownpour][0];
    u16 numAdvanceRng = val / (sRainSpriteFallingDurations[gWeatherPtr->isDownpour][1] + numFallingFrames);
    u16 frameVal = val % (sRainSpriteFallingDurations[gWeatherPtr->isDownpour][1] + numFallingFrames);

    while (--numAdvanceRng != 0xFFFF)
        StartRainSpriteFall(sprite);

    if (frameVal < numFallingFrames)
    {
        while (--frameVal != 0xFFFF)
            UpdateRainSprite(sprite);

        sprite->tWaiting = 0;
    }
    else
    {
        sprite->tCounter = frameVal - numFallingFrames;
        sprite->invisible = TRUE;
        sprite->tWaiting = 1;
    }
}

static void LoadRainSpriteSheet(void)
{
    LoadSpriteSheet(&sRainSpriteSheet);
}

static bool8 CreateRainSprite(void)
{
    u8 spriteIndex;
    u8 spriteId;

    if (gWeatherPtr->rainSpriteCount == MAX_RAIN_SPRITES)
        return FALSE;

    spriteIndex = gWeatherPtr->rainSpriteCount;
    spriteId = CreateSpriteAtEnd(&sRainSpriteTemplate,
      sRainSpriteCoords[spriteIndex].x, sRainSpriteCoords[spriteIndex].y, 78);

    if (spriteId != MAX_SPRITES)
    {
        gSprites[spriteId].tActive = FALSE;
        gSprites[spriteId].tRandom = spriteIndex * 145;
        while (gSprites[spriteId].tRandom >= 600)
            gSprites[spriteId].tRandom -= 600;

        StartRainSpriteFall(&gSprites[spriteId]);
        InitRainSpriteMovement(&gSprites[spriteId], spriteIndex * 9);
        gSprites[spriteId].invisible = TRUE;
        gWeatherPtr->sprites.s1.rainSprites[spriteIndex] = &gSprites[spriteId];
    }
    else
    {
        gWeatherPtr->sprites.s1.rainSprites[spriteIndex] = NULL;
    }

    if (++gWeatherPtr->rainSpriteCount == MAX_RAIN_SPRITES)
    {
        u16 i;
        for (i = 0; i < MAX_RAIN_SPRITES; i++)
        {
            if (gWeatherPtr->sprites.s1.rainSprites[i])
            {
                if (!gWeatherPtr->sprites.s1.rainSprites[i]->tWaiting)
                    gWeatherPtr->sprites.s1.rainSprites[i]->callback = UpdateRainSprite;
                else
                    gWeatherPtr->sprites.s1.rainSprites[i]->callback = WaitRainSprite;
            }
        }

        return FALSE;
    }

    return TRUE;
}

static bool8 UpdateVisibleRainSprites(void)
{
    if (gWeatherPtr->curRainSpriteIndex == gWeatherPtr->targetRainSpriteCount)
        return FALSE;

    if (++gWeatherPtr->rainSpriteVisibleCounter > gWeatherPtr->rainSpriteVisibleDelay)
    {
        gWeatherPtr->rainSpriteVisibleCounter = 0;
        if (gWeatherPtr->curRainSpriteIndex < gWeatherPtr->targetRainSpriteCount)
        {
            gWeatherPtr->sprites.s1.rainSprites[gWeatherPtr->curRainSpriteIndex++]->tActive = TRUE;
        }
        else
        {
            gWeatherPtr->curRainSpriteIndex--;
            gWeatherPtr->sprites.s1.rainSprites[gWeatherPtr->curRainSpriteIndex]->tActive = FALSE;
            gWeatherPtr->sprites.s1.rainSprites[gWeatherPtr->curRainSpriteIndex]->invisible = TRUE;
        }
    }
    return TRUE;
}

static void DestroyRainSprites(void)
{
    u16 i;

    for (i = 0; i < gWeatherPtr->rainSpriteCount; i++)
    {
        if (gWeatherPtr->sprites.s1.rainSprites[i] != NULL)
            DestroySprite(gWeatherPtr->sprites.s1.rainSprites[i]);
    }
    gWeatherPtr->rainSpriteCount = 0;
    FreeSpriteTilesByTag(GFXTAG_RAIN);
}

#undef tCounter
#undef tRandom
#undef tPosX
#undef tPosY
#undef tState
#undef tActive
#undef tWaiting

//------------------------------------------------------------------------------
// Snow, blizzard, diamond dust and wind
//
// Ported from the DS games: WEATHER_SNOW and WEATHER_DIAMOND_DUST are the ones in
// HeartGold/SoulSilver, WEATHER_BLIZZARD is the one in Platinum. All three keep
// releasing short-lived particles from a pool of sprites. The DS games have 64
// sprites for that; the pools here are smaller, to leave sprites for everything else.
// The leaves of WEATHER_WINDY are released the same way.
//------------------------------------------------------------------------------

struct ParticleWeather
{
    const struct SpriteSheet *spriteSheet;
    const struct SpriteTemplate *spriteTemplate;
    void (*release)(s32 count);
    u8 numSprites;
    bool8 anchoredToMap; // the particles stay where they are on the map when the camera moves
    // The emitter as the weather starts. Its targets are the weather at full strength.
    struct WeatherEmitter emitter;
    // How the emitter dies down to nothing as the weather stops.
    s16 endInterval;
    s16 endIntervalDelta;
    s16 endCountDelta;
    // The particles that are already out when the weather starts at full strength (when
    // the map loads). After the first few, each group of headStartGroupSize of them is
    // headStartFrames further along than the group before.
    u8 headStartCount;
    u8 headStartGroupSize;
    u8 headStartFrames;
};

static void ReleaseSnowflakes(s32 count);
static void ReleaseBlizzardFlakes(s32 count);
static void ReleaseDiamondDust(s32 count);
static void ReleaseLeaves(s32 count);
static void UpdateFlakeSprite(struct Sprite *sprite);
static void UpdateSnowflakeSprite(struct Sprite *sprite);
static void UpdateDiamondDustSprite(struct Sprite *sprite);
static void SetWindTileFrames(u32 grass, u32 foliage);
static void StartSettlingWindTiles(void);
static bool32 SettleWindTiles(void);
static void UpdateBlizzardOverlaySprite(struct Sprite *sprite);
static void UpdateLeafSprite(struct Sprite *sprite);

// Particles that leave this area around the screen come back in on the opposite side.
#define PARTICLE_MARGIN  64
#define PARTICLE_WRAP_X  (DISPLAY_WIDTH - 1 + PARTICLE_MARGIN)
#define PARTICLE_WRAP_Y  (DISPLAY_HEIGHT + PARTICLE_MARGIN)

#define PARTICLE_SUBPRIORITY 78

// The blizzard's overlay is a seamless grid of 64x64 sprites that wraps around an area just
// big enough to always cover the screen.
#define BLIZZARD_OVERLAY_COLUMNS  5
#define BLIZZARD_OVERLAY_ROWS     (NUM_BLIZZARD_OVERLAY_SPRITES / BLIZZARD_OVERLAY_COLUMNS)
#define BLIZZARD_OVERLAY_WRAP_X   (BLIZZARD_OVERLAY_COLUMNS * 64)
#define BLIZZARD_OVERLAY_WRAP_Y   (BLIZZARD_OVERLAY_ROWS * 64)

#define SNOW_WIND_PHASE_LENGTH  200 // in snowflakes released
#define SNOWFLAKE_MELT_TIME     3   // frames a snowflake that has landed spends at each size as it melts
#define BLIZZARD_GUST_LENGTH    512 // in releases of flakes

#define DUST_STAGES             4   // of a crystal catching the light: speck, glint, sparkle, flare
#define DUST_TINTS              (sizeof(gWeatherDiamondDustTiles) / TILE_SIZE_4BPP / DUST_STAGES)
#define DUST_FALL_PERIOD        4   // frames between pixels of sinking

// The wind blows in gusts with lulls between them. Lengths are in frames.
#define WIND_FIRST_LULL_LENGTH  60
#define WIND_LULL_MIN_LENGTH    150
#define WIND_LULL_LENGTH_RANGE  210
#define WIND_GUST_MIN_LENGTH    150
#define WIND_GUST_LENGTH_RANGE  180
#define WIND_LULL_LEAF_ODDS     6   // in a lull, only 1 release in this many lets a leaf go
#define WIND_SETTLE_TIME        30  // the longest the weather waits for the foliage to settle as it stops

#define LEAF_FRAMES             4   // of a leaf tumbling over
#define LEAF_KINDS              (sizeof(gWeatherLeafTiles) / TILE_SIZE_4BPP / LEAF_FRAMES)
#define LEAF_FRAME_LENGTH       4
#define LEAF_MAX_LIFETIME       200
#define LEAF_WAVE_HEIGHT        5   // pixels a leaf bobs up and down, each way

// Particles
#define tTimer        data[0]
#define tLifetime     data[1]
#define tDeltaY       data[2]
#define tDying        data[3]
#define tDeltaX       data[4]
#define tAccelPeriod  data[5] // frames between changes of speed
#define tAccelX       data[6]
#define tActive       data[7]

// Diamond dust (with tTimer, tLifetime and tActive)
#define tTint         data[2] // which of the sheet's colors it glints in
#define tPeak         data[3] // the stage it gets as bright as
#define tStageLength  data[4] // frames in each stage

// Leaves (with tTimer, tLifetime, tDeltaX and tActive)
#define tFallPeriod   data[2] // frames between pixels of falling
#define tKind         data[3] // which of the sheet's leaves it is
#define tWavePhase    data[5]
#define tWaveSpeed    data[6]

// Blizzard overlay sprites
#define tColumn       data[0]
#define tRow          data[1]

static const struct OamData sParticleSpriteOamData =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x8),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct OamData sBlizzardOverlaySpriteOamData =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct SpriteSheet sSnowflakeSpriteSheet =
{
    .data = gWeatherSnowTiles,
    .size = sizeof(gWeatherSnowTiles),
    .tag = GFXTAG_SNOW,
};

static const struct SpriteSheet sBlizzardFlakeSpriteSheet =
{
    .data = gWeatherBlizzardTiles,
    .size = sizeof(gWeatherBlizzardTiles),
    .tag = GFXTAG_BLIZZARD,
};

static const struct SpriteSheet sBlizzardOverlaySpriteSheet =
{
    .data = gWeatherBlizzardOverlayTiles,
    .size = sizeof(gWeatherBlizzardOverlayTiles),
    .tag = GFXTAG_BLIZZARD_OVERLAY,
};

static const struct SpriteSheet sDiamondDustSpriteSheet =
{
    .data = gWeatherDiamondDustTiles,
    .size = sizeof(gWeatherDiamondDustTiles),
    .tag = GFXTAG_DIAMOND_DUST,
};

static const struct SpriteSheet sLeafSpriteSheet =
{
    .data = gWeatherLeafTiles,
    .size = sizeof(gWeatherLeafTiles),
    .tag = GFXTAG_LEAF,
};

// The leaves are the color of the season: green, with petals among them in spring, turned in
// fall and dry in winter.
static const u16 sLeafPalette_Spring[] = INCGFX_U16("graphics/weather/leaf_spring.pal", ".gbapal");
static const u16 sLeafPalette_Summer[] = INCGFX_U16("graphics/weather/leaf.png", ".gbapal");
static const u16 sLeafPalette_Fall[] = INCGFX_U16("graphics/weather/leaf_fall.pal", ".gbapal");
static const u16 sLeafPalette_Winter[] = INCGFX_U16("graphics/weather/leaf_winter.pal", ".gbapal");

static const u16 *const sLeafPalettes[SEASONS_PER_YEAR] =
{
    [SEASON_SPRING] = sLeafPalette_Spring,
    [SEASON_SUMMER] = sLeafPalette_Summer,
    [SEASON_FALL]   = sLeafPalette_Fall,
    [SEASON_WINTER] = sLeafPalette_Winter,
};

// The particles' frames are single tiles, picked by setting the sprite's tile directly.
static const struct SpriteTemplate sSnowflakeSpriteTemplate =
{
    .tileTag = GFXTAG_SNOW,
    .paletteTag = PALTAG_WEATHER,
    .oam = &sParticleSpriteOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateSnowflakeSprite,
};

static const struct SpriteTemplate sBlizzardFlakeSpriteTemplate =
{
    .tileTag = GFXTAG_BLIZZARD,
    .paletteTag = PALTAG_WEATHER,
    .oam = &sParticleSpriteOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateFlakeSprite,
};

static const struct SpriteTemplate sBlizzardOverlaySpriteTemplate =
{
    .tileTag = GFXTAG_BLIZZARD_OVERLAY,
    .paletteTag = PALTAG_WEATHER,
    .oam = &sBlizzardOverlaySpriteOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateBlizzardOverlaySprite,
};

static const struct SpriteTemplate sDiamondDustSpriteTemplate =
{
    .tileTag = GFXTAG_DIAMOND_DUST,
    .paletteTag = PALTAG_WEATHER,
    .oam = &sParticleSpriteOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateDiamondDustSprite,
};

static const struct SpriteTemplate sLeafSpriteTemplate =
{
    .tileTag = GFXTAG_LEAF,
    .paletteTag = PALTAG_WEATHER_2,
    .oam = &sParticleSpriteOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateLeafSprite,
};

static const struct ParticleWeather sSnowParticles =
{
    .spriteSheet = &sSnowflakeSpriteSheet,
    .spriteTemplate = &sSnowflakeSpriteTemplate,
    .release = ReleaseSnowflakes,
    .numSprites = NUM_SNOWFLAKE_SPRITES,
    .anchoredToMap = TRUE,
    .emitter = {
        .count = 1,
        .interval = 30,
        .targetCount = 6,
        .targetInterval = 3,
        .countDelta = 1,
        .intervalDelta = -5,
        .countStepPeriod = 2,
    },
    .endInterval = 30,
    .endIntervalDelta = 5,
    .endCountDelta = -3,
    .headStartCount = 20,
    .headStartGroupSize = 2,
    .headStartFrames = 3,
};

static const struct ParticleWeather sBlizzardParticles =
{
    .spriteSheet = &sBlizzardFlakeSpriteSheet,
    .spriteTemplate = &sBlizzardFlakeSpriteTemplate,
    .release = ReleaseBlizzardFlakes,
    .numSprites = NUM_BLIZZARD_FLAKE_SPRITES,
    .anchoredToMap = FALSE,
    .emitter = {
        .count = 1,
        .interval = 30,
        .targetCount = 10,
        .targetInterval = 1,
        .countDelta = 3,
        .intervalDelta = -4,
        .countStepPeriod = 2,
    },
    .endInterval = 12,
    .endIntervalDelta = 4,
    .endCountDelta = -4,
    .headStartCount = 20,
    .headStartGroupSize = 2,
    .headStartFrames = 2,
};

static const struct ParticleWeather sDiamondDustParticles =
{
    .spriteSheet = &sDiamondDustSpriteSheet,
    .spriteTemplate = &sDiamondDustSpriteTemplate,
    .release = ReleaseDiamondDust,
    .numSprites = NUM_DIAMOND_DUST_SPRITES,
    .anchoredToMap = TRUE,
    .emitter = {
        .count = 2,
        .interval = 16,
        .targetCount = 20,
        .targetInterval = 2,
        .countDelta = 2,
        .intervalDelta = -2,
        .countStepPeriod = 4,
    },
    .endInterval = 16,
    .endIntervalDelta = 6,
    .endCountDelta = -10,
    .headStartCount = 20,
    .headStartGroupSize = 10,
    .headStartFrames = 1,
};

static const struct ParticleWeather sLeafParticles =
{
    .spriteSheet = &sLeafSpriteSheet,
    .spriteTemplate = &sLeafSpriteTemplate,
    .release = ReleaseLeaves,
    .numSprites = NUM_LEAF_SPRITES,
    .anchoredToMap = TRUE,
    .emitter = {
        .count = 1,
        .interval = 30,
        .targetCount = 1,
        .targetInterval = 7,
        .countDelta = 0,
        .intervalDelta = -4,
        .countStepPeriod = 1,
    },
    .endInterval = 15,
    .endIntervalDelta = 4,
    .endCountDelta = -1,
    .headStartCount = 7,
    .headStartGroupSize = 1,
    .headStartFrames = 7,
};

static void InitParticleWeatherVars(s8 colorMapIndex)
{
    gWeatherPtr->initStep = 0;
    gWeatherPtr->weatherGfxLoaded = FALSE;
    gWeatherPtr->targetColorMapIndex = colorMapIndex;
    gWeatherPtr->colorMapStepDelay = 20;
    Weather_SetBlendCoeffs(8, 12); // preserve shadow darkness
    gWeatherPtr->noShadows = FALSE;
}

// Returns FALSE if the pool of particle sprites already exists.
static bool32 CreateParticleSprites(const struct ParticleWeather *weather)
{
    u32 i;
    u8 spriteId;

    if (gWeatherPtr->snowflakeSpriteCount != 0)
        return FALSE;

    LoadSpriteSheet(weather->spriteSheet);
    for (i = 0; i < weather->numSprites; i++)
    {
        spriteId = CreateSpriteAtEnd(weather->spriteTemplate, 0, 0, PARTICLE_SUBPRIORITY);
        if (spriteId != MAX_SPRITES)
        {
            gSprites[spriteId].coordOffsetEnabled = weather->anchoredToMap;
            gSprites[spriteId].invisible = TRUE;
            gWeatherPtr->sprites.s1.snowflakeSprites[i] = &gSprites[spriteId];
        }
        else
        {
            gWeatherPtr->sprites.s1.snowflakeSprites[i] = NULL;
        }
    }

    gWeatherPtr->snowflakeSpriteCount = weather->numSprites;
    return TRUE;
}

static void DestroyParticleSprites(const struct ParticleWeather *weather)
{
    u32 i;

    for (i = 0; i < gWeatherPtr->snowflakeSpriteCount; i++)
    {
        if (gWeatherPtr->sprites.s1.snowflakeSprites[i] != NULL)
            DestroySprite(gWeatherPtr->sprites.s1.snowflakeSprites[i]);
    }

    gWeatherPtr->snowflakeSpriteCount = 0;
    FreeSpriteTilesByTag(weather->spriteSheet->tag);
}

// Takes a sprite out of the pool to be a new particle. Returns NULL if they're all out.
static struct Sprite *TakeParticleSprite(void)
{
    struct Sprite *sprite;
    u32 i;

    for (i = 0; i < gWeatherPtr->snowflakeSpriteCount; i++)
    {
        sprite = gWeatherPtr->sprites.s1.snowflakeSprites[i];
        if (sprite != NULL && !sprite->tActive)
        {
            sprite->tActive = TRUE;
            sprite->invisible = FALSE;
            return sprite;
        }
    }

    return NULL;
}

static void ReturnParticleSprite(struct Sprite *sprite)
{
    sprite->tActive = FALSE;
    sprite->invisible = TRUE;
}

static bool32 AreParticlesOut(void)
{
    struct Sprite *sprite;
    u32 i;

    for (i = 0; i < gWeatherPtr->snowflakeSpriteCount; i++)
    {
        sprite = gWeatherPtr->sprites.s1.snowflakeSprites[i];
        if (sprite != NULL && sprite->tActive)
            return TRUE;
    }

    return FALSE;
}

// Particles are placed by where their top left corner is on the screen.
static void SetParticlePos(struct Sprite *sprite, s32 x, s32 y)
{
    if (x > PARTICLE_WRAP_X)
        x %= PARTICLE_WRAP_X;
    else if (x < -PARTICLE_MARGIN)
        x += PARTICLE_WRAP_X;

    if (y > PARTICLE_WRAP_Y)
        y %= PARTICLE_WRAP_Y;
    else if (y < -PARTICLE_MARGIN)
        y += PARTICLE_WRAP_Y;

    sprite->x = x - sprite->centerToCornerVecX;
    sprite->y = y - sprite->centerToCornerVecY;
    if (sprite->coordOffsetEnabled)
    {
        sprite->x -= gSpriteCoordOffsetX;
        sprite->y -= gSpriteCoordOffsetY;
    }
}

static void MoveParticle(struct Sprite *sprite, s32 deltaX, s32 deltaY)
{
    s16 x = sprite->x + sprite->centerToCornerVecX;
    s16 y = sprite->y + sprite->centerToCornerVecY;

    if (sprite->coordOffsetEnabled)
    {
        x += gSpriteCoordOffsetX;
        y += gSpriteCoordOffsetY;
    }

    SetParticlePos(sprite, x + deltaX, y + deltaY);
}

static void SetEmitterTargets(s16 targetCount, s16 targetInterval, s16 countDelta, s16 intervalDelta)
{
    struct WeatherEmitter *emitter = &gWeatherPtr->emitter;

    emitter->targetCount = targetCount;
    emitter->targetInterval = targetInterval;
    emitter->countDelta = countDelta;
    emitter->intervalDelta = intervalDelta;
}

// Whether a value that changes by delta has got to its target. If it has, it's set to it.
static bool32 HasReachedTarget(s16 *value, s16 target, s16 delta)
{
    if (delta != 0)
    {
        if (delta > 0 ? *value < target : *value > target)
            return FALSE;
        *value = target;
    }

    return TRUE;
}

// Runs the emitter for a frame while moving it toward its targets. Returns TRUE once it
// has reached them.
static bool32 RampEmitter(void (*release)(s32 count))
{
    struct WeatherEmitter *emitter = &gWeatherPtr->emitter;
    bool32 countDone = HasReachedTarget(&emitter->count, emitter->targetCount, emitter->countDelta);
    bool32 intervalDone = HasReachedTarget(&emitter->interval, emitter->targetInterval, emitter->intervalDelta);

    if (--emitter->timer <= 0)
    {
        release(emitter->count);
        emitter->timer = emitter->interval;
        if (!intervalDone)
            emitter->interval += emitter->intervalDelta;
        if (!countDone && ++emitter->countStepCounter >= emitter->countStepPeriod)
        {
            emitter->countStepCounter = 0;
            emitter->count += emitter->countDelta;
        }
    }

    return countDone && intervalDone;
}

static void RunEmitter(void (*release)(s32 count))
{
    struct WeatherEmitter *emitter = &gWeatherPtr->emitter;

    if (emitter->timer-- <= 0)
    {
        release(emitter->count);
        emitter->timer = emitter->interval;
    }
}

static void UpdateParticleWeather(const struct ParticleWeather *weather)
{
    switch (gWeatherPtr->initStep)
    {
    case 0:
        if (CreateParticleSprites(weather))
        {
            gWeatherPtr->emitter = weather->emitter;
            gWeatherPtr->snowWindCounter = 0;
        }
        else
        {
            // The particles are still out, from this weather before it began to stop or
            // from one that passed them on. Build up from wherever the emitter has got to.
            SetEmitterTargets(weather->emitter.targetCount, weather->emitter.targetInterval,
                              weather->emitter.countDelta, weather->emitter.intervalDelta);
        }
        gWeatherPtr->initStep++;
        break;
    case 1:
        if (RampEmitter(weather->release))
        {
            gWeatherPtr->weatherGfxLoaded = TRUE;
            gWeatherPtr->initStep++;
        }
        break;
    default:
        RunEmitter(weather->release);
        break;
    }
}

static void StartParticleWeatherAtFullStrength(const struct ParticleWeather *weather)
{
    struct Sprite *sprite;
    u32 i, j;
    u32 frames = 0;

    CreateParticleSprites(weather);
    gWeatherPtr->emitter = weather->emitter;
    gWeatherPtr->emitter.count = weather->emitter.targetCount;
    gWeatherPtr->emitter.interval = weather->emitter.targetInterval;
    gWeatherPtr->snowWindCounter = 0;

    weather->release(weather->headStartCount);
    for (i = 0; i < weather->headStartCount && i < gWeatherPtr->snowflakeSpriteCount; i++)
    {
        sprite = gWeatherPtr->sprites.s1.snowflakeSprites[i];
        for (j = 0; j < frames && sprite != NULL && sprite->tActive; j++)
            sprite->callback(sprite);

        if (i >= weather->headStartGroupSize && i % weather->headStartGroupSize == 0)
            frames += weather->headStartFrames;
    }

    gWeatherPtr->weatherGfxLoaded = TRUE;
    gWeatherPtr->initStep = 2;
}

static bool8 FinishParticleWeather(const struct ParticleWeather *weather)
{
    switch (gWeatherPtr->finishStep)
    {
    case 0:
        SetEmitterTargets(0, weather->endInterval, weather->endCountDelta, weather->endIntervalDelta);
        gWeatherPtr->finishStep++;
        // fall through
    case 1:
        if (gWeatherPtr->snowflakeSpriteCount != 0)
        {
            if (!RampEmitter(weather->release) || AreParticlesOut())
                return TRUE;
            DestroyParticleSprites(weather);
        }
        gWeatherPtr->finishStep++;
        break;
    }

    return FALSE;
}

static void BlowFlake(struct Sprite *sprite)
{
    MoveParticle(sprite, sprite->tDeltaX, sprite->tDeltaY);
    if (sprite->tTimer++ > sprite->tLifetime)
        sprite->tDying = TRUE;

    // The wind carries the flake sideways faster and faster as its fall slows.
    if (sprite->tTimer % sprite->tAccelPeriod == 0)
    {
        sprite->tDeltaX += sprite->tAccelX;
        if (sprite->tDeltaY > 1)
            sprite->tDeltaY--;
    }
}

// The blizzard's flakes are gone the moment they've been blown as far as they go.
static void UpdateFlakeSprite(struct Sprite *sprite)
{
    if (!sprite->tActive)
        return;

    if (sprite->tDying)
        ReturnParticleSprite(sprite);
    else
        BlowFlake(sprite);
}

// A snowflake lands where its fall ends, and melts away there: the sheet's tiles are a
// speck, then the snowflakes from small to large, so it shrinks a tile at a time.
static void UpdateSnowflakeSprite(struct Sprite *sprite)
{
    if (!sprite->tActive)
        return;

    if (!sprite->tDying)
    {
        BlowFlake(sprite);
    }
    else if (sprite->tDying++ % SNOWFLAKE_MELT_TIME == 0)
    {
        if (sprite->oam.tileNum == sprite->sheetTileStart)
            ReturnParticleSprite(sprite);
        else
            sprite->oam.tileNum--;
    }
}

// WEATHER_SNOW and WEATHER_FLURRIES: snowflakes blown to the left as they fall.
static void ReleaseSnowflakes(s32 count)
{
    // The stronger the wind, the more often a snowflake picks up speed.
    static const u8 sWindAccelPeriods[] = {16, 32, 16, 10};
    struct Sprite *sprite;
    s32 size;
    s32 i;

    for (i = 0; i < count; i++)
    {
        sprite = TakeParticleSprite();
        if (sprite == NULL)
            break;

        if (++gWeatherPtr->snowWindCounter >= SNOW_WIND_PHASE_LENGTH * ARRAY_COUNT(sWindAccelPeriods))
            gWeatherPtr->snowWindCounter = 0;

        sprite->tTimer = 0;
        sprite->tLifetime = 4 + Random() % 42;
        // The longer a snowflake is to last, the larger and faster it is, so the small
        // ones land near the top of the screen and the large ones fall all the way down.
        size = (sprite->tLifetime - 4) / 15;
        sprite->oam.tileNum = sprite->sheetTileStart + 1 + size;
        sprite->tDeltaX = -(size + 1);
        sprite->tDeltaY = 2 * (size + 1);
        sprite->tAccelPeriod = sWindAccelPeriods[gWeatherPtr->snowWindCounter / SNOW_WIND_PHASE_LENGTH];
        sprite->tAccelX = -1;
        sprite->tDying = FALSE;
        SetParticlePos(sprite, -20 + size * 20 + Random() % (DISPLAY_WIDTH + 164), -8);
    }
}

void Snow_InitVars(void)
{
    InitParticleWeatherVars(3);
}

void Snow_InitAll(void)
{
    Snow_InitVars();
    StartParticleWeatherAtFullStrength(&sSnowParticles);
}

// WEATHER_FLURRIES: the same snowfall as WEATHER_SNOW, but the screen isn't darkened.
void Flurries_InitVars(void)
{
    InitParticleWeatherVars(0);
}

void Flurries_InitAll(void)
{
    Flurries_InitVars();
    StartParticleWeatherAtFullStrength(&sSnowParticles);
}

void Snow_Main(void)
{
    UpdateParticleWeather(&sSnowParticles);
}

bool8 Snow_Finish(void)
{
    // Snow and flurries are the same snowfall, so it carries on when switching between the two.
    if (gWeatherPtr->finishStep == 0
     && (gWeatherPtr->nextWeather == WEATHER_SNOW || gWeatherPtr->nextWeather == WEATHER_FLURRIES))
    {
        gWeatherPtr->finishStep = 0xFF;
        return FALSE;
    }

    return FinishParticleWeather(&sSnowParticles);
}

// WEATHER_BLIZZARD: flakes that streak across the screen from the right in gusts, over a
// layer of fine snow rushing the same way, while the map's tall grass and foliage thrash.
static void ReleaseBlizzardFlakes(s32 count)
{
    static const s8 sGustSpeeds[] = {-4, -6, -8, -10};
    static const u8 sGustFallSpeeds[] = {2, 4, 2, 4};
    struct Sprite *sprite;
    s32 gust;
    s32 size;
    s32 i;

    if (++gWeatherPtr->snowWindCounter >= BLIZZARD_GUST_LENGTH * ARRAY_COUNT(sGustSpeeds))
        gWeatherPtr->snowWindCounter = 0;
    gust = gWeatherPtr->snowWindCounter / BLIZZARD_GUST_LENGTH;

    for (i = 0; i < count * 4; i++)
    {
        sprite = TakeParticleSprite();
        if (sprite == NULL)
            break;

        sprite->tTimer = 0;
        sprite->tLifetime = 18 + Random() % 6;
        size = Random() % 4;
        sprite->oam.tileNum = sprite->sheetTileStart + size;
        sprite->tDeltaX = sGustSpeeds[gust] * (size + 1);
        sprite->tDeltaY = sGustFallSpeeds[gust] * (size + 1);
        if (size == 3)
        {
            sprite->tDeltaX += sGustSpeeds[gust];
            sprite->tDeltaY += sGustFallSpeeds[gust];
        }
        sprite->tAccelPeriod = 4;
        sprite->tAccelX = sGustSpeeds[gust];
        sprite->tDying = FALSE;
        SetParticlePos(sprite, DISPLAY_WIDTH + Random() % 24, -32 + Random() % (DISPLAY_HEIGHT - 24));
    }
}

static void CreateBlizzardOverlaySprites(void)
{
    struct Sprite *sprite;
    u32 i;
    u8 spriteId;

    if (gWeatherPtr->blizzardOverlayCreated)
        return;

    LoadSpriteSheet(&sBlizzardOverlaySpriteSheet);
    for (i = 0; i < NUM_BLIZZARD_OVERLAY_SPRITES; i++)
    {
        spriteId = CreateSpriteAtEnd(&sBlizzardOverlaySpriteTemplate, 0, 0, PARTICLE_SUBPRIORITY + 1);
        if (spriteId != MAX_SPRITES)
        {
            sprite = &gSprites[spriteId];
            sprite->tColumn = i % BLIZZARD_OVERLAY_COLUMNS;
            sprite->tRow = i / BLIZZARD_OVERLAY_COLUMNS;
            sprite->invisible = TRUE;
            gWeatherPtr->sprites.s1.blizzardOverlaySprites[i] = sprite;
        }
        else
        {
            gWeatherPtr->sprites.s1.blizzardOverlaySprites[i] = NULL;
        }
    }

    gWeatherPtr->blizzardScrollX = 0;
    gWeatherPtr->blizzardScrollY = 0;
    gWeatherPtr->blizzardOverlayCreated = TRUE;
}

static void DestroyBlizzardOverlaySprites(void)
{
    u32 i;

    if (!gWeatherPtr->blizzardOverlayCreated)
        return;

    for (i = 0; i < NUM_BLIZZARD_OVERLAY_SPRITES; i++)
    {
        if (gWeatherPtr->sprites.s1.blizzardOverlaySprites[i] != NULL)
            DestroySprite(gWeatherPtr->sprites.s1.blizzardOverlaySprites[i]);
    }

    gWeatherPtr->blizzardOverlayCreated = FALSE;
    FreeSpriteTilesByTag(GFXTAG_BLIZZARD_OVERLAY);
}

static void SetBlizzardOverlayVisible(bool32 visible)
{
    u32 i;

    if (!gWeatherPtr->blizzardOverlayCreated)
        return;

    for (i = 0; i < NUM_BLIZZARD_OVERLAY_SPRITES; i++)
    {
        if (gWeatherPtr->sprites.s1.blizzardOverlaySprites[i] != NULL)
            gWeatherPtr->sprites.s1.blizzardOverlaySprites[i]->invisible = !visible;
    }
}

static void UpdateBlizzardOverlaySprite(struct Sprite *sprite)
{
    s32 x = sprite->tColumn * 64 + BLIZZARD_OVERLAY_WRAP_X - gWeatherPtr->blizzardScrollX;
    s32 y = sprite->tRow * 64 + gWeatherPtr->blizzardScrollY;

    sprite->x = x % BLIZZARD_OVERLAY_WRAP_X - 64 - sprite->centerToCornerVecX;
    sprite->y = y % BLIZZARD_OVERLAY_WRAP_Y - 64 - sprite->centerToCornerVecY;
}

// The blizzard's wind whips the map's tall grass and foliage about like the gusts of
// WEATHER_WINDY do, faster the harder it blows.
static void UpdateBlizzardWind(void)
{
    static const u8 sGustSwayPeriods[] = {16, 13, 10, 8}; // frames between changes
    u32 period = sGustSwayPeriods[gWeatherPtr->snowWindCounter / BLIZZARD_GUST_LENGTH];
    u32 age = gWeatherPtr->windAge++;

    if (!gWeatherPtr->weatherGfxLoaded)
        SetWindTileFrames(WIND_FRAME_BREEZE, WIND_FRAME_BREEZE); // the blizzard is still building up
    else
        SetWindTileFrames(((age / period) & 1) ? WIND_FRAME_GUST : WIND_FRAME_BREEZE,
                          (((age + period / 2) / period) & 1) ? WIND_FRAME_BREEZE : WIND_FRAME_GUST);
}

void Blizzard_InitVars(void)
{
    InitParticleWeatherVars(3);
    gWeatherPtr->windAge = 0;
    SetWindTileFrames(WIND_FRAME_STILL, WIND_FRAME_STILL);
}

void Blizzard_InitAll(void)
{
    Blizzard_InitVars();
    CreateBlizzardOverlaySprites();
    StartParticleWeatherAtFullStrength(&sBlizzardParticles);
}

void Blizzard_Main(void)
{
    CreateBlizzardOverlaySprites();
    UpdateParticleWeather(&sBlizzardParticles);
    UpdateBlizzardWind();

    // The overlay comes in once the blizzard has built up.
    SetBlizzardOverlayVisible(gWeatherPtr->weatherGfxLoaded);
    gWeatherPtr->blizzardScrollX = (gWeatherPtr->blizzardScrollX + 24) % BLIZZARD_OVERLAY_WRAP_X;
    gWeatherPtr->blizzardScrollY = (gWeatherPtr->blizzardScrollY + 12) % BLIZZARD_OVERLAY_WRAP_Y;
}

bool8 Blizzard_Finish(void)
{
    if (gWeatherPtr->finishStep == 0)
        StartSettlingWindTiles();

    SetBlizzardOverlayVisible(FALSE);
    if (FinishParticleWeather(&sBlizzardParticles))
        return TRUE;

    DestroyBlizzardOverlaySprites();
    return SettleWindTiles();
}

// WEATHER_DIAMOND_DUST: ice crystals that glitter in the air for a moment, in clear weather.
// Each one sinks slowly as it catches the light: it brightens a stage at a time up to its
// peak, and fades back the same way.
static void ReleaseDiamondDust(s32 count)
{
    struct Sprite *sprite;
    s32 x, y;
    s32 shaftTop, shaftBottom;
    s32 i;

    for (i = 0; i < count; i++)
    {
        sprite = TakeParticleSprite();
        if (sprite == NULL)
            break;

        x = -8 + Random() % (DISPLAY_WIDTH + 8);
        y = -8 + Random() % (DISPLAY_HEIGHT + 8);

        // The crystals catch the most light in a shaft that slants down across the screen
        // from the upper right and fades toward its lower edge. In it they sparkle or flare;
        // the rest only glint.
        shaftTop = 42 - x / 3;
        shaftBottom = shaftTop + Random() % (172 - x / 3);
        if (y >= shaftTop && y <= shaftBottom)
            sprite->tPeak = DUST_STAGES - 2 + Random() % 2;
        else
            sprite->tPeak = 1;

        sprite->tTimer = 0;
        sprite->tStageLength = 3 + Random() % 3;
        sprite->tLifetime = (2 * sprite->tPeak + 1) * sprite->tStageLength;
        sprite->tTint = Random() % DUST_TINTS;
        sprite->oam.tileNum = sprite->sheetTileStart + sprite->tTint * DUST_STAGES;
        SetParticlePos(sprite, x, y);
    }
}

static void UpdateDiamondDustSprite(struct Sprite *sprite)
{
    s32 stage;

    if (!sprite->tActive)
        return;

    stage = sprite->tTimer / sprite->tStageLength;
    if (stage > sprite->tPeak)
        stage = 2 * sprite->tPeak - stage;
    sprite->oam.tileNum = sprite->sheetTileStart + sprite->tTint * DUST_STAGES + stage;

    if (++sprite->tTimer >= sprite->tLifetime)
        ReturnParticleSprite(sprite);
    else if (sprite->tTimer % DUST_FALL_PERIOD == 0)
        MoveParticle(sprite, 0, 1);
}

void DiamondDust_InitVars(void)
{
    InitParticleWeatherVars(0);
}

void DiamondDust_InitAll(void)
{
    DiamondDust_InitVars();
    StartParticleWeatherAtFullStrength(&sDiamondDustParticles);
}

void DiamondDust_Main(void)
{
    UpdateParticleWeather(&sDiamondDustParticles);
}

bool8 DiamondDust_Finish(void)
{
    return FinishParticleWeather(&sDiamondDustParticles);
}

// WEATHER_WINDY: gusts, with lulls between them, that carry leaves across the screen from the
// right. The map's tall grass and foliage move with them: the weather picks which frame those
// tiles are in, and the tileset animations draw it (see the wind section of tileset_anims.c).
static void SetWindGusting(bool32 gusting)
{
    gWeatherPtr->windGusting = gusting;
    gWeatherPtr->windAge = 0;
    if (gusting)
        gWeatherPtr->windTimer = WIND_GUST_MIN_LENGTH + Random() % WIND_GUST_LENGTH_RANGE;
    else
        gWeatherPtr->windTimer = WIND_LULL_MIN_LENGTH + Random() % WIND_LULL_LENGTH_RANGE;
}

static void SetWindTileFrames(u32 grass, u32 foliage)
{
    gWeatherPtr->windTileFrames[WIND_TILES_GRASS] = grass;
    gWeatherPtr->windTileFrames[WIND_TILES_FOLIAGE] = foliage;
}

// Called as a weather that moves the map's tall grass and foliage begins to stop.
static void StartSettlingWindTiles(void)
{
    gWeatherPtr->windTimer = WIND_SETTLE_TIME;
    SetWindTileFrames(WIND_FRAME_STILL, WIND_FRAME_STILL);
}

// Whether they are still coming to rest: the next weather has to find the map's tiles as the
// tilesets have them.
static bool32 SettleWindTiles(void)
{
    if (AreWindTilesStill() || gWeatherPtr->windTimer == 0)
        return FALSE;

    gWeatherPtr->windTimer--;
    return TRUE;
}

static void UpdateWind(void)
{
    u32 age, grass, foliage;

    if (gWeatherPtr->windTimer == 0)
        SetWindGusting(!gWeatherPtr->windGusting);
    else
        gWeatherPtr->windTimer--;
    age = gWeatherPtr->windAge++;

    if (gWeatherPtr->windGusting)
    {
        // The grass bends over before the trees do, and whips about faster.
        if (age < 10)
            grass = WIND_FRAME_BREEZE;
        else
            grass = ((age / 10) & 1) ? WIND_FRAME_GUST : WIND_FRAME_BREEZE;

        if (age < 6)
            foliage = WIND_FRAME_STILL;
        else if (age < 20)
            foliage = WIND_FRAME_BREEZE;
        else
            foliage = (((age - 20) / 14) & 1) ? WIND_FRAME_BREEZE : WIND_FRAME_GUST;
    }
    else
    {
        // The gust dies away, and from then on only a breeze stirs them now and then.
        grass = (age < 12 || age % 64 >= 50) ? WIND_FRAME_BREEZE : WIND_FRAME_STILL;
        foliage = (age < 18 || age % 88 >= 70) ? WIND_FRAME_BREEZE : WIND_FRAME_STILL;
    }
    SetWindTileFrames(grass, foliage);
}

// The frame (WIND_FRAME_*) the map's tiles of a kind (WIND_TILES_*) are to be drawn in.
u32 GetWindTileFrame(u32 kind)
{
    if (gWeatherPtr->currWeather != WEATHER_WINDY && gWeatherPtr->currWeather != WEATHER_BLIZZARD)
        return WIND_FRAME_STILL;
    return gWeatherPtr->windTileFrames[kind];
}

static void ReleaseLeaves(s32 count)
{
    struct Sprite *sprite;
    s32 i;

    // Few leaves are about in a lull, and they drift.
    if (!gWeatherPtr->windGusting && Random() % WIND_LULL_LEAF_ODDS != 0)
        return;

    for (i = 0; i < count; i++)
    {
        sprite = TakeParticleSprite();
        if (sprite == NULL)
            break;

        // Starting anywhere in the tumble keeps the leaves from turning over together.
        sprite->tTimer = Random() % (LEAF_FRAMES * LEAF_FRAME_LENGTH);
        sprite->tLifetime = sprite->tTimer + LEAF_MAX_LIFETIME;
        sprite->tDeltaX = gWeatherPtr->windGusting ? -4 - Random() % 3 : -2 - Random() % 2;
        sprite->tFallPeriod = 2 + Random() % 4;
        sprite->tKind = Random() % LEAF_KINDS;
        sprite->tWavePhase = Random() % 256;
        sprite->tWaveSpeed = 3 + Random() % 6;
        sprite->y2 = 0;
        SetParticlePos(sprite, DISPLAY_WIDTH + Random() % 32, -16 + Random() % (DISPLAY_HEIGHT + 8));
    }
}

static void UpdateLeafSprite(struct Sprite *sprite)
{
    s32 x;

    if (!sprite->tActive)
        return;

    sprite->tTimer++;
    MoveParticle(sprite, sprite->tDeltaX, sprite->tTimer % sprite->tFallPeriod == 0);
    sprite->tWavePhase = (sprite->tWavePhase + sprite->tWaveSpeed) & 0xFF;
    sprite->y2 = gSineTable[sprite->tWavePhase] * LEAF_WAVE_HEIGHT / 256;
    sprite->oam.tileNum = sprite->sheetTileStart + sprite->tKind * LEAF_FRAMES
                        + (sprite->tTimer / LEAF_FRAME_LENGTH) % LEAF_FRAMES;

    // The leaf is gone once the wind has carried it off the left of the screen.
    x = sprite->x + sprite->centerToCornerVecX;
    if (sprite->coordOffsetEnabled)
        x += gSpriteCoordOffsetX;
    if (x < -8 || sprite->tTimer > sprite->tLifetime)
        ReturnParticleSprite(sprite);
}

static void LoadLeafPalette(void)
{
    LoadCustomWeatherSpritePalette(sLeafPalettes[GetFieldSeason()]);
}

void Windy_InitVars(void)
{
    InitParticleWeatherVars(0);
    SetWindGusting(FALSE);
    gWeatherPtr->windTimer = WIND_FIRST_LULL_LENGTH;
    SetWindTileFrames(WIND_FRAME_STILL, WIND_FRAME_STILL);
}

void Windy_InitAll(void)
{
    Windy_InitVars();
    SetWindGusting(TRUE);
    LoadLeafPalette();
    StartParticleWeatherAtFullStrength(&sLeafParticles);
}

void Windy_Main(void)
{
    // The leaves' palette has to be there before their sprites are made.
    if (gWeatherPtr->snowflakeSpriteCount == 0)
        LoadLeafPalette();
    UpdateWind();
    UpdateParticleWeather(&sLeafParticles);
}

bool8 Windy_Finish(void)
{
    // The wind drops: the grass and the trees come to rest while the last leaves blow past.
    if (gWeatherPtr->finishStep == 0)
    {
        gWeatherPtr->windGusting = FALSE;
        StartSettlingWindTiles();
    }

    if (FinishParticleWeather(&sLeafParticles))
        return TRUE;

    return SettleWindTiles();
}

#undef tTimer
#undef tLifetime
#undef tDeltaY
#undef tDying
#undef tDeltaX
#undef tAccelPeriod
#undef tAccelX
#undef tActive
#undef tTint
#undef tPeak
#undef tStageLength
#undef tFallPeriod
#undef tKind
#undef tWavePhase
#undef tWaveSpeed
#undef tColumn
#undef tRow

//------------------------------------------------------------------------------
// WEATHER_RAIN_THUNDERSTORM
//------------------------------------------------------------------------------

enum {
    // This block of states is run only once
    // when first setting up the thunderstorm
    THUNDER_STATE_LOAD_RAIN,
    THUNDER_STATE_CREATE_RAIN,
    THUNDER_STATE_INIT_RAIN,
    THUNDER_STATE_WAIT_CHANGE,

    // The thunderstorm loops through these states,
    // not necessarily in order.
    THUNDER_STATE_NEW_CYCLE,
    THUNDER_STATE_NEW_CYCLE_WAIT,
    THUNDER_STATE_INIT_CYCLE_1,
    THUNDER_STATE_INIT_CYCLE_2,
    THUNDER_STATE_SHORT_BOLT,
    THUNDER_STATE_TRY_NEW_BOLT,
    THUNDER_STATE_WAIT_BOLT_SHORT,
    THUNDER_STATE_INIT_BOLT_LONG,
    THUNDER_STATE_WAIT_BOLT_LONG,
    THUNDER_STATE_FADE_BOLT_LONG,
    THUNDER_STATE_END_BOLT_LONG,
};

void Thunderstorm_InitVars(void)
{
    gWeatherPtr->initStep = THUNDER_STATE_LOAD_RAIN;
    gWeatherPtr->weatherGfxLoaded = FALSE;
    gWeatherPtr->rainSpriteVisibleCounter = 0;
    gWeatherPtr->rainSpriteVisibleDelay = 4;
    gWeatherPtr->isDownpour = FALSE;
    gWeatherPtr->targetRainSpriteCount = 16;
    gWeatherPtr->targetColorMapIndex = 3;
    gWeatherPtr->colorMapStepDelay = 20;
    gWeatherPtr->weatherGfxLoaded = FALSE;  // duplicate assignment
    gWeatherPtr->thunderEnqueued = FALSE;
    SetRainStrengthFromSoundEffect(SE_THUNDERSTORM);
    Weather_SetBlendCoeffs(8, 12); // preserve shadow darkness
    gWeatherPtr->noShadows = FALSE;
}

void Thunderstorm_InitAll(void)
{
    Thunderstorm_InitVars();
    while (gWeatherPtr->weatherGfxLoaded == FALSE)
        Thunderstorm_Main();
}

//------------------------------------------------------------------------------
// WEATHER_DOWNPOUR
//------------------------------------------------------------------------------

static void UpdateThunderSound(void);
static void EnqueueThunder(u16);

void Downpour_InitVars(void)
{
    gWeatherPtr->initStep = THUNDER_STATE_LOAD_RAIN;
    gWeatherPtr->weatherGfxLoaded = FALSE;
    gWeatherPtr->rainSpriteVisibleCounter = 0;
    gWeatherPtr->rainSpriteVisibleDelay = 4;
    gWeatherPtr->isDownpour = TRUE;
    gWeatherPtr->targetRainSpriteCount = 24;
    gWeatherPtr->targetColorMapIndex = 3;
    gWeatherPtr->colorMapStepDelay = 20;
    gWeatherPtr->weatherGfxLoaded = FALSE;  // duplicate assignment
    SetRainStrengthFromSoundEffect(SE_DOWNPOUR);
    Weather_SetBlendCoeffs(8, 12); // preserve shadow darkness
    gWeatherPtr->noShadows = FALSE;
}

void Downpour_InitAll(void)
{
    Downpour_InitVars();
    while (gWeatherPtr->weatherGfxLoaded == FALSE)
        Thunderstorm_Main();
}

// In a given cycle, there will be some shorter bolts of lightning, potentially
// followed by a longer bolt. As a "regex", the pattern is:
//   (SHORT_BOLT){1,2}(LONG_BOLT)?
//
// Thunder only plays on the final bolt of the cycle.
void Thunderstorm_Main(void)
{
    UpdateThunderSound();
    switch (gWeatherPtr->initStep)
    {
    case THUNDER_STATE_LOAD_RAIN:
        LoadRainSpriteSheet();
        gWeatherPtr->initStep++;
        break;
    case THUNDER_STATE_CREATE_RAIN:
        if (!CreateRainSprite())
            gWeatherPtr->initStep++;
        break;
    case THUNDER_STATE_INIT_RAIN:
        if (!UpdateVisibleRainSprites())
        {
            gWeatherPtr->weatherGfxLoaded = TRUE;
            gWeatherPtr->initStep++;
        }
        break;
    case THUNDER_STATE_WAIT_CHANGE:
        if (gWeatherPtr->palProcessingState != WEATHER_PAL_STATE_CHANGING_WEATHER)
            gWeatherPtr->initStep = THUNDER_STATE_INIT_CYCLE_1;
        break;
    case THUNDER_STATE_NEW_CYCLE:
        gWeatherPtr->thunderAllowEnd = TRUE;
        gWeatherPtr->thunderTimer = (Random() % 360) + 360;
        gWeatherPtr->initStep++;
        // fall through
    case THUNDER_STATE_NEW_CYCLE_WAIT:
        // Wait between 360-720 frames before starting a new cycle.
        if (--gWeatherPtr->thunderTimer == 0)
            gWeatherPtr->initStep++;
        break;
    case THUNDER_STATE_INIT_CYCLE_1:
        gWeatherPtr->thunderAllowEnd = TRUE;
        gWeatherPtr->thunderLongBolt = Random() % 2;
        gWeatherPtr->initStep++;
        break;
    case THUNDER_STATE_INIT_CYCLE_2:
        gWeatherPtr->thunderShortBolts = (Random() & 1) + 1;
        gWeatherPtr->initStep++;
        // fall through
    case THUNDER_STATE_SHORT_BOLT:
        // Short bolt of lightning strikes.
        ApplyWeatherColorMapIfIdle(19);
        // If final lightning bolt, enqueue thunder.
        if (!gWeatherPtr->thunderLongBolt && gWeatherPtr->thunderShortBolts == 1)
            EnqueueThunder(20);

        gWeatherPtr->thunderTimer = (Random() % 3) + 6;
        gWeatherPtr->initStep++;
        break;
    case THUNDER_STATE_TRY_NEW_BOLT:
        if (--gWeatherPtr->thunderTimer == 0)
        {
            // Short bolt of lightning ends.
            ApplyWeatherColorMapIfIdle(3);
            gWeatherPtr->thunderAllowEnd = TRUE;
            if (--gWeatherPtr->thunderShortBolts != 0)
            {
                // Wait a little, then do another short bolt.
                gWeatherPtr->thunderTimer = (Random() % 16) + 60;
                gWeatherPtr->initStep = THUNDER_STATE_WAIT_BOLT_SHORT;
            }
            else if (!gWeatherPtr->thunderLongBolt)
            {
                // No more bolts, restart loop.
                gWeatherPtr->initStep = THUNDER_STATE_NEW_CYCLE;
            }
            else
            {
                // Set up long bolt.
                gWeatherPtr->initStep = THUNDER_STATE_INIT_BOLT_LONG;
            }
        }
        break;
    case THUNDER_STATE_WAIT_BOLT_SHORT:
        if (--gWeatherPtr->thunderTimer == 0)
            gWeatherPtr->initStep = THUNDER_STATE_SHORT_BOLT;
        break;
    case THUNDER_STATE_INIT_BOLT_LONG:
        gWeatherPtr->thunderTimer = (Random() % 16) + 60;
        gWeatherPtr->initStep++;
        break;
    case THUNDER_STATE_WAIT_BOLT_LONG:
        if (--gWeatherPtr->thunderTimer == 0)
        {
            // Do long bolt. Enqueue thunder with a potentially longer delay.
            EnqueueThunder(100);
            ApplyWeatherColorMapIfIdle(19);
            gWeatherPtr->thunderTimer = (Random() & 0xF) + 30;
            gWeatherPtr->initStep++;
        }
        break;
    case THUNDER_STATE_FADE_BOLT_LONG:
        if (--gWeatherPtr->thunderTimer == 0)
        {
            // Fade long bolt out over time.
            ApplyWeatherColorMapIfIdle_Gradual(19, 3, 5);
            gWeatherPtr->initStep++;
        }
        break;
    case THUNDER_STATE_END_BOLT_LONG:
        if (gWeatherPtr->palProcessingState == WEATHER_PAL_STATE_IDLE)
        {
            gWeatherPtr->thunderAllowEnd = TRUE;
            gWeatherPtr->initStep = THUNDER_STATE_NEW_CYCLE;
        }
        break;
    }
}

bool8 Thunderstorm_Finish(void)
{
    switch (gWeatherPtr->finishStep)
    {
    case 0:
        gWeatherPtr->thunderAllowEnd = FALSE;
        gWeatherPtr->finishStep++;
        // fall through
    case 1:
        Thunderstorm_Main();
        if (gWeatherPtr->thunderAllowEnd)
        {
            if (gWeatherPtr->nextWeather == WEATHER_RAIN
             || gWeatherPtr->nextWeather == WEATHER_RAIN_THUNDERSTORM
             || gWeatherPtr->nextWeather == WEATHER_DOWNPOUR)
                return FALSE;

            gWeatherPtr->targetRainSpriteCount = 0;
            gWeatherPtr->finishStep++;
        }
        break;
    case 2:
        if (!UpdateVisibleRainSprites())
        {
            DestroyRainSprites();
            gWeatherPtr->thunderEnqueued = FALSE;
            gWeatherPtr->finishStep++;
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

// Enqueue a thunder sound effect for at most `waitFrames` frames from now.
static void EnqueueThunder(u16 waitFrames)
{
    if (!gWeatherPtr->thunderEnqueued)
    {
        gWeatherPtr->thunderSETimer = Random() % waitFrames;
        gWeatherPtr->thunderEnqueued = TRUE;
    }
}

static void UpdateThunderSound(void)
{
    if (gWeatherPtr->thunderEnqueued == TRUE)
    {
        if (gWeatherPtr->thunderSETimer == 0)
        {
            if (IsSEPlaying())
                return;

            if (Random() & 1)
                PlaySE(SE_THUNDER);
            else
                PlaySE(SE_THUNDER2);

            gWeatherPtr->thunderEnqueued = FALSE;
        }
        else
        {
            gWeatherPtr->thunderSETimer--;
        }
    }
}

//------------------------------------------------------------------------------
// WEATHER_FOG_HORIZONTAL and WEATHER_UNDERWATER
//------------------------------------------------------------------------------

static const u16 sUnusedData[] = {0, 6, 6, 12, 18, 42, 300, 300};

static const struct OamData sOamData_FogH =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_BLEND,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 2,
    .paletteNum = 0,
    .affineParam = 0,
};

static const union AnimCmd sAnim_FogH_0[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_END,
};

static const union AnimCmd sAnim_FogH_1[] =
{
    ANIMCMD_FRAME(32, 16),
    ANIMCMD_END,
};

static const union AnimCmd sAnim_FogH_2[] =
{
    ANIMCMD_FRAME(64, 16),
    ANIMCMD_END,
};

static const union AnimCmd sAnim_FogH_3[] =
{
    ANIMCMD_FRAME(96, 16),
    ANIMCMD_END,
};

static const union AnimCmd sAnim_FogH_4[] =
{
    ANIMCMD_FRAME(128, 16),
    ANIMCMD_END,
};

static const union AnimCmd sAnim_FogH_5[] =
{
    ANIMCMD_FRAME(160, 16),
    ANIMCMD_END,
};

static const union AnimCmd *const sAnims_FogH[] =
{
    sAnim_FogH_0,
    sAnim_FogH_1,
    sAnim_FogH_2,
    sAnim_FogH_3,
    sAnim_FogH_4,
    sAnim_FogH_5,
};

static const union AffineAnimCmd sAffineAnim_FogH[] =
{
    AFFINEANIMCMD_FRAME(0x200, 0x200, 0, 0),
    AFFINEANIMCMD_END,
};

static const union AffineAnimCmd *const sAffineAnims_FogH[] =
{
    sAffineAnim_FogH,
};

static void FogHorizontalSpriteCallback(struct Sprite *);
static const struct SpriteTemplate sFogHorizontalSpriteTemplate =
{
    .tileTag = GFXTAG_FOG_H,
    .paletteTag = PALTAG_WEATHER,
    .oam = &sOamData_FogH,
    .anims = sAnims_FogH,
    .images = NULL,
    .affineAnims = sAffineAnims_FogH,
    .callback = FogHorizontalSpriteCallback,
};

void FogHorizontal_Main(void);
static void CreateFogHorizontalSprites(void);
static void DestroyFogHorizontalSprites(void);

// Within the weather palette, shadow sprites' color index
#define SHADOW_COLOR_INDEX 9

// Updates just the color of shadows to match special weather blending
u8 UpdateShadowColor(u16 color) {
    u8 paletteNum = IndexOfSpritePaletteTag(TAG_WEATHER_START);
    u16 ALIGNED(4) tempBuffer[16];
    u16 blendedColor;
    if (paletteNum < 16) {
        u16 index = OBJ_PLTT_ID(paletteNum)+SHADOW_COLOR_INDEX;
        gPlttBufferUnfaded[index] = gPlttBufferFaded[index] = color;
        // Copy to temporary buffer, blend, and keep just the shadow color index
        CpuFastCopy(&gPlttBufferFaded[index-SHADOW_COLOR_INDEX], tempBuffer, PLTT_SIZE_4BPP);
        UpdateSpritePaletteWithTime(paletteNum);
        blendedColor = gPlttBufferFaded[index];
        CpuFastCopy(tempBuffer, &gPlttBufferFaded[index-SHADOW_COLOR_INDEX], PLTT_SIZE_4BPP);
        gPlttBufferFaded[index] = blendedColor;
    }
    return paletteNum;
}

void FogHorizontal_InitVars(void)
{
    gWeatherPtr->initStep = 0;
    gWeatherPtr->weatherGfxLoaded = FALSE;
    gWeatherPtr->targetColorMapIndex = 0;
    gWeatherPtr->colorMapStepDelay = 20;
    if (gWeatherPtr->fogHSpritesCreated == 0)
    {
        gWeatherPtr->fogHScrollCounter = 0;
        gWeatherPtr->fogHScrollOffset = 0;
        gWeatherPtr->fogHScrollPosX = 0;
        Weather_SetBlendCoeffs(0, 16);
    }
    gWeatherPtr->noShadows = FALSE;
}

void FogHorizontal_InitAll(void)
{
    FogHorizontal_InitVars();
    while (gWeatherPtr->weatherGfxLoaded == FALSE)
        FogHorizontal_Main();
}

void FogHorizontal_Main(void)
{
    gWeatherPtr->fogHScrollPosX = (gSpriteCoordOffsetX - gWeatherPtr->fogHScrollOffset) & 0xFF;
    if (++gWeatherPtr->fogHScrollCounter > 3)
    {
        gWeatherPtr->fogHScrollCounter = 0;
        gWeatherPtr->fogHScrollOffset++;
    }
    switch (gWeatherPtr->initStep)
    {
    case 0:
        CreateFogHorizontalSprites();
        if (gWeatherPtr->currWeather == WEATHER_FOG_HORIZONTAL) {
          u8 paletteNum = IndexOfSpritePaletteTag(TAG_WEATHER_START);
          Weather_SetTargetBlendCoeffs(12, 8, 3);
          UpdateShadowColor(0x3DEF); // Gray
        } else
            Weather_SetTargetBlendCoeffs(4, 16, 0);
        gWeatherPtr->initStep++;
        break;
    case 1:
        if (Weather_UpdateBlend())
        {
            gWeatherPtr->weatherGfxLoaded = TRUE;
            gWeatherPtr->initStep++;
        }
        break;
    }
}

bool8 FogHorizontal_Finish(void)
{
    gWeatherPtr->fogHScrollPosX = (gSpriteCoordOffsetX - gWeatherPtr->fogHScrollOffset) & 0xFF;
    if (++gWeatherPtr->fogHScrollCounter > 3)
    {
        gWeatherPtr->fogHScrollCounter = 0;
        gWeatherPtr->fogHScrollOffset++;
    }

    switch (gWeatherPtr->finishStep)
    {
    case 0:
        Weather_SetTargetBlendCoeffs(0, 16, 3);
        gWeatherPtr->finishStep++;
        break;
    case 1:
        if (Weather_UpdateBlend())
            gWeatherPtr->finishStep++;
        break;
    case 2:
        DestroyFogHorizontalSprites();
        gWeatherPtr->finishStep++;
        break;
    default:
        UpdateShadowColor(RGB_BLACK);
        return FALSE;
    }
    return TRUE;
}

#define tSpriteColumn data[0]

static void FogHorizontalSpriteCallback(struct Sprite *sprite)
{
    sprite->y2 = (u8)gSpriteCoordOffsetY;
    sprite->x = gWeatherPtr->fogHScrollPosX + 32 + sprite->tSpriteColumn * 64;
    if (sprite->x >= DISPLAY_WIDTH + 32)
    {
        sprite->x = (DISPLAY_WIDTH * 2) + gWeatherPtr->fogHScrollPosX - (4 - sprite->tSpriteColumn) * 64;
        sprite->x &= 0x1FF;
    }
}

static void CreateFogHorizontalSprites(void)
{
    u16 i;
    u8 spriteId;
    struct Sprite *sprite;

    if (!gWeatherPtr->fogHSpritesCreated)
    {
        struct SpriteSheet fogHorizontalSpriteSheet = {
            .data = gWeatherFogHorizontalTiles,
            .size = sizeof(gWeatherFogHorizontalTiles),
            .tag = GFXTAG_FOG_H,
        };
        LoadSpriteSheet(&fogHorizontalSpriteSheet);
        for (i = 0; i < NUM_FOG_HORIZONTAL_SPRITES; i++)
        {
            spriteId = CreateSpriteAtEnd(&sFogHorizontalSpriteTemplate, 0, 0, 0xFF);
            if (spriteId != MAX_SPRITES)
            {
                sprite = &gSprites[spriteId];
                sprite->tSpriteColumn = i % 5;
                sprite->x = (i % 5) * 64 + 32;
                sprite->y = (i / 5) * 64 + 32;
                gWeatherPtr->sprites.s2.fogHSprites[i] = sprite;
            }
            else
            {
                gWeatherPtr->sprites.s2.fogHSprites[i] = NULL;
            }
        }

        gWeatherPtr->fogHSpritesCreated = TRUE;
    }
}

static void DestroyFogHorizontalSprites(void)
{
    u16 i;

    if (gWeatherPtr->fogHSpritesCreated)
    {
        for (i = 0; i < NUM_FOG_HORIZONTAL_SPRITES; i++)
        {
            if (gWeatherPtr->sprites.s2.fogHSprites[i] != NULL)
                DestroySprite(gWeatherPtr->sprites.s2.fogHSprites[i]);
        }

        FreeSpriteTilesByTag(GFXTAG_FOG_H);
        gWeatherPtr->fogHSpritesCreated = 0;
    }
}

#undef tSpriteColumn

//------------------------------------------------------------------------------
// WEATHER_VOLCANIC_ASH
//------------------------------------------------------------------------------

static void LoadAshSpriteSheet(void);
static void CreateAshSprites(void);
static void DestroyAshSprites(void);
static void UpdateAshSprite(struct Sprite *);

void Ash_InitVars(void)
{
    gWeatherPtr->initStep = 0;
    gWeatherPtr->weatherGfxLoaded = FALSE;
    gWeatherPtr->targetColorMapIndex = 0;
    gWeatherPtr->colorMapStepDelay = 20;
    gWeatherPtr->ashUnused = 20; // Never read
    if (!gWeatherPtr->ashSpritesCreated)
    {
        Weather_SetBlendCoeffs(0, 12);
        // SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(64, 63)); // These aren't valid blend coefficients!
    }
    gWeatherPtr->noShadows = FALSE;
}

void Ash_InitAll(void)
{
    Ash_InitVars();
    while (gWeatherPtr->weatherGfxLoaded == FALSE)
        Ash_Main();
}

void Ash_Main(void)
{
    gWeatherPtr->ashBaseSpritesX = gSpriteCoordOffsetX & 0x1FF;
    while (gWeatherPtr->ashBaseSpritesX >= DISPLAY_WIDTH)
        gWeatherPtr->ashBaseSpritesX -= DISPLAY_WIDTH;

    switch (gWeatherPtr->initStep)
    {
    case 0:
        LoadAshSpriteSheet();
        gWeatherPtr->initStep++;
        break;
    case 1:
        if (!gWeatherPtr->ashSpritesCreated)
            CreateAshSprites();

        Weather_SetTargetBlendCoeffs(10, 12, 1);
        gWeatherPtr->initStep++;
        break;
    case 2:
        if (Weather_UpdateBlend())
        {
            gWeatherPtr->weatherGfxLoaded = TRUE;
            gWeatherPtr->initStep++;
        }
        break;
    default:
        Weather_UpdateBlend();
        break;
    }
}

bool8 Ash_Finish(void)
{
    switch (gWeatherPtr->finishStep)
    {
    case 0:
        Weather_SetTargetBlendCoeffs(0, 12, 1);
        gWeatherPtr->finishStep++;
        break;
    case 1:
        if (Weather_UpdateBlend())
        {
            DestroyAshSprites();
            gWeatherPtr->finishStep++;
        }
        break;
    case 2:
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        gWeatherPtr->finishStep++;
        return FALSE;
    default:
        return FALSE;
    }
    return TRUE;
}

static const struct SpriteSheet sAshSpriteSheet =
{
    .data = gWeatherAshTiles,
    .size = sizeof(gWeatherAshTiles),
    .tag = GFXTAG_ASH,
};

static void LoadAshSpriteSheet(void)
{
    LoadSpriteSheet(&sAshSpriteSheet);
}

static const struct OamData sAshSpriteOamData =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_BLEND,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 15,
};

static const union AnimCmd sAshSpriteAnimCmd0[] =
{
    ANIMCMD_FRAME(0, 60),
    ANIMCMD_FRAME(64, 60),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd *const sAshSpriteAnimCmds[] =
{
    sAshSpriteAnimCmd0,
};

static const struct SpriteTemplate sAshSpriteTemplate =
{
    .tileTag = GFXTAG_ASH,
    .paletteTag = PALTAG_WEATHER,
    .oam = &sAshSpriteOamData,
    .anims = sAshSpriteAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateAshSprite,
};

#define tOffsetY      data[0]
#define tCounterY     data[1]
#define tSpriteColumn data[2]
#define tSpriteRow    data[3]

static void CreateAshSprites(void)
{
    u8 i;
    u8 spriteId;
    struct Sprite *sprite;

    if (!gWeatherPtr->ashSpritesCreated)
    {
        for (i = 0; i < NUM_ASH_SPRITES; i++)
        {
            spriteId = CreateSpriteAtEnd(&sAshSpriteTemplate, 0, 0, 0x4E);
            if (spriteId != MAX_SPRITES)
            {
                sprite = &gSprites[spriteId];
                sprite->tCounterY = 0;
                sprite->tSpriteColumn = (u8)(i % 5);
                sprite->tSpriteRow = (u8)(i / 5);
                sprite->tOffsetY = sprite->tSpriteRow * 64 + 32;
                gWeatherPtr->sprites.s2.ashSprites[i] = sprite;
            }
            else
            {
                gWeatherPtr->sprites.s2.ashSprites[i] = NULL;
            }
        }

        gWeatherPtr->ashSpritesCreated = TRUE;
    }
}

static void DestroyAshSprites(void)
{
    u16 i;

    if (gWeatherPtr->ashSpritesCreated)
    {
        for (i = 0; i < NUM_ASH_SPRITES; i++)
        {
            if (gWeatherPtr->sprites.s2.ashSprites[i] != NULL)
                DestroySprite(gWeatherPtr->sprites.s2.ashSprites[i]);
        }

        FreeSpriteTilesByTag(GFXTAG_ASH);
        gWeatherPtr->ashSpritesCreated = FALSE;
    }
}

static void UpdateAshSprite(struct Sprite *sprite)
{
    if (++sprite->tCounterY > 5)
    {
        sprite->tCounterY = 0;
        sprite->tOffsetY++;
    }

    sprite->y = gSpriteCoordOffsetY + sprite->tOffsetY;
    sprite->x = gWeatherPtr->ashBaseSpritesX + 32 + sprite->tSpriteColumn * 64;
    if (sprite->x >= DISPLAY_WIDTH + 32)
    {
        sprite->x = gWeatherPtr->ashBaseSpritesX + (DISPLAY_WIDTH * 2) - (4 - sprite->tSpriteColumn) * 64;
        sprite->x &= 0x1FF;
    }
}

#undef tOffsetY
#undef tCounterY
#undef tSpriteColumn
#undef tSpriteRow

//------------------------------------------------------------------------------
// WEATHER_FOG_DIAGONAL
//------------------------------------------------------------------------------

static void UpdateFogDiagonalMovement(void);
static void CreateFogDiagonalSprites(void);
static void DestroyFogDiagonalSprites(void);
static void UpdateFogDiagonalSprite(struct Sprite *);

void FogDiagonal_InitVars(void)
{
    gWeatherPtr->initStep = 0;
    gWeatherPtr->weatherGfxLoaded = 0;
    gWeatherPtr->targetColorMapIndex = 0;
    gWeatherPtr->colorMapStepDelay = 20;
    gWeatherPtr->fogHScrollCounter = 0;
    gWeatherPtr->fogHScrollOffset = 1;
    if (!gWeatherPtr->fogDSpritesCreated)
    {
        gWeatherPtr->fogDScrollXCounter = 0;
        gWeatherPtr->fogDScrollYCounter = 0;
        gWeatherPtr->fogDXOffset = 0;
        gWeatherPtr->fogDYOffset = 0;
        gWeatherPtr->fogDBaseSpritesX = 0;
        gWeatherPtr->fogDPosY = 0;
        Weather_SetBlendCoeffs(0, 16);
    }
    gWeatherPtr->noShadows = TRUE;
}

void FogDiagonal_InitAll(void)
{
    FogDiagonal_InitVars();
    while (gWeatherPtr->weatherGfxLoaded == FALSE)
        FogDiagonal_Main();
}

void FogDiagonal_Main(void)
{
    UpdateFogDiagonalMovement();
    switch (gWeatherPtr->initStep)
    {
    case 0:
        CreateFogDiagonalSprites();
        gWeatherPtr->initStep++;
        break;
    case 1:
        Weather_SetTargetBlendCoeffs(12, 8, 8);
        gWeatherPtr->initStep++;
        break;
    case 2:
        if (!Weather_UpdateBlend())
            break;
        gWeatherPtr->weatherGfxLoaded = TRUE;
        gWeatherPtr->initStep++;
        break;
    }
}

bool8 FogDiagonal_Finish(void)
{
    UpdateFogDiagonalMovement();
    switch (gWeatherPtr->finishStep)
    {
    case 0:
        Weather_SetTargetBlendCoeffs(0, 16, 1);
        gWeatherPtr->finishStep++;
        break;
    case 1:
        if (!Weather_UpdateBlend())
            break;
        gWeatherPtr->finishStep++;
        break;
    case 2:
        DestroyFogDiagonalSprites();
        gWeatherPtr->finishStep++;
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

static void UpdateFogDiagonalMovement(void)
{
    if (++gWeatherPtr->fogDScrollXCounter > 2)
    {
        gWeatherPtr->fogDXOffset++;
        gWeatherPtr->fogDScrollXCounter = 0;
    }

    if (++gWeatherPtr->fogDScrollYCounter > 4)
    {
        gWeatherPtr->fogDYOffset++;
        gWeatherPtr->fogDScrollYCounter = 0;
    }

    gWeatherPtr->fogDBaseSpritesX = (gSpriteCoordOffsetX - gWeatherPtr->fogDXOffset) & 0xFF;
    gWeatherPtr->fogDPosY = gSpriteCoordOffsetY + gWeatherPtr->fogDYOffset;
}

static const struct SpriteSheet sFogDiagonalSpriteSheet =
{
    .data = gWeatherFogDiagonalTiles,
    .size = sizeof(gWeatherFogDiagonalTiles),
    .tag = GFXTAG_FOG_D,
};

static const struct OamData sFogDiagonalSpriteOamData =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_BLEND,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 2,
    .paletteNum = 0,
};

static const union AnimCmd sFogDiagonalSpriteAnimCmd0[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_END,
};

static const union AnimCmd *const sFogDiagonalSpriteAnimCmds[] =
{
    sFogDiagonalSpriteAnimCmd0,
};

static const struct SpriteTemplate sFogDiagonalSpriteTemplate =
{
    .tileTag = GFXTAG_FOG_D,
    .paletteTag = PALTAG_WEATHER,
    .oam = &sFogDiagonalSpriteOamData,
    .anims = sFogDiagonalSpriteAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateFogDiagonalSprite,
};

#define tSpriteColumn data[0]
#define tSpriteRow    data[1]

static void CreateFogDiagonalSprites(void)
{
    u16 i;
    struct SpriteSheet fogDiagonalSpriteSheet;
    u8 spriteId;
    struct Sprite *sprite;

    if (!gWeatherPtr->fogDSpritesCreated)
    {
        fogDiagonalSpriteSheet = sFogDiagonalSpriteSheet;
        LoadSpriteSheet(&fogDiagonalSpriteSheet);
        for (i = 0; i < NUM_FOG_DIAGONAL_SPRITES; i++)
        {
            spriteId = CreateSpriteAtEnd(&sFogDiagonalSpriteTemplate, 0, (i / 5) * 64, 0xFF);
            if (spriteId != MAX_SPRITES)
            {
                sprite = &gSprites[spriteId];
                sprite->tSpriteColumn = i % 5;
                sprite->tSpriteRow = i / 5;
                gWeatherPtr->sprites.s2.fogDSprites[i] = sprite;
            }
            else
            {
                gWeatherPtr->sprites.s2.fogDSprites[i] = NULL;
            }
        }

        gWeatherPtr->fogDSpritesCreated = TRUE;
    }
}

static void DestroyFogDiagonalSprites(void)
{
    u16 i;

    if (gWeatherPtr->fogDSpritesCreated)
    {
        for (i = 0; i < NUM_FOG_DIAGONAL_SPRITES; i++)
        {
            if (gWeatherPtr->sprites.s2.fogDSprites[i])
                DestroySprite(gWeatherPtr->sprites.s2.fogDSprites[i]);
        }

        FreeSpriteTilesByTag(GFXTAG_FOG_D);
        gWeatherPtr->fogDSpritesCreated = FALSE;
    }
}

static void UpdateFogDiagonalSprite(struct Sprite *sprite)
{
    sprite->y2 = gWeatherPtr->fogDPosY;
    sprite->x = gWeatherPtr->fogDBaseSpritesX + 32 + sprite->tSpriteColumn * 64;
    if (sprite->x >= DISPLAY_WIDTH + 32)
    {
        sprite->x = gWeatherPtr->fogDBaseSpritesX + (DISPLAY_WIDTH * 2) - (4 - sprite->tSpriteColumn) * 64;
        sprite->x &= 0x1FF;
    }
}

#undef tSpriteColumn
#undef tSpriteRow

//------------------------------------------------------------------------------
// Fog over the map's top layer
//------------------------------------------------------------------------------

// Colors in the fog's palette that average to the average color of the fog graphics
#define FOG_AVERAGE_COLOR_1 5
#define FOG_AVERAGE_COLOR_2 6

EWRAM_DATA static bool8 sFogLightensTopLayer = FALSE;

// How much white would lighten a color about as much as the fog blended over it does.
static u32 GetFogLightness(void)
{
    u32 paletteNum = IndexOfSpritePaletteTag(PALTAG_WEATHER);
    u32 eva = BLDALPHA_TGT1(GetGpuReg(REG_OFFSET_BLDALPHA));
    u32 i, color, brightness = 0;

    if (paletteNum == 0xFF)
        return 0;
    if (eva > 16)
        eva = 16;
    // Read after any fade, so the top layer fades along with the fog.
    for (i = FOG_AVERAGE_COLOR_1; i <= FOG_AVERAGE_COLOR_2; i++)
    {
        color = gPlttBufferFaded[OBJ_PLTT_ID(paletteNum) + i];
        brightness += GET_R(color) + GET_G(color) + GET_B(color);
    }
    // (eva / 16) * (brightness / (31 * 6)) * 16, rounded
    return (eva * brightness + 31 * 3) / (31 * 6);
}

// About the color of the fog's sprites, after any fade
u16 Fog_GetAverageColor(void)
{
    u32 paletteNum = IndexOfSpritePaletteTag(PALTAG_WEATHER);
    const u16 *colors;

    if (paletteNum == 0xFF)
        return RGB_WHITE;
    colors = &gPlttBufferFaded[OBJ_PLTT_ID(paletteNum)];
    return RGB((GET_R(colors[FOG_AVERAGE_COLOR_1]) + GET_R(colors[FOG_AVERAGE_COLOR_2])) / 2,
               (GET_G(colors[FOG_AVERAGE_COLOR_1]) + GET_G(colors[FOG_AVERAGE_COLOR_2])) / 2,
               (GET_B(colors[FOG_AVERAGE_COLOR_1]) + GET_B(colors[FOG_AVERAGE_COLOR_2])) / 2);
}

// Called each overworld frame, after the OAM buffer is built and palettes are faded.
// Fog sprites are drawn right over BG2, so the map's top layer on BG1 isn't under them.
// UpdateMapTopLayerUnderFog draws the top layer on BG2 instead wherever no sprite needs it
// covering it. What has to stay on BG1 is lightened by about as much as the fog lightens the
// rest of the map, like sprites are in fog. Blended sprites still blend in this mode.
void Fog_UpdateMapTopLayer(void)
{
    u32 tileStart = 0xFFFF;
    u32 tileCount = 0;
    u32 bldCnt = GetGpuReg(REG_OFFSET_BLDCNT);
    u32 effect = bldCnt & (BLDCNT_TGT1_ALL | BLDCNT_EFFECT_EFF_MASK);
    bool32 isFogOut;

    if (gWeatherPtr->fogHSpritesCreated)
    {
        tileStart = GetSpriteTileStartByTag(GFXTAG_FOG_H);
        tileCount = sizeof(gWeatherFogHorizontalTiles) / TILE_SIZE_4BPP;
    }
    else if (gWeatherPtr->fogDSpritesCreated)
    {
        tileStart = GetSpriteTileStartByTag(GFXTAG_FOG_D);
        tileCount = sizeof(gWeatherFogDiagonalTiles) / TILE_SIZE_4BPP;
    }
    // The sprites are gone without being destroyed when returning from another screen.
    isFogOut = (tileStart != 0xFFFF);
    UpdateMapTopLayerUnderFog(isFogOut, tileStart, tileCount);

    if (isFogOut)
    {
        // Left alone while something else is using the blend registers.
        if (effect == BLDCNT_EFFECT_NONE || effect == BLDCNT_EFFECT_BLEND || effect == BLDCNT_FOG_TOP_LAYER)
        {
            // Not written again once set: that would undo, on the first scanline, what a
            // scanline effect sets this register to (see UpdateBannerBlend)
            if (effect != BLDCNT_FOG_TOP_LAYER)
                SetGpuReg(REG_OFFSET_BLDCNT, (bldCnt & BLDCNT_TGT2_ALL) | BLDCNT_FOG_TOP_LAYER);
            SetGpuReg(REG_OFFSET_BLDY, GetFogLightness());
            sFogLightensTopLayer = TRUE;
        }
    }
    else if (sFogLightensTopLayer)
    {
        sFogLightensTopLayer = FALSE;
        if (effect == BLDCNT_FOG_TOP_LAYER)
        {
            SetGpuReg(REG_OFFSET_BLDCNT, (bldCnt & BLDCNT_TGT2_ALL) | BLDCNT_EFFECT_BLEND);
            SetGpuReg(REG_OFFSET_BLDY, 0);
        }
    }
}

//------------------------------------------------------------------------------
// WEATHER_SANDSTORM
//------------------------------------------------------------------------------

static void UpdateSandstormWaveIndex(void);
static void UpdateSandstormMovement(void);
static void CreateSandstormSprites(void);
static void CreateSwirlSandstormSprites(void);
static void DestroySandstormSprites(void);
static void UpdateSandstormSprite(struct Sprite *);
static void WaitSandSwirlSpriteEntrance(struct Sprite *);
static void UpdateSandstormSwirlSprite(struct Sprite *);

#define MIN_SANDSTORM_WAVE_INDEX 0x20

void Sandstorm_InitVars(void)
{
    gWeatherPtr->initStep = 0;
    gWeatherPtr->weatherGfxLoaded = 0;
    gWeatherPtr->targetColorMapIndex = 0;
    gWeatherPtr->colorMapStepDelay = 20;
    if (!gWeatherPtr->sandstormSpritesCreated)
    {
        gWeatherPtr->sandstormXOffset = gWeatherPtr->sandstormYOffset = 0;
        gWeatherPtr->sandstormWaveIndex = 8;
        gWeatherPtr->sandstormWaveCounter = 0;
        // Dead code. How does the compiler not optimize this out?
        if (gWeatherPtr->sandstormWaveIndex >= 0x80 - MIN_SANDSTORM_WAVE_INDEX)
            gWeatherPtr->sandstormWaveIndex = 0x80 - gWeatherPtr->sandstormWaveIndex;

        Weather_SetBlendCoeffs(0, 16);
    }
    gWeatherPtr->noShadows = FALSE;
}

void Sandstorm_InitAll(void)
{
    Sandstorm_InitVars();
    while (!gWeatherPtr->weatherGfxLoaded)
        Sandstorm_Main();
}

void Sandstorm_Main(void)
{
    UpdateSandstormMovement();
    UpdateSandstormWaveIndex();
    if (gWeatherPtr->sandstormWaveIndex >= 0x80 - MIN_SANDSTORM_WAVE_INDEX)
        gWeatherPtr->sandstormWaveIndex = MIN_SANDSTORM_WAVE_INDEX;

    switch (gWeatherPtr->initStep)
    {
    case 0:
        CreateSandstormSprites();
        CreateSwirlSandstormSprites();
        gWeatherPtr->initStep++;
        break;
    case 1:
        Weather_SetTargetBlendCoeffs(16, 2, 0);
        UpdateShadowColor(0x3DEF);
        gWeatherPtr->initStep++;
        break;
    case 2:
        if (Weather_UpdateBlend())
        {
            gWeatherPtr->weatherGfxLoaded = TRUE;
            gWeatherPtr->initStep++;
        }
        break;
    }
}

bool8 Sandstorm_Finish(void)
{
    UpdateSandstormMovement();
    UpdateSandstormWaveIndex();
    switch (gWeatherPtr->finishStep)
    {
    case 0:
        Weather_SetTargetBlendCoeffs(0, 16, 0);
        gWeatherPtr->finishStep++;
        break;
    case 1:
        if (Weather_UpdateBlend())
            gWeatherPtr->finishStep++;
        if (gWeatherPtr->currBlendEVB == 12)
          UpdateShadowColor(RGB_BLACK);
        break;
    case 2:
        DestroySandstormSprites();
        UpdateShadowColor(RGB_BLACK);
        gWeatherPtr->finishStep++;
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

static void UpdateSandstormWaveIndex(void)
{
    if (gWeatherPtr->sandstormWaveCounter++ > 4)
    {
        gWeatherPtr->sandstormWaveIndex++;
        gWeatherPtr->sandstormWaveCounter = 0;
    }
}

static void UpdateSandstormMovement(void)
{
    gWeatherPtr->sandstormXOffset -= gSineTable[gWeatherPtr->sandstormWaveIndex] * 4;
    gWeatherPtr->sandstormYOffset -= gSineTable[gWeatherPtr->sandstormWaveIndex];
    gWeatherPtr->sandstormBaseSpritesX = (gSpriteCoordOffsetX + (gWeatherPtr->sandstormXOffset >> 8)) & 0xFF;
    gWeatherPtr->sandstormPosY = gSpriteCoordOffsetY + (gWeatherPtr->sandstormYOffset >> 8);
}

static void DestroySandstormSprites(void)
{
    u16 i;

    if (gWeatherPtr->sandstormSpritesCreated)
    {
        for (i = 0; i < NUM_SANDSTORM_SPRITES; i++)
        {
            if (gWeatherPtr->sprites.s2.sandstormSprites1[i])
                DestroySprite(gWeatherPtr->sprites.s2.sandstormSprites1[i]);
        }

        gWeatherPtr->sandstormSpritesCreated = FALSE;
        FreeSpriteTilesByTag(GFXTAG_SANDSTORM);
    }

    if (gWeatherPtr->sandstormSwirlSpritesCreated)
    {
        for (i = 0; i < NUM_SWIRL_SANDSTORM_SPRITES; i++)
        {
            if (gWeatherPtr->sprites.s2.sandstormSprites2[i] != NULL)
                DestroySprite(gWeatherPtr->sprites.s2.sandstormSprites2[i]);
        }

        gWeatherPtr->sandstormSwirlSpritesCreated = FALSE;
    }
}

static const struct OamData sSandstormSpriteOamData =
{
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_BLEND,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
};

static const union AnimCmd sSandstormSpriteAnimCmd0[] =
{
    ANIMCMD_FRAME(0, 3),
    ANIMCMD_END,
};

static const union AnimCmd sSandstormSpriteAnimCmd1[] =
{
    ANIMCMD_FRAME(64, 3),
    ANIMCMD_END,
};

static const union AnimCmd *const sSandstormSpriteAnimCmds[] =
{
    sSandstormSpriteAnimCmd0,
    sSandstormSpriteAnimCmd1,
};

static const struct SpriteTemplate sSandstormSpriteTemplate =
{
    .tileTag = GFXTAG_SANDSTORM,
    .paletteTag = PALTAG_WEATHER_2,
    .oam = &sSandstormSpriteOamData,
    .anims = sSandstormSpriteAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateSandstormSprite,
};

static const struct SpriteSheet sSandstormSpriteSheet =
{
    .data = gWeatherSandstormTiles,
    .size = sizeof(gWeatherSandstormTiles),
    .tag = GFXTAG_SANDSTORM,
};

// Regular sandstorm sprites
#define tSpriteColumn  data[0]
#define tSpriteRow     data[1]

// Swirly sandstorm sprites
#define tRadius        data[0]
#define tWaveIndex     data[1]
#define tRadiusCounter data[2]
#define tEntranceDelay data[3]

static void CreateSandstormSprites(void)
{
    u16 i;
    u8 spriteId;

    if (!gWeatherPtr->sandstormSpritesCreated)
    {
        LoadSpriteSheet(&sSandstormSpriteSheet);
        LoadCustomWeatherSpritePalette(gSandstormWeatherPalette);
        for (i = 0; i < NUM_SANDSTORM_SPRITES; i++)
        {
            spriteId = CreateSpriteAtEnd(&sSandstormSpriteTemplate, 0, (i / 5) * 64, 1);
            if (spriteId != MAX_SPRITES)
            {
                gWeatherPtr->sprites.s2.sandstormSprites1[i] = &gSprites[spriteId];
                gWeatherPtr->sprites.s2.sandstormSprites1[i]->tSpriteColumn = i % 5;
                gWeatherPtr->sprites.s2.sandstormSprites1[i]->tSpriteRow = i / 5;
            }
            else
            {
                gWeatherPtr->sprites.s2.sandstormSprites1[i] = NULL;
            }
        }

        gWeatherPtr->sandstormSpritesCreated = TRUE;
    }
}

static const u16 sSwirlEntranceDelays[] = {0, 120, 80, 160, 40, 0};

static void CreateSwirlSandstormSprites(void)
{
    u16 i;
    u8 spriteId;

    if (!gWeatherPtr->sandstormSwirlSpritesCreated)
    {
        for (i = 0; i < NUM_SWIRL_SANDSTORM_SPRITES; i++)
        {
            spriteId = CreateSpriteAtEnd(&sSandstormSpriteTemplate, i * 48 + 24, 208, 1);
            if (spriteId != MAX_SPRITES)
            {
                gWeatherPtr->sprites.s2.sandstormSprites2[i] = &gSprites[spriteId];
                gWeatherPtr->sprites.s2.sandstormSprites2[i]->oam.size = ST_OAM_SIZE_2;
                gWeatherPtr->sprites.s2.sandstormSprites2[i]->tSpriteRow = i * 51;
                gWeatherPtr->sprites.s2.sandstormSprites2[i]->tRadius = 8;
                gWeatherPtr->sprites.s2.sandstormSprites2[i]->tRadiusCounter = 0;
                gWeatherPtr->sprites.s2.sandstormSprites2[i]->data[4] = 0x6730; // unused value
                gWeatherPtr->sprites.s2.sandstormSprites2[i]->tEntranceDelay = sSwirlEntranceDelays[i];
                StartSpriteAnim(gWeatherPtr->sprites.s2.sandstormSprites2[i], 1);
                CalcCenterToCornerVec(gWeatherPtr->sprites.s2.sandstormSprites2[i], SPRITE_SHAPE(32x32), SPRITE_SIZE(32x32), ST_OAM_AFFINE_OFF);
                gWeatherPtr->sprites.s2.sandstormSprites2[i]->callback = WaitSandSwirlSpriteEntrance;
            }
            else
            {
                gWeatherPtr->sprites.s2.sandstormSprites2[i] = NULL;
            }

            gWeatherPtr->sandstormSwirlSpritesCreated = TRUE;
        }
    }
}

static void UpdateSandstormSprite(struct Sprite *sprite)
{
    sprite->y2 = gWeatherPtr->sandstormPosY;
    sprite->x = gWeatherPtr->sandstormBaseSpritesX + 32 + sprite->tSpriteColumn * 64;
    if (sprite->x >= DISPLAY_WIDTH + 32)
    {
        sprite->x = gWeatherPtr->sandstormBaseSpritesX + (DISPLAY_WIDTH * 2) - (4 - sprite->tSpriteColumn) * 64;
        sprite->x &= 0x1FF;
    }
}

static void WaitSandSwirlSpriteEntrance(struct Sprite *sprite)
{
    if (--sprite->tEntranceDelay == -1)
        sprite->callback = UpdateSandstormSwirlSprite;
}

static void UpdateSandstormSwirlSprite(struct Sprite *sprite)
{
    u32 x, y;

    if (--sprite->y < -48)
    {
        sprite->y = DISPLAY_HEIGHT + 48;
        sprite->tRadius = 4;
    }

    x = sprite->tRadius * gSineTable[sprite->tWaveIndex];
    y = sprite->tRadius * gSineTable[sprite->tWaveIndex + 0x40];
    sprite->x2 = x >> 8;
    sprite->y2 = y >> 8;
    sprite->tWaveIndex = (sprite->tWaveIndex + 10) & 0xFF;
    if (++sprite->tRadiusCounter > 8)
    {
        sprite->tRadiusCounter = 0;
        sprite->tRadius++;
    }
}

#undef tSpriteColumn
#undef tSpriteRow

#undef tRadius
#undef tWaveIndex
#undef tRadiusCounter
#undef tEntranceDelay

//------------------------------------------------------------------------------
// WEATHER_SHADE
//------------------------------------------------------------------------------

void Shade_InitVars(void)
{
    gWeatherPtr->initStep = 0;
    gWeatherPtr->targetColorMapIndex = 3;
    gWeatherPtr->colorMapStepDelay = 20;
    Weather_SetBlendCoeffs(8, 12); // preserve shadow darkness
    gWeatherPtr->noShadows = FALSE;
}

void Shade_InitAll(void)
{
    Shade_InitVars();
}

void Shade_Main(void)
{
}

bool8 Shade_Finish(void)
{
    return FALSE;
}

//------------------------------------------------------------------------------
// WEATHER_UNDERWATER_BUBBLES
//------------------------------------------------------------------------------

static void CreateBubbleSprite(u16);
static void DestroyBubbleSprites(void);
static void UpdateBubbleSprite(struct Sprite *);

static const u8 sBubbleStartDelays[] = {40, 90, 60, 90, 2, 60, 40, 30};

static const struct SpriteSheet sWeatherBubbleSpriteSheet =
{
    .data = gWeatherBubbleTiles,
    .size = sizeof(gWeatherBubbleTiles),
    .tag = GFXTAG_BUBBLE,
};

static const s16 sBubbleStartCoords[][2] =
{
    {120, 160},
    {376, 160},
    { 40, 140},
    {296, 140},
    {180, 130},
    {436, 130},
    { 60, 160},
    {436, 160},
    {220, 180},
    {476, 180},
    { 10,  90},
    {266,  90},
    {256, 160},
};

void Bubbles_InitVars(void)
{
    FogHorizontal_InitVars();
    if (!gWeatherPtr->bubblesSpritesCreated)
    {
        LoadSpriteSheet(&sWeatherBubbleSpriteSheet);
        gWeatherPtr->bubblesDelayIndex = 0;
        gWeatherPtr->bubblesDelayCounter = sBubbleStartDelays[0];
        gWeatherPtr->bubblesCoordsIndex = 0;
        gWeatherPtr->bubblesSpriteCount = 0;
    }
    gWeatherPtr->noShadows = TRUE;
}

void Bubbles_InitAll(void)
{
    Bubbles_InitVars();
    while (!gWeatherPtr->weatherGfxLoaded)
        Bubbles_Main();
}

void Bubbles_Main(void)
{
    FogHorizontal_Main();
    if (++gWeatherPtr->bubblesDelayCounter > sBubbleStartDelays[gWeatherPtr->bubblesDelayIndex])
    {
        gWeatherPtr->bubblesDelayCounter = 0;
        if (++gWeatherPtr->bubblesDelayIndex > ARRAY_COUNT(sBubbleStartDelays) - 1)
            gWeatherPtr->bubblesDelayIndex = 0;

        CreateBubbleSprite(gWeatherPtr->bubblesCoordsIndex);
        if (++gWeatherPtr->bubblesCoordsIndex > ARRAY_COUNT(sBubbleStartCoords) - 1)
            gWeatherPtr->bubblesCoordsIndex = 0;
    }
}

bool8 Bubbles_Finish(void)
{
    if (!FogHorizontal_Finish())
    {
        DestroyBubbleSprites();
        return FALSE;
    }

    return TRUE;
}

static const union AnimCmd sBubbleSpriteAnimCmd0[] =
{
    ANIMCMD_FRAME(0, 16),
    ANIMCMD_FRAME(1, 16),
    ANIMCMD_END,
};

static const union AnimCmd *const sBubbleSpriteAnimCmds[] =
{
    sBubbleSpriteAnimCmd0,
};

static const struct SpriteTemplate sBubbleSpriteTemplate =
{
    .tileTag = GFXTAG_BUBBLE,
    .paletteTag = PALTAG_WEATHER,
    .oam = &gOamData_AffineOff_ObjNormal_8x8,
    .anims = sBubbleSpriteAnimCmds,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = UpdateBubbleSprite,
};

#define tScrollXCounter data[0]
#define tScrollXDir     data[1]
#define tCounter        data[2]

static void CreateBubbleSprite(u16 coordsIndex)
{
    s16 x = sBubbleStartCoords[coordsIndex][0];
    s16 y = sBubbleStartCoords[coordsIndex][1] - gSpriteCoordOffsetY;
    u8 spriteId = CreateSpriteAtEnd(&sBubbleSpriteTemplate, x, y, 0);
    if (spriteId != MAX_SPRITES)
    {
        gSprites[spriteId].oam.priority = 1;
        gSprites[spriteId].coordOffsetEnabled = TRUE;
        gSprites[spriteId].tScrollXCounter = 0;
        gSprites[spriteId].tScrollXDir = 0;
        gSprites[spriteId].tCounter = 0;
        gWeatherPtr->bubblesSpriteCount++;
    }
}

static void DestroyBubbleSprites(void)
{
    u16 i;

    if (gWeatherPtr->bubblesSpriteCount)
    {
        for (i = 0; i < MAX_SPRITES; i++)
        {
            if (gSprites[i].template == &sBubbleSpriteTemplate)
                DestroySprite(&gSprites[i]);
        }

        FreeSpriteTilesByTag(GFXTAG_BUBBLE);
        gWeatherPtr->bubblesSpriteCount = 0;
    }
}

static void UpdateBubbleSprite(struct Sprite *sprite)
{
    ++sprite->tScrollXCounter;
    if (++sprite->tScrollXCounter > 8) // double increment
    {
        sprite->tScrollXCounter = 0;
        if (sprite->tScrollXDir == 0)
        {
            if (++sprite->x2 > 4)
                sprite->tScrollXDir = 1;
        }
        else
        {
            if (--sprite->x2 <= 0)
                sprite->tScrollXDir = 0;
        }
    }

    sprite->y -= 3;
    if (++sprite->tCounter >= 120)
        DestroySprite(sprite);
}

#undef tScrollXCounter
#undef tScrollXDir
#undef tCounter

//------------------------------------------------------------------------------

static void UNUSED UnusedSetCurrentAbnormalWeather(u32 weather, u32 unknown)
{
    sCurrentAbnormalWeather = weather;
    sUnusedWeatherRelated = unknown;
}

#define tState         data[0]
#define tWeatherA      data[1]
#define tWeatherB      data[2]
#define tDelay         data[15]

static void Task_DoAbnormalWeather(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    switch (tState)
    {
    case 0:
        if (tDelay-- <= 0)
        {
            SetNextWeather(tWeatherA);
            sCurrentAbnormalWeather = tWeatherA;
            tDelay = 600;
            tState++;
        }
        break;
    case 1:
        if (tDelay-- <= 0)
        {
            SetNextWeather(tWeatherB);
            sCurrentAbnormalWeather = tWeatherB;
            tDelay = 600;
            tState = 0;
        }
        break;
    }
}

static void CreateAbnormalWeatherTask(void)
{
    u8 taskId = CreateTask(Task_DoAbnormalWeather, 0);
    s16 *data = gTasks[taskId].data;

    tDelay = 600;
    if (sCurrentAbnormalWeather == WEATHER_DOWNPOUR)
    {
        // Currently Downpour, next will be Drought
        tWeatherA = WEATHER_DROUGHT;
        tWeatherB = WEATHER_DOWNPOUR;
    }
    else if (sCurrentAbnormalWeather == WEATHER_DROUGHT)
    {
        // Currently Drought, next will be Downpour
        tWeatherA = WEATHER_DOWNPOUR;
        tWeatherB = WEATHER_DROUGHT;
    }
    else
    {
        // Default to starting with Downpour
        sCurrentAbnormalWeather = WEATHER_DOWNPOUR;
        tWeatherA = WEATHER_DROUGHT;
        tWeatherB = WEATHER_DOWNPOUR;
    }
}

#undef tState
#undef tWeatherA
#undef tWeatherB
#undef tDelay

static u8 TranslateWeatherNum(u8);
static void UpdateRainCounter(u8, u8);

// Seasonal route weather (src/data/seasons.h). The block macros below turn each map's block
// into a list of numbers: an opcode followed by its operands, with every block closed by
// SW_END. GetCurrMapSeasonalWeather walks that list.
#define WEATHER_REGULAR 0xFF // the map's own weather from its header

// The opcodes are in map group 0xFF, which has no maps, so the maps of a MAP block are
// everything up to the first opcode. MAP_UNDEFINED (0xFFFF) is above the opcodes: it can be
// listed as a map, and since no map is ever MAP_UNDEFINED its block never applies.
enum
{
    SW_END = 0xFF00,
    SW_MAP,      // one or more MAP_* constants, then the block
    SW_SEASONS,  // SEASON_TAG_* mask, then the block
    SW_YEARS,    // YEAR_TAG_* mask, then the block
    SW_WEATHER,  // WEATHER_* or WEATHER_REGULAR, then a DAY_TAG_* mask (0 = every day)
    SW_OPCODES_END
};

#define IS_SW_OPCODE(value) ((value) >= SW_END && (value) < SW_OPCODES_END)

// Every block ends in a comma, so blocks are written one after another with no commas between.
#define MAP(...)            SW_MAP, __VA_ARGS__ SW_END,
#define SPRING(...)         SW_SEASONS, SEASON_TAG_SPRING, __VA_ARGS__ SW_END,
#define SUMMER(...)         SW_SEASONS, SEASON_TAG_SUMMER, __VA_ARGS__ SW_END,
#define FALL(...)           SW_SEASONS, SEASON_TAG_FALL, __VA_ARGS__ SW_END,
#define WINTER(...)         SW_SEASONS, SEASON_TAG_WINTER, __VA_ARGS__ SW_END,
#define EVEN_YEARS(...)     SW_YEARS, YEAR_TAG_EVEN, __VA_ARGS__ SW_END,
#define ODD_YEARS(...)      SW_YEARS, YEAR_TAG_ODD, __VA_ARGS__ SW_END,
#define WEATHER(...)        WEATHER_(__VA_ARGS__, 0)
#define WEATHER_(weather, days, ...) SW_WEATHER, (weather), (days),

#include "data/seasons.h"

#undef MAP
#undef SPRING
#undef SUMMER
#undef FALL
#undef WINTER
#undef EVEN_YEARS
#undef ODD_YEARS
#undef WEATHER
#undef WEATHER_

struct SeasonalWeatherRegion
{
    const u16 *list;
    u16 count;
};

static const struct SeasonalWeatherRegion sSeasonalWeatherRegions[] =
{
    {sSeasonalWeatherKanto, ARRAY_COUNT(sSeasonalWeatherKanto)},
    {sSeasonalWeatherJohto, ARRAY_COUNT(sSeasonalWeatherJohto)},
    {sSeasonalWeatherHoenn, ARRAY_COUNT(sSeasonalWeatherHoenn)},
};

struct SeasonalWeatherChoice
{
    u16 map;
    u8 weather;
    s8 specificity; // -1 until a WEATHER(...) applies
};

// Higher is more specific: limited to some days > limited to some seasons > limited to
// even or odd years > always.
static s8 GetSeasonalWeatherSpecificity(u8 seasons, u8 years, u8 days)
{
    s8 specificity = 0;

    if (days != 0 && (days & DAY_TAG_ALL) != DAY_TAG_ALL)
        specificity += 4;
    if ((seasons & SEASON_TAG_ALL) != SEASON_TAG_ALL)
        specificity += 2;
    if ((years & YEAR_TAG_ALL) != YEAR_TAG_ALL)
        specificity += 1;
    return specificity;
}

// Reads a block's contents up to its SW_END and returns the position after it. seasons and
// years are the masks of the blocks it's inside; only WEATHER(...) in a MAP block that lists
// the current map counts.
static const u16 *ReadSeasonalWeatherBlock(const u16 *pos, const u16 *end, bool8 inMap, u8 seasons, u8 years, struct SeasonalWeatherChoice *choice)
{
    while (pos < end)
    {
        switch (*pos++)
        {
        case SW_END:
            return pos;
        case SW_MAP:
        {
            bool8 isListed = FALSE;

            for (; pos < end && !IS_SW_OPCODE(*pos); pos++)
            {
                if (*pos == choice->map)
                    isListed = TRUE;
            }
            pos = ReadSeasonalWeatherBlock(pos, end, isListed, seasons, years, choice);
            break;
        }
        case SW_SEASONS:
            pos = ReadSeasonalWeatherBlock(pos + 1, end, inMap, seasons & pos[0], years, choice);
            break;
        case SW_YEARS:
            pos = ReadSeasonalWeatherBlock(pos + 1, end, inMap, seasons, years & pos[0], choice);
            break;
        case SW_WEATHER:
            if (inMap && seasons != 0 && IsInSeason(seasons) && years != 0 && IsInYear(years) && IsOnDay(pos[1]))
            {
                s8 specificity = GetSeasonalWeatherSpecificity(seasons, years, pos[1]);

                if (specificity > choice->specificity)
                {
                    choice->weather = pos[0];
                    choice->specificity = specificity;
                }
            }
            pos += 2;
            break;
        default:
            return end;
        }
    }
    return end;
}

// The current map's weather for today's year, season and day, or WEATHER_REGULAR if nothing
// in seasons.h applies (or what applies is WEATHER_REGULAR).
static u8 FindCurrMapSeasonalWeather(void)
{
    struct SeasonalWeatherChoice choice;
    u32 i;

    choice.map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    choice.weather = WEATHER_REGULAR;
    choice.specificity = -1;
    for (i = 0; i < ARRAY_COUNT(sSeasonalWeatherRegions); i++)
    {
        const u16 *list = sSeasonalWeatherRegions[i].list;

        ReadSeasonalWeatherBlock(list, list + sSeasonalWeatherRegions[i].count, FALSE, SEASON_TAG_ALL, YEAR_TAG_ALL, &choice);
    }
    return choice.weather;
}

// The current map's seasonal weather, or its header weather if it has none today.
static u8 GetCurrMapSeasonalWeather(void)
{
    u8 weather = FindCurrMapSeasonalWeather();

    if (weather == WEATHER_REGULAR)
        return gMapHeader.weather;
    return weather;
}

void SetSavedWeather(u32 weather)
{
    u8 oldWeather = gSaveBlock1Ptr->weather;
    gSaveBlock1Ptr->weather = TranslateWeatherNum(weather);
    UpdateRainCounter(gSaveBlock1Ptr->weather, oldWeather);
}

u8 GetSavedWeather(void)
{
    return gSaveBlock1Ptr->weather;
}

void SetSavedWeatherFromCurrMapHeader(void)
{
    u8 oldWeather = gSaveBlock1Ptr->weather;
    gSaveBlock1Ptr->weather = TranslateWeatherNum(GetCurrMapSeasonalWeather());
    UpdateRainCounter(gSaveBlock1Ptr->weather, oldWeather);
}

void SetWeather(u32 weather)
{
    SetSavedWeather(weather);
    SetNextWeather(GetSavedWeather());
}

static bool32 IsClearWeather(u32 weather)
{
    return weather == WEATHER_NONE || weather == WEATHER_SUNNY;
}

// Weather trigger tiles (e.g. the edges of Route 119's and 123's rainy areas). On a day the map
// has a seasonal weather, a tile that sets the map's own header weather sets the seasonal
// weather instead, so seasons.h covers the map's normal part while the trigger zones keep their
// weather. NONE and SUNNY count as the same, as Jagged Pass has a NONE header with SUNNY tiles.
void SetCoordEventWeather(u32 weather)
{
    if (weather == gMapHeader.weather || (IsClearWeather(weather) && IsClearWeather(gMapHeader.weather)))
    {
        u8 seasonalWeather = FindCurrMapSeasonalWeather();

        if (seasonalWeather != WEATHER_REGULAR)
            weather = seasonalWeather;
    }
    SetWeather(weather);
}

void SetWeather_Unused(u32 weather)
{
    SetSavedWeather(weather);
    SetCurrentAndNextWeather(GetSavedWeather());
}

void DoCurrentWeather(void)
{
    u8 weather = GetSavedWeather();

    if (weather == WEATHER_ABNORMAL)
    {
        if (!FuncIsActiveTask(Task_DoAbnormalWeather))
            CreateAbnormalWeatherTask();
        weather = sCurrentAbnormalWeather;
    }
    else
    {
        if (FuncIsActiveTask(Task_DoAbnormalWeather))
            DestroyTask(FindTaskIdByFunc(Task_DoAbnormalWeather));
        sCurrentAbnormalWeather = WEATHER_DOWNPOUR;
    }
    SetNextWeather(weather);
}

void ResumePausedWeather(void)
{
    u8 weather = GetSavedWeather();

    if (weather == WEATHER_ABNORMAL)
    {
        if (!FuncIsActiveTask(Task_DoAbnormalWeather))
            CreateAbnormalWeatherTask();
        weather = sCurrentAbnormalWeather;
    }
    else
    {
        if (FuncIsActiveTask(Task_DoAbnormalWeather))
            DestroyTask(FindTaskIdByFunc(Task_DoAbnormalWeather));
        sCurrentAbnormalWeather = WEATHER_DOWNPOUR;
    }
    SetCurrentAndNextWeather(weather);
}

#define WEATHER_CYCLE_LENGTH  4

static const u8 sWeatherCycleRoute119[WEATHER_CYCLE_LENGTH] =
{
    WEATHER_SUNNY,
    WEATHER_RAIN,
    WEATHER_RAIN_THUNDERSTORM,
    WEATHER_RAIN,
};
static const u8 sWeatherCycleRoute123[WEATHER_CYCLE_LENGTH] =
{
    WEATHER_SUNNY,
    WEATHER_SUNNY,
    WEATHER_RAIN,
    WEATHER_SUNNY,
};

static u8 TranslateWeatherNum(u8 weather)
{
    switch (weather)
    {
    case WEATHER_NONE:               return WEATHER_NONE;
    case WEATHER_SUNNY_CLOUDS:       return WEATHER_SUNNY_CLOUDS;
    case WEATHER_SUNNY:              return WEATHER_SUNNY;
    case WEATHER_RAIN:               return WEATHER_RAIN;
    case WEATHER_SNOW:               return WEATHER_SNOW;
    case WEATHER_RAIN_THUNDERSTORM:  return WEATHER_RAIN_THUNDERSTORM;
    case WEATHER_FOG_HORIZONTAL:     return WEATHER_FOG_HORIZONTAL;
    case WEATHER_VOLCANIC_ASH:       return WEATHER_VOLCANIC_ASH;
    case WEATHER_SANDSTORM:          return WEATHER_SANDSTORM;
    case WEATHER_FOG_DIAGONAL:       return WEATHER_FOG_DIAGONAL;
    case WEATHER_UNDERWATER:         return WEATHER_UNDERWATER;
    case WEATHER_SHADE:              return WEATHER_SHADE;
    case WEATHER_DROUGHT:            return WEATHER_DROUGHT;
    case WEATHER_DOWNPOUR:           return WEATHER_DOWNPOUR;
    case WEATHER_UNDERWATER_BUBBLES: return WEATHER_UNDERWATER_BUBBLES;
    case WEATHER_ABNORMAL:           return WEATHER_ABNORMAL;
    case WEATHER_FLURRIES:           return WEATHER_FLURRIES;
    case WEATHER_BLIZZARD:           return WEATHER_BLIZZARD;
    case WEATHER_DIAMOND_DUST:       return WEATHER_DIAMOND_DUST;
    case WEATHER_WINDY:              return WEATHER_WINDY;
    case WEATHER_ROUTE119_CYCLE:     return sWeatherCycleRoute119[gSaveBlock1Ptr->weatherCycleStage];
    case WEATHER_ROUTE123_CYCLE:     return sWeatherCycleRoute123[gSaveBlock1Ptr->weatherCycleStage];
    default:                         return WEATHER_NONE;
    }
}

void UpdateWeatherPerDay(u16 increment)
{
    u16 weatherStage = gSaveBlock1Ptr->weatherCycleStage + increment;
    weatherStage %= WEATHER_CYCLE_LENGTH;
    gSaveBlock1Ptr->weatherCycleStage = weatherStage;
}

static void UpdateRainCounter(u8 newWeather, u8 oldWeather)
{
    if (newWeather != oldWeather
     && (newWeather == WEATHER_RAIN || newWeather == WEATHER_RAIN_THUNDERSTORM))
        IncrementGameStat(GAME_STAT_GOT_RAINED_ON);
}

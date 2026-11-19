#include "global.h"
#include "berry.h"
#include "bike.h"
#include "field_camera.h"
#include "field_player_avatar.h"
#include "fieldmap.h"
#include "event_object_movement.h"
#include "gpu_regs.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "rotating_gate.h"
#include "sprite.h"
#include "text.h"

EWRAM_DATA bool8 gUnusedBikeCameraAheadPanback = FALSE;

struct FieldCameraOffset
{
    u8 xPixelOffset;
    u8 yPixelOffset;
    u8 xTileOffset;
    u8 yTileOffset;
    bool8 copyBGToVRAM;
};

static void RedrawMapSliceNorth(struct FieldCameraOffset *, const struct MapLayout *);
static void RedrawMapSliceSouth(struct FieldCameraOffset *, const struct MapLayout *);
static void RedrawMapSliceEast(struct FieldCameraOffset *, const struct MapLayout *);
static void RedrawMapSliceWest(struct FieldCameraOffset *, const struct MapLayout *);
static s32 MapPosToBgTilemapOffset(struct FieldCameraOffset *, s32, s32);
static void DrawWholeMapViewInternal(int, int, const struct MapLayout *);
static void DrawMetatileAt(const struct MapLayout *, u16, int, int);
static void DrawMetatile(s32, const u16 *, u16);
static void CameraPanningCB_PanAhead(void);

#define TILEMAP_WIDTH 32 // In tiles, as is its height
// How long a tile stays on BG1 after the last sprite leaves it. Sprites that bob, like
// followers idling, would otherwise flip the tiles at their edges back and forth.
#define SPRITE_COVER_HOLD_FRAMES 30

// Fog is made of sprites drawn right over BG2: any higher and it would hide the other sprites.
// That leaves the map's top layer on BG1 clear of it, so while fog is out the top layer is drawn
// on BG2 instead, except in the tiles a sprite's pixels overlap, where it stays on BG1 to cover
// the sprite.
static EWRAM_DATA bool8 sTopLayerUnderFog = FALSE;
static EWRAM_DATA u32 sTilesUnderSprites[TILEMAP_WIDTH] = {0}; // A row of the tilemap each
static EWRAM_DATA u32 sTilesWithTopLayer[TILEMAP_WIDTH] = {0}; // Ones whose top layer can go under the fog
static EWRAM_DATA u8 sSpriteCoverTimers[TILEMAP_WIDTH][TILEMAP_WIDTH] = {0};

static const u8 sOamSizes[4][4][2] =
{
    [ST_OAM_SQUARE]      = {{ 8,  8}, {16, 16}, {32, 32}, {64, 64}},
    [ST_OAM_H_RECTANGLE] = {{16,  8}, {32,  8}, {32, 16}, {64, 32}},
    [ST_OAM_V_RECTANGLE] = {{ 8, 16}, { 8, 32}, {16, 32}, {32, 64}},
};

static struct FieldCameraOffset sFieldCameraOffset;
static s16 sHorizontalCameraPan;
static s16 sVerticalCameraPan;
static bool8 sBikeCameraPanFlag;
static void (*sFieldCameraPanningCallback)(void);

COMMON_DATA struct CameraObject gFieldCamera = {0};
COMMON_DATA u16 gTotalCameraPixelOffsetY = 0;
COMMON_DATA u16 gTotalCameraPixelOffsetX = 0;

static void ResetCameraOffset(struct FieldCameraOffset *cameraOffset)
{
    cameraOffset->xTileOffset = 0;
    cameraOffset->yTileOffset = 0;
    cameraOffset->xPixelOffset = 0;
    cameraOffset->yPixelOffset = 0;
    cameraOffset->copyBGToVRAM = TRUE;
}

static void AddCameraTileOffset(struct FieldCameraOffset *cameraOffset, u32 xOffset, u32 yOffset)
{
    cameraOffset->xTileOffset += xOffset;
    cameraOffset->xTileOffset %= 32;
    cameraOffset->yTileOffset += yOffset;
    cameraOffset->yTileOffset %= 32;
}

static void AddCameraPixelOffset(struct FieldCameraOffset *cameraOffset, u32 xOffset, u32 yOffset)
{
    cameraOffset->xPixelOffset += xOffset;
    cameraOffset->yPixelOffset += yOffset;
}

void ResetFieldCamera(void)
{
    ResetCameraOffset(&sFieldCameraOffset);
}

void FieldUpdateBgTilemapScroll(void)
{
    u32 r4, r5;
    r5 = sFieldCameraOffset.xPixelOffset + sHorizontalCameraPan;
    r4 = sVerticalCameraPan + sFieldCameraOffset.yPixelOffset + 8;

    SetGpuReg(REG_OFFSET_BG1HOFS, r5);
    SetGpuReg(REG_OFFSET_BG1VOFS, r4);
    SetGpuReg(REG_OFFSET_BG2HOFS, r5);
    SetGpuReg(REG_OFFSET_BG2VOFS, r4);
    SetGpuReg(REG_OFFSET_BG3HOFS, r5);
    SetGpuReg(REG_OFFSET_BG3VOFS, r4);
}

void GetCameraOffsetWithPan(s16 *x, s16 *y)
{
    *x = sFieldCameraOffset.xPixelOffset + sHorizontalCameraPan;
    *y = sFieldCameraOffset.yPixelOffset + sVerticalCameraPan + 8;
}

void DrawWholeMapView(void)
{
    DrawWholeMapViewInternal(gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y, gMapHeader.mapLayout);
    sFieldCameraOffset.copyBGToVRAM = TRUE;
}

static void DrawWholeMapViewInternal(int x, int y, const struct MapLayout *mapLayout)
{
    u8 i;
    u8 j;
    u32 r6;
    u8 temp;

    for (i = 0; i < 32; i += 2)
    {
        temp = sFieldCameraOffset.yTileOffset + i;
        if (temp >= 32)
            temp -= 32;
        r6 = temp * 32;
        for (j = 0; j < 32; j += 2)
        {
            temp = sFieldCameraOffset.xTileOffset + j;
            if (temp >= 32)
                temp -= 32;
            DrawMetatileAt(mapLayout, r6 + temp, x + j / 2, y + i / 2);
        }
    }
}

static void RedrawMapSlicesForCameraUpdate(struct FieldCameraOffset *cameraOffset, int x, int y)
{
    const struct MapLayout *mapLayout = gMapHeader.mapLayout;

    if (x > 0)
        RedrawMapSliceWest(cameraOffset, mapLayout);
    if (x < 0)
        RedrawMapSliceEast(cameraOffset, mapLayout);
    if (y > 0)
        RedrawMapSliceNorth(cameraOffset, mapLayout);
    if (y < 0)
        RedrawMapSliceSouth(cameraOffset, mapLayout);
    cameraOffset->copyBGToVRAM = TRUE;
}

static void RedrawMapSliceNorth(struct FieldCameraOffset *cameraOffset, const struct MapLayout *mapLayout)
{
    u8 i;
    u8 temp;
    u32 r7;

    temp = cameraOffset->yTileOffset + 28;
    if (temp >= 32)
        temp -= 32;
    r7 = temp * 32;
    for (i = 0; i < 32; i += 2)
    {
        temp = cameraOffset->xTileOffset + i;
        if (temp >= 32)
            temp -= 32;
        DrawMetatileAt(mapLayout, r7 + temp, gSaveBlock1Ptr->pos.x + i / 2, gSaveBlock1Ptr->pos.y + 14);
    }
}

static void RedrawMapSliceSouth(struct FieldCameraOffset *cameraOffset, const struct MapLayout *mapLayout)
{
    u8 i;
    u8 temp;
    u32 r7 = cameraOffset->yTileOffset * 32;

    for (i = 0; i < 32; i += 2)
    {
        temp = cameraOffset->xTileOffset + i;
        if (temp >= 32)
            temp -= 32;
        DrawMetatileAt(mapLayout, r7 + temp, gSaveBlock1Ptr->pos.x + i / 2, gSaveBlock1Ptr->pos.y);
    }
}

static void RedrawMapSliceEast(struct FieldCameraOffset *cameraOffset, const struct MapLayout *mapLayout)
{
    u8 i;
    u8 temp;
    u32 r6 = cameraOffset->xTileOffset;

    for (i = 0; i < 32; i += 2)
    {
        temp = cameraOffset->yTileOffset + i;
        if (temp >= 32)
            temp -= 32;
        DrawMetatileAt(mapLayout, temp * 32 + r6, gSaveBlock1Ptr->pos.x, gSaveBlock1Ptr->pos.y + i / 2);
    }
}

static void RedrawMapSliceWest(struct FieldCameraOffset *cameraOffset, const struct MapLayout *mapLayout)
{
    u8 i;
    u8 temp;
    u8 r5 = cameraOffset->xTileOffset + 28;

    if (r5 >= 32)
        r5 -= 32;
    for (i = 0; i < 32; i += 2)
    {
        temp = cameraOffset->yTileOffset + i;
        if (temp >= 32)
            temp -= 32;
        DrawMetatileAt(mapLayout, temp * 32 + r5, gSaveBlock1Ptr->pos.x + 14, gSaveBlock1Ptr->pos.y + i / 2);
    }
}

void CurrentMapDrawMetatileAt(int x, int y)
{
    int offset = MapPosToBgTilemapOffset(&sFieldCameraOffset, x, y);

    if (offset >= 0)
    {
        DrawMetatileAt(gMapHeader.mapLayout, offset, x, y);
        sFieldCameraOffset.copyBGToVRAM = TRUE;
    }
}

void DrawDoorMetatileAt(int x, int y, u16 *tiles)
{
    int offset = MapPosToBgTilemapOffset(&sFieldCameraOffset, x, y);

    if (offset >= 0)
    {
        DrawMetatile(METATILE_LAYER_TYPE_COVERED, tiles, offset);
        sFieldCameraOffset.copyBGToVRAM = TRUE;
    }
}

// Draws a metatile like DrawMetatile does, except that each tile whose top layer has no sprite
// under it gets that top layer drawn on BG2. A normal metatile's bottom layer has to move down to
// BG3 for that, so only tiles with a bottom layer do: a gap in it would stop showing BG3's filler.
// Also keeps track of which tiles have a top layer that can go under the fog, so only sprites
// near them need checking.
static void DrawMetatileUnderFog(u32 layerType, const u16 *tiles, u16 offset)
{
    u32 i, pos, bottom, top, bit;
    u32 *row;

    for (i = 0; i < NUM_TILES_PER_METATILE / 2; i++)
    {
        pos = offset + (i & 1) + (i >> 1) * TILEMAP_WIDTH;
        row = &sTilesWithTopLayer[pos / TILEMAP_WIDTH];
        bit = 1u << (pos % TILEMAP_WIDTH);
        bottom = tiles[i];
        top = tiles[NUM_TILES_PER_METATILE / 2 + i];
        if (layerType != METATILE_LAYER_TYPE_COVERED
         && (top & (NUM_TILES_TOTAL - 1)) != 0
         && (layerType == METATILE_LAYER_TYPE_SPLIT || (bottom & (NUM_TILES_TOTAL - 1)) != 0))
            *row |= bit;
        else
            *row &= ~bit;

        if ((*row & bit) && !(sTilesUnderSprites[pos / TILEMAP_WIDTH] & bit))
        {
            gOverworldTilemapBuffer_Bg3[pos] = bottom;
            gOverworldTilemapBuffer_Bg2[pos] = top;
            gOverworldTilemapBuffer_Bg1[pos] = 0;
        }
        else if (layerType == METATILE_LAYER_TYPE_COVERED)
        {
            gOverworldTilemapBuffer_Bg3[pos] = bottom;
            gOverworldTilemapBuffer_Bg2[pos] = top;
            gOverworldTilemapBuffer_Bg1[pos] = 0;
        }
        else if (layerType == METATILE_LAYER_TYPE_SPLIT)
        {
            gOverworldTilemapBuffer_Bg3[pos] = bottom;
            gOverworldTilemapBuffer_Bg2[pos] = 0;
            gOverworldTilemapBuffer_Bg1[pos] = top;
        }
        else
        {
            gOverworldTilemapBuffer_Bg3[pos] = 0x3014;
            gOverworldTilemapBuffer_Bg2[pos] = bottom;
            gOverworldTilemapBuffer_Bg1[pos] = top;
        }
    }
    ScheduleBgCopyTilemapToVram(1);
    ScheduleBgCopyTilemapToVram(2);
    ScheduleBgCopyTilemapToVram(3);
}

static void DrawMetatileAt(const struct MapLayout *mapLayout, u16 offset, int x, int y)
{
    u16 tiles[NUM_TILES_PER_METATILE];
    u32 layerType = GetMetatileDrawTiles(MapGridGetMetatileIdAt(x, y), tiles);

    if (sTopLayerUnderFog)
        DrawMetatileUnderFog(layerType, tiles, offset);
    else
        DrawMetatile(layerType, tiles, offset);
}

static void RedrawMetatileInTilemap(u32 tilemapX, u32 tilemapY)
{
    // The tilemap wraps around, so while the camera moves right or down, the column or row
    // after its last one is the one scrolling out before its first.
    s32 x = ((tilemapX * 2 - sFieldCameraOffset.xTileOffset) & (TILEMAP_WIDTH - 1)) / 2;
    s32 y = ((tilemapY * 2 - sFieldCameraOffset.yTileOffset) & (TILEMAP_WIDTH - 1)) / 2;

    if (x == TILEMAP_WIDTH / 2 - 1 && gFieldCamera.x > 0)
        x = -1;
    if (y == TILEMAP_WIDTH / 2 - 1 && gFieldCamera.y > 0)
        y = -1;
    DrawMetatileAt(gMapHeader.mapLayout, tilemapY * 2 * TILEMAP_WIDTH + tilemapX * 2,
                   gSaveBlock1Ptr->pos.x + x, gSaveBlock1Ptr->pos.y + y);
}

// Gets the tilemap columns and rows under a box of screen pixels, or FALSE if it's off screen.
static bool32 GetTilesUnderBox(s32 left, s32 top, s32 right, s32 bottom, s32 scrollX, s32 scrollY, u32 *columns, s32 *firstRow, s32 *lastRow)
{
    s32 x;

    if (left < 0)
        left = 0;
    if (top < 0)
        top = 0;
    if (right >= DISPLAY_WIDTH)
        right = DISPLAY_WIDTH - 1;
    if (bottom >= DISPLAY_HEIGHT)
        bottom = DISPLAY_HEIGHT - 1;
    if (left > right || top > bottom)
        return FALSE;

    *columns = 0;
    for (x = (left + scrollX) >> 3; x <= (right + scrollX) >> 3; x++)
        *columns |= 1u << (x & (TILEMAP_WIDTH - 1));
    *firstRow = (top + scrollY) >> 3;
    *lastRow = (bottom + scrollY) >> 3;
    return TRUE;
}

static void MarkTilesUnderBox(u32 *tilesUnder, s32 left, s32 top, s32 right, s32 bottom, s32 scrollX, s32 scrollY)
{
    u32 columns;
    s32 y, firstRow, lastRow;

    if (GetTilesUnderBox(left, top, right, bottom, scrollX, scrollY, &columns, &firstRow, &lastRow))
    {
        for (y = firstRow; y <= lastRow; y++)
            tilesUnder[y & (TILEMAP_WIDTH - 1)] |= columns;
    }
}

// Marks the tilemap tiles under a sprite's opaque pixels, going by the box around them in each
// of its tiles. Its whole box would also take in what's around a small Pokemon in a big frame.
static void MarkTilesUnderSprite(u32 *tilesUnder, const struct OamData *oam, s32 scrollX, s32 scrollY)
{
    s32 width = sOamSizes[oam->shape][oam->size][0];
    s32 height = sOamSizes[oam->shape][oam->size][1];
    s32 x = oam->x;
    s32 y = oam->y;
    s32 tileX, tileY, left, top, right, bottom, screenX, screenY, temp;
    u32 i, row, columns, rows;
    const u16 *images, *image;

    if (oam->affineMode == ST_OAM_AFFINE_DOUBLE)
    {
        width *= 2;
        height *= 2;
    }
    if (x + width > 512)
        x -= 512;
    if (y + height > 256)
        y -= 256;
    // Unused OAM entries are parked off screen too.
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT || x + width <= 0 || y + height <= 0)
        return;

    if (oam->affineMode != ST_OAM_AFFINE_OFF || oam->bpp == ST_OAM_8BPP)
    {
        MarkTilesUnderBox(tilesUnder, x, y, x + width - 1, y + height - 1, scrollX, scrollY);
        return;
    }

    // Only tiles with a top layer that could go under the fog need to know.
    if (!GetTilesUnderBox(x, y, x + width - 1, y + height - 1, scrollX, scrollY, &columns, &top, &bottom))
        return;
    while (top <= bottom && !(sTilesWithTopLayer[top & (TILEMAP_WIDTH - 1)] & columns))
        top++;
    if (top > bottom)
        return;

    // Sprites map their tiles one-dimensionally in the overworld, and a frame's tiles are
    // copied to VRAM together.
    images = GetObjTileImageToShow(oam->tileNum);
    for (tileY = 0; tileY < height / 8; tileY++)
    {
        screenY = y + ((oam->matrixNum & ST_OAM_VFLIP) ? height / 8 - 1 - tileY : tileY) * 8;
        if (screenY >= DISPLAY_HEIGHT || screenY + 8 <= 0)
            continue;
        for (tileX = 0; tileX < width / 8; tileX++)
        {
            screenX = x + ((oam->matrixNum & ST_OAM_HFLIP) ? width / 8 - 1 - tileX : tileX) * 8;
            if (!GetTilesUnderBox(screenX, screenY, screenX + 7, screenY + 7, scrollX, scrollY, &columns, &top, &bottom))
                continue;
            while (top <= bottom && !(sTilesWithTopLayer[top & (TILEMAP_WIDTH - 1)] & columns))
                top++;
            if (top > bottom)
                continue;

            image = &images[(tileY * (width / 8) + tileX) * (TILE_SIZE_4BPP / 2)];
            columns = 0;
            rows = 0;
            for (i = 0; i < 8; i++)
            {
                row = image[i * 2] | (image[i * 2 + 1] << 16);
                if (row != 0)
                {
                    rows |= 1 << i;
                    columns |= row;
                }
            }
            if (rows == 0)
                continue;

            for (left = 0; !(columns & (0xFu << (left * 4))); left++)
                ;
            for (right = 7; !(columns & (0xFu << (right * 4))); right--)
                ;
            for (top = 0; !(rows & (1 << top)); top++)
                ;
            for (bottom = 7; !(rows & (1 << bottom)); bottom--)
                ;
            if (oam->matrixNum & ST_OAM_HFLIP)
            {
                temp = left;
                left = 7 - right;
                right = 7 - temp;
            }
            if (oam->matrixNum & ST_OAM_VFLIP)
            {
                temp = top;
                top = 7 - bottom;
                bottom = 7 - temp;
            }
            MarkTilesUnderBox(tilesUnder, screenX + left, screenY + top, screenX + right, screenY + bottom, scrollX, scrollY);
        }
    }
}

// Called each overworld frame once the OAM buffer is built, with the fog sprites' tiles in
// VRAM. Finds the tiles under the other sprites and redraws the metatiles where they changed.
void UpdateMapTopLayerUnderFog(bool32 isFogOut, u32 fogTileStart, u32 fogTileCount)
{
    u32 tilesUnder[TILEMAP_WIDTH] = {0};
    u16 metatilesChanged[TILEMAP_WIDTH / 2] = {0};
    s16 scrollX, scrollY;
    s32 x, y;
    u32 i, count, changed, kept, held;
    bool32 redrawAll = FALSE;
    const struct OamData *oam;

    if (!isFogOut)
    {
        if (sTopLayerUnderFog)
        {
            sTopLayerUnderFog = FALSE;
            DrawWholeMapView();
        }
        return;
    }

    GetCameraOffsetWithPan(&scrollX, &scrollY);
    if (!sTopLayerUnderFog)
    {
        // Which tiles have a top layer is found out as the map is drawn under the fog.
        for (y = 0; y < TILEMAP_WIDTH; y++)
            sTilesWithTopLayer[y] = 0xFFFFFFFF;
    }
    count = GetOamBufferSpriteCount();
    for (i = 0; i < count; i++)
    {
        oam = &gMain.oamBuffer[i];
        // Sprites with a higher priority are drawn over BG1 as well.
        if (oam->priority >= 2
         && oam->affineMode != ST_OAM_AFFINE_ERASE
         && (u32)(oam->tileNum - fogTileStart) >= fogTileCount)
            MarkTilesUnderSprite(tilesUnder, oam, scrollX, scrollY);
    }

    if (!sTopLayerUnderFog)
    {
        sTopLayerUnderFog = TRUE;
        redrawAll = TRUE;
        for (y = 0; y < TILEMAP_WIDTH; y++)
            sTilesUnderSprites[y] = 0;
    }

    for (y = 0; y < TILEMAP_WIDTH; y++)
    {
        kept = tilesUnder[y];
        held = sTilesUnderSprites[y] & ~tilesUnder[y];
        for (x = 0; held != 0; x++, held >>= 1)
        {
            if ((held & 1) && --sSpriteCoverTimers[y][x] != 0)
                kept |= 1u << x;
        }
        changed = tilesUnder[y];
        for (x = 0; changed != 0; x++, changed >>= 1)
        {
            if (changed & 1)
                sSpriteCoverTimers[y][x] = SPRITE_COVER_HOLD_FRAMES;
        }

        changed = kept ^ sTilesUnderSprites[y];
        sTilesUnderSprites[y] = kept;
        for (x = 0; changed != 0; x++, changed >>= 1)
        {
            if (changed & 1)
                metatilesChanged[y / 2] |= 1 << (x / 2);
        }
    }

    if (redrawAll)
    {
        DrawWholeMapView();
        return;
    }
    for (y = 0; y < TILEMAP_WIDTH / 2; y++)
    {
        changed = metatilesChanged[y];
        for (x = 0; changed != 0; x++, changed >>= 1)
        {
            if (changed & 1)
                RedrawMetatileInTilemap(x, y);
        }
    }
}

static void DrawMetatile(s32 metatileLayerType, const u16 *tiles, u16 offset)
{
    switch (metatileLayerType)
    {
    case METATILE_LAYER_TYPE_SPLIT:
        // Draw metatile's bottom layer to the bottom background layer.
        gOverworldTilemapBuffer_Bg3[offset] = tiles[0];
        gOverworldTilemapBuffer_Bg3[offset + 1] = tiles[1];
        gOverworldTilemapBuffer_Bg3[offset + 0x20] = tiles[2];
        gOverworldTilemapBuffer_Bg3[offset + 0x21] = tiles[3];

        // Draw transparent tiles to the middle background layer.
        gOverworldTilemapBuffer_Bg2[offset] = 0;
        gOverworldTilemapBuffer_Bg2[offset + 1] = 0;
        gOverworldTilemapBuffer_Bg2[offset + 0x20] = 0;
        gOverworldTilemapBuffer_Bg2[offset + 0x21] = 0;

        // Draw metatile's top layer to the top background layer.
        gOverworldTilemapBuffer_Bg1[offset] = tiles[4];
        gOverworldTilemapBuffer_Bg1[offset + 1] = tiles[5];
        gOverworldTilemapBuffer_Bg1[offset + 0x20] = tiles[6];
        gOverworldTilemapBuffer_Bg1[offset + 0x21] = tiles[7];
        break;
    case METATILE_LAYER_TYPE_COVERED:
        // Draw metatile's bottom layer to the bottom background layer.
        gOverworldTilemapBuffer_Bg3[offset] = tiles[0];
        gOverworldTilemapBuffer_Bg3[offset + 1] = tiles[1];
        gOverworldTilemapBuffer_Bg3[offset + 0x20] = tiles[2];
        gOverworldTilemapBuffer_Bg3[offset + 0x21] = tiles[3];

        // Draw metatile's top layer to the middle background layer.
        gOverworldTilemapBuffer_Bg2[offset] = tiles[4];
        gOverworldTilemapBuffer_Bg2[offset + 1] = tiles[5];
        gOverworldTilemapBuffer_Bg2[offset + 0x20] = tiles[6];
        gOverworldTilemapBuffer_Bg2[offset + 0x21] = tiles[7];

        // Draw transparent tiles to the top background layer.
        gOverworldTilemapBuffer_Bg1[offset] = 0;
        gOverworldTilemapBuffer_Bg1[offset + 1] = 0;
        gOverworldTilemapBuffer_Bg1[offset + 0x20] = 0;
        gOverworldTilemapBuffer_Bg1[offset + 0x21] = 0;
        break;
    case METATILE_LAYER_TYPE_NORMAL:
        // Draw garbage to the bottom background layer.
        gOverworldTilemapBuffer_Bg3[offset] = 0x3014;
        gOverworldTilemapBuffer_Bg3[offset + 1] = 0x3014;
        gOverworldTilemapBuffer_Bg3[offset + 0x20] = 0x3014;
        gOverworldTilemapBuffer_Bg3[offset + 0x21] = 0x3014;

        // Draw metatile's bottom layer to the middle background layer.
        gOverworldTilemapBuffer_Bg2[offset] = tiles[0];
        gOverworldTilemapBuffer_Bg2[offset + 1] = tiles[1];
        gOverworldTilemapBuffer_Bg2[offset + 0x20] = tiles[2];
        gOverworldTilemapBuffer_Bg2[offset + 0x21] = tiles[3];

        // Draw metatile's top layer to the top background layer, which covers object event sprites.
        gOverworldTilemapBuffer_Bg1[offset] = tiles[4];
        gOverworldTilemapBuffer_Bg1[offset + 1] = tiles[5];
        gOverworldTilemapBuffer_Bg1[offset + 0x20] = tiles[6];
        gOverworldTilemapBuffer_Bg1[offset + 0x21] = tiles[7];
        break;
    }
    ScheduleBgCopyTilemapToVram(1);
    ScheduleBgCopyTilemapToVram(2);
    ScheduleBgCopyTilemapToVram(3);
}

static s32 MapPosToBgTilemapOffset(struct FieldCameraOffset *cameraOffset, s32 x, s32 y)
{
    x -= gSaveBlock1Ptr->pos.x;
    x *= 2;
    if (x >= 32 || x < 0)
        return -1;
    x = x + cameraOffset->xTileOffset;
    if (x >= 32)
        x -= 32;

    y = (y - gSaveBlock1Ptr->pos.y) * 2;
    if (y >= 32 || y < 0)
        return -1;
    y = y + cameraOffset->yTileOffset;
    if (y >= 32)
        y -= 32;

    return y * 32 + x;
}

static void CameraUpdateCallback(struct CameraObject *fieldCamera)
{
    if (fieldCamera->spriteId != 0)
    {
        fieldCamera->movementSpeedX = gSprites[fieldCamera->spriteId].sCamera_MoveX;
        fieldCamera->movementSpeedY = gSprites[fieldCamera->spriteId].sCamera_MoveY;
    }
}

void ResetCameraUpdateInfo(void)
{
    gFieldCamera.movementSpeedX = 0;
    gFieldCamera.movementSpeedY = 0;
    gFieldCamera.x = 0;
    gFieldCamera.y = 0;
    gFieldCamera.spriteId = 0;
    gFieldCamera.callback = NULL;
}

u32 InitCameraUpdateCallback(u8 trackedSpriteId)
{
    if (gFieldCamera.spriteId != 0)
        DestroySprite(&gSprites[gFieldCamera.spriteId]);
    gFieldCamera.spriteId = AddCameraObject(trackedSpriteId);
    gFieldCamera.callback = CameraUpdateCallback;
    return 0;
}

void CameraUpdate(void)
{
    int deltaX;
    int deltaY;
    int curMovementOffsetY;
    int curMovementOffsetX;
    int movementSpeedX;
    int movementSpeedY;

    if (gFieldCamera.callback != NULL)
        gFieldCamera.callback(&gFieldCamera);
    movementSpeedX = gFieldCamera.movementSpeedX;
    movementSpeedY = gFieldCamera.movementSpeedY;
    deltaX = 0;
    deltaY = 0;
    curMovementOffsetX = gFieldCamera.x;
    curMovementOffsetY = gFieldCamera.y;


    if (curMovementOffsetX == 0 && movementSpeedX != 0)
    {
        if (movementSpeedX > 0)
            deltaX = 1;
        else
            deltaX = -1;
    }
    if (curMovementOffsetY == 0 && movementSpeedY != 0)
    {
        if (movementSpeedY > 0)
            deltaY = 1;
        else
            deltaY = -1;
    }
    if (curMovementOffsetX != 0 && curMovementOffsetX == -movementSpeedX)
    {
        if (movementSpeedX > 0)
            deltaX = 1;
        else
            deltaX = -1;
    }
    if (curMovementOffsetY != 0 && curMovementOffsetY == -movementSpeedY)
    {
        if (movementSpeedY > 0)
            deltaX = 1;
        else
            deltaX = -1;
    }

    gFieldCamera.x += movementSpeedX;
    gFieldCamera.x %= 16;
    gFieldCamera.y += movementSpeedY;
    gFieldCamera.y %= 16;

    if (deltaX != 0 || deltaY != 0)
    {
        CameraMove(deltaX, deltaY);
        UpdateObjectEventsForCameraUpdate(deltaX, deltaY);
        RotatingGatePuzzleCameraUpdate(deltaX, deltaY);
        SetBerryTreesSeen();
        AddCameraTileOffset(&sFieldCameraOffset, deltaX * 2, deltaY * 2);
        RedrawMapSlicesForCameraUpdate(&sFieldCameraOffset, deltaX * 2, deltaY * 2);
    }

    AddCameraPixelOffset(&sFieldCameraOffset, movementSpeedX, movementSpeedY);
    gTotalCameraPixelOffsetX -= movementSpeedX;
    gTotalCameraPixelOffsetY -= movementSpeedY;
}

void MoveCameraAndRedrawMap(int deltaX, int deltaY) //unused
{
    CameraMove(deltaX, deltaY);
    UpdateObjectEventsForCameraUpdate(deltaX, deltaY);
    DrawWholeMapView();
    gTotalCameraPixelOffsetX -= deltaX * 16;
    gTotalCameraPixelOffsetY -= deltaY * 16;
}

void SetCameraPanningCallback(void (*callback)(void))
{
    sFieldCameraPanningCallback = callback;
}

void SetCameraPanning(s16 horizontal, s16 vertical)
{
    sHorizontalCameraPan = horizontal;
    sVerticalCameraPan = vertical + 32;
}

void InstallCameraPanAheadCallback(void)
{
    sFieldCameraPanningCallback = CameraPanningCB_PanAhead;
    sBikeCameraPanFlag = FALSE;
    sHorizontalCameraPan = 0;
    sVerticalCameraPan = 32;
}

void UpdateCameraPanning(void)
{
    if (sFieldCameraPanningCallback != NULL)
        sFieldCameraPanningCallback();
    //Update sprite offset of overworld objects
    gSpriteCoordOffsetX = gTotalCameraPixelOffsetX - sHorizontalCameraPan;
    gSpriteCoordOffsetY = gTotalCameraPixelOffsetY - sVerticalCameraPan - 8;
}

static void CameraPanningCB_PanAhead(void)
{
    u8 var;

    if (gUnusedBikeCameraAheadPanback == FALSE)
    {
        InstallCameraPanAheadCallback();
    }
    else
    {
        // this code is never reached
        if (gPlayerAvatar.tileTransitionState == T_TILE_TRANSITION)
        {
            sBikeCameraPanFlag ^= 1;
            if (sBikeCameraPanFlag == FALSE)
                return;
        }
        else
        {
            sBikeCameraPanFlag = FALSE;
        }

        var = GetPlayerMovementDirection();
        if (var == 2)
        {
            if (sVerticalCameraPan > -8)
                sVerticalCameraPan -= 2;
        }
        else if (var == 1)
        {
            if (sVerticalCameraPan < 72)
                sVerticalCameraPan += 2;
        }
        else if (sVerticalCameraPan < 32)
        {
            sVerticalCameraPan += 2;
        }
        else if (sVerticalCameraPan > 32)
        {
            sVerticalCameraPan -= 2;
        }
    }
}

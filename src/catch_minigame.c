#include "global.h"
#include "battle.h"
#include "sprite.h"
#include "palette.h"
#include "decompress.h"
#include "random.h"
#include "main.h"
#include "constants/battle.h"

// D-pad icon sprites positioned below opponent's HP bar
#define DPAD_SPRITE_Y 55
#define DPAD_SPRITE_X 24  // Fixed position for all icons (leftmost)

#define TAG_DPAD_ICONS 0xD0AD

// Direction indices (not to be confused with DPAD_UP/DOWN/etc button masks)
#define DIR_UP    0
#define DIR_DOWN  1
#define DIR_LEFT  2
#define DIR_RIGHT 3

#define MAX_SEQUENCE 3
#define MINIGAME_FRAMES 90  // 1 second for quick input before animation

// Forward declarations
static void LoadDpadTiles(void);
static u8 CreateDpadSprite(u8 direction, u8 index);
void CatchMinigame_HideIcons(void);

// The keypad icons tileset
static const u8 sKeypadIconTiles[] = INCBIN_U8("graphics/fonts/keypad_icons.4bpp");

// Pre-arranged tile data for 8x16 sprites
static EWRAM_DATA u8 sDpadSpriteTiles[4 * 2 * 0x20] ALIGNED(4) = {0};
static EWRAM_DATA u8 sDpadSpriteIds[MAX_SEQUENCE] = {0};
static EWRAM_DATA u8 sCurrentSequence[MAX_SEQUENCE] = {0};
static EWRAM_DATA u8 sSequenceIndex = 0;
static EWRAM_DATA u16 sFrameCounter = 0;
static EWRAM_DATA bool8 sDpadIconsVisible = FALSE;
static EWRAM_DATA bool8 sTilesLoaded = FALSE;
static EWRAM_DATA bool8 sMinigameWon = FALSE;
static EWRAM_DATA bool8 sMinigameFailed = FALSE;
static EWRAM_DATA bool8 sMinigameStarted = FALSE;  // Tracks if minigame was started this catch attempt

static const struct OamData sDpadOamData = {
    .y = 0,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x16),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
};

static void SpriteCB_DpadIcon(struct Sprite *sprite)
{
    // Static sprite
}

static const struct SpriteTemplate sDpadSpriteTemplate = {
    .tileTag = TAG_DPAD_ICONS,
    .paletteTag = TAG_DPAD_ICONS,
    .oam = &sDpadOamData,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_DpadIcon,
};

static const u16 sDpadPalette[] = INCBIN_U16("graphics/fonts/keypad_icons.gbapal");

static void LoadDpadTiles(void)
{
    u8 i;
    struct SpriteSheet sheet;
    struct SpritePalette palSheet;
    
    if (sTilesLoaded)
        return;
    
    // Arrange tiles for 8x16 sprites
    for (i = 0; i < 4; i++)
    {
        memcpy(&sDpadSpriteTiles[(i * 2) * 0x20], 
               &sKeypadIconTiles[(0x0C + i) * 0x20], 
               0x20);
        memcpy(&sDpadSpriteTiles[(i * 2 + 1) * 0x20], 
               &sKeypadIconTiles[(0x1C + i) * 0x20], 
               0x20);
    }
    
    sheet.data = sDpadSpriteTiles;
    sheet.size = sizeof(sDpadSpriteTiles);
    sheet.tag = TAG_DPAD_ICONS;
    
    palSheet.data = sDpadPalette;
    palSheet.tag = TAG_DPAD_ICONS;
    
    LoadSpriteSheet(&sheet);
    LoadSpritePalette(&palSheet);
    sTilesLoaded = TRUE;
}

static u8 CreateDpadSprite(u8 direction, u8 index)
{
    // All sprites appear at the same position (stacked on top of each other)
    // Use subpriority so newer sprites appear on top (lower subpriority = on top)
    u8 spriteId = CreateSprite(&sDpadSpriteTemplate, DPAD_SPRITE_X, DPAD_SPRITE_Y, 10 - index);
    if (spriteId != MAX_SPRITES)
    {
        // Each direction uses 2 tiles
        gSprites[spriteId].oam.tileNum += (direction * 2);
    }
    return spriteId;
}

// Initialize the minigame - generate sequence and show first icon
void CatchMinigame_DrawTestIcons(void)
{
    u8 i;
    
    if (sDpadIconsVisible || sMinigameStarted)
        return;
    
    LoadDpadTiles();
    
    // Reset states
    sMinigameWon = FALSE;
    sMinigameFailed = FALSE;
    sMinigameStarted = TRUE;
    sFrameCounter = MINIGAME_FRAMES;
    
    // Initialize sprite IDs
    for (i = 0; i < MAX_SEQUENCE; i++)
        sDpadSpriteIds[i] = MAX_SPRITES;
    
    // Generate random sequence with no consecutive repeats
    sCurrentSequence[0] = Random() % 4;  // 4 directions: up/down/left/right
    for (i = 1; i < MAX_SEQUENCE; i++)
    {
        do {
            sCurrentSequence[i] = Random() % 4;
        } while (sCurrentSequence[i] == sCurrentSequence[i - 1]);
    }
    
    sSequenceIndex = 0;
    
    // Create first sprite
    sDpadSpriteIds[0] = CreateDpadSprite(sCurrentSequence[0], 0);
    
    sDpadIconsVisible = TRUE;
}

// Call this every frame to check for input
void CatchMinigame_Update(void)
{
    u8 currentDirection;
    bool8 correctPress = FALSE;
    bool8 wrongPress = FALSE;
    
    if (!sDpadIconsVisible || sSequenceIndex >= MAX_SEQUENCE)
        return;
    
    // Tick down the timer
    if (sFrameCounter > 0)
        sFrameCounter--;
    
    // Time's up - end minigame without winning
    if (sFrameCounter == 0)
    {
        sMinigameFailed = TRUE;
        CatchMinigame_HideIcons();
        return;
    }
    
    currentDirection = sCurrentSequence[sSequenceIndex];
    
    // Check for any D-pad press
    if (JOY_NEW(DPAD_ANY))
    {
        switch (currentDirection)
        {
            case DIR_UP:
                correctPress = JOY_NEW(DPAD_UP);
                break;
            case DIR_DOWN:
                correctPress = JOY_NEW(DPAD_DOWN);
                break;
            case DIR_LEFT:
                correctPress = JOY_NEW(DPAD_LEFT);
                break;
            case DIR_RIGHT:
                correctPress = JOY_NEW(DPAD_RIGHT);
                break;
        }
        
        if (!correctPress)
            wrongPress = TRUE;
    }
    
    if (correctPress)
    {
        sSequenceIndex++;
        
        if (sSequenceIndex >= MAX_SEQUENCE)
        {
            // Completed successfully!
            sMinigameWon = TRUE;
            CatchMinigame_HideIcons();
        }
        else
        {
            // Show next icon on top
            sDpadSpriteIds[sSequenceIndex] = CreateDpadSprite(sCurrentSequence[sSequenceIndex], sSequenceIndex);
        }
    }
    else if (wrongPress)
    {
        // Wrong button - fail the minigame
        sMinigameFailed = TRUE;
        CatchMinigame_HideIcons();
    }
}

bool8 CatchMinigame_AreIconsVisible(void)
{
    return sDpadIconsVisible;
}

bool8 CatchMinigame_IsTimeUp(void)
{
    return sFrameCounter == 0;
}

bool8 CatchMinigame_WasWon(void)
{
    return sMinigameWon;
}

bool8 CatchMinigame_WasFailed(void)
{
    return sMinigameFailed;
}

// Returns bonus multiplier (in 10ths) based on correct presses: 1+2+2 = 5 total
u8 CatchMinigame_GetBonus(void)
{
    // sSequenceIndex tracks how many correct presses were made
    // Distribution: 1, 2, 2 for presses 1, 2, 3
    switch (sSequenceIndex)
    {
        case 0: return 0;
        case 1: return 1;  // 0.1x
        case 2: return 3;  // 0.3x
        case 3: return 5;  // 0.5x (full completion)
        default: return 5;
    }
}

void CatchMinigame_ResetWinState(void)
{
    sMinigameWon = FALSE;
    sMinigameFailed = FALSE;
    sMinigameStarted = FALSE;
    // Also reset tiles loaded flag to ensure clean state after soft reset
    sTilesLoaded = FALSE;
}

void CatchMinigame_HideIcons(void)
{
    u8 i;
    
    if (!sDpadIconsVisible)
        return;
    
    for (i = 0; i < MAX_SEQUENCE; i++)
    {
        if (sDpadSpriteIds[i] != MAX_SPRITES)
        {
            DestroySprite(&gSprites[sDpadSpriteIds[i]]);
            sDpadSpriteIds[i] = MAX_SPRITES;
        }
    }
    
    FreeSpriteTilesByTag(TAG_DPAD_ICONS);
    FreeSpritePaletteByTag(TAG_DPAD_ICONS);
    
    sDpadIconsVisible = FALSE;
    sTilesLoaded = FALSE;
    sSequenceIndex = 0;
}

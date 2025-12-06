#include "global.h"
#include "battle.h"
#include "bg.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "menu.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"

// Timing constants
#define POPUP_DISPLAY_TIME 120

// Window position - top right of screen, no frame
#define POPUP_WINDOW_LEFT   17
#define POPUP_WINDOW_TOP    0
#define POPUP_WINDOW_WIDTH  13
#define POPUP_WINDOW_HEIGHT 4

// Icon sprite position - to the left of text
#define POPUP_ICON_X        136
#define POPUP_ICON_Y        16

// Popup states
enum {
    STATE_WAIT_CONTROLS,
    STATE_INIT,
    STATE_SHOW,
    STATE_WAIT,
    STATE_HIDE,
    STATE_NEXT,
    STATE_END,
};

#define tState         data[0]
#define tDisplayTimer  data[1]
#define tCurrentSlot   data[2]
#define tPokemonFlags  data[3]
#define tIconSpriteId  data[4]

static EWRAM_DATA u8 sPopupWindowId = WINDOW_NONE;

static void Task_NewMovesPopup(u8 taskId);
static void ShowNewMovesPopupWindow(u8 taskId, u8 partySlot);
static void HideNewMovesPopupWindow(u8 taskId);

// Subtitle text
static const u8 sText_NewMoves[] = _("New Moves!");

// Window template for popup - transparent, no frame
static const struct WindowTemplate sNewMovesPopupWindowTemplate = {
    .bg = 0,
    .tilemapLeft = POPUP_WINDOW_LEFT,
    .tilemapTop = POPUP_WINDOW_TOP,
    .width = POPUP_WINDOW_WIDTH,
    .height = POPUP_WINDOW_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 0x01,
};

// Check if any Pokemon have new moves to learn and start the popup task
void CheckAndShowNewMovesPopup(void)
{
    if (gBattleResults.pokemonWithNewMoves != 0)
    {
        if (!FuncIsActiveTask(Task_NewMovesPopup))
        {
            u8 taskId = CreateTask(Task_NewMovesPopup, 80);
            gTasks[taskId].tState = STATE_WAIT_CONTROLS;
            gTasks[taskId].tDisplayTimer = 0;
            gTasks[taskId].tCurrentSlot = 0;
            gTasks[taskId].tPokemonFlags = gBattleResults.pokemonWithNewMoves;
            gTasks[taskId].tIconSpriteId = SPRITE_NONE;
            
            // Clear the flag so we don't show again
            gBattleResults.pokemonWithNewMoves = 0;
        }
    }
}

static void Task_NewMovesPopup(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case STATE_WAIT_CONTROLS:
        // Wait for player controls to be unlocked
        if (!ArePlayerFieldControlsLocked())
        {
            task->tState = STATE_INIT;
        }
        break;
        
    case STATE_INIT:
        // Find next party member with new moves
        while (task->tCurrentSlot < PARTY_SIZE)
        {
            if (task->tPokemonFlags & (1 << task->tCurrentSlot))
            {
                task->tState = STATE_SHOW;
                return;
            }
            task->tCurrentSlot++;
        }
        // No more Pokemon with new moves
        task->tState = STATE_END;
        break;
        
    case STATE_SHOW:
        ShowNewMovesPopupWindow(taskId, task->tCurrentSlot);
        task->tDisplayTimer = 0;
        task->tState = STATE_WAIT;
        break;
        
    case STATE_WAIT:
        task->tDisplayTimer++;
        if (task->tDisplayTimer > POPUP_DISPLAY_TIME)
        {
            task->tState = STATE_HIDE;
        }
        break;
        
    case STATE_HIDE:
        HideNewMovesPopupWindow(taskId);
        task->tState = STATE_NEXT;
        break;
        
    case STATE_NEXT:
        task->tCurrentSlot++;
        task->tState = STATE_INIT;
        break;
        
    case STATE_END:
        DestroyTask(taskId);
        break;
    }
}

static void ShowNewMovesPopupWindow(u8 taskId, u8 partySlot)
{
    u16 species;
    u32 personality;
    u8 x;
    u8 *nickname = gStringVar1;
    
    // Get Pokemon data
    species = GetMonData(&gPlayerParty[partySlot], MON_DATA_SPECIES);
    personality = GetMonData(&gPlayerParty[partySlot], MON_DATA_PERSONALITY);
    GetMonData(&gPlayerParty[partySlot], MON_DATA_NICKNAME, nickname);
    StringGet_Nickname(nickname);
    
    // Create window
    if (sPopupWindowId == WINDOW_NONE)
        sPopupWindowId = AddWindow(&sNewMovesPopupWindowTemplate);
    
    // Fill with transparent background (color 0)
    FillWindowPixelBuffer(sPopupWindowId, PIXEL_FILL(0));
    
    // Print Pokemon nickname - white text with dark shadow
    x = GetStringCenterAlignXOffset(FONT_NARROW, nickname, POPUP_WINDOW_WIDTH * 8);
    AddTextPrinterParameterized4(sPopupWindowId, FONT_NARROW, x, 2, 0, 0,
        (u8[]){TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY}, 
        TEXT_SKIP_DRAW, nickname);
    
    // Print "New Moves!" below - blue text
    x = GetStringCenterAlignXOffset(FONT_SMALL, sText_NewMoves, POPUP_WINDOW_WIDTH * 8);
    AddTextPrinterParameterized4(sPopupWindowId, FONT_SMALL, x, 14, 0, 0,
        (u8[]){TEXT_COLOR_TRANSPARENT, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE}, 
        TEXT_SKIP_DRAW, sText_NewMoves);
    
    // Put window on screen
    PutWindowTilemap(sPopupWindowId);
    CopyWindowToVram(sPopupWindowId, COPYWIN_FULL);
    
    // Scroll BG0 to bring window from bottom to top of screen
    SetGpuReg(REG_OFFSET_BG0VOFS, 256 - 134);  // Shift text down 2 more pixels
    
    // Create Pokemon icon sprite with high priority (0 = highest, appears above BGs)
    LoadMonIconPalette(species);
    gTasks[taskId].tIconSpriteId = CreateMonIcon(species, SpriteCB_MonIcon, POPUP_ICON_X, POPUP_ICON_Y, 0, personality, TRUE);
    
    // Set sprite subpriority to appear on top
    gSprites[gTasks[taskId].tIconSpriteId].subpriority = 0;
    
    // Play sound effect
    PlaySE(SE_EXP_MAX);
}

static void HideNewMovesPopupWindow(u8 taskId)
{
    // Destroy the icon sprite
    if (gTasks[taskId].tIconSpriteId != SPRITE_NONE)
    {
        u16 species = GetMonData(&gPlayerParty[gTasks[taskId].tCurrentSlot], MON_DATA_SPECIES);
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].tIconSpriteId]);
        FreeMonIconPalette(species);
        gTasks[taskId].tIconSpriteId = SPRITE_NONE;
    }
    
    // Remove the window
    if (sPopupWindowId != WINDOW_NONE)
    {
        ClearWindowTilemap(sPopupWindowId);
        CopyWindowToVram(sPopupWindowId, COPYWIN_MAP);
        RemoveWindow(sPopupWindowId);
        sPopupWindowId = WINDOW_NONE;
        
        // Reset BG0 scroll
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    }
}

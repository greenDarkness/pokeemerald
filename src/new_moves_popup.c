#include "global.h"
#include "battle.h"
#include "bg.h"
#include "field_message_box.h"
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
#define POPUP_SLIDE_SPEED  4

// Slide animation - offscreen and onscreen Y offsets
#define POPUP_SCROLL_ONSCREEN   (256 - 134)  // Fully visible
#define POPUP_SCROLL_OFFSCREEN  (256 - 90)   // Hidden above screen
#define POPUP_SPRITE_ONSCREEN_Y 16
#define POPUP_SPRITE_OFFSCREEN_Y -20

// Window position - positioned to appear at top of screen
#define POPUP_WINDOW_LEFT   17
#define POPUP_WINDOW_TOP    0
#define POPUP_WINDOW_WIDTH  13
#define POPUP_WINDOW_HEIGHT 4

// Text X offset within window (to align with sprite)
#define TEXT_X_OFFSET       92

// Icon sprite position - to the left of text
#define POPUP_ICON_X        96
#define POPUP_ICON_Y        16

// Popup states
enum {
    STATE_WAIT_CONTROLS,
    STATE_INIT,
    STATE_CREATE,
    STATE_SLIDE_IN,
    STATE_WAIT,
    STATE_SLIDE_OUT,
    STATE_CLEANUP,
    STATE_NEXT,
    STATE_END,
};

#define tState         data[0]
#define tDisplayTimer  data[1]
#define tCurrentSlot   data[2]
#define tPokemonFlags  data[3]
#define tIconSpriteId  data[4]
#define tSlideOffset   data[5]

static EWRAM_DATA u8 sPopupWindowId = WINDOW_NONE;
static EWRAM_DATA u8 sPopupTaskId = TASK_NONE;

static void Task_NewMovesPopup(u8 taskId);
static void ShowNewMovesPopupWindow(u8 taskId, u8 partySlot);
static void HideNewMovesPopupWindow(u8 taskId);

// Subtitle text
static const u8 sText_NewMoves[] = _("New Moves!");

// Window template for popup
static const struct WindowTemplate sNewMovesPopupWindowTemplate = {
    .bg = 0,
    .tilemapLeft = POPUP_WINDOW_LEFT,
    .tilemapTop = POPUP_WINDOW_TOP,
    .width = POPUP_WINDOW_WIDTH,
    .height = POPUP_WINDOW_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 0x280,
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
            sPopupTaskId = taskId;
            
            // Clear the flag so we don't show again
            gBattleResults.pokemonWithNewMoves = 0;
        }
    }
}

// External function to hide popup when player input is detected
void HideNewMovesPopup(void)
{
    // Always reset BG0VOFS to ensure message boxes display correctly
    SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    
    if (sPopupTaskId != TASK_NONE && FuncIsActiveTask(Task_NewMovesPopup))
    {
        HideNewMovesPopupWindow(sPopupTaskId);
        gTasks[sPopupTaskId].tState = STATE_NEXT;
    }
}

static void Task_NewMovesPopup(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case STATE_WAIT_CONTROLS:
        // Wait for player controls to be unlocked, no script running, and field message box hidden
        if (!ArePlayerFieldControlsLocked() && !ScriptContext_IsEnabled() && IsFieldMessageBoxHidden())
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
                task->tState = STATE_CREATE;
                return;
            }
            task->tCurrentSlot++;
        }
        // No more Pokemon with new moves
        task->tState = STATE_END;
        break;
        
    case STATE_CREATE:
        // Check again before creating - if controls got locked or script started, wait
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden())
        {
            task->tState = STATE_WAIT_CONTROLS;
            break;
        }
        ShowNewMovesPopupWindow(taskId, task->tCurrentSlot);
        task->tSlideOffset = 0;
        task->tState = STATE_SLIDE_IN;
        break;
        
    case STATE_SLIDE_IN:
        // If player opens a menu, script starts, or message box appears, immediately hide
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden())
        {
            SetGpuReg(REG_OFFSET_BG0VOFS, 0);
            HideNewMovesPopupWindow(taskId);
            task->tState = STATE_NEXT;
            break;
        }
        // Slide in from top
        task->tSlideOffset += POPUP_SLIDE_SPEED;
        if (task->tSlideOffset >= 44)
        {
            task->tSlideOffset = 44;
            task->tDisplayTimer = 0;
            task->tState = STATE_WAIT;
        }
        SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN - task->tSlideOffset);
        if (task->tIconSpriteId != SPRITE_NONE)
            gSprites[task->tIconSpriteId].y = POPUP_SPRITE_OFFSCREEN_Y + task->tSlideOffset;
        break;
        
    case STATE_WAIT:
        // If player opens a menu, script starts, or message box appears, immediately hide
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden())
        {
            SetGpuReg(REG_OFFSET_BG0VOFS, 0);
            HideNewMovesPopupWindow(taskId);
            task->tState = STATE_NEXT;
            break;
        }
        task->tDisplayTimer++;
        if (task->tDisplayTimer > POPUP_DISPLAY_TIME)
        {
            task->tState = STATE_SLIDE_OUT;
        }
        break;
        
    case STATE_SLIDE_OUT:
        // If player opens a menu, script starts, or message box appears, immediately hide
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden())
        {
            SetGpuReg(REG_OFFSET_BG0VOFS, 0);
            HideNewMovesPopupWindow(taskId);
            task->tState = STATE_NEXT;
            break;
        }
        // Slide out to top
        task->tSlideOffset -= POPUP_SLIDE_SPEED;
        if (task->tSlideOffset <= 0)
        {
            task->tSlideOffset = 0;
            task->tState = STATE_CLEANUP;
        }
        SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN - task->tSlideOffset);
        if (task->tIconSpriteId != SPRITE_NONE)
            gSprites[task->tIconSpriteId].y = POPUP_SPRITE_OFFSCREEN_Y + task->tSlideOffset;
        break;
        
    case STATE_CLEANUP:
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        HideNewMovesPopupWindow(taskId);
        task->tState = STATE_NEXT;
        break;
        
    case STATE_NEXT:
        task->tCurrentSlot++;
        task->tState = STATE_INIT;
        break;
        
    case STATE_END:
        sPopupTaskId = TASK_NONE;
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
    
    // Create window (only if not already created)
    if (sPopupWindowId == WINDOW_NONE)
        sPopupWindowId = AddWindow(&sNewMovesPopupWindowTemplate);
    
    // Fill with transparent background (color 0)
    FillWindowPixelBuffer(sPopupWindowId, PIXEL_FILL(0));
    
    // Print Pokemon nickname - white text with dark shadow
    // Add TEXT_X_OFFSET to shift text right within window
    x = GetStringCenterAlignXOffset(FONT_NARROW, nickname, POPUP_WINDOW_WIDTH * 8 - TEXT_X_OFFSET);
    AddTextPrinterParameterized4(sPopupWindowId, FONT_NARROW, x + TEXT_X_OFFSET, 2, 0, 0,
        (u8[]){TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY}, 
        TEXT_SKIP_DRAW, nickname);
    
    // Print "New Moves!" below - blue text
    x = GetStringCenterAlignXOffset(FONT_SMALL, sText_NewMoves, POPUP_WINDOW_WIDTH * 8 - TEXT_X_OFFSET);
    AddTextPrinterParameterized4(sPopupWindowId, FONT_SMALL, x + TEXT_X_OFFSET, 14, 0, 0,
        (u8[]){TEXT_COLOR_TRANSPARENT, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE}, 
        TEXT_SKIP_DRAW, sText_NewMoves);
    
    // Put window on screen
    PutWindowTilemap(sPopupWindowId);
    CopyWindowToVram(sPopupWindowId, COPYWIN_FULL);
    
    // Start with BG0 scrolled to offscreen position
    SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
    
    // Create Pokemon icon sprite (starts offscreen)
    LoadMonIconPalette(species);
    gTasks[taskId].tIconSpriteId = CreateMonIcon(species, SpriteCB_MonIcon, POPUP_ICON_X, POPUP_SPRITE_OFFSCREEN_Y, 0, personality, TRUE);
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
    
    // Clear and remove window
    // Note: We don't call RemoveWindow because it can free gWindowBgTilemapBuffers[0]
    // when no other windows are active on BG0, which corrupts subsequent message boxes.
    // Instead, just clear the tilemap and leave the window allocated (small memory cost).
    if (sPopupWindowId != WINDOW_NONE)
    {
        ClearWindowTilemap(sPopupWindowId);
        CopyWindowToVram(sPopupWindowId, COPYWIN_MAP);
    }
}

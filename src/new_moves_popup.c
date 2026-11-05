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
#include "map_name_popup.h"
#include "pickup_item_popup.h"
#include "chain_reroll_popup.h"
#include "constants/songs.h"

// Timing constants
#define POPUP_DISPLAY_TIME 120
#define POPUP_SLIDE_SPEED  4

// Slide animation - offscreen and onscreen Y offsets
#define POPUP_SCROLL_ONSCREEN   (256 - 134)  // Fully visible
#define POPUP_SCROLL_OFFSCREEN  (256 - 90)   // Hidden above screen
#define POPUP_SPRITE_ONSCREEN_Y 16
#define POPUP_SPRITE_OFFSCREEN_Y -20

// Window position - placed on the message box rows; the slide offsets scroll BG0 so these
// rows appear at the top of the screen
#define POPUP_WINDOW_LEFT   2
#define POPUP_WINDOW_TOP    15
#define POPUP_WINDOW_WIDTH  27
#define POPUP_WINDOW_HEIGHT 4

// Text X offset within window (to align with sprite)
#define TEXT_X_OFFSET       92

// Icon sprite position - to the left of text
#define POPUP_ICON_X        96
#define POPUP_ICON_Y        16

// Pause between consecutive popups (in frames)
#define POPUP_PAUSE_TIME 30

// Popup states
enum {
    STATE_WAIT_CONTROLS,
    STATE_INIT,
    STATE_CREATE,
    STATE_SLIDE_IN,
    STATE_WAIT,
    STATE_SLIDE_OUT,
    STATE_CLEANUP,
    STATE_RESET_SCROLL,
    STATE_PAUSE,
    STATE_NEXT,
    STATE_END,
};

#define tState         data[0]
#define tDisplayTimer  data[1]
#define tCurrentSlot   data[2]
#define tIconSpriteId  data[4]
#define tSlideOffset   data[5]

static EWRAM_DATA u8 sPopupWindowId = WINDOW_NONE;
static EWRAM_DATA u8 sPopupTaskId = TASK_NONE;

// Pokemon still owed a popup. Kept outside the task so the queue survives the task being
// wiped by ResetTasks (battles, menus), and stored by personality so reordering the party
// before the popup shows can't attach it to the wrong Pokemon.
static EWRAM_DATA u32 sQueuedPersonalities[PARTY_SIZE] = {0};
static EWRAM_DATA u8 sQueuedCount = 0;

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

static void QueueMon(u8 partySlot)
{
    u32 personality = GetMonData(&gPlayerParty[partySlot], MON_DATA_PERSONALITY);
    u8 i;

    if (GetMonData(&gPlayerParty[partySlot], MON_DATA_SPECIES) == SPECIES_NONE)
        return;
    for (i = 0; i < sQueuedCount; i++)
    {
        if (sQueuedPersonalities[i] == personality)
            return;
    }
    if (sQueuedCount < PARTY_SIZE)
        sQueuedPersonalities[sQueuedCount++] = personality;
}

static void DequeueFront(void)
{
    u8 i;

    if (sQueuedCount == 0)
        return;
    for (i = 1; i < sQueuedCount; i++)
        sQueuedPersonalities[i - 1] = sQueuedPersonalities[i];
    sQueuedCount--;
}

// Returns the party slot of the next queued Pokemon, dropping any no longer in the party
static s8 GetNextQueuedPartySlot(void)
{
    while (sQueuedCount != 0)
    {
        u8 slot;

        for (slot = 0; slot < PARTY_SIZE; slot++)
        {
            if (GetMonData(&gPlayerParty[slot], MON_DATA_SPECIES) != SPECIES_NONE
             && GetMonData(&gPlayerParty[slot], MON_DATA_PERSONALITY) == sQueuedPersonalities[0])
                return slot;
        }
        DequeueFront();
    }
    return -1;
}

static bool8 IsPopupTaskAlive(void)
{
    return sPopupTaskId != TASK_NONE
        && gTasks[sPopupTaskId].isActive
        && gTasks[sPopupTaskId].func == Task_NewMovesPopup;
}

// Check if any Pokemon have new moves to learn and start the popup task
void CheckAndShowNewMovesPopup(void)
{
    u8 slot;

    for (slot = 0; slot < PARTY_SIZE; slot++)
    {
        if (gBattleResults.pokemonWithNewMoves & (1 << slot))
            QueueMon(slot);
    }
    gBattleResults.pokemonWithNewMoves = 0;

    if (sQueuedCount != 0 && !IsPopupTaskAlive())
    {
        u8 taskId = CreateTask(Task_NewMovesPopup, 80);
        gTasks[taskId].tState = STATE_WAIT_CONTROLS;
        gTasks[taskId].tDisplayTimer = 0;
        gTasks[taskId].tCurrentSlot = 0;
        gTasks[taskId].tIconSpriteId = SPRITE_NONE;
        sPopupTaskId = taskId;
        // A previous task can only be gone without reaching STATE_END if the overworld was
        // torn down, which also frees its window
        sPopupWindowId = WINDOW_NONE;
    }
}

bool8 IsNewMovesPopupActive(void)
{
    return IsPopupTaskAlive();
}

// External function to hide popup when player input is detected.
// Only a popup that is actually on screen is dismissed; Pokemon still waiting stay queued.
void HideNewMovesPopup(void)
{
    struct Task *task;

    if (!IsPopupTaskAlive())
        return;

    task = &gTasks[sPopupTaskId];
    switch (task->tState)
    {
    case STATE_SLIDE_IN:
    case STATE_WAIT:
    case STATE_SLIDE_OUT:
        HideNewMovesPopupWindow(sPopupTaskId);
        // Keep BG0 scrolled offscreen so old tiles aren't visible
        // The task will reset BG0VOFS to 0 after DMA clears the tilemap
        SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
        task->tState = STATE_NEXT;
        break;
    case STATE_INIT:
    case STATE_CREATE:
    case STATE_PAUSE:
        task->tState = STATE_WAIT_CONTROLS;
        break;
    }
}

static void Task_NewMovesPopup(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case STATE_WAIT_CONTROLS:
        // Wait for player controls to be unlocked, no script running, field message box hidden,
        // and no other popup on screen. A pickup popup still waiting for its turn defers to this one.
        if (!ArePlayerFieldControlsLocked() && !ScriptContext_IsEnabled() && IsFieldMessageBoxHidden()
            && !IsMapNamePopupTaskActive()
            && !IsPickupItemPopupShowing()
            && !IsChainRerollPopupActive())
        {
            task->tState = STATE_INIT;
        }
        break;
        
    case STATE_INIT:
    {
        s8 slot = GetNextQueuedPartySlot();

        if (slot < 0)
        {
            task->tState = STATE_END;
            break;
        }
        task->tCurrentSlot = slot;
        task->tState = STATE_CREATE;
        break;
    }
        
    case STATE_CREATE:
        // Check again before creating - if controls got locked or script started, wait
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden()
            || IsMapNamePopupTaskActive() || IsPickupItemPopupShowing() || IsChainRerollPopupActive())
        {
            task->tState = STATE_WAIT_CONTROLS;
            break;
        }
        ShowNewMovesPopupWindow(taskId, task->tCurrentSlot);
        DequeueFront();
        task->tSlideOffset = 0;
        task->tState = STATE_SLIDE_IN;
        break;
        
    case STATE_SLIDE_IN:
        // If player opens a menu, script starts, or message box appears, immediately hide
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden()
            || IsMapNamePopupTaskActive() || IsChainRerollPopupActive())
        {
            HideNewMovesPopupWindow(taskId);
            SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
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
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden()
            || IsMapNamePopupTaskActive() || IsChainRerollPopupActive())
        {
            HideNewMovesPopupWindow(taskId);
            SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
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
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden()
            || IsMapNamePopupTaskActive() || IsChainRerollPopupActive())
        {
            HideNewMovesPopupWindow(taskId);
            SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
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
        // Clear window and keep BG0 scrolled offscreen until DMA completes
        HideNewMovesPopupWindow(taskId);
        SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
        task->tState = STATE_RESET_SCROLL;
        break;

    case STATE_RESET_SCROLL:
        // DMA has cleared the tilemap in VRAM, safe to reset scroll
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        task->tDisplayTimer = 0;
        task->tState = STATE_PAUSE;
        break;

    case STATE_PAUSE:
        // Brief pause between consecutive popups
        task->tDisplayTimer++;
        if (task->tDisplayTimer > POPUP_PAUSE_TIME)
            task->tState = STATE_INIT;
        break;
        
    case STATE_NEXT:
        // Wait one frame for DMA to clear tilemap, then reset scroll
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        task->tState = STATE_INIT;
        break;
        
    case STATE_END:
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        sPopupTaskId = TASK_NONE;
        sPopupWindowId = WINDOW_NONE;
        DestroyTask(taskId);
        break;
    }
}

static void ShowNewMovesPopupWindow(u8 taskId, u8 partySlot)
{
    u16 species;
    u32 personality;
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
    AddTextPrinterParameterized4(sPopupWindowId, FONT_NARROW, TEXT_X_OFFSET, 2, 0, 0,
        (u8[]){TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY}, 
        TEXT_SKIP_DRAW, nickname);
    
    // Print "New Moves!" below - blue text
    AddTextPrinterParameterized4(sPopupWindowId, FONT_SMALL, TEXT_X_OFFSET, 14, 0, 0,
        (u8[]){TEXT_COLOR_TRANSPARENT, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE}, 
        TEXT_SKIP_DRAW, sText_NewMoves);
    
    // Put window on screen
    PutWindowTilemap(sPopupWindowId);
    CopyWindowToVram(sPopupWindowId, COPYWIN_FULL);
    
    // Start with BG0 scrolled to offscreen position
    SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
    
    // Create Pokemon icon sprite (starts offscreen)
    LoadMonIconPalette(species);
    gTasks[taskId].tIconSpriteId = CreateMonIconDeoxysForm(species, SpriteCB_MonIcon, POPUP_ICON_X, POPUP_SPRITE_OFFSCREEN_Y, 0, personality, GetMonDeoxysForm(&gPlayerParty[partySlot]));
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

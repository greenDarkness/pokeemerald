#include "global.h"
#include "bg.h"
#include "field_message_box.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "item.h"
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
#include "new_moves_popup.h"
#include "constants/songs.h"
#include "constants/items.h"

// Timing constants
#define POPUP_DISPLAY_TIME 120
#define POPUP_SLIDE_SPEED  4

// Slide animation - offscreen and onscreen Y offsets
#define POPUP_SCROLL_ONSCREEN   (256 - 134)
#define POPUP_SCROLL_OFFSCREEN  (256 - 90)
#define POPUP_SPRITE_ONSCREEN_Y 16
#define POPUP_SPRITE_OFFSCREEN_Y -20

// Window position
#define POPUP_WINDOW_LEFT   17
#define POPUP_WINDOW_TOP    0
#define POPUP_WINDOW_WIDTH  13
#define POPUP_WINDOW_HEIGHT 4

// Text X offset within window (to align with sprite)
#define TEXT_X_OFFSET       92

// Icon sprite position
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
#define tPokemonFlags  data[3]
#define tIconSpriteId  data[4]
#define tSlideOffset   data[5]

static EWRAM_DATA u8 sPopupWindowId = WINDOW_NONE;
static EWRAM_DATA u8 sPopupTaskId = TASK_NONE;

static void Task_PickupItemPopup(u8 taskId);
static void ShowPickupItemPopupWindow(u8 taskId, u8 partySlot);
static void HidePickupItemPopupWindow(u8 taskId);

// Subtitle text
static const u8 sText_FoundItem[] = _("Found Item!");

// Window template for popup
static const struct WindowTemplate sPickupItemPopupWindowTemplate = {
    .bg = 0,
    .tilemapLeft = POPUP_WINDOW_LEFT,
    .tilemapTop = POPUP_WINDOW_TOP,
    .width = POPUP_WINDOW_WIDTH,
    .height = POPUP_WINDOW_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 0x280,
};

void CheckAndShowPickupItemPopup(void)
{
    u8 flags = gSaveBlock1Ptr->pickupItemFlags;

    if (flags != 0)
    {
        if (!FuncIsActiveTask(Task_PickupItemPopup))
        {
            u8 taskId = CreateTask(Task_PickupItemPopup, 80);
            gTasks[taskId].tState = STATE_WAIT_CONTROLS;
            gTasks[taskId].tDisplayTimer = 0;
            gTasks[taskId].tCurrentSlot = 0;
            gTasks[taskId].tPokemonFlags = flags;
            gTasks[taskId].tIconSpriteId = SPRITE_NONE;
            sPopupTaskId = taskId;

            gSaveBlock1Ptr->pickupItemFlags = 0;
        }
    }
}

bool8 IsPickupItemPopupActive(void)
{
    return sPopupTaskId != TASK_NONE;
}

void HidePickupItemPopup(void)
{
    if (sPopupTaskId != TASK_NONE && FuncIsActiveTask(Task_PickupItemPopup))
    {
        HidePickupItemPopupWindow(sPopupTaskId);
        SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
        gTasks[sPopupTaskId].tState = STATE_NEXT;
    }
}

static void Task_PickupItemPopup(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case STATE_WAIT_CONTROLS:
        // Also wait for map name and new moves popups to finish to avoid BG0 scroll conflicts
        if (!ArePlayerFieldControlsLocked() && !ScriptContext_IsEnabled() && IsFieldMessageBoxHidden()
            && !IsMapNamePopupTaskActive()
            && !IsNewMovesPopupActive())
        {
            task->tState = STATE_INIT;
        }
        break;

    case STATE_INIT:
        while (task->tCurrentSlot < PARTY_SIZE)
        {
            if (task->tPokemonFlags & (1 << task->tCurrentSlot))
            {
                task->tState = STATE_CREATE;
                return;
            }
            task->tCurrentSlot++;
        }
        task->tState = STATE_END;
        break;

    case STATE_CREATE:
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden()
            || IsMapNamePopupTaskActive())
        {
            task->tState = STATE_WAIT_CONTROLS;
            break;
        }
        ShowPickupItemPopupWindow(taskId, task->tCurrentSlot);
        task->tSlideOffset = 0;
        task->tState = STATE_SLIDE_IN;
        break;

    case STATE_SLIDE_IN:
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden()
            || IsMapNamePopupTaskActive())
        {
            HidePickupItemPopupWindow(taskId);
            SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
            task->tState = STATE_NEXT;
            break;
        }
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
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden()
            || IsMapNamePopupTaskActive())
        {
            HidePickupItemPopupWindow(taskId);
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
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden()
            || IsMapNamePopupTaskActive())
        {
            HidePickupItemPopupWindow(taskId);
            SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
            task->tState = STATE_NEXT;
            break;
        }
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
        HidePickupItemPopupWindow(taskId);
        SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
        task->tState = STATE_RESET_SCROLL;
        break;

    case STATE_RESET_SCROLL:
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        task->tCurrentSlot++;
        task->tDisplayTimer = 0;
        task->tState = STATE_PAUSE;
        break;

    case STATE_PAUSE:
        task->tDisplayTimer++;
        if (task->tDisplayTimer > POPUP_PAUSE_TIME)
            task->tState = STATE_INIT;
        break;

    case STATE_NEXT:
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        task->tCurrentSlot++;
        task->tState = STATE_INIT;
        break;

    case STATE_END:
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        sPopupTaskId = TASK_NONE;
        DestroyTask(taskId);
        break;
    }
}

static void ShowPickupItemPopupWindow(u8 taskId, u8 partySlot)
{
    u16 species;
    u16 heldItem;
    u32 personality;
    u8 x;
    u8 *nickname = gStringVar1;
    u8 *itemName = gStringVar2;

    species = GetMonData(&gPlayerParty[partySlot], MON_DATA_SPECIES);
    personality = GetMonData(&gPlayerParty[partySlot], MON_DATA_PERSONALITY);
    heldItem = GetMonData(&gPlayerParty[partySlot], MON_DATA_HELD_ITEM);
    GetMonData(&gPlayerParty[partySlot], MON_DATA_NICKNAME, nickname);
    StringGet_Nickname(nickname);
    CopyItemName(heldItem, itemName);

    if (sPopupWindowId == WINDOW_NONE)
        sPopupWindowId = AddWindow(&sPickupItemPopupWindowTemplate);

    FillWindowPixelBuffer(sPopupWindowId, PIXEL_FILL(0));

    // Print Pokemon nickname - white text with dark shadow
    x = GetStringCenterAlignXOffset(FONT_NARROW, nickname, POPUP_WINDOW_WIDTH * 8 - TEXT_X_OFFSET);
    AddTextPrinterParameterized4(sPopupWindowId, FONT_NARROW, x + TEXT_X_OFFSET, 2, 0, 0,
        (u8[]){TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY},
        TEXT_SKIP_DRAW, nickname);

    // Print item name below - green text (matches new moves popup style but green instead of blue)
    x = GetStringCenterAlignXOffset(FONT_SMALL, itemName, POPUP_WINDOW_WIDTH * 8 - TEXT_X_OFFSET);
    AddTextPrinterParameterized4(sPopupWindowId, FONT_SMALL, x + TEXT_X_OFFSET, 14, 0, 0,
        (u8[]){TEXT_COLOR_TRANSPARENT, TEXT_COLOR_GREEN, TEXT_COLOR_LIGHT_GREEN},
        TEXT_SKIP_DRAW, itemName);

    PutWindowTilemap(sPopupWindowId);
    CopyWindowToVram(sPopupWindowId, COPYWIN_FULL);

    SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);

    // Create Pokemon icon sprite
    LoadMonIconPalette(species);
    gTasks[taskId].tIconSpriteId = CreateMonIcon(species, SpriteCB_MonIcon, POPUP_ICON_X, POPUP_SPRITE_OFFSCREEN_Y, 0, personality, TRUE);
    gSprites[gTasks[taskId].tIconSpriteId].subpriority = 0;

    // Play Pokemon cry
    PlayCry_Normal(species, 0);
}

static void HidePickupItemPopupWindow(u8 taskId)
{
    if (gTasks[taskId].tIconSpriteId != SPRITE_NONE)
    {
        u16 species = GetMonData(&gPlayerParty[gTasks[taskId].tCurrentSlot], MON_DATA_SPECIES);
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].tIconSpriteId]);
        FreeMonIconPalette(species);
        gTasks[taskId].tIconSpriteId = SPRITE_NONE;
    }

    if (sPopupWindowId != WINDOW_NONE)
    {
        ClearWindowTilemap(sPopupWindowId);
        CopyWindowToVram(sPopupWindowId, COPYWIN_MAP);
    }
}

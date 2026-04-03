#include "global.h"
#include "battle.h"
#include "bg.h"
#include "battle_setup.h"
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
#include "new_moves_popup.h"
#include "pickup_item_popup.h"
#include "constants/songs.h"
#include "constants/rgb.h"

// Timing constants
#define POPUP_DISPLAY_TIME 120
#define POPUP_SLIDE_SPEED  4

// Slide animation - matching new_moves_popup exactly
#define POPUP_SCROLL_OFFSCREEN  (256 - 90)
#define POPUP_SPRITE_OFFSCREEN_Y (-20)

// Window position - same as new_moves_popup
#define POPUP_WINDOW_LEFT   17
#define POPUP_WINDOW_TOP    0
#define POPUP_WINDOW_WIDTH  13
#define POPUP_WINDOW_HEIGHT 4

// Icon sprite position
#define POPUP_ICON_X  96
#define POPUP_ICON_Y  16

// Text X offset (shift right to make room for icon)
#define TEXT_X_OFFSET 92

// Popup states
enum {
    STATE_SLIDE_IN,
    STATE_WAIT,
    STATE_SLIDE_OUT,
    STATE_CLEANUP,
    STATE_RESET_SCROLL,
    STATE_NEXT,
    STATE_END,
};

#define tState         data[0]
#define tDisplayTimer  data[1]
#define tSlideOffset   data[2]
#define tIconSpriteId  data[3]

static EWRAM_DATA u8 sPopupWindowId = WINDOW_NONE;
static EWRAM_DATA u8 sPopupTaskId = TASK_NONE;
static EWRAM_DATA u8 sRerollTextBuf[24] = {0};

static void Task_ChainRerollPopup(u8 taskId);
static void ShowChainRerollPopupWindow(u8 taskId);
static void HideChainRerollPopupWindow(u8 taskId);

static const struct WindowTemplate sChainRerollPopupWindowTemplate = {
    .bg = 0,
    .tilemapLeft = POPUP_WINDOW_LEFT,
    .tilemapTop = POPUP_WINDOW_TOP,
    .width = POPUP_WINDOW_WIDTH,
    .height = POPUP_WINDOW_HEIGHT,
    .paletteNum = 15,
    .baseBlock = 0x280,
};

bool8 CheckAndShowChainRerollPopup(void)
{
    if (HasPendingRerollNotification())
    {
        u8 count = GetPendingRerollCount();
        u8 *ptr = sRerollTextBuf;
        static const u8 sText_Plus[] = _("+");
        static const u8 sText_Rerolls[] = _(" Rerolls");

        StringCopy(ptr, sText_Plus);
        ptr += StringLength(sText_Plus);
        ptr = ConvertIntToDecimalStringN(ptr, count, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringCopy(ptr, sText_Rerolls);

        ClearPendingRerollNotification();
        if (!FuncIsActiveTask(Task_ChainRerollPopup))
        {
            u8 taskId = CreateTask(Task_ChainRerollPopup, 80);
            ShowChainRerollPopupWindow(taskId);
            gTasks[taskId].tState = STATE_SLIDE_IN;
            gTasks[taskId].tDisplayTimer = 0;
            gTasks[taskId].tSlideOffset = 0;
            sPopupTaskId = taskId;
            PlayFanfare(MUS_RG_PHOTO);
            return TRUE;
        }
    }
    return FALSE;
}

bool8 IsChainRerollPopupActive(void)
{
    return sPopupTaskId != TASK_NONE;
}

void HideChainRerollPopup(void)
{
    if (sPopupTaskId != TASK_NONE && FuncIsActiveTask(Task_ChainRerollPopup))
    {
        HideChainRerollPopupWindow(sPopupTaskId);
        SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
        gTasks[sPopupTaskId].tState = STATE_NEXT;
    }
}

static void Task_ChainRerollPopup(u8 taskId)
{
    struct Task *task = &gTasks[taskId];

    switch (task->tState)
    {
    case STATE_SLIDE_IN:
        if (ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden()
            || IsMapNamePopupTaskActive())
        {
            HideChainRerollPopupWindow(taskId);
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
            HideChainRerollPopupWindow(taskId);
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
            HideChainRerollPopupWindow(taskId);
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
        HideChainRerollPopupWindow(taskId);
        SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);
        task->tState = STATE_RESET_SCROLL;
        break;

    case STATE_RESET_SCROLL:
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        task->tState = STATE_END;
        break;

    case STATE_NEXT:
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        task->tState = STATE_END;
        break;

    case STATE_END:
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        sPopupTaskId = TASK_NONE;
        DestroyTask(taskId);
        break;
    }
}

static void ShowChainRerollPopupWindow(u8 taskId)
{
    u16 species;
    u8 x;

    species = ReadChainSpecies();

    if (sPopupWindowId == WINDOW_NONE)
        sPopupWindowId = AddWindow(&sChainRerollPopupWindowTemplate);

    FillWindowPixelBuffer(sPopupWindowId, PIXEL_FILL(0));

    // Print reroll text - shifted right to leave room for icon
    x = GetStringCenterAlignXOffset(FONT_NARROW, sRerollTextBuf, POPUP_WINDOW_WIDTH * 8 - TEXT_X_OFFSET);
    AddTextPrinterParameterized4(sPopupWindowId, FONT_NARROW, x + TEXT_X_OFFSET, 8, 0, 0,
        (u8[]){TEXT_COLOR_TRANSPARENT, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED},
        TEXT_SKIP_DRAW, sRerollTextBuf);

    PutWindowTilemap(sPopupWindowId);
    CopyWindowToVram(sPopupWindowId, COPYWIN_FULL);

    SetGpuReg(REG_OFFSET_BG0VOFS, POPUP_SCROLL_OFFSCREEN);

    // Create Pokemon icon sprite (starts offscreen)
    LoadMonIconPalette(species);
    gTasks[taskId].tIconSpriteId = CreateMonIcon(species, SpriteCB_MonIcon, POPUP_ICON_X, POPUP_SPRITE_OFFSCREEN_Y, 0, 0, TRUE);
    gSprites[gTasks[taskId].tIconSpriteId].subpriority = 0;

    PlayFanfare(MUS_RG_PHOTO);
}

static void HideChainRerollPopupWindow(u8 taskId)
{
    // Destroy the icon sprite
    if (gTasks[taskId].tIconSpriteId != SPRITE_NONE)
    {
        u16 species = ReadChainSpecies();
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

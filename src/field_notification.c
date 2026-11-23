#include "global.h"
#include "battle_setup.h"
#include "field_message_box.h"
#include "field_notification.h"
#include "field_weather.h"
#include "gpu_regs.h"
#include "item.h"
#include "map_name_popup.h"
#include "match_call.h"
#include "menu.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "text.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

#define QUEUE_SIZE 16

// Frames a popup stays put once it has slid in
#define DISPLAY_TIME 120
// A notification that is kept (see struct NotificationType) and is taken off the screen before
// it has been up this long is shown again later
#define SEEN_TIME 60

#define BANNER_TEXT_LENGTH 32
#define BANNER_WINDOW_LEFT 2
#define BANNER_WINDOW_TOP 15
#define BANNER_WINDOW_WIDTH 27
#define BANNER_HEIGHT 32
// Where the banner stops below the top of the screen, and how far it slides to get there
#define BANNER_Y 3
#define BANNER_SLIDE_DISTANCE 36
#define BANNER_PADDING 7
#define BANNER_ICON_SIZE 32
// Icons are drawn low in their frames, so the frame goes a little above the panel's middle
#define BANNER_ICON_Y (BANNER_Y + BANNER_ICON_SIZE / 2 - 2 - BANNER_SLIDE_DISTANCE)
// Nothing else in the overworld uses this BG palette, and the time of day doesn't tint it
#define BANNER_PALETTE_NUM 13
// How much of the panel and of what is under it show where the panel is, out of 16
#define BANNER_BLEND_PANEL 11
#define BANNER_BLEND_MAP 6
// The least fog's sprites can be blended with for the panel to be blended over them
#define BANNER_MIN_FOG_BLEND 8
#define BANNER_NO_TINT 0xFFFFFFFF

enum
{
    STYLE_MAP_NAME, // The map's name in a frame, at the top left
    STYLE_BANNER,   // A colored panel with a Pokémon's icon and one or two lines of text, at the top
    STYLE_COUNT
};

enum
{
    STATE_IDLE,
    STATE_DELAY,
    STATE_SLIDE_IN,
    STATE_ON_SCREEN,
    STATE_SLIDE_OUT,
    STATE_ERASE,
    STATE_RESET_SCROLL,
};

// The banner's panel. Each is a palette in graphics/field_notification/
enum
{
    BANNER_BLUE,
    BANNER_GREEN,
    BANNER_RED,
    BANNER_COLOR_COUNT
};

// What the colors of a banner's palette are for
enum
{
    BANNER_PAL_TRANSPARENT,
    BANNER_PAL_TITLE,
    BANNER_PAL_TEXT_SHADOW,
    BANNER_PAL_TEXT,
    BANNER_PAL_FILL,
    BANNER_PAL_HIGHLIGHT, // The panel's top edge
    BANNER_PAL_SHADE,     // The panel's bottom edge
    BANNER_PAL_OUTLINE,
};

struct FieldNotification
{
    u8 type;
    u16 arg;
    u32 id;
};

struct Banner
{
    u8 title[BANNER_TEXT_LENGTH]; // Top line, in white. Left empty, the banner has only the text
    u8 text[BANNER_TEXT_LENGTH];
    u8 color;                     // BANNER_*
    u8 iconDeoxysForm;
    u16 iconSpecies;
    u32 iconPersonality;
};

struct NotificationStyle
{
    u8 offscreenY;    // BG0VOFS that puts the popup just above the screen
    u8 slideDistance;
    u8 slideSpeed;
    u8 delay;         // Frames between its turn coming and it being drawn
    u8 pause;         // Frames left between it and the next popup with this style
    bool8 (*isBlocked)(void);
    bool8 (*draw)(const struct FieldNotification *notif); // FALSE if there is nothing to show
    void (*update)(void);                                 // Each frame from when it is drawn
    void (*erase)(void);
};

struct NotificationType
{
    u8 style;
    u8 priority;  // The highest is shown first. Among equals, the oldest is
    bool8 urgent; // Hurries the popup on screen away rather than wait for it to finish
    // Kept until the player has had the chance to see it: it waits for as long as it can't be
    // shown, and comes back if it is cut short. Otherwise it is only shown if it can be at once.
    bool8 keep;
    // For banners. Fills in what to show and plays its sound. FALSE if there is nothing to
    // show any more.
    bool8 (*fillBanner)(const struct FieldNotification *notif, struct Banner *banner);
};

static bool8 IsMapNameBlocked(void);
static bool8 DrawMapName(const struct FieldNotification *notif);
static bool8 IsBannerBlocked(void);
static bool8 DrawBanner(const struct FieldNotification *notif);
static void UpdateBannerBlend(void);
static void EraseBanner(void);
static bool8 FillBanner_ChainReroll(const struct FieldNotification *notif, struct Banner *banner);
static bool8 FillBanner_NewMoves(const struct FieldNotification *notif, struct Banner *banner);
static bool8 FillBanner_PickupItem(const struct FieldNotification *notif, struct Banner *banner);

static EWRAM_DATA struct FieldNotification sQueue[QUEUE_SIZE] = {0};
static EWRAM_DATA u8 sQueueCount = 0;
static EWRAM_DATA struct FieldNotification sCurrent = {0};
static EWRAM_DATA u8 sState = 0;
static EWRAM_DATA u8 sTimer = 0;
static EWRAM_DATA u8 sSlide = 0;
static EWRAM_DATA bool8 sSeen = FALSE;
static EWRAM_DATA u8 sPauseTimer = 0;
static EWRAM_DATA u8 sPauseStyle = 0;
static EWRAM_DATA u8 sBannerWindowId = 0;
static EWRAM_DATA u8 sIconSpriteId = 0;
static EWRAM_DATA u16 sIconSpecies = 0;
static EWRAM_DATA bool8 sBannerIsBlended = FALSE;
static EWRAM_DATA bool8 sBannerEnabledWindowBlend = FALSE;
static EWRAM_DATA u8 sBannerColor = 0;
static EWRAM_DATA u32 sBannerTint = 0;

static const struct NotificationStyle sStyles[STYLE_COUNT] =
{
    [STYLE_MAP_NAME] =
    {
        .offscreenY = 40,
        .slideDistance = 40,
        .slideSpeed = 2,
        .delay = 30,
        .isBlocked = IsMapNameBlocked,
        .draw = DrawMapName,
        .erase = EraseMapNamePopUpWindow,
    },
    [STYLE_BANNER] =
    {
        .offscreenY = BANNER_WINDOW_TOP * 8 - BANNER_Y + BANNER_SLIDE_DISTANCE,
        .slideDistance = BANNER_SLIDE_DISTANCE,
        .slideSpeed = 4,
        .pause = 30,
        .isBlocked = IsBannerBlocked,
        .draw = DrawBanner,
        .update = UpdateBannerBlend,
        .erase = EraseBanner,
    },
};

static const struct NotificationType sTypes[NOTIF_COUNT] =
{
    [NOTIF_MAP_NAME] =
    {
        .style = STYLE_MAP_NAME,
        .priority = 2,
        .urgent = TRUE,
    },
    [NOTIF_CHAIN_REROLL] =
    {
        .style = STYLE_BANNER,
        .priority = 1,
        .keep = TRUE,
        .fillBanner = FillBanner_ChainReroll,
    },
    [NOTIF_NEW_MOVES] =
    {
        .style = STYLE_BANNER,
        .keep = TRUE,
        .fillBanner = FillBanner_NewMoves,
    },
    [NOTIF_PICKUP_ITEM] =
    {
        .style = STYLE_BANNER,
        .keep = TRUE,
        .fillBanner = FillBanner_PickupItem,
    },
};

// The banner is drawn on the message box's rows, which the scroll brings to the top of the
// screen. Its panel is as wide as its text needs, in the middle of the screen.
static const struct WindowTemplate sBannerWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = BANNER_WINDOW_LEFT,
    .tilemapTop = BANNER_WINDOW_TOP,
    .width = BANNER_WINDOW_WIDTH,
    .height = BANNER_HEIGHT / 8,
    .paletteNum = BANNER_PALETTE_NUM,
    .baseBlock = 0x280,
};

static const u16 sBannerPalettes[BANNER_COLOR_COUNT][16] =
{
    [BANNER_BLUE]  = INCGFX_U16("graphics/field_notification/blue.pal", ".gbapal"),
    [BANNER_GREEN] = INCGFX_U16("graphics/field_notification/green.pal", ".gbapal"),
    [BANNER_RED]   = INCGFX_U16("graphics/field_notification/red.pal", ".gbapal"),
};

// The corners of the panel, in BANNER_PAL_* colors
static const u8 sBannerCorner[3][3] =
{
    {BANNER_PAL_TRANSPARENT, BANNER_PAL_TRANSPARENT, BANNER_PAL_OUTLINE},
    {BANNER_PAL_TRANSPARENT, BANNER_PAL_OUTLINE,     BANNER_PAL_OUTLINE},
    {BANNER_PAL_OUTLINE,     BANNER_PAL_OUTLINE,     BANNER_PAL_FILL},
};

static const struct ScanlineEffectParams sBannerBlendParams =
{
    .dmaDest = &REG_BLDCNT, // and REG_BLDALPHA, which is right after it
    .dmaControl = SCANLINE_EFFECT_DMACNT_32BIT,
    .initState = 1,
};

static const u8 sText_NewMoves[] = _("New Moves!");
static const u8 sText_Plus[] = _("+");
static const u8 sText_Rerolls[] = _(" Rerolls");

static const struct NotificationStyle *GetStyle(const struct FieldNotification *notif)
{
    return &sStyles[sTypes[notif->type].style];
}

static bool8 IsInQueue(u8 type, u32 id, u8 *pos)
{
    u8 i;

    for (i = 0; i < sQueueCount; i++)
    {
        if (sQueue[i].type == type && sQueue[i].id == id)
        {
            *pos = i;
            return TRUE;
        }
    }
    return FALSE;
}

static void RemoveFromQueue(u8 i)
{
    sQueueCount--;
    for (; i < sQueueCount; i++)
        sQueue[i] = sQueue[i + 1];
}

// Goes behind the notifications with its priority, or ahead of them if it is to be the next
// one shown. With the queue full, the last one in line is dropped.
static void AddToQueue(const struct FieldNotification *notif, bool8 next)
{
    u8 priority = sTypes[notif->type].priority;
    u8 pos, i;

    for (pos = 0; pos < sQueueCount; pos++)
    {
        u8 other = sTypes[sQueue[pos].type].priority;

        if (other < priority || (next && other == priority))
            break;
    }
    if (pos == QUEUE_SIZE)
        return;

    if (sQueueCount < QUEUE_SIZE)
        sQueueCount++;
    for (i = sQueueCount - 1; i > pos; i--)
        sQueue[i] = sQueue[i - 1];
    sQueue[pos] = *notif;
}

static void RemoveUnkeptFromQueue(void)
{
    u8 i = 0;

    while (i < sQueueCount)
    {
        if (sTypes[sQueue[i].type].keep)
            i++;
        else
            RemoveFromQueue(i);
    }
}

void QueueFieldNotification(u8 type, u32 id, u16 arg)
{
    struct FieldNotification notif;
    u8 pos;

    if (IsInQueue(type, id, &pos))
    {
        sQueue[pos].arg = arg;
        return;
    }

    if (sState == STATE_DELAY && sCurrent.type == type && sCurrent.id == id)
    {
        sCurrent.arg = arg;
        sTimer = 0;
        return;
    }

    notif.type = type;
    notif.arg = arg;
    notif.id = id;
    AddToQueue(&notif, FALSE);
    if (sTypes[type].urgent && (sState == STATE_SLIDE_IN || sState == STATE_ON_SCREEN))
        sState = STATE_SLIDE_OUT;
}

void QueueMonFieldNotification(u8 type, struct Pokemon *mon, u16 arg)
{
    if (GetMonData(mon, MON_DATA_SPECIES) != SPECIES_NONE)
        QueueFieldNotification(type, GetMonData(mon, MON_DATA_PERSONALITY), arg);
}

void QueuePickupItemNotification(struct Pokemon *mon)
{
    QueueMonFieldNotification(NOTIF_PICKUP_ITEM, mon, GetMonData(mon, MON_DATA_HELD_ITEM));
}

bool8 AreFieldNotificationsPending(void)
{
    return sState != STATE_IDLE || sQueueCount != 0;
}

static void SetSlide(u8 slide)
{
    sSlide = slide;
    SetGpuReg(REG_OFFSET_BG0VOFS, GetStyle(&sCurrent)->offscreenY - slide);
    if (sIconSpriteId != SPRITE_NONE)
        gSprites[sIconSpriteId].y = BANNER_ICON_Y + slide;
}

// The current notification is done with, having been seen or not
static void RetireCurrent(void)
{
    u8 pos;

    if (sTypes[sCurrent.type].keep && !sSeen && !IsInQueue(sCurrent.type, sCurrent.id, &pos))
        AddToQueue(&sCurrent, TRUE);
}

static void Dismiss(void)
{
    const struct NotificationStyle *style = GetStyle(&sCurrent);

    style->erase();
    // BG0 is only scrolled back once the erased tilemap has reached VRAM
    SetGpuReg(REG_OFFSET_BG0VOFS, style->offscreenY);
    RetireCurrent();
    sState = STATE_RESET_SCROLL;
}

void HideFieldNotifications(void)
{
    RemoveUnkeptFromQueue();
    if (sState != STATE_IDLE && sState != STATE_RESET_SCROLL)
        Dismiss();
}

// The overworld has been rebuilt, which took any popup's window and sprite with it
void ResetFieldNotificationGfx(void)
{
    RemoveUnkeptFromQueue();
    if (sState != STATE_IDLE && sState != STATE_RESET_SCROLL)
        RetireCurrent();
    sState = STATE_IDLE;
    sPauseTimer = 0;
    sBannerWindowId = WINDOW_NONE;
    sIconSpriteId = SPRITE_NONE;
    sBannerIsBlended = FALSE;
    sBannerEnabledWindowBlend = FALSE;
}

static bool8 TakeNextFromQueue(void)
{
    bool8 blocked[STYLE_COUNT];
    u8 i;

    for (i = 0; i < STYLE_COUNT; i++)
        blocked[i] = sStyles[i].isBlocked();

    i = 0;
    while (i < sQueueCount)
    {
        const struct NotificationType *type = &sTypes[sQueue[i].type];

        if (blocked[type->style])
        {
            if (type->keep)
                i++;
            else
                RemoveFromQueue(i);
        }
        else if (sPauseTimer != 0 && type->style == sPauseStyle)
        {
            i++;
        }
        else
        {
            sCurrent = sQueue[i];
            RemoveFromQueue(i);
            sState = STATE_DELAY;
            sTimer = 0;
            sSeen = FALSE;
            SetSlide(0);
            return TRUE;
        }
    }
    return FALSE;
}

// Runs every frame of the overworld
void UpdateFieldNotifications(void)
{
    const struct NotificationStyle *style;

    if (sState == STATE_IDLE)
    {
        if (sPauseTimer != 0)
            sPauseTimer--;
        if (sQueueCount == 0 || !TakeNextFromQueue())
            return;
    }

    style = GetStyle(&sCurrent);
    if (sState <= STATE_SLIDE_OUT && style->isBlocked())
    {
        Dismiss();
        return;
    }

    switch (sState)
    {
    case STATE_DELAY:
        if (sTimer < style->delay)
        {
            sTimer++;
        }
        else if (style->draw(&sCurrent))
        {
            sState = STATE_SLIDE_IN;
        }
        else
        {
            SetGpuReg(REG_OFFSET_BG0VOFS, 0);
            sState = STATE_IDLE;
        }
        break;
    case STATE_SLIDE_IN:
        if (sSlide + style->slideSpeed < style->slideDistance)
        {
            SetSlide(sSlide + style->slideSpeed);
        }
        else
        {
            SetSlide(style->slideDistance);
            sTimer = 0;
            sState = STATE_ON_SCREEN;
        }
        break;
    case STATE_ON_SCREEN:
        sTimer++;
        if (sTimer >= SEEN_TIME)
            sSeen = TRUE;
        if (sTimer > DISPLAY_TIME)
            sState = STATE_SLIDE_OUT;
        break;
    case STATE_SLIDE_OUT:
        if (sSlide > style->slideSpeed)
        {
            SetSlide(sSlide - style->slideSpeed);
        }
        else
        {
            SetSlide(0);
            sState = STATE_ERASE;
        }
        break;
    case STATE_ERASE:
        Dismiss();
        sPauseTimer = style->pause;
        sPauseStyle = sTypes[sCurrent.type].style;
        break;
    case STATE_RESET_SCROLL:
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        sState = STATE_IDLE;
        break;
    }

    if (style->update != NULL && sState >= STATE_SLIDE_IN && sState <= STATE_ERASE)
        style->update();
}

// Map name

static bool8 IsMapNameBlocked(void)
{
    // The match call window uses the popup's palette
    return IsMatchCallTaskActive();
}

static bool8 DrawMapName(const struct FieldNotification *notif)
{
    DrawMapNamePopUpWindow();
    return TRUE;
}

// Banners

static bool8 IsBannerBlocked(void)
{
    return ArePlayerFieldControlsLocked() || ScriptContext_IsEnabled() || !IsFieldMessageBoxHidden();
}

// The panel is see-through: on the scanlines it is on, BG0 is blended over what is under it.
// Shadows and the weather's sprites are blended with the same registers, so a scanline effect
// sets them for the panel's scanlines only, and the rest of the screen keeps its own. While
// something else has the blend registers or the scanline effect to itself (a screen fade, the
// darkness of a cave), the panel is solid instead.

static bool32 IsBannerBlendScanlineEffectSet(void)
{
    return gScanlineEffect.state != 0 && gScanlineEffect.dmaDest == &REG_BLDCNT;
}

// Fog is sprites blended over the whole screen. The coefficients they are blended with can't
// be changed on the panel's scanlines without the fog on those changing with them, so there
// the panel is blended with the fog's. What is under it is the fog itself rather than the map,
// which only one layer can be blended with, and its colors are set to what makes it come out
// over the fog as it does over the map elsewhere.
static bool32 IsBannerOverFog(void)
{
    return gWeatherPtr->fogHSpritesCreated || gWeatherPtr->fogDSpritesCreated;
}

static bool32 CanBlendBanner(void)
{
    u32 effect = GetGpuReg(REG_OFFSET_BLDCNT) & (BLDCNT_TGT1_ALL | BLDCNT_EFFECT_EFF_MASK);

    if (gPaletteFade.active)
        return FALSE;
    if (effect != BLDCNT_EFFECT_NONE && effect != BLDCNT_EFFECT_BLEND && effect != BLDCNT_FOG_TOP_LAYER)
        return FALSE;
    // Fog that is coming in or clearing
    if (IsBannerOverFog() && BLDALPHA_TGT1(GetGpuReg(REG_OFFSET_BLDALPHA)) < BANNER_MIN_FOG_BLEND)
        return FALSE;
    return gScanlineEffect.state == 0 || IsBannerBlendScanlineEffectSet();
}

static void LoadBannerPalette(const u16 *palette)
{
    // A fade goes on from the colors it is given
    if (gPaletteFade.active)
        CpuCopy16(palette, &gPlttBufferUnfaded[BG_PLTT_ID(BANNER_PALETTE_NUM)], PLTT_SIZE_4BPP);
    else
        LoadPalette(palette, BG_PLTT_ID(BANNER_PALETTE_NUM), PLTT_SIZE_4BPP);
}

static void RemoveBannerTint(void)
{
    if (sBannerTint != BANNER_NO_TINT)
    {
        sBannerTint = BANNER_NO_TINT;
        LoadBannerPalette(sBannerPalettes[sBannerColor]);
    }
}

static s32 TintBannerColor(s32 color, s32 fog, s32 panelBlend, s32 fogBlend)
{
    color = (color * BANNER_BLEND_PANEL + fog * (BANNER_BLEND_MAP - fogBlend)) / panelBlend;
    if (color < 0)
        return 0;
    return min(color, 31);
}

// Sets the panel's colors for it to look, blended over the fog with the fog's coefficients,
// as it would with its own
static void TintBannerForFog(u16 blend)
{
    const u16 *src = sBannerPalettes[sBannerColor];
    u16 palette[16];
    u16 fog = Fog_GetAverageColor();
    u32 tint = (fog << 16) | blend;
    s32 panelBlend = min(BLDALPHA_TGT1(blend), 16);
    s32 fogBlend = min(BLDALPHA_TGT2(blend), 16);
    u32 i;

    if (tint == sBannerTint)
        return;
    sBannerTint = tint;

    for (i = 0; i < 16; i++)
    {
        palette[i] = RGB(TintBannerColor(GET_R(src[i]), GET_R(fog), panelBlend, fogBlend),
                         TintBannerColor(GET_G(src[i]), GET_G(fog), panelBlend, fogBlend),
                         TintBannerColor(GET_B(src[i]), GET_B(fog), panelBlend, fogBlend));
    }
    LoadBannerPalette(palette);
}

static void SetBannerBlendScanlines(u16 *buffer, u16 panelBlend)
{
    // Read every frame, as the weather and the lights of the night keep changing them
    u16 mapBldCnt = GetGpuReg(REG_OFFSET_BLDCNT);
    u16 mapBlend = GetGpuReg(REG_OFFSET_BLDALPHA);
    u16 panelBldCnt = (mapBldCnt & BLDCNT_TGT2_ALL) | BLDCNT_TGT1_BG0 | BLDCNT_EFFECT_BLEND;
    s32 top = BANNER_Y - BANNER_SLIDE_DISTANCE + sSlide;
    s32 y;

    // One more than there are scanlines: the last is what the registers are left with
    for (y = 0; y <= DISPLAY_HEIGHT; y++)
    {
        if (y >= top && y < top + BANNER_HEIGHT)
        {
            *buffer++ = panelBldCnt;
            *buffer++ = panelBlend;
        }
        else
        {
            *buffer++ = mapBldCnt;
            *buffer++ = mapBlend;
        }
    }
}

static void StopBannerBlend(void)
{
    if (!sBannerIsBlended)
        return;
    sBannerIsBlended = FALSE;

    RemoveBannerTint();
    // A screen fade turns this on for itself
    if (sBannerEnabledWindowBlend && !gPaletteFade.active)
        ClearGpuRegBits(REG_OFFSET_WININ, WININ_WIN0_CLR);
    sBannerEnabledWindowBlend = FALSE;
    if (IsBannerBlendScanlineEffectSet())
    {
        // Stops it in the next VBlank, which is also when the panel's tilemap is erased
        gScanlineEffect.state = 3;
        SetGpuReg(REG_OFFSET_BLDCNT, GetGpuReg(REG_OFFSET_BLDCNT));
        SetGpuReg(REG_OFFSET_BLDALPHA, GetGpuReg(REG_OFFSET_BLDALPHA));
    }
}

// Runs after the sprites' callbacks, as some of them set the blend coefficients
static void UpdateBannerBlend(void)
{
    u16 panelBlend;

    if (!CanBlendBanner())
    {
        StopBannerBlend();
        return;
    }

    if (IsBannerOverFog())
    {
        panelBlend = GetGpuReg(REG_OFFSET_BLDALPHA);
        TintBannerForFog(panelBlend);
    }
    else
    {
        panelBlend = BLDALPHA_BLEND(BANNER_BLEND_PANEL, BANNER_BLEND_MAP);
        RemoveBannerTint();
    }

    if (!sBannerIsBlended || !IsBannerBlendScanlineEffectSet())
    {
        SetBannerBlendScanlines(gScanlineEffectRegBuffers[0], panelBlend);
        SetBannerBlendScanlines(gScanlineEffectRegBuffers[1], panelBlend);
        ScanlineEffect_SetParams(sBannerBlendParams);
        sBannerIsBlended = TRUE;
    }
    else
    {
        SetBannerBlendScanlines(gScanlineEffectRegBuffers[gScanlineEffect.srcBuffer], panelBlend);
    }

    // The whole screen is inside window 0
    if (!(GetGpuReg(REG_OFFSET_WININ) & WININ_WIN0_CLR))
    {
        SetGpuRegBits(REG_OFFSET_WININ, WININ_WIN0_CLR);
        sBannerEnabledWindowBlend = TRUE;
    }
}

// Whole tiles are filled with the panel's color, as going a pixel at a time is far too slow
// for more than its edges. Only the tiles the panel is on are then put on the tilemap.
static void DrawBannerPanel(s32 x, s32 width)
{
    s32 right = x + width - 1;
    s32 bottom = BANNER_HEIGHT - 1;
    s32 i, j;

    FillWindowPixelBuffer(sBannerWindowId, PIXEL_FILL(BANNER_PAL_FILL));
    FillWindowPixelRect(sBannerWindowId, PIXEL_FILL(BANNER_PAL_OUTLINE), x + 3, 0, width - 6, 1);
    FillWindowPixelRect(sBannerWindowId, PIXEL_FILL(BANNER_PAL_HIGHLIGHT), x + 3, 1, width - 6, 1);
    FillWindowPixelRect(sBannerWindowId, PIXEL_FILL(BANNER_PAL_SHADE), x + 3, bottom - 1, width - 6, 1);
    FillWindowPixelRect(sBannerWindowId, PIXEL_FILL(BANNER_PAL_OUTLINE), x + 3, bottom, width - 6, 1);
    FillWindowPixelRect(sBannerWindowId, PIXEL_FILL(BANNER_PAL_OUTLINE), x, 3, 1, BANNER_HEIGHT - 6);
    FillWindowPixelRect(sBannerWindowId, PIXEL_FILL(BANNER_PAL_OUTLINE), right, 3, 1, BANNER_HEIGHT - 6);
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 3; j++)
        {
            u8 fill = PIXEL_FILL(sBannerCorner[i][j]);

            FillWindowPixelRect(sBannerWindowId, fill, x + j, i, 1, 1);
            FillWindowPixelRect(sBannerWindowId, fill, right - j, i, 1, 1);
            FillWindowPixelRect(sBannerWindowId, fill, x + j, bottom - i, 1, 1);
            FillWindowPixelRect(sBannerWindowId, fill, right - j, bottom - i, 1, 1);
        }
    }
}

static bool8 DrawBanner(const struct FieldNotification *notif)
{
    static const u8 sTitleColors[] = {BANNER_PAL_TRANSPARENT, BANNER_PAL_TITLE, BANNER_PAL_TEXT_SHADOW};
    static const u8 sTextColors[] = {BANNER_PAL_TRANSPARENT, BANNER_PAL_TEXT, BANNER_PAL_TEXT_SHADOW};
    struct Banner banner;
    bool32 hasTitle, hasIcon;
    s32 x, textX, width, column, columns;
    u8 textFont;

    banner.title[0] = EOS;
    banner.text[0] = EOS;
    banner.color = BANNER_BLUE;
    banner.iconDeoxysForm = 0;
    banner.iconSpecies = SPECIES_NONE;
    banner.iconPersonality = 0;

    if (sBannerWindowId == WINDOW_NONE)
    {
        sBannerWindowId = AddWindow(&sBannerWindowTemplate);
        if (sBannerWindowId == WINDOW_NONE)
            return FALSE;
    }
    if (!sTypes[notif->type].fillBanner(notif, &banner))
        return FALSE;

    hasTitle = banner.title[0] != EOS;
    hasIcon = banner.iconSpecies != SPECIES_NONE;
    textFont = hasTitle ? FONT_SMALL : FONT_NARROW;

    width = GetStringWidth(textFont, banner.text, 0);
    if (hasTitle)
        width = max(width, GetStringWidth(FONT_NARROW, banner.title, 0));
    textX = hasIcon ? BANNER_ICON_SIZE + 4 : BANNER_PADDING;
    width += textX + BANNER_PADDING;
    // An even number of tiles, for the panel to be in the middle of the screen
    columns = min((width + 15) / 16 * 2, BANNER_WINDOW_WIDTH - 1);
    column = (DISPLAY_WIDTH / 8 - columns) / 2 - BANNER_WINDOW_LEFT;
    // The icon and the text are in the middle of the panel
    x = column * 8 + (columns * 8 - width) / 2;

    sBannerColor = banner.color;
    sBannerTint = BANNER_NO_TINT;
    LoadPalette(sBannerPalettes[banner.color], BG_PLTT_ID(BANNER_PALETTE_NUM), PLTT_SIZE_4BPP);
    DrawBannerPanel(column * 8, columns * 8);
    if (hasTitle)
    {
        AddTextPrinterParameterized4(sBannerWindowId, FONT_NARROW, x + textX, 3, 0, 0, sTitleColors, TEXT_SKIP_DRAW, banner.title);
        AddTextPrinterParameterized4(sBannerWindowId, textFont, x + textX, 15, 0, 0, sTextColors, TEXT_SKIP_DRAW, banner.text);
    }
    else
    {
        AddTextPrinterParameterized4(sBannerWindowId, textFont, x + textX, 9, 0, 0, sTextColors, TEXT_SKIP_DRAW, banner.text);
    }
    PutWindowRectTilemap(sBannerWindowId, column, 0, columns, BANNER_HEIGHT / 8);
    CopyWindowToVram(sBannerWindowId, COPYWIN_FULL);

    if (hasIcon)
    {
        LoadMonIconPalette(banner.iconSpecies);
        sIconSpecies = banner.iconSpecies;
        sIconSpriteId = CreateMonIconDeoxysForm(banner.iconSpecies, SpriteCB_MonIcon,
                                                BANNER_WINDOW_LEFT * 8 + x + 2 + BANNER_ICON_SIZE / 2, BANNER_ICON_Y,
                                                0, banner.iconPersonality, banner.iconDeoxysForm);
        // In front of BG0, for the panel not to cover it
        gSprites[sIconSpriteId].oam.priority = 0;
    }
    return TRUE;
}

static void EraseBanner(void)
{
    StopBannerBlend();
    if (sIconSpriteId != SPRITE_NONE)
    {
        FreeAndDestroyMonIconSprite(&gSprites[sIconSpriteId]);
        FreeMonIconPalette(sIconSpecies);
        sIconSpriteId = SPRITE_NONE;
    }

    // The window itself is kept for the next banner
    if (sBannerWindowId != WINDOW_NONE)
    {
        ClearWindowTilemap(sBannerWindowId);
        CopyWindowToVram(sBannerWindowId, COPYWIN_MAP);
    }
}

static struct Pokemon *GetNotificationMon(const struct FieldNotification *notif)
{
    u32 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY) == notif->id
         && GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) != SPECIES_NONE)
            return &gPlayerParty[i];
    }
    return NULL;
}

// The Pokémon's icon, with its nickname for a title
static void SetBannerMon(struct Banner *banner, struct Pokemon *mon)
{
    banner->iconSpecies = GetMonData(mon, MON_DATA_SPECIES);
    banner->iconPersonality = GetMonData(mon, MON_DATA_PERSONALITY);
    banner->iconDeoxysForm = GetMonDeoxysForm(mon);
    GetMonData(mon, MON_DATA_NICKNAME, banner->title);
    StringGet_Nickname(banner->title);
}

// arg: the rerolls the chain now gives
static bool8 FillBanner_ChainReroll(const struct FieldNotification *notif, struct Banner *banner)
{
    u8 *text;

    banner->iconSpecies = ReadChainSpecies();
    text = StringCopy(banner->text, sText_Plus);
    text = ConvertIntToDecimalStringN(text, notif->arg, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringCopy(text, sText_Rerolls);
    banner->color = BANNER_RED;
    PlayFanfare(MUS_RG_PHOTO);
    return TRUE;
}

static bool8 FillBanner_NewMoves(const struct FieldNotification *notif, struct Banner *banner)
{
    struct Pokemon *mon = GetNotificationMon(notif);

    if (mon == NULL)
        return FALSE;

    SetBannerMon(banner, mon);
    StringCopy(banner->text, sText_NewMoves);
    banner->color = BANNER_BLUE;
    PlaySE(SE_EXP_MAX);
    return TRUE;
}

// arg: the item found
static bool8 FillBanner_PickupItem(const struct FieldNotification *notif, struct Banner *banner)
{
    struct Pokemon *mon = GetNotificationMon(notif);

    // Nothing to tell once the item has been taken from it
    if (mon == NULL || GetMonData(mon, MON_DATA_HELD_ITEM) != notif->arg)
        return FALSE;

    SetBannerMon(banner, mon);
    CopyItemName(notif->arg, banner->text);
    banner->color = BANNER_GREEN;
    PlayCry_Normal(banner->iconSpecies, 0);
    return TRUE;
}

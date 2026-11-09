#include "global.h"
#include "wall_calendar.h"
#include "main.h"
#include "menu.h"
#include "overworld.h"
#include "palette.h"
#include "script.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text.h"
#include "time_events.h"
#include "wallclock.h"
#include "window.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// The wall calendar in the player's and rival's houses. Like the wall clock, it
// fades to its own screen (on the clock's background) showing the whole year:
//
//                  YEAR 1
//          Mon Tue Wed Thu Fri Sat Sun
//  Spring   1   2   3   4   5   6   7
//  Summer   1   2   3   4   5   6   7
//  Fall     1   2   3   4   5   6   7
//  Winter   1   2   3   4   5   6   7
//
// with the current season and today's day highlighted.

#define CAL_WIDTH_TILES  24
#define CAL_HEIGHT_TILES 12
#define CAL_LABEL_X      6
#define CAL_COLUMN_X     44
#define CAL_COLUMN_WIDTH 20
#define CAL_TITLE_Y      1
#define CAL_WEEKDAY_Y    18
#define CAL_DIVIDER_Y    31
#define CAL_FIRST_ROW_Y  34
// Taller than a FONT_SMALL glyph (13px incl. shadow), so a highlighted cell covers its whole glyph
#define CAL_ROW_HEIGHT   15

// The clock screen's user window frame (see LoadWallClockGraphics)
#define CAL_FRAME_TILE   0x250
#define CAL_FRAME_PAL    13

// Indexes into the overworld text window palette (gMessageBox_Pal)
#define CAL_COLOR_BG            TEXT_COLOR_WHITE
#define CAL_COLOR_ALT_COLUMN    11
#define CAL_COLOR_TEXT          TEXT_COLOR_DARK_GRAY
#define CAL_COLOR_SHADOW        TEXT_COLOR_LIGHT_GRAY
#define CAL_COLOR_TODAY         TEXT_COLOR_BLUE
#define CAL_COLOR_TODAY_TEXT    TEXT_COLOR_WHITE
#define CAL_COLOR_SEASON        TEXT_COLOR_BLUE
#define CAL_COLOR_SEASON_SHADOW TEXT_COLOR_LIGHT_BLUE

static void CB2_ViewWallCalendar(void);
static void Task_WallCalendar_WaitFadeIn(u8 taskId);
static void Task_WallCalendar_HandleInput(u8 taskId);
static void Task_WallCalendar_Exit(u8 taskId);

static const struct WindowTemplate sWallCalendarWindowTemplate =
{
    .bg = 0,
    .tilemapLeft = (DISPLAY_TILE_WIDTH - CAL_WIDTH_TILES) / 2,
    .tilemapTop = 2,
    .width = CAL_WIDTH_TILES,
    .height = CAL_HEIGHT_TILES,
    .paletteNum = 14,
    // BG0 tiles 48-59 share VRAM with the clock screen's BG2 button label
    .baseBlock = 64,
};

static const u8 sText_Year[] = _("YEAR ");

static void PrintCalendarText(u8 windowId, u8 font, u8 bg, u8 fg, u8 shadow, s32 x, s32 y, const u8 *str)
{
    u8 colors[3] = {bg, fg, shadow};

    AddTextPrinterParameterized3(windowId, font, x, y, colors, TEXT_SKIP_DRAW, str);
}

static void PrintCentered(u8 windowId, u8 font, u8 bg, u8 fg, u8 shadow, s32 x, s32 width, s32 y, const u8 *str)
{
    PrintCalendarText(windowId, font, bg, fg, shadow, x + (width - (s32)GetStringWidth(font, str, 0)) / 2, y, str);
}

static void DrawWallCalendar(u8 windowId)
{
    struct CalendarDate date;
    u8 text[16];
    s32 season, day, x, y;
    s32 windowWidth = CAL_WIDTH_TILES * 8;
    s32 columnsHeight = CAL_HEIGHT_TILES * 8 - (CAL_WEEKDAY_Y - 1);

    GetCalendarDate(&date);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(CAL_COLOR_BG));

    ConvertIntToDecimalStringN(StringCopy(text, sText_Year), date.year, STR_CONV_MODE_LEFT_ALIGN, 5);
    PrintCentered(windowId, FONT_NORMAL, CAL_COLOR_BG, CAL_COLOR_TEXT, CAL_COLOR_SHADOW, 0, windowWidth, CAL_TITLE_Y, text);

    for (day = 0; day < DAYS_PER_WEEK; day++)
    {
        u8 bg = (day % 2) ? CAL_COLOR_ALT_COLUMN : CAL_COLOR_BG;

        x = CAL_COLUMN_X + day * CAL_COLUMN_WIDTH;
        FillWindowPixelRect(windowId, PIXEL_FILL(bg), x, CAL_WEEKDAY_Y - 1, CAL_COLUMN_WIDTH, columnsHeight);
        PrintCentered(windowId, FONT_SMALL, bg, CAL_COLOR_TEXT, CAL_COLOR_SHADOW, x, CAL_COLUMN_WIDTH, CAL_WEEKDAY_Y, GetDayOfWeekName(day));

        for (season = 0; season < SEASONS_PER_YEAR; season++)
        {
            u8 cellBg = bg, fg = CAL_COLOR_TEXT, shadow = CAL_COLOR_SHADOW;

            y = CAL_FIRST_ROW_Y + season * CAL_ROW_HEIGHT;
            if (season == date.season && day == date.dayOfWeek)
            {
                cellBg = CAL_COLOR_TODAY;
                fg = CAL_COLOR_TODAY_TEXT;
                shadow = CAL_COLOR_TODAY;
                FillWindowPixelRect(windowId, PIXEL_FILL(cellBg), x, y - 1, CAL_COLUMN_WIDTH, CAL_ROW_HEIGHT);
            }
            ConvertIntToDecimalStringN(text, day + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
            PrintCentered(windowId, FONT_SMALL, cellBg, fg, shadow, x, CAL_COLUMN_WIDTH, y, text);
        }
    }

    FillWindowPixelRect(windowId, PIXEL_FILL(CAL_COLOR_SHADOW), 2, CAL_DIVIDER_Y, windowWidth - 4, 1);

    for (season = 0; season < SEASONS_PER_YEAR; season++)
    {
        y = CAL_FIRST_ROW_Y + season * CAL_ROW_HEIGHT;
        if (season == date.season)
            PrintCalendarText(windowId, FONT_SMALL, CAL_COLOR_BG, CAL_COLOR_SEASON, CAL_COLOR_SEASON_SHADOW, CAL_LABEL_X, y, GetSeasonName(season));
        else
            PrintCalendarText(windowId, FONT_SMALL, CAL_COLOR_BG, CAL_COLOR_TEXT, CAL_COLOR_SHADOW, CAL_LABEL_X, y, GetSeasonName(season));
    }
}

// gSpecialVar_0x8004 (MALE/FEMALE) picks the background color, as for the wall clock
void ShowWallCalendar(void)
{
    gMain.savedCallback = CB2_ReturnToField;
    SetMainCallback2(CB2_ViewWallCalendar);
    LockPlayerFieldControls();
}

static void CB2_ViewWallCalendar(void)
{
    u8 windowId;

    LoadWallClockBackground(gText_Cancel4);

    windowId = AddWindow(&sWallCalendarWindowTemplate);
    DrawStdFrameWithCustomTileAndPalette(windowId, FALSE, CAL_FRAME_TILE, CAL_FRAME_PAL);
    DrawWallCalendar(windowId);
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_FULL);

    CreateTask(Task_WallCalendar_WaitFadeIn, 0);
}

static void Task_WallCalendar_WaitFadeIn(u8 taskId)
{
    if (!gPaletteFade.active)
        gTasks[taskId].func = Task_WallCalendar_HandleInput;
}

static void Task_WallCalendar_HandleInput(u8 taskId)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_WallCalendar_Exit;
    }
}

static void Task_WallCalendar_Exit(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAllWindowBuffers();
        SetMainCallback2(gMain.savedCallback);
    }
}

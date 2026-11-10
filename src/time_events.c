#include "global.h"
#include "time_events.h"
#include "event_data.h"
#include "field_weather.h"
#include "pokemon.h"
#include "random.h"
#include "overworld.h"
#include "rtc.h"
#include "script.h"
#include "task.h"

static u32 GetMirageRnd(void)
{
    u32 hi = VarGet(VAR_MIRAGE_RND_H);
    u32 lo = VarGet(VAR_MIRAGE_RND_L);
    return (hi << 16) | lo;
}

static void SetMirageRnd(u32 rnd)
{
    VarSet(VAR_MIRAGE_RND_H, rnd >> 16);
    VarSet(VAR_MIRAGE_RND_L, rnd);
}

// unused
void InitMirageRnd(void)
{
    SetMirageRnd((Random() << 16) | Random());
}

void UpdateMirageRnd(u16 days)
{
    s32 rnd = GetMirageRnd();
    while (days)
    {
        rnd = ISO_RANDOMIZE2(rnd);
        days--;
    }
    SetMirageRnd(rnd);
}

bool8 IsMirageIslandPresent(void)
{
    u16 rnd = GetMirageRnd() >> 16;
    int i;

    for (i = 0; i < PARTY_SIZE; i++)
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES) && (GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY) & 0xFFFF) == rnd)
            return TRUE;

    return FALSE;
}

void UpdateShoalTideFlag(void)
{
    static const u8 tide[] =
    {
        1, // 00
        1, // 01
        1, // 02
        0, // 03
        0, // 04
        0, // 05
        0, // 06
        0, // 07
        0, // 08
        1, // 09
        1, // 10
        1, // 11
        1, // 12
        1, // 13
        1, // 14
        0, // 15
        0, // 16
        0, // 17
        0, // 18
        0, // 19
        0, // 20
        1, // 21
        1, // 22
        1, // 23
    };

    if (IsMapTypeOutdoors(GetLastUsedWarpMapType()))
    {
        RtcCalcLocalTime();
        if (tide[gLocalTime.hours])
            FlagSet(FLAG_SYS_SHOAL_TIDE);
        else
            FlagClear(FLAG_SYS_SHOAL_TIDE);
    }
}

static void Task_WaitWeather(u8 taskId)
{
    if (IsWeatherChangeComplete())
    {
        ScriptContext_Enable();
        DestroyTask(taskId);
    }
}

void WaitWeather(void)
{
    CreateTask(Task_WaitWeather, 80);
}

void InitBirchState(void)
{
    // Initialize VAR_BIRCH_STATE to 0 to start the roaming cycle
    VarSet(VAR_BIRCH_STATE, 0);
}

void UpdateBirchState(u16 days)
{
    u16 *state = GetVarPointer(VAR_BIRCH_STATE);
    // Increment by 1 each day update, regardless of how many days passed
    if (days > 0)
    {
        *state += 1;
        *state %= 7;
    }
}

void InitBirchStateRandom(void)
{
    // Initialize VAR_BIRCH_STATE_RANDOM to 0
    VarSet(VAR_BIRCH_STATE_RANDOM, 0);
}

void UpdateBirchStateRandom(u16 days)
{
    // Set to a random value 0-6 each day
    if (days > 0)
    {
        VarSet(VAR_BIRCH_STATE_RANDOM, Random() % 7);
    }
}

// ---------------------------------------------------------------------------
// Calendar / seasons
// ---------------------------------------------------------------------------
// VAR_BIRCH_STATE is the day of the week (0 = Monday .. 6 = Sunday), advanced
// once per in-game day by UpdateBirchState. Each season is one week long, so
// when it wraps from Sunday back to Monday, VAR_SEASON advances
// (SPRING -> SUMMER -> FALL -> WINTER), and leaving WINTER starts a new year.
// VAR_SEASON_PREV_DAY caches the previous day of the week so the wrap is still
// detected if VAR_BIRCH_STATE is changed by something else (e.g. a debug NPC).

static const u8 sText_Monday[]    = _("Mon");
static const u8 sText_Tuesday[]   = _("Tue");
static const u8 sText_Wednesday[] = _("Wed");
static const u8 sText_Thursday[]  = _("Thu");
static const u8 sText_Friday[]    = _("Fri");
static const u8 sText_Saturday[]  = _("Sat");
static const u8 sText_Sunday[]    = _("Sun");

static const u8 *const sDayOfWeekNames[DAYS_PER_WEEK] =
{
    [WEEKDAY_MONDAY]    = sText_Monday,
    [WEEKDAY_TUESDAY]   = sText_Tuesday,
    [WEEKDAY_WEDNESDAY] = sText_Wednesday,
    [WEEKDAY_THURSDAY]  = sText_Thursday,
    [WEEKDAY_FRIDAY]    = sText_Friday,
    [WEEKDAY_SATURDAY]  = sText_Saturday,
    [WEEKDAY_SUNDAY]    = sText_Sunday,
};

static const u8 sText_Spring[] = _("Spring");
static const u8 sText_Summer[] = _("Summer");
static const u8 sText_Fall[]   = _("Fall");
static const u8 sText_Winter[] = _("Winter");

static const u8 *const sSeasonNames[SEASONS_PER_YEAR] =
{
    [SEASON_SPRING] = sText_Spring,
    [SEASON_SUMMER] = sText_Summer,
    [SEASON_FALL]   = sText_Fall,
    [SEASON_WINTER] = sText_Winter,
};

void InitCalendar(void)
{
    VarSet(VAR_SEASON, SEASON_SPRING);
    VarSet(VAR_YEAR, CALENDAR_FIRST_YEAR);
    VarSet(VAR_SEASON_PREV_DAY, GetCurrentDayOfWeek());
}

static void AdvanceCalendarSeason(struct CalendarDate *date)
{
    if (++date->season >= SEASONS_PER_YEAR)
    {
        date->season = SEASON_SPRING;
        if (date->year < 0xFFFF)
            date->year++;
    }
}

void UpdateCalendar(u16 days)
{
    struct CalendarDate date;

    if (days == 0)
        return;

    date.dayOfWeek = GetCurrentDayOfWeek();
    date.season = GetCurrentSeason();
    date.year = GetCurrentYear();
    if (date.dayOfWeek < VarGet(VAR_SEASON_PREV_DAY))
    {
        AdvanceCalendarSeason(&date);
        VarSet(VAR_SEASON, date.season);
        VarSet(VAR_YEAR, date.year);
    }
    VarSet(VAR_SEASON_PREV_DAY, date.dayOfWeek);
}

u16 GetCurrentDayOfWeek(void)
{
    return VarGet(VAR_BIRCH_STATE) % DAYS_PER_WEEK;
}

u16 GetCurrentSeason(void)
{
    return VarGet(VAR_SEASON) % SEASONS_PER_YEAR;
}

// seasonTags is a mask of SEASON_TAG_* values; 0 means every season.
bool8 IsInSeason(u8 seasonTags)
{
    return seasonTags == 0 || (seasonTags & (1 << GetCurrentSeason())) != 0;
}

u16 GetCurrentYear(void)
{
    u16 year = VarGet(VAR_YEAR);

    if (year < CALENDAR_FIRST_YEAR)
        year = CALENDAR_FIRST_YEAR;
    return year;
}

// Like the GetCurrent* functions, but also applies a day change that the
// in-game clock has reached and DoTimeBasedEvents hasn't processed yet (it
// only runs in the field, outside Pokemon Centers). Used by UI such as the
// PokeNav so the date always agrees with the clock shown next to it.
void GetCalendarDate(struct CalendarDate *date)
{
    date->dayOfWeek = GetCurrentDayOfWeek();
    date->season = GetCurrentSeason();
    date->year = GetCurrentYear();

    RtcCalcLocalTime();
    if (FlagGet(FLAG_SYS_CLOCK_SET) && (s32)VarGet(VAR_DAYS) < gLocalTime.days)
    {
        // Mirrors UpdateBirchState + UpdateCalendar: one step, however many days passed.
        date->dayOfWeek = (date->dayOfWeek + 1) % DAYS_PER_WEEK;
        if (date->dayOfWeek < VarGet(VAR_SEASON_PREV_DAY))
            AdvanceCalendarSeason(date);
    }
}

const u8 *GetDayOfWeekName(u8 dayOfWeek)
{
    return sDayOfWeekNames[dayOfWeek % DAYS_PER_WEEK];
}

const u8 *GetSeasonName(u8 season)
{
    return sSeasonNames[season % SEASONS_PER_YEAR];
}

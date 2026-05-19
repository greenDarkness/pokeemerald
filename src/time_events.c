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
// VAR_BIRCH_STATE doubles as the "day of season" counter (0..6, advanced once
// per in-game day by UpdateBirchState). When it wraps from 6 -> 0 a new week
// has started, so we advance VAR_SEASON (SPRING -> SUMMER -> FALL -> WINTER
// -> SPRING). The year is implicit: 4 seasons * 7 days = 28-day cycle that
// loops forever. VAR_SEASON_PREV_DAY caches the previous day-of-season so we
// can detect the wrap robustly even if VAR_BIRCH_STATE was just initialized
// or the player skipped days.

void InitSeason(void)
{
    VarSet(VAR_SEASON, SEASON_SPRING);
    VarSet(VAR_SEASON_PREV_DAY, VarGet(VAR_BIRCH_STATE));
}

void UpdateSeason(u16 days)
{
    u16 *season;
    u16 birch;
    u16 prevDay;

    if (days == 0)
        return;

    birch = VarGet(VAR_BIRCH_STATE);
    prevDay = VarGet(VAR_SEASON_PREV_DAY);
    // Detect that we just crossed a week boundary. The "prev == 6, now == 0"
    // case covers a single-day step; treating any decrease as a wrap covers
    // multi-day skips where UpdateBirchState's modulo lands us anywhere from
    // 0..prev-1.
    if (birch < prevDay)
    {
        season = GetVarPointer(VAR_SEASON);
        *season = (*season + 1) & (SEASONS_PER_YEAR - 1);
    }
    VarSet(VAR_SEASON_PREV_DAY, birch);
}

u16 GetCurrentSeason(void)
{
    return VarGet(VAR_SEASON);
}

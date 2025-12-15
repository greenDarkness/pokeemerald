#include "global.h"

#include "rtc.h"

// iwram common
COMMON_DATA struct Time gLocalTime = {0};

struct Time *GetFakeRtc(void)
{
    return &gSaveBlock2Ptr->fakeRTC;
}

void RtcAdvanceTime(struct Time *time)
{
    struct Time *fakeRtc = GetFakeRtc();
    fakeRtc->seconds += time->seconds;
    fakeRtc->minutes += time->minutes;
    fakeRtc->hours += time->hours;
    fakeRtc->days += time->days;

    // Normalize seconds
    while (fakeRtc->seconds >= SECONDS_PER_MINUTE)
    {
        fakeRtc->seconds -= SECONDS_PER_MINUTE;
        fakeRtc->minutes++;
    }

    // Normalize minutes
    while (fakeRtc->minutes >= MINUTES_PER_HOUR)
    {
        fakeRtc->minutes -= MINUTES_PER_HOUR;
        fakeRtc->hours++;
    }

    // Normalize hours
    while (fakeRtc->hours >= HOURS_PER_DAY)
    {
        fakeRtc->hours -= HOURS_PER_DAY;
        fakeRtc->days++;
    }
}

void RtcAdvanceTimeTo(struct Time *time)
{
    struct Time *fakeRtc = GetFakeRtc();
    *fakeRtc = *time;
}

void RtcCalcLocalTime(void)
{
    struct Time *fakeRtc = GetFakeRtc();
    gLocalTime = *fakeRtc;
}

void RtcInitLocalTimeOffset(s32 hour, s32 minute)
{
    RtcCalcLocalTimeOffset(0, hour, minute, 0);
}

void RtcCalcLocalTimeOffset(s32 days, s32 hours, s32 minutes, s32 seconds)
{
    struct Time *fakeRtc = GetFakeRtc();
    fakeRtc->days = days;
    fakeRtc->hours = hours;
    fakeRtc->minutes = minutes;
    fakeRtc->seconds = seconds;
}

u32 RtcGetMinuteCount(void)
{
    struct Time *fakeRtc = GetFakeRtc();
    return (HOURS_PER_DAY * MINUTES_PER_HOUR) * fakeRtc->days + MINUTES_PER_HOUR * fakeRtc->hours + fakeRtc->minutes;
}

u32 RtcGetLocalDayCount(void)
{
    struct Time *fakeRtc = GetFakeRtc();
    return fakeRtc->days;
}

// Dummy implementations for compatibility
void RtcInit(void)
{
    // Do nothing
}

u16 RtcGetErrorStatus(void)
{
    return 0;
}

void RtcReset(void)
{
    // Do nothing
}

void CalcTimeDifference(struct Time *result, struct Time *t1, struct Time *t2)
{
    result->seconds = t2->seconds - t1->seconds;
    result->minutes = t2->minutes - t1->minutes;
    result->hours = t2->hours - t1->hours;
    result->days = t2->days - t1->days;

    if (result->seconds < 0)
    {
        result->seconds += SECONDS_PER_MINUTE;
        --result->minutes;
    }

    if (result->minutes < 0)
    {
        result->minutes += MINUTES_PER_HOUR;
        --result->hours;
    }

    if (result->hours < 0)
    {
        result->hours += HOURS_PER_DAY;
        --result->days;
    }
}

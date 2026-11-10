#ifndef GUARD_TIME_EVENTS_H
#define GUARD_TIME_EVENTS_H

// Calendar: VAR_BIRCH_STATE is the day of the week, each season lasts one
// week (VAR_SEASON), and four seasons make a 28-day year (VAR_YEAR).
#define WEEKDAY_MONDAY      0
#define WEEKDAY_TUESDAY     1
#define WEEKDAY_WEDNESDAY   2
#define WEEKDAY_THURSDAY    3
#define WEEKDAY_FRIDAY      4
#define WEEKDAY_SATURDAY    5
#define WEEKDAY_SUNDAY      6
#define DAYS_PER_WEEK       7

#define SEASON_SPRING       0
#define SEASON_SUMMER       1
#define SEASON_FALL         2
#define SEASON_WINTER       3
#define SEASONS_PER_YEAR    4
#define DAYS_PER_SEASON     DAYS_PER_WEEK

// Season tags for data tables (swarms in src/tv.c, grottos in src/data/hidden_grottos.h):
// .seasons = SEASON_TAG_SPRING | SEASON_TAG_FALL. Leaving .seasons out (0) means every season.
#define SEASON_TAG_SPRING   (1 << SEASON_SPRING)
#define SEASON_TAG_SUMMER   (1 << SEASON_SUMMER)
#define SEASON_TAG_FALL     (1 << SEASON_FALL)
#define SEASON_TAG_WINTER   (1 << SEASON_WINTER)
#define SEASON_TAG_ALL      (SEASON_TAG_SPRING | SEASON_TAG_SUMMER | SEASON_TAG_FALL | SEASON_TAG_WINTER)

#define CALENDAR_FIRST_YEAR 1

struct CalendarDate
{
    u8 dayOfWeek;
    u8 season;
    u16 year;
};

void UpdateMirageRnd(u16 days);
bool8 IsMirageIslandPresent(void);
void UpdateBirchState(u16 days);
void InitBirchStateRandom(void);
void UpdateBirchStateRandom(u16 days);
void InitCalendar(void);
void UpdateCalendar(u16 days);
u16 GetCurrentDayOfWeek(void);
u16 GetCurrentSeason(void);
bool8 IsInSeason(u8 seasonTags);
u16 GetCurrentYear(void);
void GetCalendarDate(struct CalendarDate *date);
const u8 *GetDayOfWeekName(u8 dayOfWeek);
const u8 *GetSeasonName(u8 season);

#endif // GUARD_TIME_EVENTS_H

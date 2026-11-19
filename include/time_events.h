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

// Day tags, for the days of a WEATHER(weather, days) in src/data/seasons.h:
// DAY_TAG_SATURDAY | DAY_TAG_SUNDAY. 0 means every day.
// A season starts on Monday, so DAY_TAG_MONDAY is the season's 1st day and DAY_TAG_SUNDAY its 7th.
#define DAY_TAG_MONDAY      (1 << WEEKDAY_MONDAY)
#define DAY_TAG_TUESDAY     (1 << WEEKDAY_TUESDAY)
#define DAY_TAG_WEDNESDAY   (1 << WEEKDAY_WEDNESDAY)
#define DAY_TAG_THURSDAY    (1 << WEEKDAY_THURSDAY)
#define DAY_TAG_FRIDAY      (1 << WEEKDAY_FRIDAY)
#define DAY_TAG_SATURDAY    (1 << WEEKDAY_SATURDAY)
#define DAY_TAG_SUNDAY      (1 << WEEKDAY_SUNDAY)
#define DAY_TAG_WEEKDAYS    (DAY_TAG_MONDAY | DAY_TAG_TUESDAY | DAY_TAG_WEDNESDAY | DAY_TAG_THURSDAY | DAY_TAG_FRIDAY)
#define DAY_TAG_WEEKEND     (DAY_TAG_SATURDAY | DAY_TAG_SUNDAY)
#define DAY_TAG_ALL         (DAY_TAG_WEEKDAYS | DAY_TAG_WEEKEND)

// Day numbers, as another way to write day tags: DAYS(1-3, 7) is days 1 to 3 and day 7
// of the season (1 = Monday .. 7 = Sunday). Each item is a single day or a first-last range, up
// to 7 items. The result is a DAY_TAG_* mask, so it can be joined with day tags using |.
// A day outside 1-7 or a backwards range (3-1) fails to compile with a negative array size error.
#define DAYS(...)           (CAT(DAYS_, NARG_8(__VA_ARGS__))(__VA_ARGS__))

// The item is left unparenthesized on purpose: 100 * 4 is 400 and 100 * 1-3 is 97, so the first
// day is the value rounded up to a multiple of 100 and the last day is how far below that it is
// (0 for a single day).
#define DAYS_VALUE_(item)   (100 * item)
#define DAYS_FIRST_(item)   ((DAYS_VALUE_(item) + 99) / 100)
#define DAYS_BELOW_(item)   (DAYS_FIRST_(item) * 100 - DAYS_VALUE_(item))
#define DAYS_LAST_(item)    (DAYS_BELOW_(item) ? DAYS_BELOW_(item) : DAYS_FIRST_(item))
#define DAYS_VALID_(item)   (DAYS_FIRST_(item) >= 1 && DAYS_FIRST_(item) <= DAYS_LAST_(item) && DAYS_LAST_(item) <= DAYS_PER_WEEK)
#define DAYS_MASK_(item)    (((1 << DAYS_LAST_(item)) - (1 << (DAYS_FIRST_(item) - 1))) * (int)sizeof(char[DAYS_VALID_(item) ? 1 : -1]))
#define DAYS_1(a)                   DAYS_MASK_(a)
#define DAYS_2(a, b)                DAYS_MASK_(a) | DAYS_1(b)
#define DAYS_3(a, b, c)             DAYS_MASK_(a) | DAYS_2(b, c)
#define DAYS_4(a, b, c, d)          DAYS_MASK_(a) | DAYS_3(b, c, d)
#define DAYS_5(a, b, c, d, e)       DAYS_MASK_(a) | DAYS_4(b, c, d, e)
#define DAYS_6(a, b, c, d, e, f)    DAYS_MASK_(a) | DAYS_5(b, c, d, e, f)
#define DAYS_7(a, b, c, d, e, f, g) DAYS_MASK_(a) | DAYS_6(b, c, d, e, f, g)

// Year tags, used by the EVEN_YEARS(...) and ODD_YEARS(...) blocks in src/data/seasons.h.
// The calendar starts in year 1, so the first year is an odd one.
#define YEAR_TAG_EVEN       (1 << 0)
#define YEAR_TAG_ODD        (1 << 1)
#define YEAR_TAG_ALL        (YEAR_TAG_EVEN | YEAR_TAG_ODD)

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
bool8 IsOnDay(u8 dayTags);
u16 GetCurrentYear(void);
bool8 IsInYear(u8 yearTags);
void GetCalendarDate(struct CalendarDate *date);
const u8 *GetDayOfWeekName(u8 dayOfWeek);
const u8 *GetSeasonName(u8 season);

#endif // GUARD_TIME_EVENTS_H

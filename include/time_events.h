#ifndef GUARD_TIME_EVENTS_H
#define GUARD_TIME_EVENTS_H

// Calendar seasons (28-day year, 7 days per season).
#define SEASON_SPRING   0
#define SEASON_SUMMER   1
#define SEASON_FALL     2
#define SEASON_WINTER   3
#define SEASONS_PER_YEAR 4
#define DAYS_PER_SEASON  7

void UpdateMirageRnd(u16 days);
bool8 IsMirageIslandPresent(void);
void UpdateBirchState(u16 days);
void InitBirchStateRandom(void);
void UpdateBirchStateRandom(u16 days);
void InitSeason(void);
void UpdateSeason(u16 days);
u16 GetCurrentSeason(void);

#endif // GUARD_TIME_EVENTS_H

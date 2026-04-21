#ifndef GUARD_PLAY_TIME_H
#define GUARD_PLAY_TIME_H

// Number of real-time minutes that make up one in-game day (24 game hours).
// Change this to adjust how fast the in-game clock advances.
// Must be a divisor of 1440 so that game-seconds-per-real-second is an integer.
// Examples: 24 (vanilla-like 1 game min/real sec), 30, 48, 60, 72, 90, 120, ...
#define REAL_MINUTES_PER_GAME_DAY 30

// Derived: how many in-game seconds elapse for each real second.
// 24 game hours = 1440 game minutes = 86400 game seconds per game day.
#define GAME_SECONDS_PER_REAL_SECOND (86400 / (REAL_MINUTES_PER_GAME_DAY * 60))

void PlayTimeCounter_Reset(void);
void PlayTimeCounter_Start(void);
void PlayTimeCounter_Stop(void);
void PlayTimeCounter_Update(void);
void PlayTimeCounter_SetToMax(void);

#endif // GUARD_PLAY_TIME_H

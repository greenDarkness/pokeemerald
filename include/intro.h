#ifndef GUARD_INTRO_H
#define GUARD_INTRO_H

// Exported type declarations

// Number of (intro, title screen) pairs in the random pool.
#define INTRO_PAIR_COUNT 3

// Exported RAM declarations
extern u8 gIntroPairIndex;

// Exported ROM declarations
void CB2_InitCopyrightScreenAfterBootup(void);
void CB2_InitCopyrightScreenAfterTitleScreen(void);
void PanFadeAndZoomScreen(u16 screenX, u16 screenY, u16 zoom, u16 alpha);

// FRLG intro entry point (pair 1)
void StartFrlgIntro(void);

#endif // GUARD_INTRO_H

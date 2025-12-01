#ifndef GUARD_CATCH_MINIGAME_H
#define GUARD_CATCH_MINIGAME_H

void CatchMinigame_DrawTestIcons(void);
void CatchMinigame_Update(void);
bool8 CatchMinigame_AreIconsVisible(void);
bool8 CatchMinigame_IsTimeUp(void);
bool8 CatchMinigame_WasWon(void);
bool8 CatchMinigame_WasFailed(void);
u8 CatchMinigame_GetBonus(void);
void CatchMinigame_ResetWinState(void);
void CatchMinigame_HideIcons(void);

#endif // GUARD_CATCH_MINIGAME_H

#ifndef GUARD_OVERRIDES_H
#define GUARD_OVERRIDES_H

// Data for all of these lives in src/data/overrides.h.
void SetCaughtMonMetData(struct Pokemon *mon);
void TrySetHatchedMet(struct Pokemon *mon);
bool8 TryReassignFatefulPid(struct Pokemon *mon, u8 nature, u8 abilityNum, u8 gender);
void GiveOddEgg(void);
bool8 TryApplyCustomWildMonIVs(u16 species, struct Pokemon *mon);

#endif // GUARD_OVERRIDES_H

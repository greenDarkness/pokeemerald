#ifndef GUARD_OVERRIDES_H
#define GUARD_OVERRIDES_H

// Data for all of these lives in src/data/overrides.h.
void SetCaughtMonMetData(struct Pokemon *mon);
void TrySetHatchedMet(struct Pokemon *mon);
bool8 DoesCaughtMonGetOverrideName(struct Pokemon *mon);
bool32 IsColosseumTrainerId(u16 trainerId, u16 secretId);
void GetMonIVs(struct Pokemon *mon, u8 *ivs);

// TryChangeMonPid flags
#define PID_CHANGE_FLAG_TOGGLE_SHINY (1 << 0)
#define PID_CHANGE_FLAG_FORCE        (1 << 1)

enum
{
    PID_EDIT_NATURE,
    PID_EDIT_GENDER,
    PID_EDIT_ABILITY,
    PID_EDIT_IV,
};

// A change one of the NPCs makes
struct PidEdit
{
    u8 type;  // PID_EDIT_*
    u8 stat;  // STAT_*, for PID_EDIT_IV
    u8 value; // the nature, gender, ability number or IV
};

#define MAX_PID_IV_FIXES 3

// Other edits that would make an impossible change legal (FindPidChangeSuggestions)
struct PidSuggestions
{
    u8 sameStatCount;          // all IVs were 31 and one is being changed: up to 3 other values (0-30) of that
    u8 sameStatValues[3];      //   IV that would, closest to the asked value first
    u8 sameStat;
    u32 natures;               // bit n: nature n would on its own
    bool8 otherGender;         // switching gender would
    bool8 otherAbility;        // switching ability would
    u8 ivFixCount;             // the cheapest change of up to MAX_PID_IV_FIXES other IVs that would (0 = none)
    u8 comboCount;             // if none of the above would: the cheapest two edits of any kind (0 = none)
    struct PidEdit ivFix[MAX_PID_IV_FIXES];
    struct PidEdit combo[2];
};

u8 TryChangeMonPid(struct Pokemon *mon, u8 nature, u8 gender, u8 abilityNum, const u8 *ivs, u8 flags);
bool8 FindPidChangeSuggestions(struct Pokemon *mon, const struct PidEdit *wanted, struct PidSuggestions *suggestions);
void BufferPidChangeSuggestions(struct Pokemon *mon, const struct PidSuggestions *suggestions);
void GiveOddEgg(void);
bool8 TryApplyCustomWildMonIVs(u16 species, struct Pokemon *mon);

#endif // GUARD_OVERRIDES_H

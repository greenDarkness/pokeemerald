#ifndef GUARD_EGG_HATCH_H
#define GUARD_EGG_HATCH_H

void ScriptHatchMon(void);
bool8 CheckDaycareMonReceivedMail(void);
void EggHatch(void);
u8 GetEggCyclesToSubtract(void);
u16 CountPartyAliveNonEggMons(void);
void GetEggColoringTypes(u16 hatchedSpecies, u8 *outPrimary, u8 *outSecondary);
u16 GetEggTypeColor(u8 type);

#endif // GUARD_EGG_HATCH_H

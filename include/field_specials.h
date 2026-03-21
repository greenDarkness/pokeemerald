#ifndef GUARD_FIELD_SPECIALS_H
#define GUARD_FIELD_SPECIALS_H

extern bool8 gBikeCyclingChallenge;
extern u8 gBikeCollisions;

u8 GetLeadMonIndex(void);
bool8 IsDestinationBoxFull(void);
u16 GetPCBoxToSendMon(void);
bool8 InMultiPartnerRoom(void);
void UpdateTrainerFansAfterLinkBattle(void);
void IncrementBirthIslandRockStepCount(void);
bool8 AbnormalWeatherHasExpired(void);
bool8 ShouldDoBrailleRegicePuzzle(void);
bool32 ShouldDoWallyCall(void);
bool32 ShouldDoScottFortreeCall(void);
bool32 ShouldDoScottBattleFrontierCall(void);
bool32 ShouldDoMrStoneCall(void);
bool32 ShouldDoRoxanneCall(void);
bool32 ShouldDoRivalRayquazaCall(void);
bool32 CountSSTidalStep(u16 delta);
u8 GetSSTidalLocation(s8 *mapGroup, s8 *mapNum, s16 *x, s16 *y);
void ShowScrollableMultichoice(void);
void FrontierGamblerSetWonOrLost(bool8 won);
u8 TryGainNewFanFromCounter(u8 incrementId);
bool8 InPokemonCenter(void);
void SetShoalItemFlag(u16 unused);
void UpdateFrontierManiac(u16 daysSince);
void UpdateFrontierGambler(u16 daysSince);
void ResetCyclingRoadChallengeData(void);
bool8 UsedPokemonCenterWarp(void);
void ResetFanClub(void);
bool8 ShouldShowBoxWasFullMessage(void);
void SetPCBoxToSendMon(u8 boxId);
void GiveRandomPerfectIVEgg(void);
void GiveRandomHealingItem(void);
bool8 TryRegeneratePP(void);
void UpdateDaycareGirlEggCounter(void);
void UpdatePotionBoyCounter(void);
void TryFieldPickup(void);
void ChooseMonForGenderChange(void);
void ChangePokemonGender(void);
void ChooseMonForAbilitySwap(void);
void SwapPokemonAbility(void);
void SetCutTreePermanentFlag(void);
void GetCutTreePermanentFlag(void);
void InvalidateCachedPartyWeakTypes(void);
bool8 AreCachedPartyWeakTypesValid(void);
u8 GetCachedWeakTypeCount(void);
u8 GetCachedWeakType(u8 index);
u16 GetReleaseMonSpeciesInfo(void);
u16 GetReleaseMonSpecies(void);
void ReleaseMonFromParty(void);
u16 GetReleaseMonLevel(void);

u16 ScriptGetLandEncounter(void);
u16 GetLandEncounter(void);

u16 ScriptGetWaterEncounter(void);
u16 GetWaterEncounter(void);

u16 ScriptGetFishingEncounter(void);
u16 GetFishingEncounter(void);

u16 ScriptGetRockSmashEncounter(void);
u16 GetRockSmashEncounter(void);

void GiveMomBerry(void);
void GiveGroundMomItem(void);
void GiveBoughtMomItem(void);

#endif // GUARD_FIELD_SPECIALS_H

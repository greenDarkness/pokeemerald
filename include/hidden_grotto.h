#ifndef GUARD_HIDDEN_GROTTO_H
#define GUARD_HIDDEN_GROTTO_H

const u8 *GetHiddenGrottoEntranceScript(s16 x, s16 y);
bool8 IsHiddenGrottoMap(u8 mapGroup, u8 mapNum);
bool8 IsPlayerInHiddenGrotto(void);
mapsec_u16_t GetHiddenGrottoEntranceMapSec(void);
u16 GetHiddenGrottoMusic(void);
void UpdateHiddenGrottosPerDay(void);
bool8 GetPokedexGrottoMonCandidate(u16 index, u16 *species, u8 *minLevel, u8 *maxLevel, u8 *mapGroup, u8 *mapNum);

// Specials
void GetFacingHiddenGrotto(void);
void EnterHiddenGrotto(void);
void SetupHiddenGrottoRoom(void);
void GetHiddenGrottoPokemon(void);
void StartHiddenGrottoBattle(void);
void GetHiddenGrottoItem(void);
void ClearHiddenGrottoContents(void);
void GetHiddenGrottoEgg(void);
void GiveHiddenGrottoEgg(void);
void RefreshHiddenGrottoRoomEggPalette(void);

extern u16 gHiddenGrottoEggPalette[16];

#endif // GUARD_HIDDEN_GROTTO_H

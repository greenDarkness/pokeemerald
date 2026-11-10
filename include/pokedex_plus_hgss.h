#ifndef GUARD_POKEDEX_PLUS_HGSS_H
#define GUARD_POKEDEX_PLUS_HGSS_H

// Pokedex Plus HGSS by TheXaman. Enabled with POKEDEX_PLUS_HGSS in include/config.h.
// gUnusedPokedexU8 / gPokedexVBlankCB are shared with (and defined in) src/pokedex.c; see pokedex.h.

void CB2_OpenPokedexPlusHGSS(void);
// Opens the Pokédex straight into local mode; leaving local mode goes to exitCallback
void OpenPokedexLocalMode(MainCallback exitCallback);
u8 DisplayCaughtMonDexPageHGSS(u16 species, u32 otId, u32 personality);
u16 NationalPokedexNumToSpeciesHGSS(u16 nationalNum);
void ResetPokedexPlusHGSSScrollPositions(void);
s8 GetPokedexDisplayFlag(u16 nationalNum, u8 caseId);
bool8 IsPokedexPageMonOwned(u16 species);

// Time-of-day icons (16x16 frames: morning, day, night, L badge, R badge) shared with the area screen
#define POKEDEX_TIME_ICONS_SIZE (5 * 16 * 16 / 2)

#endif // GUARD_POKEDEX_PLUS_HGSS_H

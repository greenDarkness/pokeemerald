#ifndef GUARD_POKEDEX_PLUS_HGSS_H
#define GUARD_POKEDEX_PLUS_HGSS_H

// Pokedex Plus HGSS by TheXaman. Enabled with POKEDEX_PLUS_HGSS in include/config.h.
// gUnusedPokedexU8 / gPokedexVBlankCB are shared with (and defined in) src/pokedex.c; see pokedex.h.

void CB2_OpenPokedexPlusHGSS(void);
u8 DisplayCaughtMonDexPageHGSS(u16 species, u32 otId, u32 personality);
u16 NationalPokedexNumToSpeciesHGSS(u16 nationalNum);
void ResetPokedexPlusHGSSScrollPositions(void);

#endif // GUARD_POKEDEX_PLUS_HGSS_H

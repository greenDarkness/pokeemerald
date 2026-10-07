#ifndef GUARD_SCRIPT_POKEMON_UTIL_H
#define GUARD_SCRIPT_POKEMON_UTIL_H

struct Pokemon;

u8 ScriptGiveMon(u16 species, u8 level, u16 item, u32 unused1, u32 unused2, u8 fixedIV);
void ScriptCreateGiftMon(struct Pokemon *mon, u16 species, u8 level, u16 item, u8 fixedIV);
void ScriptCreateGiftMonWithPersonality(struct Pokemon *mon, u16 species, u8 level, u16 item, u8 fixedIV, bool8 hasFixedPersonality, u32 personality);
u8 ScriptGiveCreatedMon(struct Pokemon *mon);
void ShowGiftMonPic(void);
void GiveGiftMon(void);
u8 ScriptGiveEgg(u16 species);
void CreateScriptedWildMon(u16 species, u8 level, u16 item);
void ScriptSetMonMoveSlot(u8 monIndex, u16 move, u8 slot);
void ReducePlayerPartyToSelectedMons(void);
void HealPlayerParty(void);
void GetPokerus(void);

#endif // GUARD_SCRIPT_POKEMON_UTIL_H

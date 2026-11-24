#ifndef GUARD_HERB_SHOP_H
#define GUARD_HERB_SHOP_H

// Specials (see data/maps/PetalburgCity_HerbShop/scripts.inc and data/scripts/herb_shop.inc)
void HerbShop_SetupShop(void);
u16 HerbShop_GetMission(void);
u16 HerbShop_GiveMission(void);
void HerbShop_StartFirstEscort(void);
u16 HerbShop_CanCompleteMission(void);
u16 HerbShop_GiveChosenMon(void);
void HerbShop_GiveIngredients(void);
u16 HerbShop_CompleteMission(void);
u16 HerbShop_UpdateEscortNpc(void);
void HerbShop_EscortMove(void);
void HerbShop_SetupGrottoEscort(void);
void HerbShop_FinishGrottoEscort(void);

#endif // GUARD_HERB_SHOP_H

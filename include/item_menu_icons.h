#ifndef GUARD_ITEM_MENU_ICONS_H
#define GUARD_ITEM_MENU_ICONS_H

extern const struct CompressedSpriteSheet gBagMaleSpriteSheet;
extern const struct CompressedSpriteSheet gBagFemaleSpriteSheet;
extern const struct CompressedSpritePalette gBagPaletteTable;
extern const struct CompressedSpriteSheet gBerryCheckCircleSpriteSheet;
extern const struct CompressedSpritePalette gBerryCheckCirclePaletteTable;

void RemoveBagSprite(u8 id);
void AddBagVisualSprite(u8 bagPocketId);
void SetBagVisualPocketId(u8 bagPocketId, bool8 isSwitchingPockets);
void ShakeBagSprite(void);
void AddSwitchPocketRotatingBallSprite(s16 rotationDirection);
void AddBagItemIconSprite(u16 itemId, u8 id);
void RemoveBagItemIconSprite(u8 id);

/* PC icon shown above the current item icon */
void AddBagPCIconSprite(u8 id);
void RemoveBagPCIconSprite(u8 id);
/* START button shown next to PC icon */
void AddBagStartButtonSprite(u8 id);
void RemoveBagStartButtonSprite(u8 id);
void AnimateStartButtonPress(void);
void CreateItemMenuSwapLine(void);
void SetItemMenuSwapLineInvisibility(bool8 invisible);
void UpdateItemMenuSwapLinePos(u8 y);
u8 CreateBerryTagSprite(u8 id, s16 x, s16 y);
void FreeBerryTagSpritePalette(void);
u8 CreateSpinningBerrySprite(u8 berryId, u8 x, u8 y, bool8 startAffine);
u8 CreateBerryFlavorCircleSprite(s16 x);

#endif // GUARD_ITEM_MENU_ICONS_H

#include "global.h"
#include "item_menu.h"
#include "battle.h"
#include "battle_controllers.h"
#include "battle_pyramid.h"
#include "frontier_util.h"
#include "battle_pyramid_bag.h"
#include "berry_tag_screen.h"
#include "bg.h"
#include "data.h"
#include "decompress.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_scripts.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "graphics.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "item.h"
#include "item_icon.h"
#include "item_menu_icons.h"
#include "item_use.h"
#include "lilycove_lady.h"
#include "list_menu.h"
#include "link.h"
#include "mail.h"
#include "malloc.h"
#include "map_name_popup.h"
#include "menu.h"
#include "money.h"
#include "new_moves_popup.h"
#include "pickup_item_popup.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "player_pc.h"
#include "pokemon.h"
#include "pokemon_summary_screen.h"
#include "scanline_effect.h"
#include "script.h"
#include "shop.h"
#include "sound.h"
#include "sprite.h"
#include "strings.h"
#include "string_util.h"
#include "task.h"
#include "text_window.h"
#include "menu_helpers.h"
#include "window.h"
#include "apprentice.h"
#include "battle_pike.h"
#include "constants/items.h"
#include "constants/flags.h"
#include "constants/rgb.h"
#include "constants/songs.h"

// PC Item Storage Overlay for quick withdraw
#define PC_OVERLAY_MAX_SHOWN 8
#define PC_OVERLAY_TAG_SCROLL_ARROW 5112
#define PC_OVERLAY_TAG_ITEM_ICON    5110

enum {
    PC_OVERLAY_WIN_LIST,
    PC_OVERLAY_WIN_MESSAGE,
    PC_OVERLAY_WIN_ICON,
    PC_OVERLAY_WIN_TITLE,
    PC_OVERLAY_WIN_QUANTITY,
    PC_OVERLAY_WIN_COUNT
};

struct BagPCOverlay {
    struct ListMenuItem listItems[PC_ITEMS_COUNT + 1];
    u8 itemNames[PC_ITEMS_COUNT + 1][ITEM_NAME_LENGTH + 10];
    u8 windowIds[PC_OVERLAY_WIN_COUNT];
    u8 spriteId;
    u8 listTaskId;
    u8 scrollIndicatorTaskId;
    u16 cursorPos;
    u16 itemsAbove;
    u8 pageItems;
    u8 count;
    s16 quantity;
};

#define TAG_POCKET_SCROLL_ARROW 110
#define TAG_BAG_SCROLL_ARROW    111

// The buffer for the bag item list needs to be large enough to hold the maximum
// number of item slots that could fit in a single pocket, + 1 for Cancel.
// This constant picks the max of the existing pocket sizes.
// By default, the largest pocket is BAG_TMHM_COUNT at 64.
#define MAX_POCKET_ITEMS  ((max(BAG_TMHM_COUNT,              \
                            max(BAG_BERRIES_COUNT,           \
                            max(BAG_ITEMS_COUNT,             \
                            max(BAG_KEYITEMS_COUNT,          \
                                BAG_POKEBALLS_COUNT))))) + 1)

// Up to 8 item slots can be visible at a time
#define MAX_ITEMS_SHOWN 8

enum {
    SWITCH_POCKET_NONE,
    SWITCH_POCKET_LEFT,
    SWITCH_POCKET_RIGHT
};

enum {
    ACTION_USE,
    ACTION_TOSS,
    ACTION_REGISTER,
    ACTION_GIVE,
    ACTION_CANCEL,
    ACTION_BATTLE_USE,
    ACTION_CHECK,
    ACTION_WALK,
    ACTION_DESELECT,
    ACTION_CHECK_TAG,
    ACTION_CONFIRM,
    ACTION_SHOW,
    ACTION_GIVE_FAVOR_LADY,
    ACTION_CONFIRM_QUIZ_LADY,
    ACTION_DUMMY,
};

enum {
    WIN_ITEM_LIST,
    WIN_DESCRIPTION,
    WIN_POCKET_NAME,
    WIN_IP_OVERLAY,
    WIN_AP_OVERLAY,
    WIN_TMHM_INFO_ICONS,
    WIN_TMHM_INFO,
    WIN_MESSAGE, // Identical to ITEMWIN_MESSAGE. Unused?
};

// Item list ID for toSwapPos to indicate an item is not currently being swapped
#define NOT_SWAPPING 0xFF

struct ListBuffer1 {
    struct ListMenuItem subBuffers[MAX_POCKET_ITEMS];
};

struct ListBuffer2 {
    u8 name[MAX_POCKET_ITEMS][ITEM_NAME_LENGTH + 10];
};

struct TempWallyBag {
    struct ItemSlot bagPocket_Items[BAG_ITEMS_COUNT];
    struct ItemSlot bagPocket_PokeBalls[BAG_POKEBALLS_COUNT];
    u16 cursorPosition[POCKETS_COUNT];
    u16 scrollPosition[POCKETS_COUNT];
    u16 unused;
    u16 pocket;
};

static void CB2_Bag(void);
static bool8 SetupBagMenu(void);
static void BagMenu_InitBGs(void);
static bool8 LoadBagMenu_Graphics(void);
static void LoadBagMenuTextWindows(void);
static void AllocateBagItemListBuffers(void);
static void LoadBagItemListBuffers(u8);
static void PrintPocketNames(const u8 *, const u8 *);
static void CopyPocketNameToWindow(u32);
static void PrintIPOverlay(void);
static void PrintAPOverlay(void);
static void DrawPocketIndicatorSquare(u8, bool8);
static void CreatePocketScrollArrowPair(void);
static void CreatePocketSwitchArrowPair(void);
static void DestroyPocketSwitchArrowPair(void);
static void PrepareTMHMMoveWindow(void);
static bool8 IsWallysBag(void);
static void Task_WallyTutorialBagMenu(u8);
static void Task_BagMenu_HandleInput(u8);
static void GetItemNameFromPocket(u8 *, u16);
static void PrintItemDescription(int);
static void BagMenu_PrintCursorAtPos(u8, u8);
static void BagMenu_Print(u8, u8, const u8 *, u8, u8, u8, u8, u8, u8);
static void Task_CloseBagMenu(u8);
static u8 AddItemMessageWindow(u8);
static void RemoveItemMessageWindow(u8);
static void ReturnToItemList(u8);
static void PrintItemQuantity(u8, s16);
static u8 BagMenu_AddWindow(u8);
static u8 GetSwitchBagPocketDirection(void);
static void SwitchBagPocket(u8, s16, bool16);
static bool8 CanSwapItems(void);
static void StartItemSwap(u8 taskId);
static void Task_SwitchBagPocket(u8);
static void Task_HandleSwappingItemsInput(u8);
static void DoItemSwap(u8);
static void CancelItemSwap(u8);
static void SendSwappedItemToPC(u8);
static void Task_ChooseHowManyToDepositFromSwap(u8);
static void TryDepositItemFromSwap(u8);
static void PrintTMHMMoveData(u16);
static void PrintContextMenuItems(u8);
static void PrintContextMenuItemGrid(u8, u8, u8);
static void Task_ItemContext_SingleRow(u8);
static void Task_ItemContext_MultipleRows(u8);
static bool8 IsValidContextMenuPos(s8);
static void BagMenu_RemoveWindow(u8);
static void PrintThereIsNoPokemon(u8);
static void Task_ChooseHowManyToToss(u8);
static void AskTossItems(u8);
static void Task_RemoveItemFromBag(u8);
static void ItemMenu_Cancel(u8);
static void HandleErrorMessage(u8);
static void PrintItemCantBeHeld(u8);
static void DisplayCurrentMoneyWindow(void);
static void DisplaySellItemPriceAndConfirm(u8);
static void InitSellHowManyInput(u8);
static void AskSellItems(u8);
static void RemoveMoneyWindow(void);
static void Task_ChooseHowManyToSell(u8);
static void SellItem(u8);
static void WaitAfterItemSell(u8);
static void TryDepositItem(u8);
static void Task_ChooseHowManyToDeposit(u8 taskId);
static void WaitDepositErrorMessage(u8);
static void CB2_ApprenticeExitBagMenu(void);
static void CB2_FavorLadyExitBagMenu(void);
static void CB2_QuizLadyExitBagMenu(void);
static void UpdatePocketItemLists(void);
static void InitPocketListPositions(void);
static void InitPocketScrollPositions(void);
static u8 CreateBagInputHandlerTask(u8);
static void DrawItemListBgRow(u8);
static void BagMenu_MoveCursorCallback(s32, bool8, struct ListMenu *);
static void BagMenu_ItemPrintCallback(u8, u32, u8);
static void ItemMenu_UseOutOfBattle(u8);
static void ItemMenu_Toss(u8);
static void ItemMenu_Register(u8);
static void ItemMenu_Give(u8);
static void ItemMenu_Cancel(u8);
static void ItemMenu_UseInBattle(u8);
static void ItemMenu_CheckTag(u8);
static void ItemMenu_Show(u8);
static void ItemMenu_GiveFavorLady(u8);
static void ItemMenu_ConfirmQuizLady(u8);
static void Task_ItemContext_Normal(u8);
static void Task_ItemContext_GiveToParty(u8);
static void Task_ItemContext_Sell(u8);
static void Task_ItemContext_Deposit(u8);
static void Task_ItemContext_GiveToPC(u8);
static void ConfirmToss(u8);
static void CancelToss(u8);
static void ConfirmSell(u8);
static void CancelSell(u8);

// PC Storage Overlay forward declarations
static void PCOverlay_Open(u8);
static void PCOverlay_Init(void);
static void PCOverlay_Free(void);
static u8 PCOverlay_AddWindow(u8);
static void PCOverlay_RemoveWindow(u8);
static void PCOverlay_RefreshListMenu(void);
static void PCOverlay_CreateListMenu(u8);
static void PCOverlay_ProcessInput(u8);
static void PCOverlay_DoItemAction(u8);
static void PCOverlay_DoItemWithdraw(u8);
static void PCOverlay_HandleQuantityRolling(u8);
static void PCOverlay_HandleRemoveItem(u8);
static void PCOverlay_HandleErrorMessageInput(u8);
static void PCOverlay_ReturnToListInput(u8);
static void PCOverlay_Close(u8);
static void PCOverlay_MoveCursor(s32, bool8, struct ListMenu *);
static void PCOverlay_PrintMenuItem(u8, u32, u8);
static void PCOverlay_PrintDescription(s32);
static void PCOverlay_PrintMessage(const u8 *);
static void PCOverlay_DrawItemIcon(u16);
static void PCOverlay_EraseItemIcon(void);
static void PCOverlay_AddScrollIndicator(void);
static void PCOverlay_RemoveScrollIndicator(void);
static void PCOverlay_PrintItemQuantity(u8, u16, u32, u8, u8, u8);

static const struct BgTemplate sBgTemplates_ItemMenu[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0,
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
    {
        .bg = 2,
        .charBaseIndex = 3,
        .mapBaseIndex = 29,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0,
    },
};

static const struct ListMenuTemplate sItemListMenu =
{
    .items = NULL,
    .moveCursorFunc = BagMenu_MoveCursorCallback,
    .itemPrintFunc = BagMenu_ItemPrintCallback,
    .totalItems = 0,
    .maxShowed = 0,
    .windowId = WIN_ITEM_LIST,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 1,
    .cursorPal = 1,
    .fillValue = 0,
    .cursorShadowPal = 3,
    .lettersSpacing = 0,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = FONT_NARROW,
    .cursorKind = CURSOR_BLACK_ARROW
};

static const struct MenuAction sItemMenuActions[] = {
    [ACTION_USE]               = {gMenuText_Use,      {ItemMenu_UseOutOfBattle}},
    [ACTION_TOSS]              = {gMenuText_Toss,     {ItemMenu_Toss}},
    [ACTION_REGISTER]          = {gMenuText_Register, {ItemMenu_Register}},
    [ACTION_GIVE]              = {gMenuText_Give,     {ItemMenu_Give}},
    [ACTION_CANCEL]            = {gText_Cancel2,      {ItemMenu_Cancel}},
    [ACTION_BATTLE_USE]        = {gMenuText_Use,      {ItemMenu_UseInBattle}},
    [ACTION_CHECK]             = {gMenuText_Check,    {ItemMenu_UseOutOfBattle}},
    [ACTION_WALK]              = {gMenuText_Walk,     {ItemMenu_UseOutOfBattle}},
    [ACTION_DESELECT]          = {gMenuText_Deselect, {ItemMenu_Register}},
    [ACTION_CHECK_TAG]         = {gMenuText_CheckTag, {ItemMenu_CheckTag}},
    [ACTION_CONFIRM]           = {gMenuText_Confirm,  {Task_FadeAndCloseBagMenu}},
    [ACTION_SHOW]              = {gMenuText_Show,     {ItemMenu_Show}},
    [ACTION_GIVE_FAVOR_LADY]   = {gMenuText_Give2,    {ItemMenu_GiveFavorLady}},
    [ACTION_CONFIRM_QUIZ_LADY] = {gMenuText_Confirm,  {ItemMenu_ConfirmQuizLady}},
    [ACTION_DUMMY]             = {gText_EmptyString2, {NULL}}
};

// these are all 2D arrays with a width of 2 but are represented as 1D arrays
// ACTION_DUMMY is used to represent blank spaces
static const u8 sContextMenuItems_ItemsPocket[] = {
    ACTION_USE,         ACTION_GIVE,
    ACTION_TOSS,        ACTION_CANCEL
};

static const u8 sContextMenuItems_KeyItemsPocket[] = {
    ACTION_USE,         ACTION_REGISTER,
    ACTION_DUMMY,       ACTION_CANCEL
};

static const u8 sContextMenuItems_BallsPocket[] = {
    ACTION_GIVE,        ACTION_DUMMY,
    ACTION_TOSS,        ACTION_CANCEL
};

static const u8 sContextMenuItems_TmHmPocket[] = {
    ACTION_USE,         ACTION_GIVE,
    ACTION_DUMMY,       ACTION_CANCEL
};

static const u8 sContextMenuItems_BerriesPocket[] = {
    ACTION_CHECK_TAG,   ACTION_DUMMY,
    ACTION_USE,         ACTION_GIVE,
    ACTION_TOSS,        ACTION_CANCEL
};

static const u8 sContextMenuItems_BattleUse[] = {
    ACTION_BATTLE_USE,  ACTION_CANCEL
};

static const u8 sText_NotEnoughAP[] = _("You don't have enough AP!");
static const u8 sText_NotEnoughIP[] = _("You don't have enough IP!");

static const u8 sContextMenuItems_Give[] = {
    ACTION_GIVE,        ACTION_CANCEL
};

static const u8 sContextMenuItems_Cancel[] = {
    ACTION_CANCEL
};

static const u8 sContextMenuItems_BerryBlenderCrush[] = {
    ACTION_CONFIRM,     ACTION_CHECK_TAG,
    ACTION_DUMMY,       ACTION_CANCEL
};

static const u8 sContextMenuItems_Apprentice[] = {
    ACTION_SHOW,        ACTION_CANCEL
};

static const u8 sContextMenuItems_FavorLady[] = {
    ACTION_GIVE_FAVOR_LADY, ACTION_CANCEL
};

static const u8 sContextMenuItems_QuizLady[] = {
    ACTION_CONFIRM_QUIZ_LADY, ACTION_CANCEL
};

static const TaskFunc sContextMenuFuncs[] = {
    [ITEMMENULOCATION_FIELD] =                  Task_ItemContext_Normal,
    [ITEMMENULOCATION_BATTLE] =                 Task_ItemContext_Normal,
    [ITEMMENULOCATION_PARTY] =                  Task_ItemContext_GiveToParty,
    [ITEMMENULOCATION_SHOP] =                   Task_ItemContext_Sell,
    [ITEMMENULOCATION_BERRY_TREE] =             Task_FadeAndCloseBagMenu,
    [ITEMMENULOCATION_BERRY_BLENDER_CRUSH] =    Task_ItemContext_Normal,
    [ITEMMENULOCATION_ITEMPC] =                 Task_ItemContext_Deposit,
    [ITEMMENULOCATION_FAVOR_LADY] =             Task_ItemContext_Normal,
    [ITEMMENULOCATION_QUIZ_LADY] =              Task_ItemContext_Normal,
    [ITEMMENULOCATION_APPRENTICE] =             Task_ItemContext_Normal,
    [ITEMMENULOCATION_WALLY] =                  NULL,
    [ITEMMENULOCATION_PCBOX] =                  Task_ItemContext_GiveToPC,
    [ITEMMENULOCATION_BALL_SWAP] =              Task_FadeAndCloseBagMenu
};

static const struct YesNoFuncTable sYesNoTossFunctions = {ConfirmToss, CancelToss};

static const struct YesNoFuncTable sYesNoSellItemFunctions = {ConfirmSell, CancelSell};

static const struct ScrollArrowsTemplate sBagScrollArrowsTemplate = {
    .firstArrowType = SCROLL_ARROW_LEFT,
    .firstX = 28,
    .firstY = 16,
    .secondArrowType = SCROLL_ARROW_RIGHT,
    .secondX = 100,
    .secondY = 16,
    .fullyUpThreshold = -1,
    .fullyDownThreshold = -1,
    .tileTag = TAG_BAG_SCROLL_ARROW,
    .palTag = TAG_BAG_SCROLL_ARROW,
    .palNum = 0,
};

static const u8 sRegisteredSelect_Gfx[] = INCBIN_U8("graphics/bag/select_button.4bpp");
static const u8 sMachoGearOn_Gfx[] = INCBIN_U8("graphics/bag/on_button.4bpp");

enum {
    COLORID_NORMAL,
    COLORID_POCKET_NAME,
    COLORID_GRAY_CURSOR,
    COLORID_UNUSED,
    COLORID_TMHM_INFO,
    COLORID_NONE = 0xFF
};
static const u8 sFontColorTable[][3] = {
                            // bgColor, textColor, shadowColor
    [COLORID_NORMAL]      = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE,      TEXT_COLOR_LIGHT_GRAY},
    [COLORID_POCKET_NAME] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE,      TEXT_COLOR_RED},
    [COLORID_GRAY_CURSOR] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_GRAY, TEXT_COLOR_GREEN},
    [COLORID_UNUSED]      = {TEXT_COLOR_DARK_GRAY,   TEXT_COLOR_WHITE,      TEXT_COLOR_LIGHT_GRAY},
    [COLORID_TMHM_INFO]   = {TEXT_COLOR_TRANSPARENT, TEXT_DYNAMIC_COLOR_5,  TEXT_DYNAMIC_COLOR_1}
};

static const struct WindowTemplate sDefaultBagWindows[] =
{
    [WIN_ITEM_LIST] = {
        .bg = 0,
        .tilemapLeft = 14,
        .tilemapTop = 2,
        .width = 15,
        .height = 16,
        .paletteNum = 1,
        .baseBlock = 0x27,
    },
    [WIN_DESCRIPTION] = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 13,
        .width = 14,
        .height = 6,
        .paletteNum = 1,
        .baseBlock = 0x117,
    },
    [WIN_POCKET_NAME] = {
        .bg = 0,
        .tilemapLeft = 4,
        .tilemapTop = 1,
        .width = 8,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 0x1A1,
    },
    [WIN_IP_OVERLAY] = {
        .bg = 0,
        .tilemapLeft = 23, // adjust this to move X
        .tilemapTop = 0,  // adjust this to move Y
        .width = 6,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 0x01B,  // between frame tiles (~0x009) and WIN_ITEM_LIST (0x027)
    },
    [WIN_AP_OVERLAY] = {
        .bg = 0,
        .tilemapLeft = 14,
        .tilemapTop = 0,
        .width = 6,
        .height = 2,
        .paletteNum = 1,
        .baseBlock = 0x271,
    },
    [WIN_TMHM_INFO_ICONS] = {
        .bg = 0,
        .tilemapLeft = 1,
        .tilemapTop = 13,
        .width = 5,
        .height = 6,
        .paletteNum = 12,
        .baseBlock = 0x16B,
    },
    [WIN_TMHM_INFO] = {
        .bg = 0,
        .tilemapLeft = 7,
        .tilemapTop = 13,
        .width = 4,
        .height = 6,
        .paletteNum = 12,
        .baseBlock = 0x189,
    },
    [WIN_MESSAGE] = {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 27,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x1B1,
    },
    DUMMY_WIN_TEMPLATE,
};

static const struct WindowTemplate sContextMenuWindowTemplates[] =
{
    [ITEMWIN_1x1] = {
        .bg = 1,
        .tilemapLeft = 22,
        .tilemapTop = 17,
        .width = 7,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x21D,
    },
    [ITEMWIN_1x2] = {
        .bg = 1,
        .tilemapLeft = 22,
        .tilemapTop = 15,
        .width = 7,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x21D,
    },
    [ITEMWIN_2x2] = {
        .bg = 1,
        .tilemapLeft = 15,
        .tilemapTop = 15,
        .width = 14,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x21D,
    },
    [ITEMWIN_2x3] = {
        .bg = 1,
        .tilemapLeft = 15,
        .tilemapTop = 13,
        .width = 14,
        .height = 6,
        .paletteNum = 15,
        .baseBlock = 0x21D,
    },
    [ITEMWIN_MESSAGE] = {
        .bg = 1,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 27,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x1B1,
    },
    [ITEMWIN_YESNO_LOW] = { // Yes/No tucked in corner, for toss confirm
        .bg = 1,
        .tilemapLeft = 24,
        .tilemapTop = 15,
        .width = 5,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x21D,
    },
    [ITEMWIN_YESNO_HIGH] = { // Yes/No higher up, positioned above a lower message box
        .bg = 1,
        .tilemapLeft = 21,
        .tilemapTop = 9,
        .width = 5,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 0x21D,
    },
    [ITEMWIN_QUANTITY] = { // Used for quantity of items to Toss/Deposit
        .bg = 1,
        .tilemapLeft = 24,
        .tilemapTop = 17,
        .width = 5,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x21D,
    },
    [ITEMWIN_QUANTITY_WIDE] = { // Used for quantity and price of items to Sell
        .bg = 1,
        .tilemapLeft = 18,
        .tilemapTop = 11,
        .width = 10,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x245,
    },
    [ITEMWIN_MONEY] = {
        .bg = 1,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 10,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x231,
    },
};

// PC Storage Overlay window templates
// baseBlocks start at 0x280 to avoid conflicts with bag windows (max ~0x245) and frame tiles (at offset 1+)
static const struct WindowTemplate sPCOverlayWindowTemplates[PC_OVERLAY_WIN_COUNT] =
{
    [PC_OVERLAY_WIN_LIST] = {
        .bg = 1,
        .tilemapLeft = 14,
        .tilemapTop = 1,
        .width = 15,
        .height = 18,
        .paletteNum = 15,
        .baseBlock = 0x280
    },
    [PC_OVERLAY_WIN_MESSAGE] = {
        .bg = 1,
        .tilemapLeft = 1,
        .tilemapTop = 13,
        .width = 11,
        .height = 6,
        .paletteNum = 15,
        .baseBlock = 0x38E  // 0x280 + (15 * 18)
    },
    [PC_OVERLAY_WIN_ICON] = {
        .bg = 1,
        .tilemapLeft = 1,
        .tilemapTop = 8,
        .width = 3,
        .height = 3,
        .paletteNum = 15,
        .baseBlock = 0x3D0  // 0x38E + (11 * 6)
    },
    [PC_OVERLAY_WIN_TITLE] = {
        .bg = 1,
        .tilemapLeft = 1,
        .tilemapTop = 1,
        .width = 11,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x3D9  // 0x3D0 + (3 * 3)
    },
    [PC_OVERLAY_WIN_QUANTITY] = {
        .bg = 1,
        .tilemapLeft = 6,
        .tilemapTop = 9,
        .width = 6,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0x3EF  // 0x3D9 + (11 * 2)
    },
};

static const struct ListMenuTemplate sPCOverlayListMenuTemplate =
{
    .items = NULL,
    .moveCursorFunc = PCOverlay_MoveCursor,
    .itemPrintFunc = PCOverlay_PrintMenuItem,
    .totalItems = 0,
    .maxShowed = 0,
    .windowId = 0,
    .header_X = 0,
    .item_X = 8,
    .cursor_X = 0,
    .upText_Y = 9,
    .cursorPal = 2,
    .fillValue = 1,
    .cursorShadowPal = 3,
    .lettersSpacing = FALSE,
    .itemVerticalPadding = 0,
    .scrollMultiple = LIST_NO_MULTIPLE_SCROLL,
    .fontId = FONT_NARROW,
    .cursorKind = CURSOR_BLACK_ARROW,
};

EWRAM_DATA struct BagMenu *gBagMenu = 0;
EWRAM_DATA struct BagPosition gBagPosition = {0};
static EWRAM_DATA struct ListBuffer1 *sListBuffer1 = 0;
static EWRAM_DATA struct ListBuffer2 *sListBuffer2 = 0;
EWRAM_DATA u16 gSpecialVar_ItemId = 0;
static EWRAM_DATA struct TempWallyBag *sTempWallyBag = 0;
static EWRAM_DATA struct BagPCOverlay *sPCOverlay = NULL;

void ResetBagScrollPositions(void)
{
    gBagPosition.pocket = ITEMS_POCKET;
    memset(gBagPosition.cursorPosition, 0, sizeof(gBagPosition.cursorPosition));
    memset(gBagPosition.scrollPosition, 0, sizeof(gBagPosition.scrollPosition));
}

void CB2_BagMenuFromStartMenu(void)
{
    GoToBagMenu(ITEMMENULOCATION_FIELD, POCKETS_COUNT, CB2_ReturnToFieldWithOpenMenu);
}

void CB2_BagMenuFromBattle(void)
{
    if (CurrentBattlePyramidLocation() == PYRAMID_LOCATION_NONE)
        GoToBagMenu(ITEMMENULOCATION_BATTLE, POCKETS_COUNT, CB2_SetUpReshowBattleScreenAfterMenu2);
    else
        GoToBattlePyramidBagMenu(PYRAMIDBAG_LOC_BATTLE, CB2_SetUpReshowBattleScreenAfterMenu2);
}

// Choosing berry to plant
void CB2_ChooseBerry(void)
{
    GoToBagMenu(ITEMMENULOCATION_BERRY_TREE, BERRIES_POCKET, CB2_ReturnToFieldContinueScript);
}

// Choosing berry for Berry Blender or Berry Crush
void ChooseBerryForMachine(MainCallback exitCallback)
{
    GoToBagMenu(ITEMMENULOCATION_BERRY_BLENDER_CRUSH, BERRIES_POCKET, exitCallback);
}

void CB2_GoToSellMenu(void)
{
    GoToBagMenu(ITEMMENULOCATION_SHOP, POCKETS_COUNT, CB2_ExitSellMenu);
}

void CB2_GoToItemDepositMenu(void)
{
    GoToBagMenu(ITEMMENULOCATION_ITEMPC, POCKETS_COUNT, CB2_PlayerPCExitBagMenu);
}

void ApprenticeOpenBagMenu(void)
{
    GoToBagMenu(ITEMMENULOCATION_APPRENTICE, POCKETS_COUNT, CB2_ApprenticeExitBagMenu);
    gSpecialVar_0x8005 = ITEM_NONE;
    gSpecialVar_Result = FALSE;
}

void FavorLadyOpenBagMenu(void)
{
    GoToBagMenu(ITEMMENULOCATION_FAVOR_LADY, POCKETS_COUNT, CB2_FavorLadyExitBagMenu);
    gSpecialVar_Result = FALSE;
}

void QuizLadyOpenBagMenu(void)
{
    GoToBagMenu(ITEMMENULOCATION_QUIZ_LADY, POCKETS_COUNT, CB2_QuizLadyExitBagMenu);
    gSpecialVar_Result = FALSE;
}

void GoToBagMenu(u8 location, u8 pocket, MainCallback exitCallback)
{
    gBagMenu = AllocZeroed(sizeof(*gBagMenu));
    if (gBagMenu == NULL)
    {
        // Alloc failed, exit
        SetMainCallback2(exitCallback);
    }
    else
    {
        if (location != ITEMMENULOCATION_LAST)
            gBagPosition.location = location;
        if (exitCallback)
            gBagPosition.exitCallback = exitCallback;
        if (pocket < POCKETS_COUNT)
            gBagPosition.pocket = pocket;
        if (gBagPosition.location == ITEMMENULOCATION_BERRY_TREE ||
            gBagPosition.location == ITEMMENULOCATION_BERRY_BLENDER_CRUSH ||
            gBagPosition.location == ITEMMENULOCATION_BALL_SWAP)
            gBagMenu->pocketSwitchDisabled = TRUE;
        gBagMenu->newScreenCallback = NULL;
        gBagMenu->toSwapPos = NOT_SWAPPING;
        gBagMenu->showItemCost = FALSE;
        gBagMenu->pocketScrollArrowsTask = TASK_NONE;
        gBagMenu->pocketSwitchArrowsTask = TASK_NONE;
        memset(gBagMenu->spriteIds, SPRITE_NONE, sizeof(gBagMenu->spriteIds));
        memset(gBagMenu->windowIds, WINDOW_NONE, sizeof(gBagMenu->windowIds));
        SetMainCallback2(CB2_Bag);
    }
}

void CB2_BagMenuRun(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

void VBlankCB_BagMenuRun(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

#define tListTaskId        data[0]
#define tListPosition      data[1]
#define tQuantity          data[2]
#define tNeverRead         data[3]
#define tItemCount         data[8]
#define tMsgWindowId       data[10]
#define tPocketSwitchDir   data[11]
#define tPocketSwitchTimer data[12]
#define tPocketSwitchState data[13]

static void CB2_Bag(void)
{
    while(MenuHelpers_ShouldWaitForLinkRecv() != TRUE && SetupBagMenu() != TRUE && MenuHelpers_IsLinkActive() != TRUE)
        {};
}

static bool8 SetupBagMenu(void)
{
    u8 taskId;

    switch (gMain.state)
    {
    case 0:
        SetVBlankHBlankCallbacksToNull();
        ClearScheduledBgCopiesToVram();
        gMain.state++;
        break;
    case 1:
        ScanlineEffect_Stop();
        gMain.state++;
        break;
    case 2:
        FreeAllSpritePalettes();
        gMain.state++;
        break;
    case 3:
        ResetPaletteFade();
        gPaletteFade.bufferTransferDisabled = TRUE;
        gMain.state++;
        break;
    case 4:
        ResetSpriteData();
        gMain.state++;
        break;
    case 5:
        gMain.state++;
        break;
    case 6:
        if (!MenuHelpers_IsLinkActive())
            ResetTasks();
        gMain.state++;
        break;
    case 7:
        BagMenu_InitBGs();
        gBagMenu->graphicsLoadState = 0;
        gMain.state++;
        break;
    case 8:
        if (!LoadBagMenu_Graphics())
            break;
        gMain.state++;
        break;
    case 9:
        LoadBagMenuTextWindows();
        gMain.state++;
        break;
    case 10:
        UpdatePocketItemLists();
        InitPocketListPositions();
        InitPocketScrollPositions();
        gMain.state++;
        break;
    case 11:
        AllocateBagItemListBuffers();
        gMain.state++;
        break;
    case 12:
        LoadBagItemListBuffers(gBagPosition.pocket);
        gMain.state++;
        break;
    case 13:
        PrintPocketNames(gPocketNamesStringsTable[gBagPosition.pocket], 0);
        CopyPocketNameToWindow(0);
        DrawPocketIndicatorSquare(gBagPosition.pocket, TRUE);
        gMain.state++;
        break;
    case 14:
        taskId = CreateBagInputHandlerTask(gBagPosition.location);
        gTasks[taskId].tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, gBagPosition.scrollPosition[gBagPosition.pocket], gBagPosition.cursorPosition[gBagPosition.pocket]);
        gTasks[taskId].tNeverRead = 0;
        gTasks[taskId].tItemCount = 0;
        gMain.state++;
        break;
    case 15:
        AddBagVisualSprite(gBagPosition.pocket);
        gMain.state++;
        break;
    case 16:
        CreateItemMenuSwapLine();
        gMain.state++;
        break;
    case 17:
        CreatePocketScrollArrowPair();
        CreatePocketSwitchArrowPair();
        gMain.state++;
        break;
    case 18:
        PrepareTMHMMoveWindow();
        gMain.state++;
        break;
    case 19:
        BlendPalettes(PALETTES_ALL, 16, 0);
        gMain.state++;
        break;
    case 20:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        gPaletteFade.bufferTransferDisabled = FALSE;
        gMain.state++;
        break;
    default:
        SetVBlankCallback(VBlankCB_BagMenuRun);
        SetMainCallback2(CB2_BagMenuRun);
        return TRUE;
    }
    return FALSE;
}

static void BagMenu_InitBGs(void)
{
    ResetVramOamAndBgCntRegs();
    memset(gBagMenu->tilemapBuffer, 0, sizeof(gBagMenu->tilemapBuffer));
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sBgTemplates_ItemMenu, ARRAY_COUNT(sBgTemplates_ItemMenu));
    SetBgTilemapBuffer(2, gBagMenu->tilemapBuffer);
    ResetAllBgsCoordinates();
    ScheduleBgCopyTilemapToVram(2);
    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
    ShowBg(0);
    ShowBg(1);
    ShowBg(2);
    SetGpuReg(REG_OFFSET_BLDCNT, 0);
}

static bool8 LoadBagMenu_Graphics(void)
{
    switch (gBagMenu->graphicsLoadState)
    {
    case 0:
        ResetTempTileDataBuffers();
        DecompressAndCopyTileDataToVram(2, gBagScreen_Gfx, 0, 0, 0);
        gBagMenu->graphicsLoadState++;
        break;
    case 1:
        if (FreeTempTileDataBuffersIfPossible() != TRUE)
        {
            LZDecompressWram(gBagScreen_GfxTileMap, gBagMenu->tilemapBuffer);
            gBagMenu->graphicsLoadState++;
        }
        break;
    case 2:
        if (!IsWallysBag() && gSaveBlock2Ptr->playerGender != MALE)
            LoadCompressedPalette(gBagScreenFemale_Pal, BG_PLTT_ID(0), 2 * PLTT_SIZE_4BPP);
        else
            LoadCompressedPalette(gBagScreenMale_Pal, BG_PLTT_ID(0), 2 * PLTT_SIZE_4BPP);
        gBagMenu->graphicsLoadState++;
        break;
    case 3:
        if (IsWallysBag() == TRUE || gSaveBlock2Ptr->playerGender == MALE)
            LoadCompressedSpriteSheet(&gBagMaleSpriteSheet);
        else
            LoadCompressedSpriteSheet(&gBagFemaleSpriteSheet);
        gBagMenu->graphicsLoadState++;
        break;
    case 4:
        LoadCompressedSpritePalette(&gBagPaletteTable);
        gBagMenu->graphicsLoadState++;
        break;
    default:
        LoadListMenuSwapLineGfx();
        gBagMenu->graphicsLoadState = 0;
        return TRUE;
    }
    return FALSE;
}

static u8 CreateBagInputHandlerTask(u8 location)
{
    u8 taskId;
    if (location == ITEMMENULOCATION_WALLY)
        taskId = CreateTask(Task_WallyTutorialBagMenu, 0);
    else
        taskId = CreateTask(Task_BagMenu_HandleInput, 0);
    return taskId;
}

static void AllocateBagItemListBuffers(void)
{
    sListBuffer1 = Alloc(sizeof(*sListBuffer1));
    sListBuffer2 = Alloc(sizeof(*sListBuffer2));
}

static void LoadBagItemListBuffers(u8 pocketId)
{
    u16 i;
    struct BagPocket *pocket = &gBagPockets[pocketId];
    struct ListMenuItem *subBuffer;

    if (!gBagMenu->hideCloseBagText)
    {
        for (i = 0; i < gBagMenu->numItemStacks[pocketId] - 1; i++)
        {
            GetItemNameFromPocket(sListBuffer2->name[i], pocket->itemSlots[i].itemId);
            subBuffer = sListBuffer1->subBuffers;
            subBuffer[i].name = sListBuffer2->name[i];
            subBuffer[i].id = i;
        }
        StringCopy(sListBuffer2->name[i], gText_CloseBag);
        subBuffer = sListBuffer1->subBuffers;
        subBuffer[i].name = sListBuffer2->name[i];
        subBuffer[i].id = LIST_CANCEL;
    }
    else
    {
        for (i = 0; i < gBagMenu->numItemStacks[pocketId]; i++)
        {
            GetItemNameFromPocket(sListBuffer2->name[i], pocket->itemSlots[i].itemId);
            subBuffer = sListBuffer1->subBuffers;
            subBuffer[i].name = sListBuffer2->name[i];
            subBuffer[i].id = i;
        }
    }
    gMultiuseListMenuTemplate = sItemListMenu;
    gMultiuseListMenuTemplate.totalItems = gBagMenu->numItemStacks[pocketId];
    gMultiuseListMenuTemplate.items = sListBuffer1->subBuffers;
    gMultiuseListMenuTemplate.maxShowed = gBagMenu->numShownItems[pocketId];
}

static void GetItemNameFromPocket(u8 *dest, u16 itemId)
{
    switch (gBagPosition.pocket)
    {
    case TMHM_POCKET:
        StringCopy(gStringVar2, gMoveNames[ItemIdToBattleMoveId(itemId)]);
        if (itemId >= ITEM_HM01)
        {
            // Get HM number
            ConvertIntToDecimalStringN(gStringVar1, itemId - ITEM_HM01 + 1, STR_CONV_MODE_LEADING_ZEROS, 1);
            StringExpandPlaceholders(dest, gText_NumberItem_HM);
        }
        else
        {
            // Get TM number
            ConvertIntToDecimalStringN(gStringVar1, itemId - ITEM_TM01 + 1, STR_CONV_MODE_LEADING_ZEROS, 2);
            StringExpandPlaceholders(dest, gText_NumberItem_TMBerry);
        }
        break;
    case BERRIES_POCKET:
        ConvertIntToDecimalStringN(gStringVar1, itemId - FIRST_BERRY_INDEX + 1, STR_CONV_MODE_LEADING_ZEROS, 2);
        CopyItemName(itemId, gStringVar2);
        StringExpandPlaceholders(dest, gText_NumberItem_TMBerry);
        break;
    default:
        CopyItemName(itemId, dest);
        break;
    }
}

static void BagMenu_MoveCursorCallback(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    if (gBagMenu->toSwapPos == NOT_SWAPPING)
    {
        if (onInit == TRUE)
        {
            // First time the bag opens (or after a full refresh), clear both
            // icon slots entirely.  This avoids leftover sprites when the
            // bag is opened with a nonzero slot or when returning from the PC
            // overlay.
            RemoveBagItemIconSprite(0);
            RemoveBagItemIconSprite(1);
            RemoveBagPCIconSprite(0);
            RemoveBagStartButtonSprite(0);
            gBagMenu->itemIconSlot = 0;

            if (itemIndex != LIST_CANCEL)
            {
                AddBagItemIconSprite(BagGetItemIdByPocketPosition(gBagPosition.pocket + 1, itemIndex), gBagMenu->itemIconSlot);
            }
            else
            {
                AddBagItemIconSprite(ITEM_LIST_END, gBagMenu->itemIconSlot);
            }
            // Only show the PC icon if the bag isn't opened from battle or a shop.
            // When selling items in a shop the PC icon shouldn't be visible.
            if (gBagPosition.location != ITEMMENULOCATION_BATTLE && gBagPosition.location != ITEMMENULOCATION_SHOP)
            {
                AddBagPCIconSprite(gBagMenu->itemIconSlot);
                AddBagStartButtonSprite(gBagMenu->itemIconSlot);
            }
        }
        else
        {
            PlaySE(SE_SELECT);
            ShakeBagSprite();

            // Remove any icons from either slot before drawing the new ones.
            // This guarantees the icon will update even on the first scroll
            // and stops the PC icon from flashing when the slot toggles.
            RemoveBagItemIconSprite(0);
            RemoveBagItemIconSprite(1);
            RemoveBagPCIconSprite(0);
            RemoveBagStartButtonSprite(0);
            gBagMenu->itemIconSlot = 0;

            if (itemIndex != LIST_CANCEL)
            {
                AddBagItemIconSprite(BagGetItemIdByPocketPosition(gBagPosition.pocket + 1, itemIndex), gBagMenu->itemIconSlot);
                // Don't show the PC icon when in a shop selling menu
                if (gBagPosition.location != ITEMMENULOCATION_BATTLE && gBagPosition.location != ITEMMENULOCATION_SHOP)
                {
                    AddBagPCIconSprite(gBagMenu->itemIconSlot);
                    AddBagStartButtonSprite(gBagMenu->itemIconSlot);
                }
            }
            else
            {
                AddBagItemIconSprite(ITEM_LIST_END, gBagMenu->itemIconSlot);
                // No PC icon for empty list in shop
                if (gBagPosition.location != ITEMMENULOCATION_BATTLE && gBagPosition.location != ITEMMENULOCATION_SHOP)
                {
                    AddBagPCIconSprite(gBagMenu->itemIconSlot);
                    AddBagStartButtonSprite(gBagMenu->itemIconSlot);
                }
            }
            // keep slot 0 always
        }

        if (!gBagMenu->inhibitItemDescriptionPrint)
            PrintItemDescription(itemIndex);
    }
}

static void BagMenu_ItemPrintCallback(u8 windowId, u32 itemIndex, u8 y)
{
    u16 itemId;
    u16 itemQuantity;
    int offset;

    if (itemIndex != LIST_CANCEL)
    {
        if (gBagMenu->toSwapPos != NOT_SWAPPING)
        {
            // Swapping items, draw cursor at original item's location
            if (gBagMenu->toSwapPos == (u8)itemIndex)
                BagMenu_PrintCursorAtPos(y, COLORID_GRAY_CURSOR);
            else
                BagMenu_PrintCursorAtPos(y, COLORID_NONE);
        }

        itemId = BagGetItemIdByPocketPosition(gBagPosition.pocket + 1, itemIndex);
        itemQuantity = BagGetQuantityByPocketPosition(gBagPosition.pocket + 1, itemIndex);

        // Draw HM icon
        if (itemId >= ITEM_HM01 && itemId <= ITEM_HM08)
            BlitBitmapToWindow(windowId, gBagMenuHMIcon_Gfx, 8, y - 1, 16, 16);

        if ((gBagMenu->toSwapPos != NOT_SWAPPING || gBagMenu->showItemCost) && (gBagPosition.pocket == ITEMS_POCKET || gBagPosition.pocket == BERRIES_POCKET))
        {
            // While organizing or when cost toggle is enabled, show item IP cost as small p + value in Item/Berry only
            u16 ipCost = GetItemIPCost(itemId);
            u8 numDigits = (gBagPosition.pocket == BERRIES_POCKET) ? BERRY_CAPACITY_DIGITS : BAG_ITEM_CAPACITY_DIGITS;
            ConvertIntToDecimalStringN(gStringVar1, ipCost, STR_CONV_MODE_RIGHT_ALIGN, numDigits);
            StringExpandPlaceholders(gStringVar4, gText_pVar1);
            offset = GetStringRightAlignXOffset(FONT_NARROW, gStringVar4, 119);
            BagMenu_Print(windowId, FONT_NARROW, gStringVar4, offset, y, 0, 0, TEXT_SKIP_DRAW, COLORID_NORMAL);
        }
        else if (gBagPosition.pocket == BERRIES_POCKET)
        {
            // Print berry quantity
            ConvertIntToDecimalStringN(gStringVar1, itemQuantity, STR_CONV_MODE_RIGHT_ALIGN, BERRY_CAPACITY_DIGITS);
            StringExpandPlaceholders(gStringVar4, gText_xVar1);
            offset = GetStringRightAlignXOffset(FONT_NARROW, gStringVar4, 119);
            BagMenu_Print(windowId, FONT_NARROW, gStringVar4, offset, y, 0, 0, TEXT_SKIP_DRAW, COLORID_NORMAL);
        }
        else if (gBagPosition.pocket != KEYITEMS_POCKET && GetItemImportance(itemId) == FALSE)
        {
            // Print item quantity
            ConvertIntToDecimalStringN(gStringVar1, itemQuantity, STR_CONV_MODE_RIGHT_ALIGN, BAG_ITEM_CAPACITY_DIGITS);
            StringExpandPlaceholders(gStringVar4, gText_xVar1);
            offset = GetStringRightAlignXOffset(FONT_NARROW, gStringVar4, 119);
            BagMenu_Print(windowId, FONT_NARROW, gStringVar4, offset, y, 0, 0, TEXT_SKIP_DRAW, COLORID_NORMAL);
        }
        else
        {
            // Print enabled icon for toggleable key items
            if ((itemId == ITEM_MACHO_GEAR && FlagGet(FLAG_SYS_MACHO_GEAR_ENABLED))
                || (itemId == ITEM_EXP_ALL && FlagGet(FLAG_SYS_EXP_ALL_ENABLED))
                || (itemId == ITEM_LUCKY_DOZEN && FlagGet(FLAG_SYS_LUCKY_DOZEN_ENABLED))
                || (itemId == ITEM_BONDING_CHIME && FlagGet(FLAG_SYS_BONDING_CHIME_ENABLED))
                || (itemId == ITEM_PAY_ROLL && FlagGet(FLAG_SYS_PAY_ROLL_ENABLED))
                || (itemId == ITEM_SMOKE_CLOAK && FlagGet(FLAG_SYS_SMOKE_CLOAK_ENABLED))
                || (itemId == ITEM_GEO_DUD && FlagGet(FLAG_SYS_GEO_DUD_ENABLED))
                || (itemId == ITEM_REPELLENT_FOG && FlagGet(FLAG_SYS_REPELLENT_FOG_ENABLED)))
            {
                BlitBitmapToWindow(windowId, sMachoGearOn_Gfx, 96, y - 1, 24, 16);
            }
            // Print registered item icon
            else if (gSaveBlock1Ptr->registeredItem != ITEM_NONE && gSaveBlock1Ptr->registeredItem == itemId)
            {
                BlitBitmapToWindow(windowId, sRegisteredSelect_Gfx, 96, y - 1, 24, 16);
            }
        }
    }
}

static void BagMenu_RedrawItemAmountDisplay(u8 listTaskId)
{
    struct ListMenu *list = (void *) gTasks[listTaskId].data;
    u16 itemIndex = list->scrollOffset;
    u8 i;
    u8 yOffset;
    u8 yIncrement = GetFontAttribute(list->template.fontId, FONTATTR_MAX_LETTER_HEIGHT) + list->template.itemVerticalPadding;
    u8 amountCharCount;
    u8 amountPixelWidth;
    u8 amountX;

    amountCharCount = (gBagPosition.pocket == BERRIES_POCKET) ? (BERRY_CAPACITY_DIGITS + 1) : (BAG_ITEM_CAPACITY_DIGITS + 1);
    amountPixelWidth = (GetFontAttribute(FONT_NARROW, FONTATTR_MAX_LETTER_WIDTH) + GetFontAttribute(FONT_NARROW, FONTATTR_LETTER_SPACING)) * amountCharCount + 2;
    if (amountPixelWidth > 40)
        amountPixelWidth = 40;
    amountX = 120 - amountPixelWidth;

    for (i = 0; i < list->template.maxShowed; i++)
    {
        yOffset = i * yIncrement + list->template.upText_Y;
        // Clear quantity/cost area only, so item names remain intact.
        FillWindowPixelRect(list->template.windowId, PIXEL_FILL(list->template.fillValue), amountX, yOffset, amountPixelWidth, yIncrement);

        if (list->template.items[itemIndex].id != LIST_HEADER && list->template.itemPrintFunc != NULL)
            list->template.itemPrintFunc(list->template.windowId, list->template.items[itemIndex].id, yOffset);

        itemIndex++;
    }

    CopyWindowToVram(list->template.windowId, COPYWIN_GFX);
}

// Determine whether a given value corresponds to the cancel/close-bag
// entry.  "pos" may be either a list index (0-based) or an ID obtained from
// the ListMenu item structure; the two are almost always the same but the
// cancel entry uses LIST_CANCEL as its ID.
static bool8 IsCancelEntryAtPos(int pos)
{
    if (pos == LIST_CANCEL)
        return TRUE;
    if (!gBagMenu->hideCloseBagText
        && pos >= 0
        && pos == gBagMenu->numItemStacks[gBagPosition.pocket] - 1)
        return TRUE;
    return FALSE;
}

static void PrintItemDescription(int itemIndex)
{
    const u8 *str;

    if (IsCancelEntryAtPos(itemIndex))
    {
        // Print 'Cancel' description
        StringCopy(gStringVar1, gBagMenu_ReturnToStrings[gBagPosition.location]);
        StringExpandPlaceholders(gStringVar4, gText_ReturnToVar1);
        str = gStringVar4;
    }
    else
    {
        str = GetItemDescription(BagGetItemIdByPocketPosition(gBagPosition.pocket + 1, itemIndex));
    }

    FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
    BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, str, 3, 1, 0, 0, 0, COLORID_NORMAL);
}

static void BagMenu_PrintCursor(u8 listTaskId, u8 colorIndex)
{
    BagMenu_PrintCursorAtPos(ListMenuGetYCoordForPrintingArrowCursor(listTaskId), colorIndex);
}

static void PrintIPOverlay(void)
{
    u8 xOffset;
    u16 cur = GetPlayerIP();
    u16 max = GetPlayerIPMax();

    StringCopy(gStringVar4, gText_IPColon);
    if (cur == max)
    {
        ConvertIntToDecimalStringN(gStringVar1, max, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(gStringVar4, gStringVar1);
    }
    else
    {
        ConvertIntToDecimalStringN(gStringVar1, cur, STR_CONV_MODE_LEFT_ALIGN, 3);
        ConvertIntToDecimalStringN(gStringVar2, max, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringAppend(gStringVar4, gStringVar1);
        StringAppend(gStringVar4, gText_Slash);
        StringAppend(gStringVar4, gStringVar2);
    }

    xOffset = GetStringRightAlignXOffset(FONT_SMALL, gStringVar4, 48);

    FillWindowPixelBuffer(WIN_IP_OVERLAY, PIXEL_FILL(0));
    BagMenu_Print(WIN_IP_OVERLAY, FONT_SMALL, gStringVar4, xOffset, 3, 0, 0, 0, COLORID_POCKET_NAME);
    PutWindowTilemap(WIN_IP_OVERLAY);
    CopyWindowToVram(WIN_IP_OVERLAY, COPYWIN_GFX);
}

static void PrintAPOverlay(void)
{
    u16 cur = GetPlayerAP();
    u16 max = GetPlayerAPMax();

    StringCopy(gStringVar4, gText_APColon);
    if (cur == max)
    {
        ConvertIntToDecimalStringN(gStringVar1, max, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringAppend(gStringVar4, gStringVar1);
    }
    else
    {
        ConvertIntToDecimalStringN(gStringVar1, cur, STR_CONV_MODE_LEFT_ALIGN, 2);
        ConvertIntToDecimalStringN(gStringVar2, max, STR_CONV_MODE_LEFT_ALIGN, 2);
        StringAppend(gStringVar4, gStringVar1);
        StringAppend(gStringVar4, gText_Slash);
        StringAppend(gStringVar4, gStringVar2);
    }

    FillWindowPixelBuffer(WIN_AP_OVERLAY, PIXEL_FILL(0));
    BagMenu_Print(WIN_AP_OVERLAY, FONT_SMALL, gStringVar4, 0, 3, 0, 0, 0, COLORID_POCKET_NAME);
    PutWindowTilemap(WIN_AP_OVERLAY);
    CopyWindowToVram(WIN_AP_OVERLAY, COPYWIN_GFX);
}

static void BagMenu_PrintCursorAtPos(u8 y, u8 colorIndex)
{
    if (colorIndex == COLORID_NONE)
        FillWindowPixelRect(WIN_ITEM_LIST, PIXEL_FILL(0), 0, y, GetMenuCursorDimensionByFont(FONT_NORMAL, 0), GetMenuCursorDimensionByFont(FONT_NORMAL, 1));
    else
        BagMenu_Print(WIN_ITEM_LIST, FONT_NORMAL, gText_SelectorArrow2, 0, y, 0, 0, 0, colorIndex);

}

static void CreatePocketScrollArrowPair(void)
{
    if (gBagMenu->pocketScrollArrowsTask == TASK_NONE)
        gBagMenu->pocketScrollArrowsTask = AddScrollIndicatorArrowPairParameterized(
            SCROLL_ARROW_UP,
            172,
            12,
            148,
            gBagMenu->numItemStacks[gBagPosition.pocket] - gBagMenu->numShownItems[gBagPosition.pocket],
            TAG_POCKET_SCROLL_ARROW,
            TAG_POCKET_SCROLL_ARROW,
            &gBagPosition.scrollPosition[gBagPosition.pocket]);
}

void BagDestroyPocketScrollArrowPair(void)
{
    if (gBagMenu->pocketScrollArrowsTask != TASK_NONE)
    {
        RemoveScrollIndicatorArrowPair(gBagMenu->pocketScrollArrowsTask);
        gBagMenu->pocketScrollArrowsTask = TASK_NONE;
    }
    DestroyPocketSwitchArrowPair();
}

static void CreatePocketSwitchArrowPair(void)
{
    if (gBagMenu->pocketSwitchDisabled != TRUE && gBagMenu->pocketSwitchArrowsTask == TASK_NONE)
        gBagMenu->pocketSwitchArrowsTask = AddScrollIndicatorArrowPair(&sBagScrollArrowsTemplate, &gBagPosition.pocketSwitchArrowPos);
}

static void DestroyPocketSwitchArrowPair(void)
{
    if (gBagMenu->pocketSwitchArrowsTask != TASK_NONE)
    {
        RemoveScrollIndicatorArrowPair(gBagMenu->pocketSwitchArrowsTask);
        gBagMenu->pocketSwitchArrowsTask = TASK_NONE;
    }
}

static void FreeBagMenu(void)
{
    Free(sListBuffer2);
    Free(sListBuffer1);
    FreeAllWindowBuffers();
    Free(gBagMenu);
}

void Task_FadeAndCloseBagMenu(u8 taskId)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_CloseBagMenu;
}

static void Task_CloseBagMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    if (!gPaletteFade.active)
    {
        DestroyListMenuTask(tListTaskId, &gBagPosition.scrollPosition[gBagPosition.pocket], &gBagPosition.cursorPosition[gBagPosition.pocket]);

        // If ready for a new screen (e.g. party menu for giving an item) go to that screen
        // Otherwise exit the bag and use callback set up when the bag was first opened
        if (gBagMenu->newScreenCallback != NULL)
            SetMainCallback2(gBagMenu->newScreenCallback);
        else
            SetMainCallback2(gBagPosition.exitCallback);

        BagDestroyPocketScrollArrowPair();
        ResetSpriteData();
        FreeAllSpritePalettes();
        FreeBagMenu();
        DestroyTask(taskId);
    }
}

void UpdatePocketItemList(u8 pocketId)
{
    u16 i;
    struct BagPocket *pocket = &gBagPockets[pocketId];
    switch (pocketId)
    {
    case TMHM_POCKET:
    case BERRIES_POCKET:
        SortBerriesOrTMHMs(pocket);
        break;
    default:
        CompactItemsInBagPocket(pocket);
        break;
    }

    gBagMenu->numItemStacks[pocketId] = 0;

    for (i = 0; i < pocket->capacity && pocket->itemSlots[i].itemId; i++)
        gBagMenu->numItemStacks[pocketId]++;

    if (!gBagMenu->hideCloseBagText)
        gBagMenu->numItemStacks[pocketId]++;

    if (gBagMenu->numItemStacks[pocketId] > MAX_ITEMS_SHOWN)
        gBagMenu->numShownItems[pocketId] = MAX_ITEMS_SHOWN;
    else
        gBagMenu->numShownItems[pocketId] = gBagMenu->numItemStacks[pocketId];
}

static void UpdatePocketItemLists(void)
{
    u8 i;
    for (i = 0; i < POCKETS_COUNT; i++)
        UpdatePocketItemList(i);
}

void UpdatePocketListPosition(u8 pocketId)
{
    SetCursorWithinListBounds(&gBagPosition.scrollPosition[pocketId], &gBagPosition.cursorPosition[pocketId], gBagMenu->numShownItems[pocketId], gBagMenu->numItemStacks[pocketId]);
}

static void InitPocketListPositions(void)
{
    u8 i;
    for (i = 0; i < POCKETS_COUNT; i++)
        UpdatePocketListPosition(i);
}

static void InitPocketScrollPositions(void)
{
    u8 i;
    for (i = 0; i < POCKETS_COUNT; i++)
        SetCursorScrollWithinListBounds(&gBagPosition.scrollPosition[i], &gBagPosition.cursorPosition[i], gBagMenu->numShownItems[i], gBagMenu->numItemStacks[i], MAX_ITEMS_SHOWN);
}

u8 GetItemListPosition(u8 pocketId)
{
    return gBagPosition.scrollPosition[pocketId] + gBagPosition.cursorPosition[pocketId];
}

void DisplayItemMessage(u8 taskId, u8 fontId, const u8 *str, TaskFunc callback)
{
    s16 *data = gTasks[taskId].data;

    tMsgWindowId = AddItemMessageWindow(ITEMWIN_MESSAGE);
    FillWindowPixelBuffer(tMsgWindowId, PIXEL_FILL(1));
    DisplayMessageAndContinueTask(taskId, tMsgWindowId, 10, 13, fontId, GetPlayerTextSpeedDelay(), str, callback);
    ScheduleBgCopyTilemapToVram(1);
}

void CloseItemMessage(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u16 *scrollPos = &gBagPosition.scrollPosition[gBagPosition.pocket];
    u16 *cursorPos = &gBagPosition.cursorPosition[gBagPosition.pocket];
    RemoveItemMessageWindow(ITEMWIN_MESSAGE);
    DestroyListMenuTask(tListTaskId, scrollPos, cursorPos);
    UpdatePocketItemList(gBagPosition.pocket);
    UpdatePocketListPosition(gBagPosition.pocket);
    LoadBagItemListBuffers(gBagPosition.pocket);
    tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, *scrollPos, *cursorPos);
    ScheduleBgCopyTilemapToVram(0);
    ReturnToItemList(taskId);
}

static void AddItemQuantityWindow(u8 windowType)
{
    PrintItemQuantity(BagMenu_AddWindow(windowType), 1);
}

static void PrintItemQuantity(u8 windowId, s16 quantity)
{
    u8 numDigits = (gBagPosition.pocket == BERRIES_POCKET) ? BERRY_CAPACITY_DIGITS : BAG_ITEM_CAPACITY_DIGITS;
    ConvertIntToDecimalStringN(gStringVar1, quantity, STR_CONV_MODE_LEADING_ZEROS, numDigits);
    StringExpandPlaceholders(gStringVar4, gText_xVar1);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, GetStringCenterAlignXOffset(FONT_NORMAL, gStringVar4, 0x28), 2, 0, 0);
}

// Prints the quantity of items to be sold and the amount that would be earned
static void PrintItemSoldAmount(int windowId, int numSold, int moneyEarned)
{
    u8 numDigits = (gBagPosition.pocket == BERRIES_POCKET) ? BERRY_CAPACITY_DIGITS : BAG_ITEM_CAPACITY_DIGITS;
    ConvertIntToDecimalStringN(gStringVar1, numSold, STR_CONV_MODE_LEADING_ZEROS, numDigits);
    StringExpandPlaceholders(gStringVar4, gText_xVar1);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 0, 1, TEXT_SKIP_DRAW, 0);
    PrintMoneyAmount(windowId, 38, 1, moneyEarned, 0);
}

static void Task_BagMenu_HandleInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u16 *scrollPos = &gBagPosition.scrollPosition[gBagPosition.pocket];
    u16 *cursorPos = &gBagPosition.cursorPosition[gBagPosition.pocket];
    s32 listPosition;

    if (MenuHelpers_ShouldWaitForLinkRecv() != TRUE && !gPaletteFade.active)
    {
        switch (GetSwitchBagPocketDirection())
        {
        case SWITCH_POCKET_LEFT:
            SwitchBagPocket(taskId, MENU_CURSOR_DELTA_LEFT, FALSE);
            return;
        case SWITCH_POCKET_RIGHT:
            SwitchBagPocket(taskId, MENU_CURSOR_DELTA_RIGHT, FALSE);
            return;
        default:
            if (JOY_NEW(SELECT_BUTTON))
            {
                bool8 selectProcessed = FALSE;

                ListMenuGetScrollAndRow(tListTaskId, scrollPos, cursorPos);
                if (IsCancelEntryAtPos(*scrollPos + *cursorPos))
                    break;

                if (gBagPosition.pocket == ITEMS_POCKET)
                {
                    // Items: Toggle IP and allow organizing
                    gBagMenu->showItemCost ^= 1;
                    BagMenu_RedrawItemAmountDisplay(tListTaskId);
                    selectProcessed = TRUE;
                }
                else if (gBagPosition.pocket == BERRIES_POCKET)
                {
                    // Berries: only toggle IP cost
                    gBagMenu->showItemCost ^= 1;
                    BagMenu_RedrawItemAmountDisplay(tListTaskId);
                    selectProcessed = TRUE;
                }
                else if (gBagPosition.pocket == BALLS_POCKET || gBagPosition.pocket == KEYITEMS_POCKET)
                {
                    // Pokeballs/Key items: organizing only (not available in shop)
                    if (gBagPosition.location != ITEMMENULOCATION_SHOP)
                        selectProcessed = TRUE;
                }

                if (selectProcessed)
                {
                    PlaySE(SE_SELECT);

                    if (CanSwapItems() == TRUE
                        && (gBagPosition.pocket == ITEMS_POCKET || gBagPosition.pocket == BALLS_POCKET || gBagPosition.pocket == KEYITEMS_POCKET))
                    {
                        ListMenuGetScrollAndRow(tListTaskId, scrollPos, cursorPos);
                        if ((*scrollPos + *cursorPos) != gBagMenu->numItemStacks[gBagPosition.pocket] - 1)
                        {
                            // If the select press just enabled showing item cost, remember
                            // to revert it when exiting swap mode so the toggle behaves
                            // like a momentary show during organize.
                            if (gBagPosition.pocket == ITEMS_POCKET && gBagMenu->showItemCost)
                                gBagMenu->unused1[0] = 1;
                            StartItemSwap(taskId);
                            return;
                        }
                    }

                    return;
                }
            }
            // START opens PC item storage overlay (only in field/overworld)
            if (JOY_NEW(START_BUTTON))
            {
                if (gBagPosition.location == ITEMMENULOCATION_FIELD)
                {
                    PlaySE(SE_SELECT);
                    AnimateStartButtonPress();
                    PCOverlay_Open(taskId);
                    return;
                }
            }
            break;
        }

        listPosition = ListMenu_ProcessInput(tListTaskId);
        ListMenuGetScrollAndRow(tListTaskId, scrollPos, cursorPos);
        switch (listPosition)
        {
        case LIST_NOTHING_CHOSEN:
            break;
        case LIST_CANCEL:
            if (gBagPosition.location == ITEMMENULOCATION_BERRY_BLENDER_CRUSH)
            {
                PlaySE(SE_FAILURE);
                break;
            }
            PlaySE(SE_SELECT);
            gSpecialVar_ItemId = ITEM_NONE;
            gTasks[taskId].func = Task_FadeAndCloseBagMenu;
            break;
        default: // A_BUTTON
            PlaySE(SE_SELECT);
            BagDestroyPocketScrollArrowPair();
            BagMenu_PrintCursor(tListTaskId, COLORID_GRAY_CURSOR);
            tListPosition = listPosition;
            tQuantity = BagGetQuantityByPocketPosition(gBagPosition.pocket + 1, listPosition);
            gSpecialVar_ItemId = BagGetItemIdByPocketPosition(gBagPosition.pocket + 1, listPosition);
            sContextMenuFuncs[gBagPosition.location](taskId);
            break;
        }
    }
}

static void ReturnToItemList(u8 taskId)
{
    CreatePocketScrollArrowPair();
    CreatePocketSwitchArrowPair();
    ClearWindowTilemap(WIN_TMHM_INFO_ICONS);
    ClearWindowTilemap(WIN_TMHM_INFO);
    PutWindowTilemap(WIN_DESCRIPTION);

    PrintIPOverlay();
    PrintAPOverlay();

    ScheduleBgCopyTilemapToVram(0);
    gTasks[taskId].func = Task_BagMenu_HandleInput;
}

static u8 GetSwitchBagPocketDirection(void)
{
    u8 LRKeys;
    if (gBagMenu->pocketSwitchDisabled)
        return SWITCH_POCKET_NONE;
    LRKeys = GetLRKeysPressed();
    if (JOY_NEW(DPAD_LEFT) || LRKeys == MENU_L_PRESSED)
    {
        PlaySE(SE_SELECT);
        return SWITCH_POCKET_LEFT;
    }
    if (JOY_NEW(DPAD_RIGHT) || LRKeys == MENU_R_PRESSED)
    {
        PlaySE(SE_SELECT);
        return SWITCH_POCKET_RIGHT;
    }
    return SWITCH_POCKET_NONE;
}

static void ChangeBagPocketId(u8 *bagPocketId, s8 deltaBagPocketId)
{
    if (deltaBagPocketId == MENU_CURSOR_DELTA_RIGHT && *bagPocketId == POCKETS_COUNT - 1)
        *bagPocketId = 0;
    else if (deltaBagPocketId == MENU_CURSOR_DELTA_LEFT && *bagPocketId == 0)
        *bagPocketId = POCKETS_COUNT - 1;
    else
        *bagPocketId += deltaBagPocketId;
}

static void SwitchBagPocket(u8 taskId, s16 deltaBagPocketId, bool16 skipEraseList)
{
    s16 *data = gTasks[taskId].data;
    u8 newPocket;

    tPocketSwitchState = 0;
    tPocketSwitchTimer = 0;
    tPocketSwitchDir = deltaBagPocketId;

    // If leaving Berries pocket, disable the shared IP cost overlay state.
    // This prevents the berry Select toggle from carrying over to ITEMS.
    if (gBagPosition.pocket == BERRIES_POCKET)
        gBagMenu->showItemCost = FALSE;

    if (!skipEraseList)
    {
        ClearWindowTilemap(WIN_ITEM_LIST);
        ClearWindowTilemap(WIN_DESCRIPTION);
        DestroyListMenuTask(tListTaskId, &gBagPosition.scrollPosition[gBagPosition.pocket], &gBagPosition.cursorPosition[gBagPosition.pocket]);
        ScheduleBgCopyTilemapToVram(0);
        BagDestroyPocketScrollArrowPair();
    }
    if (gBagMenu->spriteIds[ITEMMENUSPRITE_ITEM] != SPRITE_NONE)
        gSprites[gBagMenu->spriteIds[ITEMMENUSPRITE_ITEM]].invisible = TRUE;
    if (gBagMenu->spriteIds[ITEMMENUSPRITE_ITEM_ALT] != SPRITE_NONE)
        gSprites[gBagMenu->spriteIds[ITEMMENUSPRITE_ITEM_ALT]].invisible = TRUE;
    newPocket = gBagPosition.pocket;
    ChangeBagPocketId(&newPocket, deltaBagPocketId);
    if (deltaBagPocketId == MENU_CURSOR_DELTA_RIGHT)
    {
        PrintPocketNames(gPocketNamesStringsTable[gBagPosition.pocket], gPocketNamesStringsTable[newPocket]);
        CopyPocketNameToWindow(0);
    }
    else
    {
        PrintPocketNames(gPocketNamesStringsTable[newPocket], gPocketNamesStringsTable[gBagPosition.pocket]);
        CopyPocketNameToWindow(8);
    }
    DrawPocketIndicatorSquare(gBagPosition.pocket, FALSE);
    DrawPocketIndicatorSquare(newPocket, TRUE);
    FillBgTilemapBufferRect_Palette0(2, 11, 14, 2, 15, 16);
    ScheduleBgCopyTilemapToVram(2);
    SetBagVisualPocketId(newPocket, TRUE);
    RemoveBagSprite(ITEMMENUSPRITE_BALL);
    AddSwitchPocketRotatingBallSprite(deltaBagPocketId);
    SetTaskFuncWithFollowupFunc(taskId, Task_SwitchBagPocket, gTasks[taskId].func);
}

static void Task_SwitchBagPocket(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!MenuHelpers_IsLinkActive() && !IsWallysBag())
    {
        switch (GetSwitchBagPocketDirection())
        {
        case SWITCH_POCKET_LEFT:
            ChangeBagPocketId(&gBagPosition.pocket, tPocketSwitchDir);
            SwitchTaskToFollowupFunc(taskId);
            SwitchBagPocket(taskId, MENU_CURSOR_DELTA_LEFT, TRUE);
            return;
        case SWITCH_POCKET_RIGHT:
            ChangeBagPocketId(&gBagPosition.pocket, tPocketSwitchDir);
            SwitchTaskToFollowupFunc(taskId);
            SwitchBagPocket(taskId, MENU_CURSOR_DELTA_RIGHT, TRUE);
            return;
        }
    }
    switch (tPocketSwitchState)
    {
    case 0:
        DrawItemListBgRow(tPocketSwitchTimer);
        if (!(++tPocketSwitchTimer & 1))
        {
            if (tPocketSwitchDir == MENU_CURSOR_DELTA_RIGHT)
                CopyPocketNameToWindow((u8)(tPocketSwitchTimer >> 1));
            else
                CopyPocketNameToWindow((u8)(8 - (tPocketSwitchTimer >> 1)));
        }
        if (tPocketSwitchTimer == 16)
            tPocketSwitchState++;
        break;
    case 1:
        ChangeBagPocketId(&gBagPosition.pocket, tPocketSwitchDir);
        LoadBagItemListBuffers(gBagPosition.pocket);
        tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, gBagPosition.scrollPosition[gBagPosition.pocket], gBagPosition.cursorPosition[gBagPosition.pocket]);
        PutWindowTilemap(WIN_DESCRIPTION);
        PutWindowTilemap(WIN_POCKET_NAME);
        ScheduleBgCopyTilemapToVram(0);
        CreatePocketScrollArrowPair();
        CreatePocketSwitchArrowPair();
        SwitchTaskToFollowupFunc(taskId);
    }
}

// The background of the item list is a lighter color than the surrounding menu
// When the pocket is switched this lighter background is redrawn row by row
static void DrawItemListBgRow(u8 y)
{
    FillBgTilemapBufferRect_Palette0(2, 17, 14, y + 2, 15, 1);
    ScheduleBgCopyTilemapToVram(2);
}

static void DrawPocketIndicatorSquare(u8 x, bool8 isCurrentPocket)
{
    if (!isCurrentPocket)
        FillBgTilemapBufferRect_Palette0(2, 0x1017, x + 5, 3, 1, 1);
    else
        FillBgTilemapBufferRect_Palette0(2, 0x102B, x + 5, 3, 1, 1);
    ScheduleBgCopyTilemapToVram(2);
}

static bool8 CanSwapItems(void)
{
    // Swaps can only be done from the field or in battle (as opposed to while selling items, for example)
    if (gBagPosition.location == ITEMMENULOCATION_FIELD
     || gBagPosition.location == ITEMMENULOCATION_BATTLE)
    {
        // TMHMs and berries are numbered, and so may not be swapped
        if (gBagPosition.pocket != TMHM_POCKET
         && gBagPosition.pocket != BERRIES_POCKET)
            return TRUE;
    }
    return FALSE;
}

static void StartItemSwap(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    ListMenuSetTemplateField(tListTaskId, LISTFIELD_CURSORKIND, CURSOR_INVISIBLE);
    tListPosition = gBagPosition.scrollPosition[gBagPosition.pocket] + gBagPosition.cursorPosition[gBagPosition.pocket];
    gBagMenu->toSwapPos = tListPosition;
    CopyItemName(BagGetItemIdByPocketPosition(gBagPosition.pocket + 1, tListPosition), gStringVar1);
    StringExpandPlaceholders(gStringVar4, gText_MoveVar1Where);
    FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
    BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gStringVar4, 3, 1, 0, 0, 0, COLORID_NORMAL);
    UpdateItemMenuSwapLinePos(tListPosition);
    DestroyPocketSwitchArrowPair();
    // Do not rerender item names; just update quantity/IP and cursor state for swap mode.
    BagMenu_RedrawItemAmountDisplay(tListTaskId);
    gTasks[taskId].func = Task_HandleSwappingItemsInput;
}

static void Task_HandleSwappingItemsInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (MenuHelpers_ShouldWaitForLinkRecv() != TRUE)
    {
        if (JOY_NEW(SELECT_BUTTON))
        {
            PlaySE(SE_SELECT);
            CancelItemSwap(taskId);
        }
        else if (JOY_NEW(START_BUTTON))
        {
            // Send selected item to PC storage (only in field/overworld)
            if (gBagPosition.location == ITEMMENULOCATION_FIELD)
            {
                PlaySE(SE_SELECT);
                SendSwappedItemToPC(taskId);
            }
        }
        else
        {
            s32 input = ListMenu_ProcessInput(tListTaskId);
            ListMenuGetScrollAndRow(tListTaskId, &gBagPosition.scrollPosition[gBagPosition.pocket], &gBagPosition.cursorPosition[gBagPosition.pocket]);
            SetItemMenuSwapLineInvisibility(FALSE);
            UpdateItemMenuSwapLinePos(gBagPosition.cursorPosition[gBagPosition.pocket]);
            switch (input)
            {
            case LIST_NOTHING_CHOSEN:
                break;
            case LIST_CANCEL:
                PlaySE(SE_SELECT);
                if (JOY_NEW(A_BUTTON))
                    DoItemSwap(taskId);
                else
                    CancelItemSwap(taskId);
                break;
            default:
                PlaySE(SE_SELECT);
                DoItemSwap(taskId);
                break;
            }
        }
    }
}

static void DoItemSwap(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u16 *scrollPos = &gBagPosition.scrollPosition[gBagPosition.pocket];
    u16 *cursorPos = &gBagPosition.cursorPosition[gBagPosition.pocket];
    u16 realPos = (*scrollPos + *cursorPos);

    if (tListPosition == realPos || tListPosition == realPos - 1)
    {
        // Position is the same as the original, cancel
        CancelItemSwap(taskId);
    }
    else
    {
        MoveItemSlotInList(gBagPockets[gBagPosition.pocket].itemSlots, tListPosition, realPos);
        gBagMenu->toSwapPos = NOT_SWAPPING;
        DestroyListMenuTask(tListTaskId, scrollPos, cursorPos);
        if (tListPosition < realPos)
            gBagPosition.cursorPosition[gBagPosition.pocket]--;
        LoadBagItemListBuffers(gBagPosition.pocket);
        tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, *scrollPos, *cursorPos);
        SetItemMenuSwapLineInvisibility(TRUE);
        CreatePocketSwitchArrowPair();
        // If the select-initiated cost toggle was set, clear it now and
        // update the display so the toggle doesn't remain enabled.
        if (gBagMenu->unused1[0])
        {
            gBagMenu->showItemCost ^= 1;
            gBagMenu->unused1[0] = 0;
            BagMenu_RedrawItemAmountDisplay(tListTaskId);
        }
        gTasks[taskId].func = Task_BagMenu_HandleInput;
    }
}

static void CancelItemSwap(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u16 *scrollPos = &gBagPosition.scrollPosition[gBagPosition.pocket];
    u16 *cursorPos = &gBagPosition.cursorPosition[gBagPosition.pocket];

    gBagMenu->toSwapPos = NOT_SWAPPING;
    DestroyListMenuTask(tListTaskId, scrollPos, cursorPos);
    if (tListPosition < *scrollPos + *cursorPos)
        gBagPosition.cursorPosition[gBagPosition.pocket]--;
    LoadBagItemListBuffers(gBagPosition.pocket);
    tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, *scrollPos, *cursorPos);
    SetItemMenuSwapLineInvisibility(TRUE);
    CreatePocketSwitchArrowPair();
    // If the select-initiated cost toggle was set, clear it now and
    // update the display so the toggle doesn't remain enabled.
    if (gBagMenu->unused1[0])
    {
        gBagMenu->showItemCost ^= 1;
        gBagMenu->unused1[0] = 0;
        BagMenu_RedrawItemAmountDisplay(tListTaskId);
    }
    gTasks[taskId].func = Task_BagMenu_HandleInput;
}

// Send the selected swap item to PC storage
static void SendSwappedItemToPC(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u16 itemId = BagGetItemIdByPocketPosition(gBagPosition.pocket + 1, tListPosition);
    u16 quantity = BagGetQuantityByPocketPosition(gBagPosition.pocket + 1, tListPosition);

    // Set up for deposit
    gSpecialVar_ItemId = itemId;
    tQuantity = quantity;
    tItemCount = 1;

    FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
    if (GetItemImportance(itemId))
    {
        // Can't deposit important items - exit swap mode and show error
        gBagMenu->toSwapPos = NOT_SWAPPING;
        SetItemMenuSwapLineInvisibility(TRUE);
        CreatePocketSwitchArrowPair();
        if (gBagMenu->unused1[0])
        {
            gBagMenu->showItemCost ^= 1;
            gBagMenu->unused1[0] = 0;
            BagMenu_RedrawItemAmountDisplay(tListTaskId);
        }
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gText_CantStoreImportantItems, 3, 1, 0, 0, 0, COLORID_NORMAL);
        gTasks[taskId].func = WaitDepositErrorMessage;
    }
    else if (quantity == 1)
    {
        // Only 1 item, deposit directly
        TryDepositItemFromSwap(taskId);
    }
    else
    {
        // Multiple items, show quantity selection
        CopyItemName(itemId, gStringVar1);
        StringExpandPlaceholders(gStringVar4, gText_DepositHowManyVar1);
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gStringVar4, 3, 1, 0, 0, 0, COLORID_NORMAL);
        AddItemQuantityWindow(ITEMWIN_QUANTITY);
        gTasks[taskId].func = Task_ChooseHowManyToDepositFromSwap;
    }
}

static void Task_ChooseHowManyToDepositFromSwap(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (AdjustQuantityAccordingToDPadInput(&tItemCount, tQuantity) == TRUE)
    {
        PrintItemQuantity(gBagMenu->windowIds[ITEMWIN_QUANTITY], tItemCount);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        BagMenu_RemoveWindow(ITEMWIN_QUANTITY);
        TryDepositItemFromSwap(taskId);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        // Cancel - return to swap mode
        PlaySE(SE_SELECT);
        BagMenu_RemoveWindow(ITEMWIN_QUANTITY);
        CopyItemName(BagGetItemIdByPocketPosition(gBagPosition.pocket + 1, tListPosition), gStringVar1);
        StringExpandPlaceholders(gStringVar4, gText_MoveVar1Where);
        FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gStringVar4, 3, 1, 0, 0, 0, COLORID_NORMAL);
        gTasks[taskId].func = Task_HandleSwappingItemsInput;
    }
}

static void TryDepositItemFromSwap(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    // Exit swap mode
    gBagMenu->toSwapPos = NOT_SWAPPING;
    SetItemMenuSwapLineInvisibility(TRUE);
    CreatePocketSwitchArrowPair();
    if (gBagMenu->unused1[0])
    {
        gBagMenu->showItemCost ^= 1;
        gBagMenu->unused1[0] = 0;
        BagMenu_RedrawItemAmountDisplay(tListTaskId);
    }

    // Clear only the item icon; the PC icon remains until the list is
    // refreshed in Task_RemoveItemFromBag.
    RemoveBagItemIconSprite(0);
    RemoveBagItemIconSprite(1);
    gBagMenu->itemIconSlot = 0;

    FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
    if (gBagPosition.pocket == KEYITEMS_POCKET || GetItemImportance(gSpecialVar_ItemId))
    {
        // Can't deposit key items or important items
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gText_CantStoreImportantItems, 3, 1, 0, 0, 0, COLORID_NORMAL);
        gTasks[taskId].func = WaitDepositErrorMessage;
    }
    else if (AddPCItem(gSpecialVar_ItemId, tItemCount) == TRUE)
    {
        // Successfully deposited
        CopyItemName(gSpecialVar_ItemId, gStringVar1);
        ConvertIntToDecimalStringN(gStringVar2, tItemCount, STR_CONV_MODE_LEFT_ALIGN, MAX_ITEM_DIGITS);
        StringExpandPlaceholders(gStringVar4, gText_DepositedVar2Var1s);
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gStringVar4, 3, 1, 0, 0, 0, COLORID_NORMAL);
        gTasks[taskId].func = Task_RemoveItemFromBag;
    }
    else
    {
        // No room to deposit
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gText_NoRoomForItems, 3, 1, 0, 0, 0, COLORID_NORMAL);
        gTasks[taskId].func = WaitDepositErrorMessage;
    }
}

static void OpenContextMenu(u8 taskId)
{
    switch (gBagPosition.location)
    {
    case ITEMMENULOCATION_BATTLE:
    case ITEMMENULOCATION_WALLY:
        if (GetItemBattleUsage(gSpecialVar_ItemId))
        {
            gBagMenu->contextMenuItemsPtr = sContextMenuItems_BattleUse;
            gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_BattleUse);
        }
        else
        {
            gBagMenu->contextMenuItemsPtr = sContextMenuItems_Cancel;
            gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_Cancel);
        }
        break;
    case ITEMMENULOCATION_BERRY_BLENDER_CRUSH:
        gBagMenu->contextMenuItemsPtr = sContextMenuItems_BerryBlenderCrush;
        gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_BerryBlenderCrush);
        break;
    case ITEMMENULOCATION_APPRENTICE:
        if (!GetItemImportance(gSpecialVar_ItemId) && gSpecialVar_ItemId != ITEM_ENIGMA_BERRY)
        {
            gBagMenu->contextMenuItemsPtr = sContextMenuItems_Apprentice;
            gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_Apprentice);
        }
        else
        {
            gBagMenu->contextMenuItemsPtr = sContextMenuItems_Cancel;
            gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_Cancel);
        }
        break;
    case ITEMMENULOCATION_FAVOR_LADY:
        if (!GetItemImportance(gSpecialVar_ItemId) && gSpecialVar_ItemId != ITEM_ENIGMA_BERRY)
        {
            gBagMenu->contextMenuItemsPtr = sContextMenuItems_FavorLady;
            gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_FavorLady);
        }
        else
        {
            gBagMenu->contextMenuItemsPtr = sContextMenuItems_Cancel;
            gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_Cancel);
        }
        break;
    case ITEMMENULOCATION_QUIZ_LADY:
        if (!GetItemImportance(gSpecialVar_ItemId) && gSpecialVar_ItemId != ITEM_ENIGMA_BERRY)
        {
            gBagMenu->contextMenuItemsPtr = sContextMenuItems_QuizLady;
            gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_QuizLady);
        }
        else
        {
            gBagMenu->contextMenuItemsPtr = sContextMenuItems_Cancel;
            gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_Cancel);
        }
        break;
    case ITEMMENULOCATION_PARTY:
    case ITEMMENULOCATION_SHOP:
    case ITEMMENULOCATION_BERRY_TREE:
    case ITEMMENULOCATION_ITEMPC:
    default:
        if (MenuHelpers_IsLinkActive() == TRUE || InUnionRoom() == TRUE)
        {
            if (gBagPosition.pocket == KEYITEMS_POCKET || !IsHoldingItemAllowed(gSpecialVar_ItemId))
            {
                gBagMenu->contextMenuItemsPtr = sContextMenuItems_Cancel;
                gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_Cancel);
            }
            else
            {
                gBagMenu->contextMenuItemsPtr = sContextMenuItems_Give;
                gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_Give);
            }
        }
        else
        {
            switch (gBagPosition.pocket)
            {
            case ITEMS_POCKET:
                gBagMenu->contextMenuItemsPtr = gBagMenu->contextMenuItemsBuffer;
                gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_ItemsPocket);
                memcpy(&gBagMenu->contextMenuItemsBuffer, &sContextMenuItems_ItemsPocket, sizeof(sContextMenuItems_ItemsPocket));
                if (ItemIsMail(gSpecialVar_ItemId) == TRUE)
                    gBagMenu->contextMenuItemsBuffer[0] = ACTION_CHECK;
                break;
            case KEYITEMS_POCKET:
                gBagMenu->contextMenuItemsPtr = gBagMenu->contextMenuItemsBuffer;
                gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_KeyItemsPocket);
                memcpy(&gBagMenu->contextMenuItemsBuffer, &sContextMenuItems_KeyItemsPocket, sizeof(sContextMenuItems_KeyItemsPocket));
                if (gSaveBlock1Ptr->registeredItem == gSpecialVar_ItemId)
                    gBagMenu->contextMenuItemsBuffer[1] = ACTION_DESELECT;
                if (gSpecialVar_ItemId == ITEM_MACH_BIKE || gSpecialVar_ItemId == ITEM_ACRO_BIKE)
                {
                    if (TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_MACH_BIKE | PLAYER_AVATAR_FLAG_ACRO_BIKE))
                        gBagMenu->contextMenuItemsBuffer[0] = ACTION_WALK;
                }
                break;
            case BALLS_POCKET:
                gBagMenu->contextMenuItemsPtr = sContextMenuItems_BallsPocket;
                gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_BallsPocket);
                break;
            case TMHM_POCKET:
                gBagMenu->contextMenuItemsPtr = sContextMenuItems_TmHmPocket;
                gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_TmHmPocket);
                break;
            case BERRIES_POCKET:
                gBagMenu->contextMenuItemsPtr = sContextMenuItems_BerriesPocket;
                gBagMenu->contextMenuNumItems = ARRAY_COUNT(sContextMenuItems_BerriesPocket);
                break;
            }
        }
    }
    if (gBagPosition.pocket == TMHM_POCKET)
    {
        ClearWindowTilemap(WIN_DESCRIPTION);
        PrintTMHMMoveData(gSpecialVar_ItemId);
        PutWindowTilemap(WIN_TMHM_INFO_ICONS);
        PutWindowTilemap(WIN_TMHM_INFO);
        ScheduleBgCopyTilemapToVram(0);
    }
    else
    {
        CopyItemName(gSpecialVar_ItemId, gStringVar1);
        StringExpandPlaceholders(gStringVar4, gText_Var1IsSelected);
        FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gStringVar4, 3, 1, 0, 0, 0, COLORID_NORMAL);
    }
    if (gBagMenu->contextMenuNumItems == 1)
        PrintContextMenuItems(BagMenu_AddWindow(ITEMWIN_1x1));
    else if (gBagMenu->contextMenuNumItems == 2)
        PrintContextMenuItems(BagMenu_AddWindow(ITEMWIN_1x2));
    else if (gBagMenu->contextMenuNumItems == 4)
        PrintContextMenuItemGrid(BagMenu_AddWindow(ITEMWIN_2x2), 2, 2);
    else
        PrintContextMenuItemGrid(BagMenu_AddWindow(ITEMWIN_2x3), 2, 3);
}

static void PrintContextMenuItems(u8 windowId)
{
    PrintMenuActionTexts(windowId, FONT_NARROW, 8, 1, 0, 16, gBagMenu->contextMenuNumItems, sItemMenuActions, gBagMenu->contextMenuItemsPtr);
    InitMenuInUpperLeftCornerNormal(windowId, gBagMenu->contextMenuNumItems, 0);
}

static void PrintContextMenuItemGrid(u8 windowId, u8 columns, u8 rows)
{
    PrintMenuActionGrid(windowId, FONT_NARROW, 8, 1, 56, columns, rows, sItemMenuActions, gBagMenu->contextMenuItemsPtr);
    InitMenuActionGrid(windowId, 56, columns, rows, 0);
}

static void Task_ItemContext_Normal(u8 taskId)
{
    OpenContextMenu(taskId);

    // Context menu width is never greater than 2 columns, so if
    // there are more than 2 items then there are multiple rows
    if (gBagMenu->contextMenuNumItems <= 2)
        gTasks[taskId].func = Task_ItemContext_SingleRow;
    else
        gTasks[taskId].func = Task_ItemContext_MultipleRows;
}

static void Task_ItemContext_SingleRow(u8 taskId)
{
    if (MenuHelpers_ShouldWaitForLinkRecv() != TRUE)
    {
        s8 selection = Menu_ProcessInputNoWrap();
        switch (selection)
        {
        case MENU_NOTHING_CHOSEN:
            break;
        case MENU_B_PRESSED:
            PlaySE(SE_SELECT);
            sItemMenuActions[ACTION_CANCEL].func.void_u8(taskId);
            break;
        default:
            PlaySE(SE_SELECT);
            sItemMenuActions[gBagMenu->contextMenuItemsPtr[selection]].func.void_u8(taskId);
            break;
        }
    }
}

static void Task_ItemContext_MultipleRows(u8 taskId)
{
    if (MenuHelpers_ShouldWaitForLinkRecv() != TRUE)
    {
        s8 cursorPos = Menu_GetCursorPos();
        if (JOY_NEW(DPAD_UP))
        {
            if (cursorPos > 0 && IsValidContextMenuPos(cursorPos - 2))
            {
                PlaySE(SE_SELECT);
                ChangeMenuGridCursorPosition(MENU_CURSOR_DELTA_NONE, MENU_CURSOR_DELTA_UP);
            }
        }
        else if (JOY_NEW(DPAD_DOWN))
        {
            if (cursorPos < (gBagMenu->contextMenuNumItems - 2) && IsValidContextMenuPos(cursorPos + 2))
            {
                PlaySE(SE_SELECT);
                ChangeMenuGridCursorPosition(MENU_CURSOR_DELTA_NONE, MENU_CURSOR_DELTA_DOWN);
            }
        }
        else if (JOY_NEW(DPAD_LEFT) || GetLRKeysPressed() == MENU_L_PRESSED)
        {
            if ((cursorPos & 1) && IsValidContextMenuPos(cursorPos - 1))
            {
                PlaySE(SE_SELECT);
                ChangeMenuGridCursorPosition(MENU_CURSOR_DELTA_LEFT, MENU_CURSOR_DELTA_NONE);
            }
        }
        else if (JOY_NEW(DPAD_RIGHT) || GetLRKeysPressed() == MENU_R_PRESSED)
        {
            if (!(cursorPos & 1) && IsValidContextMenuPos(cursorPos + 1))
            {
                PlaySE(SE_SELECT);
                ChangeMenuGridCursorPosition(MENU_CURSOR_DELTA_RIGHT, MENU_CURSOR_DELTA_NONE);
            }
        }
        else if (JOY_NEW(A_BUTTON))
        {
            PlaySE(SE_SELECT);
            sItemMenuActions[gBagMenu->contextMenuItemsPtr[cursorPos]].func.void_u8(taskId);
        }
        else if (JOY_NEW(B_BUTTON))
        {
            PlaySE(SE_SELECT);
            sItemMenuActions[ACTION_CANCEL].func.void_u8(taskId);
        }
    }
}

static bool8 IsValidContextMenuPos(s8 cursorPos)
{
    if (cursorPos < 0)
        return FALSE;
    if (cursorPos > gBagMenu->contextMenuNumItems)
        return FALSE;
    if (gBagMenu->contextMenuItemsPtr[cursorPos] == ACTION_DUMMY)
        return FALSE;
    return TRUE;
}

static void RemoveContextWindow(void)
{
    if (gBagMenu->contextMenuNumItems == 1)
        BagMenu_RemoveWindow(ITEMWIN_1x1);
    else if (gBagMenu->contextMenuNumItems == 2)
        BagMenu_RemoveWindow(ITEMWIN_1x2);
    else if (gBagMenu->contextMenuNumItems == 4)
        BagMenu_RemoveWindow(ITEMWIN_2x2);
    else
        BagMenu_RemoveWindow(ITEMWIN_2x3);
}

static void ItemMenu_UseOutOfBattle(u8 taskId)
{
    if (GetItemFieldFunc(gSpecialVar_ItemId))
    {
        RemoveContextWindow();
        if (CalculatePlayerPartyCount() == 0 && GetItemType(gSpecialVar_ItemId) == ITEM_USE_PARTY_MENU)
        {
            PrintThereIsNoPokemon(taskId);
        }
        else
        {
            FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
            ScheduleBgCopyTilemapToVram(0);
            if (gBagPosition.pocket != BERRIES_POCKET)
                GetItemFieldFunc(gSpecialVar_ItemId)(taskId);
            else
                ItemUseOutOfBattle_Berry(taskId);
        }
    }
}

static void ItemMenu_Toss(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    RemoveContextWindow();
    tItemCount = 1;
    if (tQuantity == 1)
    {
        AskTossItems(taskId);
    }
    else
    {
        CopyItemName(gSpecialVar_ItemId, gStringVar1);
        StringExpandPlaceholders(gStringVar4, gText_TossHowManyVar1s);
        FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gStringVar4, 3, 1, 0, 0, 0, COLORID_NORMAL);
        AddItemQuantityWindow(ITEMWIN_QUANTITY);
        gTasks[taskId].func = Task_ChooseHowManyToToss;
    }
}

static void AskTossItems(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    CopyItemName(gSpecialVar_ItemId, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, tItemCount, STR_CONV_MODE_LEFT_ALIGN, MAX_ITEM_DIGITS);
    StringExpandPlaceholders(gStringVar4, gText_ConfirmTossItems);
    FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
    BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gStringVar4, 3, 1, 0, 0, 0, COLORID_NORMAL);
    BagMenu_YesNo(taskId, ITEMWIN_YESNO_LOW, &sYesNoTossFunctions);
}

static void CancelToss(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    PrintItemDescription(tListPosition);
    BagMenu_PrintCursor(tListTaskId, COLORID_NORMAL);
    ReturnToItemList(taskId);
}

static void Task_ChooseHowManyToToss(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (AdjustQuantityAccordingToDPadInput(&tItemCount, tQuantity) == TRUE)
    {
        PrintItemQuantity(gBagMenu->windowIds[ITEMWIN_QUANTITY], tItemCount);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        BagMenu_RemoveWindow(ITEMWIN_QUANTITY);
        AskTossItems(taskId);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        BagMenu_RemoveWindow(ITEMWIN_QUANTITY);
        CancelToss(taskId);
    }
}

static void ConfirmToss(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    CopyItemName(gSpecialVar_ItemId, gStringVar1);
    ConvertIntToDecimalStringN(gStringVar2, tItemCount, STR_CONV_MODE_LEFT_ALIGN, MAX_ITEM_DIGITS);
    StringExpandPlaceholders(gStringVar4, gText_ThrewAwayVar2Var1s);
    FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
    BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gStringVar4, 3, 1, 0, 0, 0, COLORID_NORMAL);
    gTasks[taskId].func = Task_RemoveItemFromBag;
}

// Remove selected item(s) from the bag and update list
// For when items are tossed or deposited
static void Task_RemoveItemFromBag(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u16 *scrollPos = &gBagPosition.scrollPosition[gBagPosition.pocket];
    u16 *cursorPos = &gBagPosition.cursorPosition[gBagPosition.pocket];

    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        // just before rebuilding the bag, clear any lingering sprites so
        // the new item can be drawn without overwriting the PC icon.  The
        // PC icon (and any item icon) may still be present from before the
        // message; removing them here prevents tag/palette conflicts.
        RemoveBagItemIconSprite(0);
        RemoveBagItemIconSprite(1);
        RemoveBagPCIconSprite(0);
        RemoveBagStartButtonSprite(0);
        gBagMenu->itemIconSlot = 0;

        RemoveBagItem(gSpecialVar_ItemId, tItemCount);
        DestroyListMenuTask(tListTaskId, scrollPos, cursorPos);
        UpdatePocketItemList(gBagPosition.pocket);
        UpdatePocketListPosition(gBagPosition.pocket);
        LoadBagItemListBuffers(gBagPosition.pocket);
        tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, *scrollPos, *cursorPos);
        ScheduleBgCopyTilemapToVram(0);
        ReturnToItemList(taskId);
    }
}

static void ItemMenu_Register(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u16 *scrollPos = &gBagPosition.scrollPosition[gBagPosition.pocket];
    u16 *cursorPos = &gBagPosition.cursorPosition[gBagPosition.pocket];

    if (gSaveBlock1Ptr->registeredItem == gSpecialVar_ItemId)
        gSaveBlock1Ptr->registeredItem = ITEM_NONE;
    else
        gSaveBlock1Ptr->registeredItem = gSpecialVar_ItemId;
    DestroyListMenuTask(tListTaskId, scrollPos, cursorPos);
    LoadBagItemListBuffers(gBagPosition.pocket);
    tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, *scrollPos, *cursorPos);
    ScheduleBgCopyTilemapToVram(0);
    ItemMenu_Cancel(taskId);
}

static void ItemMenu_Give(u8 taskId)
{
    RemoveContextWindow();
    if (!IsWritingMailAllowed(gSpecialVar_ItemId))
    {
        DisplayItemMessage(taskId, FONT_NORMAL, gText_CantWriteMail, HandleErrorMessage);
    }
    else if (!GetItemImportance(gSpecialVar_ItemId))
    {
        if (CalculatePlayerPartyCount() == 0)
        {
            PrintThereIsNoPokemon(taskId);
        }
        else
        {
            gBagMenu->newScreenCallback = CB2_ChooseMonToGiveItem;
            Task_FadeAndCloseBagMenu(taskId);
        }
    }
    else
    {
        PrintItemCantBeHeld(taskId);
    }
}

static void PrintThereIsNoPokemon(u8 taskId)
{
    DisplayItemMessage(taskId, FONT_NORMAL, gText_NoPokemon, HandleErrorMessage);
}

static void PrintItemCantBeHeld(u8 taskId)
{
    CopyItemName(gSpecialVar_ItemId, gStringVar1);
    StringExpandPlaceholders(gStringVar4, gText_Var1CantBeHeld);
    DisplayItemMessage(taskId, FONT_NORMAL, gStringVar4, HandleErrorMessage);
}

static void HandleErrorMessage(u8 taskId)
{
    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseItemMessage(taskId);
    }
}

static void ItemMenu_CheckTag(u8 taskId)
{
    gBagMenu->newScreenCallback = DoBerryTagScreen;
    Task_FadeAndCloseBagMenu(taskId);
}

static void ItemMenu_Cancel(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    RemoveContextWindow();
    PrintItemDescription(tListPosition);
    ScheduleBgCopyTilemapToVram(0);
    ScheduleBgCopyTilemapToVram(1);
    BagMenu_PrintCursor(tListTaskId, COLORID_NORMAL);
    ReturnToItemList(taskId);
}

static void ItemMenu_UseInBattle(u8 taskId)
{
    u16 ipCost = GetItemIPCost(gSpecialVar_ItemId);

    if (ipCost > 0 && GetPlayerAP() == 0)
    {
        RemoveContextWindow();
        if (CurrentBattlePyramidLocation() == PYRAMID_LOCATION_NONE)
            DisplayItemMessage(taskId, FONT_NORMAL, sText_NotEnoughAP, HandleErrorMessage);
        else
            DisplayItemMessageInBattlePyramid(taskId, sText_NotEnoughAP, Task_CloseBattlePyramidBagMessage);
        return;
    }

    if (ipCost > 0 && GetPlayerIP() < ipCost)
    {
        RemoveContextWindow();
        if (CurrentBattlePyramidLocation() == PYRAMID_LOCATION_NONE)
            DisplayItemMessage(taskId, FONT_NORMAL, sText_NotEnoughIP, HandleErrorMessage);
        else
            DisplayItemMessageInBattlePyramid(taskId, sText_NotEnoughIP, Task_CloseBattlePyramidBagMessage);
        return;
    }

    if (GetItemBattleFunc(gSpecialVar_ItemId))
    {
        RemoveContextWindow();
        GetItemBattleFunc(gSpecialVar_ItemId)(taskId);
    }
}

void CB2_ReturnToBagMenuPocket(void)
{
    GoToBagMenu(ITEMMENULOCATION_LAST, POCKETS_COUNT, NULL);
}

static void Task_ItemContext_GiveToParty(u8 taskId)
{
    if (!IsWritingMailAllowed(gSpecialVar_ItemId))
    {
        DisplayItemMessage(taskId, FONT_NORMAL, gText_CantWriteMail, HandleErrorMessage);
    }
    else if (!IsHoldingItemAllowed(gSpecialVar_ItemId))
    {
        CopyItemName(gSpecialVar_ItemId, gStringVar1);
        StringExpandPlaceholders(gStringVar4, gText_Var1CantBeHeldHere);
        DisplayItemMessage(taskId, FONT_NORMAL, gStringVar4, HandleErrorMessage);
    }
    else if (gBagPosition.pocket != KEYITEMS_POCKET && !GetItemImportance(gSpecialVar_ItemId))
    {
        Task_FadeAndCloseBagMenu(taskId);
    }
    else
    {
        PrintItemCantBeHeld(taskId);
    }
}

// Selected item to give to a Pokémon in PC storage
static void Task_ItemContext_GiveToPC(u8 taskId)
{
    if (ItemIsMail(gSpecialVar_ItemId) == TRUE)
        DisplayItemMessage(taskId, FONT_NORMAL, gText_CantWriteMail, HandleErrorMessage);
    else if (gBagPosition.pocket != KEYITEMS_POCKET && !GetItemImportance(gSpecialVar_ItemId))
        gTasks[taskId].func = Task_FadeAndCloseBagMenu;
    else
        PrintItemCantBeHeld(taskId);
}

#define tUsingRegisteredKeyItem data[3] // See usage in item_use.c

bool8 UseRegisteredKeyItemOnField(void)
{
    u8 taskId;

    if (InUnionRoom() == TRUE || CurrentBattlePyramidLocation() != PYRAMID_LOCATION_NONE || InBattlePike() || InMultiPartnerRoom() == TRUE)
        return FALSE;
    HideMapNamePopUpWindow();
    HideNewMovesPopup();
    HidePickupItemPopup();
    ChangeBgY_ScreenOff(0, 0, BG_COORD_SET);
    if (gSaveBlock1Ptr->registeredItem != ITEM_NONE)
    {
        if (CheckBagHasItem(gSaveBlock1Ptr->registeredItem, 1) == TRUE)
        {
            LockPlayerFieldControls();
            FreezeObjectEvents();
            PlayerFreeze();
            StopPlayerAvatar();
            gSpecialVar_ItemId = gSaveBlock1Ptr->registeredItem;
            taskId = CreateTask(GetItemFieldFunc(gSaveBlock1Ptr->registeredItem), 8);
            gTasks[taskId].tUsingRegisteredKeyItem = TRUE;
            return TRUE;
        }
        else
        {
            gSaveBlock1Ptr->registeredItem = ITEM_NONE;
        }
    }
    ScriptContext_SetupScript(EventScript_SelectWithoutRegisteredItem);
    return TRUE;
}

#undef tUsingRegisteredKeyItem

static void Task_ItemContext_Sell(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (GetItemPrice(gSpecialVar_ItemId) == 0)
    {
        CopyItemName(gSpecialVar_ItemId, gStringVar2);
        StringExpandPlaceholders(gStringVar4, gText_CantBuyKeyItem);
        DisplayItemMessage(taskId, FONT_NORMAL, gStringVar4, CloseItemMessage);
    }
    else
    {
        tItemCount = 1;
        if (tQuantity == 1)
        {
            DisplayCurrentMoneyWindow();
            DisplaySellItemPriceAndConfirm(taskId);
        }
        else
        {
            CopyItemName(gSpecialVar_ItemId, gStringVar2);
            StringExpandPlaceholders(gStringVar4, gText_HowManyToSell);
            DisplayItemMessage(taskId, FONT_NORMAL, gStringVar4, InitSellHowManyInput);
        }
    }
}

static void DisplaySellItemPriceAndConfirm(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    ConvertIntToDecimalStringN(gStringVar1, (GetItemPrice(gSpecialVar_ItemId) / 2) * tItemCount, STR_CONV_MODE_LEFT_ALIGN, 6);
    StringExpandPlaceholders(gStringVar4, gText_ICanPayVar1);
    DisplayItemMessage(taskId, FONT_NORMAL, gStringVar4, AskSellItems);
}

static void AskSellItems(u8 taskId)
{
    BagMenu_YesNo(taskId, ITEMWIN_YESNO_HIGH, &sYesNoSellItemFunctions);
}

static void CancelSell(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    RemoveMoneyWindow();
    RemoveItemMessageWindow(ITEMWIN_MESSAGE);
    BagMenu_PrintCursor(tListTaskId, COLORID_NORMAL);
    ReturnToItemList(taskId);
}

static void InitSellHowManyInput(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u8 windowId = BagMenu_AddWindow(ITEMWIN_QUANTITY_WIDE);

    PrintItemSoldAmount(windowId, 1, (GetItemPrice(gSpecialVar_ItemId) / 2) * tItemCount);
    DisplayCurrentMoneyWindow();
    gTasks[taskId].func = Task_ChooseHowManyToSell;
}

static void Task_ChooseHowManyToSell(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (AdjustQuantityAccordingToDPadInput(&tItemCount, tQuantity) == TRUE)
    {
        PrintItemSoldAmount(gBagMenu->windowIds[ITEMWIN_QUANTITY_WIDE], tItemCount, (GetItemPrice(gSpecialVar_ItemId) / 2) * tItemCount);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        BagMenu_RemoveWindow(ITEMWIN_QUANTITY_WIDE);
        DisplaySellItemPriceAndConfirm(taskId);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        BagMenu_PrintCursor(tListTaskId, COLORID_NORMAL);
        RemoveMoneyWindow();
        BagMenu_RemoveWindow(ITEMWIN_QUANTITY_WIDE);
        RemoveItemMessageWindow(ITEMWIN_MESSAGE);
        ReturnToItemList(taskId);
    }
}

static void ConfirmSell(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    CopyItemName(gSpecialVar_ItemId, gStringVar2);
    ConvertIntToDecimalStringN(gStringVar1, (GetItemPrice(gSpecialVar_ItemId) / 2) * tItemCount, STR_CONV_MODE_LEFT_ALIGN, 6);
    StringExpandPlaceholders(gStringVar4, gText_TurnedOverVar1ForVar2);
    DisplayItemMessage(taskId, FONT_NORMAL, gStringVar4, SellItem);
}

static void SellItem(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u16 *scrollPos = &gBagPosition.scrollPosition[gBagPosition.pocket];
    u16 *cursorPos = &gBagPosition.cursorPosition[gBagPosition.pocket];

    PlaySE(SE_SHOP);
    RemoveBagItem(gSpecialVar_ItemId, tItemCount);
    AddMoney(&gSaveBlock1Ptr->money, (GetItemPrice(gSpecialVar_ItemId) / 2) * tItemCount);
    DestroyListMenuTask(tListTaskId, scrollPos, cursorPos);
    UpdatePocketItemList(gBagPosition.pocket);
    UpdatePocketListPosition(gBagPosition.pocket);
    LoadBagItemListBuffers(gBagPosition.pocket);
    tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, *scrollPos, *cursorPos);
    BagMenu_PrintCursor(tListTaskId, COLORID_GRAY_CURSOR);
    PrintMoneyAmountInMoneyBox(gBagMenu->windowIds[ITEMWIN_MONEY], GetMoney(&gSaveBlock1Ptr->money), 0);
    gTasks[taskId].func = WaitAfterItemSell;
}

static void WaitAfterItemSell(u8 taskId)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        RemoveMoneyWindow();
        CloseItemMessage(taskId);
    }
}

static void Task_ItemContext_Deposit(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    tItemCount = 1;
    if (tQuantity == 1)
    {
        TryDepositItem(taskId);
    }
    else
    {
        CopyItemName(gSpecialVar_ItemId, gStringVar1);
        StringExpandPlaceholders(gStringVar4, gText_DepositHowManyVar1);
        FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gStringVar4, 3, 1, 0, 0, 0, COLORID_NORMAL);
        AddItemQuantityWindow(ITEMWIN_QUANTITY);
        gTasks[taskId].func = Task_ChooseHowManyToDeposit;
    }
}

static void Task_ChooseHowManyToDeposit(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (AdjustQuantityAccordingToDPadInput(&tItemCount, tQuantity) == TRUE)
    {
        PrintItemQuantity(gBagMenu->windowIds[ITEMWIN_QUANTITY], tItemCount);
    }
    else if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_SELECT);
        BagMenu_RemoveWindow(ITEMWIN_QUANTITY);
        TryDepositItem(taskId);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        PrintItemDescription(tListPosition);
        BagMenu_PrintCursor(tListTaskId, COLORID_NORMAL);
        BagMenu_RemoveWindow(ITEMWIN_QUANTITY);
        ReturnToItemList(taskId);
    }
}

static void TryDepositItem(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    // Before showing the deposit message, clear the currently-selected
    // **item** icon.  The PC icon is left alone so it stays visible while
    // the message is displayed; it will be removed later when the bag is
    // refreshed in Task_RemoveItemFromBag.
    RemoveBagItemIconSprite(0);
    RemoveBagItemIconSprite(1);
    gBagMenu->itemIconSlot = 0;

    FillWindowPixelBuffer(WIN_DESCRIPTION, PIXEL_FILL(0));
    if (gBagPosition.pocket == KEYITEMS_POCKET || GetItemImportance(gSpecialVar_ItemId))
    {
        // Can't deposit key items or important items
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gText_CantStoreImportantItems, 3, 1, 0, 0, 0, COLORID_NORMAL);
        gTasks[taskId].func = WaitDepositErrorMessage;
    }
    else if (AddPCItem(gSpecialVar_ItemId, tItemCount) == TRUE)
    {
        // Successfully deposited
        CopyItemName(gSpecialVar_ItemId, gStringVar1);
        ConvertIntToDecimalStringN(gStringVar2, tItemCount, STR_CONV_MODE_LEFT_ALIGN, MAX_ITEM_DIGITS);
        StringExpandPlaceholders(gStringVar4, gText_DepositedVar2Var1s);
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gStringVar4, 3, 1, 0, 0, 0, COLORID_NORMAL);
        gTasks[taskId].func = Task_RemoveItemFromBag;
    }
    else
    {
        // No room to deposit
        BagMenu_Print(WIN_DESCRIPTION, FONT_NORMAL, gText_NoRoomForItems, 3, 1, 0, 0, 0, COLORID_NORMAL);
        gTasks[taskId].func = WaitDepositErrorMessage;
    }
}

static void WaitDepositErrorMessage(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PlaySE(SE_SELECT);
        CloseItemMessage(taskId);
    }
}

static bool8 IsWallysBag(void)
{
    if (gBagPosition.location == ITEMMENULOCATION_WALLY)
        return TRUE;
    return FALSE;
}

static void PrepareBagForWallyTutorial(void)
{
    u32 i;

    sTempWallyBag = AllocZeroed(sizeof(*sTempWallyBag));
    memcpy(sTempWallyBag->bagPocket_Items, gSaveBlock1Ptr->bagPocket_Items, sizeof(gSaveBlock1Ptr->bagPocket_Items));
    memcpy(sTempWallyBag->bagPocket_PokeBalls, gSaveBlock1Ptr->bagPocket_PokeBalls, sizeof(gSaveBlock1Ptr->bagPocket_PokeBalls));
    sTempWallyBag->pocket = gBagPosition.pocket;
    for (i = 0; i < POCKETS_COUNT; i++)
    {
        sTempWallyBag->cursorPosition[i] = gBagPosition.cursorPosition[i];
        sTempWallyBag->scrollPosition[i] = gBagPosition.scrollPosition[i];
    }
    ClearItemSlots(gSaveBlock1Ptr->bagPocket_Items, BAG_ITEMS_COUNT);
    ClearItemSlots(gSaveBlock1Ptr->bagPocket_PokeBalls, BAG_POKEBALLS_COUNT);
    ResetBagScrollPositions();
}

static void RestoreBagAfterWallyTutorial(void)
{
    u32 i;

    memcpy(gSaveBlock1Ptr->bagPocket_Items, sTempWallyBag->bagPocket_Items, sizeof(sTempWallyBag->bagPocket_Items));
    memcpy(gSaveBlock1Ptr->bagPocket_PokeBalls, sTempWallyBag->bagPocket_PokeBalls, sizeof(sTempWallyBag->bagPocket_PokeBalls));
    gBagPosition.pocket = sTempWallyBag->pocket;
    for (i = 0; i < POCKETS_COUNT; i++)
    {
        gBagPosition.cursorPosition[i] = sTempWallyBag->cursorPosition[i];
        gBagPosition.scrollPosition[i] = sTempWallyBag->scrollPosition[i];
    }
    Free(sTempWallyBag);
}

void DoWallyTutorialBagMenu(void)
{
    PrepareBagForWallyTutorial();
    AddBagItem(ITEM_POTION, 1);
    AddBagItem(ITEM_POKE_BALL, 1);
    GoToBagMenu(ITEMMENULOCATION_WALLY, ITEMS_POCKET, CB2_SetUpReshowBattleScreenAfterMenu2);
}

#define tTimer data[8]
#define WALLY_BAG_DELAY 102 // The number of frames between each action Wally takes in the bag

static void Task_WallyTutorialBagMenu(u8 taskId)
{
    s16 *data = gTasks[taskId].data;

    if (!gPaletteFade.active)
    {
        switch (tTimer)
        {
        case WALLY_BAG_DELAY * 1:
            PlaySE(SE_SELECT);
            SwitchBagPocket(taskId, MENU_CURSOR_DELTA_RIGHT, FALSE);
            tTimer++;
            break;
        case WALLY_BAG_DELAY * 2:
            PlaySE(SE_SELECT);
            BagMenu_PrintCursor(tListTaskId, COLORID_GRAY_CURSOR);
            gSpecialVar_ItemId = ITEM_POKE_BALL;
            OpenContextMenu(taskId);
            tTimer++;
            break;
        case WALLY_BAG_DELAY * 3:
            PlaySE(SE_SELECT);
            RemoveContextWindow();
            DestroyListMenuTask(tListTaskId, 0, 0);
            RestoreBagAfterWallyTutorial();
            Task_FadeAndCloseBagMenu(taskId);
            break;
        default:
            tTimer++;
            break;
        }
    }
}

#undef tTimer

// This action is used to show the Apprentice an item when
// they ask what item they should make their Pokémon hold
static void ItemMenu_Show(u8 taskId)
{
    gSpecialVar_0x8005 = gSpecialVar_ItemId;
    gSpecialVar_Result = TRUE;
    RemoveContextWindow();
    Task_FadeAndCloseBagMenu(taskId);
}

static void CB2_ApprenticeExitBagMenu(void)
{
    gFieldCallback = Apprentice_ScriptContext_Enable;
    SetMainCallback2(CB2_ReturnToField);
}

static void ItemMenu_GiveFavorLady(u8 taskId)
{
    RemoveBagItem(gSpecialVar_ItemId, 1);
    gSpecialVar_Result = TRUE;
    RemoveContextWindow();
    Task_FadeAndCloseBagMenu(taskId);
}

static void CB2_FavorLadyExitBagMenu(void)
{
    gFieldCallback = FieldCallback_FavorLadyEnableScriptContexts;
    SetMainCallback2(CB2_ReturnToField);
}

// This action is used to confirm which item to use as
// a prize for a custom quiz with the Lilycove Quiz Lady
static void ItemMenu_ConfirmQuizLady(u8 taskId)
{
    gSpecialVar_Result = TRUE;
    RemoveContextWindow();
    Task_FadeAndCloseBagMenu(taskId);
}

static void CB2_QuizLadyExitBagMenu(void)
{
    gFieldCallback = FieldCallback_QuizLadyEnableScriptContexts;
    SetMainCallback2(CB2_ReturnToField);
}

static void PrintPocketNames(const u8 *pocketName1, const u8 *pocketName2)
{
    struct WindowTemplate window = {0};
    u16 windowId;
    int offset;

    window.width = 16;
    window.height = 2;
    windowId = AddWindow(&window);
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
    offset = GetStringCenterAlignXOffset(FONT_NORMAL, pocketName1, 0x40);
    BagMenu_Print(windowId, FONT_NORMAL, pocketName1, offset, 1, 0, 0, TEXT_SKIP_DRAW, COLORID_POCKET_NAME);
    if (pocketName2)
    {
        offset = GetStringCenterAlignXOffset(FONT_NORMAL, pocketName2, 0x40);
        BagMenu_Print(windowId, FONT_NORMAL, pocketName2, offset + 0x40, 1, 0, 0, TEXT_SKIP_DRAW, COLORID_POCKET_NAME);
    }

    CpuCopy32((u8 *)GetWindowAttribute(windowId, WINDOW_TILE_DATA), gBagMenu->pocketNameBuffer, sizeof(gBagMenu->pocketNameBuffer));
    RemoveWindow(windowId);
}

static void CopyPocketNameToWindow(u32 a)
{
    u8 (*tileDataBuffer)[32][32];
    u8 *windowTileData;
    int b;
    if (a > 8)
        a = 8;
    tileDataBuffer = &gBagMenu->pocketNameBuffer;
    windowTileData = (u8 *)GetWindowAttribute(2, WINDOW_TILE_DATA);
    CpuCopy32(&tileDataBuffer[0][a], windowTileData, 0x100); // Top half of pocket name
    b = a + 16;
    CpuCopy32(&tileDataBuffer[0][b], windowTileData + 0x100, 0x100); // Bottom half of pocket name
    CopyWindowToVram(WIN_POCKET_NAME, COPYWIN_GFX);
}

static void LoadBagMenuTextWindows(void)
{
    u8 i;

    InitWindows(sDefaultBagWindows);
    DeactivateAllTextPrinters();
    LoadUserWindowBorderGfx(0, 1, BG_PLTT_ID(14));
    LoadMessageBoxGfx(0, 10, BG_PLTT_ID(13));
    ListMenuLoadStdPalAt(BG_PLTT_ID(12), 1);
    LoadPalette(&gStandardMenuPalette, BG_PLTT_ID(15), PLTT_SIZE_4BPP);
    for (i = 0; i <= WIN_AP_OVERLAY; i++)
    {
        FillWindowPixelBuffer(i, PIXEL_FILL(0));
        PutWindowTilemap(i);
    }
    ScheduleBgCopyTilemapToVram(0);
    ScheduleBgCopyTilemapToVram(1);

    PrintIPOverlay();
    PrintAPOverlay();
}

static void BagMenu_Print(u8 windowId, u8 fontId, const u8 *str, u8 left, u8 top, u8 letterSpacing, u8 lineSpacing, u8 speed, u8 colorIndex)
{
    AddTextPrinterParameterized4(windowId, fontId, left, top, letterSpacing, lineSpacing, sFontColorTable[colorIndex], speed, str);
}

static u8 UNUSED BagMenu_GetWindowId(u8 windowType)
{
    return gBagMenu->windowIds[windowType];
}

static u8 BagMenu_AddWindow(u8 windowType)
{
    u8 *windowId = &gBagMenu->windowIds[windowType];
    if (*windowId == WINDOW_NONE)
    {
        *windowId = AddWindow(&sContextMenuWindowTemplates[windowType]);
        DrawStdFrameWithCustomTileAndPalette(*windowId, FALSE, 1, 14);
        ScheduleBgCopyTilemapToVram(1);
    }
    return *windowId;
}

static void BagMenu_RemoveWindow(u8 windowType)
{
    u8 *windowId = &gBagMenu->windowIds[windowType];
    if (*windowId != WINDOW_NONE)
    {
        ClearStdWindowAndFrameToTransparent(*windowId, FALSE);
        ClearWindowTilemap(*windowId);
        RemoveWindow(*windowId);
        ScheduleBgCopyTilemapToVram(1);
        *windowId = WINDOW_NONE;
    }
}

static u8 AddItemMessageWindow(u8 windowType)
{
    u8 *windowId = &gBagMenu->windowIds[windowType];
    if (*windowId == WINDOW_NONE)
        *windowId = AddWindow(&sContextMenuWindowTemplates[windowType]);
    return *windowId;
}

static void RemoveItemMessageWindow(u8 windowType)
{
    u8 *windowId = &gBagMenu->windowIds[windowType];
    if (*windowId != WINDOW_NONE)
    {
        ClearDialogWindowAndFrameToTransparent(*windowId, FALSE);
        // This ClearWindowTilemap call is redundant, since ClearDialogWindowAndFrameToTransparent already calls it.
        ClearWindowTilemap(*windowId);
        RemoveWindow(*windowId);
        ScheduleBgCopyTilemapToVram(1);
        *windowId = WINDOW_NONE;
    }
}

void BagMenu_YesNo(u8 taskId, u8 windowType, const struct YesNoFuncTable *funcTable)
{
    CreateYesNoMenuWithCallbacks(taskId, &sContextMenuWindowTemplates[windowType], 1, 0, 2, 1, 14, funcTable);
}

static void DisplayCurrentMoneyWindow(void)
{
    u8 windowId = BagMenu_AddWindow(ITEMWIN_MONEY);
    PrintMoneyAmountInMoneyBoxWithBorder(windowId, 1, 14, GetMoney(&gSaveBlock1Ptr->money));
    AddMoneyLabelObject(19, 11);
}

static void RemoveMoneyWindow(void)
{
    BagMenu_RemoveWindow(ITEMWIN_MONEY);
    RemoveMoneyLabelObject();
}

static void PrepareTMHMMoveWindow(void)
{
    FillWindowPixelBuffer(WIN_TMHM_INFO_ICONS, PIXEL_FILL(0));
    BlitMenuInfoIcon(WIN_TMHM_INFO_ICONS, MENU_INFO_ICON_TYPE, 0, 0);
    BlitMenuInfoIcon(WIN_TMHM_INFO_ICONS, MENU_INFO_ICON_POWER, 0, 12);
    BlitMenuInfoIcon(WIN_TMHM_INFO_ICONS, MENU_INFO_ICON_ACCURACY, 0, 24);
    BlitMenuInfoIcon(WIN_TMHM_INFO_ICONS, MENU_INFO_ICON_PP, 0, 36);
    CopyWindowToVram(WIN_TMHM_INFO_ICONS, COPYWIN_GFX);
}

static void PrintTMHMMoveData(u16 itemId)
{
    u8 i;
    u16 move;
    const u8 *text;

    FillWindowPixelBuffer(WIN_TMHM_INFO, PIXEL_FILL(0));
    if (itemId == ITEM_NONE)
    {
        for (i = 0; i < 4; i++)
            BagMenu_Print(WIN_TMHM_INFO, FONT_NORMAL, gText_ThreeDashes, 7, i * 12, 0, 0, TEXT_SKIP_DRAW, COLORID_TMHM_INFO);
        CopyWindowToVram(WIN_TMHM_INFO, COPYWIN_GFX);
    }
    else
    {
        move = ItemIdToBattleMoveId(itemId);
        BlitMenuInfoIcon(WIN_TMHM_INFO, gBattleMoves[move].type + 1, 0, 0);

        // Print TMHM power
        if (gBattleMoves[move].power <= 1)
        {
            text = gText_ThreeDashes;
        }
        else
        {
            ConvertIntToDecimalStringN(gStringVar1, gBattleMoves[move].power, STR_CONV_MODE_RIGHT_ALIGN, 3);
            text = gStringVar1;
        }
        BagMenu_Print(WIN_TMHM_INFO, FONT_NORMAL, text, 7, 12, 0, 0, TEXT_SKIP_DRAW, COLORID_TMHM_INFO);

        // Print TMHM accuracy
        if (gBattleMoves[move].accuracy == 0)
        {
            text = gText_ThreeDashes;
        }
        else
        {
            ConvertIntToDecimalStringN(gStringVar1, gBattleMoves[move].accuracy, STR_CONV_MODE_RIGHT_ALIGN, 3);
            text = gStringVar1;
        }
        BagMenu_Print(WIN_TMHM_INFO, FONT_NORMAL, text, 7, 24, 0, 0, TEXT_SKIP_DRAW, COLORID_TMHM_INFO);

        // Print TMHM pp
        ConvertIntToDecimalStringN(gStringVar1, gBattleMoves[move].pp, STR_CONV_MODE_RIGHT_ALIGN, 3);
        BagMenu_Print(WIN_TMHM_INFO, FONT_NORMAL, gStringVar1, 7, 36, 0, 0, TEXT_SKIP_DRAW, COLORID_TMHM_INFO);

        CopyWindowToVram(WIN_TMHM_INFO, COPYWIN_GFX);
    }
}

// ==========================================
// PC Storage Overlay Functions
// ==========================================

static void PCOverlay_Init(void)
{
    sPCOverlay = AllocZeroed(sizeof(*sPCOverlay));
    memset(sPCOverlay->windowIds, WINDOW_NONE, PC_OVERLAY_WIN_COUNT);
    sPCOverlay->spriteId = SPRITE_NONE;
    sPCOverlay->scrollIndicatorTaskId = TASK_NONE;
}

static void PCOverlay_Free(void)
{
    u32 i;
    for (i = 0; i < PC_OVERLAY_WIN_COUNT; i++)
        PCOverlay_RemoveWindow(i);
    Free(sPCOverlay);
    sPCOverlay = NULL;
}

static u8 PCOverlay_AddWindow(u8 i)
{
    u8 *windowIdLoc = &sPCOverlay->windowIds[i];
    if (*windowIdLoc == WINDOW_NONE)
    {
        *windowIdLoc = AddWindow(&sPCOverlayWindowTemplates[i]);
        DrawStdFrameWithCustomTileAndPalette(*windowIdLoc, FALSE, 1, 14);
        FillWindowPixelBuffer(*windowIdLoc, PIXEL_FILL(1));
        ScheduleBgCopyTilemapToVram(1);
    }
    return *windowIdLoc;
}

static void PCOverlay_RemoveWindow(u8 i)
{
    u8 *windowIdLoc = &sPCOverlay->windowIds[i];
    if (*windowIdLoc != WINDOW_NONE)
    {
        ClearStdWindowAndFrameToTransparent(*windowIdLoc, FALSE);
        ClearWindowTilemap(*windowIdLoc);
        ScheduleBgCopyTilemapToVram(1);
        RemoveWindow(*windowIdLoc);
        *windowIdLoc = WINDOW_NONE;
    }
}

static void PCOverlay_Open(u8 taskId)
{
    u8 usedSlots = CountUsedPCItemSlots();

    if (usedSlots == 0)
    {
        // No items in PC.  Hide the *bottom* scroll arrow only so it doesn't
        // draw over the message window.  Leave the left/right pocket switch
        // arrows alone.
        if (gBagMenu->pocketScrollArrowsTask != TASK_NONE)
        {
            RemoveScrollIndicatorArrowPair(gBagMenu->pocketScrollArrowsTask);
            gBagMenu->pocketScrollArrowsTask = TASK_NONE;
        }
        DisplayItemMessage(taskId, FONT_NORMAL, gText_NoItems, CloseItemMessage);
        return;
    }

    // Play PC open sound effect (same as Pokémon PC)
    PlaySE(SE_PC_LOGIN);

    // Initialize overlay
    PCOverlay_Init();
    sPCOverlay->cursorPos = 0;
    sPCOverlay->itemsAbove = 0;
    sPCOverlay->count = usedSlots + 1; // +1 for Cancel
    
    if (sPCOverlay->count > PC_OVERLAY_MAX_SHOWN)
        sPCOverlay->pageItems = PC_OVERLAY_MAX_SHOWN;
    else
        sPCOverlay->pageItems = sPCOverlay->count;
    
    // Hide bag pocket switch arrows
    DestroyPocketSwitchArrowPair();
    BagDestroyPocketScrollArrowPair();
    
    // Create the list menu
    gTasks[taskId].func = PCOverlay_CreateListMenu;
}

static void PCOverlay_RefreshListMenu(void)
{
    u16 i;
    u8 usedSlots = CountUsedPCItemSlots();
    
    // Copy item names for all entries but the last (which is Cancel)
    for (i = 0; i < usedSlots; i++)
    {
        CopyItemName(gSaveBlock1Ptr->pcItems[i].itemId, &sPCOverlay->itemNames[i][0]);
        sPCOverlay->listItems[i].name = &sPCOverlay->itemNames[i][0];
        sPCOverlay->listItems[i].id = i;
    }
    
    // Set up Cancel entry
    StringCopy(&sPCOverlay->itemNames[i][0], gText_Cancel2);
    sPCOverlay->listItems[i].name = &sPCOverlay->itemNames[i][0];
    sPCOverlay->listItems[i].id = LIST_CANCEL;
    
    // Set list menu data
    gMultiuseListMenuTemplate = sPCOverlayListMenuTemplate;
    gMultiuseListMenuTemplate.windowId = PCOverlay_AddWindow(PC_OVERLAY_WIN_LIST);
    gMultiuseListMenuTemplate.totalItems = sPCOverlay->count;
    gMultiuseListMenuTemplate.items = sPCOverlay->listItems;
    gMultiuseListMenuTemplate.maxShowed = sPCOverlay->pageItems;
}

static void PCOverlay_CreateListMenu(u8 taskId)
{
    u32 i;
    u32 x;
    
    // Add title and message windows
    for (i = 0; i <= PC_OVERLAY_WIN_TITLE; i++)
        PCOverlay_AddWindow(i);
    
    // Print title
    x = GetStringCenterAlignXOffset(FONT_NORMAL, gText_WithdrawItem, 88);
    AddTextPrinterParameterized(sPCOverlay->windowIds[PC_OVERLAY_WIN_TITLE], FONT_NORMAL, gText_WithdrawItem, x, 1, 0, NULL);
    CopyWindowToVram(sPCOverlay->windowIds[PC_OVERLAY_WIN_ICON], COPYWIN_GFX);
    
    // Update count and cursor
    sPCOverlay->count = CountUsedPCItemSlots() + 1;
    if (sPCOverlay->count > PC_OVERLAY_MAX_SHOWN)
        sPCOverlay->pageItems = PC_OVERLAY_MAX_SHOWN;
    else
        sPCOverlay->pageItems = sPCOverlay->count;
    
    SetCursorWithinListBounds(&sPCOverlay->itemsAbove, &sPCOverlay->cursorPos, sPCOverlay->pageItems, sPCOverlay->count);
    
    PCOverlay_RefreshListMenu();
    sPCOverlay->listTaskId = ListMenuInit(&gMultiuseListMenuTemplate, sPCOverlay->itemsAbove, sPCOverlay->cursorPos);
    PCOverlay_AddScrollIndicator();
    ScheduleBgCopyTilemapToVram(1);
    gTasks[taskId].func = PCOverlay_ProcessInput;
}

static void PCOverlay_AddScrollIndicator(void)
{
    if (sPCOverlay->scrollIndicatorTaskId == TASK_NONE)
        sPCOverlay->scrollIndicatorTaskId = AddScrollIndicatorArrowPairParameterized(SCROLL_ARROW_UP, 176, 12, 148,
                                                                                       sPCOverlay->count - sPCOverlay->pageItems,
                                                                                       PC_OVERLAY_TAG_SCROLL_ARROW,
                                                                                       PC_OVERLAY_TAG_SCROLL_ARROW,
                                                                                       &sPCOverlay->itemsAbove);
}

static void PCOverlay_RemoveScrollIndicator(void)
{
    if (sPCOverlay->scrollIndicatorTaskId != TASK_NONE)
    {
        RemoveScrollIndicatorArrowPair(sPCOverlay->scrollIndicatorTaskId);
        sPCOverlay->scrollIndicatorTaskId = TASK_NONE;
    }
}

static void PCOverlay_MoveCursor(s32 id, bool8 onInit, struct ListMenu *list)
{
    if (onInit != TRUE)
        PlaySE(SE_SELECT);
    
    PCOverlay_EraseItemIcon();
    if (id != LIST_CANCEL)
        PCOverlay_DrawItemIcon(gSaveBlock1Ptr->pcItems[id].itemId);
    else
        PCOverlay_DrawItemIcon(ITEM_LIST_END);
    PCOverlay_PrintDescription(id);
}

static void PCOverlay_PrintMenuItem(u8 windowId, u32 id, u8 yOffset)
{
    if (id != LIST_CANCEL)
    {
        ConvertIntToDecimalStringN(gStringVar1, gSaveBlock1Ptr->pcItems[id].quantity, STR_CONV_MODE_RIGHT_ALIGN, 3);
        StringExpandPlaceholders(gStringVar4, gText_xVar1);
        AddTextPrinterParameterized(windowId, FONT_NARROW, gStringVar4, GetStringRightAlignXOffset(FONT_NARROW, gStringVar4, 120), yOffset, TEXT_SKIP_DRAW, NULL);
    }
}

static void PCOverlay_PrintDescription(s32 id)
{
    const u8 *description;
    u8 windowId = sPCOverlay->windowIds[PC_OVERLAY_WIN_MESSAGE];
    
    if (id != LIST_CANCEL)
        description = (u8 *)GetItemDescription(gSaveBlock1Ptr->pcItems[id].itemId);
    else
        description = gText_GoBackPrevMenu;
    
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(windowId, FONT_NORMAL, description, 0, 1, 0, NULL);
}

static void PCOverlay_PrintMessage(const u8 *string)
{
    u8 windowId = sPCOverlay->windowIds[PC_OVERLAY_WIN_MESSAGE];
    FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
    StringExpandPlaceholders(gStringVar4, string);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, 0, 1, 0, NULL);
}

static void PCOverlay_DrawItemIcon(u16 itemId)
{
    u8 spriteId;
    u8 *spriteIdLoc = &sPCOverlay->spriteId;
    
    if (*spriteIdLoc == SPRITE_NONE)
    {
        FreeSpriteTilesByTag(PC_OVERLAY_TAG_ITEM_ICON);
        FreeSpritePaletteByTag(PC_OVERLAY_TAG_ITEM_ICON);
        spriteId = AddItemIconSprite(PC_OVERLAY_TAG_ITEM_ICON, PC_OVERLAY_TAG_ITEM_ICON, itemId);
        if (spriteId != MAX_SPRITES)
        {
            *spriteIdLoc = spriteId;
            gSprites[spriteId].oam.priority = 0;
            gSprites[spriteId].x2 = 24;
            gSprites[spriteId].y2 = 80;
        }
    }
}

static void PCOverlay_EraseItemIcon(void)
{
    u8 *spriteIdLoc = &sPCOverlay->spriteId;
    if (*spriteIdLoc != SPRITE_NONE)
    {
        gSprites[*spriteIdLoc].invisible = TRUE;
        DestroySprite(&gSprites[*spriteIdLoc]);
        *spriteIdLoc = SPRITE_NONE;
    }
}

static void PCOverlay_PrintItemQuantity(u8 windowId, u16 value, u32 mode, u8 x, u8 y, u8 n)
{
    ConvertIntToDecimalStringN(gStringVar1, value, mode, n);
    StringExpandPlaceholders(gStringVar4, gText_xVar1);
    AddTextPrinterParameterized(windowId, FONT_NORMAL, gStringVar4, GetStringCenterAlignXOffset(FONT_NORMAL, gStringVar4, 48), y, 0, NULL);
}

static void PCOverlay_ProcessInput(u8 taskId)
{
    s32 id;
    
    if (JOY_NEW(B_BUTTON) || JOY_NEW(START_BUTTON))
    {
        PlaySE(SE_SELECT);
        PCOverlay_Close(taskId);
        return;
    }
    
    id = ListMenu_ProcessInput(sPCOverlay->listTaskId);
    ListMenuGetScrollAndRow(sPCOverlay->listTaskId, &sPCOverlay->itemsAbove, &sPCOverlay->cursorPos);
    
    switch (id)
    {
    case LIST_NOTHING_CHOSEN:
        break;
    case LIST_CANCEL:
        PlaySE(SE_SELECT);
        PCOverlay_Close(taskId);
        break;
    default:
        PlaySE(SE_SELECT);
        PCOverlay_DoItemAction(taskId);
        break;
    }
}

static void PCOverlay_DoItemAction(u8 taskId)
{
    u16 pos = sPCOverlay->cursorPos + sPCOverlay->itemsAbove;
    PCOverlay_RemoveScrollIndicator();
    sPCOverlay->quantity = 1;
    
    if (gSaveBlock1Ptr->pcItems[pos].quantity == 1)
    {
        // Withdrawing 1 item, do it automatically
        PCOverlay_DoItemWithdraw(taskId);
        return;
    }
    
    // Withdrawing multiple items, show "how many" message
    CopyItemName(gSaveBlock1Ptr->pcItems[pos].itemId, gStringVar1);
    PCOverlay_PrintMessage(gText_WithdrawHowManyItems);
    
    // Set up "how many" prompt
    PCOverlay_PrintItemQuantity(PCOverlay_AddWindow(PC_OVERLAY_WIN_QUANTITY), sPCOverlay->quantity, STR_CONV_MODE_LEADING_ZEROS, 8, 1, 3);
    gTasks[taskId].func = PCOverlay_HandleQuantityRolling;
}

static void PCOverlay_HandleQuantityRolling(u8 taskId)
{
    u16 pos = sPCOverlay->cursorPos + sPCOverlay->itemsAbove;
    
    if (AdjustQuantityAccordingToDPadInput(&sPCOverlay->quantity, gSaveBlock1Ptr->pcItems[pos].quantity) == TRUE)
    {
        PCOverlay_PrintItemQuantity(PCOverlay_AddWindow(PC_OVERLAY_WIN_QUANTITY), sPCOverlay->quantity, STR_CONV_MODE_LEADING_ZEROS, 8, 1, 3);
    }
    else
    {
        if (JOY_NEW(A_BUTTON))
        {
            // Quantity confirmed, perform withdraw
            PlaySE(SE_SELECT);
            PCOverlay_RemoveWindow(PC_OVERLAY_WIN_QUANTITY);
            PCOverlay_DoItemWithdraw(taskId);
        }
        else if (JOY_NEW(B_BUTTON))
        {
            // Canceled action
            PlaySE(SE_SELECT);
            PCOverlay_RemoveWindow(PC_OVERLAY_WIN_QUANTITY);
            PCOverlay_PrintDescription(pos);
            PCOverlay_ReturnToListInput(taskId);
        }
    }
}

static void PCOverlay_DoItemWithdraw(u8 taskId)
{
    u16 pos = sPCOverlay->cursorPos + sPCOverlay->itemsAbove;
    
    if (AddBagItem(gSaveBlock1Ptr->pcItems[pos].itemId, sPCOverlay->quantity) == TRUE)
    {
        // Item withdrawn
        CopyItemName(gSaveBlock1Ptr->pcItems[pos].itemId, gStringVar1);
        ConvertIntToDecimalStringN(gStringVar2, sPCOverlay->quantity, STR_CONV_MODE_LEFT_ALIGN, 3);
        PCOverlay_PrintMessage(gText_WithdrawXItems);
        gTasks[taskId].func = PCOverlay_HandleRemoveItem;
    }
    else
    {
        // No room to withdraw items
        sPCOverlay->quantity = 0;
        PCOverlay_PrintMessage(gText_NoRoomInBag);
        gTasks[taskId].func = PCOverlay_HandleErrorMessageInput;
    }
}

static void PCOverlay_HandleRemoveItem(u8 taskId)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        // Remove item from PC
        RemovePCItem(sPCOverlay->cursorPos + sPCOverlay->itemsAbove, sPCOverlay->quantity);
        
        DestroyListMenuTask(sPCOverlay->listTaskId, &sPCOverlay->itemsAbove, &sPCOverlay->cursorPos);
        
        // Compact and update list
        CompactPCItems();
        sPCOverlay->count = CountUsedPCItemSlots() + 1;
        if (sPCOverlay->count > PC_OVERLAY_MAX_SHOWN)
            sPCOverlay->pageItems = PC_OVERLAY_MAX_SHOWN;
        else
            sPCOverlay->pageItems = sPCOverlay->count;
        
        SetCursorWithinListBounds(&sPCOverlay->itemsAbove, &sPCOverlay->cursorPos, sPCOverlay->pageItems, sPCOverlay->count);
        
        PCOverlay_RefreshListMenu();
        sPCOverlay->listTaskId = ListMenuInit(&gMultiuseListMenuTemplate, sPCOverlay->itemsAbove, sPCOverlay->cursorPos);
        
        // Update ALL bag pockets since withdrawn item could go to any pocket
        UpdatePocketItemLists();
        InitPocketListPositions();
        
        PCOverlay_ReturnToListInput(taskId);
    }
}

static void PCOverlay_HandleErrorMessageInput(u8 taskId)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        PCOverlay_PrintDescription(sPCOverlay->itemsAbove + sPCOverlay->cursorPos);
        PCOverlay_ReturnToListInput(taskId);
    }
}

static void PCOverlay_ReturnToListInput(u8 taskId)
{
    PCOverlay_AddScrollIndicator();
    gTasks[taskId].func = PCOverlay_ProcessInput;
}

static void PCOverlay_Close(u8 taskId)
{
    s16 *data = gTasks[taskId].data;
    u16 *scrollPos = &gBagPosition.scrollPosition[gBagPosition.pocket];
    u16 *cursorPos = &gBagPosition.cursorPosition[gBagPosition.pocket];

    // Play PC close sound effect (same as Pokémon PC)
    PlaySE(SE_PC_OFF);
    
    // Clean up overlay
    PCOverlay_EraseItemIcon();
    PCOverlay_RemoveScrollIndicator();
    DestroyListMenuTask(sPCOverlay->listTaskId, NULL, NULL);
    PCOverlay_Free();

    // Hide any item icon sprites left in the bag so they don't flash with
    // a wrong palette while being destroyed.  The upcoming ListMenuInit
    // will call BagMenu_MoveCursorCallback(onInit=TRUE) which properly
    // removes and recreates the icons.
    if (gBagMenu->spriteIds[ITEMMENUSPRITE_ITEM] != SPRITE_NONE)
        gSprites[gBagMenu->spriteIds[ITEMMENUSPRITE_ITEM]].invisible = TRUE;
    if (gBagMenu->spriteIds[ITEMMENUSPRITE_ITEM_ALT] != SPRITE_NONE)
        gSprites[gBagMenu->spriteIds[ITEMMENUSPRITE_ITEM_ALT]].invisible = TRUE;

    // Refresh ALL bag pockets to show any newly withdrawn items
    DestroyListMenuTask(tListTaskId, scrollPos, cursorPos);
    UpdatePocketItemLists();
    InitPocketListPositions();
    LoadBagItemListBuffers(gBagPosition.pocket);
    tListTaskId = ListMenuInit(&gMultiuseListMenuTemplate, *scrollPos, *cursorPos);
    
    // Redraw the bag's description window for current item
    // Compute selected list position (scroll + cursor).  This is an index
    // into the current pocket's list; it is *not* necessarily the same value
    // as the item ID stored in the list item structure (which could be
    // LIST_CANCEL).  If we pass the raw position to PrintItemDescription it
    // will treat 0 as a valid item ID when the bag is empty, resulting in the
    // "?????" string being shown.  Convert to the proper ID here.
    tListPosition = *scrollPos + *cursorPos;

    // Determine the ID at the selected position.  When hideCloseBagText is
    // FALSE the cancel entry is appended to the end of the list and has the
    // special ID LIST_CANCEL.  Otherwise the ID is simply equal to the
    // position.
    {
        int id = tListPosition;
        if (!gBagMenu->hideCloseBagText
            && tListPosition == gBagMenu->numItemStacks[gBagPosition.pocket] - 1)
        {
            id = LIST_CANCEL;
        }
        PrintItemDescription(id);
    }
    
    // Ensure all bag windows are properly displayed
    PutWindowTilemap(WIN_ITEM_LIST);
    PutWindowTilemap(WIN_DESCRIPTION);
    PutWindowTilemap(WIN_POCKET_NAME);
    
    // Schedule BG copies to ensure proper redraw
    ScheduleBgCopyTilemapToVram(0);
    ScheduleBgCopyTilemapToVram(1);
    
    // Restore pocket arrows
    ReturnToItemList(taskId);
}

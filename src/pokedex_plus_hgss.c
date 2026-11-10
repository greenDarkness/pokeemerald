// Pokedex Plus HGSS
// HGSS-style Pokedex for pokeemerald by TheXaman (https://github.com/TheXaman/pokeemerald/tree/tx_pokedexPlus_hgss).
// Credits: TheXaman, AsparagusEduardo, DizzyEgg, Jaizu, NobodySociety.
// Forked from the vanilla src/pokedex.c; toggled with POKEDEX_PLUS_HGSS in include/config.h.
#include "global.h"
#include "battle_main.h"
#ifdef BATTLE_ENGINE
#include "battle_util.h"
#endif
#include "bg.h"
#include "contest_effect.h"
#include "data.h"
#include "daycare.h"
#include "decompress.h"
#include "event_data.h"
#include "gpu_regs.h"
#include "graphics.h"
#include "international_string_util.h"
#include "item.h"
#include "item_icon.h"
#include "main.h"
#include "malloc.h"
#include "menu.h"
#include "m4a.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokedex_plus_hgss.h"
#include "pokedex_area_screen.h"
#include "pokedex_cry_screen.h"
#include "pokemon_icon.h"
#include "pokemon_summary_screen.h"
#include "pokemon_color_variation.h"
#ifdef POKEMON_EXPANSION
#include "region_map.h"
#include "pokemon.h"
#endif
#include "reset_rtc_screen.h"
#include "scanline_effect.h"
#include "shop.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "strings.h"
#include "task.h"
#include "text_window.h"
#include "trainer_pokemon_sprites.h"
#include "trig.h"
#include "window.h"
#include "region_map.h"
#include "wild_encounter.h"
#include "hidden_grotto.h"
#include "tv.h"
#include "constants/abilities.h"
#include "constants/flags.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/party_menu.h"
#include "constants/rgb.h"
#include "constants/songs.h"
#ifdef TX_RANDOMIZER_AND_CHALLENGES
    //#include "tx_randomizer_and_challenges.h"
#endif


enum
{
    PAGE_MAIN,
    PAGE_INFO,
    PAGE_SEARCH,
    PAGE_SEARCH_RESULTS,
    PAGE_UNK,
    PAGE_AREA,
    PAGE_CRY,
    PAGE_SIZE,
    PAGE_LOCAL
};

enum
{
    INFO_SCREEN,
    STATS_SCREEN,
    EVO_SCREEN,
    FORMS_SCREEN, //Pokemonexpansion only (rhh)
    AREA_SCREEN,
    CRY_SCREEN,
    SIZE_SCREEN,
    CANCEL_SCREEN,
    SCREEN_COUNT
};

enum
{
    SEARCH_NAME,
    SEARCH_COLOR,
    SEARCH_TYPE_LEFT,
    SEARCH_TYPE_RIGHT,
    SEARCH_ORDER,
    SEARCH_MODE,
    SEARCH_OK,
    // Right-hand column (and ALL next to OK)
    SEARCH_BST_STAT,
    SEARCH_BST_OP,
    SEARCH_BST_VALUE,
    SEARCH_YIELD,
    SEARCH_MOVE_1,
    SEARCH_MOVE_2,
    SEARCH_MOVE_3,
    SEARCH_MOVE_4,
    SEARCH_ALL,
    SEARCH_COUNT
};

#define SEARCH_EXTRA_FIRST SEARCH_BST_STAT
#define SEARCH_EXTRA_COUNT (SEARCH_COUNT - SEARCH_EXTRA_FIRST)
#define IS_SEARCH_MOVE(item) ((item) >= SEARCH_MOVE_1 && (item) <= SEARCH_MOVE_4)

// BST field: stat list (0 = don't specify) and comparison operators
enum
{
    SEARCH_STAT_NONE,
    SEARCH_STAT_HP,
    SEARCH_STAT_ATK,
    SEARCH_STAT_DEF,
    SEARCH_STAT_SPATK,
    SEARCH_STAT_SPDEF,
    SEARCH_STAT_SPEED,
    SEARCH_STAT_TOTAL,
    SEARCH_STAT_COUNT
};

enum
{
    SEARCH_OP_EQUAL,
    SEARCH_OP_AT_LEAST,
    SEARCH_OP_AT_MOST,
    SEARCH_OP_LESS,
    SEARCH_OP_MORE,
    SEARCH_OP_COUNT
};

#define SEARCH_BST_DEFAULT   100
#define SEARCH_BST_MAX       255
#define SEARCH_BST_TOTAL_MAX 780

enum
{
    SEARCH_TOPBAR_SEARCH,
    SEARCH_TOPBAR_SHIFT,
    SEARCH_TOPBAR_CANCEL,
    SEARCH_TOPBAR_COUNT
};

enum
{
   ORDER_NUMERICAL,
   ORDER_ALPHABETICAL,
   ORDER_HEAVIEST,
   ORDER_LIGHTEST,
   ORDER_TALLEST,
   ORDER_SMALLEST
};

enum
{
    NAME_ABC = 1,
    NAME_DEF,
    NAME_GHI,
    NAME_JKL,
    NAME_MNO,
    NAME_PQR,
    NAME_STU,
    NAME_VWX,
    NAME_YZ,
};


extern const u8 *const gMonFootprintTable[];
extern const u16 gPokedexOrder_Alphabetical[];
extern const u16 gPokedexOrder_Height[];
extern const u16 gPokedexOrder_Weight[];
extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];
extern const struct PokedexEntry gPokedexEntries[];


static const u8 sText_No000[] = _("{NO}000");
static const u8 sCaughtBall_Gfx[] = INCBIN_U8("graphics/pokedex/caught_ball.4bpp");
static const u8 sText_TenDashes[] = _("----------");
ALIGNED(4) static const u8 sExpandedPlaceholder_PokedexDescription[] = _("");
static const u16 sSizeScreenSilhouette_Pal[] = INCBIN_U16("graphics/pokedex/size_silhouette.gbapal");
static const u8 sText_TenDashes2[] = _("----------");


#define SCROLLING_MON_X 94
#define HGSS_DECAPPED 0 //0 false, 1 true
#define HGSS_DARK_MODE 0 //0 false, 1 true
#define HGSS_HIDE_UNSEEN_EVOLUTION_NAMES 0 //0 false, 1 true



// For scrolling search parameter
#define MAX_SEARCH_PARAM_ON_SCREEN   6
#define MAX_SEARCH_PARAM_CURSOR_POS  (MAX_SEARCH_PARAM_ON_SCREEN - 1)

#define MAX_MONS_ON_SCREEN 4

#define LIST_SCROLL_STEP         16

#define POKEBALL_ROTATION_TOP    64
#define POKEBALL_ROTATION_BOTTOM (POKEBALL_ROTATION_TOP - 16)

// Coordinates of the Pokémon sprite on its page (info/cry screens)
#define MON_PAGE_X 48
#define MON_PAGE_Y 56





static EWRAM_DATA struct PokedexView *sPokedexView = NULL;
static EWRAM_DATA u16 sLastSelectedPokemon = 0;
static EWRAM_DATA u8 sPokeBallRotation = 0;
static EWRAM_DATA struct PokedexListItem *sPokedexListItem = NULL;
// Set while the Pokédex was opened by the POKéDEX key item (straight into local mode)
static EWRAM_DATA MainCallback sLocalModeExitCallback = NULL;
// While set, the Pokédex treats every Pokémon as seen and caught (used by a search with ALL on)
static EWRAM_DATA bool8 sSearchAsAllCaught = FALSE;
static void OpenPokedexPlusHGSS(void);
static void Task_LoadLocalScreen(u8 taskId);
//Pokedex Plus HGSS_Ui
// Egg moves listed on the stats page, including those inherited from up to two earlier stages
#define STATS_EGG_MOVES_MAX (EGG_MOVES_ARRAY_COUNT * 3)
#ifndef BATTLE_ENGINE
#define MOVES_COUNT_TOTAL (STATS_EGG_MOVES_MAX + MAX_LEVEL_UP_MOVES + NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES + TUTOR_MOVE_COUNT)
#else
#define MOVES_COUNT_TOTAL (STATS_EGG_MOVES_MAX + MAX_LEVEL_UP_MOVES + NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES + 20)
#endif
static u16 GetPreEvolutionSpecies(u16 species, const u16 *preEvoTable);
EWRAM_DATA static u16 sStatsMoves[MOVES_COUNT_TOTAL] = {0};
EWRAM_DATA static u16 sStatsMovesTMHM_ID[NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES] = {0};
// Level shown for each level-up move (0 = learned on evolution, shown as EVO)
EWRAM_DATA static u8 sStatsMovesLevel[MAX_LEVEL_UP_MOVES] = {0};
static const u8 sText_Stats_MoveEvo[] = _("EVO");












struct SearchOptionText
{
    const u8 *description;
    const u8 *title;
};

struct SearchOption
{
    const struct SearchOptionText *texts;
    u8 taskDataCursorPos;
    u8 taskDataScrollOffset;
    u16 numOptions;
};

struct SearchMenuTopBarItem
{
    const u8 *description;
    u8 highlightX;
    u8 highlightY;
    u8 highlightWidth;
};

struct SearchMenuItem
{
    const u8 *description;
    u8 titleBgX;
    u8 titleBgY;
    u8 titleBgWidth;
    u8 selectionBgX;
    u8 selectionBgY;
    u8 selectionBgWidth;
};

struct PokedexListItem
{
    u16 dexNum;
    u16 seen:1;
    u16 owned:1;
};











//HGSS_Ui
struct PokemonStats
{
    u16 species;
    u8  genderRatio;
    u8  baseHP;
    u8  baseSpeed;
    u8  baseAttack;
    u8  baseSpAttack;
    u8  baseDefense;
    u8  baseSpDefense;
    u8  differentEVs;
    u8  evYield_HP;
    u8  evYield_Speed;
    u8  evYield_Attack;
    u8  evYield_SpAttack;
    u8  evYield_Defense;
    u8  evYield_SpDefense;
    u8  catchRate;
    u8  growthRate;
    u8  eggGroup1;
    u8  eggGroup2;
    u8  eggCycles;
    u8  expYield;
    u8  friendship;
    u16 ability0;
    u16 ability1;
    u16 abilityHidden;
};

#define EVO_SCREEN_MAX_ENTRIES 10

struct EvoScreenData
{
    bool8 fromEvoPage;
    u8 numAllEvolutions;
    u16 targetSpecies[EVO_SCREEN_MAX_ENTRIES];
    u8 numSeen;
    bool8 seen[EVO_SCREEN_MAX_ENTRIES];
    u8 menuPos;
    u8 arrowSpriteId;
    #ifdef POKEMON_EXPANSION
    bool8 isMega;
    #endif
};

#ifdef POKEMON_EXPANSION
struct FromScreenData
{
    u8 numForms;
    u16 formIds[30];
    bool8 inSubmenu;
    u8 menuPos;
    u8 arrowSpriteId;
};
#endif

struct PokedexView
{
    struct PokedexListItem pokedexList[NATIONAL_DEX_COUNT + 1];
    u16 pokemonListCount;
    u16 selectedPokemon;
    u16 selectedPokemonBackup;
    u16 dexMode;
    u16 dexModeBackup;
    u16 dexOrder;
    u16 dexOrderBackup;
    u16 seenCount;
    u16 ownCount;
    u16 monSpriteIds[MAX_MONS_ON_SCREEN];
    u8 typeIconSpriteIds[2]; //HGSS_Ui
    u16 moveSelected; //HGSS_Ui
    u8 movesTotal; //HGSS_Ui
    u8 statBarsSpriteId; //HGSS_Ui
    u8 statBarsBgSpriteId; //HGSS_Ui
    bool8 justScrolled; //HGSS_Ui
    #ifdef BATTLE_ENGINE
    u8 splitIconSpriteId;  //HGSS_Ui Physical/Special Split from BE
    #endif
    u8 numEggMoves; //HGSS_Ui
    u8 numLevelUpMoves; //HGSS_Ui
    u8 numTMHMMoves; //HGSS_Ui
    u8 numTutorMoves; //HGSS_Ui
    u8 numPreEvolutions; //HGSS_Ui
    struct PokemonStats sPokemonStats; //HGSS_Ui
    struct EvoScreenData sEvoScreenData; //HGSS_Ui
    #ifdef POKEMON_EXPANSION
    struct FromScreenData sFormScreenData; //HGSS_Ui
    #endif
    struct PokedexListItem evoTargetListItem; //HGSS_Ui: info page target when jumping from the EVO page (keeps pokedexList intact)
    u16 natDexToSpecies[NATIONAL_DEX_COUNT + 1]; // Cache for DexNumToSpecies, see BuildNatDexToSpeciesTable
    u16 formSpecies;
    u16 selectedMonSpriteId;
    u16 pokeBallRotationStep;
    u16 pokeBallRotationBackup;
    u8 pokeBallRotation;
    u8 initialVOffset;
    u8 scrollTimer;
    u8 scrollDirection;
    s16 listVOffset;
    s16 listMovingVOffset;
    u16 scrollMonIncrement;
    u16 maxScrollTimer;
    u16 scrollSpeed;
    u16 unkArr1[4]; // Cleared, never read
    u8 filler[8];
    u8 currentPage;
    u8 currentPageBackup;
    bool8 isSearchResults:1;
    u8 selectedScreen;
    u8 screenSwitchState;
    u8 menuIsOpen;
    u16 menuCursorPos;
    s16 menuY;     //Menu Y position (inverted because we use REG_BG0VOFS for this)
    u8 unkArr2[8]; // Cleared, never read
    u8 unkArr3[8]; // Cleared, never read
    // Search screen fields beyond the task data: picker positions for the right-hand column,
    // the BST number, ALL, and every move sorted by name for the MOVE picker
    u16 searchCursor[SEARCH_EXTRA_COUNT];
    u16 searchScroll[SEARCH_EXTRA_COUNT];
    u16 searchBstValue;
    bool8 searchAll;
    u16 searchMoveOrder[MOVES_COUNT];
};

















static void ResetPokedexView(struct PokedexView *pokedexView);
static void BuildNatDexToSpeciesTable(void);
static u16 DexNumToSpecies(u16 dexNum);
static void VBlankCB_Pokedex(void);
static void CB2_Pokedex(void);
static void Task_OpenPokedexMainPage(u8);
static void Task_HandlePokedexInput(u8);
static void Task_WaitForScroll(u8);
static void Task_HandlePokedexStartMenuInput(u8);
static u16 GetPokedexDisplayCount(bool8 hoenn, u8 caseId);
static const u32 *GetStartMenuMainTilemap(void);
static void Task_OpenLocalScreen(u8 taskId);
static void Task_OpenInfoScreenAfterMonMovement(u8);
static void Task_WaitForExitInfoScreen(u8);
static void Task_WaitForExitSearch(u8);
static void Task_ClosePokedex(u8);
static void Task_OpenSearchResults(u8);
static void Task_HandleSearchResultsInput(u8);
static void Task_WaitForSearchResultsScroll(u8);
static void Task_HandleSearchResultsStartMenuInput(u8);
static void Task_OpenSearchResultsInfoScreenAfterMonMovement(u8);
static void Task_WaitForExitSearchResultsInfoScreen(u8);
static void Task_ReturnToPokedexFromSearchResults(u8);
static void Task_ClosePokedexFromSearchResultsStartMenu(u8);
static bool8 LoadPokedexListPage(u8);
static void LoadPokedexBgPalette(bool8);
static void FreeWindowAndBgBuffers(void);
static void CreatePokedexList(u8, u8);
static void CreateMonDexNum(u16, u8, u8, u16);
static void CreateCaughtBall(u16, u8, u8, u16);
static u8 CreateMonName(u16, u8, u8);
static void ClearMonListEntry(u8 x, u8 y, u16 unused);
static void CreateMonSpritesAtPos(u16, u16);
static bool8 UpdateDexListScroll(u8, u8, u8);
static u16 TryDoPokedexScroll(u16, u16);
static void UpdateSelectedMonSpriteId(void);
static bool8 TryDoInfoScreenScroll(void);
static u8 ClearMonSprites(void);
static u16 GetPokemonSpriteToDisplay(u16);
static u32 CreatePokedexMonSprite(u16, s16, s16);
static void CreateInterfaceSprites(u8);
static void SpriteCB_MoveMonForInfoScreen(struct Sprite *sprite);
static void SpriteCB_Scrollbar(struct Sprite *sprite);
static void SpriteCB_ScrollArrow(struct Sprite *sprite);
static void SpriteCB_DexListInterfaceText(struct Sprite *sprite);
static void SpriteCB_RotatingPokeBall(struct Sprite *sprite);
static void SpriteCB_SeenOwnInfo(struct Sprite *sprite);
static void SpriteCB_DexListStartMenuCursor(struct Sprite *sprite);
static void SpriteCB_PokedexListMonSprite(struct Sprite *sprite);
static u8 LoadInfoScreen(struct PokedexListItem *, u8 monSpriteId);
static bool8 IsInfoScreenScrolling(u8);
static u8 StartInfoScreenScroll(struct PokedexListItem *, u8);
static void Task_LoadInfoScreen(u8);
static void Task_HandleInfoScreenInput(u8);
static void Task_SwitchScreensFromInfoScreen(u8);
static void Task_LoadInfoScreenWaitForFade(u8);
static void Task_ExitInfoScreen(u8);
static void Task_LoadAreaScreen(u8);
static void Task_WaitForAreaScreenInput(u8 taskId);
static void Task_SwitchScreensFromAreaScreen(u8);
static void Task_LoadCryScreen(u8);
static void Task_HandleCryScreenInput(u8);
static void Task_SwitchScreensFromCryScreen(u8);
static void LoadPlayArrowPalette(bool8);
static void Task_LoadSizeScreen(u8);
static void Task_HandleSizeScreenInput(u8);
static void Task_SwitchScreensFromSizeScreen(u8);
static void LoadScreenSelectBarMain(u16);
static void LoadScreenSelectBarSubmenu(u16);
static void HighlightScreenSelectBarItem(u8, u16);
static void HighlightSubmenuScreenSelectBarItem(u8, u16);
static void Task_DisplayCaughtMonDexPage(u8);
static void Task_HandleCaughtMonPageInput(u8);
static void Task_ExitCaughtMonPage(u8);
static void SpriteCB_SlideCaughtMonToCenter(struct Sprite *sprite);
static void PrintMonInfo(u32 num, u32, u32 owned, u32 newEntry);
static void PrintMonHeight(u16 height, u8 left, u8 top);
static void PrintMonWeight(u16 weight, u8 left, u8 top);
static void ResetOtherVideoRegisters(u16);
static u8 PrintCryScreenSpeciesName(u8, u16, u8, u8);
static void PrintDecimalNum(u8 windowId, u16 num, u8 left, u8 top);
static void DrawFootprint(u8 windowId, u16 dexNum);
static u16 CreateSizeScreenTrainerPic(u16, s16, s16, s8);
static u16 GetNextPosition(u8, u16, u16, u16);
static u8 LoadSearchMenu(void);
static void Task_LoadSearchMenu(u8);
static void Task_SwitchToSearchMenuTopBar(u8);
static void Task_HandleSearchTopBarInput(u8);
static void Task_SwitchToSearchMenu(u8);
static void Task_HandleSearchMenuInput(u8);
static void Task_StartPokedexSearch(u8);
static void Task_WaitAndCompleteSearch(u8);
static void Task_SearchCompleteWaitForInput(u8);
static void Task_SelectSearchMenuItem(u8);
static void Task_HandleSearchParameterInput(u8);
static void Task_HandleSearchNumberInput(u8 taskId);
static void PrintSelectedSearchExtras(u8 taskId);
static void Task_ExitSearch(u8);
static void Task_ExitSearchWaitForFade(u8);
static void HighlightSelectedSearchTopBarItem(u8);
static void HighlightSelectedSearchMenuItem(u8, u8);
static void PrintSelectedSearchParameters(u8);
static void DrawOrEraseSearchParameterBox(bool8);
static void PrintSearchParameterText(u8);
static u8 GetSearchModeSelection(u8 taskId, u8 option);
static void SetDefaultSearchModeAndOrder(u8);
static void CreateSearchParameterScrollArrows(u8);
static void EraseAndPrintSearchTextBox(const u8 *);
static void EraseSelectorArrow(u32);
static void PrintSelectorArrow(u32);
static void PrintSearchParameterTitle(u32, const u8 *);
static void ClearSearchParameterBoxText(void);
static void SetSpriteInvisibility(u8 spriteArrayId, bool8 invisible);
static void CreateTypeIconSprites(void);
static void SetSearchRectHighlight(u8 flags, u8 x, u8 y, u8 width);
static void PrintInfoSubMenuText(u8 windowId, const u8 *str, u8 left, u8 top);









//Stats screen HGSS_Ui

static void LoadTilesetTilemapHGSS(u8 page);
static void Task_HandleStatsScreenInput(u8 taskId);
static void Task_LoadStatsScreen(u8 taskId);
static void Task_SwitchScreensFromStatsScreen(u8 taskId);
static void Task_ExitStatsScreen(u8 taskId);
static bool8 CalculateMoves(void);
static void PrintStatsScreen_NameGender(u8 taskId, u32 num, u32 value);
static void PrintStatsScreen_DestroyMoveItemIcon(u8 taskId);
static void PrintStatsScreen_Moves_Top(u8 taskId);
static void PrintStatsScreen_Moves_Description(u8 taskId);
static void PrintStatsScreen_Moves_BottomText(u8 taskId);
static void PrintStatsScreen_Moves_Bottom(u8 taskId);
static void PrintStatsScreen_Left(u8 taskId);
static void PrintStatsScreen_Abilities(u8 taskId);

static void PrintInfoScreenTextSmall(const u8* str, u8 left, u8 top);
static void PrintInfoScreenTextSmallWhite(const u8* str, u8 left, u8 top);
static void Task_LoadEvolutionScreen(u8 taskId);
static void Task_HandleEvolutionScreenInput(u8 taskId);
// The EVO page creates a variable number of icons (pre-evolutions, current mon, evolutions) whose ids
// were tracked in task data slots that other pages also use; destroy them by callback instead.
static void DestroyEvoScreenMonIcons(void)
{
    u8 i;

    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse && gSprites[i].callback == SpriteCB_MonIcon)
            FreeAndDestroyMonIconSprite(&gSprites[i]);
    }
}

static void Task_SwitchScreensFromEvolutionScreen(u8 taskId);
static void Task_ExitEvolutionScreen(u8 taskId);
static u8 PrintEvolutionTargetSpeciesAndMethod(u8 taskId, u16 species, u8 depth, u8 depth_i);
static u8 PrintPreEvolutions(u8 taskId, u16 species);
//Stat bars on scrolling screens
static void TryDestroyStatBars(void);
static void TryDestroyStatBarsBg(void);
static void CreateStatBars(struct PokedexListItem *dexMon);
static void CreateStatBarsBg(void);
static void SpriteCB_StatBars(struct Sprite *sprite);
static void SpriteCB_StatBarsBg(struct Sprite *sprite);

//HGSS_UI Forms screen for PokemonExpansion (rhh)
#ifdef POKEMON_EXPANSION
static void Task_LoadFormsScreen(u8 taskId);
static void Task_HandleFormsScreenInput(u8 taskId);
static void PrintForms(u8 taskId, u16 species);
static void Task_SwitchScreensFromFormsScreen(u8 taskId);
static void Task_ExitFormsScreen(u8 taskId);
#endif

//HGSS_UI Physical Special Split icon for BattleEngine (rhh)
#ifdef BATTLE_ENGINE
static u8 ShowSplitIcon(u32 split); //Physical/Special Split from BE
static void DestroySplitIcon(void); //Physical/Special Split from BE

//Physical/Special Split from BE
#define TAG_SPLIT_ICONS 30004

// The INCBINs go through a macro so the Makefile's dependency scanner (scaninc, which ignores #ifdef)
// does not require graphics/interface/split_icons.png, which only exists in battle engine repos.
#define SPLIT_ICONS_INCBIN(type, path) INCBIN_##type(path)
static const u16 sSplitIcons_Pal[] = SPLIT_ICONS_INCBIN(U16, "graphics/interface/split_icons.gbapal");
static const u32 sSplitIcons_Gfx[] = SPLIT_ICONS_INCBIN(U32, "graphics/interface/split_icons.4bpp.lz");

static const struct OamData sOamData_SplitIcons =
{
    .size = SPRITE_SIZE(16x16),
    .shape = SPRITE_SHAPE(16x16),
    .priority = 0,
};
static const struct CompressedSpriteSheet sSpriteSheet_SplitIcons =
{
    .data = sSplitIcons_Gfx,
    .size = 16*16*3/2,
    .tag = TAG_SPLIT_ICONS,
};
static const struct SpritePalette sSpritePal_SplitIcons =
{
    .data = sSplitIcons_Pal,
    .tag = TAG_SPLIT_ICONS
};
static const union AnimCmd sSpriteAnim_SplitIcon0[] =
{
    ANIMCMD_FRAME(0, 0),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_SplitIcon1[] =
{
    ANIMCMD_FRAME(4, 0),
    ANIMCMD_END
};
static const union AnimCmd sSpriteAnim_SplitIcon2[] =
{
    ANIMCMD_FRAME(8, 0),
    ANIMCMD_END
};
static const union AnimCmd *const sSpriteAnimTable_SplitIcons[] =
{
    sSpriteAnim_SplitIcon0,
    sSpriteAnim_SplitIcon1,
    sSpriteAnim_SplitIcon2,
};
static const struct SpriteTemplate sSpriteTemplate_SplitIcons =
{
    .tileTag = TAG_SPLIT_ICONS,
    .paletteTag = TAG_SPLIT_ICONS,
    .oam = &sOamData_SplitIcons,
    .anims = sSpriteAnimTable_SplitIcons,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy
};
#endif

//HGSS_Ui Stat bars by DizzyEgg
#define TAG_STAT_BAR 4097
#define TAG_STAT_BAR_BG 4098
static const struct OamData sOamData_StatBar =
{
    .y = 160,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
};
static const struct OamData sOamData_StatBarBg =
{
    .y = 160,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x64),
    .size = SPRITE_SIZE(64x64),
};
static const struct SpriteTemplate sStatBarSpriteTemplate =
{
    .tileTag = TAG_STAT_BAR,
    .paletteTag = TAG_STAT_BAR,
    .oam = &sOamData_StatBar,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_StatBars,
};
static const struct SpriteTemplate sStatBarBgSpriteTemplate =
{
    .tileTag = TAG_STAT_BAR_BG,
    .paletteTag = TAG_STAT_BAR_BG,
    .oam = &sOamData_StatBarBg,
    .anims = gDummySpriteAnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_StatBarsBg,
};
enum
{
    COLOR_ID_ALPHA,
    COLOR_ID_BAR_WHITE,
    // These are repeated 6 times
    COLOR_ID_FILL,
    COLOR_ID_FILL_SHADOW,
    COLOR_ID_FONT = 14,
    COLOR_ID_FONT_SHADOW = 15,
};
enum
{
    COLOR_BEST, // Light blue
    COLOR_VERY_GOOD, // Green
    COLOR_GOOD, // Light Green
    COLOR_AVERAGE, // Yellow
    COLOR_BAD, // Orange
    COLOR_WORST, // Red
};
static const u8 sStatBarsGfx[] = INCBIN_U8("graphics/pokedex/hgss/stat_bars.4bpp");
static const u16 sStatBarPalette[16] = {
    [COLOR_ID_ALPHA] = RGB(0, 0, 10),
    [COLOR_ID_BAR_WHITE] = RGB_WHITE,

    [COLOR_ID_FILL + COLOR_BEST * 2] = RGB(2, 25, 25),
    [COLOR_ID_FILL_SHADOW + COLOR_BEST * 2] = RGB(13, 27, 27),

    [COLOR_ID_FILL + COLOR_VERY_GOOD * 2] = RGB(11, 25, 2),
    [COLOR_ID_FILL_SHADOW + COLOR_VERY_GOOD * 2] = RGB(19, 27, 13),

    [COLOR_ID_FILL + COLOR_GOOD * 2] = RGB(22, 25, 2),
    [COLOR_ID_FILL_SHADOW + COLOR_GOOD * 2] = RGB(26, 27, 13),

    [COLOR_ID_FILL + COLOR_AVERAGE * 2] = RGB(25, 22, 2),
    [COLOR_ID_FILL_SHADOW + COLOR_AVERAGE * 2] = RGB(27, 26, 13),

    [COLOR_ID_FILL + COLOR_BAD * 2] = RGB(25, 17, 2),
    [COLOR_ID_FILL_SHADOW + COLOR_BAD * 2] = RGB(27, 22, 13),

    [COLOR_ID_FILL + COLOR_WORST * 2] = RGB(25, 4, 2),
    [COLOR_ID_FILL_SHADOW + COLOR_WORST * 2] = RGB(27, 15, 13),

    [COLOR_ID_FONT] = RGB_BLACK,
    [COLOR_ID_FONT_SHADOW] = RGB(22, 22, 22),
};
static const struct SpritePalette sStatBarSpritePal[] = //{sStatBarPalette, TAG_STAT_BAR};
{
    {sStatBarPalette, TAG_STAT_BAR},
    {sStatBarPalette, TAG_STAT_BAR_BG},
    {0}
};

















static const struct OamData sOamData_ScrollBar =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x8),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_ScrollArrow =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(16x8),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(16x8),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_InterfaceText =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x16),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_RotatingPokeBall =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_WINDOW,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(32x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(32x32),
    .tileNum = 0,
    .priority = 1,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_SeenOwnText =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(64x32),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x32),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const struct OamData sOamData_Dex8x16 =
{
    .y = DISPLAY_HEIGHT,
    .affineMode = ST_OAM_AFFINE_OFF,
    .objMode = ST_OAM_OBJ_NORMAL,
    .mosaic = FALSE,
    .bpp = ST_OAM_4BPP,
    .shape = SPRITE_SHAPE(8x16),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(8x16),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0
};

static const union AnimCmd sSpriteAnim_ScrollBar[] =
{
    ANIMCMD_FRAME(3, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_ScrollArrow[] =
{
    ANIMCMD_FRAME(1, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_RotatingPokeBall[] =
{
    ANIMCMD_FRAME(16, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_StartButton[] =
{
    ANIMCMD_FRAME(48, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_SearchText[] =
{
    ANIMCMD_FRAME(40, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_SelectButton[] =
{
    ANIMCMD_FRAME(32, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_MenuText[] =
{
    ANIMCMD_FRAME(56, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_SeenText[] =
{
    ANIMCMD_FRAME(64, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_OwnText[] =
{
    ANIMCMD_FRAME(96, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_HoennText[] =
{
    ANIMCMD_FRAME(160, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_NationalText[] =
{
    ANIMCMD_FRAME(168, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_HoennSeenOwnDigit0[] =
{
    ANIMCMD_FRAME(128, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_HoennSeenOwnDigit1[] =
{
    ANIMCMD_FRAME(130, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_HoennSeenOwnDigit2[] =
{
    ANIMCMD_FRAME(132, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_HoennSeenOwnDigit3[] =
{
    ANIMCMD_FRAME(134, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_HoennSeenOwnDigit4[] =
{
    ANIMCMD_FRAME(136, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_HoennSeenOwnDigit5[] =
{
    ANIMCMD_FRAME(138, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_HoennSeenOwnDigit6[] =
{
    ANIMCMD_FRAME(140, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_HoennSeenOwnDigit7[] =
{
    ANIMCMD_FRAME(142, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_HoennSeenOwnDigit8[] =
{
    ANIMCMD_FRAME(144, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_HoennSeenOwnDigit9[] =
{
    ANIMCMD_FRAME(146, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_NationalSeenOwnDigit0[] =
{
    ANIMCMD_FRAME(176, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_NationalSeenOwnDigit1[] =
{
    ANIMCMD_FRAME(178, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_NationalSeenOwnDigit2[] =
{
    ANIMCMD_FRAME(180, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_NationalSeenOwnDigit3[] =
{
    ANIMCMD_FRAME(182, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_NationalSeenOwnDigit4[] =
{
    ANIMCMD_FRAME(184, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_NationalSeenOwnDigit5[] =
{
    ANIMCMD_FRAME(186, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_NationalSeenOwnDigit6[] =
{
    ANIMCMD_FRAME(188, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_NationalSeenOwnDigit7[] =
{
    ANIMCMD_FRAME(190, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_NationalSeenOwnDigit8[] =
{
    ANIMCMD_FRAME(192, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_NationalSeenOwnDigit9[] =
{
    ANIMCMD_FRAME(194, 30),
    ANIMCMD_END
};

static const union AnimCmd sSpriteAnim_DexListStartMenuCursor[] =
{
    ANIMCMD_FRAME(4, 30),
    ANIMCMD_END
};

static const union AnimCmd *const sSpriteAnimTable_ScrollBar[] =
{
    sSpriteAnim_ScrollBar
};

static const union AnimCmd *const sSpriteAnimTable_ScrollArrow[] =
{
    sSpriteAnim_ScrollArrow
};

static const union AnimCmd *const sSpriteAnimTable_RotatingPokeBall[] =
{
    sSpriteAnim_RotatingPokeBall
};

static const union AnimCmd *const sSpriteAnimTable_InterfaceText[] =
{
    sSpriteAnim_StartButton,
    sSpriteAnim_SearchText,
    sSpriteAnim_SelectButton,
    sSpriteAnim_MenuText
};

static const union AnimCmd *const sSpriteAnimTable_SeenOwnText[] =
{
    sSpriteAnim_SeenText,
    sSpriteAnim_OwnText
};

static const union AnimCmd *const sSpriteAnimTable_HoennNationalText[] =
{
    sSpriteAnim_HoennText,
    sSpriteAnim_NationalText
};

static const union AnimCmd *const sSpriteAnimTable_HoennSeenOwnNumber[] =
{
    sSpriteAnim_HoennSeenOwnDigit0,
    sSpriteAnim_HoennSeenOwnDigit1,
    sSpriteAnim_HoennSeenOwnDigit2,
    sSpriteAnim_HoennSeenOwnDigit3,
    sSpriteAnim_HoennSeenOwnDigit4,
    sSpriteAnim_HoennSeenOwnDigit5,
    sSpriteAnim_HoennSeenOwnDigit6,
    sSpriteAnim_HoennSeenOwnDigit7,
    sSpriteAnim_HoennSeenOwnDigit8,
    sSpriteAnim_HoennSeenOwnDigit9
};

static const union AnimCmd *const sSpriteAnimTable_NationalSeenOwnNumber[] =
{
    sSpriteAnim_NationalSeenOwnDigit0,
    sSpriteAnim_NationalSeenOwnDigit1,
    sSpriteAnim_NationalSeenOwnDigit2,
    sSpriteAnim_NationalSeenOwnDigit3,
    sSpriteAnim_NationalSeenOwnDigit4,
    sSpriteAnim_NationalSeenOwnDigit5,
    sSpriteAnim_NationalSeenOwnDigit6,
    sSpriteAnim_NationalSeenOwnDigit7,
    sSpriteAnim_NationalSeenOwnDigit8,
    sSpriteAnim_NationalSeenOwnDigit9
};

static const union AnimCmd *const sSpriteAnimTable_DexListStartMenuCursor[] =
{
    sSpriteAnim_DexListStartMenuCursor
};

#define TAG_DEX_INTERFACE 4096 // Tile and pal tag used for all interface sprites.

static const struct SpriteTemplate sScrollBarSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_ScrollBar,
    .anims = sSpriteAnimTable_ScrollBar,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_Scrollbar,
};

static const struct SpriteTemplate sScrollArrowSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_ScrollArrow,
    .anims = sSpriteAnimTable_ScrollArrow,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_ScrollArrow,
};

static const struct SpriteTemplate sInterfaceTextSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_InterfaceText,
    .anims = sSpriteAnimTable_InterfaceText,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_DexListInterfaceText,
};

static const struct SpriteTemplate sRotatingPokeBallSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_RotatingPokeBall,
    .anims = sSpriteAnimTable_RotatingPokeBall,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_RotatingPokeBall,
};

static const struct SpriteTemplate sSeenOwnTextSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_SeenOwnText,
    .anims = sSpriteAnimTable_SeenOwnText,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_SeenOwnInfo,
};

static const struct SpriteTemplate sHoennNationalTextSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_InterfaceText,
    .anims = sSpriteAnimTable_HoennNationalText,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_SeenOwnInfo,
};

static const struct SpriteTemplate sHoennDexSeenOwnNumberSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_Dex8x16,
    .anims = sSpriteAnimTable_HoennSeenOwnNumber,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_SeenOwnInfo,
};

static const struct SpriteTemplate sNationalDexSeenOwnNumberSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_Dex8x16,
    .anims = sSpriteAnimTable_NationalSeenOwnNumber,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_SeenOwnInfo,
};

static const struct SpriteTemplate sDexListStartMenuCursorSpriteTemplate =
{
    .tileTag = TAG_DEX_INTERFACE,
    .paletteTag = TAG_DEX_INTERFACE,
    .oam = &sOamData_Dex8x16,
    .anims = sSpriteAnimTable_DexListStartMenuCursor,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCB_DexListStartMenuCursor,
};

static const struct CompressedSpriteSheet sInterfaceSpriteSheet[] =
{
    {gPokedexPlusHGSS_Interface_Gfx, 0x2000, TAG_DEX_INTERFACE},
    {gPokedexPlusHGSS_Interface_DECA_Gfx, 0x2000, TAG_DEX_INTERFACE},
    {0}
};

static const struct SpritePalette sInterfaceSpritePalette[] =
{
    {gPokedexPlusHGSS_Default_Pal, TAG_DEX_INTERFACE},
    {0}
};

// By scroll speed. Last element of each unused
static const u8 sScrollMonIncrements[] = {4, 8, 16, 32, 32};
static const u8 sScrollTimers[] = {8, 4, 2, 1, 1};



















static const struct BgTemplate sPokedex_BgTemplate[] =
{
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 12,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 13,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 14,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    }
};

static const struct WindowTemplate sPokemonList_WindowTemplate[] =
{
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 32,
        .height = 32,
        .paletteNum = 0,
        .baseBlock = 1,
    },
    DUMMY_WIN_TEMPLATE
};















static const struct BgTemplate sInfoScreen_BgTemplate[] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 12,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 13,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 14,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    }
};

#define WIN_INFO 0
#define WIN_FOOTPRINT 1
#define WIN_CRY_WAVE 2
#define WIN_VU_METER 3
#define WIN_NAVIGATION_BUTTONS 4

static const struct WindowTemplate sInfoScreen_WindowTemplates[] =
{
    [WIN_INFO] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 32,
        .height = 20,
        .paletteNum = 0,
        .baseBlock = 1,
    },
    [WIN_FOOTPRINT] =
    {
        .bg = 2,
        .tilemapLeft = 25,
        .tilemapTop = 3,
        .width = 2,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 641,
    },
    [WIN_CRY_WAVE] =
    {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 12,
        .width = 32,
        .height = 7,
        .paletteNum = 8,
        .baseBlock = 645,
    },
    [WIN_VU_METER] =
    {
        .bg = 2,
        .tilemapLeft = 18,
        .tilemapTop = 3,
        .width = 10,
        .height = 8,
        .paletteNum = 9,
        .baseBlock = 869,
    },
    [WIN_NAVIGATION_BUTTONS] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 18,
        .width = 20,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 641,
    },

    DUMMY_WIN_TEMPLATE
};

#define WIN_STATS_TOPBAR 0
#define WIN_STATS_SIDEBAR 1
#define WIN_STATS_NAME_GENDER 2
#define WIN_STATS_LEFT 3
#define WIN_STATS_NAVIGATION_BUTTONS 4
#define WIN_STATS_MOVES_TOP 5
#define WIN_STATS_MOVES_DESCRIPTION 7
#define WIN_STATS_MOVES_BOTTOM 8
#define WIN_STATS_ABILITIES 9
#define WIN_STATS_LEFT_UNUSED 10
#define WIN_STATS_END WIN_STATS_LEFT_UNUSED
static const struct WindowTemplate sStatsScreen_WindowTemplates[] =
{
    [WIN_STATS_TOPBAR] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 1,
    },
    [WIN_STATS_SIDEBAR] =
    {
        .bg = 2,
        .tilemapLeft = 30,
        .tilemapTop = 0,
        .width = 2,
        .height = 20,
        .paletteNum = 0,
        .baseBlock = 1 + 60,
    },
    [WIN_STATS_NAME_GENDER] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 2,
        .width = 12,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40,
    },
    [WIN_STATS_LEFT] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 6,
        .width = 12,
        .height = 8,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48,
    },
    [WIN_STATS_NAVIGATION_BUTTONS] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 18,
        .width = 12,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 1 + 60 + 40 + 48 + 96,
    },
    [WIN_STATS_MOVES_TOP] =
    {
        .bg = 2,
        .tilemapLeft = 12,
        .tilemapTop = 2,
        .width = 18,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48 + 96 + 24,
    },
    [WIN_STATS_MOVES_DESCRIPTION] =
    {
        .bg = 2,
        .tilemapLeft = 12,
        .tilemapTop = 6,
        .width = 18,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48 + 96 + 24 + 72,
    },
    [WIN_STATS_MOVES_BOTTOM] =
    {
        .bg = 2,
        .tilemapLeft = 12,
        .tilemapTop = 10,
        .width = 18,
        .height = 2,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48 + 96 + 24 + 72 + 72,
    },
    [WIN_STATS_ABILITIES] =
    {
        .bg = 2,
        .tilemapLeft = 12,
        .tilemapTop = 12,
        .width = 18,
        .height = 8,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48 + 96 + 24 + 72 + 72 + 36,
    },
    [WIN_STATS_LEFT_UNUSED] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 14,
        .width = 12,
        .height = 4,
        .paletteNum = 0,
        .baseBlock = 1 + 60 + 40 + 48 + 96 + 24 + 72 + 72 + 36 + 144,
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate sNewEntryInfoScreen_BgTemplate[] =
{
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 14,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 1,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    },
};

static const struct WindowTemplate sNewEntryInfoScreen_WindowTemplates[] =
{
    [WIN_INFO] =
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 32,
        .height = 20,
        .paletteNum = 0,
        .baseBlock = 1,
    },
    [WIN_FOOTPRINT] =
    {
        .bg = 2,
        .tilemapLeft = 25,
        .tilemapTop = 3,
        .width = 2,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 641,
    },
    DUMMY_WIN_TEMPLATE
};









// First character in range followed by number of characters in range for upper and lowercase
static const u8 sLetterSearchRanges[][4] =
{
    {}, // Name not specified, shouldn't be reached
    [NAME_ABC] = {CHAR_A, 3, CHAR_a, 3},
    [NAME_DEF] = {CHAR_D, 3, CHAR_d, 3},
    [NAME_GHI] = {CHAR_G, 3, CHAR_g, 3},
    [NAME_JKL] = {CHAR_J, 3, CHAR_j, 3},
    [NAME_MNO] = {CHAR_M, 3, CHAR_m, 3},
    [NAME_PQR] = {CHAR_P, 3, CHAR_p, 3},
    [NAME_STU] = {CHAR_S, 3, CHAR_s, 3},
    [NAME_VWX] = {CHAR_V, 3, CHAR_v, 3},
    [NAME_YZ]  = {CHAR_Y, 2, CHAR_y, 2},
};

#define LETTER_IN_RANGE_UPPER(letter, range) \
    ((letter) >= sLetterSearchRanges[range][0]                                  \
  && (letter) < sLetterSearchRanges[range][0] + sLetterSearchRanges[range][1])  \

#define LETTER_IN_RANGE_LOWER(letter, range) \
    ((letter) >= sLetterSearchRanges[range][2]                                  \
  && (letter) < sLetterSearchRanges[range][2] + sLetterSearchRanges[range][3])  \

// Right-hand search fields
static const u8 sText_SearchBstDesc[] = _("Search by a base stat: pick the stat,\nhow it compares, and the number.");
static const u8 sText_SearchBstValueDesc[] = _("{DPAD_UPDOWN} change by 1   {DPAD_LEFTRIGHT} change by 10\n1-255, or up to 780 for TOT.");
static const u8 sText_SearchYieldDesc[] = _("List POKéMON giving this EV yield,\nhighest first, mixed yields last.");
static const u8 sText_SearchMoveDesc[] = _("List POKéMON that can learn all\nthe chosen moves. L/R: next letter.");
static const u8 sText_SearchAllDesc[] = _("ON: search every POKéMON as if\nall of them were caught.");
static const u8 sText_SearchNone[] = _("---");
static const u8 sText_SearchMoveNone[] = _("------");
static const u8 sText_SearchOn[] = _("ON");
static const u8 sText_SearchOff[] = _("OFF");

static const u8 sText_SearchStatHp[] = _("HP");
static const u8 sText_SearchStatAtk[] = _("ATK");
static const u8 sText_SearchStatDef[] = _("DEF");
static const u8 sText_SearchStatSpAtk[] = _("SPA");
static const u8 sText_SearchStatSpDef[] = _("SPD");
static const u8 sText_SearchStatSpeed[] = _("SPE");
static const u8 sText_SearchStatTotal[] = _("TOT");
static const u8 sText_SearchStatHpDesc[] = _("Base HP.");
static const u8 sText_SearchStatAtkDesc[] = _("Base ATTACK.");
static const u8 sText_SearchStatDefDesc[] = _("Base DEFENSE.");
static const u8 sText_SearchStatSpAtkDesc[] = _("Base SP. ATK.");
static const u8 sText_SearchStatSpDefDesc[] = _("Base SP. DEF.");
static const u8 sText_SearchStatSpeedDesc[] = _("Base SPEED.");
static const u8 sText_SearchStatTotalDesc[] = _("Base stat total: all six base\nstats added up.");

static const struct SearchOptionText sSearchBstStatOptions[SEARCH_STAT_COUNT] =
{
    [SEARCH_STAT_NONE]  = {sText_SearchBstDesc,       sText_SearchNone},
    [SEARCH_STAT_HP]    = {sText_SearchStatHpDesc,    sText_SearchStatHp},
    [SEARCH_STAT_ATK]   = {sText_SearchStatAtkDesc,   sText_SearchStatAtk},
    [SEARCH_STAT_DEF]   = {sText_SearchStatDefDesc,   sText_SearchStatDef},
    [SEARCH_STAT_SPATK] = {sText_SearchStatSpAtkDesc, sText_SearchStatSpAtk},
    [SEARCH_STAT_SPDEF] = {sText_SearchStatSpDefDesc, sText_SearchStatSpDef},
    [SEARCH_STAT_SPEED] = {sText_SearchStatSpeedDesc, sText_SearchStatSpeed},
    [SEARCH_STAT_TOTAL] = {sText_SearchStatTotalDesc, sText_SearchStatTotal},
};

// The yield list has no TOTAL
static const struct SearchOptionText sSearchYieldOptions[SEARCH_STAT_COUNT - 1] =
{
    [SEARCH_STAT_NONE]  = {sText_SearchYieldDesc,     sText_SearchNone},
    [SEARCH_STAT_HP]    = {sText_SearchYieldDesc,     gText_HP3},
    [SEARCH_STAT_ATK]   = {sText_SearchYieldDesc,     gText_Attack3},
    [SEARCH_STAT_DEF]   = {sText_SearchYieldDesc,     gText_Defense3},
    [SEARCH_STAT_SPATK] = {sText_SearchYieldDesc,     gText_SpAtk3},
    [SEARCH_STAT_SPDEF] = {sText_SearchYieldDesc,     gText_SpDef3},
    [SEARCH_STAT_SPEED] = {sText_SearchYieldDesc,     gText_Speed2},
};

static const u8 sText_SearchOpEqual[] = _("=");
static const u8 sText_SearchOpAtLeast[] = _(">=");
static const u8 sText_SearchOpAtMost[] = _("<=");
static const u8 sText_SearchOpLess[] = _("<");
static const u8 sText_SearchOpMore[] = _(">");
static const u8 sText_SearchOpEqualDesc[] = _("Exactly the number.");
static const u8 sText_SearchOpAtLeastDesc[] = _("The number or more.");
static const u8 sText_SearchOpAtMostDesc[] = _("The number or less.");
static const u8 sText_SearchOpLessDesc[] = _("Less than the number.");
static const u8 sText_SearchOpMoreDesc[] = _("More than the number.");

static const struct SearchOptionText sSearchBstOpOptions[SEARCH_OP_COUNT] =
{
    [SEARCH_OP_EQUAL]    = {sText_SearchOpEqualDesc,   sText_SearchOpEqual},
    [SEARCH_OP_AT_LEAST] = {sText_SearchOpAtLeastDesc, sText_SearchOpAtLeast},
    [SEARCH_OP_AT_MOST]  = {sText_SearchOpAtMostDesc,  sText_SearchOpAtMost},
    [SEARCH_OP_LESS]     = {sText_SearchOpLessDesc,    sText_SearchOpLess},
    [SEARCH_OP_MORE]     = {sText_SearchOpMoreDesc,    sText_SearchOpMore},
};

static const struct SearchMenuTopBarItem sSearchMenuTopBarItems[SEARCH_TOPBAR_COUNT] =
{
    [SEARCH_TOPBAR_SEARCH] =
    {
        .description = gText_SearchForPkmnBasedOnParameters,
        .highlightX = 0,
        .highlightY = 0,
        .highlightWidth = 5,
    },
    [SEARCH_TOPBAR_SHIFT] =
    {
        .description = gText_SwitchPokedexListings,
        .highlightX = 6,
        .highlightY = 0,
        .highlightWidth = 5,
    },
    [SEARCH_TOPBAR_CANCEL] =
    {
        .description = gText_ReturnToPokedex,
        .highlightX = 12,
        .highlightY = 0,
        .highlightWidth = 5,
    },
};

static const struct SearchMenuItem sSearchMenuItems[SEARCH_COUNT] =
{
    [SEARCH_NAME] =
    {
        .description = gText_ListByFirstLetter,
        .titleBgX = 0,
        .titleBgY = 2,
        .titleBgWidth = 5,
        .selectionBgX = 5,
        .selectionBgY = 2,
        .selectionBgWidth = 12,
    },
    [SEARCH_COLOR] =
    {
        .description = gText_ListByBodyColor,
        .titleBgX = 0,
        .titleBgY = 4,
        .titleBgWidth = 5,
        .selectionBgX = 5,
        .selectionBgY = 4,
        .selectionBgWidth = 12,
    },
    [SEARCH_TYPE_LEFT] =
    {
        .description = gText_ListByType,
        .titleBgX = 0,
        .titleBgY = 6,
        .titleBgWidth = 5,
        .selectionBgX = 5,
        .selectionBgY = 6,
        .selectionBgWidth = 6,
    },
    [SEARCH_TYPE_RIGHT] =
    {
        .description = gText_ListByType,
        .titleBgX = 0,
        .titleBgY = 6,
        .titleBgWidth = 5,
        .selectionBgX = 11,
        .selectionBgY = 6,
        .selectionBgWidth = 6,
    },
    [SEARCH_ORDER] =
    {
        .description = gText_SelectPokedexListingMode,
        .titleBgX = 0,
        .titleBgY = 8,
        .titleBgWidth = 5,
        .selectionBgX = 5,
        .selectionBgY = 8,
        .selectionBgWidth = 12,
    },
    [SEARCH_MODE] =
    {
        .description = gText_SelectPokedexMode,
        .titleBgX = 0,
        .titleBgY = 10,
        .titleBgWidth = 5,
        .selectionBgX = 5,
        .selectionBgY = 10,
        .selectionBgWidth = 12,
    },
    [SEARCH_OK] =
    {
        .description = gText_ExecuteSearchSwitch,
        .titleBgX = 0,
        .titleBgY = 12,
        .titleBgWidth = 5,
        .selectionBgX = 0,
        .selectionBgY = 0,
        .selectionBgWidth = 0,
    },
    // Right-hand column, drawn by tech/pokedex_hgss_reskin.py (SEARCH_RIGHT_ROWS)
    [SEARCH_BST_STAT] =
    {
        .description = sText_SearchBstDesc,
        .titleBgX = 17, .titleBgY = 2, .titleBgWidth = 4,
        .selectionBgX = 21, .selectionBgY = 2, .selectionBgWidth = 3,
    },
    [SEARCH_BST_OP] =
    {
        .description = sText_SearchBstDesc,
        .titleBgX = 17, .titleBgY = 2, .titleBgWidth = 4,
        .selectionBgX = 24, .selectionBgY = 2, .selectionBgWidth = 3,
    },
    [SEARCH_BST_VALUE] =
    {
        .description = sText_SearchBstValueDesc,
        .titleBgX = 17, .titleBgY = 2, .titleBgWidth = 4,
        .selectionBgX = 27, .selectionBgY = 2, .selectionBgWidth = 3,
    },
    [SEARCH_YIELD] =
    {
        .description = sText_SearchYieldDesc,
        .titleBgX = 17, .titleBgY = 4, .titleBgWidth = 4,
        .selectionBgX = 21, .selectionBgY = 4, .selectionBgWidth = 9,
    },
    [SEARCH_MOVE_1] =
    {
        .description = sText_SearchMoveDesc,
        .titleBgX = 17, .titleBgY = 6, .titleBgWidth = 4,
        .selectionBgX = 21, .selectionBgY = 6, .selectionBgWidth = 9,
    },
    [SEARCH_MOVE_2] =
    {
        .description = sText_SearchMoveDesc,
        .titleBgX = 17, .titleBgY = 8, .titleBgWidth = 4,
        .selectionBgX = 21, .selectionBgY = 8, .selectionBgWidth = 9,
    },
    [SEARCH_MOVE_3] =
    {
        .description = sText_SearchMoveDesc,
        .titleBgX = 17, .titleBgY = 10, .titleBgWidth = 4,
        .selectionBgX = 21, .selectionBgY = 10, .selectionBgWidth = 9,
    },
    [SEARCH_MOVE_4] =
    {
        .description = sText_SearchMoveDesc,
        .titleBgX = 17, .titleBgY = 12, .titleBgWidth = 4,
        .selectionBgX = 21, .selectionBgY = 12, .selectionBgWidth = 9,
    },
    // Next to OK; moves up 2 rows with OK in the Hoenn Pokédex (no MODE row)
    [SEARCH_ALL] =
    {
        .description = sText_SearchAllDesc,
        .titleBgX = 6, .titleBgY = 12, .titleBgWidth = 3,
        .selectionBgX = 9, .selectionBgY = 12, .selectionBgWidth = 4,
    },
};

// Left, Right, Up, Down
static const u8 sSearchMovementMap_SearchNatDex[SEARCH_COUNT][4] =
{
    [SEARCH_NAME] = {0xFF, SEARCH_BST_STAT, 0xFF, SEARCH_COLOR},
    [SEARCH_COLOR] = {0xFF, SEARCH_YIELD, SEARCH_NAME, SEARCH_TYPE_LEFT},
    [SEARCH_TYPE_LEFT] = {0xFF, SEARCH_TYPE_RIGHT, SEARCH_COLOR, SEARCH_ORDER},
    [SEARCH_TYPE_RIGHT] = {SEARCH_TYPE_LEFT, SEARCH_MOVE_1, SEARCH_COLOR, SEARCH_ORDER},
    [SEARCH_ORDER] = {0xFF, SEARCH_MOVE_2, SEARCH_TYPE_LEFT, SEARCH_MODE},
    [SEARCH_MODE] = {0xFF, SEARCH_MOVE_3, SEARCH_ORDER, SEARCH_OK},
    [SEARCH_OK] = {0xFF, SEARCH_ALL, SEARCH_MODE, 0xFF},
    [SEARCH_BST_STAT] = {SEARCH_NAME, SEARCH_BST_OP, 0xFF, SEARCH_YIELD},
    [SEARCH_BST_OP] = {SEARCH_BST_STAT, SEARCH_BST_VALUE, 0xFF, SEARCH_YIELD},
    [SEARCH_BST_VALUE] = {SEARCH_BST_OP, 0xFF, 0xFF, SEARCH_YIELD},
    [SEARCH_YIELD] = {SEARCH_COLOR, 0xFF, SEARCH_BST_STAT, SEARCH_MOVE_1},
    [SEARCH_MOVE_1] = {SEARCH_TYPE_RIGHT, 0xFF, SEARCH_YIELD, SEARCH_MOVE_2},
    [SEARCH_MOVE_2] = {SEARCH_ORDER, 0xFF, SEARCH_MOVE_1, SEARCH_MOVE_3},
    [SEARCH_MOVE_3] = {SEARCH_MODE, 0xFF, SEARCH_MOVE_2, SEARCH_MOVE_4},
    [SEARCH_MOVE_4] = {SEARCH_ALL, 0xFF, SEARCH_MOVE_3, 0xFF},
    [SEARCH_ALL] = {SEARCH_OK, SEARCH_MOVE_4, SEARCH_MODE, 0xFF},
};

// Left, Right, Up, Down
static const u8 sSearchMovementMap_ShiftNatDex[SEARCH_COUNT][4] =
{
    [SEARCH_NAME] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_COLOR] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_TYPE_LEFT] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_TYPE_RIGHT] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_ORDER] = {0xFF, 0xFF, 0xFF, SEARCH_MODE},
    [SEARCH_MODE] = {0xFF, 0xFF, SEARCH_ORDER, SEARCH_OK},
    [SEARCH_OK] = {0xFF, 0xFF, SEARCH_MODE, 0xFF},
    [SEARCH_BST_STAT] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_BST_OP] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_BST_VALUE] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_YIELD] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_MOVE_1] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_MOVE_2] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_MOVE_3] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_MOVE_4] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_ALL] = {0xFF, 0xFF, 0xFF, 0xFF},
};

// Left, Right, Up, Down
static const u8 sSearchMovementMap_SearchHoennDex[SEARCH_COUNT][4] =
{
    [SEARCH_NAME] = {0xFF, SEARCH_BST_STAT, 0xFF, SEARCH_COLOR},
    [SEARCH_COLOR] = {0xFF, SEARCH_YIELD, SEARCH_NAME, SEARCH_TYPE_LEFT},
    [SEARCH_TYPE_LEFT] = {0xFF, SEARCH_TYPE_RIGHT, SEARCH_COLOR, SEARCH_ORDER},
    [SEARCH_TYPE_RIGHT] = {SEARCH_TYPE_LEFT, SEARCH_MOVE_1, SEARCH_COLOR, SEARCH_ORDER},
    [SEARCH_ORDER] = {0xFF, SEARCH_MOVE_2, SEARCH_TYPE_LEFT, SEARCH_OK},
    [SEARCH_MODE] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_OK] = {0xFF, SEARCH_ALL, SEARCH_ORDER, 0xFF},
    [SEARCH_BST_STAT] = {SEARCH_NAME, SEARCH_BST_OP, 0xFF, SEARCH_YIELD},
    [SEARCH_BST_OP] = {SEARCH_BST_STAT, SEARCH_BST_VALUE, 0xFF, SEARCH_YIELD},
    [SEARCH_BST_VALUE] = {SEARCH_BST_OP, 0xFF, 0xFF, SEARCH_YIELD},
    [SEARCH_YIELD] = {SEARCH_COLOR, 0xFF, SEARCH_BST_STAT, SEARCH_MOVE_1},
    [SEARCH_MOVE_1] = {SEARCH_TYPE_RIGHT, 0xFF, SEARCH_YIELD, SEARCH_MOVE_2},
    [SEARCH_MOVE_2] = {SEARCH_ORDER, 0xFF, SEARCH_MOVE_1, SEARCH_MOVE_3},
    [SEARCH_MOVE_3] = {SEARCH_ALL, 0xFF, SEARCH_MOVE_2, SEARCH_MOVE_4},
    [SEARCH_MOVE_4] = {SEARCH_ALL, 0xFF, SEARCH_MOVE_3, 0xFF},
    [SEARCH_ALL] = {SEARCH_OK, SEARCH_MOVE_3, SEARCH_ORDER, 0xFF},
};

// Left, Right, Up, Down
static const u8 sSearchMovementMap_ShiftHoennDex[SEARCH_COUNT][4] =
{
    [SEARCH_NAME] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_COLOR] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_TYPE_LEFT] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_TYPE_RIGHT] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_ORDER] = {0xFF, 0xFF, 0xFF, SEARCH_OK},
    [SEARCH_MODE] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_OK] = {0xFF, 0xFF, SEARCH_ORDER, 0xFF},
    [SEARCH_BST_STAT] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_BST_OP] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_BST_VALUE] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_YIELD] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_MOVE_1] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_MOVE_2] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_MOVE_3] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_MOVE_4] = {0xFF, 0xFF, 0xFF, 0xFF},
    [SEARCH_ALL] = {0xFF, 0xFF, 0xFF, 0xFF},
};

static const struct SearchOptionText sDexModeOptions[] =
{
    [DEX_MODE_HOENN]    = {gText_DexHoennDescription, gText_DexHoennTitle},
    [DEX_MODE_NATIONAL] = {gText_DexNatDescription,   gText_DexNatTitle},
    {},
};

static const struct SearchOptionText sDexOrderOptions[] =
{
    [ORDER_NUMERICAL]    = {gText_DexSortNumericalDescription, gText_DexSortNumericalTitle},
    [ORDER_ALPHABETICAL] = {gText_DexSortAtoZDescription,      gText_DexSortAtoZTitle},
    [ORDER_HEAVIEST]     = {gText_DexSortHeaviestDescription,  gText_DexSortHeaviestTitle},
    [ORDER_LIGHTEST]     = {gText_DexSortLightestDescription,  gText_DexSortLightestTitle},
    [ORDER_TALLEST]      = {gText_DexSortTallestDescription,   gText_DexSortTallestTitle},
    [ORDER_SMALLEST]     = {gText_DexSortSmallestDescription,  gText_DexSortSmallestTitle},
    {},
};

static const struct SearchOptionText sDexSearchNameOptions[] =
{
    {gText_DexEmptyString, gText_DexSearchDontSpecify},
    [NAME_ABC] = {gText_DexEmptyString, gText_DexSearchAlphaABC},
    [NAME_DEF] = {gText_DexEmptyString, gText_DexSearchAlphaDEF},
    [NAME_GHI] = {gText_DexEmptyString, gText_DexSearchAlphaGHI},
    [NAME_JKL] = {gText_DexEmptyString, gText_DexSearchAlphaJKL},
    [NAME_MNO] = {gText_DexEmptyString, gText_DexSearchAlphaMNO},
    [NAME_PQR] = {gText_DexEmptyString, gText_DexSearchAlphaPQR},
    [NAME_STU] = {gText_DexEmptyString, gText_DexSearchAlphaSTU},
    [NAME_VWX] = {gText_DexEmptyString, gText_DexSearchAlphaVWX},
    [NAME_YZ]  = {gText_DexEmptyString, gText_DexSearchAlphaYZ},
    {},
};

static const struct SearchOptionText sDexSearchColorOptions[] =
{
    {gText_DexEmptyString, gText_DexSearchDontSpecify},
    [BODY_COLOR_RED + 1]    = {gText_DexEmptyString, gText_DexSearchColorRed},
    [BODY_COLOR_BLUE + 1]   = {gText_DexEmptyString, gText_DexSearchColorBlue},
    [BODY_COLOR_YELLOW + 1] = {gText_DexEmptyString, gText_DexSearchColorYellow},
    [BODY_COLOR_GREEN + 1]  = {gText_DexEmptyString, gText_DexSearchColorGreen},
    [BODY_COLOR_BLACK + 1]  = {gText_DexEmptyString, gText_DexSearchColorBlack},
    [BODY_COLOR_BROWN + 1]  = {gText_DexEmptyString, gText_DexSearchColorBrown},
    [BODY_COLOR_PURPLE + 1] = {gText_DexEmptyString, gText_DexSearchColorPurple},
    [BODY_COLOR_GRAY + 1]   = {gText_DexEmptyString, gText_DexSearchColorGray},
    [BODY_COLOR_WHITE + 1]  = {gText_DexEmptyString, gText_DexSearchColorWhite},
    [BODY_COLOR_PINK + 1]   = {gText_DexEmptyString, gText_DexSearchColorPink},
    {},
};

static const struct SearchOptionText sDexSearchTypeOptions[NUMBER_OF_MON_TYPES + 1] = // + 2 for "None" and terminator, - 1 for Mystery
{
    {gText_DexEmptyString, gText_DexSearchTypeNone},
    {gText_DexEmptyString, gTypeNames[TYPE_NORMAL]},
    {gText_DexEmptyString, gTypeNames[TYPE_FIGHTING]},
    {gText_DexEmptyString, gTypeNames[TYPE_FLYING]},
    {gText_DexEmptyString, gTypeNames[TYPE_POISON]},
    {gText_DexEmptyString, gTypeNames[TYPE_GROUND]},
    {gText_DexEmptyString, gTypeNames[TYPE_ROCK]},
    {gText_DexEmptyString, gTypeNames[TYPE_BUG]},
    {gText_DexEmptyString, gTypeNames[TYPE_GHOST]},
    {gText_DexEmptyString, gTypeNames[TYPE_STEEL]},
    {gText_DexEmptyString, gTypeNames[TYPE_FIRE]},
    {gText_DexEmptyString, gTypeNames[TYPE_WATER]},
    {gText_DexEmptyString, gTypeNames[TYPE_GRASS]},
    {gText_DexEmptyString, gTypeNames[TYPE_ELECTRIC]},
    {gText_DexEmptyString, gTypeNames[TYPE_PSYCHIC]},
    {gText_DexEmptyString, gTypeNames[TYPE_ICE]},
    {gText_DexEmptyString, gTypeNames[TYPE_DRAGON]},
    {gText_DexEmptyString, gTypeNames[TYPE_DARK]},
    #ifdef BATTLE_ENGINE
    {gText_DexEmptyString, gTypeNames[TYPE_FAIRY]},
    #endif
    {},
};

static const u8 sPokedexModes[] = {DEX_MODE_HOENN, DEX_MODE_NATIONAL};
static const u8 sOrderOptions[] =
{
    ORDER_NUMERICAL,
    ORDER_ALPHABETICAL,
    ORDER_HEAVIEST,
    ORDER_LIGHTEST,
    ORDER_TALLEST,
    ORDER_SMALLEST,
};

static const u8 sDexSearchTypeIds[NUMBER_OF_MON_TYPES] =
{
    TYPE_NONE,
    TYPE_NORMAL,
    TYPE_FIGHTING,
    TYPE_FLYING,
    TYPE_POISON,
    TYPE_GROUND,
    TYPE_ROCK,
    TYPE_BUG,
    TYPE_GHOST,
    TYPE_STEEL,
    TYPE_FIRE,
    TYPE_WATER,
    TYPE_GRASS,
    TYPE_ELECTRIC,
    TYPE_PSYCHIC,
    TYPE_ICE,
    TYPE_DRAGON,
    TYPE_DARK,
    #ifdef BATTLE_ENGINE
    TYPE_FAIRY,
    #endif
};

// Number pairs are the task data for tracking the cursor pos and scroll offset of each option list
// See task data defines above Task_LoadSearchMenu
static const struct SearchOption sSearchOptions[] =
{
    [SEARCH_NAME]       = {sDexSearchNameOptions,  6,  7, ARRAY_COUNT(sDexSearchNameOptions) - 1},
    [SEARCH_COLOR]      = {sDexSearchColorOptions, 8,  9, ARRAY_COUNT(sDexSearchColorOptions) - 1},
    [SEARCH_TYPE_LEFT]  = {sDexSearchTypeOptions, 10, 11, ARRAY_COUNT(sDexSearchTypeOptions) - 1},
    [SEARCH_TYPE_RIGHT] = {sDexSearchTypeOptions, 12, 13, ARRAY_COUNT(sDexSearchTypeOptions) - 1},
    [SEARCH_ORDER]      = {sDexOrderOptions,       4,  5, ARRAY_COUNT(sDexOrderOptions) - 1},
    [SEARCH_MODE]       = {sDexModeOptions,        2,  3, ARRAY_COUNT(sDexModeOptions) - 1},
};

static const struct BgTemplate sSearchMenu_BgTemplate[] =
{
    {
        .bg = 0,
        .charBaseIndex = 2,
        .mapBaseIndex = 12,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0
    },
    {
        .bg = 1,
        .charBaseIndex = 0,
        .mapBaseIndex = 13,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 1,
        .baseTile = 0
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 14,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 0,
        .mapBaseIndex = 15,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0
    }
};

static const struct WindowTemplate sSearchMenu_WindowTemplate[] =
{
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 32,
        .height = 20,
        .paletteNum = 0,
        .baseBlock = 0x0001,
    },
    DUMMY_WIN_TEMPLATE
};


//************************************
//*                                  *
//*        MAIN                      *
//*                                  *
//************************************
void CB2_OpenPokedexPlusHGSS(void)
{
    // Opened normally (start menu): forget any local-mode exit left over from the key item
    if (gMain.state == 0)
        sLocalModeExitCallback = NULL;
    OpenPokedexPlusHGSS();
}

static void CB2_OpenPokedexPlusHGSSLocalMode(void)
{
    OpenPokedexPlusHGSS();
}

void OpenPokedexLocalMode(MainCallback exitCallback)
{
    sLocalModeExitCallback = exitCallback;
    SetMainCallback2(CB2_OpenPokedexPlusHGSSLocalMode);
}

static void Task_OpenLocalModeDirectly(u8 taskId)
{
    sPokedexView->isSearchResults = FALSE;
    gMain.state = 0;
    gTasks[taskId].func = Task_LoadLocalScreen;
}

static void OpenPokedexPlusHGSS(void)
{
    switch (gMain.state)
    {
    case 0:
    default:
        SetVBlankCallback(NULL);
        ResetOtherVideoRegisters(0);
        DmaFillLarge16(3, 0, (u8 *)VRAM, VRAM_SIZE, 0x1000);
        DmaClear32(3, OAM, OAM_SIZE);
        DmaClear16(3, PLTT, PLTT_SIZE);
        gMain.state = 1;
        break;
    case 1:
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        ResetPaletteFade();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 8;
        ResetAllPicSprites();
        gMain.state++;
        break;
    case 2:
        sPokedexView = AllocZeroed(sizeof(struct PokedexView));
        ResetPokedexView(sPokedexView);
        BuildNatDexToSpeciesTable();
        CreateTask(sLocalModeExitCallback != NULL ? Task_OpenLocalModeDirectly : Task_OpenPokedexMainPage, 0);
        sPokedexView->dexMode = gSaveBlock2Ptr->pokedex.mode;
        if (!IsNationalPokedexEnabled())
            sPokedexView->dexMode = DEX_MODE_HOENN;
        sPokedexView->dexOrder = gSaveBlock2Ptr->pokedex.order;
        sPokedexView->selectedPokemon = sLastSelectedPokemon;
        sPokedexView->pokeBallRotation = sPokeBallRotation;
        sPokedexView->selectedScreen = AREA_SCREEN;
        if (!IsNationalPokedexEnabled())
        {
            sPokedexView->seenCount = GetPokedexDisplayCount(TRUE, FLAG_GET_SEEN);
            sPokedexView->ownCount = GetPokedexDisplayCount(TRUE, FLAG_GET_CAUGHT);
        }
        else
        {
            sPokedexView->seenCount = GetPokedexDisplayCount(FALSE, FLAG_GET_SEEN);
            sPokedexView->ownCount = GetPokedexDisplayCount(FALSE, FLAG_GET_CAUGHT);
        }
        sPokedexView->initialVOffset = 8;
        gMain.state++;
        break;
    case 3:
        EnableInterrupts(1);
        SetVBlankCallback(VBlankCB_Pokedex);
        SetMainCallback2(CB2_Pokedex);
        CreatePokedexList(sPokedexView->dexMode, sPokedexView->dexOrder);
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x80);
        break;
    }
}

// Called from ResetPokedexScrollPositions (pokedex.c), e.g. on new game
void ResetPokedexPlusHGSSScrollPositions(void)
{
    sLastSelectedPokemon = 0;
    sPokeBallRotation = POKEBALL_ROTATION_TOP;
}

static void ResetPokedexView(struct PokedexView *pokedexView)
{
    u16 i;

    for (i = 0; i < NATIONAL_DEX_COUNT; i++)
    {
        pokedexView->pokedexList[i].dexNum = 0xFFFF;
        pokedexView->pokedexList[i].seen = FALSE;
        pokedexView->pokedexList[i].owned = FALSE;
    }
    pokedexView->pokedexList[NATIONAL_DEX_COUNT].dexNum = 0;
    pokedexView->pokedexList[NATIONAL_DEX_COUNT].seen = FALSE;
    pokedexView->pokedexList[NATIONAL_DEX_COUNT].owned = FALSE;
    pokedexView->pokemonListCount = 0;
    pokedexView->selectedPokemon = 0;
    pokedexView->selectedPokemonBackup = 0;
    pokedexView->dexMode = DEX_MODE_HOENN;
    pokedexView->dexModeBackup = DEX_MODE_HOENN;
    pokedexView->dexOrder = ORDER_NUMERICAL;
    pokedexView->dexOrderBackup = ORDER_NUMERICAL;
    pokedexView->seenCount = 0;
    pokedexView->ownCount = 0;
    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
        pokedexView->monSpriteIds[i] = 0xFFFF;
    pokedexView->pokeBallRotationStep = 0;
    pokedexView->pokeBallRotationBackup = 0;
    pokedexView->pokeBallRotation = 0;
    pokedexView->initialVOffset = 0;
    pokedexView->scrollTimer = 0;
    pokedexView->scrollDirection = 0;
    pokedexView->listVOffset = 0;
    pokedexView->listMovingVOffset = 0;
    pokedexView->scrollMonIncrement = 0;
    pokedexView->maxScrollTimer = 0;
    pokedexView->scrollSpeed = 0;
    for (i = 0; i < ARRAY_COUNT(pokedexView->unkArr1); i++)
        pokedexView->unkArr1[i] = 0;
    pokedexView->currentPage = PAGE_MAIN;
    pokedexView->currentPageBackup = PAGE_MAIN;
    pokedexView->isSearchResults = FALSE;
    pokedexView->selectedScreen = AREA_SCREEN;
    pokedexView->screenSwitchState = 0;
    pokedexView->menuIsOpen = 0;
    pokedexView->menuCursorPos = 0;
    pokedexView->menuY = 0;
    for (i = 0; i < ARRAY_COUNT(pokedexView->unkArr2); i++)
        pokedexView->unkArr2[i] = 0;
    for (i = 0; i < ARRAY_COUNT(pokedexView->unkArr3); i++)
        pokedexView->unkArr3[i] = 0;
}

static void VBlankCB_Pokedex(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void CB2_Pokedex(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

void Task_OpenPokedexMainPage(u8 taskId)
{
    sPokedexView->isSearchResults = FALSE;
    sPokedexView->sEvoScreenData.fromEvoPage = FALSE;
    #ifdef POKEMON_EXPANSION
    sPokedexView->formSpecies = 0;
    #endif
    if (LoadPokedexListPage(PAGE_MAIN))
        gTasks[taskId].func = Task_HandlePokedexInput;
}

#define tLoadScreenTaskId data[0]

static void Task_HandlePokedexInput(u8 taskId)
{
    SetGpuReg(REG_OFFSET_BG0VOFS, sPokedexView->menuY);

    if (sPokedexView->menuY)
    {
        sPokedexView->menuY -= 8;
        if (sPokedexView->menuIsOpen == FALSE && sPokedexView->menuY == 8) //HGSS_Ui
        {
            CreateStatBars(&sPokedexView->pokedexList[sPokedexView->selectedPokemon]);
            CreateStatBarsBg();
        }
    }
    else
    {
        if (JOY_NEW(A_BUTTON) && sPokedexView->pokedexList[sPokedexView->selectedPokemon].seen)
        {
            TryDestroyStatBars(); //HGSS_Ui
            UpdateSelectedMonSpriteId();
            BeginNormalPaletteFade(~(1 << (gSprites[sPokedexView->selectedMonSpriteId].oam.paletteNum + 16)), 0, 0, 0x10, RGB_BLACK);
            gSprites[sPokedexView->selectedMonSpriteId].callback = SpriteCB_MoveMonForInfoScreen;
            gTasks[taskId].func = Task_OpenInfoScreenAfterMonMovement;
            PlaySE(SE_PIN);
            FreeWindowAndBgBuffers();
        }
        else if (JOY_NEW(START_BUTTON))
        {
            sPokedexView->menuY = 0;
            sPokedexView->menuIsOpen = TRUE;
            sPokedexView->menuCursorPos = 0;
            gTasks[taskId].func = Task_HandlePokedexStartMenuInput;
            PlaySE(SE_SELECT);
        }
        else if (JOY_NEW(SELECT_BUTTON))
        {
            PlaySE(SE_SELECT);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].tLoadScreenTaskId = LoadSearchMenu();
            sPokedexView->screenSwitchState = 0;
            sPokedexView->pokeBallRotationBackup = sPokedexView->pokeBallRotation;
            sPokedexView->selectedPokemonBackup = sPokedexView->selectedPokemon;
            sPokedexView->dexModeBackup = sPokedexView->dexMode;
            sPokedexView->dexOrderBackup = sPokedexView->dexOrder;
            gTasks[taskId].func = Task_WaitForExitSearch;
            PlaySE(SE_PC_LOGIN);
            FreeWindowAndBgBuffers();
        }
        else if (JOY_NEW(B_BUTTON))
        {
            TryDestroyStatBars(); //HGSS_Ui
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].func = Task_ClosePokedex;
            PlaySE(SE_PC_OFF);
        }
        else
        {
            //Handle D-pad
            sPokedexView->selectedPokemon = TryDoPokedexScroll(sPokedexView->selectedPokemon, 0xE);
            if (sPokedexView->scrollTimer)
                gTasks[taskId].func = Task_WaitForScroll;
            else if (!sPokedexView->scrollTimer && !sPokedexView->scrollSpeed &&sPokedexView->justScrolled) //HGSS_Ui
                CreateStatBars(&sPokedexView->pokedexList[sPokedexView->selectedPokemon]); //HGSS_Ui
        }
    }
}

static void Task_WaitForScroll(u8 taskId)
{
    TryDestroyStatBars(); //HGSS_Ui
    if (UpdateDexListScroll(sPokedexView->scrollDirection, sPokedexView->scrollMonIncrement, sPokedexView->maxScrollTimer))
        gTasks[taskId].func = Task_HandlePokedexInput;
}

#define MAIN_MENU_Y 96
#define MAIN_MENU_CURSOR_X 106

// MARK ALL SEEN / MARK ALL CAPTURED only change what the Pokédex shows. The real dex flags are
// never written, so turning the option back off restores exactly what the player has registered.
s8 GetPokedexDisplayFlag(u16 nationalNum, u8 caseId)
{
    if (nationalNum != 0 && nationalNum <= NATIONAL_DEX_COUNT)
    {
        if (sSearchAsAllCaught || FlagGet(FLAG_DEX_SHOW_ALL_CAUGHT))
            return TRUE;
        if (caseId == FLAG_GET_SEEN && FlagGet(FLAG_DEX_SHOW_ALL_SEEN))
            return TRUE;
    }
    return GetSetPokedexFlag(nationalNum, caseId);
}

// The info pages follow the list entry, which a search with ALL on marks as caught
bool8 IsPokedexPageMonOwned(u16 species)
{
    u16 dexNum = SpeciesToNationalPokedexNum(species);

    if (sPokedexListItem != NULL && sPokedexListItem->dexNum == dexNum)
        return sPokedexListItem->owned;
    return GetPokedexDisplayFlag(dexNum, FLAG_GET_CAUGHT);
}

static u16 GetPokedexDisplayCount(bool8 hoenn, u8 caseId)
{
    u16 i;
    u16 count = 0;

    if (hoenn)
    {
        for (i = 1; i <= HOENN_DEX_COUNT; i++)
            count += (GetPokedexDisplayFlag(HoennToNationalOrder(i), caseId) != 0);
    }
    else
    {
        for (i = 1; i <= NATIONAL_DEX_COUNT; i++)
            count += (GetPokedexDisplayFlag(i, caseId) != 0);
    }
    return count;
}

static const u32 *GetStartMenuMainTilemap(void)
{
    static const u32 *const sTilemaps[] =
    {
        gPokedexPlusHGSS_StartMenuMain_Tilemap,
        gPokedexPlusHGSS_StartMenuMain_SeenOn_Tilemap,
        gPokedexPlusHGSS_StartMenuMain_CaughtOn_Tilemap,
        gPokedexPlusHGSS_StartMenuMain_BothOn_Tilemap,
    };

    return sTilemaps[FlagGet(FLAG_DEX_SHOW_ALL_SEEN) | (FlagGet(FLAG_DEX_SHOW_ALL_CAUGHT) << 1)];
}

static void ToggleDexDisplayFlag(u16 flag)
{
    if (FlagGet(flag))
        FlagClear(flag);
    else
        FlagSet(flag);
}

// Rebuilds the list page so the new seen/caught flags and counts are shown
static void Task_ReloadPokedexListAfterMarkAll(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        u16 i;
        u16 dexNum = sPokedexView->pokedexList[sPokedexView->selectedPokemon].dexNum;

        // The list starts at the first seen Pokémon, so keep the same Pokémon selected
        CreatePokedexList(sPokedexView->dexMode, sPokedexView->dexOrder);
        for (i = 0; i < sPokedexView->pokemonListCount; i++)
        {
            if (sPokedexView->pokedexList[i].dexNum == dexNum)
            {
                sPokedexView->selectedPokemon = i;
                break;
            }
        }
        // Unmarking can shorten the list past the old selection
        if (i == sPokedexView->pokemonListCount && sPokedexView->selectedPokemon >= sPokedexView->pokemonListCount)
            sPokedexView->selectedPokemon = sPokedexView->pokemonListCount - 1;
        sPokedexView->menuIsOpen = FALSE;
        sPokedexView->menuY = 0;
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
        if (!IsNationalPokedexEnabled())
        {
            sPokedexView->seenCount = GetPokedexDisplayCount(TRUE, FLAG_GET_SEEN);
            sPokedexView->ownCount = GetPokedexDisplayCount(TRUE, FLAG_GET_CAUGHT);
        }
        else
        {
            sPokedexView->seenCount = GetPokedexDisplayCount(FALSE, FLAG_GET_SEEN);
            sPokedexView->ownCount = GetPokedexDisplayCount(FALSE, FLAG_GET_CAUGHT);
        }
        ClearMonSprites();
        TryDestroyStatBars();
        TryDestroyStatBarsBg();
        FreeWindowAndBgBuffers();
        gTasks[taskId].func = Task_OpenPokedexMainPage;
    }
}

static void Task_HandlePokedexStartMenuInput(u8 taskId)
{
    SetGpuReg(REG_OFFSET_BG0VOFS, sPokedexView->menuY);

    //If menu is not open, slide it up, on screen
    if (sPokedexView->menuY != MAIN_MENU_Y)
    {
        sPokedexView->menuY += 8;
    }
    else
    {
        if (JOY_NEW(A_BUTTON))
        {
            switch (sPokedexView->menuCursorPos)
            {
            case 0: //LOCAL
            default:
                PlaySE(SE_PIN);
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
                gTasks[taskId].func = Task_OpenLocalScreen;
                return;
            case 1: //MARK / UNMARK ALL SEEN
            case 2: //MARK / UNMARK ALL CAPTURED
                ToggleDexDisplayFlag(sPokedexView->menuCursorPos == 1 ? FLAG_DEX_SHOW_ALL_SEEN : FLAG_DEX_SHOW_ALL_CAUGHT);
                PlaySE(SE_SUCCESS);
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
                gTasks[taskId].func = Task_ReloadPokedexListAfterMarkAll;
                return;
            case 3: //LIST TOP
                sPokedexView->selectedPokemon = 0;
                sPokedexView->pokeBallRotation = POKEBALL_ROTATION_TOP;
                ClearMonSprites();
                CreateMonSpritesAtPos(sPokedexView->selectedPokemon, 0xE);
                gMain.newKeys |= START_BUTTON;  //Exit menu
                break;
            case 4: //LIST BOTTOM
                sPokedexView->selectedPokemon = sPokedexView->pokemonListCount - 1;
                sPokedexView->pokeBallRotation = sPokedexView->pokemonListCount * 16 + POKEBALL_ROTATION_BOTTOM;
                ClearMonSprites();
                CreateMonSpritesAtPos(sPokedexView->selectedPokemon, 0xE);
                gMain.newKeys |= START_BUTTON;  //Exit menu
                break;
            }
        }

        //Exit menu when Start or B is pressed
        if (JOY_NEW(START_BUTTON | B_BUTTON))
        {
            sPokedexView->menuIsOpen = FALSE;
            gTasks[taskId].func = Task_HandlePokedexInput;
            PlaySE(SE_SELECT);
        }
        else if (JOY_REPEAT(DPAD_UP) && sPokedexView->menuCursorPos != 0)
        {
            sPokedexView->menuCursorPos--;
            PlaySE(SE_SELECT);
        }
        else if (JOY_REPEAT(DPAD_DOWN) && sPokedexView->menuCursorPos < 4)
        {
            sPokedexView->menuCursorPos++;
            PlaySE(SE_SELECT);
        }
    }
}

// Opening the info screen from list view. Pokémon sprite is moving to its new position, wait for it to arrive
static void Task_OpenInfoScreenAfterMonMovement(u8 taskId)
{
    if (gSprites[sPokedexView->selectedMonSpriteId].x == MON_PAGE_X && gSprites[sPokedexView->selectedMonSpriteId].y == MON_PAGE_Y)
    {
        sPokedexView->currentPageBackup = sPokedexView->currentPage;
        gTasks[taskId].tLoadScreenTaskId = LoadInfoScreen(&sPokedexView->pokedexList[sPokedexView->selectedPokemon], sPokedexView->selectedMonSpriteId);
        gTasks[taskId].func = Task_WaitForExitInfoScreen;
    }
}

static void Task_WaitForExitInfoScreen(u8 taskId)
{
    if (gTasks[gTasks[taskId].tLoadScreenTaskId].isActive)
    {
        // While active, handle scroll input
        if (sPokedexView->currentPage == PAGE_INFO && !IsInfoScreenScrolling(gTasks[taskId].tLoadScreenTaskId) && TryDoInfoScreenScroll())
            StartInfoScreenScroll(&sPokedexView->pokedexList[sPokedexView->selectedPokemon], gTasks[taskId].tLoadScreenTaskId);
    }
    else
    {
        // Exiting, back to list view
        sLastSelectedPokemon = sPokedexView->selectedPokemon;
        sPokeBallRotation = sPokedexView->pokeBallRotation;
        gTasks[taskId].func = Task_OpenPokedexMainPage;
    }
}


static void Task_ClosePokedex(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        gSaveBlock2Ptr->pokedex.mode = sPokedexView->dexMode;
        if (!IsNationalPokedexEnabled())
            gSaveBlock2Ptr->pokedex.mode = DEX_MODE_HOENN;
        gSaveBlock2Ptr->pokedex.order = sPokedexView->dexOrder;
        ClearMonSprites();
        FreeWindowAndBgBuffers();
        DestroyTask(taskId);
        SetMainCallback2(CB2_ReturnToFieldWithOpenMenu);
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x100);
        FREE_AND_SET_NULL(sPokedexView);
    }
}

static void LoadPokedexBgPalette(bool8 isSearchResults)
{
    if (!HGSS_DARK_MODE)
    {
        if (isSearchResults == TRUE)
            LoadPalette(gPokedexPlusHGSS_SearchResults_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(6 * 16 - 1));
        else if (!IsNationalPokedexEnabled())
            LoadPalette(gPokedexPlusHGSS_Default_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(6 * 16 - 1));
        else
            LoadPalette(gPokedexPlusHGSS_National_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(6 * 16 - 1));
        LoadPalette(GetOverworldTextboxPalettePtr(), 0xF0, 32);
    }
    else
    {
        if (isSearchResults == TRUE)
            LoadPalette(gPokedexPlusHGSS_SearchResults_dark_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(6 * 16 - 1));
        else if (!IsNationalPokedexEnabled())
            LoadPalette(gPokedexPlusHGSS_Default_dark_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(6 * 16 - 1));
        else
            LoadPalette(gPokedexPlusHGSS_National_dark_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(6 * 16 - 1));
        LoadPalette(GetOverworldTextboxPalettePtr(), 0xF0, 32);
    }

}


//************************************
//*                                  *
//*    Main scrolling list screen    *
//*                                  *
//************************************

// For loading main pokedex page or pokedex search results
static bool8 LoadPokedexListPage(u8 page)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (gPaletteFade.active)
            return 0;
        SetVBlankCallback(NULL);
        sPokedexView->currentPage = page;
        ResetOtherVideoRegisters(0);
        SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sPokedex_BgTemplate, ARRAY_COUNT(sPokedex_BgTemplate));
        SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
        SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
        SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
        SetBgTilemapBuffer(0, AllocZeroed(BG_SCREEN_SIZE));
        if (!HGSS_DECAPPED)
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_MenuList_Gfx, 0x2000, 0, 0);
        else
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_MenuList_DECA_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(1, gPokedexPlusHGSS_ScreenList_Tilemap, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenListUnderlay_Tilemap, 0, 0);
        if (page == PAGE_MAIN)
            CopyToBgTilemapBuffer(0, GetStartMenuMainTilemap(), 0, 0x280);
        else
            CopyToBgTilemapBuffer(0, gPokedexPlusHGSS_StartMenuSearchResults_Tilemap, 0, 0x280);
        ResetPaletteFade();
        if (page == PAGE_MAIN)
            sPokedexView->isSearchResults = FALSE;
        else
            sPokedexView->isSearchResults = TRUE;
        LoadPokedexBgPalette(sPokedexView->isSearchResults);
        InitWindows(sPokemonList_WindowTemplate);
        DeactivateAllTextPrinters();
        PutWindowTilemap(0);
        CopyWindowToVram(0, COPYWIN_FULL);
        gMain.state = 1;
        break;
    case 1:
        ResetSpriteData();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 8;
        LoadCompressedSpriteSheet(&sInterfaceSpriteSheet[HGSS_DECAPPED]);
        LoadSpritePalettes(sInterfaceSpritePalette);
        LoadSpritePalettes(sStatBarSpritePal); //HGSS_Ui
        CreateInterfaceSprites(page);
        gMain.state++;
        break;
    case 2:
        gMain.state++;
        break;
    case 3:
        if (page == PAGE_MAIN)
            CreatePokedexList(sPokedexView->dexMode, sPokedexView->dexOrder);
        CreateMonSpritesAtPos(sPokedexView->selectedPokemon, 0xE);
        sPokedexView->statBarsSpriteId = 0xFF;  //HGSS_Ui stat bars
        CreateStatBars(&sPokedexView->pokedexList[sPokedexView->selectedPokemon]); //HGSS_Ui stat bars
        sPokedexView->statBarsBgSpriteId = 0xFF;  //HGSS_Ui stat bars background
        CreateStatBarsBg(); //HGSS_Ui stat bars background
        sPokedexView->menuIsOpen = FALSE;
        sPokedexView->menuY = 0;
        CopyBgTilemapBufferToVram(0);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 4:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_Pokedex);
        gMain.state++;
        break;
    case 5:
        SetGpuReg(REG_OFFSET_WININ, WININ_WIN0_ALL | WININ_WIN1_ALL);
        SetGpuReg(REG_OFFSET_WINOUT, WINOUT_WIN01_ALL | WINOUT_WINOBJ_BG0 | WINOUT_WINOBJ_BG2 | WINOUT_WINOBJ_BG3 | WINOUT_WINOBJ_OBJ);
        SetGpuReg(REG_OFFSET_WIN0H, 0);
        SetGpuReg(REG_OFFSET_WIN0V, 0);
        SetGpuReg(REG_OFFSET_WIN1H, 0);
        SetGpuReg(REG_OFFSET_WIN1V, 0);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_OBJWIN_ON);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 6:
        if (!gPaletteFade.active)
        {
            gMain.state = 0;
            return TRUE;
        }
        break;
    }
    return FALSE;
}

static void CreatePokedexList(u8 dexMode, u8 order)
{
    u16 vars[3]; //I have no idea why three regular variables are stored in an array, but whatever.
#define temp_dexCount   vars[0]
#define temp_isHoennDex vars[1]
#define temp_dexNum     vars[2]
    s16 i;

    sPokedexView->pokemonListCount = 0;

    switch (dexMode)
    {
    default:
    case DEX_MODE_HOENN:
        temp_dexCount = HOENN_DEX_COUNT;
        temp_isHoennDex = TRUE;
        break;
    case DEX_MODE_NATIONAL:
        if (IsNationalPokedexEnabled())
        {
            temp_dexCount = NATIONAL_DEX_COUNT;
            temp_isHoennDex = FALSE;
        }
        else
        {
            temp_dexCount = HOENN_DEX_COUNT;
            temp_isHoennDex = TRUE;
        }
        break;
    }

    switch (order)
    {
    case ORDER_NUMERICAL:
        if (temp_isHoennDex)
        {
            for (i = 0; i < temp_dexCount; i++)
            {
                temp_dexNum = HoennToNationalOrder(i + 1);
                sPokedexView->pokedexList[i].dexNum = temp_dexNum;
                sPokedexView->pokedexList[i].seen = GetPokedexDisplayFlag(temp_dexNum, FLAG_GET_SEEN);
                sPokedexView->pokedexList[i].owned = GetPokedexDisplayFlag(temp_dexNum, FLAG_GET_CAUGHT);
                if (sPokedexView->pokedexList[i].seen)
                    sPokedexView->pokemonListCount = i + 1;
            }
        }
        else
        {
            s16 r5, r10;
            for (i = 0, r5 = 0, r10 = 0; i < temp_dexCount; i++)
            {
                temp_dexNum = i + 1;
                if (GetPokedexDisplayFlag(temp_dexNum, FLAG_GET_SEEN))
                    r10 = 1;
                if (r10)
                {
                    sPokedexView->pokedexList[r5].dexNum = temp_dexNum;
                    sPokedexView->pokedexList[r5].seen = GetPokedexDisplayFlag(temp_dexNum, FLAG_GET_SEEN);
                    sPokedexView->pokedexList[r5].owned = GetPokedexDisplayFlag(temp_dexNum, FLAG_GET_CAUGHT);
                    if (sPokedexView->pokedexList[r5].seen)
                        sPokedexView->pokemonListCount = r5 + 1;
                    r5++;
                }
            }
        }
        break;
    case ORDER_ALPHABETICAL:
        for (i = 0; i < NUM_SPECIES - 1; i++)
        {
            temp_dexNum = gPokedexOrder_Alphabetical[i];

            if (NationalToHoennOrder(temp_dexNum) <= temp_dexCount && GetPokedexDisplayFlag(temp_dexNum, FLAG_GET_SEEN))
            {
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].dexNum = temp_dexNum;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].seen = TRUE;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].owned = GetPokedexDisplayFlag(temp_dexNum, FLAG_GET_CAUGHT);
                sPokedexView->pokemonListCount++;
            }
        }
        break;
    case ORDER_HEAVIEST:
        for (i = NATIONAL_DEX_COUNT - 1; i >= 0; i--)
        {
            temp_dexNum = gPokedexOrder_Weight[i];

            if (NationalToHoennOrder(temp_dexNum) <= temp_dexCount && GetPokedexDisplayFlag(temp_dexNum, FLAG_GET_CAUGHT))
            {
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].dexNum = temp_dexNum;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].seen = TRUE;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].owned = TRUE;
                sPokedexView->pokemonListCount++;
            }
        }
        break;
    case ORDER_LIGHTEST:
        for (i = 0; i < NATIONAL_DEX_COUNT; i++)
        {
            temp_dexNum = gPokedexOrder_Weight[i];

            if (NationalToHoennOrder(temp_dexNum) <= temp_dexCount && GetPokedexDisplayFlag(temp_dexNum, FLAG_GET_CAUGHT))
            {
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].dexNum = temp_dexNum;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].seen = TRUE;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].owned = TRUE;
                sPokedexView->pokemonListCount++;
            }
        }
        break;
    case ORDER_TALLEST:
        for (i = NATIONAL_DEX_COUNT - 1; i >= 0; i--)
        {
            temp_dexNum = gPokedexOrder_Height[i];

            if (NationalToHoennOrder(temp_dexNum) <= temp_dexCount && GetPokedexDisplayFlag(temp_dexNum, FLAG_GET_CAUGHT))
            {
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].dexNum = temp_dexNum;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].seen = TRUE;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].owned = TRUE;
                sPokedexView->pokemonListCount++;
            }
        }
        break;
    case ORDER_SMALLEST:
        for (i = 0; i < NATIONAL_DEX_COUNT; i++)
        {
            temp_dexNum = gPokedexOrder_Height[i];

            if (NationalToHoennOrder(temp_dexNum) <= temp_dexCount && GetPokedexDisplayFlag(temp_dexNum, FLAG_GET_CAUGHT))
            {
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].dexNum = temp_dexNum;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].seen = TRUE;
                sPokedexView->pokedexList[sPokedexView->pokemonListCount].owned = TRUE;
                sPokedexView->pokemonListCount++;
            }
        }
        break;
    }

    for (i = sPokedexView->pokemonListCount; i < NATIONAL_DEX_COUNT; i++)
    {
        sPokedexView->pokedexList[i].dexNum = 0xFFFF;
        sPokedexView->pokedexList[i].seen = FALSE;
        sPokedexView->pokedexList[i].owned = FALSE;
    }
}

static void PrintMonDexNumAndName(u8 windowId, u8 fontId, const u8 *str, u8 left, u8 top)
{
    u8 color[3];

    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;
    AddTextPrinterParameterized4(windowId, fontId, left * 8, (top * 8) + 1, 0, 0, color, TEXT_SKIP_DRAW, str);
}

static void PrintMonDexNumAndName_2(u8 windowId, u8 fontId, const u8* str, u8 left, u8 top) //HGSS_Ui offset for closer numer + text
{
    u8 color[3];

    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;
    AddTextPrinterParameterized4(windowId, fontId, left * 8 - 3, (top * 8) + 1, 0, 0, color, -1, str);
}

// u16 ignored is passed but never used
#define MON_LIST_X 17
static void CreateMonListEntry(u8 position, u16 b, u16 ignored)
{
    s16 entryNum;
    u16 i;
    u16 vOffset;

    switch (position)
    {
    case 0: // Initial
    default:
        entryNum = b - 5;
        for (i = 0; i <= 10; i++)
        {
            if (entryNum < 0 || entryNum >= NATIONAL_DEX_COUNT || sPokedexView->pokedexList[entryNum].dexNum == 0xFFFF)
            {
                ClearMonListEntry(MON_LIST_X, i * 2, ignored);
            }
            else
            {
                ClearMonListEntry(MON_LIST_X, i * 2, ignored);
                if (sPokedexView->pokedexList[entryNum].seen)
                {
                    CreateMonDexNum(entryNum, MON_LIST_X+1, i * 2, ignored);
                    CreateCaughtBall(sPokedexView->pokedexList[entryNum].owned, MON_LIST_X, i * 2, ignored);
                    CreateMonName(sPokedexView->pokedexList[entryNum].dexNum, MON_LIST_X+5, i * 2);
                }
                else
                {
                    CreateMonDexNum(entryNum, MON_LIST_X+1, i * 2, ignored);
                    CreateCaughtBall(FALSE, MON_LIST_X, i * 2, ignored);
                    CreateMonName(0, MON_LIST_X+5, i * 2);
                }
            }
            entryNum++;
        }
        break;
    case 1: // Up
        entryNum = b - 5;
        if (entryNum < 0 || entryNum >= NATIONAL_DEX_COUNT || sPokedexView->pokedexList[entryNum].dexNum == 0xFFFF)
        {
            ClearMonListEntry(MON_LIST_X, sPokedexView->listVOffset * 2, ignored);
        }
        else
        {
            ClearMonListEntry(MON_LIST_X, sPokedexView->listVOffset * 2, ignored);
            if (sPokedexView->pokedexList[entryNum].seen)
            {
                CreateMonDexNum(entryNum, MON_LIST_X+1, sPokedexView->listVOffset * 2, ignored);
                CreateCaughtBall(sPokedexView->pokedexList[entryNum].owned, MON_LIST_X, sPokedexView->listVOffset * 2, ignored);
                CreateMonName(sPokedexView->pokedexList[entryNum].dexNum, MON_LIST_X+5, sPokedexView->listVOffset * 2);
            }
            else
            {
                CreateMonDexNum(entryNum, MON_LIST_X+1, sPokedexView->listVOffset * 2, ignored);
                CreateCaughtBall(FALSE, MON_LIST_X, sPokedexView->listVOffset * 2, ignored);
                CreateMonName(0, MON_LIST_X+5, sPokedexView->listVOffset * 2);
            }
        }
        break;
    case 2: // Down
        entryNum = b + 5;
        vOffset = sPokedexView->listVOffset + 10;
        if (vOffset >= LIST_SCROLL_STEP)
            vOffset -= LIST_SCROLL_STEP;
        if (entryNum < 0 || entryNum >= NATIONAL_DEX_COUNT || sPokedexView->pokedexList[entryNum].dexNum == 0xFFFF)
            ClearMonListEntry(MON_LIST_X, vOffset * 2, ignored);
        else
        {
            ClearMonListEntry(MON_LIST_X, vOffset * 2, ignored);
            if (sPokedexView->pokedexList[entryNum].seen)
            {
                CreateMonDexNum(entryNum, MON_LIST_X+1, vOffset * 2, ignored);
                CreateCaughtBall(sPokedexView->pokedexList[entryNum].owned, MON_LIST_X, vOffset * 2, ignored);
                CreateMonName(sPokedexView->pokedexList[entryNum].dexNum, MON_LIST_X+5, vOffset * 2);
            }
            else
            {
                CreateMonDexNum(entryNum, MON_LIST_X+1, vOffset * 2, ignored);
                CreateCaughtBall(FALSE, MON_LIST_X, vOffset * 2, ignored);
                CreateMonName(0, MON_LIST_X+5, vOffset * 2);
            }
        }
        break;
    }
    CopyWindowToVram(0, COPYWIN_GFX);
}

static void CreateMonDexNum(u16 entryNum, u8 left, u8 top, u16 unused)
{
    u8 text[6];
    u16 dexNum;

    memcpy(text, sText_No000, ARRAY_COUNT(text));
    dexNum = sPokedexView->pokedexList[entryNum].dexNum;
    if (sPokedexView->dexMode == DEX_MODE_HOENN)
        dexNum = NationalToHoennOrder(dexNum);
    text[2] = CHAR_0 + dexNum / 100;
    text[3] = CHAR_0 + (dexNum % 100) / 10;
    text[4] = CHAR_0 + (dexNum % 100) % 10;
    PrintMonDexNumAndName(0, FONT_NARROW, text, left, top);
}

static void CreateCaughtBall(bool16 owned, u8 x, u8 y, u16 unused)
{
    if (owned)
        BlitBitmapToWindow(0, sCaughtBall_Gfx, x * 8, y * 8, 8, 16);
    else
        FillWindowPixelRect(0, PIXEL_FILL(0), x * 8, y * 8, 8, 16);
}

static u8 CreateMonName(u16 num, u8 left, u8 top)
{
    const u8 *str;

    num = DexNumToSpecies(num);
    if (num)
        str = gSpeciesNames[num];
    else
        str = sText_TenDashes;
    PrintMonDexNumAndName_2(0, FONT_NARROW, str, left, top);
    return StringLength(str);
}

static void ClearMonListEntry(u8 x, u8 y, u16 unused)
{
    FillWindowPixelRect(0, PIXEL_FILL(0), x * 8, y * 8, 0x60, 16);
}

// u16 ignored is passed but never used
static void CreateMonSpritesAtPos(u16 selectedMon, u16 ignored)
{
    u8 i;
    u16 dexNum;
    u8 spriteId;

    gPaletteFade.bufferTransferDisabled = TRUE;

    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
        sPokedexView->monSpriteIds[i] = 0xFFFF;
    sPokedexView->selectedMonSpriteId = 0xFFFF;

    // Create top mon sprite
    dexNum = GetPokemonSpriteToDisplay(selectedMon - 1);
    if (dexNum != 0xFFFF)
    {
        spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
        gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
        gSprites[spriteId].data[5] = -32;
    }

    // Create mid mon sprite
    dexNum = GetPokemonSpriteToDisplay(selectedMon);
    if (dexNum != 0xFFFF)
    {
        spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
        gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
        gSprites[spriteId].data[5] = 0;
    }

    // Create bottom mon sprite
    dexNum = GetPokemonSpriteToDisplay(selectedMon + 1);
    if (dexNum != 0xFFFF)
    {
        spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
        gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
        gSprites[spriteId].data[5] = 32;
    }

    CreateMonListEntry(0, selectedMon, ignored);
    SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset);

    sPokedexView->listVOffset = 0;
    sPokedexView->listMovingVOffset = 0;

    gPaletteFade.bufferTransferDisabled = FALSE;
}

static bool8 UpdateDexListScroll(u8 direction, u8 monMoveIncrement, u8 scrollTimerMax)
{
    u16 i;
    u8 step;

    if (sPokedexView->scrollTimer)
    {
        sPokedexView->scrollTimer--;
        switch (direction)
        {
        case 1: // Up
            for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
            {
                if (sPokedexView->monSpriteIds[i] != 0xFFFF)
                    gSprites[sPokedexView->monSpriteIds[i]].data[5] += monMoveIncrement;
            }
            step = LIST_SCROLL_STEP * (scrollTimerMax - sPokedexView->scrollTimer) / scrollTimerMax;
            SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset + sPokedexView->listMovingVOffset * LIST_SCROLL_STEP - step);
            sPokedexView->pokeBallRotation -= sPokedexView->pokeBallRotationStep;
            break;
        case 2: // Down
            for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
            {
                if (sPokedexView->monSpriteIds[i] != 0xFFFF)
                    gSprites[sPokedexView->monSpriteIds[i]].data[5] -= monMoveIncrement;
            }
            step = LIST_SCROLL_STEP * (scrollTimerMax - sPokedexView->scrollTimer) / scrollTimerMax;
            SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset + sPokedexView->listMovingVOffset * LIST_SCROLL_STEP + step);
            sPokedexView->pokeBallRotation += sPokedexView->pokeBallRotationStep;
            break;
        }
        return FALSE;
    }
    else
    {
        SetGpuReg(REG_OFFSET_BG2VOFS, sPokedexView->initialVOffset + sPokedexView->listVOffset * LIST_SCROLL_STEP);
        return TRUE;
    }
}

static void CreateScrollingPokemonSprite(u8 direction, u16 selectedMon)
{
    u16 dexNum;
    u8 spriteId;

    sPokedexView->listMovingVOffset = sPokedexView->listVOffset;
    switch (direction)
    {
    case 1: // up
        dexNum = GetPokemonSpriteToDisplay(selectedMon - 1);
        if (dexNum != 0xFFFF)
        {
            spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
            gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
            gSprites[spriteId].data[5] = -64;
        }
        if (sPokedexView->listVOffset > 0)
            sPokedexView->listVOffset--;
        else
            sPokedexView->listVOffset = LIST_SCROLL_STEP - 1;
        break;
    case 2: // down
        dexNum = GetPokemonSpriteToDisplay(selectedMon + 1);
        if (dexNum != 0xFFFF)
        {
            spriteId = CreatePokedexMonSprite(dexNum, SCROLLING_MON_X, 0x50);
            gSprites[spriteId].callback = SpriteCB_PokedexListMonSprite;
            gSprites[spriteId].data[5] = 64;
        }
        if (sPokedexView->listVOffset < LIST_SCROLL_STEP - 1)
            sPokedexView->listVOffset++;
        else
            sPokedexView->listVOffset = 0;
        break;
    }
}

// u16 ignored is passed but never used
static u16 TryDoPokedexScroll(u16 selectedMon, u16 ignored)
{
    u8 scrollTimer;
    u8 scrollMonIncrement;
    u8 i;
    u16 startingPos;
    u8 scrollDir = 0;

    if (JOY_HELD(DPAD_UP) && (selectedMon > 0))
    {
        scrollDir = 1;
        selectedMon = GetNextPosition(1, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        CreateScrollingPokemonSprite(1, selectedMon);
        CreateMonListEntry(1, selectedMon, ignored);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_SCROLL);
    }
    else if (JOY_HELD(DPAD_DOWN) && (selectedMon < sPokedexView->pokemonListCount - 1))
    {
        scrollDir = 2;
        selectedMon = GetNextPosition(0, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        CreateScrollingPokemonSprite(2, selectedMon);
        CreateMonListEntry(2, selectedMon, ignored);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_SCROLL);
    }
    else if (JOY_NEW(DPAD_LEFT) && (selectedMon > 0))
    {
        startingPos = selectedMon;

        for (i = 0; i < 7; i++)
            selectedMon = GetNextPosition(1, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        sPokedexView->pokeBallRotation += 16 * (selectedMon - startingPos);
        ClearMonSprites();
        CreateMonSpritesAtPos(selectedMon, 0xE);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_PAGE);
    }
    else if (JOY_NEW(DPAD_RIGHT) && (selectedMon < sPokedexView->pokemonListCount - 1))
    {
        startingPos = selectedMon;
        for (i = 0; i < 7; i++)
            selectedMon = GetNextPosition(0, selectedMon, 0, sPokedexView->pokemonListCount - 1);
        sPokedexView->pokeBallRotation += 16 * (selectedMon - startingPos);
        ClearMonSprites();
        CreateMonSpritesAtPos(selectedMon, 0xE);
        sPokedexView->justScrolled = TRUE; //HGSS_Ui
        PlaySE(SE_DEX_PAGE);
    }

    if (scrollDir == 0)
    {
        // Left/right input just snaps up/down, no scrolling
        sPokedexView->scrollSpeed = 0;
        return selectedMon;
    }

    scrollMonIncrement = sScrollMonIncrements[sPokedexView->scrollSpeed / 4];
    scrollTimer = sScrollTimers[sPokedexView->scrollSpeed / 4];
    sPokedexView->scrollTimer = scrollTimer;
    sPokedexView->maxScrollTimer = scrollTimer;
    sPokedexView->scrollMonIncrement = scrollMonIncrement;
    sPokedexView->scrollDirection = scrollDir;
    sPokedexView->pokeBallRotationStep = scrollMonIncrement / 2;
    UpdateDexListScroll(sPokedexView->scrollDirection, sPokedexView->scrollMonIncrement, sPokedexView->maxScrollTimer);
    if (sPokedexView->scrollSpeed < 12)
        sPokedexView->scrollSpeed++;
    return selectedMon;
}

static void UpdateSelectedMonSpriteId(void)
{
    u16 i;

    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
    {
        u16 spriteId = sPokedexView->monSpriteIds[i];

        if (gSprites[spriteId].x2 == 0 && gSprites[spriteId].y2 == 0 && spriteId != 0xFFFF)
            sPokedexView->selectedMonSpriteId = spriteId;
    }
}

static bool8 TryDoInfoScreenScroll(void)
{
    u16 nextPokemon;
    u16 selectedPokemon = sPokedexView->selectedPokemon;

    if (sPokedexView->sEvoScreenData.fromEvoPage)
        return FALSE;

    if (JOY_NEW(DPAD_UP) && selectedPokemon)
    {
        nextPokemon = selectedPokemon;
        while (nextPokemon != 0)
        {
            nextPokemon = GetNextPosition(1, nextPokemon, 0, sPokedexView->pokemonListCount - 1);

            if (sPokedexView->pokedexList[nextPokemon].seen)
            {
                selectedPokemon = nextPokemon;
                break;
            }
        }

        if (sPokedexView->selectedPokemon == selectedPokemon)
            return FALSE;
        else
        {
            sPokedexView->selectedPokemon = selectedPokemon;
            sPokedexView->pokeBallRotation -= 16;
            return TRUE;
        }
    }
    else if (JOY_NEW(DPAD_DOWN) && selectedPokemon < sPokedexView->pokemonListCount - 1)
    {
        nextPokemon = selectedPokemon;
        while (nextPokemon < sPokedexView->pokemonListCount - 1)
        {
            nextPokemon = GetNextPosition(0, nextPokemon, 0, sPokedexView->pokemonListCount - 1);

            if (sPokedexView->pokedexList[nextPokemon].seen)
            {
                selectedPokemon = nextPokemon;
                break;
            }
        }

        if (sPokedexView->selectedPokemon == selectedPokemon)
            return FALSE;
        else
        {
            sPokedexView->selectedPokemon = selectedPokemon;
            sPokedexView->pokeBallRotation += 16;
            return TRUE;
        }
    }
    return FALSE;
}

static u8 ClearMonSprites(void)
{
    u16 i;

    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
    {
        if (sPokedexView->monSpriteIds[i] != 0xFFFF)
        {
            FreeAndDestroyMonPicSprite(sPokedexView->monSpriteIds[i]);
            sPokedexView->monSpriteIds[i] = 0xFFFF;
        }
    }
    return FALSE;
}

static u16 GetPokemonSpriteToDisplay(u16 species)
{
    if (species >= NATIONAL_DEX_COUNT || sPokedexView->pokedexList[species].dexNum == 0xFFFF)
        return 0xFFFF;
    else if (sPokedexView->pokedexList[species].seen)
        return sPokedexView->pokedexList[species].dexNum;
    else
        return 0;
}

static u32 CreatePokedexMonSprite(u16 num, s16 x, s16 y)
{
    u8 i;

    for (i = 0; i < MAX_MONS_ON_SCREEN; i++)
    {
        if (sPokedexView->monSpriteIds[i] == 0xFFFF)
        {
            u8 spriteId = CreateMonSpriteFromNationalDexNumber(num, x, y, i);

            gSprites[spriteId].oam.affineMode = ST_OAM_AFFINE_NORMAL;
            gSprites[spriteId].oam.priority = 3;
            gSprites[spriteId].data[0] = 0;
            gSprites[spriteId].data[1] = i;
            gSprites[spriteId].data[2] = DexNumToSpecies(num);
            sPokedexView->monSpriteIds[i] = spriteId;
            return spriteId;
        }
    }
    return 0xFFFF;
}

#define sIsDownArrow data[1]
#define LIST_RIGHT_SIDE_TEXT_X 24
#define LIST_RIGHT_SIDE_TEXT_X_OFFSET 12
#define LIST_RIGHT_SIDE_TEXT_Y_OFFSET 9
static void CreateInterfaceSprites(u8 page)
{
    u8 spriteId;
    u16 digitNum;
    u8 color[3];
    bool32 drawNextDigit;

    // Scroll arrows
    spriteId = CreateSprite(&sScrollArrowSpriteTemplate, 184, 4, 0);
    gSprites[spriteId].sIsDownArrow = FALSE;
    spriteId = CreateSprite(&sScrollArrowSpriteTemplate, 184, 156, 0);
    gSprites[spriteId].sIsDownArrow = TRUE;
    gSprites[spriteId].vFlip = TRUE;

    CreateSprite(&sScrollBarSpriteTemplate, 230, 20, 0);

    // Select / Start buttons and their labels, under the mon box
    spriteId = CreateSprite(&sInterfaceTextSpriteTemplate, 76, 144, 0);
    StartSpriteAnim(&gSprites[spriteId], 2);
    spriteId = CreateSprite(&sInterfaceTextSpriteTemplate, 77, 155, 0);
    StartSpriteAnim(&gSprites[spriteId], 1);
    CreateSprite(&sInterfaceTextSpriteTemplate, 108, 144, 0);
    spriteId = CreateSprite(&sInterfaceTextSpriteTemplate, 112, 155, 0);
    StartSpriteAnim(&gSprites[spriteId], 3);

    if (!IsNationalPokedexEnabled() && page == PAGE_MAIN)
    {
        // Hoenn text
        CreateSprite(&sHoennNationalTextSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X, 40 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET - 6, 1);
        // Hoenn seen
        CreateSprite(&sSeenOwnTextSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X, 45 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET + 6, 1);
        // Hoenn own
        spriteId = CreateSprite(&sSeenOwnTextSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X, 55 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET + 7, 1);
        StartSpriteAnim(&gSprites[spriteId], 1);

        // Hoenn seen value - 100s
        drawNextDigit = FALSE;
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET, 45 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = sPokedexView->seenCount / 100;
        StartSpriteAnim(&gSprites[spriteId], digitNum);
        if (digitNum != 0)
            drawNextDigit = TRUE;
        else
            gSprites[spriteId].invisible = TRUE;

        // Hoenn seen value - 10s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 8, 45 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (sPokedexView->seenCount % 100) / 10;
        if (digitNum != 0 || drawNextDigit)
            StartSpriteAnim(&gSprites[spriteId], digitNum);
        else
            gSprites[spriteId].invisible = TRUE;

        // Hoenn seen value - 1s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 16, 45 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (sPokedexView->seenCount % 100) % 10;
        StartSpriteAnim(&gSprites[spriteId], digitNum);


        // Hoenn owned value - 100s
        drawNextDigit = FALSE;
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET, 55 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = sPokedexView->ownCount / 100;
        StartSpriteAnim(&gSprites[spriteId], digitNum);
        if (digitNum != 0)
            drawNextDigit = TRUE;
        else
            gSprites[spriteId].invisible = TRUE;

        // Hoenn owned value - 10s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 8, 55 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (sPokedexView->ownCount % 100) / 10;
        if (digitNum != 0 || drawNextDigit)
            StartSpriteAnim(&gSprites[spriteId], digitNum);
        else
            gSprites[spriteId].invisible = TRUE;

        // Hoenn owned value - 1s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 16, 55 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (sPokedexView->ownCount % 100) % 10;
        StartSpriteAnim(&gSprites[spriteId], digitNum);
    }
    else if (page == PAGE_MAIN)
    {
        u16 seenOwnedCount;

        // Hoenn text
        CreateSprite(&sHoennNationalTextSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X, 40 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET - 6, 1);
        // Hoenn seen
        CreateSprite(&sSeenOwnTextSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X, 45 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET + 6, 1);
        // Hoenn own
        spriteId = CreateSprite(&sSeenOwnTextSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X, 55 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET + 7, 1);
        StartSpriteAnim(&gSprites[spriteId], 1);

        // National text
        spriteId = CreateSprite(&sHoennNationalTextSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X, 73 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET - 6, 1);
        StartSpriteAnim(&gSprites[spriteId], 1);
        // National seen
        CreateSprite(&sSeenOwnTextSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X, 78 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET + 6, 1);
        // National own
        spriteId = CreateSprite(&sSeenOwnTextSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X, 88 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET + 6, 1);
        StartSpriteAnim(&gSprites[spriteId], 1);

        // Hoenn seen value - 100s
        seenOwnedCount = GetPokedexDisplayCount(TRUE, FLAG_GET_SEEN);
        drawNextDigit = FALSE;
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET, 45 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = seenOwnedCount / 100;
        StartSpriteAnim(&gSprites[spriteId], digitNum);
        if (digitNum != 0)
            drawNextDigit = TRUE;
        else
            gSprites[spriteId].invisible = TRUE;

        // Hoenn seen value - 10s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 8, 45 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (seenOwnedCount % 100) / 10;
        if (digitNum != 0 || drawNextDigit)
            StartSpriteAnim(&gSprites[spriteId], digitNum);
        else
            gSprites[spriteId].invisible = TRUE;

        // Hoenn seen value - 1s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 16, 45 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (seenOwnedCount % 100) % 10;
        StartSpriteAnim(&gSprites[spriteId], digitNum);

        seenOwnedCount = GetPokedexDisplayCount(TRUE, FLAG_GET_CAUGHT);
        // Hoenn owned value - 100s
        drawNextDigit = FALSE;
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET, 55 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = seenOwnedCount / 100;
        StartSpriteAnim(&gSprites[spriteId], digitNum);
        if (digitNum != 0)
            drawNextDigit = TRUE;
        else
            gSprites[spriteId].invisible = TRUE;

        // Hoenn owned value - 10s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 8, 55 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (seenOwnedCount % 100) / 10;
        if (digitNum != 0 || drawNextDigit)
            StartSpriteAnim(&gSprites[spriteId], digitNum);
        else
            gSprites[spriteId].invisible = TRUE;

        // Hoenn owned value - 1s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 16, 55 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (seenOwnedCount % 100) % 10;
        StartSpriteAnim(&gSprites[spriteId], digitNum);

        //****************************
        // National seen value - 100s
        drawNextDigit = FALSE;
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET, 78 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = sPokedexView->seenCount / 100;
        StartSpriteAnim(&gSprites[spriteId], digitNum);
        if (digitNum != 0)
            drawNextDigit = TRUE;
        else
            gSprites[spriteId].invisible = TRUE;

        // National seen value - 10s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 8, 78 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (sPokedexView->seenCount % 100) / 10;
        if (digitNum != 0 || drawNextDigit)
            StartSpriteAnim(&gSprites[spriteId], digitNum);
        else
            gSprites[spriteId].invisible = TRUE;

        // National seen value - 1s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 16, 78 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (sPokedexView->seenCount % 100) % 10;
        StartSpriteAnim(&gSprites[spriteId], digitNum);

        // National owned value - 100s
        drawNextDigit = FALSE;
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET, 88 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = sPokedexView->ownCount / 100;
        StartSpriteAnim(&gSprites[spriteId], digitNum);
        if (digitNum != 0)
            drawNextDigit = TRUE;
        else
            gSprites[spriteId].invisible = TRUE;

        // National owned value  - 10s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 8, 88 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (sPokedexView->ownCount % 100) / 10;
        if (digitNum != 0 || drawNextDigit)
            StartSpriteAnim(&gSprites[spriteId], digitNum);
        else
            gSprites[spriteId].invisible = TRUE;

        // National owned value - 1s
        spriteId = CreateSprite(&sNationalDexSeenOwnNumberSpriteTemplate, LIST_RIGHT_SIDE_TEXT_X + LIST_RIGHT_SIDE_TEXT_X_OFFSET + 16, 88 - LIST_RIGHT_SIDE_TEXT_Y_OFFSET, 1);
        digitNum = (sPokedexView->ownCount % 100) % 10;
        StartSpriteAnim(&gSprites[spriteId], digitNum);
    }

    if (page == PAGE_MAIN)
    {
        spriteId = CreateSprite(&sDexListStartMenuCursorSpriteTemplate, MAIN_MENU_CURSOR_X, 80 - (MAIN_MENU_Y - 96), 1);
        gSprites[spriteId].invisible = TRUE;
    }
    else // PAGE_SEARCH_RESULTS
    {
        spriteId = CreateSprite(&sDexListStartMenuCursorSpriteTemplate, 136, 80, 1);
        gSprites[spriteId].invisible = TRUE;
    }
}

static void SpriteCB_EndMoveMonForInfoScreen(struct Sprite *sprite)
{
    // Once mon is done moving there's nothing left to do
}

static void SpriteCB_SeenOwnInfo(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
        DestroySprite(sprite);
}

void SpriteCB_MoveMonForInfoScreen(struct Sprite *sprite)
{
    sprite->oam.priority = 0;
    sprite->oam.affineMode = ST_OAM_AFFINE_OFF;
    sprite->x2 = 0;
    sprite->y2 = 0;
    if (sprite->x != MON_PAGE_X || sprite->y != MON_PAGE_Y)
    {
        if (sprite->x > 48)
            sprite->x -= 4;
        if (sprite->x < 48)
            sprite->x = 48;

        if (sprite->y > 56)
            sprite->y -= 4;
        if (sprite->y < 56)
            sprite->y = 56;
    }
    else
    {
        sprite->callback = SpriteCB_EndMoveMonForInfoScreen;
    }
}

static void SpriteCB_PokedexListMonSprite(struct Sprite *sprite)
{
    u8 monId = sprite->data[1];

    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
    {
        FreeAndDestroyMonPicSprite(sPokedexView->monSpriteIds[monId]);
        sPokedexView->monSpriteIds[monId] = 0xFFFF;
    }
    else
    {
        u32 var;
        sprite->y2 = gSineTable[(u8)sprite->data[5]] * 76 / 256;
        var = SAFE_DIV(0x10000, gSineTable[sprite->data[5] + 64]);
        if (var > 0xFFFF)
            var = 0xFFFF;
        SetOamMatrix(sprite->data[1] + 1, 0x100, 0, 0, var);
        sprite->oam.matrixNum = monId + 1;

        if (sprite->data[5] > -64 && sprite->data[5] < 64)
        {
            sprite->invisible = FALSE;
            sprite->data[0] = 1;
        }
        else
        {
            sprite->invisible = TRUE;
        }

        if ((sprite->data[5] <= -64 || sprite->data[5] >= 64) && sprite->data[0] != 0)
        {
            FreeAndDestroyMonPicSprite(sPokedexView->monSpriteIds[monId]);
            sPokedexView->monSpriteIds[monId] = 0xFFFF;
        }
    }
}

static void SpriteCB_Scrollbar(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
        DestroySprite(sprite);
    else
        sprite->y2 = sPokedexView->selectedPokemon * 120 / (sPokedexView->pokemonListCount - 1);
}

static void SpriteCB_ScrollArrow(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
    {
        DestroySprite(sprite);
    }
    else
    {
        u8 r0;

        if (sprite->sIsDownArrow)
        {
            if (sPokedexView->selectedPokemon == sPokedexView->pokemonListCount - 1)
                sprite->invisible = TRUE;
            else
                sprite->invisible = FALSE;
            r0 = sprite->data[2];
        }
        else
        {
            if (sPokedexView->selectedPokemon == 0)
                sprite->invisible = TRUE;
            else
                sprite->invisible = FALSE;
            r0 = sprite->data[2] - 128;
        }
        sprite->y2 = gSineTable[r0] / 64;
        sprite->data[2] = sprite->data[2] + 8;
        if (sPokedexView->menuIsOpen == FALSE && sPokedexView->menuY == 0 && sprite->invisible == FALSE)
            sprite->invisible = FALSE;
        else
            sprite->invisible = TRUE;
    }
}

static void SpriteCB_DexListInterfaceText(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
        DestroySprite(sprite);
}

static void SpriteCB_RotatingPokeBall(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
    {
        DestroySprite(sprite);
    }
    else
    {
        u8 val;
        s16 r3;
        s16 r0;

        val = sPokedexView->pokeBallRotation + sprite->data[1];
        r3 = gSineTable[val];
        r0 = gSineTable[val + 64];
        SetOamMatrix(sprite->data[0], r0, r3, -r3, r0);

        val = sPokedexView->pokeBallRotation + (sprite->data[1] + 64);
        r3 = gSineTable[val];
        r0 = gSineTable[val + 64];
        sprite->x2 = r0 * 40 / 256;
        sprite->y2 = r3 * 40 / 256;
    }
}

static void SpriteCB_DexListStartMenuCursor(struct Sprite *sprite)
{
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
    {
        DestroySprite(sprite);
    }
    else
    {
        u16 r1 = sPokedexView->currentPage == PAGE_MAIN ? MAIN_MENU_Y : 96;

        if (sPokedexView->menuIsOpen && sPokedexView->menuY == r1)
        {
            sprite->invisible = FALSE;
            sprite->y2 = sPokedexView->menuCursorPos * 16;
            sprite->x2 = gSineTable[(u8)sprite->data[2]] / 64;
            sprite->data[2] += 8;
        }
        else
        {
            sprite->invisible = TRUE;
        }
    }
}



//************************************
//*                                  *
//*        STATS BARS                *
//*                                  *
//************************************

//Stat bars on main screen, code by DizzyEgg, idea by Jaizu
#define PIXEL_COORDS_TO_OFFSET(x, y)(			\
/*Add tiles by X*/								\
((y / 8) * 32 * 8)								\
/*Add tiles by X*/								\
+ ((x / 8) * 32)								\
/*Add pixels by Y*/								\
+ ((((y) - ((y / 8) * 8))) * 4)				    \
/*Add pixels by X*/								\
+ ((((x) - ((x / 8) * 8)) / 2)))

static inline void WritePixel(u8 *dst, u32 x, u32 y, u32 value)
{
    if (x & 1)
    {
        dst[PIXEL_COORDS_TO_OFFSET(x, y)] &= ~0xF0;
        dst[PIXEL_COORDS_TO_OFFSET(x, y)] |= (value << 4);
    }
    else
    {
        dst[PIXEL_COORDS_TO_OFFSET(x, y)] &= ~0xF;
        dst[PIXEL_COORDS_TO_OFFSET(x, y)] |= (value);
    }
}
#define STAT_BAR_X_OFFSET 10
static void CreateStatBar(u8 *dst, u32 y, u32 width)
{
    u32 i, color;

    switch (width)
    {
    case 0 ... 5:
        color = COLOR_WORST;
        break;
    case 6 ... 15:
        color = COLOR_BAD;
        break;
    case 16 ... 25:
        color = COLOR_AVERAGE;
        break;
    case 26 ... 31:
        color = COLOR_GOOD;
        break;
    case 32 ... 37:
        color = COLOR_VERY_GOOD;
        break;
    default:
        color = COLOR_BEST;
        break;
    }

    // white pixes left side
    WritePixel(dst, STAT_BAR_X_OFFSET, y + 0, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET, y + 1, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET, y + 2, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET, y + 3, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET, y + 4, COLOR_ID_BAR_WHITE);

    // white pixels right side
    WritePixel(dst, STAT_BAR_X_OFFSET + width - 1, y + 0, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET + width - 1, y + 1, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET + width - 1, y + 2, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET + width - 1, y + 3, COLOR_ID_BAR_WHITE);
    WritePixel(dst, STAT_BAR_X_OFFSET + width - 1, y + 4, COLOR_ID_BAR_WHITE);

    // Fill
    for (i = 1; i < width - 1; i++)
    {
        WritePixel(dst, STAT_BAR_X_OFFSET + i, y + 0, COLOR_ID_BAR_WHITE);
        WritePixel(dst, STAT_BAR_X_OFFSET + i, y + 1, COLOR_ID_FILL_SHADOW + color * 2);
        WritePixel(dst, STAT_BAR_X_OFFSET + i, y + 2, COLOR_ID_FILL + color * 2);
        WritePixel(dst, STAT_BAR_X_OFFSET + i, y + 3, COLOR_ID_FILL + color * 2);
        WritePixel(dst, STAT_BAR_X_OFFSET + i, y + 4, COLOR_ID_BAR_WHITE);
    }
}
static const u8 sBaseStatOffsets[] =
{
    offsetof(struct SpeciesInfo, baseHP),
    offsetof(struct SpeciesInfo, baseAttack),
    offsetof(struct SpeciesInfo, baseDefense),
    offsetof(struct SpeciesInfo, baseSpAttack),
    offsetof(struct SpeciesInfo, baseSpDefense),
    offsetof(struct SpeciesInfo, baseSpeed),
};
static void TryDestroyStatBars(void)
{
    if (sPokedexView->statBarsSpriteId != 0xFF)
    {
        FreeSpriteTilesByTag(TAG_STAT_BAR);
        //FreeSpriteOamMatrix(&gSprites[sPokedexView->statBarsSpriteId]);
        DestroySprite(&gSprites[sPokedexView->statBarsSpriteId]);
        sPokedexView->statBarsSpriteId = 0xFF;
    }
}
static void TryDestroyStatBarsBg(void)
{
    if (sPokedexView->statBarsBgSpriteId != 0xFF)
    {
        FreeSpriteTilesByTag(TAG_STAT_BAR_BG);
        //FreeSpriteOamMatrix(&gSprites[sPokedexView->statBarsBgSpriteId]);
        DestroySprite(&gSprites[sPokedexView->statBarsBgSpriteId]);
        sPokedexView->statBarsBgSpriteId = 0xFF;
    }
}
static void CreateStatBars(struct PokedexListItem *dexMon)
{
    u8 offset_x = 4; //Moves the complete stat box left/right
    u8 offset_y = 20; //Moves the complete stat box up/down
    TryDestroyStatBars();

    sPokedexView->justScrolled = FALSE;


    if (dexMon->owned) // Show filed bars
    {
        u8 i;
        u32 width, statValue;
        u8 *gfx = Alloc(64 * 64);
        static const u8 sBarsYOffset[] = {3, 13, 23, 33, 43, 53};
        struct SpriteSheet sheet = {gfx, 64 * 64, TAG_STAT_BAR};
        u32 species = DexNumToSpecies(dexMon->dexNum);

        memcpy(gfx, sStatBarsGfx, sizeof(sStatBarsGfx));
        for (i = 0; i < NUM_STATS; i++)
        {
            statValue = *((u8*)(&gSpeciesInfo[species]) + sBaseStatOffsets[i]);
            if (statValue <= 100)
            {
                width = statValue / 3;
                if (width >= 33)
                    width -= 1;
            }
            else
                width = (100 / 3) + ((statValue - 100) / 14);

            if (width > 39) // Max pixels
                width = 39;
            if (width < 3)
                width = 3;

            CreateStatBar(gfx, sBarsYOffset[i], width);
        }

        LoadSpriteSheet(&sheet);
        Free(gfx);
    }
    else if (dexMon->seen) // Just HP/ATK/DEF
    {
        static const struct SpriteSheet sheet = {sStatBarsGfx, 64 * 64, TAG_STAT_BAR};

        LoadSpriteSheet(&sheet);
    }
    else // neither seen nor owned
    {
        return;
    }
    sPokedexView->statBarsSpriteId = CreateSprite(&sStatBarSpriteTemplate, 36+offset_x, 107+offset_y, 10);
}
static void CreateStatBarsBg(void) //HGSS_Ui stat bars background text
{
    static const struct SpriteSheet sheetStatBarsBg = {sStatBarsGfx, 64 * 64, TAG_STAT_BAR_BG};
    u8 offset_x = 4; //Moves the complete stat box left/right
    u8 offset_y = 20; //Moves the complete stat box up/down

    TryDestroyStatBarsBg();

    LoadSpriteSheet(&sheetStatBarsBg);
    sPokedexView->statBarsBgSpriteId = CreateSprite(&sStatBarBgSpriteTemplate, 36+offset_x, 107+offset_y, 0);
}
// Hack to destroy sprites when a pokemon data is being loaded in
static bool32 IsMonInfoBeingLoaded(void)
{
    return (gSprites[sPokedexView->selectedMonSpriteId].callback == SpriteCB_MoveMonForInfoScreen);
}
static void SpriteCB_StatBars(struct Sprite *sprite)
{
    if (IsMonInfoBeingLoaded())
        sprite->invisible = TRUE;
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
    {
        FreeSpriteTilesByTag(TAG_STAT_BAR);
        FreeSpriteOamMatrix(&gSprites[sPokedexView->statBarsSpriteId]);
        DestroySprite(&gSprites[sPokedexView->statBarsSpriteId]);
        sPokedexView->statBarsSpriteId = 0xFF;
    }
}
static void SpriteCB_StatBarsBg(struct Sprite *sprite)
{
    if (IsMonInfoBeingLoaded())
        sprite->invisible = TRUE;
    if (sPokedexView->currentPage != PAGE_MAIN && sPokedexView->currentPage != PAGE_SEARCH_RESULTS)
    {
        FreeSpriteTilesByTag(TAG_STAT_BAR_BG);
        FreeSpriteOamMatrix(&gSprites[sPokedexView->statBarsBgSpriteId]);
        DestroySprite(&gSprites[sPokedexView->statBarsBgSpriteId]);
        sPokedexView->statBarsBgSpriteId = 0xFF;
    }
}



//************************************
//*                                  *
//*        Info screen               *
//*                                  *
//************************************
#define tScrolling       data[0]
#define tMonSpriteDone   data[1]
#define tBgLoaded        data[2]
#define tSkipCry         data[3]
#define tMonSpriteId     data[4]
#define tTrainerSpriteId data[5]

static u8 LoadInfoScreen(struct PokedexListItem *item, u8 monSpriteId)
{
    u8 taskId;

    sPokedexListItem = item;
    taskId = CreateTask(Task_LoadInfoScreen, 0);
    gTasks[taskId].tScrolling = FALSE;
    gTasks[taskId].tMonSpriteDone = TRUE; // Already has sprite from list view
    gTasks[taskId].tBgLoaded = FALSE;
    gTasks[taskId].tSkipCry = FALSE;
    gTasks[taskId].tMonSpriteId = monSpriteId;
    gTasks[taskId].tTrainerSpriteId = SPRITE_NONE;
    ResetBgsAndClearDma3BusyFlags(0);
    InitBgsFromTemplates(0, sInfoScreen_BgTemplate, ARRAY_COUNT(sInfoScreen_BgTemplate));
    SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
    SetBgTilemapBuffer(0, AllocZeroed(BG_SCREEN_SIZE));
    InitWindows(sInfoScreen_WindowTemplates);
    DeactivateAllTextPrinters();

    return taskId;
}

// Opening the info screen from the local area screen: there is no list sprite to carry over,
// so the info screen creates (and fades in) the Pokémon sprite itself.
static u8 LoadInfoScreenWithoutMonSprite(struct PokedexListItem *item)
{
    u8 taskId = LoadInfoScreen(item, 0);

    gTasks[taskId].tMonSpriteDone = FALSE;
    return taskId;
}

static bool8 IsInfoScreenScrolling(u8 taskId)
{
    if (!gTasks[taskId].tScrolling && gTasks[taskId].func == Task_HandleInfoScreenInput)
        return FALSE;
    else
        return TRUE;
}

static u8 StartInfoScreenScroll(struct PokedexListItem *item, u8 taskId)
{
    sPokedexListItem = item;
    gTasks[taskId].tScrolling = TRUE;
    gTasks[taskId].tMonSpriteDone = FALSE;
    gTasks[taskId].tBgLoaded = FALSE;
    gTasks[taskId].tSkipCry = FALSE;
    return taskId;
}

static void Task_LoadInfoScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            u16 r2;

            sPokedexView->currentPage = PAGE_INFO;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            r2 = 0;
            if (gTasks[taskId].tMonSpriteDone)
                r2 += DISPCNT_OBJ_ON;
            if (gTasks[taskId].tBgLoaded)
                r2 |= DISPCNT_BG1_ON;
            ResetOtherVideoRegisters(r2);
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(INFO_SCREEN);
        // DecompressAndLoadBgGfxUsingHeap(3, gPokedexMenu_Gfx, 0x2000, 0, 0);
        // CopyToBgTilemapBuffer(3, gPokedexInfoScreen_Tilemap, 0, 0);
        FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
        PutWindowTilemap(WIN_INFO);
        PutWindowTilemap(WIN_FOOTPRINT);
        DrawFootprint(WIN_FOOTPRINT, sPokedexListItem->dexNum);
        CopyWindowToVram(WIN_FOOTPRINT, COPYWIN_GFX);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain(0xD);
        // HighlightScreenSelectBarItem(sPokedexView->selectedScreen, 0xD);
        LoadPokedexBgPalette(sPokedexView->isSearchResults);
        gMain.state++;
        break;
    case 3:
        sPokedexView->typeIconSpriteIds[0] = 0xFF;
        sPokedexView->typeIconSpriteIds[1] = 0xFF;
        CreateTypeIconSprites();
        gMain.state++;
        break;
    case 4:
        PrintMonInfo(sPokedexListItem->dexNum, sPokedexView->dexMode == DEX_MODE_HOENN ? FALSE : TRUE, sPokedexListItem->owned, 0);
        if (!sPokedexListItem->owned)
            LoadPalette(&gPlttBufferUnfaded[BG_PLTT_ID(0) + 1], BG_PLTT_ID(3) + 1, PLTT_SIZEOF(16 - 1));
        CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 5:
        if (!gTasks[taskId].tMonSpriteDone)
        {
            gTasks[taskId].tMonSpriteId = (u16)CreateMonSpriteFromNationalDexNumber(sPokedexListItem->dexNum, MON_PAGE_X, MON_PAGE_Y, 0);
            gSprites[gTasks[taskId].tMonSpriteId].oam.priority = 0;
        }
        gMain.state++;
        break;
    case 6:
        {
            u32 preservedPalettes = 0;

            if (gTasks[taskId].tBgLoaded)
                preservedPalettes = 0x14; // each bit represents a palette index
            if (gTasks[taskId].tMonSpriteDone)
                preservedPalettes |= (1 << (gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum + 16));
            BeginNormalPaletteFade(~preservedPalettes, 0, 16, 0, RGB_BLACK);
            SetVBlankCallback(gPokedexVBlankCB);
            gMain.state++;
        }
        break;
    case 7:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 8:
        if (!gPaletteFade.active)
        {
            gMain.state++;
        }
        break;
    case 9:
        // if (!IsCryPlayingOrClearCrySongs())
        gMain.state++;
        break;
    case 10:
        gTasks[taskId].tScrolling = FALSE;
        gTasks[taskId].tMonSpriteDone = FALSE; // Reload next time screen comes up
        gTasks[taskId].tBgLoaded = TRUE;
        gTasks[taskId].tSkipCry = TRUE;
        gTasks[taskId].func = Task_HandleInfoScreenInput;
        gMain.state = 0;
        break;
    }
}

static void FreeInfoScreenWindowAndBgBuffers(void)
{
    void *tilemapBuffer;

    FreeAllWindowBuffers();
    tilemapBuffer = GetBgTilemapBuffer(0);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(1);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(2);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(3);
    if (tilemapBuffer)
        Free(tilemapBuffer);
}

static void Task_HandleInfoScreenInput(u8 taskId)
{
    if (gTasks[taskId].tScrolling)
    {
        // Scroll up/down
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_LoadInfoScreenWaitForFade;
        PlaySE(SE_DEX_SCROLL);
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_ExitInfoScreen;
        PlaySE(SE_PC_OFF);
        return;
    }

    if ((JOY_NEW(DPAD_RIGHT) || (JOY_NEW(R_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        sPokedexView->selectedScreen = AREA_SCREEN;
        BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 1;
        gTasks[taskId].func = Task_SwitchScreensFromInfoScreen;
        PlaySE(SE_PIN);
    }

}

static void Task_SwitchScreensFromInfoScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        switch (sPokedexView->screenSwitchState)
        {
        case 1:
        default:
            gTasks[taskId].func = Task_LoadAreaScreen;
            break;
        case 2:
            gTasks[taskId].func = Task_LoadCryScreen;
            break;
        case 3:
            gTasks[taskId].func = Task_LoadSizeScreen;
            break;
        }
    }
}

static void Task_LoadInfoScreenWaitForFade(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        gTasks[taskId].func = Task_LoadInfoScreen;
    }
}

static void Task_ExitInfoScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}

#undef tMonSpriteId

//************************************
//*                                  *
//*        Area screen               *
//*                                  *
//************************************
static void Task_LoadAreaScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            sPokedexView->currentPage = PAGE_AREA;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            ResetOtherVideoRegisters(DISPCNT_BG1_ON);
            sPokedexView->selectedScreen = AREA_SCREEN;
            gMain.state = 1;
        }
        break;
    case 1:
        LoadScreenSelectBarSubmenu(0xD); //HGSS_Ui
        LoadPokedexBgPalette(sPokedexView->isSearchResults);
        SetGpuReg(REG_OFFSET_BG1CNT, BGCNT_PRIORITY(0) | BGCNT_CHARBASE(0) | BGCNT_SCREENBASE(13) | BGCNT_16COLOR | BGCNT_TXT256x256);
        gMain.state++;
        break;
    case 2:
        ShowPokedexAreaScreen(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), &sPokedexView->screenSwitchState);
        SetVBlankCallback(gPokedexVBlankCB);
        sPokedexView->screenSwitchState = 0;
        gMain.state = 0;
        gTasks[taskId].func = Task_WaitForAreaScreenInput;
        break;
    }
}

static void Task_WaitForAreaScreenInput(u8 taskId)
{
// See Task_HandlePokedexAreaScreenInput() in pokedex_area_screen.c
    if (sPokedexView->screenSwitchState != 0)
        gTasks[taskId].func = Task_SwitchScreensFromAreaScreen;
}

static void Task_SwitchScreensFromAreaScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        switch (sPokedexView->screenSwitchState)
        {
        case 1:
        default:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        case 2:
            if (!sPokedexListItem->owned)
                gTasks[taskId].func = Task_LoadInfoScreen;
            else
                gTasks[taskId].func = Task_LoadStatsScreen;
            break;
        }
    }
}



//************************************
//*                                  *
//*        Select bar                *
//*                                  *
//************************************
static void LoadScreenSelectBarMain(u16 unused)
{
    CopyToBgTilemapBuffer(1, gPokedexPlusHGSS_ScreenSelectBarSubmenu_Tilemap_Clear, 0, 0);
    CopyBgTilemapBufferToVram(1);
}

static void LoadScreenSelectBarSubmenu(u16 unused)
{
    CopyToBgTilemapBuffer(1, gPokedexPlusHGSS_ScreenSelectBarSubmenu_Tilemap, 0, 0);
    CopyBgTilemapBufferToVram(1);
}

static void HighlightScreenSelectBarItem(u8 selectedScreen, u16 unused) //HGSS_Ui
{
    u8 i;
    u8 j;
    u16 *ptr = GetBgTilemapBuffer(1);

    for (i = 0; i < SCREEN_COUNT; i++)
    {
        u8 row = (i * 7) + 1;
        u16 newPalette;

        do
        {
            newPalette = 0x4000;
            if (i == selectedScreen)
                newPalette = 0x2000;
        } while (0);

        for (j = 0; j < 7; j++)
        {
            ptr[row + j] = (ptr[row + j] % 0x1000) | newPalette;
            ptr[row + j + 0x20] = (ptr[row + j + 0x20] % 0x1000) | newPalette;
        }
    }
    CopyBgTilemapBufferToVram(1);
}

static void HighlightSubmenuScreenSelectBarItem(u8 a, u16 b)
{
    u8 i;
    u8 j;
    u16 *ptr = GetBgTilemapBuffer(1);

    for (i = 0; i < 4; i++)
    {
        u8 row = i * 7 + 1;
        u32 newPalette;

        do
        {
            if (i == a || i == 3)
                newPalette = 0x2000;
            else
                newPalette = 0x4000;
        } while (0);

        for (j = 0; j < 7; j++)
        {
            ptr[row + j] = (ptr[row + j] % 0x1000) | newPalette;
            ptr[row + j + 0x20] = (ptr[row + j + 0x20] % 0x1000) | newPalette;
        }
    }
    CopyBgTilemapBufferToVram(1);
}



//************************************
//*                                  *
//*        CAUGHT MON                *
//*                                  *
//************************************
#define tState         data[0]
#define tSpecies       data[1]
#define tPalTimer      data[2]
#define tMonSpriteId   data[3]
#define tOtIdLo        data[12]
#define tOtIdHi        data[13]
#define tPersonalityLo data[14]
#define tPersonalityHi data[15]

// Called from DisplayCaughtMonDexPage (pokedex.c) when POKEDEX_PLUS_HGSS is enabled
u8 DisplayCaughtMonDexPageHGSS(u16 species, u32 otId, u32 personality)
{
    u8 taskId = CreateTask(Task_DisplayCaughtMonDexPage, 0);

    gTasks[taskId].tState = 0;
    gTasks[taskId].tSpecies = species;
    gTasks[taskId].tOtIdLo = otId;
    gTasks[taskId].tOtIdHi = otId >> 16;
    gTasks[taskId].tPersonalityLo = personality;
    gTasks[taskId].tPersonalityHi = personality >> 16;
    return taskId;
}

static void Task_DisplayCaughtMonDexPage(u8 taskId)
{
    u8 spriteId;
    u16 species = gTasks[taskId].tSpecies;
    u16 dexNum = SpeciesToNationalPokedexNum(species);

    switch (gTasks[taskId].tState)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            ResetOtherVideoRegisters(DISPCNT_BG0_ON);
            ResetBgsAndClearDma3BusyFlags(0);
            InitBgsFromTemplates(0, sNewEntryInfoScreen_BgTemplate, ARRAY_COUNT(sNewEntryInfoScreen_BgTemplate));
            SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
            SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
            InitWindows(sNewEntryInfoScreen_WindowTemplates);
            DeactivateAllTextPrinters();
            gTasks[taskId].tState = 1;
        }
        break;
    case 1:
        sPokedexView = AllocZeroed(sizeof(struct PokedexView)); //for type icons
        ResetPokedexView(sPokedexView);
        BuildNatDexToSpeciesTable();

        #ifdef POKEMON_EXPANSION
        if (gFormSpeciesIdTables[species] != NULL)
            sPokedexView->formSpecies = species;
        else
            sPokedexView->formSpecies = 0;
        #endif

        LoadTilesetTilemapHGSS(INFO_SCREEN);
        FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
        PutWindowTilemap(WIN_INFO);
        PutWindowTilemap(WIN_FOOTPRINT);
        DrawFootprint(WIN_FOOTPRINT, dexNum);
        CopyWindowToVram(WIN_FOOTPRINT, COPYWIN_GFX);
        ResetPaletteFade();
        LoadPokedexBgPalette(FALSE);
        gTasks[taskId].tState++;
        break;
    case 2:
        sPokedexView->typeIconSpriteIds[0] = 0xFF;
        sPokedexView->typeIconSpriteIds[1] = 0xFF;
        CreateTypeIconSprites();
        gTasks[taskId].tState++;
        break;
    case 3:
        PrintMonInfo(dexNum, IsNationalPokedexEnabled(), 1, 1);
        CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gTasks[taskId].tState++;
        break;
    case 4:
        #ifndef POKEMON_EXPANSION
        spriteId = CreateMonSpriteFromNationalDexNumber(dexNum, MON_PAGE_X, MON_PAGE_Y, 0);
        #else
        spriteId = CreateMonPicSprite(species, 0, ((u16)gTasks[taskId].tPersonalityHi << 16) | (u16)gTasks[taskId].tPersonalityLo, TRUE, MON_PAGE_X, MON_PAGE_Y, 0, TAG_NONE);
        #endif
        gSprites[spriteId].oam.priority = 0;
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gTasks[taskId].tMonSpriteId = spriteId;
        gTasks[taskId].tState++;
        break;
    case 5:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        ShowBg(2);
        ShowBg(3);
        gTasks[taskId].tState++;
        break;
    case 6:
        if (!gPaletteFade.active)
        {
            PlayCry_Normal(NationalPokedexNumToSpeciesHGSS(dexNum), 0);
            gTasks[taskId].tPalTimer = 0;
            gTasks[taskId].func = Task_HandleCaughtMonPageInput;
        }
        break;
    }
}

static void Task_HandleCaughtMonPageInput(u8 taskId)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_BG, 0, 0, 16, RGB_BLACK);
        SetSpriteInvisibility(0, TRUE);
        SetSpriteInvisibility(1, TRUE);
        gSprites[gTasks[taskId].tMonSpriteId].callback = SpriteCB_SlideCaughtMonToCenter;
        gTasks[taskId].func = Task_ExitCaughtMonPage;
    }
    // Flicker caught screen color
    else if (++gTasks[taskId].tPalTimer & 16)
    {
        if (!HGSS_DARK_MODE)
            LoadPalette(gPokedexPlusHGSS_Default_Pal + 1, BG_PLTT_ID(3) + 1, PLTT_SIZEOF(7));
        else
            LoadPalette(gPokedexPlusHGSS_Default_dark_Pal + 1, BG_PLTT_ID(3) + 1, PLTT_SIZEOF(7));
    }
    else
    {
        if (!HGSS_DARK_MODE)
            LoadPalette(gPokedexPlusHGSS_Default_Pal + 49, BG_PLTT_ID(3) + 1, PLTT_SIZEOF(7));
        else
            LoadPalette(gPokedexPlusHGSS_Default_dark_Pal + 49, BG_PLTT_ID(3) + 1, PLTT_SIZEOF(7));
    }
}

static void Task_ExitCaughtMonPage(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        u16 species;
        u32 otId;
        u32 personality;
        u8 paletteNum;
        const u32 *lzPaletteData;
        void *buffer;

        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        FreeAllWindowBuffers();
        buffer = GetBgTilemapBuffer(2);
        if (buffer)
            Free(buffer);
        buffer = GetBgTilemapBuffer(3);
        if (buffer)
            Free(buffer);

        species = gTasks[taskId].tSpecies;
        otId = ((u16)gTasks[taskId].tOtIdHi << 16) | (u16)gTasks[taskId].tOtIdLo;
        personality = ((u16)gTasks[taskId].tPersonalityHi << 16) | (u16)gTasks[taskId].tPersonalityLo;
        paletteNum = gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum;
        lzPaletteData = GetMonSpritePalFromSpeciesAndPersonality(species, otId, personality);
        LoadCompressedPalette(lzPaletteData, OBJ_PLTT_ID(paletteNum), PLTT_SIZE_4BPP);
        ApplyIndividualColorVariation(&gPlttBufferUnfaded[OBJ_PLTT_ID(paletteNum)], personality, species, IsShinyOtIdPersonality(otId, personality));
        ApplyIndividualColorVariation(&gPlttBufferFaded[OBJ_PLTT_ID(paletteNum)], personality, species, IsShinyOtIdPersonality(otId, personality));

        // Clean up the type icons and the view struct allocated by Task_DisplayCaughtMonDexPage
        if (sPokedexView != NULL)
        {
            u8 i;

            for (i = 0; i < ARRAY_COUNT(sPokedexView->typeIconSpriteIds); i++)
            {
                if (sPokedexView->typeIconSpriteIds[i] != 0xFF)
                    DestroySprite(&gSprites[sPokedexView->typeIconSpriteIds[i]]);
            }
            FreeSpriteTilesByTag(gSpriteSheet_MoveTypes.tag);
            FREE_AND_SET_NULL(sPokedexView);
        }
        DestroyTask(taskId);
    }
}

static void SpriteCB_SlideCaughtMonToCenter(struct Sprite *sprite)
{
    if (sprite->x < DISPLAY_WIDTH / 2)
        sprite->x += 2;
    if (sprite->x > DISPLAY_WIDTH / 2)
        sprite->x -= 2;

    if (sprite->y < DISPLAY_HEIGHT / 2)
        sprite->y += 1;
    if (sprite->y > DISPLAY_HEIGHT / 2)
        sprite->y -= 1;
}

#undef tState
#undef tSpecies
#undef tPalTimer
#undef tMonSpriteId
#undef tOtIdLo
#undef tOtIdHi
#undef tPersonalityLo
#undef tPersonalityHi







//************************************
//*                                  *
//*        Print data                *
//*                                  *
//************************************
static void PrintInfoScreenText(const u8 *str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;

    AddTextPrinterParameterized4(0, 1, left, top, 0, 0, color, -1, str);
}
static void PrintInfoScreenTextSmall(const u8* str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;

    AddTextPrinterParameterized4(0, 0, left, top, 0, 0, color, 0, str);
}
static void PrintInfoScreenTextSmallWhite(const u8* str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_COLOR_WHITE;
    color[2] = TEXT_DYNAMIC_COLOR_6;

    AddTextPrinterParameterized4(0, 0, left, top, 0, 0, color, 0, str);
}
//Stats screen
static void PrintStatsScreenTextSmall(u8 windowId, const u8* str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;

    AddTextPrinterParameterized4(windowId, 0, left, top, 0, 0, color, 0, str);
}
static void PrintStatsScreenTextSmallWhite(u8 windowId, const u8* str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_COLOR_WHITE;
    color[2] = TEXT_DYNAMIC_COLOR_6;

    AddTextPrinterParameterized4(windowId, 0, left, top, 0, 0, color, 0, str);
}

//Type Icon
static void SetSpriteInvisibility(u8 spriteArrayId, bool8 invisible)
{
    gSprites[sPokedexView->typeIconSpriteIds[spriteArrayId]].invisible = invisible;
}
// different from pokemon_summary_screen
#define TYPE_ICON_PAL_NUM_0     13
#define TYPE_ICON_PAL_NUM_1     14
#define TYPE_ICON_PAL_NUM_2     15
static const u8 sMoveTypeToOamPaletteNum[NUMBER_OF_MON_TYPES + CONTEST_CATEGORIES_COUNT] =
{
    [TYPE_NORMAL] = TYPE_ICON_PAL_NUM_0,
    [TYPE_FIGHTING] = TYPE_ICON_PAL_NUM_0,
    [TYPE_FLYING] = TYPE_ICON_PAL_NUM_1,
    [TYPE_POISON] = TYPE_ICON_PAL_NUM_1,
    [TYPE_GROUND] = TYPE_ICON_PAL_NUM_0,
    [TYPE_ROCK] = TYPE_ICON_PAL_NUM_0,
    [TYPE_BUG] = TYPE_ICON_PAL_NUM_2,
    [TYPE_GHOST] = TYPE_ICON_PAL_NUM_1,
    [TYPE_STEEL] = TYPE_ICON_PAL_NUM_0,
    [TYPE_MYSTERY] = TYPE_ICON_PAL_NUM_2,
    [TYPE_FIRE] = TYPE_ICON_PAL_NUM_0,
    [TYPE_WATER] = TYPE_ICON_PAL_NUM_1,
    [TYPE_GRASS] = TYPE_ICON_PAL_NUM_2,
    [TYPE_ELECTRIC] = TYPE_ICON_PAL_NUM_0,
    [TYPE_PSYCHIC] = TYPE_ICON_PAL_NUM_1,
    [TYPE_ICE] = TYPE_ICON_PAL_NUM_1,
    [TYPE_DRAGON] = TYPE_ICON_PAL_NUM_2,
    [TYPE_DARK] = TYPE_ICON_PAL_NUM_0,
    [NUMBER_OF_MON_TYPES + CONTEST_CATEGORY_COOL] = TYPE_ICON_PAL_NUM_0,
    [NUMBER_OF_MON_TYPES + CONTEST_CATEGORY_BEAUTY] = TYPE_ICON_PAL_NUM_1,
    [NUMBER_OF_MON_TYPES + CONTEST_CATEGORY_CUTE] = TYPE_ICON_PAL_NUM_1,
    [NUMBER_OF_MON_TYPES + CONTEST_CATEGORY_SMART] = TYPE_ICON_PAL_NUM_2,
    [NUMBER_OF_MON_TYPES + CONTEST_CATEGORY_TOUGH] = TYPE_ICON_PAL_NUM_0,
    #ifdef TYPE_FAIRY
    [TYPE_FAIRY] = TYPE_ICON_PAL_NUM_1, //based on battle_engine
    #endif
};
static void SetTypeIconPosAndPal(u8 typeId, u8 x, u8 y, u8 spriteArrayId)
{
    struct Sprite *sprite;

    sprite = &gSprites[sPokedexView->typeIconSpriteIds[spriteArrayId]];
    StartSpriteAnim(sprite, typeId);
    sprite->oam.paletteNum = sMoveTypeToOamPaletteNum[typeId];
    sprite->x = x + 16;
    sprite->y = y + 8;
    SetSpriteInvisibility(spriteArrayId, FALSE);
}
static void PrintCurrentSpeciesTypeInfo(u8 newEntry, u16 species)
{
    u32 i;
    u16 dexNum = SpeciesToNationalPokedexNum(species);
    u8 type1, type2;

    if (!newEntry)
    {
        species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
        dexNum = SpeciesToNationalPokedexNum(species);
    }
    //type icon(s)
    #ifdef TX_RANDOMIZER_AND_CHALLENGES
        type1 = GetTypeBySpecies(species, 1);
        type2 = GetTypeBySpecies(species, 2);
    #else
        type1 = gSpeciesInfo[species].types[0];
        type2 = gSpeciesInfo[species].types[1];
    #endif
    if (species == SPECIES_NONE)
        type1 = type2 = TYPE_MYSTERY;

    if (type1 == type2)
    {
        SetTypeIconPosAndPal(type1, 188, 57, 0);
        SetSpriteInvisibility(1, TRUE);
    }
    else
    {
        SetTypeIconPosAndPal(type1, 188, 57, 0);
        SetTypeIconPosAndPal(type2, 188, 73, 1);
    }

}
static void CreateTypeIconSprites(void)
{
    u8 i;

    LoadCompressedSpriteSheet(&gSpriteSheet_MoveTypes);
    LoadCompressedPalette(gMoveTypes_Pal, 0x1D0, 0x60);
    for (i = 0; i < 2; i++)
    {
        if (sPokedexView->typeIconSpriteIds[i] == 0xFF)
            sPokedexView->typeIconSpriteIds[i] = CreateSprite(&gSpriteTemplate_MoveTypes, 10, 10, 2);

        SetSpriteInvisibility(i, TRUE);
    }
}

// u32 value is re-used, but passed as a bool that's TRUE if national dex is enabled
static void PrintMonInfo(u32 num, u32 value, u32 owned, u32 newEntry)
{
    u8 str[16];
    u8 str2[32];
    u16 species;
    const u8 *name;
    const u8 *category;
    const u8 *description;

    // if (newEntry)
    //     PrintInfoScreenText(gText_PokedexRegistration, GetStringCenterAlignXOffset(1, gText_PokedexRegistration, 0xF0), 0);
    if (value == 0)
        value = NationalToHoennOrder(num);
    else
        value = num;
    ConvertIntToDecimalStringN(StringCopy(str, gText_NumberClear01), value, STR_CONV_MODE_LEADING_ZEROS, 3);
    PrintInfoScreenText(str, 96, 25);
    species = NationalPokedexNumToSpeciesHGSS(num);
    if (species)
        name = gSpeciesNames[species];
    else
        name = sText_TenDashes2;
    PrintInfoScreenText(name, 132, 25);
    if (owned)
    {
        CopyMonCategoryText(num, str2);
        category = str2;
    }
    else
    {
        category = gText_5MarksPokemon;
    }
    PrintInfoScreenText(category, 100, 41);
    PrintInfoScreenText(gText_HTHeight, 96, 57);
    PrintInfoScreenText(gText_WTWeight, 96, 73);
    if (owned)
    {
        PrintMonHeight(gPokedexEntries[num].height, 129, 57);
        PrintMonWeight(gPokedexEntries[num].weight, 129, 73);
    }
    else
    {
        PrintInfoScreenText(gText_UnkHeight, 129, 57);
        PrintInfoScreenText(gText_UnkWeight, 129, 73);
    }
    if (owned)
    {
        description = gPokedexEntries[num].description;
    }
    else
        description = sExpandedPlaceholder_PokedexDescription;
    PrintInfoScreenText(description, GetStringCenterAlignXOffset(FONT_NORMAL, description, DISPLAY_WIDTH), 95);

    //Type Icon(s) //HGSS_Ui
    if (owned)
        PrintCurrentSpeciesTypeInfo(newEntry, species); //HGSS_Ui
}

static void PrintMonHeight(u16 height, u8 left, u8 top)
{
    u8 buffer[16];
    u32 inches, feet;
    u8 i = 0;

    inches = (height * 10000) / 254;
    if (inches % 10 >= 5)
        inches += 10;
    feet = inches / 120;
    inches = (inches - (feet * 120)) / 10;

    buffer[i++] = EXT_CTRL_CODE_BEGIN;
    buffer[i++] = EXT_CTRL_CODE_CLEAR_TO;
    if (feet / 10 == 0)
    {
        buffer[i++] = 18;
        buffer[i++] = feet + CHAR_0;
    }
    else
    {
        buffer[i++] = 12;
        buffer[i++] = feet / 10 + CHAR_0;
        buffer[i++] = (feet % 10) + CHAR_0;
    }
    buffer[i++] = CHAR_SGL_QUOTE_RIGHT;
    buffer[i++] = (inches / 10) + CHAR_0;
    buffer[i++] = (inches % 10) + CHAR_0;
    buffer[i++] = CHAR_DBL_QUOTE_RIGHT;
    buffer[i++] = EOS;
    PrintInfoScreenText(buffer, left, top);
}

static void PrintMonWeight(u16 weight, u8 left, u8 top)
{
    u8 buffer[16];
    bool8 output;
    u8 i;
    u32 lbs = (weight * 100000) / 4536;

    if (lbs % 10u >= 5)
        lbs += 10;
    i = 0;
    output = FALSE;

    if ((buffer[i] = (lbs / 100000) + CHAR_0) == CHAR_0 && !output)
    {
        buffer[i++] = CHAR_SPACER;
    }
    else
    {
        output = TRUE;
        i++;
    }

    lbs %= 100000;
    if ((buffer[i] = (lbs / 10000) + CHAR_0) == CHAR_0 && !output)
    {
        buffer[i++] = CHAR_SPACER;
    }
    else
    {
        output = TRUE;
        i++;
    }

    lbs %= 10000;
    if ((buffer[i] = (lbs / 1000) + CHAR_0) == CHAR_0 && !output)
    {
        buffer[i++] = CHAR_SPACER;
    }
    else
    {
        output = TRUE;
        i++;
    }

    lbs %= 1000;
    buffer[i++] = (lbs / 100) + CHAR_0;
    lbs %= 100;
    buffer[i++] = CHAR_PERIOD;
    buffer[i++] = (lbs / 10) + CHAR_0;
    buffer[i++] = CHAR_SPACE;
    buffer[i++] = CHAR_l;
    buffer[i++] = CHAR_b;
    buffer[i++] = CHAR_s;
    buffer[i++] = CHAR_PERIOD;
    buffer[i++] = EOS;
    PrintInfoScreenText(buffer, left, top);
}

// Unused in the English version, used to print height/weight in versions which use metric system.
static void PrintDecimalNum(u8 windowId, u16 num, u8 left, u8 top)
{
    u8 str[6];
    bool8 outputted = FALSE;
    u8 result;

    result = num / 1000;
    if (result == 0)
    {
        str[0] = CHAR_SPACER;
        outputted = FALSE;
    }
    else
    {
        str[0] = CHAR_0 + result;
        outputted = TRUE;
    }

    result = (num % 1000) / 100;
    if (result == 0 && !outputted)
    {
        str[1] = CHAR_SPACER;
        outputted = FALSE;
    }
    else
    {
        str[1] = CHAR_0 + result;
        outputted = TRUE;
    }

    str[2] = CHAR_0 + ((num % 1000) % 100) / 10;
    str[3] = CHAR_DEC_SEPARATOR;
    str[4] = CHAR_0 + ((num % 1000) % 100) % 10;
    str[5] = EOS;
    PrintInfoSubMenuText(windowId, str, left, top);
}

// The footprints are drawn on WIN_FOOTPRINT, which uses BG palette 15 (loaded with the overworld textbox palette)
// The footprint pixels are stored as 1BPP, and set to the below color index in this palette when converted to 4BPP.
#define FOOTPRINT_COLOR_IDX  2

#define NUM_FOOTPRINT_TILES  4

static void DrawFootprint(u8 windowId, u16 dexNum)
{
    u8 footprint4bpp[TILE_SIZE_4BPP * NUM_FOOTPRINT_TILES];
    const u8 *footprintGfx = gMonFootprintTable[DexNumToSpecies(dexNum)];
    u16 tileIdx = 0;
    u16 i, j;

    for (i = 0; i < TILE_SIZE_1BPP * NUM_FOOTPRINT_TILES; i++)
    {
        u8 footprint1bpp = footprintGfx[i];

        // Convert the 8 pixels in the above 1BPP byte to 4BPP.
        // Each iteration creates one 4BPP byte (2 pixels),
        // so we need 4 iterations to do all 8 pixels.
        for (j = 0; j < 4; j++)
        {
            u8 tile = 0;
            if (footprint1bpp & (1 << (2 * j)))
                tile |= FOOTPRINT_COLOR_IDX; // Set pixel
            if (footprint1bpp & (2 << (2 * j)))
                tile |= FOOTPRINT_COLOR_IDX << 4; // Set pixel
            footprint4bpp[tileIdx] = tile;
            tileIdx++;
        }
    }
    CopyToWindowPixelBuffer(windowId, footprint4bpp, sizeof(footprint4bpp), 0);
}

static void PrintInfoSubMenuText(u8 windowId, const u8 *str, u8 left, u8 top)
{
    u8 color[3];
    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_LIGHT_GRAY;

    AddTextPrinterParameterized4(windowId, FONT_NORMAL, left, top, 0, 0, color, TEXT_SKIP_DRAW, str);
}

static u8 PrintCryScreenSpeciesName(u8 windowId, u16 num, u8 left, u8 top)
{
    u8 str[POKEMON_NAME_LENGTH + 1];
    u8 i;

    for (i = 0; i < ARRAY_COUNT(str); i++)
        str[i] = EOS;
    num = NationalPokedexNumToSpeciesHGSS(num);
    switch (num)
    {
    default:
        for (i = 0; gSpeciesNames[num][i] != EOS && i < POKEMON_NAME_LENGTH; i++)
            str[i] = gSpeciesNames[num][i];
        break;
    case 0:
        for (i = 0; i < 5; i++)
            str[i] = CHAR_HYPHEN;
        break;
    }
    PrintInfoSubMenuText(windowId, str, left, top);
    return i;
}

static u16 CreateSizeScreenTrainerPic(u16 species, s16 x, s16 y, s8 paletteSlot)
{
    return CreateTrainerPicSprite(species, TRUE, x, y, paletteSlot, TAG_NONE);
}


//************************************
//*                                  *
//*        Helper functions          *
//*                                  *
//************************************

static void ResetOtherVideoRegisters(u16 regBits)
{
    if (!(regBits & DISPCNT_BG0_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG0_ON);
        SetGpuReg(REG_OFFSET_BG0CNT, 0);
        SetGpuReg(REG_OFFSET_BG0HOFS, 0);
        SetGpuReg(REG_OFFSET_BG0VOFS, 0);
    }
    if (!(regBits & DISPCNT_BG1_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG1_ON);
        SetGpuReg(REG_OFFSET_BG1CNT, 0);
        SetGpuReg(REG_OFFSET_BG1HOFS, 0);
        SetGpuReg(REG_OFFSET_BG1VOFS, 0);
    }
    if (!(regBits & DISPCNT_BG2_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG2_ON);
        SetGpuReg(REG_OFFSET_BG2CNT, 0);
        SetGpuReg(REG_OFFSET_BG2HOFS, 0);
        SetGpuReg(REG_OFFSET_BG2VOFS, 0);
    }
    if (!(regBits & DISPCNT_BG3_ON))
    {
        ClearGpuRegBits(0, DISPCNT_BG3_ON);
        SetGpuReg(REG_OFFSET_BG3CNT, 0);
        SetGpuReg(REG_OFFSET_BG3HOFS, 0);
        SetGpuReg(REG_OFFSET_BG3VOFS, 0);
    }
    if (!(regBits & DISPCNT_OBJ_ON))
    {
        ClearGpuRegBits(0, DISPCNT_OBJ_ON);
        ResetSpriteData();
        FreeAllSpritePalettes();
        gReservedSpritePaletteCount = 8;
    }
}

static void FreeWindowAndBgBuffers(void)
{
    void *tilemapBuffer;

    FreeAllWindowBuffers();
    tilemapBuffer = GetBgTilemapBuffer(0);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(1);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(2);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(3);
    if (tilemapBuffer)
        Free(tilemapBuffer);
}

static u16 GetNextPosition(u8 direction, u16 position, u16 min, u16 max)
{
    switch (direction)
    {
    case 1: // Up/Left
        if (position > min)
            position--;
        break;
    case 0: // Down/Right
        if (position < max)
            position++;
        break;
    case 3: // Up/Left with loop (unused)
        if (position > min)
            position--;
        else
            position = max;
        break;
    case 2: // Down/Right with loop (unused)
        if (position < max)
            position++;
        else
            position = min;
        break;
    }
    return position;
}

// Unown and Spinda use the personality of the first seen individual of that species
// All others use personality 0
static u32 GetPokedexMonPersonality(u16 species)
{
    if (species == SPECIES_UNOWN || species == SPECIES_SPINDA)
    {
        if (species == SPECIES_UNOWN)
            return gSaveBlock2Ptr->pokedex.unownPersonality;
        else
            return gSaveBlock2Ptr->pokedex.spindaPersonality;
    }
    else
    {
        return 0;
    }
}



//************************************
//*                                  *
//*        HGSS                      *
//*                                  *
//************************************

// NationalPokedexNumToSpecies() is a linear scan over all species, so the dex caches the
// national dex number -> species mapping once (O(NUM_SPECIES)) whenever sPokedexView is allocated.
static void BuildNatDexToSpeciesTable(void)
{
    u16 species, dexNum;

    for (dexNum = 0; dexNum <= NATIONAL_DEX_COUNT; dexNum++)
        sPokedexView->natDexToSpecies[dexNum] = SPECIES_NONE;

    for (species = 1; species < NUM_SPECIES; species++)
    {
        dexNum = SpeciesToNationalPokedexNum(species);
        // Keep the lowest species index, like NationalPokedexNumToSpecies does
        if (dexNum != 0 && dexNum <= NATIONAL_DEX_COUNT && sPokedexView->natDexToSpecies[dexNum] == SPECIES_NONE)
            sPokedexView->natDexToSpecies[dexNum] = species;
    }
}

static u16 DexNumToSpecies(u16 dexNum)
{
    if (dexNum == 0 || dexNum > NATIONAL_DEX_COUNT)
        return SPECIES_NONE;
    if (sPokedexView != NULL)
        return sPokedexView->natDexToSpecies[dexNum];
    return NationalPokedexNumToSpecies(dexNum);
}

u16 NationalPokedexNumToSpeciesHGSS(u16 nationalNum)
{
    u16 species;

    if (!nationalNum)
        return 0;

    #ifndef POKEMON_EXPANSION
        return DexNumToSpecies(nationalNum);
    #else
        if (sPokedexView->formSpecies != 0)
            return sPokedexView->formSpecies;
        else
            return DexNumToSpecies(nationalNum);
    #endif
}

static void LoadTilesetTilemapHGSS(u8 page)
{
    switch (page)
    {
    case INFO_SCREEN:
        if (!HGSS_DECAPPED)
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_1_Gfx, 0x2000, 0, 0);
        else
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_1_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenInfo_Tilemap, 0, 0);
        break;
    case STATS_SCREEN:
        if (!HGSS_DECAPPED)
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_1_Gfx, 0x2000, 0, 0);
        else
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_1_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenStats_Tilemap, 0, 0);
        break;
    case EVO_SCREEN:
        if (!HGSS_DECAPPED)
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_2_Gfx, 0x2000, 0, 0);
        else
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_2_Gfx, 0x2000, 0, 0);
        #ifndef POKEMON_EXPANSION
            CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenEvolution_Tilemap, 0, 0);
        #endif
        #ifdef POKEMON_EXPANSION
            CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenEvolution_Tilemap_PE, 0, 0);
        #endif
        break;
    case FORMS_SCREEN: //Pokemonexpansion only (rhh)
        if (!HGSS_DECAPPED)
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_2_Gfx, 0x2000, 0, 0);
        else
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_2_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenForms_Tilemap, 0, 0);
        break;
    case CRY_SCREEN:
        if (!HGSS_DECAPPED)
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_3_Gfx, 0x2000, 0, 0);
        else
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_3_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenCry_Tilemap, 0, 0);
        break;
    case SIZE_SCREEN:
        if (!HGSS_DECAPPED)
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_3_Gfx, 0x2000, 0, 0);
        else
            DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_3_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenSize_Tilemap, 0, 0);
        break;
    }
}

//Physical/Special Split from BE
#ifdef BATTLE_ENGINE
static u8 ShowSplitIcon(u32 split)
{
    if (sPokedexView->splitIconSpriteId == 0xFF)
        sPokedexView->splitIconSpriteId = CreateSprite(&sSpriteTemplate_SplitIcons, 139, 90, 0);

    gSprites[sPokedexView->splitIconSpriteId].invisible = FALSE;
    StartSpriteAnim(&gSprites[sPokedexView->splitIconSpriteId], split);
    return sPokedexView->splitIconSpriteId;
}
static void DestroySplitIcon(void)
{
    if (sPokedexView->splitIconSpriteId != 0xFF)
        DestroySprite(&gSprites[sPokedexView->splitIconSpriteId]);
    sPokedexView->splitIconSpriteId = 0xFF;
}
#endif



//************************************
//*                                  *
//*        STATS                     *
//*                                  *
//************************************
static const u8 sStatsPageNavigationTextColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};

static void StatsPage_PrintNavigationButtons(void)
{
    u8 x = 9;
    u8 y = 0;
    if (!HGSS_DECAPPED)
        AddTextPrinterParameterized3(WIN_STATS_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, gText_Stats_Buttons);
    else
        AddTextPrinterParameterized3(WIN_STATS_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, gText_Stats_Buttons_Decapped);

    PutWindowTilemap(WIN_STATS_NAVIGATION_BUTTONS);
    CopyWindowToVram(WIN_STATS_NAVIGATION_BUTTONS, 3);
}

static void ResetStatsWindows(void)
{
    u8 i;

    FreeAllWindowBuffers();
    InitWindows(sStatsScreen_WindowTemplates);

    for (i = 0; i < WIN_STATS_END + 1; i++)
    {
        FillWindowPixelBuffer(i, PIXEL_FILL(0));
        PutWindowTilemap(i);
        CopyWindowToVram(i, 3);
    }
}

static void SaveMonDataInStruct(void)
{
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
    u8 EVs[6] = {gSpeciesInfo[species].evYield_HP, gSpeciesInfo[species].evYield_Speed, gSpeciesInfo[species].evYield_Attack, gSpeciesInfo[species].evYield_SpAttack, gSpeciesInfo[species].evYield_Defense, gSpeciesInfo[species].evYield_SpDefense};
    u8 differentEVs = 0;
    u8 i;

    //Count how many different EVs
    for (i = 0; i<6; i++)
    {
        if (EVs[i] > 0) //HP//Speed//Attack//Special Attack//Defense//Special Defense
            differentEVs++;
    }

    sPokedexView->sPokemonStats.species             = species;
    sPokedexView->sPokemonStats.genderRatio         = gSpeciesInfo[species].genderRatio;
    sPokedexView->sPokemonStats.baseHP              = gSpeciesInfo[species].baseHP;
    sPokedexView->sPokemonStats.baseSpeed           = gSpeciesInfo[species].baseSpeed;
    sPokedexView->sPokemonStats.baseAttack          = gSpeciesInfo[species].baseAttack;
    sPokedexView->sPokemonStats.baseSpAttack        = gSpeciesInfo[species].baseSpAttack;
    sPokedexView->sPokemonStats.baseDefense         = gSpeciesInfo[species].baseDefense;
    sPokedexView->sPokemonStats.baseSpDefense       = gSpeciesInfo[species].baseSpDefense;
    sPokedexView->sPokemonStats.differentEVs        = differentEVs;
    sPokedexView->sPokemonStats.evYield_HP          = EVs[0];
    sPokedexView->sPokemonStats.evYield_Speed       = EVs[1];
    sPokedexView->sPokemonStats.evYield_Attack      = EVs[2];
    sPokedexView->sPokemonStats.evYield_SpAttack    = EVs[3];
    sPokedexView->sPokemonStats.evYield_Defense     = EVs[4];
    sPokedexView->sPokemonStats.evYield_SpDefense   = EVs[5];
    sPokedexView->sPokemonStats.catchRate           = gSpeciesInfo[species].catchRate;
    sPokedexView->sPokemonStats.growthRate          = gSpeciesInfo[species].growthRate;
    sPokedexView->sPokemonStats.eggGroup1           = gSpeciesInfo[species].eggGroups[0];
    sPokedexView->sPokemonStats.eggGroup2           = gSpeciesInfo[species].eggGroups[1];
    sPokedexView->sPokemonStats.eggCycles           = gSpeciesInfo[species].eggCycles;
    sPokedexView->sPokemonStats.expYield            = gSpeciesInfo[species].expYield;
    sPokedexView->sPokemonStats.friendship          = gSpeciesInfo[species].friendship;
    sPokedexView->sPokemonStats.ability0            = GetAbilityBySpecies(species, 0);
    sPokedexView->sPokemonStats.ability1            = GetAbilityBySpecies(species, 1);
    #ifdef POKEMON_EXPANSION
    sPokedexView->sPokemonStats.abilityHidden       = GetAbilityBySpecies(species, 2);
    #endif
}

#define tMonSpriteId data[4]

static void Task_LoadStatsScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            u16 r2;

            sPokedexView->currentPage = STATS_SCREEN;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            r2 = 0;
            if (gTasks[taskId].data[1] != 0)
                r2 += DISPCNT_OBJ_ON;
            if (gTasks[taskId].data[2] != 0)
                r2 |= DISPCNT_BG1_ON;
            ResetOtherVideoRegisters(r2);
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(STATS_SCREEN);

        ResetStatsWindows();

        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain(0xD);
        LoadPokedexBgPalette(sPokedexView->isSearchResults);
        gMain.state++;
        break;
    case 3:
        sPokedexView->typeIconSpriteIds[0] = 0xFF;
        sPokedexView->typeIconSpriteIds[1] = 0xFF;
        CreateTypeIconSprites();
        #ifdef BATTLE_ENGINE
            sPokedexView->splitIconSpriteId = 0xFF; //Physical/Special Split from BE
            LoadCompressedPalette(gMoveTypes_Pal, 0x1D0, 0x60); //Physical/Special Split from BE
            LoadCompressedSpriteSheet(&sSpriteSheet_SplitIcons); //Physical/Special Split from BE
            LoadSpritePalette(&sSpritePal_SplitIcons); //Physical/Special Split from BE
        #endif
        gMain.state++;
        break;
    case 4:
        SaveMonDataInStruct();
        sPokedexView->moveSelected = 0;
        sPokedexView->movesTotal = 0;
        sPokedexView->numEggMoves = 0;
        sPokedexView->numLevelUpMoves = 0;
        sPokedexView->numTMHMMoves = 0;
        if (CalculateMoves())
            gMain.state++;
        break;
    case 5:
        if (gTasks[taskId].data[1] == 0)
        {
            //Icon
            FreeMonIconPalettes(); //Free space for new pallete
            LoadMonIconPalette(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum)); //Loads pallete for current mon
            #ifndef POKEMON_EXPANSION
                gTasks[taskId].data[4] = CreateMonIcon(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), SpriteCB_MonIcon, 18, 31, 4, 0, TRUE); //Create pokemon sprite
            #endif
            #ifdef POKEMON_EXPANSION
                gTasks[taskId].data[6] = CreateMonIcon(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), SpriteCB_MonIcon, 18, 31, 4, 0); //Create pokemon sprite
            #endif
            gSprites[gTasks[taskId].data[4]].oam.priority = 0;
        }
        gMain.state++;
        break;
    case 6:
        gTasks[taskId].data[5] = 0;
        PrintStatsScreen_NameGender(taskId, sPokedexListItem->dexNum, sPokedexView->dexMode == DEX_MODE_HOENN ? FALSE : TRUE);
        PrintStatsScreen_Left(taskId);
        PrintStatsScreen_Abilities(taskId);
        PrintStatsScreen_Moves_Top(taskId);
        PrintStatsScreen_Moves_Description(taskId);
        PrintStatsScreen_Moves_BottomText(taskId);
        PrintStatsScreen_Moves_Bottom(taskId);
        if (!sPokedexListItem->owned)
            LoadPalette(&gPlttBufferUnfaded[BG_PLTT_ID(0) + 1], BG_PLTT_ID(3) + 1, PLTT_SIZEOF(16 - 1));
        StatsPage_PrintNavigationButtons(); //gText_Stats_Buttons
        gMain.state++;
        break;
    case 7:
        {
        u32 preservedPalettes = 0;

        if (gTasks[taskId].data[2] != 0)
            preservedPalettes = 0x14; // each bit represents a palette index
        if (gTasks[taskId].data[1] != 0)
            preservedPalettes |= (1 << (gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum + 16));
        BeginNormalPaletteFade(~preservedPalettes, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gMain.state++;
        }
        break;
    case 8:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 9:
        if (!gPaletteFade.active)
            gMain.state++;
        break;
    case 10:
        gMain.state++;
        break;
    case 11:
        gTasks[taskId].data[0] = 0;
        gTasks[taskId].data[1] = 0;
        gTasks[taskId].data[2] = 1;
        gTasks[taskId].func = Task_HandleStatsScreenInput;
        gMain.state = 0;
        break;
    }
}

static void FreeStatsScreenWindowAndBgBuffers(void)
{
    void *tilemapBuffer;

    FreeAllWindowBuffers();
    tilemapBuffer = GetBgTilemapBuffer(0);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(1);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(2);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(3);
    if (tilemapBuffer)
        Free(tilemapBuffer);
}

static void Task_HandleStatsScreenInput(u8 taskId)
{
    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_DEX_PAGE);
        if (gTasks[taskId].data[5] == 0)
            gTasks[taskId].data[5] = 1;
        else
            gTasks[taskId].data[5] = 0;

        FillWindowPixelBuffer(WIN_STATS_LEFT, PIXEL_FILL(0));
        PrintStatsScreen_Left(taskId);

        FillWindowPixelBuffer(WIN_STATS_MOVES_DESCRIPTION, PIXEL_FILL(0));
        PrintStatsScreen_Moves_Description(taskId);

        FillWindowPixelBuffer(WIN_STATS_MOVES_BOTTOM, PIXEL_FILL(0));
        PrintStatsScreen_Moves_BottomText(taskId);
        PrintStatsScreen_Moves_Bottom(taskId);

        #ifdef POKEMON_EXPANSION
        FillWindowPixelBuffer(WIN_STATS_ABILITIES, PIXEL_FILL(0));
        PrintStatsScreen_Abilities(taskId);
        #endif
    }
    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_ExitStatsScreen;
        PlaySE(SE_PC_OFF);
        return;
    }

    //Change moves
    if (JOY_REPEAT(DPAD_UP) && sPokedexView->moveSelected > 0)
    {
        sPokedexView->moveSelected -= 1;
        PlaySE(SE_SELECT);
        FillWindowPixelBuffer(WIN_STATS_MOVES_TOP, PIXEL_FILL(0));
        PrintStatsScreen_DestroyMoveItemIcon(taskId);
        PrintStatsScreen_Moves_Top(taskId);

        FillWindowPixelBuffer(WIN_STATS_MOVES_DESCRIPTION, PIXEL_FILL(0));
        PrintStatsScreen_Moves_Description(taskId);

        FillWindowPixelRect(WIN_STATS_MOVES_BOTTOM, PIXEL_FILL(0), 50, 0, 20, 16);
        FillWindowPixelRect(WIN_STATS_MOVES_BOTTOM, PIXEL_FILL(0), 120, 0, 20, 16);
        PrintStatsScreen_Moves_Bottom(taskId);
    }
    if (JOY_REPEAT(DPAD_DOWN) && sPokedexView->moveSelected < sPokedexView->movesTotal -1 )
    {
        sPokedexView->moveSelected = sPokedexView->moveSelected + 1;
        PlaySE(SE_SELECT);
        FillWindowPixelBuffer(WIN_STATS_MOVES_TOP, PIXEL_FILL(0));
        PrintStatsScreen_DestroyMoveItemIcon(taskId);
        PrintStatsScreen_Moves_Top(taskId);

        FillWindowPixelBuffer(WIN_STATS_MOVES_DESCRIPTION, PIXEL_FILL(0));
        PrintStatsScreen_Moves_Description(taskId);

        FillWindowPixelRect(WIN_STATS_MOVES_BOTTOM, PIXEL_FILL(0), 50, 0, 20, 16);
        FillWindowPixelRect(WIN_STATS_MOVES_BOTTOM, PIXEL_FILL(0), 120, 0, 20, 16);
        PrintStatsScreen_Moves_Bottom(taskId);
    }

    //Switch screens
    if ((JOY_NEW(DPAD_LEFT) || (JOY_NEW(L_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        sPokedexView->selectedScreen = INFO_SCREEN;
        BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 1;
        gTasks[taskId].func = Task_SwitchScreensFromStatsScreen;
        PlaySE(SE_PIN);
    }
    if ((JOY_NEW(DPAD_RIGHT) || (JOY_NEW(R_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        if (!sPokedexListItem->owned)
            PlaySE(SE_FAILURE);
        else
        {
            sPokedexView->selectedScreen = EVO_SCREEN;
            BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
            sPokedexView->screenSwitchState = 3;
            gTasks[taskId].func = Task_SwitchScreensFromStatsScreen;
            PlaySE(SE_PIN);
        }
    }
}

#define ITEM_TAG 0xFDF3

static void PrintStatsScreen_DestroyMoveItemIcon(u8 taskId)
{
    FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
    FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
    FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[3]]); //Destroy item icon
    DestroySprite(&gSprites[gTasks[taskId].data[3]]);       //Destroy item icon
}

static u8 CollectInheritedEggMoves(u16 species, u16 *eggMoves)
{
    u16 stageMoves[EGG_MOVES_ARRAY_COUNT];
    u8 count = 0, stage, num, i, j;

    for (stage = 0; stage < 3 && species != SPECIES_NONE; stage++)
    {
        num = GetEggMovesSpecies(species, stageMoves);
        for (i = 0; i < num && count < STATS_EGG_MOVES_MAX; i++)
        {
            for (j = 0; j < count; j++)
            {
                if (eggMoves[j] == stageMoves[i])
                    break;
            }
            if (j == count)
                eggMoves[count++] = stageMoves[i];
        }
        species = GetPreEvolutionSpecies(species, NULL);
    }
    return count;
}

static bool8 CalculateMoves(void)
{
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);

    u16 statsMovesEgg[STATS_EGG_MOVES_MAX] = {0};
    u16 statsMovesLevelUp[MAX_LEVEL_UP_MOVES] = {0};
    u16 statsMovesTMHM[NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES] = {0};
    #ifndef BATTLE_ENGINE
    u16 statsMovesTutor[TUTOR_MOVE_COUNT] = {0};
    #else
    u16 move;
    #endif

    u8 numEggMoves = 0;
    u8 numLevelUpMoves = 0;
    u8 numTMHMMoves = 0;
    u8 numTutorMoves = 0;
    u16 movesTotal = 0;
    u8 i,j;

    #ifdef POKEMON_EXPANSION
    // Mega pokemon don't have distinct learnsets from their base form; so use base species for calculation
    if (species >= SPECIES_VENUSAUR_MEGA && species <= SPECIES_GROUDON_PRIMAL)
        species = GetFormSpeciesId(species, 0);
    #endif

    //Egg moves of this species, then those inherited from its pre-evolutions (no repeats)
    numEggMoves = CollectInheritedEggMoves(species, statsMovesEgg);

    // Order: level up, TM/HM, tutor, egg

    //Level up moves in learnset order. Entries repeating the same move at the same level are
    //listed once; the same move at different levels (e.g. an evolved Pokémon's Lv1 copy of a
    //pre-evolution move) stays listed at each level. Lv0 (learned on evolution) shows as EVO.
    for (i = 0; i < MAX_LEVEL_UP_MOVES && gLevelUpLearnsets[species][i] != LEVEL_UP_END; i++)
    {
        u16 move = gLevelUpLearnsets[species][i] & LEVEL_UP_MOVE_ID;
        u8 level = (gLevelUpLearnsets[species][i] & LEVEL_UP_MOVE_LV) >> 9;

        for (j = 0; j < numLevelUpMoves; j++)
        {
            if (statsMovesLevelUp[j] == move && sStatsMovesLevel[j] == level)
                break;
        }
        if (j == numLevelUpMoves)
        {
            statsMovesLevelUp[numLevelUpMoves] = move;
            sStatsMovesLevel[numLevelUpMoves] = level;
            numLevelUpMoves++;
        }
    }
    for (i=0; i < numLevelUpMoves; i++)
    {
        sStatsMoves[movesTotal] = statsMovesLevelUp[i];
        movesTotal++;
    }

    #ifndef BATTLE_ENGINE
    //TMHM moves
    for (j = 0; j < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; j++)
    {
        if (CanSpeciesLearnTMHM(species, j))
        {
            sStatsMoves[movesTotal] = ItemIdToBattleMoveId(ITEM_TM01 + j);
            movesTotal++;
            sStatsMovesTMHM_ID[numTMHMMoves] = (ITEM_TM01 + j);
            numTMHMMoves++;
        }
    }

    //Tutor moves
    for (i=0; i < TUTOR_MOVE_COUNT; i++)
    {
        if (CanLearnTutorMove(species, i)) //if (sTutorLearnsets[species] & (1 << i))
        {
            sStatsMoves[movesTotal] = gTutorMoves[i];
            numTutorMoves++;
            movesTotal++;
        }
    }
    #else
    for (i = 0; gTeachableLearnsets[species][i] != MOVE_UNAVAILABLE; i++)
    {
        move = gTeachableLearnsets[species][i];
        for (j = 0; j < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; j++)
        {
            if (GetTMHMMoves(j) == move)
            {
                sStatsMovesTMHM_ID[numTMHMMoves] = (ITEM_TM01 + j);
                numTMHMMoves++;
                break;
            }
        }
        if (j >= NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES)
            numTutorMoves++;

        sStatsMoves[movesTotal] = move;
        movesTotal++;
    }

    #endif

    //Egg moves
    for (i=0; i < numEggMoves; i++)
    {
        sStatsMoves[movesTotal] = statsMovesEgg[i];
        movesTotal++;
    }

    sPokedexView->numEggMoves = numEggMoves;
    sPokedexView->numLevelUpMoves = numLevelUpMoves;
    sPokedexView->numTMHMMoves = numTMHMMoves;
    sPokedexView->numTutorMoves = numTutorMoves;
    sPokedexView->movesTotal = movesTotal;

    return TRUE;
}

static void PrintStatsScreen_Moves_Top(u8 taskId)
{
    u8 numEggMoves      = sPokedexView->numEggMoves;
    u8 numLevelUpMoves  = sPokedexView->numLevelUpMoves;
    u8 numTMHMMoves     = sPokedexView->numTMHMMoves;
    u8 numTutorMoves    = sPokedexView->numTutorMoves;
    u8 movesTotal       = sPokedexView->movesTotal;
    u8 selected         = sPokedexView->moveSelected;
    u8 level;
    u8 moves_x = 5;
    u8 moves_y = 3;
    u16 move;
    u16 item;

    //Move
    move = sStatsMoves[selected];

    //Moves selected from move max
    ConvertIntToDecimalStringN(gStringVar1, (selected+1), STR_CONV_MODE_RIGHT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, movesTotal, STR_CONV_MODE_RIGHT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar1, gText_Stats_MoveSelectedMax);
    PrintStatsScreenTextSmallWhite(WIN_STATS_MOVES_TOP, gStringVar1, moves_x-1, moves_y+1);

    //Move name
    StringCopy(gStringVar3, gMoveNames[move]);
    StringCopyPadded(gStringVar3, gStringVar3, CHAR_SPACE, 20);
    PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, gStringVar3, moves_x, moves_y + 17);

    //Draw move type icon
    if (gTasks[taskId].data[5] == 0)
    {
        SetTypeIconPosAndPal(gBattleMoves[move].type, moves_x + 146, moves_y + 17, 0);
        SetSpriteInvisibility(1, TRUE);
    }
    else
    {
        SetTypeIconPosAndPal(NUMBER_OF_MON_TYPES + gContestMoves[move].contestCategory, moves_x + 146, moves_y + 17, 1);
        SetSpriteInvisibility(0, TRUE);
    }

    //Calculate and retrieve correct move from the arrays (order: level up, TM/HM, tutor, egg)
    if (selected < numLevelUpMoves)
    {
        level = sStatsMovesLevel[selected];
        if (level == 0)
            StringCopy(gStringVar1, sText_Stats_MoveEvo);
        else
            ConvertIntToDecimalStringN(gStringVar1, level, STR_CONV_MODE_LEFT_ALIGN, 3); //Move learn lvl
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, gText_Stats_MoveLevel, moves_x + 113, moves_y + 3); //Level text
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, gStringVar1, moves_x + 113, moves_y + 14); //Print level
        item = ITEM_EXP_SHARE;
    }
    else if (selected < (numLevelUpMoves + numTMHMMoves))
    {
        CopyItemName(sStatsMovesTMHM_ID[(selected-numLevelUpMoves)], gStringVar1); //TM name
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, gStringVar1, moves_x + 113, moves_y + 9);
        item = sStatsMovesTMHM_ID[(selected-numLevelUpMoves)];
    }
    else if (selected < (numLevelUpMoves + numTMHMMoves + numTutorMoves))
    {
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, gText_ThreeDashes, moves_x + 113, moves_y + 9);
        item = ITEM_TEACHY_TV;
    }
    else if (selected < (numLevelUpMoves + numTMHMMoves + numTutorMoves + numEggMoves))
    {
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_TOP, gText_ThreeDashes, moves_x + 113, moves_y + 9);
        item = ITEM_LUCKY_EGG;
    }
    else
    {
        StringCopy(gStringVar4, gText_CommunicationError);
        item = ITEM_MASTER_BALL;
    }

    //Egg/TM/Level/Tutor Item Icon
    gTasks[taskId].data[3] = AddItemIconSprite(ITEM_TAG, ITEM_TAG, item);
    gSprites[gTasks[taskId].data[3]].x2 = 203;
    gSprites[gTasks[taskId].data[3]].y2 = 39;
    gSprites[gTasks[taskId].data[3]].oam.priority = 0;

}

static void PrintStatsScreen_Moves_Description(u8 taskId)
{
    u8 selected = sPokedexView->moveSelected;
    u16 move;
    u8 moves_x = 5;
    u8 moves_y = 5;

    //Move
    move = sStatsMoves[selected];

    //Move description
    if (gTasks[taskId].data[5] == 0)
    {
        StringCopy(gStringVar4, gMoveDescriptionPointers[(move - 1)]);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_DESCRIPTION, gStringVar4, moves_x, moves_y);
    }
    else
    {
        StringCopy(gStringVar4, gContestEffectDescriptionPointers[gContestMoves[move].effect]);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_DESCRIPTION, gStringVar4, moves_x, moves_y);
    }
}

static void PrintStatsScreen_Moves_BottomText(u8 taskId)
{
    u8 moves_x = 8;
    u8 moves_y = 3;
    if (gTasks[taskId].data[5] == 0)
    {
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gText_Power,  moves_x, moves_y);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gText_Accuracy2,  moves_x + 66, moves_y);
    }
    else
    {
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gText_Appeal,  moves_x, moves_y);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gText_Jam,  moves_x + 66, moves_y);
    }
}

static void PrintStatsScreen_Moves_Bottom(u8 taskId)
{
    u8 moves_x = 8;
    u8 moves_y = 3;
    u8 selected = sPokedexView->moveSelected;
    u16 move;
    //Contest
    u8 contest_effectValue;
    u8 contest_appeal = 0;
    u8 contest_jam = 0;

    //Move
    move = sStatsMoves[selected];

    //Power + Accuracy
    if (gTasks[taskId].data[5] == 0)
    {
        //Power
        if (gBattleMoves[move].power < 2)
            StringCopy(gStringVar1, gText_ThreeDashes);
        else
            ConvertIntToDecimalStringN(gStringVar1, gBattleMoves[move].power, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gStringVar1, moves_x + 45, moves_y);
        //Physical/Special Split from BE
        #ifdef BATTLE_ENGINE
            DestroySplitIcon();
            ShowSplitIcon(GetBattleMoveSplit(move));
        #endif
        //Accuracy
        if (gBattleMoves[move].accuracy == 0)
            StringCopy(gStringVar1, gText_ThreeDashes);
        else
            ConvertIntToDecimalStringN(gStringVar1, gBattleMoves[move].accuracy, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gStringVar1,  moves_x + 114, moves_y);
    }
    else //Appeal + Jam
    {
        #ifdef BATTLE_ENGINE
            DestroySplitIcon();
            gSprites[sPokedexView->splitIconSpriteId].invisible = TRUE;
        #endif
        //Appeal
        contest_effectValue = gContestEffects[gContestMoves[move].effect].appeal;
        if (contest_effectValue != 0xFF)
            contest_appeal = contest_effectValue / 10;
        ConvertIntToDecimalStringN(gStringVar1, contest_appeal, STR_CONV_MODE_RIGHT_ALIGN, 1);
        StringCopy(gStringVar2, gText_PlusSymbol);
        StringAppend(gStringVar2, gStringVar1);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gStringVar2, moves_x + 45, moves_y);

        //Jam
        contest_effectValue = gContestEffects[gContestMoves[move].effect].jam;
        if (contest_effectValue != 0xFF)
            contest_jam = contest_effectValue / 10;
        ConvertIntToDecimalStringN(gStringVar1, contest_jam, STR_CONV_MODE_RIGHT_ALIGN, 1);
        StringCopy(gStringVar2, gText_Stats_Minus);
        StringAppend(gStringVar2, gStringVar1);
        PrintStatsScreenTextSmall(WIN_STATS_MOVES_BOTTOM, gStringVar2,  moves_x + 119, moves_y);
    }
}

static void PrintStatsScreen_NameGender(u8 taskId, u32 num, u32 value)
{
    u8 str[16];
    u8 str2[32];
    u8 strEV[25];
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
    u16 natNum;
    u8 evVal;
    const u8 *category;
    const u8 *description;
    const u8 *strEVtype;

    u8 base_x = 38;
    u8 base_y = 0;
    u8 gender_x, gender_y;

    //Name
    PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, gSpeciesNames[species], base_x, base_y);

    //Number
    if (value == 0)
        value = NationalToHoennOrder(num);
    else
        value = num;
    ConvertIntToDecimalStringN(StringCopy(str, gText_NumberClear01), value, STR_CONV_MODE_LEADING_ZEROS, 3);
    PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, str, base_x, base_y + 10);

    //Gender ratio //MON_GENDERLESS == 0xFF
    gender_x = base_x;
    gender_y = base_y + 20;
    switch (sPokedexView->sPokemonStats.genderRatio)
    {
    case 0:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, gText_Stats_Gender_0, gender_x, gender_y);
        break;
    case 31:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, gText_Stats_Gender_12_5, gender_x, gender_y);
        break;
    case 63:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, gText_Stats_Gender_25, gender_x, gender_y);
        break;
    case 127:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, gText_Stats_Gender_50, gender_x, gender_y);
        break;
    case 191:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, gText_Stats_Gender_75, gender_x, gender_y);
        break;
    case 223:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, gText_Stats_Gender_87_5, gender_x, gender_y);
        break;
    case 254:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, gText_Stats_Gender_100, gender_x, gender_y);
        break;
    default:
        PrintStatsScreenTextSmall(WIN_STATS_NAME_GENDER, gText_ThreeDashes, gender_x, gender_y);
        break;
    }
}

static u8 PrintMonStatsToggle_DifferentEVsColumn(u8 differentEVs)
{
    if (differentEVs == 1 || differentEVs == 3)
        return 0;
    else
        return 1;
}

static u8 PrintMonStatsToggle_DifferentEVsRow(u8 differentEVs)
{
    if (differentEVs == 1 || differentEVs == 2)
        return 0;
    else
        return 1;
}

static u8* PrintMonStatsToggle_EV_Arrows(u8 *dest, u8 value)
{
    switch (value)
    {
        case 1:
            StringCopy(dest, gText_Stats_EV_Plus1);
            break;
        case 2:
            StringCopy(dest, gText_Stats_EV_Plus2);
            break;
        case 3:
            StringCopy(dest, gText_Stats_EV_Plus3);
            break;
    }
    return dest;
}

static void PrintStatsScreen_Left(u8 taskId)
{
    u8 base_x = 8;
    u8 x_offset_column = 43;
    u8 x_offset_value = 26;
    u8 column = 0;
    u8 base_x_offset = 70;
    u8 base_x_first_row = 23;
    u8 base_x_second_row = 43;
    u8 base_y_offset = 11;
    u8 base_i = 0;
    u8 base_y = 5;
    u32 align_x;
    u8 total_x = 93;
    u8 strEV[25];
    u8 strBase[14];
    u8 EVs[6] = {sPokedexView->sPokemonStats.evYield_HP, sPokedexView->sPokemonStats.evYield_Speed, sPokedexView->sPokemonStats.evYield_Attack, sPokedexView->sPokemonStats.evYield_SpAttack, sPokedexView->sPokemonStats.evYield_Defense, sPokedexView->sPokemonStats.evYield_SpDefense};
    u8 differentEVs = 0;
    u8 i;

    //Base stats
    if (gTasks[taskId].data[5] == 0)
    {
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_HP, base_x, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseHP, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_first_row, base_y + base_y_offset*base_i);

        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_Speed, base_x+base_x_second_row, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpeed, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_offset, base_y + base_y_offset*base_i);

        base_i++;
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_Attack, base_x, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseAttack, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_first_row, base_y + base_y_offset*base_i);

        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_SpAttack, base_x+base_x_second_row, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpAttack, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_offset, base_y + base_y_offset*base_i);

        base_i++;
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_Defense, base_x, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseDefense, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_first_row, base_y + base_y_offset*base_i);

        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_SpDefense, base_x+base_x_second_row, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(strBase, sPokedexView->sPokemonStats.baseSpDefense, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strBase, base_x+base_x_offset, base_y + base_y_offset*base_i);
        base_i++;
    }
    else //EV increases
    {
        //If 1 or 2 EVs display with the same layout as the base stats
        if (sPokedexView->sPokemonStats.differentEVs < 3)
        {
            differentEVs = 0;
            //HP
            if (EVs[0] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_HP);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[0]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Speed
            if (EVs[1]> 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_Speed);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[1]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Attack
            if (EVs[2] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_Attack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[2]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Special Attack
            if (EVs[3] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_SpAttack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[3]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Defense
            if (EVs[4] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_Defense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[4]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
            //Special Defense
            if (EVs[5] > 0)
            {
                differentEVs++;
                column = PrintMonStatsToggle_DifferentEVsColumn(differentEVs);
                base_i = PrintMonStatsToggle_DifferentEVsRow(differentEVs);
                StringCopy(gStringVar1, gText_Stats_SpDefense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[5]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + x_offset_column*column, base_y + base_y_offset*base_i);
            }
        }
        else //3 different EVs in 1 row
        {
            column = 0;
            //HP
            if (EVs[0] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_HP);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[0]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Speed
            if (EVs[1] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_Speed);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[1]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Attack
            if (EVs[2] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_Attack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[2]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Special Attack
            if (EVs[3] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_SpAttack);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[3]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Defense
            if (EVs[4] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_Defense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[4]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
            //Special Defense
            if (EVs[5] > 0)
            {
                StringCopy(gStringVar1, gText_Stats_SpDefense);
                PrintMonStatsToggle_EV_Arrows(gStringVar2, EVs[5]);
                StringExpandPlaceholders(gStringVar3, gText_Stats_EvStr1Str2);
                PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x + 29*column, base_y + base_y_offset*base_i);
                column++;
            }
        }
        base_i++;
    }

    //TOGGLE--------------------------------------
    if (gTasks[taskId].data[5] == 0)
    {
        u32 catchRate = sPokedexView->sPokemonStats.catchRate;
        u32 growthRate = sPokedexView->sPokemonStats.growthRate;

        //Catch rate
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_CatchRate, base_x, base_y + base_y_offset*base_i);
        if (catchRate <= 10)
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_CatchRate_Legend, base_x + x_offset_column, base_y + base_y_offset*base_i);
        else if (catchRate <= 70)
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_CatchRate_VeryHard, base_x + x_offset_column, base_y + base_y_offset*base_i);
        else if (catchRate <= 100)
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_CatchRate_Difficult, base_x + x_offset_column, base_y + base_y_offset*base_i);
        else if (catchRate <= 150)
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_CatchRate_Medium, base_x + x_offset_column, base_y + base_y_offset*base_i);
        else if (catchRate <= 200)
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_CatchRate_Relaxed, base_x + x_offset_column, base_y + base_y_offset*base_i);
        else
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_CatchRate_Easy, base_x + x_offset_column, base_y + base_y_offset*base_i);
        base_i++;

        //Growth rate
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_Growthrate, base_x, base_y + base_y_offset*base_i);
        switch (growthRate)
        {
        case GROWTH_MEDIUM_FAST:
            StringCopy(strEV, gText_Stats_MEDIUM_FAST);
            break;
        case GROWTH_ERRATIC:
            StringCopy(strEV, gText_Stats_ERRATIC);
            break;
        case GROWTH_FLUCTUATING:
            StringCopy(strEV, gText_Stats_FLUCTUATING);
            break;
        case GROWTH_MEDIUM_SLOW:
            StringCopy(strEV, gText_Stats_MEDIUM_SLOW);
            break;
        case GROWTH_FAST:
            StringCopy(strEV, gText_Stats_FAST);
            break;
        case GROWTH_SLOW:
            StringCopy(strEV, gText_Stats_SLOW);
            break;
        default:
            StringCopy(strEV, gText_ThreeDashes);
            break;
        }
        align_x = GetStringRightAlignXOffset(0, strEV, total_x);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strEV, align_x, base_y + base_y_offset*base_i);
    }
    else
    {
        //Exp Yield
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_ExpYield, base_x, base_y + base_y_offset*base_i);
        ConvertIntToDecimalStringN(gStringVar1, sPokedexView->sPokemonStats.expYield, STR_CONV_MODE_RIGHT_ALIGN, 3);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar1, base_x + base_x_offset, base_y + base_y_offset*base_i);
        base_i++;

        //Friendship
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_Friendship, base_x, base_y + base_y_offset*base_i);
        switch (sPokedexView->sPokemonStats.friendship)
        {
        case 35:
            StringCopy(strEV, gText_Stats_Friendship_BigAnger);
            break;
        case 70:
            StringCopy(strEV, gText_Stats_Friendship_Neutral);
            break;
        case 90:
            StringCopy(strEV, gText_Stats_Friendship_Happy);
            break;
        case 100:
            StringCopy(strEV, gText_Stats_Friendship_Happy);
            break;
        case 140:
            StringCopy(strEV, gText_Stats_Friendship_BigSmile);
            break;
        default:
            ConvertIntToDecimalStringN(strEV, sPokedexView->sPokemonStats.friendship, STR_CONV_MODE_RIGHT_ALIGN, 3);
            break;
        }
        align_x = GetStringRightAlignXOffset(0, strEV, total_x);
        PrintStatsScreenTextSmall(WIN_STATS_LEFT, strEV, align_x, base_y + base_y_offset*base_i);
        base_i++;

        //Egg cycles
        if (sPokedexView->sPokemonStats.eggGroup1 == EGG_GROUP_NO_EGGS_DISCOVERED || sPokedexView->sPokemonStats.eggGroup2 == EGG_GROUP_NO_EGGS_DISCOVERED) //Species without eggs (legendaries etc)
        {
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_EggCycles, base_x, base_y + base_y_offset*base_i);
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_ThreeDashes, 78, base_y + base_y_offset*base_i);
        }
        else
        {
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gText_Stats_EggCycles, base_x, base_y + base_y_offset*base_i);
            if (sPokedexView->sPokemonStats.eggCycles <= 10)
            {
                StringCopy(strEV, gText_Stats_EggCycles_VeryFast);
                align_x = 76;
            }
            else if (sPokedexView->sPokemonStats.eggCycles <= 20)
            {
                StringCopy(strEV, gText_Stats_EggCycles_Fast);
                align_x = 85;
            }
            else if (sPokedexView->sPokemonStats.eggCycles <= 30)
            {
                StringCopy(strEV, gText_Stats_EggCycles_Normal);
                align_x = 76;
            }
            else
            {
                StringCopy(strEV, gText_Stats_EggCycles_Slow);
                align_x = 67;
            }
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, strEV, align_x, base_y + base_y_offset*base_i);
        }
        base_i++;

        //Egg group 1
        switch (sPokedexView->sPokemonStats.eggGroup1)
        {
        case EGG_GROUP_MONSTER     :
            StringCopy(gStringVar1, gText_Stats_eggGroup_MONSTER);
            break;
        case EGG_GROUP_WATER_1     :
            StringCopy(gStringVar1, gText_Stats_eggGroup_WATER_1);
            break;
        case EGG_GROUP_BUG         :
            StringCopy(gStringVar1, gText_Stats_eggGroup_BUG);
            break;
        case EGG_GROUP_FLYING      :
            StringCopy(gStringVar1, gText_Stats_eggGroup_FLYING);
            break;
        case EGG_GROUP_FIELD       :
            StringCopy(gStringVar1, gText_Stats_eggGroup_FIELD);
            break;
        case EGG_GROUP_FAIRY       :
            StringCopy(gStringVar1, gText_Stats_eggGroup_FAIRY);
            break;
        case EGG_GROUP_GRASS       :
            StringCopy(gStringVar1, gText_Stats_eggGroup_GRASS);
            break;
        case EGG_GROUP_HUMAN_LIKE  :
            StringCopy(gStringVar1, gText_Stats_eggGroup_HUMAN_LIKE);
            break;
        case EGG_GROUP_WATER_3     :
            StringCopy(gStringVar1, gText_Stats_eggGroup_WATER_3);
            break;
        case EGG_GROUP_MINERAL     :
            StringCopy(gStringVar1, gText_Stats_eggGroup_MINERAL);
            break;
        case EGG_GROUP_AMORPHOUS   :
            StringCopy(gStringVar1, gText_Stats_eggGroup_AMORPHOUS);
            break;
        case EGG_GROUP_WATER_2     :
            StringCopy(gStringVar1, gText_Stats_eggGroup_WATER_2);
            break;
        case EGG_GROUP_DITTO       :
            StringCopy(gStringVar1, gText_Stats_eggGroup_DITTO);
            break;
        case EGG_GROUP_DRAGON      :
            StringCopy(gStringVar1, gText_Stats_eggGroup_DRAGON);
            break;
        case EGG_GROUP_NO_EGGS_DISCOVERED:
            StringCopy(gStringVar1, gText_Stats_eggGroup_UNDISCOVERED);
            break;
        }
        //Egg group 2
        if (sPokedexView->sPokemonStats.eggGroup1 != sPokedexView->sPokemonStats.eggGroup2)
        {
            switch (sPokedexView->sPokemonStats.eggGroup2)
            {
            case EGG_GROUP_MONSTER     :
                StringCopy(gStringVar2, gText_Stats_eggGroup_MONSTER);
                break;
            case EGG_GROUP_WATER_1     :
                StringCopy(gStringVar2, gText_Stats_eggGroup_WATER_1);
                break;
            case EGG_GROUP_BUG         :
                StringCopy(gStringVar2, gText_Stats_eggGroup_BUG);
                break;
            case EGG_GROUP_FLYING      :
                StringCopy(gStringVar2, gText_Stats_eggGroup_FLYING);
                break;
            case EGG_GROUP_FIELD       :
                StringCopy(gStringVar2, gText_Stats_eggGroup_FIELD);
                break;
            case EGG_GROUP_FAIRY       :
                StringCopy(gStringVar2, gText_Stats_eggGroup_FAIRY);
                break;
            case EGG_GROUP_GRASS       :
                StringCopy(gStringVar2, gText_Stats_eggGroup_GRASS);
                break;
            case EGG_GROUP_HUMAN_LIKE  :
                StringCopy(gStringVar2, gText_Stats_eggGroup_HUMAN_LIKE);
                break;
            case EGG_GROUP_WATER_3     :
                StringCopy(gStringVar2, gText_Stats_eggGroup_WATER_3);
                break;
            case EGG_GROUP_MINERAL     :
                StringCopy(gStringVar2, gText_Stats_eggGroup_MINERAL);
                break;
            case EGG_GROUP_AMORPHOUS   :
                StringCopy(gStringVar2, gText_Stats_eggGroup_AMORPHOUS);
                break;
            case EGG_GROUP_WATER_2     :
                StringCopy(gStringVar2, gText_Stats_eggGroup_WATER_2);
                break;
            case EGG_GROUP_DITTO       :
                StringCopy(gStringVar2, gText_Stats_eggGroup_DITTO);
                break;
            case EGG_GROUP_DRAGON      :
                StringCopy(gStringVar2, gText_Stats_eggGroup_DRAGON);
                break;
            case EGG_GROUP_NO_EGGS_DISCOVERED:
                StringCopy(gStringVar2, gText_Stats_eggGroup_UNDISCOVERED);
                break;
            }
            StringExpandPlaceholders(gStringVar3, gText_Stats_eggGroup_Groups);
            align_x = GetStringRightAlignXOffset(0, gStringVar3, total_x);
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar3, base_x, base_y + base_y_offset*base_i);
        }
        else
        {
            align_x = GetStringRightAlignXOffset(0, gStringVar1, total_x);
            PrintStatsScreenTextSmall(WIN_STATS_LEFT, gStringVar1, base_x, base_y + base_y_offset*base_i);
        }
        base_i++;
    }
}

static void PrintStatsScreen_Abilities(u8 taskId)
{
    u16 species = NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum);
    u8 abilities_x = 5;
    u8 abilities_y = 3;
    u16 ability0;
    u16 ability1;
    u16 abilityHidden;

    //Abilitie(s)
    #ifdef POKEMON_EXPANSION
    if (gTasks[taskId].data[5] == 0)
    {
    #endif
        ability0 = sPokedexView->sPokemonStats.ability0;
        PrintStatsScreenTextSmallWhite(WIN_STATS_ABILITIES, gAbilityNames[ability0], abilities_x, abilities_y);
        PrintStatsScreenTextSmall(WIN_STATS_ABILITIES, gAbilityDescriptionPointers[ability0], abilities_x, abilities_y + 14);

        ability1 = sPokedexView->sPokemonStats.ability1;
        if (ability1 != ABILITY_NONE && ability1 != ability0)
        {
            PrintStatsScreenTextSmallWhite(WIN_STATS_ABILITIES, gAbilityNames[ability1], abilities_x, abilities_y + 30);
            PrintStatsScreenTextSmall(WIN_STATS_ABILITIES, gAbilityDescriptionPointers[ability1], abilities_x, abilities_y + 44);
        }
    #ifdef POKEMON_EXPANSION
    }
    else //Hidden abilities
    {
        abilityHidden = sPokedexView->sPokemonStats.abilityHidden;
        PrintStatsScreenTextSmallWhite(WIN_STATS_ABILITIES, gAbilityNames[abilityHidden], abilities_x, abilities_y);
        PrintStatsScreenTextSmall(WIN_STATS_ABILITIES, gAbilityDescriptionPointers[abilityHidden], abilities_x, abilities_y + 14);
    }
    #endif
}

static void Task_SwitchScreensFromStatsScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
        FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
        FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[3]]); //Destroy item icon
        DestroySprite(&gSprites[gTasks[taskId].data[3]]);       //Destroy item icon
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite

        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        switch (sPokedexView->screenSwitchState)
        {
        case 1:
            FreeAllWindowBuffers();
            InitWindows(sInfoScreen_WindowTemplates);
            gTasks[taskId].func = Task_LoadAreaScreen;
            break;
        case 2:
            gTasks[taskId].func = Task_LoadCryScreen;
            break;
        case 3:
            FreeAllWindowBuffers();
            InitWindows(sInfoScreen_WindowTemplates);
            gTasks[taskId].func = Task_LoadEvolutionScreen;
            break;
        default:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        }
    }
}

static void Task_ExitStatsScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeSpriteTilesByTag(ITEM_TAG);                         //Destroy item icon
        FreeSpritePaletteByTag(ITEM_TAG);                       //Destroy item icon
        FreeSpriteOamMatrix(&gSprites[gTasks[taskId].data[3]]); //Destroy item icon
        DestroySprite(&gSprites[gTasks[taskId].data[3]]);       //Destroy item icon
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite

        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}


//************************************
//*                                  *
//*        EVOS                      *
//*                                  *
//************************************
static const u8 sEvoFormsPageNavigationTextColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY};

static void EvoFormsPage_PrintNavigationButtons(void)
{
    u8 x = 6;
    u8 y = 0;

    FillWindowPixelBuffer(WIN_NAVIGATION_BUTTONS, PIXEL_FILL(0));

    #ifdef POKEMON_EXPANSION
        if (sPokedexView->selectedScreen == EVO_SCREEN)
        {
            if (!HGSS_DECAPPED)
                AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x+9, y, sStatsPageNavigationTextColor, 0, gText_EVO_Buttons_PE);
            else
                AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x+9, y, sStatsPageNavigationTextColor, 0, gText_EVO_Buttons_Decapped_PE);
        }
        else if (sPokedexView->selectedScreen == FORMS_SCREEN)
        {
            if (sPokedexView->sFormScreenData.inSubmenu)
            {
                if (!HGSS_DECAPPED)
                    AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, gText_FORMS_Buttons_Submenu_PE);
                else
                    AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, gText_FORMS_Buttons_Submenu_Decapped_PE);
            }
            else
            {
                if (!HGSS_DECAPPED)
                    AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, gText_FORMS_Buttons_PE);
                else
                    AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x, y, sStatsPageNavigationTextColor, 0, gText_FORMS_Buttons_Decapped_PE);
            }
        }
    #else
        if (!HGSS_DECAPPED)
            AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x+9, y, sStatsPageNavigationTextColor, 0, gText_EVO_Buttons);
        else
            AddTextPrinterParameterized3(WIN_NAVIGATION_BUTTONS, 0, x+9, y, sStatsPageNavigationTextColor, 0, gText_EVO_Buttons_Decapped);
    #endif
    // DrawKeypadIcon(WIN_NAVIGATION_BUTTONS, 10, 5, 0); //(u8 windowId, u8 keypadIconId, u16 x, u16 y)
    PutWindowTilemap(WIN_NAVIGATION_BUTTONS);
    CopyWindowToVram(WIN_NAVIGATION_BUTTONS, 3);
}

static void ResetEvoScreenDataStruct(void)
{
    u8 i;
    sPokedexView->sEvoScreenData.numAllEvolutions = 0;
    sPokedexView->sEvoScreenData.numSeen = 0;
    sPokedexView->sEvoScreenData.menuPos = 0;
    for (i = 0; i < EVO_SCREEN_MAX_ENTRIES; i++)
    {
        sPokedexView->sEvoScreenData.targetSpecies[i] = 0;
        sPokedexView->sEvoScreenData.seen[i] = 0;
    }

}

static void GetSeenFlagTargetSpecies(void)
{
    u8 i;
    u16 species;

    if (sPokedexView->sEvoScreenData.numAllEvolutions > EVO_SCREEN_MAX_ENTRIES)
        sPokedexView->sEvoScreenData.numAllEvolutions = EVO_SCREEN_MAX_ENTRIES;

    for (i = 0; i < sPokedexView->sEvoScreenData.numAllEvolutions; i++)
    {
        species = sPokedexView->sEvoScreenData.targetSpecies[i];
        if (GetPokedexDisplayFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN))
        {
            sPokedexView->sEvoScreenData.seen[i] = TRUE;
            sPokedexView->sEvoScreenData.numSeen += 1;
        }

    }
}

static void Task_LoadEvolutionScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            u16 r2;

            sPokedexView->currentPage = EVO_SCREEN;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            r2 = 0;
            if (gTasks[taskId].data[1] != 0)
                r2 += DISPCNT_OBJ_ON;
            if (gTasks[taskId].data[2] != 0)
                r2 |= DISPCNT_BG1_ON;
            ResetOtherVideoRegisters(r2);
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(EVO_SCREEN);
        FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
        PutWindowTilemap(WIN_INFO);
        CopyWindowToVram(WIN_INFO, 3);
        FillWindowPixelBuffer(WIN_NAVIGATION_BUTTONS, PIXEL_FILL(0));
        PutWindowTilemap(WIN_NAVIGATION_BUTTONS);
        CopyWindowToVram(WIN_NAVIGATION_BUTTONS, 3);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain(0xD);
        LoadPokedexBgPalette(sPokedexView->isSearchResults);
        gMain.state++;
        break;
    case 3:
        if (gTasks[taskId].data[1] == 0)
        {
            sPokedexView->selectedScreen = EVO_SCREEN;
            ResetEvoScreenDataStruct();
            //Icon
            FreeMonIconPalettes(); //Free space for new pallete
            LoadMonIconPalette(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum)); //Loads pallete for current mon
            PrintPreEvolutions(taskId, NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum));
            #ifdef POKEMON_EXPANSION
                gTasks[taskId].data[4] = CreateMonIcon(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), SpriteCB_MonIcon, 18 + 32*sPokedexView->numPreEvolutions, 31, 4, 0); //Create pokemon sprite
            #else
                gTasks[taskId].data[4] = CreateMonIcon(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), SpriteCB_MonIcon, 18 + 32*sPokedexView->numPreEvolutions, 31, 4, 0, TRUE); //Create pokemon sprite
            #endif
            EvoFormsPage_PrintNavigationButtons(); //HGSS_Ui Navigation buttons
            gSprites[gTasks[taskId].data[4]].oam.priority = 0;
        }
        gMain.state++;
        break;
    case 4:
        //Print evo info and icons
        gTasks[taskId].data[3] = 0;
        PrintEvolutionTargetSpeciesAndMethod(taskId, NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), 0, sPokedexView->numPreEvolutions);
        LoadSpritePalette(&gSpritePalette_Arrow);
        GetSeenFlagTargetSpecies();
        if (sPokedexView->sEvoScreenData.numAllEvolutions != 0 && sPokedexView->sEvoScreenData.numSeen != 0)
        {
            u8 pos = 0;

            while (pos < sPokedexView->sEvoScreenData.numAllEvolutions - 1 && !sPokedexView->sEvoScreenData.seen[pos])
                pos++;
            sPokedexView->sEvoScreenData.menuPos = pos;
            sPokedexView->sEvoScreenData.arrowSpriteId = CreateSprite(&gSpriteTemplate_Arrow, 7, 58 + 9 * pos, 0);
            gSprites[sPokedexView->sEvoScreenData.arrowSpriteId].animNum = 2;
        }
        gMain.state++;
        break;
    case 5:
        {
        u32 preservedPalettes = 0;

        if (gTasks[taskId].data[2] != 0)
            preservedPalettes = 0x14; // each bit represents a palette index
        if (gTasks[taskId].data[1] != 0)
            preservedPalettes |= (1 << (gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum + 16));
        BeginNormalPaletteFade(~preservedPalettes, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gMain.state++;
        }
        break;
    case 6:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 7:
        if (!gPaletteFade.active)
            gMain.state++;
        break;
    case 8:
        gMain.state++;
        break;
    case 9:
        sPokedexView->screenSwitchState = 0;
        gTasks[taskId].data[0] = 0;
        gTasks[taskId].data[1] = 0;
        gTasks[taskId].data[2] = 1;
        gTasks[taskId].func = Task_HandleEvolutionScreenInput;
        gMain.state = 0;
        break;
    }
}

static void Task_HandleEvolutionScreenInput(u8 taskId)
{
    //Switch to forms screen, Pokemon Expansion only (rhh)
    #ifdef POKEMON_EXPANSION
    if (JOY_NEW(START_BUTTON))
    {
        sPokedexView->selectedScreen = FORMS_SCREEN;
        BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 3;
        gTasks[taskId].func = Task_SwitchScreensFromEvolutionScreen;
        PlaySE(SE_PIN);
    }
    #endif

    if (sPokedexView->sEvoScreenData.numAllEvolutions != 0 && sPokedexView->sEvoScreenData.numSeen != 0)
    {
        u8 i;
        u8 base_y = 58;
        u8 base_y_offset = 9;
        u8 pos = sPokedexView->sEvoScreenData.menuPos;
        u8 max = sPokedexView->sEvoScreenData.numAllEvolutions;
        if (JOY_NEW(DPAD_DOWN))
        {
            while (TRUE)
            {
                pos += 1;
                if (pos >= max)
                    pos = 0;

                if (sPokedexView->sEvoScreenData.seen[pos] == TRUE)
                    break;
            }
            gSprites[sPokedexView->sEvoScreenData.arrowSpriteId].y = base_y + base_y_offset * pos;
            sPokedexView->sEvoScreenData.menuPos = pos;
        }
        else if (JOY_NEW(DPAD_UP))
        {
            while (TRUE)
            {
                if (pos == 0)
                    pos = max - 1;
                else
                    pos -= 1;

                if (sPokedexView->sEvoScreenData.seen[pos] == TRUE)
                    break;
            }
            gSprites[sPokedexView->sEvoScreenData.arrowSpriteId].y = base_y + base_y_offset * pos;
            sPokedexView->sEvoScreenData.menuPos = pos;
        }

        if (JOY_NEW(A_BUTTON) && sPokedexView->sEvoScreenData.seen[sPokedexView->sEvoScreenData.menuPos])
        {
            u16 targetSpecies   = sPokedexView->sEvoScreenData.targetSpecies[sPokedexView->sEvoScreenData.menuPos];
            u16 dexNum          = SpeciesToNationalPokedexNum(targetSpecies);

            // Use a separate list item so the entry in pokedexList (e.g. a search result) isn't overwritten
            sPokedexView->evoTargetListItem.dexNum = dexNum;
            sPokedexView->evoTargetListItem.seen   = GetPokedexDisplayFlag(dexNum, FLAG_GET_SEEN);
            sPokedexView->evoTargetListItem.owned  = GetPokedexDisplayFlag(dexNum, FLAG_GET_CAUGHT);
            sPokedexListItem = &sPokedexView->evoTargetListItem;

            #ifdef POKEMON_EXPANSION
                if (gFormSpeciesIdTables[targetSpecies] != NULL)
                    sPokedexView->formSpecies = targetSpecies;
                else
                    sPokedexView->formSpecies = 0;
            #endif

            sPokedexView->sEvoScreenData.fromEvoPage = TRUE;
            PlaySE(SE_PIN);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_LoadInfoScreenWaitForFade;
        }
    }

    //Exit to overview
    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_ExitEvolutionScreen;
        PlaySE(SE_PC_OFF);
        return;
    }

    //Switch screens
    if ((JOY_NEW(DPAD_LEFT) || (JOY_NEW(L_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        sPokedexView->selectedScreen = STATS_SCREEN;
        BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 1;
        gTasks[taskId].func = Task_SwitchScreensFromEvolutionScreen;
        PlaySE(SE_PIN);
    }
    if ((JOY_NEW(DPAD_RIGHT) || (JOY_NEW(R_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR)))
    {
        if (!sPokedexListItem->owned)
            PlaySE(SE_FAILURE);
        else
        {
            sPokedexView->selectedScreen = CRY_SCREEN;
            BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
            sPokedexView->screenSwitchState = 2;
            gTasks[taskId].func = Task_SwitchScreensFromEvolutionScreen;
            PlaySE(SE_PIN);
        }
    }
}

static void HandleTargetSpeciesPrint(u8 taskId, u16 targetSpecies, u16 previousTargetSpecies, u8 base_x, u8 base_y, u8 base_y_offset, u8 base_i, bool8 isEevee)
{
    u8 iterations = 6;
    bool8 seen = GetPokedexDisplayFlag(SpeciesToNationalPokedexNum(targetSpecies), FLAG_GET_SEEN);

    if (seen || !HGSS_HIDE_UNSEEN_EVOLUTION_NAMES)
        StringCopy(gStringVar3, gSpeciesNames[targetSpecies]); //evolution mon name
    else
        StringCopy(gStringVar3, gText_ThreeQuestionMarks); //show questionmarks instead of name
    StringExpandPlaceholders(gStringVar3, gText_EVO_Name); //evolution mon name
    PrintInfoScreenTextSmall(gStringVar3, base_x, base_y + base_y_offset*base_i); //evolution mon name

    //Print mon icon in the top row
    if (isEevee)
    {
        iterations = 9;
        if (targetSpecies == previousTargetSpecies)
            return;
        #ifdef POKEMON_EXPANSION
            else if (targetSpecies == SPECIES_GLACEON)
                base_i -= 1;
            else if (targetSpecies == SPECIES_SYLVEON)
                base_i -= 2;
        #endif
    }

    #ifndef POKEMON_EXPANSION
    {
        // A species can be listed more than once (e.g. Eevee -> Espeon by friendship or Sun Stone):
        // only one icon per species, and later icons move left to fill the gap.
        u8 j, numDuplicates = 0;

        for (j = 0; j < base_i && j < EVO_SCREEN_MAX_ENTRIES; j++)
        {
            u8 k;

            if (sPokedexView->sEvoScreenData.targetSpecies[j] == targetSpecies)
                return;
            for (k = 0; k < j; k++)
            {
                if (sPokedexView->sEvoScreenData.targetSpecies[k] == sPokedexView->sEvoScreenData.targetSpecies[j])
                {
                    numDuplicates++;
                    break;
                }
            }
        }
        base_i -= numDuplicates;
    }
    #endif

    if (base_i < iterations)
    {
        LoadMonIconPalette(targetSpecies); //Loads pallete for current mon
        #ifndef POKEMON_EXPANSION
            gTasks[taskId].data[4+base_i] = CreateMonIcon(targetSpecies, SpriteCB_MonIcon, 50 + 32*base_i, 31, 4, 0, TRUE); //Create pokemon sprite
        #endif
        #ifdef POKEMON_EXPANSION
            if (isEevee)
                gTasks[taskId].data[4+base_i] = CreateMonIcon(targetSpecies, SpriteCB_MonIcon, 45 + 26*base_i, 31, 4, 0); //Create pokemon sprite
            else
                gTasks[taskId].data[4+base_i] = CreateMonIcon(targetSpecies, SpriteCB_MonIcon, 50 + 32*base_i, 31, 4, 0); //Create pokemon sprite
        #endif
        gSprites[gTasks[taskId].data[4+base_i]].oam.priority = 0;
    }
}

static void CreateCaughtBallEvolutionScreen(u16 targetSpecies, u8 x, u8 y, u16 unused)
{
    bool8 owned = GetPokedexDisplayFlag(SpeciesToNationalPokedexNum(targetSpecies), FLAG_GET_CAUGHT);
    if (owned)
        BlitBitmapToWindow(0, sCaughtBall_Gfx, x, y-1, 8, 16);
    else
    {
        //FillWindowPixelRect(0, PIXEL_FILL(0), x, y, 8, 16); //not sure why this was even here
        PrintInfoScreenTextSmall(gText_OneDash, x+1, y-1);
    }
}

static void HandlePreEvolutionSpeciesPrint(u8 taskId, u16 preSpecies, u16 species, u8 base_x, u8 base_y, u8 base_y_offset, u8 base_i)
{
    bool8 seen = GetPokedexDisplayFlag(SpeciesToNationalPokedexNum(preSpecies), FLAG_GET_SEEN);

    StringCopy(gStringVar1, gSpeciesNames[species]); //evolution mon name

    #ifdef POKEMON_EXPANSION
    if (sPokedexView->sEvoScreenData.isMega)
        StringExpandPlaceholders(gStringVar3, gText_EVO_PreEvo_PE_Mega);
    else
    {
    #endif

        if (seen || !HGSS_HIDE_UNSEEN_EVOLUTION_NAMES)
            StringCopy(gStringVar2, gSpeciesNames[preSpecies]); //evolution mon name
        else
            StringCopy(gStringVar2, gText_ThreeQuestionMarks); //show questionmarks instead of name

        StringExpandPlaceholders(gStringVar3, gText_EVO_PreEvo); //evolution mon name

    #ifdef POKEMON_EXPANSION
    }
    #endif

    PrintInfoScreenTextSmall(gStringVar3, base_x, base_y + base_y_offset*base_i); //evolution mon name

    if (base_i < 3)
    {
        LoadMonIconPalette(preSpecies); //Loads pallete for current mon
        #ifndef POKEMON_EXPANSION
            gTasks[taskId].data[4+base_i] = CreateMonIcon(preSpecies, SpriteCB_MonIcon, 18 + 32*base_i, 31, 4, 0, TRUE); //Create pokemon sprite
        #endif
        #ifdef POKEMON_EXPANSION
            gTasks[taskId].data[4+base_i] = CreateMonIcon(preSpecies, SpriteCB_MonIcon, 18 + 32*base_i, 31, 4, 0); //Create pokemon sprite
        #endif
        gSprites[gTasks[taskId].data[4+base_i]].oam.priority = 0;
    }
}

static u8 PrintPreEvolutions(u8 taskId, u16 species)
{
    u16 i;
    u16 j;

    u8 base_x = 13+8;
    u8 base_y = 51;
    u8 base_y_offset = 9;
    u8 base_i = 0;
    u8 depth_x = 16;

    u16 preEvolutionOne = 0;
    u16 preEvolutionTwo = 0;
    u8 numPreEvolutions = 0;

    #ifdef POKEMON_EXPANSION
    bool8 isMega = FALSE;
    sPokedexView->sEvoScreenData.isMega = FALSE;
    #endif

    #ifdef TX_RANDOMIZER_AND_CHALLENGES
    if (gSaveBlock1Ptr->tx_Random_Evolutions || gSaveBlock1Ptr->tx_Random_EvolutionMethods)
        return 0;
    #endif


    //Calculate previous evolution
    for (i = 0; i < NUM_SPECIES; i++)
    {
        for (j = 0; j < EVOS_PER_MON; j++)
        {
            if (gEvolutionTable[i][j].targetSpecies == species)
            {
                preEvolutionOne = i;
                numPreEvolutions += 1;
                #ifdef POKEMON_EXPANSION
                    if (gEvolutionTable[i][j].method == EVO_MEGA_EVOLUTION)
                    {
                        CopyItemName(gEvolutionTable[i][j].param, gStringVar2); //item
                        isMega = TRUE;
                    }
                #endif
                break;
            }
        }
    }

    #ifdef POKEMON_EXPANSION
    if (isMega)
    {
        sPokedexView->numPreEvolutions = numPreEvolutions;
        sPokedexView->sEvoScreenData.numAllEvolutions += numPreEvolutions;
        sPokedexView->sEvoScreenData.isMega = isMega;

        CreateCaughtBallEvolutionScreen(preEvolutionOne, base_x - 9 - 8, base_y + base_y_offset*(numPreEvolutions - 1), 0);
        HandlePreEvolutionSpeciesPrint(taskId, preEvolutionOne, species, base_x - 8, base_y, base_y_offset, numPreEvolutions - 1);
        return numPreEvolutions;
    }
    #endif

    //Calculate if previous evolution also has a previous evolution
    if (numPreEvolutions != 0)
    {
        for (i = 0; i < NUM_SPECIES; i++)
        {
            for (j = 0; j < EVOS_PER_MON; j++)
            {
                if (gEvolutionTable[i][j].targetSpecies == preEvolutionOne)
                {
                    preEvolutionTwo = i;
                    numPreEvolutions += 1;
                    CreateCaughtBallEvolutionScreen(preEvolutionTwo, base_x - 9, base_y + base_y_offset*0, 0);
                    HandlePreEvolutionSpeciesPrint(taskId, preEvolutionTwo, preEvolutionOne, base_x, base_y, base_y_offset, 0);
                    break;
                }
            }
        }
    }

    //Print ball and name
    if (preEvolutionOne != 0)
    {
        CreateCaughtBallEvolutionScreen(preEvolutionOne, base_x - 9, base_y + base_y_offset*(numPreEvolutions - 1), 0);
        HandlePreEvolutionSpeciesPrint(taskId, preEvolutionOne, species, base_x, base_y, base_y_offset, numPreEvolutions - 1);
    }

    if (preEvolutionTwo != 0)
    {
        sPokedexView->sEvoScreenData.targetSpecies[0] = preEvolutionTwo;
        sPokedexView->sEvoScreenData.targetSpecies[1] = preEvolutionOne;
    }
    else if (preEvolutionOne != 0)
    {
        sPokedexView->sEvoScreenData.targetSpecies[0] = preEvolutionOne;
    }

    //vertical line
    //FillWindowPixelRect(0, PIXEL_FILL(5), 33 + 32*numPreEvolutions, 18, 1, 32); //PIXEL_FILL(15) for black

    sPokedexView->numPreEvolutions = numPreEvolutions;
    sPokedexView->sEvoScreenData.numAllEvolutions += numPreEvolutions;

    return numPreEvolutions;
}

#define EVO_SCREEN_LVL_DIGITS 2

static u8 PrintEvolutionTargetSpeciesAndMethod(u8 taskId, u16 species, u8 depth, u8 depth_i)
{
    u16 i;
    #ifdef POKEMON_EXPANSION
        u16 j;
        const struct MapHeader *mapHeader;
    #endif
    u16 targetSpecies = 0;
    u16 previousTargetSpecies = 0;

    u16 item;

    bool8 left = TRUE;
    u8 base_x = 13+8;
    u8 base_x_offset = 54+8;
    u8 base_y = 51;
    u8 base_y_offset = 9;
    u8 base_i = 0;
    u8 times = 0;
    u8 depth_x = 16;
    bool8 isEevee = FALSE;

    #ifdef POKEMON_EXPANSION
    if (sPokedexView->sEvoScreenData.isMega)
        return 0;
    #endif

    StringCopy(gStringVar1, gSpeciesNames[species]);

    if (species == SPECIES_EEVEE)
        isEevee = TRUE;

    #ifdef TX_RANDOMIZER_AND_CHALLENGES
    if (EvolutionBlockedByEvoLimit(species)) //No Evos already previously checked
        species = SPECIES_NONE;
    else if (gSaveBlock1Ptr->tx_Random_EvolutionMethods)
        species = GetSpeciesRandomSeeded(species, TX_RANDOM_T_EVO_METH, 0);
    #endif

    //Calculate number of possible direct evolutions (e.g. Eevee has 5 but torchic has 1)
    for (i = 0; i < EVOS_PER_MON; i++)
    {
        #ifndef POKEMON_EXPANSION
            if (gEvolutionTable[species][i].method != 0)
                times += 1;
        #endif
        #ifdef POKEMON_EXPANSION
            if (gEvolutionTable[species][i].method != 0 && gEvolutionTable[species][i].method != EVO_MEGA_EVOLUTION)
                times += 1;
        #endif
    }
    gTasks[taskId].data[3] = times;
    sPokedexView->sEvoScreenData.numAllEvolutions += times;

    //If there are no evolutions print text
    if (times == 0 && depth == 0)
    {
        StringExpandPlaceholders(gStringVar4, gText_EVO_NONE);
        PrintInfoScreenTextSmall(gStringVar4, base_x-7-7, base_y + base_y_offset*depth_i);
    }

    //If there are evolutions find out which and print them 1 by 1
    for (i = 0; i < times; i++)
    {
        base_i = i + depth_i;
        left = !left;

        previousTargetSpecies = targetSpecies;
        targetSpecies = gEvolutionTable[species][i].targetSpecies;
        if (base_i < EVO_SCREEN_MAX_ENTRIES)
            sPokedexView->sEvoScreenData.targetSpecies[base_i] = targetSpecies;
        #ifdef TX_DIFFICULTY_CHALLENGES_USED
            if (gSaveBlock1Ptr->txRandEvolutions && targetSpecies != SPECIES_NONE) //tx_difficulty_challenges
                targetSpecies = GetSpeciesRandomSeeded(targetSpecies, TX_RANDOM_T_EVO, 0);
        #endif
        CreateCaughtBallEvolutionScreen(targetSpecies, base_x + depth_x*depth-9, base_y + base_y_offset*base_i, 0);
        HandleTargetSpeciesPrint(taskId, targetSpecies, previousTargetSpecies, base_x + depth_x*depth, base_y, base_y_offset, base_i, isEevee); //evolution mon name

        switch (gEvolutionTable[species][i].method)
        {
        case EVO_FRIENDSHIP:
            ConvertIntToDecimalStringN(gStringVar2, 220, STR_CONV_MODE_LEADING_ZEROS, 3); //friendship value
            StringExpandPlaceholders(gStringVar4, gText_EVO_FRIENDSHIP );
            break;
        case EVO_FRIENDSHIP_DAY:
            StringExpandPlaceholders(gStringVar4, gText_EVO_FRIENDSHIP_DAY );
            break;
        case EVO_FRIENDSHIP_NIGHT:
            StringExpandPlaceholders(gStringVar4, gText_EVO_FRIENDSHIP_NIGHT );
            break;
        case EVO_LEVEL:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL );
            break;
        case EVO_TRADE:
            StringExpandPlaceholders(gStringVar4, gText_EVO_TRADE );
            break;
        case EVO_TRADE_ITEM:
            item = gEvolutionTable[species][i].param; //item
            CopyItemName(item, gStringVar2); //item
            StringExpandPlaceholders(gStringVar4, gText_EVO_TRADE_ITEM );
            break;
        case EVO_ITEM:
            item = gEvolutionTable[species][i].param;
            CopyItemName(item, gStringVar2);
            StringExpandPlaceholders(gStringVar4, gText_EVO_ITEM );
            break;
        case EVO_LEVEL_ATK_GT_DEF:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_ATK_GT_DEF );
            break;
        case EVO_LEVEL_ATK_EQ_DEF:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_ATK_EQ_DEF );
            break;
        case EVO_LEVEL_ATK_LT_DEF:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_ATK_LT_DEF );
            break;
        case EVO_LEVEL_SILCOON:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_SILCOON );
            break;
        case EVO_LEVEL_CASCOON:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_CASCOON );
            break;
        case EVO_LEVEL_NINJASK:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_NINJASK );
            break;
        case EVO_LEVEL_SHEDINJA:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
            StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_SHEDINJA );
            break;
        case EVO_BEAUTY:
            ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, 3); //beauty
            StringExpandPlaceholders(gStringVar4, gText_EVO_BEAUTY );
            break;
        #ifdef POKEMON_EXPANSION
            case EVO_LEVEL_FEMALE:
                ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
                StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_FEMALE );
                break;
            case EVO_LEVEL_MALE:
                ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
                StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_MALE );
                break;
            case EVO_LEVEL_NIGHT:
                ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
                StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_NIGHT );
                break;
            case EVO_LEVEL_DAY:
                ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
                StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_DAY );
                break;
            case EVO_LEVEL_DUSK:
                ConvertIntToDecimalStringN(gStringVar2, gEvolutionTable[species][i].param, STR_CONV_MODE_LEADING_ZEROS, EVO_SCREEN_LVL_DIGITS); //level
                StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_DUSK );
                break;
            case EVO_ITEM_HOLD_DAY:
                item = gEvolutionTable[species][i].param; //item
                CopyItemName(item, gStringVar2); //item
                StringExpandPlaceholders(gStringVar4, gText_EVO_ITEM_HOLD_DAY );
                break;
            case EVO_ITEM_HOLD_NIGHT:
                item = gEvolutionTable[species][i].param; //item
                CopyItemName(item, gStringVar2); //item
                StringExpandPlaceholders(gStringVar4, gText_EVO_ITEM_HOLD_NIGHT );
                break;
            case EVO_MOVE:
                StringCopy(gStringVar2, gMoveNames[gEvolutionTable[species][i].param]);
                StringExpandPlaceholders(gStringVar4, gText_EVO_MOVE );
                break;
            case EVO_MOVE_TYPE:
                StringCopy(gStringVar2, gTypeNames[gEvolutionTable[species][i].param]);
                StringExpandPlaceholders(gStringVar4, gText_EVO_MOVE_TYPE );
                break;
            case EVO_MAPSEC:
                StringCopy(gStringVar2, gRegionMapEntries[gEvolutionTable[species][i].param].name);
                StringExpandPlaceholders(gStringVar4, gText_EVO_MAPSEC );
                break;
            case EVO_ITEM_MALE:
                item = gEvolutionTable[species][i].param; //item
                CopyItemName(item, gStringVar2); //item
                StringExpandPlaceholders(gStringVar4, gText_EVO_ITEM_MALE );
                break;
            case EVO_ITEM_FEMALE:
                item = gEvolutionTable[species][i].param; //item
                CopyItemName(item, gStringVar2); //item
                StringExpandPlaceholders(gStringVar4, gText_EVO_ITEM_FEMALE );
                break;
            case EVO_LEVEL_RAIN:
                //if (j == WEATHER_RAIN || j == WEATHER_RAIN_THUNDERSTORM || j == WEATHER_DOWNPOUR)
                StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_RAIN );
                break;
            case EVO_SPECIFIC_MON_IN_PARTY:
                StringCopy(gStringVar2, gSpeciesNames[gEvolutionTable[species][i].param]); //mon name
                StringExpandPlaceholders(gStringVar4, gText_EVO_SPECIFIC_MON_IN_PARTY );
                break;
            case EVO_LEVEL_DARK_TYPE_MON_IN_PARTY:
                StringExpandPlaceholders(gStringVar4, gText_EVO_LEVEL_DARK_TYPE_MON_IN_PARTY );
                break;
            case EVO_TRADE_SPECIFIC_MON:
                StringCopy(gStringVar2, gSpeciesNames[gEvolutionTable[species][i].param]); //mon name
                StringExpandPlaceholders(gStringVar4, gText_EVO_TRADE_SPECIFIC_MON );
                break;
            case EVO_SPECIFIC_MAP:
                mapHeader = Overworld_GetMapHeaderByGroupAndId(gEvolutionTable[species][i].param >> 8, gEvolutionTable[species][i].param & 0xFF);
                GetMapName(gStringVar2, mapHeader->regionMapSectionId, 0);
                StringExpandPlaceholders(gStringVar4, gText_EVO_SPECIFIC_MAP );
                break;
        #endif
        default:
            StringExpandPlaceholders(gStringVar4, gText_EVO_UNKNOWN );
            break;
        }//Switch end
        PrintInfoScreenTextSmall(gStringVar4, base_x + depth_x*depth+base_x_offset, base_y + base_y_offset*base_i); //Print actual instructions

        depth_i += PrintEvolutionTargetSpeciesAndMethod(taskId, targetSpecies, depth+1, base_i+1);
    }//For loop end

    return times;
}

static void Task_SwitchScreensFromEvolutionScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        DestroyEvoScreenMonIcons();
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);

        switch (sPokedexView->screenSwitchState)
        {
        case 1:
            gTasks[taskId].func = Task_LoadStatsScreen;
            break;
        case 2:
            gTasks[taskId].func = Task_LoadCryScreen;
            break;
        #ifdef POKEMON_EXPANSION
            case 3:
                gTasks[taskId].func = Task_LoadFormsScreen;
                break;
        #endif
        default:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        }
    }
}

static void Task_ExitEvolutionScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        DestroyEvoScreenMonIcons();
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);

        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}



//************************************
//*                                  *
//*        FORMS                      *
//*                                  *
//************************************
#ifdef POKEMON_EXPANSION
static void Task_LoadFormsScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            u16 r2;

            sPokedexView->currentPage = FORMS_SCREEN;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            r2 = 0;
            if (gTasks[taskId].data[1] != 0)
                r2 += DISPCNT_OBJ_ON;
            if (gTasks[taskId].data[2] != 0)
                r2 |= DISPCNT_BG1_ON;
            ResetOtherVideoRegisters(r2);
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(FORMS_SCREEN);
        FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
        PutWindowTilemap(WIN_INFO);
        CopyWindowToVram(WIN_INFO, 3);
        FillWindowPixelBuffer(WIN_NAVIGATION_BUTTONS, PIXEL_FILL(0));
        PutWindowTilemap(WIN_NAVIGATION_BUTTONS);
        CopyWindowToVram(WIN_NAVIGATION_BUTTONS, 3);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain(0xD);
        LoadPokedexBgPalette(sPokedexView->isSearchResults);
        gMain.state++;
        break;
    case 3:
        if (gTasks[taskId].data[1] == 0)
        {
            //Icon
            FreeMonIconPalettes(); //Free space for new pallete
            LoadMonIconPalette(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum)); //Loads pallete for current mon
            gTasks[taskId].data[4] = CreateMonIcon(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum), SpriteCB_MonIcon, 18, 31, 4, 0); //Create pokemon sprite
            gSprites[gTasks[taskId].data[4]].oam.priority = 0;
        }
        EvoFormsPage_PrintNavigationButtons(); //HGSS_Ui Navigation buttons
        sPokedexView->sFormScreenData.menuPos = 1;
        gMain.state++;
        break;
    case 4:
        //Print form icons
        gTasks[taskId].data[3] = 0;
        PrintForms(taskId, NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum));
        LoadSpritePalette(&gSpritePalette_Arrow);
        gMain.state++;
        break;
    case 5:
        {
        u32 preservedPalettes = 0;

        if (gTasks[taskId].data[2] != 0)
            preservedPalettes = 0x14; // each bit represents a palette index
        if (gTasks[taskId].data[1] != 0)
            preservedPalettes |= (1 << (gSprites[gTasks[taskId].tMonSpriteId].oam.paletteNum + 16));
        BeginNormalPaletteFade(~preservedPalettes, 0, 16, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gMain.state++;
        }
        break;
    case 6:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 7:
        if (!gPaletteFade.active)
            gMain.state++;
        break;
    case 8:
        gMain.state++;
        break;
    case 9:
        sPokedexView->screenSwitchState = 0;
        gTasks[taskId].data[0] = 0;
        gTasks[taskId].data[1] = 0;
        gTasks[taskId].data[2] = 1;
        gTasks[taskId].func = Task_HandleFormsScreenInput;
        gMain.state = 0;
        break;
    }
}

static void Task_HandleFormsScreenInput(u8 taskId)
{
    u8 base_x = 5;
    u8 base_y = 34;
    u8 offset_x = 34;
    u8 offset_y = 34;
    if (!sPokedexView->sFormScreenData.inSubmenu)
    {
        if (JOY_NEW(A_BUTTON) && sPokedexView->sFormScreenData.numForms != 0)
        {
            sPokedexView->sFormScreenData.inSubmenu = TRUE;
            sPokedexView->sFormScreenData.arrowSpriteId = CreateSprite(&gSpriteTemplate_Arrow, base_x + offset_x, base_y, 0);
            gSprites[sPokedexView->sFormScreenData.arrowSpriteId].animNum = 2;
            EvoFormsPage_PrintNavigationButtons();
        }

        if (JOY_NEW(START_BUTTON))
        {
            sPokedexView->selectedScreen = EVO_SCREEN;
            BeginNormalPaletteFade(0xFFFFFFEB, 0, 0, 0x10, RGB_BLACK);
            sPokedexView->screenSwitchState = 1;
            gTasks[taskId].func = Task_SwitchScreensFromFormsScreen;
            PlaySE(SE_PIN);
        }

        //Exit to overview
        if (JOY_NEW(B_BUTTON))
        {
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_ExitFormsScreen;
            PlaySE(SE_PC_OFF);
            return;
        }
    }
    else //inSubmenu
    {
        u8 row = 0;
        u8 column = 0;
        u8 menuPos = sPokedexView->sFormScreenData.menuPos;
        //Grid navigation
        if (JOY_NEW(DPAD_RIGHT))
            menuPos += 1;
        else if (JOY_NEW(DPAD_LEFT))
            menuPos -= 1;
        else if (JOY_NEW(DPAD_DOWN))
            menuPos += 7;
        else if (JOY_NEW(DPAD_UP))
        {
            if (menuPos > 7)
                menuPos -= 7;
            else
                menuPos = 1;
        }

        if (menuPos <= 0)
            menuPos = 1;
        else if (menuPos >= sPokedexView->sFormScreenData.numForms)
            menuPos = sPokedexView->sFormScreenData.numForms;

        row = menuPos / 7;
        column = menuPos % 7;
        gSprites[sPokedexView->sFormScreenData.arrowSpriteId].x = base_x + offset_x * column;
        gSprites[sPokedexView->sFormScreenData.arrowSpriteId].y = base_y + offset_y * row;

        sPokedexView->sFormScreenData.menuPos = menuPos;


        if (JOY_NEW(A_BUTTON))
        {
            u8 formId = sPokedexView->sFormScreenData.formIds[menuPos];
            u16 formSpecies = GetFormSpeciesId(DexNumToSpecies(sPokedexListItem->dexNum), formId);
            if (formSpecies == GetFormSpeciesId(formSpecies, 0))
                sPokedexView->formSpecies = 0;
            else
                sPokedexView->formSpecies = formSpecies;

            sPokedexView->sEvoScreenData.fromEvoPage = TRUE;
            sPokedexView->sFormScreenData.inSubmenu = FALSE;
            PlaySE(SE_PIN);
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_LoadInfoScreenWaitForFade;
        }

        if (JOY_NEW(B_BUTTON))
        {
            DestroySprite(&gSprites[sPokedexView->sFormScreenData.arrowSpriteId]);
            sPokedexView->sFormScreenData.inSubmenu = FALSE;
            EvoFormsPage_PrintNavigationButtons();
        }
    }
}

#define FORM_SPECIES_END (0xffff)

static void PrintForms(u8 taskId, u16 species)
{
    u8 i;
    u8 j = 1;
    u16 speciesForm;


    bool8 left = TRUE;
    u8 base_x = 5;
    u8 base_x_offset = 54;
    u8 base_y = 52;
    u8 base_y_offset = 9;
    u8 base_i = 0;
    u8 times = 0;
    u8 y_offset_icons = 0; //For unown only

    if (species == SPECIES_UNOWN)
        y_offset_icons = 8;

    if (GetFormSpeciesId(species, 0) == SPECIES_UNOWN)
        y_offset_icons = 8;

    StringCopy(gStringVar1, gSpeciesNames[species]);

    for (i=0; i < 30; i++)
    {
        speciesForm = GetFormSpeciesId(species, i);
        if (speciesForm == FORM_SPECIES_END)
            break;
        else if (speciesForm == species)
            continue;
        else
        {
            sPokedexView->sFormScreenData.formIds[j++] = i;
            times += 1;
            LoadMonIconPalette(speciesForm); //Loads pallete for current mon
            if (times < 7)
                gTasks[taskId].data[4+times] = CreateMonIcon(speciesForm, SpriteCB_MonIcon, 52 + 34*(times-1), 31, 4, 0); //Create pokemon sprite
            else if (times < 14)
                gTasks[taskId].data[4+times] = CreateMonIcon(speciesForm, SpriteCB_MonIcon, 18 + 34*(times-7), 70 - y_offset_icons, 4, 0); //Create pokemon sprite
            else if (times < 21)
                gTasks[taskId].data[4+times] = CreateMonIcon(speciesForm, SpriteCB_MonIcon, 18 + 34*(times-14), 104 - y_offset_icons, 4, 0); //Create pokemon sprite
            else
                gTasks[taskId].data[4+times] = CreateMonIcon(speciesForm, SpriteCB_MonIcon, 18 + 34*(times-21), 138 - y_offset_icons, 4, 0); //Create pokemon sprite
            gSprites[gTasks[taskId].data[4+times]].oam.priority = 0;
        }
    }
    gTasks[taskId].data[3] = times;
    sPokedexView->sFormScreenData.numForms = times;

    //If there are no forms print text
    if (times == 0)
    {
        StringExpandPlaceholders(gStringVar4, gText_FORMS_NONE);
        PrintInfoScreenTextSmall(gStringVar4, base_x, base_y + base_y_offset*times);
    }
}

static void Task_SwitchScreensFromFormsScreen(u8 taskId)
{
    u8 i;
    if (!gPaletteFade.active)
    {
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite
        for (i = 1; i <= gTasks[taskId].data[3]; i++)
        {
            FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4+i]]); //Destroy pokemon icon sprite
        }
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);

        switch (sPokedexView->screenSwitchState)
        {
        case 1:
            gTasks[taskId].func = Task_LoadEvolutionScreen;
            break;
        default:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        }
    }
}

static void Task_ExitFormsScreen(u8 taskId)
{
    u8 i;
    if (!gPaletteFade.active)
    {
        FreeMonIconPalettes();                                          //Destroy pokemon icon sprite
        FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4]]); //Destroy pokemon icon sprite
        for (i = 1; i <= gTasks[taskId].data[3]; i++)
        {
            FreeAndDestroyMonIconSprite(&gSprites[gTasks[taskId].data[4+i]]); //Destroy pokemon icon sprite
        }
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);

        FreeInfoScreenWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}
#endif

#undef tMonSpriteId


//************************************
//*                                  *
//*        Cry screen                *
//*                                  *
//************************************
#define tScrolling       data[0]
#define tMonSpriteDone   data[1]
#define tBgLoaded        data[2]
#define tSkipCry         data[3]
#define tMonSpriteId     data[4]
#define tTrainerSpriteId data[5]

static void Task_LoadCryScreen(u8 taskId)
{
    switch (gMain.state)
    {
    case 0:
    default:
        if (!gPaletteFade.active)
        {
            m4aMPlayStop(&gMPlayInfo_BGM);
            sPokedexView->currentPage = PAGE_CRY;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            ResetOtherVideoRegisters(DISPCNT_BG1_ON);
            sPokedexView->selectedScreen = CRY_SCREEN;
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(CRY_SCREEN);
        FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
        PutWindowTilemap(WIN_INFO);
        PutWindowTilemap(WIN_VU_METER);
        PutWindowTilemap(WIN_CRY_WAVE);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain(0xD); //HGSS_Ui
        LoadPokedexBgPalette(sPokedexView->isSearchResults);
        gMain.state++;
        break;
    case 3:
        ResetPaletteFade();
        gMain.state++;
        break;
    case 4:
        PrintInfoScreenText(gText_CryOf, 82, 33);
        PrintCryScreenSpeciesName(0, sPokedexListItem->dexNum, 82, 49);
        gMain.state++;
        break;
    case 5:
        gTasks[taskId].tMonSpriteId = CreateMonSpriteFromNationalDexNumber(sPokedexListItem->dexNum, MON_PAGE_X, MON_PAGE_Y, 0);
        gSprites[gTasks[taskId].tMonSpriteId].oam.priority = 0;
        gDexCryScreenState = 0;
        gMain.state++;
        break;
    case 6:
        {
            struct CryScreenWindow waveformWindow;

            waveformWindow.unk0 = 0x4020;
            waveformWindow.unk2 = 31;
            waveformWindow.paletteNo = 8;
            waveformWindow.yPos = 30;
            waveformWindow.xPos = 12;
            if (LoadCryWaveformWindow(&waveformWindow, 2))
            {
                gMain.state++;
                gDexCryScreenState = 0;
            }
        }
        break;
    case 7:
        {
            struct CryScreenWindow cryMeter;

            cryMeter.paletteNo = 9;
            cryMeter.xPos = 18;
            cryMeter.yPos = 3;
            if (LoadCryMeter(&cryMeter, 3))
                gMain.state++;
            CopyWindowToVram(WIN_VU_METER, COPYWIN_GFX);
            CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
            CopyBgTilemapBufferToVram(0);
            CopyBgTilemapBufferToVram(1);
            CopyBgTilemapBufferToVram(2);
            CopyBgTilemapBufferToVram(3);
        }
        break;
    case 8:
        BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gMain.state++;
        break;
    case 9:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 10:
        sPokedexView->screenSwitchState = 0;
        gMain.state = 0;
        gTasks[taskId].func = Task_HandleCryScreenInput;
        break;
    }
}

static void Task_HandleCryScreenInput(u8 taskId)
{
    UpdateCryWaveformWindow(2);

    if (IsCryPlaying())
        LoadPlayArrowPalette(TRUE);
    else
        LoadPlayArrowPalette(FALSE);

    if (JOY_NEW(A_BUTTON))
    {
        LoadPlayArrowPalette(TRUE);
        CryScreenPlayButton(NationalPokedexNumToSpeciesHGSS(sPokedexListItem->dexNum));
        return;
    }
    else if (!gPaletteFade.active)
    {
        if (JOY_NEW(B_BUTTON))
        {
            BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0, 0x10, RGB_BLACK);
            m4aMPlayContinue(&gMPlayInfo_BGM);
            sPokedexView->screenSwitchState = 1;
            gTasks[taskId].func = Task_SwitchScreensFromCryScreen;
            PlaySE(SE_PC_OFF);
            return;
        }
        if (JOY_NEW(DPAD_LEFT)
         || (JOY_NEW(L_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR))
        {
            BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0, 0x10, RGB_BLACK);
            m4aMPlayContinue(&gMPlayInfo_BGM);
            sPokedexView->screenSwitchState = 2;
            gTasks[taskId].func = Task_SwitchScreensFromCryScreen;
            PlaySE(SE_DEX_PAGE);
            return;
        }
        if (JOY_NEW(DPAD_RIGHT)
         || (JOY_NEW(R_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR))
        {
            if (!sPokedexListItem->owned)
            {
                PlaySE(SE_FAILURE);
            }
            else
            {
                BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0, 0x10, RGB_BLACK);
                m4aMPlayContinue(&gMPlayInfo_BGM);
                sPokedexView->screenSwitchState = 3;
                gTasks[taskId].func = Task_SwitchScreensFromCryScreen;
                PlaySE(SE_DEX_PAGE);
            }
            return;
        }
    }
}

static void Task_SwitchScreensFromCryScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeCryScreen();
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        switch (sPokedexView->screenSwitchState)
        {
        default:
        case 1:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        case 2:
            gTasks[taskId].func = Task_LoadEvolutionScreen;
            break;
        case 3:
            gTasks[taskId].func = Task_LoadSizeScreen;
            break;
        }
    }
}



//************************************
//*                                  *
//*        Size screen               *
//*                                  *
//************************************
static void Task_LoadSizeScreen(u8 taskId)
{
    u8 spriteId;

    switch (gMain.state)
    {
    default:
    case 0:
        if (!gPaletteFade.active)
        {
            sPokedexView->currentPage = PAGE_SIZE;
            gPokedexVBlankCB = gMain.vblankCallback;
            SetVBlankCallback(NULL);
            ResetOtherVideoRegisters(DISPCNT_BG1_ON);
            sPokedexView->selectedScreen = SIZE_SCREEN;
            gMain.state = 1;
        }
        break;
    case 1:
        LoadTilesetTilemapHGSS(SIZE_SCREEN);
        FillWindowPixelBuffer(WIN_INFO, PIXEL_FILL(0));
        PutWindowTilemap(WIN_INFO);
        gMain.state++;
        break;
    case 2:
        LoadScreenSelectBarMain(0xD); //HGSS_Ui
        LoadPokedexBgPalette(sPokedexView->isSearchResults);
        gMain.state++;
        break;
    case 3:
        {
            u8 string[64];

            StringCopy(string, gText_SizeComparedTo);
            StringAppend(string, gSaveBlock2Ptr->playerName);
            PrintInfoScreenText(string, GetStringCenterAlignXOffset(FONT_NORMAL, string, DISPLAY_WIDTH), 121);
            gMain.state++;
        }
        break;
    case 4:
        ResetPaletteFade();
        gMain.state++;
        break;
    case 5:
        spriteId = CreateSizeScreenTrainerPic(PlayerGenderToFrontTrainerPicId(gSaveBlock2Ptr->playerGender), 152, 56, 0);
        gSprites[spriteId].oam.affineMode = ST_OAM_AFFINE_NORMAL;
        gSprites[spriteId].oam.matrixNum = 1;
        gSprites[spriteId].oam.priority = 0;
        gSprites[spriteId].y2 = gPokedexEntries[sPokedexListItem->dexNum].trainerOffset;
        SetOamMatrix(1, gPokedexEntries[sPokedexListItem->dexNum].trainerScale, 0, 0, gPokedexEntries[sPokedexListItem->dexNum].trainerScale);
        LoadPalette(sSizeScreenSilhouette_Pal, OBJ_PLTT_ID2(gSprites[spriteId].oam.paletteNum), PLTT_SIZE_4BPP);
        gTasks[taskId].tTrainerSpriteId = spriteId;
        gMain.state++;
        break;
    case 6:
        spriteId = CreateMonSpriteFromNationalDexNumber(sPokedexListItem->dexNum, 88, 56, 1);
        gSprites[spriteId].oam.affineMode = ST_OAM_AFFINE_NORMAL;
        gSprites[spriteId].oam.matrixNum = 2;
        gSprites[spriteId].oam.priority = 0;
        gSprites[spriteId].y2 = gPokedexEntries[sPokedexListItem->dexNum].pokemonOffset;
        SetOamMatrix(2, gPokedexEntries[sPokedexListItem->dexNum].pokemonScale, 0, 0, gPokedexEntries[sPokedexListItem->dexNum].pokemonScale);
        LoadPalette(sSizeScreenSilhouette_Pal, OBJ_PLTT_ID2(gSprites[spriteId].oam.paletteNum), PLTT_SIZE_4BPP);
        gTasks[taskId].tMonSpriteId = spriteId;
        CopyWindowToVram(WIN_INFO, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 7:
        BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(gPokedexVBlankCB);
        gMain.state++;
        break;
    case 8:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 9:
        if (!gPaletteFade.active)
        {
            sPokedexView->screenSwitchState = 0;
            gMain.state = 0;
            gTasks[taskId].func = Task_HandleSizeScreenInput;
        }
        break;
    }
}

static void LoadPlayArrowPalette(bool8 cryPlaying)
{
    u16 color;

    if (cryPlaying)
        color = RGB(18, 28, 0);
    else
        color = RGB(15, 21, 0);
    LoadPalette(&color, BG_PLTT_ID(5) + 13, PLTT_SIZEOF(1));
}

static void Task_HandleSizeScreenInput(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 1;
        gTasks[taskId].func = Task_SwitchScreensFromSizeScreen;
        PlaySE(SE_PC_OFF);
    }
    else if (JOY_NEW(DPAD_LEFT)
     || (JOY_NEW(L_BUTTON) && gSaveBlock2Ptr->optionsButtonMode == OPTIONS_BUTTON_MODE_LR))
    {
        BeginNormalPaletteFade(PALETTES_ALL & ~(0x14), 0, 0, 0x10, RGB_BLACK);
        sPokedexView->screenSwitchState = 2;
        gTasks[taskId].func = Task_SwitchScreensFromSizeScreen;
        PlaySE(SE_DEX_PAGE);
    }
}

static void Task_SwitchScreensFromSizeScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeAndDestroyMonPicSprite(gTasks[taskId].tMonSpriteId);
        FreeAndDestroyTrainerPicSprite(gTasks[taskId].tTrainerSpriteId);
        switch (sPokedexView->screenSwitchState)
        {
        default:
        case 1:
            gTasks[taskId].func = Task_LoadInfoScreen;
            break;
        case 2:
            gTasks[taskId].func = Task_LoadCryScreen;
            break;
        }
    }
}

#undef tScrolling
#undef tMonSpriteDone
#undef tBgLoaded
#undef tSkipCry
#undef tMonSpriteId
#undef tTrainerSpriteId



//************************************
//*                                  *
//*        Local area screen         *
//*                                  *
//************************************
// Every wild Pokémon available on the player's current map, one encounter method per page.
// Left/Right change the method, Up/Down the time of day. Unseen Pokémon are shown as ???.

#define LOCAL_VISIBLE_ROWS  4
#define LOCAL_MAX_ENTRIES   24
#define LOCAL_ROW_HEIGHT    24
#define LOCAL_LIST_Y        44
#define LOCAL_BAR_X         8
#define LOCAL_BAR_WIDTH     212
#define LOCAL_ICON_X        24
#define LOCAL_BALL_X        42
#define LOCAL_NAME_X        52
#define LOCAL_LEVEL_RIGHT   204
#define LOCAL_MARKER_X      208
#define LOCAL_ARROW_X       224
#define LOCAL_SHADE_SWARM   14 // tan
#define LOCAL_SHADE_SPECIAL 12 // green
#define LOCAL_FILL          10 // the list's yellow
// Button badges are drawn into the background art by tech/pokedex_hgss_reskin.py (LOCAL_BADGES)
#define LOCAL_L_BADGE_X     138
#define LOCAL_R_BADGE_X     220
#define LOCAL_BADGE_Y       6
#define LOCAL_TIME_ICON_X   160
#define LOCAL_TIME_ICON_Y   12
#define LOCAL_TIME_TEXT_X   170
#define TAG_LOCAL_TIME_ICON 30020

enum
{
    LOCAL_LAND,
    LOCAL_WATER,
    LOCAL_ROCK_SMASH,
    LOCAL_OLD_ROD,
    LOCAL_GOOD_ROD,
    LOCAL_SUPER_ROD,
    LOCAL_GROTTO,
    LOCAL_METHOD_COUNT
};

enum
{
    LOCAL_TAG_NONE,
    LOCAL_TAG_SWARM,
    LOCAL_TAG_SPECIAL, // Feebas fishing spots
};

struct LocalDexEntry
{
    u16 species;
    u8 minLevel;
    u8 maxLevel;
    u8 tag;
};

struct LocalDex
{
    const struct WildPokemonHeader *header;
    u8 methods[LOCAL_METHOD_COUNT];
    u8 methodCount;
    u8 page;
    u8 timeOfDay;
    u8 entryCount;
    u8 cursor;
    u8 scrollTop;
    u8 timeSpriteId;
    u8 iconSpriteIds[LOCAL_VISIBLE_ROWS];
    struct LocalDexEntry entries[LOCAL_MAX_ENTRIES];
};

// Allocated while the local screen (or a Pokédex page opened from it) is showing
static EWRAM_DATA struct LocalDex *sLocalDex = NULL;

static const u8 sText_LocalGrass[] = _("GRASS");
static const u8 sText_LocalCave[] = _("CAVE");
static const u8 sText_LocalSurfing[] = _("SURFING");
static const u8 sText_LocalRockSmash[] = _("ROCK SMASH");
static const u8 sText_LocalOldRod[] = _("OLD ROD");
static const u8 sText_LocalGoodRod[] = _("GOOD ROD");
static const u8 sText_LocalSuperRod[] = _("SUPER ROD");
static const u8 sText_LocalGrotto[] = _("GROTTO");
static const u8 sText_LocalMorning[] = _("MORNING");
static const u8 sText_LocalDay[] = _("DAY");
static const u8 sText_LocalNight[] = _("NIGHT");
static const u8 sText_LocalAnyTime[] = _("ANY TIME");
static const u8 sText_LocalUp[] = _("{UP_ARROW}");
static const u8 sText_LocalDown[] = _("{DOWN_ARROW}");
static const u8 sText_LocalLeft[] = _("{LEFT_ARROW}");
static const u8 sText_LocalRight[] = _("{RIGHT_ARROW}");
static const u8 sText_LocalUnseen[] = _("???");
static const u8 sText_LocalLv[] = _("{LV}");
static const u8 sText_LocalDash[] = _("-");
static const u8 sText_LocalSlash[] = _("/");
static const u8 sText_LocalSwarm[] = _("SWARM");
static const u8 sText_LocalSpecial[] = _("SPECIAL");
static const u8 sText_LocalEntry[] = _("ENTRY");
static const u8 sText_LocalBack[] = _("BACK");
static const u8 sText_LocalNoWildMons[] = _("No wild POKéMON live here.");
static const u8 sText_LocalNoneNow[] = _("None appear at this time.");

static const u8 sLocalTextColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_DYNAMIC_COLOR_6, TEXT_COLOR_LIGHT_GRAY};
static const u8 sLocalFooterTextColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_GREEN};

static const struct SpriteSheet sLocalTimeIconSheet = {gPokedexPlusHGSS_TimeIcons_Gfx, POKEDEX_TIME_ICONS_SIZE, TAG_LOCAL_TIME_ICON};
static const struct SpritePalette sLocalTimeIconPalette = {gPokedexPlusHGSS_TimeIcons_Pal, TAG_LOCAL_TIME_ICON};

static const struct OamData sOamData_LocalTimeIcon =
{
    .shape = SPRITE_SHAPE(16x16),
    .size = SPRITE_SIZE(16x16),
    .priority = 1,
};

static const union AnimCmd sAnim_LocalTimeMorning[] = {ANIMCMD_FRAME(0, 1), ANIMCMD_END};
static const union AnimCmd sAnim_LocalTimeDay[] = {ANIMCMD_FRAME(4, 1), ANIMCMD_END};
static const union AnimCmd sAnim_LocalTimeNight[] = {ANIMCMD_FRAME(8, 1), ANIMCMD_END};

static const union AnimCmd *const sAnims_LocalTimeIcon[] =
{
    [TIME_OF_DAY_NIGHT] = sAnim_LocalTimeNight,
    [TIME_OF_DAY_MORNING] = sAnim_LocalTimeMorning,
    [TIME_OF_DAY_DAY] = sAnim_LocalTimeDay,
};

static const struct SpriteTemplate sSpriteTemplate_LocalTimeIcon =
{
    .tileTag = TAG_LOCAL_TIME_ICON,
    .paletteTag = TAG_LOCAL_TIME_ICON,
    .oam = &sOamData_LocalTimeIcon,
    .anims = sAnims_LocalTimeIcon,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct WindowTemplate sLocalScreen_WindowTemplates[] =
{
    {
        .bg = 2,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 20,
        .paletteNum = 0,
        .baseBlock = 1,
    },
    DUMMY_WIN_TEMPLATE
};

static void Task_LoadLocalScreen(u8 taskId);
static void Task_HandleLocalScreenInput(u8 taskId);
static void Task_ExitLocalScreen(u8 taskId);
static void Task_OpenInfoFromLocalScreen(u8 taskId);
static void Task_WaitForExitLocalInfoScreen(u8 taskId);

static bool8 IsOutbreakOnCurrentMap(u8 encounterType)
{
    return gSaveBlock1Ptr->outbreakPokemonSpecies != SPECIES_NONE
        && gSaveBlock1Ptr->outbreakEncounterType == encounterType
        && gSaveBlock1Ptr->outbreakLocationMapNum == gSaveBlock1Ptr->location.mapNum
        && gSaveBlock1Ptr->outbreakLocationMapGroup == gSaveBlock1Ptr->location.mapGroup;
}

static bool8 IsOnCurrentMap(u8 mapGroup, u8 mapNum)
{
    return mapGroup == gSaveBlock1Ptr->location.mapGroup && mapNum == gSaveBlock1Ptr->location.mapNum;
}

static void LocalDex_AddEntry(u16 species, u8 minLevel, u8 maxLevel, u8 tag);
static void LocalDex_AddOutbreak(u8 encounterType);

// Every swarm that can happen on this map this season (shaded), plus the one running now.
// Returns whether there are any.
static bool8 LocalDex_AddSwarms(u8 encounterType, bool8 add)
{
    u16 i, species;
    u8 level, mapGroup, mapNum, type;
    bool8 found = IsOutbreakOnCurrentMap(encounterType);

    if (found && add)
        LocalDex_AddOutbreak(encounterType);
    for (i = 0; GetPokedexSwarmCandidate(i, &species, &level, &mapGroup, &mapNum, &type); i++)
    {
        if (species == SPECIES_NONE || type != encounterType || !IsOnCurrentMap(mapGroup, mapNum))
            continue;
        found = TRUE;
        if (add)
            LocalDex_AddEntry(species, level, level, LOCAL_TAG_SWARM);
    }
    return found;
}

// Pokémon that this map's hidden grottos can hold this season. Returns whether there are any.
static bool8 LocalDex_AddGrottoMons(bool8 add)
{
    u16 i, species;
    u8 level, mapGroup, mapNum;
    bool8 found = FALSE;

    for (i = 0; GetPokedexGrottoMonCandidate(i, &species, &level, &mapGroup, &mapNum); i++)
    {
        if (species == SPECIES_NONE || !IsOnCurrentMap(mapGroup, mapNum))
            continue;
        found = TRUE;
        if (add)
            LocalDex_AddEntry(species, level, level, LOCAL_TAG_NONE);
    }
    return found;
}

static void LocalDex_FindMethods(void)
{
    const struct WildPokemonHeader *header = GetCurrentMapWildMonHeader();
    bool8 hasRods;

    sLocalDex->header = header;
    sLocalDex->methodCount = 0;
    if ((header != NULL && header->landMonsInfo != NULL) || LocalDex_AddSwarms(OUTBREAK_LAND, FALSE))
        sLocalDex->methods[sLocalDex->methodCount++] = LOCAL_LAND;
    if ((header != NULL && header->waterMonsInfo != NULL) || LocalDex_AddSwarms(OUTBREAK_SURF, FALSE))
        sLocalDex->methods[sLocalDex->methodCount++] = LOCAL_WATER;
    if (header != NULL && header->rockSmashMonsInfo != NULL)
        sLocalDex->methods[sLocalDex->methodCount++] = LOCAL_ROCK_SMASH;
    hasRods = (header != NULL && header->fishingMonsInfo != NULL)
           || LocalDex_AddSwarms(OUTBREAK_FISHING, FALSE) || GetCurrentMapFeebas() != NULL;
    if (hasRods)
    {
        sLocalDex->methods[sLocalDex->methodCount++] = LOCAL_OLD_ROD;
        sLocalDex->methods[sLocalDex->methodCount++] = LOCAL_GOOD_ROD;
        sLocalDex->methods[sLocalDex->methodCount++] = LOCAL_SUPER_ROD;
    }
    if (LocalDex_AddGrottoMons(FALSE))
        sLocalDex->methods[sLocalDex->methodCount++] = LOCAL_GROTTO;
}

static void LocalDex_AddEntry(u16 species, u8 minLevel, u8 maxLevel, u8 tag)
{
    u8 i;
    struct LocalDexEntry *entry;

    for (i = 0; i < sLocalDex->entryCount; i++)
    {
        entry = &sLocalDex->entries[i];
        if (entry->species == species)
        {
            if (minLevel < entry->minLevel)
                entry->minLevel = minLevel;
            if (maxLevel > entry->maxLevel)
                entry->maxLevel = maxLevel;
            if (tag > entry->tag)
                entry->tag = tag;
            return;
        }
    }
    if (sLocalDex->entryCount >= LOCAL_MAX_ENTRIES)
        return;
    entry = &sLocalDex->entries[sLocalDex->entryCount++];
    entry->species = species;
    entry->minLevel = minLevel;
    entry->maxLevel = maxLevel;
    entry->tag = tag;
}

static void LocalDex_AddSlots(const struct WildPokemonInfo *info, u8 first, u8 count, bool8 checkTime)
{
    u8 i;

    if (info == NULL)
        return;
    for (i = first; i < first + count; i++)
    {
        const struct WildPokemon *mon = &info->wildPokemon[i];

        if (mon->species == SPECIES_NONE)
            continue;
        if (checkTime && !IsWildSpeciesAvailableAtTime(mon->species, sLocalDex->timeOfDay))
            continue;
        LocalDex_AddEntry(mon->species, mon->minLevel, mon->maxLevel, LOCAL_TAG_NONE);
    }
}

static void LocalDex_AddOutbreak(u8 encounterType)
{
    if (IsOutbreakOnCurrentMap(encounterType))
        LocalDex_AddEntry(gSaveBlock1Ptr->outbreakPokemonSpecies, gSaveBlock1Ptr->outbreakPokemonLevel,
                          gSaveBlock1Ptr->outbreakPokemonLevel, LOCAL_TAG_SWARM);
}

// Mirrors the encounter code: land, water and rock smash slots respect the species' time of day;
// fishing picks its slot without a time check. Swarms appear at any time.
static void LocalDex_BuildEntries(void)
{
    const struct WildPokemonHeader *header = sLocalDex->header;
    const struct WildPokemon *feebas = GetCurrentMapFeebas();

    sLocalDex->entryCount = 0;
    if (sLocalDex->methodCount == 0)
        return;
    switch (sLocalDex->methods[sLocalDex->page])
    {
    case LOCAL_LAND:
        LocalDex_AddSwarms(OUTBREAK_LAND, TRUE);
        LocalDex_AddSlots(header ? header->landMonsInfo : NULL, 0, LAND_WILD_COUNT, TRUE);
        break;
    case LOCAL_WATER:
        LocalDex_AddSwarms(OUTBREAK_SURF, TRUE);
        LocalDex_AddSlots(header ? header->waterMonsInfo : NULL, 0, WATER_WILD_COUNT, TRUE);
        break;
    case LOCAL_ROCK_SMASH:
        LocalDex_AddSlots(header ? header->rockSmashMonsInfo : NULL, 0, ROCK_WILD_COUNT, TRUE);
        break;
    case LOCAL_GROTTO:
        LocalDex_AddGrottoMons(TRUE);
        break;
    case LOCAL_OLD_ROD:
    case LOCAL_GOOD_ROD:
    case LOCAL_SUPER_ROD:
        LocalDex_AddSwarms(OUTBREAK_FISHING, TRUE);
        if (feebas != NULL)
            LocalDex_AddEntry(feebas->species, feebas->minLevel, feebas->maxLevel, LOCAL_TAG_SPECIAL);
        if (sLocalDex->methods[sLocalDex->page] == LOCAL_OLD_ROD)
            LocalDex_AddSlots(header ? header->fishingMonsInfo : NULL, 0, 2, FALSE);
        else if (sLocalDex->methods[sLocalDex->page] == LOCAL_GOOD_ROD)
            LocalDex_AddSlots(header ? header->fishingMonsInfo : NULL, 2, 3, FALSE);
        else
            LocalDex_AddSlots(header ? header->fishingMonsInfo : NULL, 5, 5, FALSE);
        break;
    }
}

static void LocalDex_ScrollToCursor(void)
{
    if (sLocalDex->cursor < sLocalDex->scrollTop)
        sLocalDex->scrollTop = sLocalDex->cursor;
    else if (sLocalDex->cursor >= sLocalDex->scrollTop + LOCAL_VISIBLE_ROWS)
        sLocalDex->scrollTop = sLocalDex->cursor - LOCAL_VISIBLE_ROWS + 1;
}

// Rebuilds the entries for the current page/time, keeping the same Pokémon selected if it is still listed
static void LocalDex_Rebuild(bool8 keepSelection)
{
    u16 species = SPECIES_NONE;
    u8 i;

    if (keepSelection && sLocalDex->cursor < sLocalDex->entryCount)
        species = sLocalDex->entries[sLocalDex->cursor].species;
    LocalDex_BuildEntries();
    sLocalDex->cursor = 0;
    sLocalDex->scrollTop = 0;
    for (i = 0; i < sLocalDex->entryCount; i++)
    {
        if (sLocalDex->entries[i].species == species)
        {
            sLocalDex->cursor = i;
            break;
        }
    }
    LocalDex_ScrollToCursor();
}

static const u8 *LocalDex_GetMethodName(u8 method)
{
    switch (method)
    {
    case LOCAL_LAND:
    default:
        return DoesCurrentMapIgnoreTimeOfDay() ? sText_LocalCave : sText_LocalGrass;
    case LOCAL_WATER:
        return sText_LocalSurfing;
    case LOCAL_ROCK_SMASH:
        return sText_LocalRockSmash;
    case LOCAL_OLD_ROD:
        return sText_LocalOldRod;
    case LOCAL_GOOD_ROD:
        return sText_LocalGoodRod;
    case LOCAL_SUPER_ROD:
        return sText_LocalSuperRod;
    case LOCAL_GROTTO:
        return sText_LocalGrotto;
    }
}

static void LocalDex_Print(u8 fontId, const u8 *str, u8 x, u8 y)
{
    AddTextPrinterParameterized4(0, fontId, x, y, 0, 0, sLocalTextColor, TEXT_SKIP_DRAW, str);
}

static void LocalDex_PrintRight(u8 fontId, const u8 *str, u8 right, u8 y)
{
    LocalDex_Print(fontId, str, right - GetStringWidth(fontId, str, 0), y);
}

// Bar like the main list's selection: a filled rectangle with 2px rounded corners
static void LocalDex_FillBar(u8 color, u16 x, u16 y, u16 width, u16 height)
{
    u16 r = x + width - 1, b = y + height - 1;

    FillWindowPixelRect(0, PIXEL_FILL(color), x, y, width, height);
    FillWindowPixelRect(0, PIXEL_FILL(0), x, y, 2, 1);
    FillWindowPixelRect(0, PIXEL_FILL(0), x, y + 1, 1, 1);
    FillWindowPixelRect(0, PIXEL_FILL(0), r - 1, y, 2, 1);
    FillWindowPixelRect(0, PIXEL_FILL(0), r, y + 1, 1, 1);
    FillWindowPixelRect(0, PIXEL_FILL(0), x, b, 2, 1);
    FillWindowPixelRect(0, PIXEL_FILL(0), x, b - 1, 1, 1);
    FillWindowPixelRect(0, PIXEL_FILL(0), r - 1, b, 2, 1);
    FillWindowPixelRect(0, PIXEL_FILL(0), r, b - 1, 1, 1);
}

// A small shaded square followed by a label, explaining one row shading
static u8 LocalDex_PrintLegend(u8 shade, const u8 *label, u8 x)
{
    FillWindowPixelRect(0, PIXEL_FILL(shade), x, 24, 7, 7);
    LocalDex_Print(FONT_SMALL, label, x + 9, 20);
    return x + 9 + GetStringWidth(FONT_SMALL, label, 0) + 8;
}

static u8 LocalDex_GetShade(u8 tag)
{
    return tag == LOCAL_TAG_SWARM ? LOCAL_SHADE_SWARM : LOCAL_SHADE_SPECIAL;
}

static void LocalDex_DrawRow(u8 index, u8 row)
{
    const struct LocalDexEntry *entry = &sLocalDex->entries[index];
    u8 y = LOCAL_LIST_Y + row * LOCAL_ROW_HEIGHT;
    u16 dexNum = SpeciesToNationalPokedexNum(entry->species);
    u8 str[16];
    u8 *end;

    if (index == sLocalDex->cursor)
    {
        LocalDex_FillBar(TEXT_COLOR_WHITE, LOCAL_BAR_X, y, LOCAL_BAR_WIDTH, LOCAL_ROW_HEIGHT - 2);
        if (entry->tag != LOCAL_TAG_NONE)
            FillWindowPixelRect(0, PIXEL_FILL(LocalDex_GetShade(entry->tag)), LOCAL_MARKER_X, y + 7, 7, 7);
    }
    else if (entry->tag != LOCAL_TAG_NONE)
    {
        LocalDex_FillBar(LocalDex_GetShade(entry->tag), LOCAL_BAR_X, y, LOCAL_BAR_WIDTH, LOCAL_ROW_HEIGHT - 2);
    }

    if (GetPokedexDisplayFlag(dexNum, FLAG_GET_CAUGHT))
        BlitBitmapToWindow(0, sCaughtBall_Gfx, LOCAL_BALL_X, y + 3, 8, 16);
    if (GetPokedexDisplayFlag(dexNum, FLAG_GET_SEEN))
        LocalDex_Print(FONT_NARROW, gSpeciesNames[entry->species], LOCAL_NAME_X, y + 3);
    else
        LocalDex_Print(FONT_NARROW, sText_LocalUnseen, LOCAL_NAME_X, y + 3);

    end = StringCopy(str, sText_LocalLv);
    end = ConvertIntToDecimalStringN(end, entry->minLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (entry->maxLevel != entry->minLevel)
    {
        end = StringCopy(end, sText_LocalDash);
        ConvertIntToDecimalStringN(end, entry->maxLevel, STR_CONV_MODE_LEFT_ALIGN, 3);
    }
    LocalDex_PrintRight(FONT_SMALL, str, LOCAL_LEVEL_RIGHT, y + 5);
}

static void LocalDex_DestroyIcons(void)
{
    u8 i;

    for (i = 0; i < LOCAL_VISIBLE_ROWS; i++)
    {
        if (sLocalDex->iconSpriteIds[i] != SPRITE_NONE)
        {
            FreeAndDestroyMonIconSprite(&gSprites[sLocalDex->iconSpriteIds[i]]);
            sLocalDex->iconSpriteIds[i] = SPRITE_NONE;
        }
    }
}

// PC-style icons for the visible rows; the selected Pokémon's icon animates. Unseen ones use the "?" icon.
static void LocalDex_CreateIcons(void)
{
    u8 row;

    LocalDex_DestroyIcons();
    for (row = 0; row < LOCAL_VISIBLE_ROWS && sLocalDex->scrollTop + row < sLocalDex->entryCount; row++)
    {
        u8 index = sLocalDex->scrollTop + row;
        u16 species = sLocalDex->entries[index].species;

        if (!GetPokedexDisplayFlag(SpeciesToNationalPokedexNum(species), FLAG_GET_SEEN))
            species = SPECIES_OLD_UNOWN_B; // question mark icon
        sLocalDex->iconSpriteIds[row] = CreateMonIcon(species,
                                                      index == sLocalDex->cursor ? SpriteCB_MonIcon : SpriteCallbackDummy,
                                                      LOCAL_ICON_X, LOCAL_LIST_Y + row * LOCAL_ROW_HEIGHT + 9,
                                                      1, 0, TRUE);
        gSprites[sLocalDex->iconSpriteIds[row]].oam.priority = 0;
    }
}

static void LocalDex_Draw(void)
{
    u8 str[32];
    u8 *end;
    u8 i, x;
    bool8 hasSwarm = FALSE, hasSpecial = FALSE;

    FillWindowPixelBuffer(0, PIXEL_FILL(0));

    GetMapName(str, gMapHeader.regionMapSectionId, 0);
    LocalDex_Print(FONT_NORMAL, str, 10, 4);
    if (DoesCurrentMapIgnoreTimeOfDay())
    {
        // Time of day doesn't matter underground: cover the L/R badges
        FillWindowPixelRect(0, PIXEL_FILL(LOCAL_FILL), LOCAL_L_BADGE_X, LOCAL_BADGE_Y, 11, 11);
        FillWindowPixelRect(0, PIXEL_FILL(LOCAL_FILL), LOCAL_R_BADGE_X, LOCAL_BADGE_Y, 11, 11);
        LocalDex_PrintRight(FONT_NORMAL, sText_LocalAnyTime, 230, 4);
        gSprites[sLocalDex->timeSpriteId].invisible = TRUE;
    }
    else
    {
        static const u8 *const sTimeNames[] =
        {
            [TIME_OF_DAY_NIGHT] = sText_LocalNight,
            [TIME_OF_DAY_MORNING] = sText_LocalMorning,
            [TIME_OF_DAY_DAY] = sText_LocalDay,
        };

        LocalDex_Print(FONT_NORMAL, sTimeNames[sLocalDex->timeOfDay], LOCAL_TIME_TEXT_X, 4);
        StartSpriteAnim(&gSprites[sLocalDex->timeSpriteId], sLocalDex->timeOfDay);
        gSprites[sLocalDex->timeSpriteId].invisible = FALSE;
    }

    AddTextPrinterParameterized4(0, FONT_SMALL, 20, 146, 0, 0, sLocalFooterTextColor, TEXT_SKIP_DRAW, sText_LocalEntry);
    AddTextPrinterParameterized4(0, FONT_SMALL, 76, 146, 0, 0, sLocalFooterTextColor, TEXT_SKIP_DRAW, sText_LocalBack);

    if (sLocalDex->methodCount == 0)
    {
        LocalDex_Print(FONT_NORMAL, sText_LocalNoWildMons, GetStringCenterAlignXOffset(FONT_NORMAL, sText_LocalNoWildMons, DISPLAY_WIDTH), 84);
        CopyWindowToVram(0, COPYWIN_GFX);
        return;
    }

    end = StringCopy(str, sText_LocalLeft);
    end = StringCopy(end, LocalDex_GetMethodName(sLocalDex->methods[sLocalDex->page]));
    StringCopy(end, sText_LocalRight);
    LocalDex_Print(FONT_NORMAL, str, 10, 19);
    end = ConvertIntToDecimalStringN(str, sLocalDex->page + 1, STR_CONV_MODE_LEFT_ALIGN, 1);
    end = StringCopy(end, sText_LocalSlash);
    ConvertIntToDecimalStringN(end, sLocalDex->methodCount, STR_CONV_MODE_LEFT_ALIGN, 1);
    LocalDex_PrintRight(FONT_NORMAL, str, 230, 19);

    for (i = 0; i < sLocalDex->entryCount; i++)
    {
        if (sLocalDex->entries[i].tag == LOCAL_TAG_SWARM)
            hasSwarm = TRUE;
        else if (sLocalDex->entries[i].tag == LOCAL_TAG_SPECIAL)
            hasSpecial = TRUE;
    }
    x = 104;
    if (hasSwarm)
        x = LocalDex_PrintLegend(LOCAL_SHADE_SWARM, sText_LocalSwarm, x);
    if (hasSpecial)
        LocalDex_PrintLegend(LOCAL_SHADE_SPECIAL, sText_LocalSpecial, x);

    for (i = 0; i < LOCAL_VISIBLE_ROWS && sLocalDex->scrollTop + i < sLocalDex->entryCount; i++)
        LocalDex_DrawRow(sLocalDex->scrollTop + i, i);
    if (sLocalDex->scrollTop > 0)
        LocalDex_Print(FONT_NORMAL, sText_LocalUp, LOCAL_ARROW_X, 38);
    if (sLocalDex->scrollTop + LOCAL_VISIBLE_ROWS < sLocalDex->entryCount)
        LocalDex_Print(FONT_NORMAL, sText_LocalDown, LOCAL_ARROW_X, 128);
    if (sLocalDex->entryCount == 0)
        LocalDex_Print(FONT_NORMAL, sText_LocalNoneNow, GetStringCenterAlignXOffset(FONT_NORMAL, sText_LocalNoneNow, DISPLAY_WIDTH), 84);
    CopyWindowToVram(0, COPYWIN_GFX);
}

static void LocalDex_Refresh(void)
{
    LocalDex_Draw();
    LocalDex_CreateIcons();
}

static void LocalDex_DestroySprites(void)
{
    LocalDex_DestroyIcons();
    FreeMonIconPalettes();
    if (sLocalDex->timeSpriteId != SPRITE_NONE)
    {
        DestroySpriteAndFreeResources(&gSprites[sLocalDex->timeSpriteId]);
        sLocalDex->timeSpriteId = SPRITE_NONE;
    }
}

// Called from the list's start menu once the screen has faded out
static void Task_OpenLocalScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        sPokedexView->menuIsOpen = FALSE;
        sPokedexView->menuY = 0;
        ClearMonSprites();
        TryDestroyStatBars();
        TryDestroyStatBarsBg();
        FreeWindowAndBgBuffers();
        FREE_AND_SET_NULL(sLocalDex);
        gMain.state = 0;
        gTasks[taskId].func = Task_LoadLocalScreen;
    }
}

// Also used to come back from a Pokédex page opened from the local screen (sLocalDex is kept)
static void Task_LoadLocalScreen(u8 taskId)
{
    u8 i;

    switch (gMain.state)
    {
    case 0:
    default:
        sPokedexView->currentPage = PAGE_LOCAL;
        SetVBlankCallback(NULL);
        ResetOtherVideoRegisters(0);
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sInfoScreen_BgTemplate, ARRAY_COUNT(sInfoScreen_BgTemplate));
        SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
        SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
        InitWindows(sLocalScreen_WindowTemplates);
        DeactivateAllTextPrinters();
        DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_Menu_3_Gfx, 0x2000, 0, 0);
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenLocal_Tilemap, 0, 0);
        ResetPaletteFade();
        LoadPokedexBgPalette(FALSE);
        PutWindowTilemap(0);
        gMain.state++;
        break;
    case 1:
        if (sLocalDex == NULL)
        {
            sLocalDex = AllocZeroed(sizeof(*sLocalDex));
            sLocalDex->timeOfDay = UpdateTimeOfDay();
            LocalDex_FindMethods();
            LocalDex_Rebuild(FALSE);
        }
        else
        {
            LocalDex_Rebuild(TRUE);
        }
        for (i = 0; i < LOCAL_VISIBLE_ROWS; i++)
            sLocalDex->iconSpriteIds[i] = SPRITE_NONE;
        LoadMonIconPalettes();
        LoadSpriteSheet(&sLocalTimeIconSheet);
        LoadSpritePalette(&sLocalTimeIconPalette);
        sLocalDex->timeSpriteId = CreateSprite(&sSpriteTemplate_LocalTimeIcon, LOCAL_TIME_ICON_X, LOCAL_TIME_ICON_Y, 0);
        LocalDex_Refresh();
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 2:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        SetVBlankCallback(VBlankCB_Pokedex);
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        HideBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 3:
        if (!gPaletteFade.active)
        {
            gMain.state = 0;
            gTasks[taskId].func = Task_HandleLocalScreenInput;
        }
        break;
    }
}

static void Task_HandleLocalScreenInput(u8 taskId)
{
    // L/R first: with the "L=A" button option, L also reports A
    if (JOY_NEW(L_BUTTON | R_BUTTON))
    {
        // Shown in the order MORNING -> DAY -> NIGHT
        static const u8 sNextTime[] =
        {
            [TIME_OF_DAY_MORNING] = TIME_OF_DAY_DAY,
            [TIME_OF_DAY_DAY] = TIME_OF_DAY_NIGHT,
            [TIME_OF_DAY_NIGHT] = TIME_OF_DAY_MORNING,
        };
        static const u8 sPrevTime[] =
        {
            [TIME_OF_DAY_MORNING] = TIME_OF_DAY_NIGHT,
            [TIME_OF_DAY_DAY] = TIME_OF_DAY_MORNING,
            [TIME_OF_DAY_NIGHT] = TIME_OF_DAY_DAY,
        };

        if (DoesCurrentMapIgnoreTimeOfDay())
            return;
        if (JOY_NEW(L_BUTTON))
            sLocalDex->timeOfDay = sPrevTime[sLocalDex->timeOfDay];
        else
            sLocalDex->timeOfDay = sNextTime[sLocalDex->timeOfDay];
        LocalDex_Rebuild(TRUE);
        LocalDex_Refresh();
        PlaySE(SE_SELECT);
    }
    else if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_PC_OFF);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
        gTasks[taskId].func = Task_ExitLocalScreen;
    }
    else if (JOY_NEW(A_BUTTON))
    {
        u16 dexNum, i;

        if (sLocalDex->entryCount != 0)
        {
            dexNum = SpeciesToNationalPokedexNum(sLocalDex->entries[sLocalDex->cursor].species);
            for (i = 0; i < sPokedexView->pokemonListCount; i++)
            {
                if (sPokedexView->pokedexList[i].dexNum == dexNum && sPokedexView->pokedexList[i].seen)
                {
                    sPokedexView->selectedPokemon = i;
                    PlaySE(SE_PIN);
                    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
                    gTasks[taskId].func = Task_OpenInfoFromLocalScreen;
                    return;
                }
            }
        }
        // Unseen, or not in the current (Hoenn) list
        PlaySE(SE_FAILURE);
    }
    else if (JOY_NEW(DPAD_LEFT | DPAD_RIGHT) && sLocalDex->methodCount > 1)
    {
        if (JOY_NEW(DPAD_LEFT))
            sLocalDex->page = (sLocalDex->page == 0) ? sLocalDex->methodCount - 1 : sLocalDex->page - 1;
        else
            sLocalDex->page = (sLocalDex->page + 1) % sLocalDex->methodCount;
        LocalDex_Rebuild(FALSE);
        LocalDex_Refresh();
        PlaySE(SE_DEX_PAGE);
    }
    else if (JOY_REPEAT(DPAD_UP) && sLocalDex->cursor > 0)
    {
        sLocalDex->cursor--;
        LocalDex_ScrollToCursor();
        LocalDex_Refresh();
        PlaySE(SE_SELECT);
    }
    else if (JOY_REPEAT(DPAD_DOWN) && sLocalDex->cursor + 1 < sLocalDex->entryCount)
    {
        sLocalDex->cursor++;
        LocalDex_ScrollToCursor();
        LocalDex_Refresh();
        PlaySE(SE_SELECT);
    }
}

static void Task_OpenInfoFromLocalScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        LocalDex_DestroySprites();
        FreeWindowAndBgBuffers();
        sPokedexView->currentPageBackup = PAGE_MAIN;
        gTasks[taskId].data[0] = LoadInfoScreenWithoutMonSprite(&sPokedexView->pokedexList[sPokedexView->selectedPokemon]);
        gTasks[taskId].func = Task_WaitForExitLocalInfoScreen;
    }
}

// Like Task_WaitForExitInfoScreen, but B on the Pokédex page comes back to the local screen
static void Task_WaitForExitLocalInfoScreen(u8 taskId)
{
    u8 infoTaskId = gTasks[taskId].data[0];

    if (gTasks[infoTaskId].isActive)
    {
        if (sPokedexView->currentPage == PAGE_INFO && !IsInfoScreenScrolling(infoTaskId) && TryDoInfoScreenScroll())
            StartInfoScreenScroll(&sPokedexView->pokedexList[sPokedexView->selectedPokemon], infoTaskId);
    }
    else
    {
        gMain.state = 0;
        gTasks[taskId].func = Task_LoadLocalScreen;
    }
}

static void Task_ExitLocalScreen(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        LocalDex_DestroySprites();
        FREE_AND_SET_NULL(sLocalDex);
        if (sLocalModeExitCallback != NULL)
        {
            // Opened from the POKéDEX key item: close the Pokédex
            MainCallback exitCallback = sLocalModeExitCallback;

            sLocalModeExitCallback = NULL;
            ClearMonSprites();
            FreeWindowAndBgBuffers();
            DestroyTask(taskId);
            SetMainCallback2(exitCallback);
            m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0x100);
            FREE_AND_SET_NULL(sPokedexView);
            return;
        }
        FreeWindowAndBgBuffers();
        gTasks[taskId].func = Task_OpenPokedexMainPage;
    }
}



//************************************
//*                                  *
//*        Search Screen             *
//*                                  *
//************************************
static void Task_WaitForExitSearch(u8 taskId)
{
    if (!gTasks[gTasks[taskId].tLoadScreenTaskId].isActive)
    {
        ClearMonSprites();
        TryDestroyStatBars(); //HGSS_Ui
        TryDestroyStatBarsBg(); //HGSS_Ui

        // Search produced results
        if (sPokedexView->screenSwitchState != 0)
        {
            sPokedexView->selectedPokemon = 0;
            sPokedexView->pokeBallRotation = POKEBALL_ROTATION_TOP;
            gTasks[taskId].func = Task_OpenSearchResults;
        }
        // Search didn't produce results
        else
        {
            sPokedexView->pokeBallRotation = sPokedexView->pokeBallRotationBackup;
            sPokedexView->selectedPokemon = sPokedexView->selectedPokemonBackup;
            sPokedexView->dexMode = sPokedexView->dexModeBackup;
            if (!IsNationalPokedexEnabled())
                sPokedexView->dexMode = DEX_MODE_HOENN;
            sPokedexView->dexOrder = sPokedexView->dexOrderBackup;
            gTasks[taskId].func = Task_OpenPokedexMainPage;
        }
    }
}

static void Task_OpenSearchResults(u8 taskId)
{
    sPokedexView->isSearchResults = TRUE;
    sPokedexView->sEvoScreenData.fromEvoPage = FALSE;
    #ifdef POKEMON_EXPANSION
    sPokedexView->formSpecies = 0;
    #endif
    if (LoadPokedexListPage(PAGE_SEARCH_RESULTS))
        gTasks[taskId].func = Task_HandleSearchResultsInput;
}

static void Task_HandleSearchResultsInput(u8 taskId)
{
    SetGpuReg(REG_OFFSET_BG0VOFS, sPokedexView->menuY);

    if (sPokedexView->menuY)
    {
        sPokedexView->menuY -= 8;
        if (sPokedexView->menuIsOpen == FALSE && sPokedexView->menuY == 8) //HGSS_Ui
        {
            CreateStatBars(&sPokedexView->pokedexList[sPokedexView->selectedPokemon]);
            CreateStatBarsBg();
        }
    }
    else
    {
        if (JOY_NEW(A_BUTTON) && sPokedexView->pokedexList[sPokedexView->selectedPokemon].seen)
        {
            u32 a;

            TryDestroyStatBars(); //HGSS_Ui
            UpdateSelectedMonSpriteId();
            a = (1 << (gSprites[sPokedexView->selectedMonSpriteId].oam.paletteNum + 16));
            gSprites[sPokedexView->selectedMonSpriteId].callback = SpriteCB_MoveMonForInfoScreen;
            BeginNormalPaletteFade(~a, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].func = Task_OpenSearchResultsInfoScreenAfterMonMovement;
            PlaySE(SE_PIN);
            FreeWindowAndBgBuffers();
        }
        else if (JOY_NEW(START_BUTTON))
        {
            sPokedexView->menuY = 0;
            sPokedexView->menuIsOpen = TRUE;
            sPokedexView->menuCursorPos = 0;
            gTasks[taskId].func = Task_HandleSearchResultsStartMenuInput;
            PlaySE(SE_SELECT);
        }
        else if (JOY_NEW(SELECT_BUTTON))
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].tLoadScreenTaskId = LoadSearchMenu();
            sPokedexView->screenSwitchState = 0;
            gTasks[taskId].func = Task_WaitForExitSearch;
            PlaySE(SE_PC_LOGIN);
            FreeWindowAndBgBuffers();
        }
        else if (JOY_NEW(B_BUTTON))
        {
            TryDestroyStatBars(); //HGSS_Ui
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gTasks[taskId].func = Task_ReturnToPokedexFromSearchResults;
            PlaySE(SE_PC_OFF);
        }
        else
        {
            //Handle D-pad
            sPokedexView->selectedPokemon = TryDoPokedexScroll(sPokedexView->selectedPokemon, 0xE);
            if (sPokedexView->scrollTimer)
                gTasks[taskId].func = Task_WaitForSearchResultsScroll;
            else if (!sPokedexView->scrollTimer && !sPokedexView->scrollSpeed && sPokedexView->justScrolled) //HGSS_Ui
                CreateStatBars(&sPokedexView->pokedexList[sPokedexView->selectedPokemon]); //HGSS_Ui
        }
    }
}

static void Task_WaitForSearchResultsScroll(u8 taskId)
{
    TryDestroyStatBars(); //HGSS_Ui
    if (UpdateDexListScroll(sPokedexView->scrollDirection, sPokedexView->scrollMonIncrement, sPokedexView->maxScrollTimer))
        gTasks[taskId].func = Task_HandleSearchResultsInput;
}

static void Task_HandleSearchResultsStartMenuInput(u8 taskId)
{
    SetGpuReg(REG_OFFSET_BG0VOFS, sPokedexView->menuY);

    if (sPokedexView->menuY != 96)
    {
        sPokedexView->menuY += 8;
    }
    else
    {
        if (JOY_NEW(A_BUTTON))
        {
            switch (sPokedexView->menuCursorPos)
            {
            case 0: //BACK TO LIST
            default:
                gMain.newKeys |= START_BUTTON;
                break;
            case 1: //LIST TOP
                sPokedexView->selectedPokemon = 0;
                sPokedexView->pokeBallRotation = POKEBALL_ROTATION_TOP;
                ClearMonSprites();
                CreateMonSpritesAtPos(sPokedexView->selectedPokemon, 0xE);
                gMain.newKeys |= START_BUTTON;
                break;
            case 2: //LIST BOTTOM
                sPokedexView->selectedPokemon = sPokedexView->pokemonListCount - 1;
                sPokedexView->pokeBallRotation = sPokedexView->pokemonListCount * 16 + POKEBALL_ROTATION_BOTTOM;
                ClearMonSprites();
                CreateMonSpritesAtPos(sPokedexView->selectedPokemon, 0xE);
                gMain.newKeys |= START_BUTTON;
                break;
            case 3: //BACK TO POKEDEX
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
                gTasks[taskId].func = Task_ReturnToPokedexFromSearchResults;
                PlaySE(SE_TRUCK_DOOR);
                break;
            case 4: //CLOSE POKEDEX
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
                gTasks[taskId].func = Task_ClosePokedexFromSearchResultsStartMenu;
                PlaySE(SE_PC_OFF);
                break;
            }
        }

        //Exit menu when Start or B is pressed
        if (JOY_NEW(START_BUTTON | B_BUTTON))
        {
            sPokedexView->menuIsOpen = FALSE;
            gTasks[taskId].func = Task_HandleSearchResultsInput;
            PlaySE(SE_SELECT);
        }
        else if (JOY_REPEAT(DPAD_UP) && sPokedexView->menuCursorPos)
        {
            sPokedexView->menuCursorPos--;
            PlaySE(SE_SELECT);
        }
        else if (JOY_REPEAT(DPAD_DOWN) && sPokedexView->menuCursorPos < 4)
        {
            sPokedexView->menuCursorPos++;
            PlaySE(SE_SELECT);
        }
    }
}

static void Task_OpenSearchResultsInfoScreenAfterMonMovement(u8 taskId)
{
    if (gSprites[sPokedexView->selectedMonSpriteId].x == MON_PAGE_X && gSprites[sPokedexView->selectedMonSpriteId].y == MON_PAGE_Y)
    {
        sPokedexView->currentPageBackup = sPokedexView->currentPage;
        gTasks[taskId].tLoadScreenTaskId = LoadInfoScreen(&sPokedexView->pokedexList[sPokedexView->selectedPokemon], sPokedexView->selectedMonSpriteId);
        sPokedexView->selectedMonSpriteId = -1;
        gTasks[taskId].func = Task_WaitForExitSearchResultsInfoScreen;
    }
}

static void Task_WaitForExitSearchResultsInfoScreen(u8 taskId)
{
    if (gTasks[gTasks[taskId].tLoadScreenTaskId].isActive)
    {
        // While active, handle scroll input
        if (sPokedexView->currentPage == PAGE_INFO && !IsInfoScreenScrolling(gTasks[taskId].tLoadScreenTaskId) && TryDoInfoScreenScroll())
            StartInfoScreenScroll(&sPokedexView->pokedexList[sPokedexView->selectedPokemon], gTasks[taskId].tLoadScreenTaskId);
    }
    else
    {
        // Exiting, back to search results
        gTasks[taskId].func = Task_OpenSearchResults;
    }
}

static void Task_ReturnToPokedexFromSearchResults(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        sPokedexView->pokeBallRotation = sPokedexView->pokeBallRotationBackup;
        sPokedexView->selectedPokemon = sPokedexView->selectedPokemonBackup;
        sPokedexView->dexMode = sPokedexView->dexModeBackup;
        if (!IsNationalPokedexEnabled())
            sPokedexView->dexMode = DEX_MODE_HOENN;
        sPokedexView->dexOrder = sPokedexView->dexOrderBackup;
        gTasks[taskId].func = Task_OpenPokedexMainPage;
        ClearMonSprites();
        FreeWindowAndBgBuffers();
    }
}

static void Task_ClosePokedexFromSearchResultsStartMenu(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        sPokedexView->pokeBallRotation = sPokedexView->pokeBallRotationBackup;
        sPokedexView->selectedPokemon = sPokedexView->selectedPokemonBackup;
        sPokedexView->dexMode = sPokedexView->dexModeBackup;
        if (!IsNationalPokedexEnabled())
            sPokedexView->dexMode = DEX_MODE_HOENN;
        sPokedexView->dexOrder = sPokedexView->dexOrderBackup;
        gTasks[taskId].func = Task_ClosePokedex;
    }
}

#undef tLoadScreenTaskId


//************************************
//*                                  *
//*        Search code               *
//*                                  *
//************************************
// Filters from the right-hand search column
struct SearchExtraParams
{
    u8 bstStat;     // SEARCH_STAT_*
    u8 bstOp;       // SEARCH_OP_*
    u16 bstValue;
    u8 yieldStat;   // SEARCH_STAT_*, never TOTAL
    u16 moves[4];
    bool8 all;      // search every Pokémon as if caught
};

static u16 GetSearchBaseStat(u16 species, u8 stat)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];

    switch (stat)
    {
    case SEARCH_STAT_HP:    return info->baseHP;
    case SEARCH_STAT_ATK:   return info->baseAttack;
    case SEARCH_STAT_DEF:   return info->baseDefense;
    case SEARCH_STAT_SPATK: return info->baseSpAttack;
    case SEARCH_STAT_SPDEF: return info->baseSpDefense;
    case SEARCH_STAT_SPEED: return info->baseSpeed;
    case SEARCH_STAT_TOTAL:
        return info->baseHP + info->baseAttack + info->baseDefense
             + info->baseSpAttack + info->baseSpDefense + info->baseSpeed;
    }
    return 0;
}

static u8 GetSearchEvYield(u16 species, u8 stat)
{
    const struct SpeciesInfo *info = &gSpeciesInfo[species];

    switch (stat)
    {
    case SEARCH_STAT_HP:    return info->evYield_HP;
    case SEARCH_STAT_ATK:   return info->evYield_Attack;
    case SEARCH_STAT_DEF:   return info->evYield_Defense;
    case SEARCH_STAT_SPATK: return info->evYield_SpAttack;
    case SEARCH_STAT_SPDEF: return info->evYield_SpDefense;
    case SEARCH_STAT_SPEED: return info->evYield_Speed;
    }
    return 0;
}

static bool8 SearchStatMatches(u16 value, u8 op, u16 target)
{
    switch (op)
    {
    case SEARCH_OP_EQUAL:    return value == target;
    case SEARCH_OP_AT_LEAST: return value >= target;
    case SEARCH_OP_AT_MOST:  return value <= target;
    case SEARCH_OP_LESS:     return value < target;
    case SEARCH_OP_MORE:     return value > target;
    }
    return FALSE;
}

// Pre-evolution of species (SPECIES_NONE for a base form). preEvoTable is an optional
// species -> pre-evolution table from BuildPreEvolutionTable; without it the evolution table is scanned.
static u16 GetPreEvolutionSpecies(u16 species, const u16 *preEvoTable)
{
    u16 i;
    u8 j;

    if (preEvoTable != NULL)
        return preEvoTable[species];
    for (i = 1; i < NUM_SPECIES; i++)
    {
        for (j = 0; j < EVOS_PER_MON; j++)
        {
            if (gEvolutionTable[i][j].targetSpecies == species)
                return i;
        }
    }
    return SPECIES_NONE;
}

static u16 *BuildPreEvolutionTable(void)
{
    u16 *table = AllocZeroed(NUM_SPECIES * sizeof(u16));
    u16 i, target;
    u8 j;

    if (table == NULL)
        return NULL;
    for (i = 1; i < NUM_SPECIES; i++)
    {
        for (j = 0; j < EVOS_PER_MON; j++)
        {
            target = gEvolutionTable[i][j].targetSpecies;
            if (target != SPECIES_NONE && target < NUM_SPECIES && table[target] == SPECIES_NONE)
                table[target] = i;
        }
    }
    return table;
}

// Egg moves of the species and of every earlier stage of its evolution line
static bool8 SpeciesCanLearnInheritedEggMove(u16 species, u16 move, const u16 *preEvoTable)
{
    u8 stage;

    for (stage = 0; stage < EVOS_PER_MON && species != SPECIES_NONE; stage++)
    {
        if (SpeciesCanLearnEggMove(species, move))
            return TRUE;
        species = GetPreEvolutionSpecies(species, preEvoTable);
    }
    return FALSE;
}

// Level up, TM/HM, tutor or egg move (egg moves inherited from pre-evolutions count)
static bool8 SearchSpeciesCanLearnMove(u16 species, u16 move, const u16 *preEvoTable)
{
    u16 i;

    for (i = 0; i < MAX_LEVEL_UP_MOVES && gLevelUpLearnsets[species][i] != LEVEL_UP_END; i++)
    {
        if ((gLevelUpLearnsets[species][i] & LEVEL_UP_MOVE_ID) == move)
            return TRUE;
    }
    for (i = 0; i < NUM_TECHNICAL_MACHINES + NUM_HIDDEN_MACHINES; i++)
    {
        if (ItemIdToBattleMoveId(ITEM_TM01 + i) == move && CanSpeciesLearnTMHM(species, i))
            return TRUE;
    }
    for (i = 0; i < TUTOR_MOVE_COUNT; i++)
    {
        if (gTutorMoves[i] == move && CanLearnTutorMove(species, i))
            return TRUE;
    }
    return SpeciesCanLearnInheritedEggMove(species, move, preEvoTable);
}

// Yield order: Pokémon yielding only the chosen stat first, then those that also yield other stats;
// each group by the chosen stat's yield, highest first (stable, so ties keep the dex order)
static bool8 SearchYieldGoesBefore(u16 speciesA, u16 speciesB, u8 stat)
{
    u8 i, othersA = 0, othersB = 0;

    for (i = SEARCH_STAT_HP; i <= SEARCH_STAT_SPEED; i++)
    {
        if (i != stat)
        {
            othersA += (GetSearchEvYield(speciesA, i) != 0);
            othersB += (GetSearchEvYield(speciesB, i) != 0);
        }
    }
    if ((othersA != 0) != (othersB != 0))
        return othersA == 0;
    return GetSearchEvYield(speciesA, stat) > GetSearchEvYield(speciesB, stat);
}

static void ApplySearchExtras(const struct SearchExtraParams *extra)
{
    u16 i, j, count, species;
    u8 k;
    struct PokedexListItem item;
    u16 *preEvoTable = NULL;

    if (extra->bstStat != SEARCH_STAT_NONE)
    {
        for (i = 0, count = 0; i < sPokedexView->pokemonListCount; i++)
        {
            species = DexNumToSpecies(sPokedexView->pokedexList[i].dexNum);
            if (SearchStatMatches(GetSearchBaseStat(species, extra->bstStat), extra->bstOp, extra->bstValue))
                sPokedexView->pokedexList[count++] = sPokedexView->pokedexList[i];
        }
        sPokedexView->pokemonListCount = count;
    }

    for (k = 0; k < ARRAY_COUNT(extra->moves); k++)
    {
        if (extra->moves[k] == MOVE_NONE)
            continue;
        if (preEvoTable == NULL)
            preEvoTable = BuildPreEvolutionTable();
        for (i = 0, count = 0; i < sPokedexView->pokemonListCount; i++)
        {
            species = DexNumToSpecies(sPokedexView->pokedexList[i].dexNum);
            if (SearchSpeciesCanLearnMove(species, extra->moves[k], preEvoTable))
                sPokedexView->pokedexList[count++] = sPokedexView->pokedexList[i];
        }
        sPokedexView->pokemonListCount = count;
    }
    if (preEvoTable != NULL)
        Free(preEvoTable);

    if (extra->yieldStat != SEARCH_STAT_NONE)
    {
        for (i = 0, count = 0; i < sPokedexView->pokemonListCount; i++)
        {
            species = DexNumToSpecies(sPokedexView->pokedexList[i].dexNum);
            if (GetSearchEvYield(species, extra->yieldStat) != 0)
                sPokedexView->pokedexList[count++] = sPokedexView->pokedexList[i];
        }
        sPokedexView->pokemonListCount = count;

        for (i = 1; i < count; i++)
        {
            item = sPokedexView->pokedexList[i];
            species = DexNumToSpecies(item.dexNum);
            for (j = i; j > 0 && SearchYieldGoesBefore(species, DexNumToSpecies(sPokedexView->pokedexList[j - 1].dexNum), extra->yieldStat); j--)
                sPokedexView->pokedexList[j] = sPokedexView->pokedexList[j - 1];
            sPokedexView->pokedexList[j] = item;
        }
    }
}

static int DoPokedexSearch(u8 dexMode, u8 order, u8 abcGroup, u8 bodyColor, u8 type1, u8 type2, const struct SearchExtraParams *extra)
{
    u16 species;
    u16 i,j;
    u16 resultsCount;
    u8 types[2];

    sSearchAsAllCaught = extra->all;
    CreatePokedexList(dexMode, order);
    sSearchAsAllCaught = FALSE;

    for (i = 0, resultsCount = 0; i < NATIONAL_DEX_COUNT; i++)
    {
        if (sPokedexView->pokedexList[i].seen)
        {
            sPokedexView->pokedexList[resultsCount] = sPokedexView->pokedexList[i];
            resultsCount++;
        }
    }
    sPokedexView->pokemonListCount = resultsCount;

    // Search by name
    if (abcGroup != 0xFF)
    {
        for (i = 0, resultsCount = 0; i < sPokedexView->pokemonListCount; i++)
        {
            u8 firstLetter;

            species = DexNumToSpecies(sPokedexView->pokedexList[i].dexNum);
            firstLetter = gSpeciesNames[species][0];
            if (LETTER_IN_RANGE_UPPER(firstLetter, abcGroup) || LETTER_IN_RANGE_LOWER(firstLetter, abcGroup))
            {
                sPokedexView->pokedexList[resultsCount] = sPokedexView->pokedexList[i];
                resultsCount++;
            }
        }
        sPokedexView->pokemonListCount = resultsCount;
    }

    // Search by body color
    if (bodyColor != 0xFF)
    {
        for (i = 0, resultsCount = 0; i < sPokedexView->pokemonListCount; i++)
        {
            species = DexNumToSpecies(sPokedexView->pokedexList[i].dexNum);

            if (bodyColor == gSpeciesInfo[species].bodyColor)
            {
                sPokedexView->pokedexList[resultsCount] = sPokedexView->pokedexList[i];
                resultsCount++;
            }
        }
        sPokedexView->pokemonListCount = resultsCount;
    }

    // Search by type
    if (type1 != TYPE_NONE || type2 != TYPE_NONE)
    {
        if (type1 == TYPE_NONE)
        {
            type1 = type2;
            type2 = TYPE_NONE;
        }

        if (type2 == TYPE_NONE)
        {
            for (i = 0, resultsCount = 0; i < sPokedexView->pokemonListCount; i++)
            {
                if (sPokedexView->pokedexList[i].owned)
                {
                    species = DexNumToSpecies(sPokedexView->pokedexList[i].dexNum);

                    types[0] = gSpeciesInfo[species].types[0];
                    types[1] = gSpeciesInfo[species].types[1];
                    if (types[0] == type1 || types[1] == type1)
                    {
                        sPokedexView->pokedexList[resultsCount] = sPokedexView->pokedexList[i];
                        resultsCount++;
                    }
                }
            }
        }
        else
        {
            for (i = 0, resultsCount = 0; i < sPokedexView->pokemonListCount; i++)
            {
                if (sPokedexView->pokedexList[i].owned)
                {
                    species = DexNumToSpecies(sPokedexView->pokedexList[i].dexNum);

                    types[0] = gSpeciesInfo[species].types[0];
                    types[1] = gSpeciesInfo[species].types[1];
                    if ((types[0] == type1 && types[1] == type2) || (types[0] == type2 && types[1] == type1))
                    {
                        sPokedexView->pokedexList[resultsCount] = sPokedexView->pokedexList[i];
                        resultsCount++;
                    }
                }
            }
        }
        sPokedexView->pokemonListCount = resultsCount;
    }

    ApplySearchExtras(extra);
    resultsCount = sPokedexView->pokemonListCount;

    if (sPokedexView->pokemonListCount != 0)
    {
        for (i = sPokedexView->pokemonListCount; i < NATIONAL_DEX_COUNT; i++)
        {
            sPokedexView->pokedexList[i].dexNum = 0xFFFF;
            sPokedexView->pokedexList[i].seen = FALSE;
            sPokedexView->pokedexList[i].owned = FALSE;
        }
    }

    return resultsCount;
}

static u8 LoadSearchMenu(void)
{
    return CreateTask(Task_LoadSearchMenu, 0);
}

static void PrintSearchText(const u8 *str, u32 x, u32 y)
{
    u8 color[3];

    color[0] = TEXT_COLOR_TRANSPARENT;
    color[1] = TEXT_DYNAMIC_COLOR_6;
    color[2] = TEXT_COLOR_DARK_GRAY;
    AddTextPrinterParameterized4(0, FONT_NORMAL, x, y, 0, 0, color, TEXT_SKIP_DRAW, str);
}

static void ClearSearchMenuRect(u32 x, u32 y, u32 width, u32 height)
{
    FillWindowPixelRect(0, PIXEL_FILL(0), x, y, width, height);
}

// Search task data
#define tTopBarItem             data[0]
#define tMenuItem               data[1]
#define tCursorPos_Mode         data[2]
#define tScrollOffset_Mode      data[3]
#define tCursorPos_Order        data[4]
#define tScrollOffset_Order     data[5]
#define tCursorPos_Name         data[6]
#define tScrollOffset_Name      data[7]
#define tCursorPos_Color        data[8]
#define tScrollOffset_Color     data[9]
#define tCursorPos_TypeLeft     data[10]
#define tScrollOffset_TypeLeft  data[11]
#define tCursorPos_TypeRight    data[12]
#define tScrollOffset_TypeRight data[13]
#define tCursorPos              data[14]
#define tScrollOffset           data[15]

// The original fields keep their picker positions in the task data, which is full; the new
// fields keep theirs in sPokedexView. These give the picker one view of both.
static u16 *GetSearchCursorPtr(u8 taskId, u8 item)
{
    if (item >= SEARCH_EXTRA_FIRST)
        return &sPokedexView->searchCursor[item - SEARCH_EXTRA_FIRST];
    return &gTasks[taskId].data[sSearchOptions[item].taskDataCursorPos];
}

static u16 *GetSearchScrollPtr(u8 taskId, u8 item)
{
    if (item >= SEARCH_EXTRA_FIRST)
        return &sPokedexView->searchScroll[item - SEARCH_EXTRA_FIRST];
    return &gTasks[taskId].data[sSearchOptions[item].taskDataScrollOffset];
}

static u16 GetSearchSelectedIndex(u8 taskId, u8 item)
{
    return *GetSearchCursorPtr(taskId, item) + *GetSearchScrollPtr(taskId, item);
}

static u16 GetSearchOptionCount(u8 item)
{
    switch (item)
    {
    case SEARCH_BST_STAT:
        return ARRAY_COUNT(sSearchBstStatOptions);
    case SEARCH_BST_OP:
        return ARRAY_COUNT(sSearchBstOpOptions);
    case SEARCH_YIELD:
        return ARRAY_COUNT(sSearchYieldOptions);
    case SEARCH_MOVE_1:
    case SEARCH_MOVE_2:
    case SEARCH_MOVE_3:
    case SEARCH_MOVE_4:
        return MOVES_COUNT; // "------" plus every move
    default:
        return sSearchOptions[item].numOptions;
    }
}

// Move picker entry -> move id (entry 0 is "------", i.e. no move)
static u16 GetSearchMoveAt(u16 index)
{
    return index == 0 ? MOVE_NONE : sPokedexView->searchMoveOrder[index - 1];
}

static const u8 *GetSearchOptionTitle(u8 item, u16 index)
{
    switch (item)
    {
    case SEARCH_BST_STAT:
        return sSearchBstStatOptions[index].title;
    case SEARCH_BST_OP:
        return sSearchBstOpOptions[index].title;
    case SEARCH_YIELD:
        return sSearchYieldOptions[index].title;
    case SEARCH_MOVE_1:
    case SEARCH_MOVE_2:
    case SEARCH_MOVE_3:
    case SEARCH_MOVE_4:
        return index == 0 ? sText_SearchMoveNone : gMoveNames[GetSearchMoveAt(index)];
    default:
        return sSearchOptions[item].texts[index].title;
    }
}

static const u8 *GetSearchOptionDescription(u8 item, u16 index)
{
    switch (item)
    {
    case SEARCH_BST_STAT:
        return sSearchBstStatOptions[index].description;
    case SEARCH_BST_OP:
        return sSearchBstOpOptions[index].description;
    case SEARCH_YIELD:
        return sSearchYieldOptions[index].description;
    case SEARCH_MOVE_1:
    case SEARCH_MOVE_2:
    case SEARCH_MOVE_3:
    case SEARCH_MOVE_4:
        return index == 0 ? sText_SearchMoveDesc : gMoveDescriptionPointers[GetSearchMoveAt(index) - 1];
    default:
        return sSearchOptions[item].texts[index].description;
    }
}

// Sorts every move by name for the MOVE picker (insertion sort; runs once per search screen)
static void BuildSearchMoveOrder(void)
{
    u16 i, j, move;

    for (i = 0; i < MOVES_COUNT - 1; i++)
    {
        move = i + 1;
        for (j = i; j > 0 && StringCompare(gMoveNames[sPokedexView->searchMoveOrder[j - 1]], gMoveNames[move]) > 0; j--)
            sPokedexView->searchMoveOrder[j] = sPokedexView->searchMoveOrder[j - 1];
        sPokedexView->searchMoveOrder[j] = move;
    }
}

static void ResetSearchExtras(void)
{
    u8 i;

    for (i = 0; i < SEARCH_EXTRA_COUNT; i++)
    {
        sPokedexView->searchCursor[i] = 0;
        sPokedexView->searchScroll[i] = 0;
    }
    sPokedexView->searchCursor[SEARCH_BST_OP - SEARCH_EXTRA_FIRST] = SEARCH_OP_AT_LEAST;
    sPokedexView->searchBstValue = SEARCH_BST_DEFAULT;
    sPokedexView->searchAll = FALSE;
}

static u16 GetSearchBstMax(u8 taskId)
{
    return GetSearchSelectedIndex(taskId, SEARCH_BST_STAT) == SEARCH_STAT_TOTAL ? SEARCH_BST_TOTAL_MAX : SEARCH_BST_MAX;
}

static void LoadSearchMenuTilemap(void)
{
    if (!IsNationalPokedexEnabled())
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenSearchHoenn_Tilemap, 0, 0);
    else
        CopyToBgTilemapBuffer(3, gPokedexPlusHGSS_ScreenSearchNational_Tilemap, 0, 0);
}

static void Task_LoadSearchMenu(u8 taskId)
{
    u16 i;

    switch (gMain.state)
    {
    default:
    case 0:
        if (!gPaletteFade.active)
        {
            sPokedexView->currentPage = PAGE_SEARCH;
            ResetOtherVideoRegisters(0);
            ResetBgsAndClearDma3BusyFlags(0);
            InitBgsFromTemplates(0, sSearchMenu_BgTemplate, ARRAY_COUNT(sSearchMenu_BgTemplate));
            SetBgTilemapBuffer(3, AllocZeroed(BG_SCREEN_SIZE));
            SetBgTilemapBuffer(2, AllocZeroed(BG_SCREEN_SIZE));
            SetBgTilemapBuffer(1, AllocZeroed(BG_SCREEN_SIZE));
            SetBgTilemapBuffer(0, AllocZeroed(BG_SCREEN_SIZE));
            InitWindows(sSearchMenu_WindowTemplate);
            DeactivateAllTextPrinters();
            PutWindowTilemap(0);
            if (!HGSS_DECAPPED)
                DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_MenuSearch_Gfx, 0x2000, 0, 0);
            else
                DecompressAndLoadBgGfxUsingHeap(3, gPokedexPlusHGSS_MenuSearch_DECA_Gfx, 0x2000, 0, 0);
            LoadSearchMenuTilemap();
            if (!HGSS_DARK_MODE)
                LoadPalette(gPokedexPlusHGSS_MenuSearch_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(4 * 16 - 1));
            else
                LoadPalette(gPokedexPlusHGSS_MenuSearch_dark_Pal + 1, BG_PLTT_ID(0) + 1, PLTT_SIZEOF(4 * 16 - 1));
            gMain.state = 1;
        }
        break;
    case 1:
        LoadCompressedSpriteSheet(&sInterfaceSpriteSheet[HGSS_DECAPPED]);
        LoadSpritePalettes(sInterfaceSpritePalette);
        LoadSpritePalettes(sStatBarSpritePal); //HGSS_Ui
        CreateSearchParameterScrollArrows(taskId);
        for (i = 0; i < NUM_TASK_DATA; i++)
            gTasks[taskId].data[i] = 0;
        ResetSearchExtras();
        BuildSearchMoveOrder();
        SetDefaultSearchModeAndOrder(taskId);
        HighlightSelectedSearchTopBarItem(SEARCH_TOPBAR_SEARCH);
        PrintSelectedSearchParameters(taskId);
        CopyWindowToVram(0, COPYWIN_FULL);
        CopyBgTilemapBufferToVram(1);
        CopyBgTilemapBufferToVram(2);
        CopyBgTilemapBufferToVram(3);
        gMain.state++;
        break;
    case 2:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
        sPokedexView->statBarsSpriteId = 0xFF;  //HGSS_Ui
        CreateStatBars(&sPokedexView->pokedexList[sPokedexView->selectedPokemon]); //HGSS_Ui
        gMain.state++;
        break;
    case 3:
        SetGpuReg(REG_OFFSET_BLDCNT, 0);
        SetGpuReg(REG_OFFSET_BLDALPHA, 0);
        SetGpuReg(REG_OFFSET_BLDY, 0);
        SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
        HideBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        gMain.state++;
        break;
    case 4:
        if (!gPaletteFade.active)
        {
            gTasks[taskId].func = Task_SwitchToSearchMenuTopBar;
            gMain.state = 0;
        }
        break;
    }
}

static void FreeSearchWindowAndBgBuffers(void)
{
    void *tilemapBuffer;

    FreeAllWindowBuffers();
    tilemapBuffer = GetBgTilemapBuffer(0);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(1);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(2);
    if (tilemapBuffer)
        Free(tilemapBuffer);
    tilemapBuffer = GetBgTilemapBuffer(3);
    if (tilemapBuffer)
        Free(tilemapBuffer);
}

static void Task_SwitchToSearchMenuTopBar(u8 taskId)
{
    HighlightSelectedSearchTopBarItem(gTasks[taskId].tTopBarItem);
    PrintSelectedSearchParameters(taskId);
    CopyWindowToVram(0, COPYWIN_GFX);
    CopyBgTilemapBufferToVram(3);
    gTasks[taskId].func = Task_HandleSearchTopBarInput;
}

static void Task_HandleSearchTopBarInput(u8 taskId)
{
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_PC_OFF);
        gTasks[taskId].func = Task_ExitSearch;
        return;
    }
    if (JOY_NEW(A_BUTTON))
    {
        switch (gTasks[taskId].tTopBarItem)
        {
        case SEARCH_TOPBAR_SEARCH:
            PlaySE(SE_PIN);
            gTasks[taskId].tMenuItem = SEARCH_NAME;
            gTasks[taskId].func = Task_SwitchToSearchMenu;
            break;
        case SEARCH_TOPBAR_SHIFT:
            PlaySE(SE_PIN);
            gTasks[taskId].tMenuItem = SEARCH_ORDER;
            gTasks[taskId].func = Task_SwitchToSearchMenu;
            break;
        case SEARCH_TOPBAR_CANCEL:
            PlaySE(SE_PC_OFF);
            gTasks[taskId].func = Task_ExitSearch;
            break;
        }
        return;
    }
    if (JOY_NEW(DPAD_LEFT) && gTasks[taskId].tTopBarItem > SEARCH_TOPBAR_SEARCH)
    {
        PlaySE(SE_DEX_PAGE);
        gTasks[taskId].tTopBarItem--;
        HighlightSelectedSearchTopBarItem(gTasks[taskId].tTopBarItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
    if (JOY_NEW(DPAD_RIGHT) && gTasks[taskId].tTopBarItem < SEARCH_TOPBAR_CANCEL)
    {
        PlaySE(SE_DEX_PAGE);
        gTasks[taskId].tTopBarItem++;
        HighlightSelectedSearchTopBarItem(gTasks[taskId].tTopBarItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
}

static void Task_SwitchToSearchMenu(u8 taskId)
{
    HighlightSelectedSearchMenuItem(gTasks[taskId].tTopBarItem, gTasks[taskId].tMenuItem);
    PrintSelectedSearchParameters(taskId);
    CopyWindowToVram(0, COPYWIN_GFX);
    CopyBgTilemapBufferToVram(3);
    gTasks[taskId].func = Task_HandleSearchMenuInput;
}

// Input for main search menu
static void Task_HandleSearchMenuInput(u8 taskId)
{
    const u8 (*movementMap)[4];

    if (gTasks[taskId].tTopBarItem != SEARCH_TOPBAR_SEARCH)
    {
        if (!IsNationalPokedexEnabled())
            movementMap = sSearchMovementMap_ShiftHoennDex;
        else
            movementMap = sSearchMovementMap_ShiftNatDex;
    }
    else
    {
        if (!IsNationalPokedexEnabled())
            movementMap = sSearchMovementMap_SearchHoennDex;
        else
            movementMap = sSearchMovementMap_SearchNatDex;
    }

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_BALL);
        SetDefaultSearchModeAndOrder(taskId);
        gTasks[taskId].func = Task_SwitchToSearchMenuTopBar;
        return;
    }
    if (JOY_NEW(A_BUTTON))
    {
        if (gTasks[taskId].tMenuItem == SEARCH_OK)
        {
            if (gTasks[taskId].tTopBarItem != SEARCH_TOPBAR_SEARCH)
            {
                sPokeBallRotation = POKEBALL_ROTATION_TOP;
                sPokedexView->pokeBallRotationBackup = POKEBALL_ROTATION_TOP;
                sLastSelectedPokemon = 0;
                sPokedexView->selectedPokemonBackup = 0;
                gSaveBlock2Ptr->pokedex.mode = GetSearchModeSelection(taskId, SEARCH_MODE);
                if (!IsNationalPokedexEnabled())
                    gSaveBlock2Ptr->pokedex.mode = DEX_MODE_HOENN;
                sPokedexView->dexModeBackup = gSaveBlock2Ptr->pokedex.mode;
                gSaveBlock2Ptr->pokedex.order = GetSearchModeSelection(taskId, SEARCH_ORDER);
                sPokedexView->dexOrderBackup = gSaveBlock2Ptr->pokedex.order;
                PlaySE(SE_PC_OFF);
                gTasks[taskId].func = Task_ExitSearch;
            }
            else
            {
                EraseAndPrintSearchTextBox(gText_SearchingPleaseWait);
                gTasks[taskId].func = Task_StartPokedexSearch;
                PlaySE(SE_DEX_SEARCH);
                CopyWindowToVram(0, COPYWIN_GFX);
            }
        }
        else if (gTasks[taskId].tMenuItem == SEARCH_ALL)
        {
            PlaySE(SE_SELECT);
            sPokedexView->searchAll ^= TRUE;
            PrintSelectedSearchParameters(taskId);
            CopyWindowToVram(0, COPYWIN_GFX);
        }
        else if (gTasks[taskId].tMenuItem == SEARCH_BST_VALUE)
        {
            PlaySE(SE_PIN);
            gTasks[taskId].tCursorPos = sPokedexView->searchBstValue; // restored on B
            gTasks[taskId].func = Task_HandleSearchNumberInput;
        }
        else
        {
            PlaySE(SE_PIN);
            gTasks[taskId].func = Task_SelectSearchMenuItem;
        }
        return;
    }

    if (JOY_NEW(DPAD_LEFT) && movementMap[gTasks[taskId].tMenuItem][0] != 0xFF)
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].tMenuItem = movementMap[gTasks[taskId].tMenuItem][0];
        HighlightSelectedSearchMenuItem(gTasks[taskId].tTopBarItem, gTasks[taskId].tMenuItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
    if (JOY_NEW(DPAD_RIGHT) && movementMap[gTasks[taskId].tMenuItem][1] != 0xFF)
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].tMenuItem = movementMap[gTasks[taskId].tMenuItem][1];
        HighlightSelectedSearchMenuItem(gTasks[taskId].tTopBarItem, gTasks[taskId].tMenuItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
    if (JOY_NEW(DPAD_UP) && movementMap[gTasks[taskId].tMenuItem][2] != 0xFF)
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].tMenuItem = movementMap[gTasks[taskId].tMenuItem][2];
        HighlightSelectedSearchMenuItem(gTasks[taskId].tTopBarItem, gTasks[taskId].tMenuItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
    if (JOY_NEW(DPAD_DOWN) && movementMap[gTasks[taskId].tMenuItem][3] != 0xFF)
    {
        PlaySE(SE_SELECT);
        gTasks[taskId].tMenuItem = movementMap[gTasks[taskId].tMenuItem][3];
        HighlightSelectedSearchMenuItem(gTasks[taskId].tTopBarItem, gTasks[taskId].tMenuItem);
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
    }
}

static void Task_StartPokedexSearch(u8 taskId)
{
    u8 dexMode = GetSearchModeSelection(taskId, SEARCH_MODE);
    u8 order = GetSearchModeSelection(taskId, SEARCH_ORDER);
    u8 abcGroup = GetSearchModeSelection(taskId, SEARCH_NAME);
    u8 bodyColor = GetSearchModeSelection(taskId, SEARCH_COLOR);
    u8 type1 = GetSearchModeSelection(taskId, SEARCH_TYPE_LEFT);
    u8 type2 = GetSearchModeSelection(taskId, SEARCH_TYPE_RIGHT);
    struct SearchExtraParams extra;
    u8 i;

    extra.bstStat = GetSearchSelectedIndex(taskId, SEARCH_BST_STAT);
    extra.bstOp = GetSearchSelectedIndex(taskId, SEARCH_BST_OP);
    extra.bstValue = sPokedexView->searchBstValue;
    extra.yieldStat = GetSearchSelectedIndex(taskId, SEARCH_YIELD);
    for (i = 0; i < ARRAY_COUNT(extra.moves); i++)
        extra.moves[i] = GetSearchMoveAt(GetSearchSelectedIndex(taskId, SEARCH_MOVE_1 + i));
    extra.all = sPokedexView->searchAll;

    DoPokedexSearch(dexMode, order, abcGroup, bodyColor, type1, type2, &extra);
    gTasks[taskId].func = Task_WaitAndCompleteSearch;
}

static void Task_WaitAndCompleteSearch(u8 taskId)
{
    if (!IsSEPlaying())
    {
        if (sPokedexView->pokemonListCount != 0)
        {
            PlaySE(SE_SUCCESS);
            EraseAndPrintSearchTextBox(gText_SearchCompleted);
        }
        else
        {
            PlaySE(SE_FAILURE);
            EraseAndPrintSearchTextBox(gText_NoMatchingPkmnWereFound);
        }
        gTasks[taskId].func = Task_SearchCompleteWaitForInput;
        CopyWindowToVram(0, COPYWIN_GFX);
    }
}

static void Task_SearchCompleteWaitForInput(u8 taskId)
{
    if (JOY_NEW(A_BUTTON))
    {
        if (sPokedexView->pokemonListCount != 0)
        {
            // Return to dex list and show search results
            sPokedexView->screenSwitchState = 1;
            sPokedexView->dexMode = GetSearchModeSelection(taskId, SEARCH_MODE);
            sPokedexView->dexOrder = GetSearchModeSelection(taskId, SEARCH_ORDER);
            gTasks[taskId].func = Task_ExitSearch;
            PlaySE(SE_PC_OFF);
        }
        else
        {
            gTasks[taskId].func = Task_SwitchToSearchMenu;
            PlaySE(SE_BALL);
        }
    }
}

static void Task_SelectSearchMenuItem(u8 taskId)
{
    u8 menuItem;
    u16 *cursorPos;
    u16 *scrollOffset;

    DrawOrEraseSearchParameterBox(FALSE);
    menuItem = gTasks[taskId].tMenuItem;
    cursorPos = GetSearchCursorPtr(taskId, menuItem);
    scrollOffset = GetSearchScrollPtr(taskId, menuItem);
    gTasks[taskId].tCursorPos = *cursorPos;
    gTasks[taskId].tScrollOffset = *scrollOffset;
    PrintSearchParameterText(taskId);
    PrintSelectorArrow(*cursorPos);
    gTasks[taskId].func = Task_HandleSearchParameterInput;
    CopyWindowToVram(0, COPYWIN_GFX);
    CopyBgTilemapBufferToVram(3);
}

// Editing the BST number in place: Up/Down by 1, Left/Right by 10
static void Task_HandleSearchNumberInput(u8 taskId)
{
    s16 value = sPokedexView->searchBstValue;
    s16 max = GetSearchBstMax(taskId);

    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_PIN);
        gTasks[taskId].func = Task_SwitchToSearchMenu;
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_BALL);
        sPokedexView->searchBstValue = gTasks[taskId].tCursorPos;
        gTasks[taskId].func = Task_SwitchToSearchMenu;
        return;
    }
    if (JOY_REPEAT(DPAD_UP))
        value++;
    else if (JOY_REPEAT(DPAD_DOWN))
        value--;
    else if (JOY_REPEAT(DPAD_RIGHT))
        value += 10;
    else if (JOY_REPEAT(DPAD_LEFT))
        value -= 10;
    if (value < 1)
        value = 1;
    if (value > max)
        value = max;
    if (value != sPokedexView->searchBstValue)
    {
        PlaySE(SE_SELECT);
        sPokedexView->searchBstValue = value;
        PrintSelectedSearchParameters(taskId);
        CopyWindowToVram(0, COPYWIN_GFX);
    }
}

// MOVE picker: L/R jump to the first move of the previous/next starting letter
static void JumpSearchMovePickerLetter(u8 taskId, s8 dir)
{
    u8 item = gTasks[taskId].tMenuItem;
    u16 *cursorPos = GetSearchCursorPtr(taskId, item);
    u16 *scrollOffset = GetSearchScrollPtr(taskId, item);
    u16 count = GetSearchOptionCount(item);
    u16 index = *cursorPos + *scrollOffset;
    u16 target = index;
    u8 letter;

    if (dir > 0)
    {
        letter = (index == 0) ? 0 : GetSearchOptionTitle(item, index)[0];
        while (target + 1 < count && (target == 0 || GetSearchOptionTitle(item, target)[0] == letter))
            target++;
    }
    else if (index > 1)
    {
        // start of this letter, or of the previous letter if already there
        letter = GetSearchOptionTitle(item, index)[0];
        while (target > 1 && GetSearchOptionTitle(item, target - 1)[0] == letter)
            target--;
        if (target == index)
        {
            target--;
            letter = GetSearchOptionTitle(item, target)[0];
            while (target > 1 && GetSearchOptionTitle(item, target - 1)[0] == letter)
                target--;
        }
    }
    else
    {
        target = 0;
    }
    if (target == index)
        return;

    EraseSelectorArrow(*cursorPos);
    *scrollOffset = target;
    if (*scrollOffset > count - 1 - MAX_SEARCH_PARAM_CURSOR_POS)
        *scrollOffset = count - 1 - MAX_SEARCH_PARAM_CURSOR_POS;
    *cursorPos = target - *scrollOffset;
    PrintSearchParameterText(taskId);
    PrintSelectorArrow(*cursorPos);
    PlaySE(SE_SELECT);
    CopyWindowToVram(0, COPYWIN_GFX);
}

// Input for scrolling parameter box in right column
static void Task_HandleSearchParameterInput(u8 taskId)
{
    u8 menuItem;
    u16 *cursorPos;
    u16 *scrollOffset;
    u16 maxOption;
    bool8 moved;

    menuItem = gTasks[taskId].tMenuItem;
    cursorPos = GetSearchCursorPtr(taskId, menuItem);
    scrollOffset = GetSearchScrollPtr(taskId, menuItem);
    maxOption = GetSearchOptionCount(menuItem) - 1;
    if (IS_SEARCH_MOVE(menuItem) && JOY_NEW(L_BUTTON | R_BUTTON))
    {
        JumpSearchMovePickerLetter(taskId, JOY_NEW(R_BUTTON) ? 1 : -1);
        return;
    }
    if (JOY_NEW(A_BUTTON))
    {
        PlaySE(SE_PIN);
        // Leaving TOTAL: keep the BST number within a single stat's range
        if (menuItem == SEARCH_BST_STAT && sPokedexView->searchBstValue > GetSearchBstMax(taskId))
            sPokedexView->searchBstValue = GetSearchBstMax(taskId);
        ClearSearchParameterBoxText();
        DrawOrEraseSearchParameterBox(TRUE);
        gTasks[taskId].func = Task_SwitchToSearchMenu;
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_BALL);
        ClearSearchParameterBoxText();
        DrawOrEraseSearchParameterBox(TRUE);
        *cursorPos = gTasks[taskId].tCursorPos;
        *scrollOffset = gTasks[taskId].tScrollOffset;
        gTasks[taskId].func = Task_SwitchToSearchMenu;
        CopyWindowToVram(0, COPYWIN_GFX);
        CopyBgTilemapBufferToVram(3);
        return;
    }
    moved = FALSE;
    if (JOY_REPEAT(DPAD_UP))
    {
        if (*cursorPos != 0)
        {
            // Move cursor up
            EraseSelectorArrow(*cursorPos);
            (*cursorPos)--;
            PrintSelectorArrow(*cursorPos);
            moved = TRUE;
        }
        else if (*scrollOffset != 0)
        {
            // Scroll up
            (*scrollOffset)--;
            PrintSearchParameterText(taskId);
            PrintSelectorArrow(*cursorPos);
            moved = TRUE;
        }
        if (moved)
        {
            PlaySE(SE_SELECT);
            EraseAndPrintSearchTextBox(GetSearchOptionDescription(menuItem, *cursorPos + *scrollOffset));
            CopyWindowToVram(0, COPYWIN_GFX);
        }
        return;
    }
    if (JOY_REPEAT(DPAD_DOWN))
    {
        if (*cursorPos < MAX_SEARCH_PARAM_CURSOR_POS && *cursorPos < maxOption)
        {
            // Move cursor down
            EraseSelectorArrow(*cursorPos);
            (*cursorPos)++;
            PrintSelectorArrow(*cursorPos);
            moved = TRUE;
        }
        else if (maxOption > MAX_SEARCH_PARAM_CURSOR_POS && *scrollOffset < maxOption - MAX_SEARCH_PARAM_CURSOR_POS)
        {
            // Scroll down
            (*scrollOffset)++;
            PrintSearchParameterText(taskId);
            PrintSelectorArrow(5);
            moved = TRUE;
        }
        if (moved)
        {
            PlaySE(SE_SELECT);
            EraseAndPrintSearchTextBox(GetSearchOptionDescription(menuItem, *cursorPos + *scrollOffset));
            CopyWindowToVram(0, COPYWIN_GFX);
        }
        return;
    }
}

static void Task_ExitSearch(u8 taskId)
{
    BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
    gTasks[taskId].func = Task_ExitSearchWaitForFade;
}

static void Task_ExitSearchWaitForFade(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        FreeSearchWindowAndBgBuffers();
        DestroyTask(taskId);
    }
}

static void SetSearchRectHighlight(u8 flags, u8 x, u8 y, u8 width)
{
    u16 i;
    u16 temp; //should be a pointer, but does not match as one
    u32 ptr = (u32)GetBgTilemapBuffer(3); //same as above

    for (i = 0; i < width; i++)
    {
        temp = *(u16 *)(ptr + (y + 0) * 64 + (x + i) * 2);
        temp &= 0x0fff;
        temp |= (flags << 12);
        *(u16 *)(ptr + (y + 0) * 64 + (x + i) * 2) = temp;

        temp = *(u16 *)(ptr + (y + 1) * 64 + (x + i) * 2);
        temp &= 0x0fff;
        temp |= (flags << 12);
        *(u16 *)(ptr + (y + 1) * 64 + (x + i) * 2) = temp;
    }
}


#define SEARCH_BG_SEARCH                SEARCH_TOPBAR_SEARCH
#define SEARCH_BG_SHIFT                 SEARCH_TOPBAR_SHIFT
#define SEARCH_BG_CANCEL                SEARCH_TOPBAR_CANCEL
#define SEARCH_BG_NAME                  (SEARCH_NAME + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_COLOR                 (SEARCH_COLOR + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_TYPE_SELECTION_LEFT   (SEARCH_TYPE_LEFT + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_TYPE_SELECTION_RIGHT  (SEARCH_TYPE_RIGHT + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_ORDER                 (SEARCH_ORDER + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_MODE                  (SEARCH_MODE + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_OK                    (SEARCH_OK + SEARCH_TOPBAR_COUNT)
#define SEARCH_BG_TYPE_TITLE            (SEARCH_COUNT + SEARCH_TOPBAR_COUNT)

static void DrawSearchMenuItemBgHighlight(u8 searchBg, bool8 unselected, bool8 disabled)
{
    u8 highlightFlags = (unselected & 1) | ((disabled & 1) << 1);

    // Right-hand column items and ALL: title pill and value box
    if (searchBg >= SEARCH_EXTRA_FIRST + SEARCH_TOPBAR_COUNT && searchBg < SEARCH_COUNT + SEARCH_TOPBAR_COUNT)
    {
        const struct SearchMenuItem *item = &sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT];
        u8 dy = (searchBg == SEARCH_ALL + SEARCH_TOPBAR_COUNT && !IsNationalPokedexEnabled()) ? 2 : 0;

        SetSearchRectHighlight(highlightFlags, item->titleBgX, item->titleBgY - dy, item->titleBgWidth);
        SetSearchRectHighlight(highlightFlags, item->selectionBgX, item->selectionBgY - dy, item->selectionBgWidth);
        return;
    }
    switch (searchBg)
    {
    case SEARCH_BG_SEARCH:
    case SEARCH_BG_SHIFT:
    case SEARCH_BG_CANCEL:
        SetSearchRectHighlight(highlightFlags, sSearchMenuTopBarItems[searchBg].highlightX, sSearchMenuTopBarItems[searchBg].highlightY, sSearchMenuTopBarItems[searchBg].highlightWidth);
        break;
    case SEARCH_BG_NAME:
    case SEARCH_BG_COLOR:
    case SEARCH_BG_ORDER:
    case SEARCH_BG_MODE:
        SetSearchRectHighlight(highlightFlags, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgX, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgY, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgWidth);
        // fall through, draw selectionBg for above
    case SEARCH_BG_TYPE_SELECTION_LEFT:
    case SEARCH_BG_TYPE_SELECTION_RIGHT:
        SetSearchRectHighlight(highlightFlags, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].selectionBgX, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].selectionBgY, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].selectionBgWidth);
        break;
    case SEARCH_BG_TYPE_TITLE:
        SetSearchRectHighlight(highlightFlags, sSearchMenuItems[SEARCH_TYPE_LEFT].titleBgX, sSearchMenuItems[SEARCH_TYPE_LEFT].titleBgY, sSearchMenuItems[SEARCH_TYPE_LEFT].titleBgWidth);
        break;
    case SEARCH_BG_OK:
        if (!IsNationalPokedexEnabled())
            SetSearchRectHighlight(highlightFlags, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgX, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgY - 2, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgWidth);
        else
            SetSearchRectHighlight(highlightFlags, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgX, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgY, sSearchMenuItems[searchBg - SEARCH_TOPBAR_COUNT].titleBgWidth);
        break;
    }
}

// Right-hand search fields and ALL, unselected (and greyed out while SHIFT/CANCEL is chosen)
static void DrawSearchExtraItemsBg(bool8 disabled)
{
    u8 item;

    for (item = SEARCH_EXTRA_FIRST; item < SEARCH_COUNT; item++)
        DrawSearchMenuItemBgHighlight(item + SEARCH_TOPBAR_COUNT, TRUE, disabled);
}

static void SetInitialSearchMenuBgHighlights(u8 topBarItem)
{
    switch (topBarItem)
    {
    case SEARCH_TOPBAR_SEARCH:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SEARCH, FALSE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SHIFT, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_CANCEL, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_NAME, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_COLOR, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_TITLE, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_LEFT, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_RIGHT, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_ORDER, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_MODE, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_OK, TRUE, FALSE);
        DrawSearchExtraItemsBg(FALSE);
        break;
    case SEARCH_TOPBAR_SHIFT:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SEARCH, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SHIFT, FALSE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_CANCEL, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_NAME, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_COLOR, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_TITLE, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_LEFT, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_RIGHT, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_ORDER, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_MODE, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_OK, TRUE, FALSE);
        DrawSearchExtraItemsBg(TRUE);
        break;
    case SEARCH_TOPBAR_CANCEL:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SEARCH, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_SHIFT, TRUE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_CANCEL, FALSE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_NAME, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_COLOR, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_TITLE, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_LEFT, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_RIGHT, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_ORDER, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_MODE, TRUE, TRUE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_OK, TRUE, TRUE);
        DrawSearchExtraItemsBg(TRUE);
        break;
    }
}

static void HighlightSelectedSearchTopBarItem(u8 topBarItem)
{
    SetInitialSearchMenuBgHighlights(topBarItem);
    EraseAndPrintSearchTextBox(sSearchMenuTopBarItems[topBarItem].description);
}

static void HighlightSelectedSearchMenuItem(u8 topBarItem, u8 menuItem)
{
    SetInitialSearchMenuBgHighlights(topBarItem);
    switch (menuItem)
    {
    case SEARCH_NAME:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_NAME, FALSE, FALSE);
        break;
    case SEARCH_COLOR:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_COLOR, FALSE, FALSE);
        break;
    case SEARCH_TYPE_LEFT:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_TITLE, FALSE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_LEFT, FALSE, FALSE);
        break;
    case SEARCH_TYPE_RIGHT:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_TITLE, FALSE, FALSE);
        DrawSearchMenuItemBgHighlight(SEARCH_BG_TYPE_SELECTION_RIGHT, FALSE, FALSE);
        break;
    case SEARCH_ORDER:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_ORDER, FALSE, FALSE);
        break;
    case SEARCH_MODE:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_MODE, FALSE, FALSE);
        break;
    case SEARCH_OK:
        DrawSearchMenuItemBgHighlight(SEARCH_BG_OK, FALSE, FALSE);
        break;
    default:
        DrawSearchMenuItemBgHighlight(menuItem + SEARCH_TOPBAR_COUNT, FALSE, FALSE);
        break;
    }
    EraseAndPrintSearchTextBox(sSearchMenuItems[menuItem].description);
}

// Prints the currently selected search parameters in the search menu selection boxes
static void PrintSelectedSearchParameters(u8 taskId)
{
    u16 searchParamId;

    ClearSearchMenuRect(40, 16, 96, 80);

    searchParamId = gTasks[taskId].tCursorPos_Name + gTasks[taskId].tScrollOffset_Name;
    PrintSearchText(sDexSearchNameOptions[searchParamId].title, 0x2D, 0x11);

    searchParamId = gTasks[taskId].tCursorPos_Color + gTasks[taskId].tScrollOffset_Color;
    PrintSearchText(sDexSearchColorOptions[searchParamId].title, 0x2D, 0x21);

    searchParamId = gTasks[taskId].tCursorPos_TypeLeft + gTasks[taskId].tScrollOffset_TypeLeft;
    PrintSearchText(sDexSearchTypeOptions[searchParamId].title, 0x2D, 0x31);

    searchParamId = gTasks[taskId].tCursorPos_TypeRight + gTasks[taskId].tScrollOffset_TypeRight;
    PrintSearchText(sDexSearchTypeOptions[searchParamId].title, 0x5D, 0x31);

    searchParamId = gTasks[taskId].tCursorPos_Order + gTasks[taskId].tScrollOffset_Order;
    PrintSearchText(sDexOrderOptions[searchParamId].title, 0x2D, 0x41);

    if (IsNationalPokedexEnabled())
    {
        searchParamId = gTasks[taskId].tCursorPos_Mode + gTasks[taskId].tScrollOffset_Mode;
        PrintSearchText(sDexModeOptions[searchParamId].title, 0x2D, 0x51);
    }

    PrintSelectedSearchExtras(taskId);
}

static void PrintSearchTextNarrow(const u8 *str, u32 x, u32 y)
{
    static const u8 sColor[] = {TEXT_COLOR_TRANSPARENT, TEXT_DYNAMIC_COLOR_6, TEXT_COLOR_DARK_GRAY};

    AddTextPrinterParameterized4(0, FONT_NARROW, x, y, 0, 0, sColor, TEXT_SKIP_DRAW, str);
}

// Values of the right-hand search column and ALL (box positions match sSearchMenuItems)
static void PrintSelectedSearchExtras(u8 taskId)
{
    u8 str[8];
    u8 i;
    u8 allY = IsNationalPokedexEnabled() ? 12 * 8 : 10 * 8;

    ClearSearchMenuRect(168, 16, 72, 96);
    ClearSearchMenuRect(72, allY, 32, 16);

    PrintSearchTextNarrow(GetSearchOptionTitle(SEARCH_BST_STAT, GetSearchSelectedIndex(taskId, SEARCH_BST_STAT)), 21 * 8 + 4, 2 * 8 + 1);
    PrintSearchTextNarrow(GetSearchOptionTitle(SEARCH_BST_OP, GetSearchSelectedIndex(taskId, SEARCH_BST_OP)), 24 * 8 + 4, 2 * 8 + 1);
    ConvertIntToDecimalStringN(str, sPokedexView->searchBstValue, STR_CONV_MODE_LEFT_ALIGN, 3);
    PrintSearchTextNarrow(str, 27 * 8 + 4, 2 * 8 + 1);
    PrintSearchTextNarrow(GetSearchOptionTitle(SEARCH_YIELD, GetSearchSelectedIndex(taskId, SEARCH_YIELD)), 21 * 8 + 4, 4 * 8 + 1);
    for (i = 0; i < 4; i++)
        PrintSearchTextNarrow(GetSearchOptionTitle(SEARCH_MOVE_1 + i, GetSearchSelectedIndex(taskId, SEARCH_MOVE_1 + i)), 21 * 8 + 4, (6 + i * 2) * 8 + 1);
    PrintSearchTextNarrow(sPokedexView->searchAll ? sText_SearchOn : sText_SearchOff, 9 * 8 + 4, allY + 1);
}

static void DrawOrEraseSearchParameterBox(bool8 erase)
{
    u16 i;
    u16 j;
    u16 *ptr = GetBgTilemapBuffer(3);

    if (!erase)
    {
        *(ptr + 0x11) = 0xC0B;
        for (i = 0x12; i < 0x1F; i++)
            *(ptr + i) = 0x80D;
        for (j = 1; j < 13; j++)
        {
            *(ptr + 0x11 + j * 32) = 0x40A;
            for (i = 0x12; i < 0x1F; i++)
                *(ptr + j * 32 + i) = 2;
        }
        *(ptr + 0x1B1) = 0x40B;
        for (i = 0x12; i < 0x1F; i++)
            *(ptr + 0x1A0 + i) = 0xD;
    }
    else
    {
        // The box sits on the right-hand search fields; put the screen's art back
        LoadSearchMenuTilemap();
    }
}

// Prints the currently viewable search parameter titles in the right-hand text box
// and the currently selected search parameter description in the bottom text box
static void PrintSearchParameterText(u8 taskId)
{
    u8 menuItem = gTasks[taskId].tMenuItem;
    const u16 *cursorPos = GetSearchCursorPtr(taskId, menuItem);
    const u16 *scrollOffset = GetSearchScrollPtr(taskId, menuItem);
    u16 count = GetSearchOptionCount(menuItem);
    u16 i;
    u16 j;

    ClearSearchParameterBoxText();

    for (i = 0, j = *scrollOffset; i < MAX_SEARCH_PARAM_ON_SCREEN && j < count; i++, j++)
        PrintSearchParameterTitle(i, GetSearchOptionTitle(menuItem, j));

    EraseAndPrintSearchTextBox(GetSearchOptionDescription(menuItem, *cursorPos + *scrollOffset));
}

static u8 GetSearchModeSelection(u8 taskId, u8 option)
{
    const u16 *cursorPos = &gTasks[taskId].data[sSearchOptions[option].taskDataCursorPos];
    const u16 *scrollOffset = &gTasks[taskId].data[sSearchOptions[option].taskDataScrollOffset];
    u16 id = *cursorPos + *scrollOffset;

    switch (option)
    {
    default:
        return 0;
    case SEARCH_MODE:
        return sPokedexModes[id];
    case SEARCH_ORDER:
        return sOrderOptions[id];
    case SEARCH_NAME:
        if (id == 0)
            return 0xFF;
        else
            return id;
    case SEARCH_COLOR:
        if (id == 0)
            return 0xFF;
        else
            return id - 1;
    case SEARCH_TYPE_LEFT:
    case SEARCH_TYPE_RIGHT:
        return sDexSearchTypeIds[id];
    }
}

static void SetDefaultSearchModeAndOrder(u8 taskId)
{
    u16 selected;

    switch (sPokedexView->dexModeBackup)
    {
    default:
    case DEX_MODE_HOENN:
        selected = DEX_MODE_HOENN;
        break;
    case DEX_MODE_NATIONAL:
        selected = DEX_MODE_NATIONAL;
        break;
    }
    gTasks[taskId].tCursorPos_Mode = selected;

    switch (sPokedexView->dexOrderBackup)
    {
    default:
    case ORDER_NUMERICAL:
        selected = ORDER_NUMERICAL;
        break;
    case ORDER_ALPHABETICAL:
        selected = ORDER_ALPHABETICAL;
        break;
    case ORDER_HEAVIEST:
        selected = ORDER_HEAVIEST;
        break;
    case ORDER_LIGHTEST:
        selected = ORDER_LIGHTEST;
        break;
    case ORDER_TALLEST:
        selected = ORDER_TALLEST;
        break;
    case ORDER_SMALLEST:
        selected = ORDER_SMALLEST;
        break;
    }
    gTasks[taskId].tCursorPos_Order = selected;
}

static bool8 SearchParamCantScrollUp(u8 taskId)
{
    u8 menuItem = gTasks[taskId].tMenuItem;
    const u16 *scrollOffset = GetSearchScrollPtr(taskId, menuItem);
    u16 lastOption = GetSearchOptionCount(menuItem) - 1;

    if (lastOption > MAX_SEARCH_PARAM_CURSOR_POS && *scrollOffset != 0)
        return FALSE;
    else
        return TRUE;
}

static bool8 SearchParamCantScrollDown(u8 taskId)
{
    u8 menuItem = gTasks[taskId].tMenuItem;
    const u16 *scrollOffset = GetSearchScrollPtr(taskId, menuItem);
    u16 lastOption = GetSearchOptionCount(menuItem) - 1;

    if (lastOption > MAX_SEARCH_PARAM_CURSOR_POS && *scrollOffset < lastOption - MAX_SEARCH_PARAM_CURSOR_POS)
        return FALSE;
    else
        return TRUE;
}

#define sTaskId      data[0]

static void SpriteCB_SearchParameterScrollArrow(struct Sprite *sprite)
{
    if (gTasks[sprite->sTaskId].func == Task_HandleSearchParameterInput)
    {
        u8 val;

        if (sprite->sIsDownArrow)
        {
            if (SearchParamCantScrollDown(sprite->sTaskId))
                sprite->invisible = TRUE;
            else
                sprite->invisible = FALSE;
        }
        else
        {
            if (SearchParamCantScrollUp(sprite->sTaskId))
                sprite->invisible = TRUE;
            else
                sprite->invisible = FALSE;
        }
        val = sprite->data[2] + sprite->sIsDownArrow * 128;
        sprite->y2 = gSineTable[val] / 128;
        sprite->data[2] += 8;
    }
    else
    {
        sprite->invisible = TRUE;
    }
}

static void CreateSearchParameterScrollArrows(u8 taskId)
{
    u8 spriteId;

    spriteId = CreateSprite(&sScrollArrowSpriteTemplate, 184, 4, 0);
    gSprites[spriteId].sTaskId = taskId;
    gSprites[spriteId].sIsDownArrow = FALSE;
    gSprites[spriteId].callback = SpriteCB_SearchParameterScrollArrow;

    spriteId = CreateSprite(&sScrollArrowSpriteTemplate, 184, 108, 0);
    gSprites[spriteId].sTaskId = taskId;
    gSprites[spriteId].sIsDownArrow = TRUE;
    gSprites[spriteId].vFlip = TRUE;
    gSprites[spriteId].callback = SpriteCB_SearchParameterScrollArrow;
}

#undef sTaskId
#undef sIsDownArrow

static void EraseAndPrintSearchTextBox(const u8 *str)
{
    ClearSearchMenuRect(8, 120, 224, 32);
    PrintSearchText(str, 8, 121);
}

static void EraseSelectorArrow(u32 y)
{
    ClearSearchMenuRect(144, y * 16 + 8, 8, 16);
}

static void PrintSelectorArrow(u32 y)
{
    PrintSearchText(gText_SelectorArrow, 144, y * 16 + 9);
}

static void PrintSearchParameterTitle(u32 y, const u8 *str)
{
    PrintSearchText(str, 152, y * 16 + 9);
}

static void ClearSearchParameterBoxText(void)
{
    ClearSearchMenuRect(136, 8, 104, 104);
}
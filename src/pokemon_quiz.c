// Pokemon Speed Quiz minigame - 10 question speed-only mode.
// Hosted by the Trainer School woman in Viridian City and by the Rustboro
// Pokemon School teacher (after he gives the Quick Claw). The player is shown
// four GROUPS of 1-4 random Pokemon (rendered as party icons) and must
// pick the group with the highest base Speed, or "Same" if all four tie.
// The hovered group is highlighted and its icons animate. After picking,
// the correct answer turns green, a wrong pick turns red, and every group's
// base Speed is overlaid on its icons. Completing the quiz awards a random
// vitamin (HP Up / Protein / Iron / Carbos / Calcium / Zinc).

#include "global.h"
#include "data.h"
#include "event_data.h"
#include "main.h"
#include "menu.h"
#include "palette.h"
#include "pokemon.h"
#include "pokemon_icon.h"
#include "random.h"
#include "script.h"
#include "script_menu.h"
#include "sound.h"
#include "sprite.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "window.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/pokedex.h"
#include "constants/rgb.h"
#include "constants/script_menu.h"
#include "constants/songs.h"
#include "constants/species.h"

#define POKEMON_QUIZ_QUESTION_COUNT 10
#define POKEMON_QUIZ_GROUP_COUNT    4
#define POKEMON_QUIZ_MAX_PER_GROUP  4
#define POKEMON_QUIZ_SAME_INDEX     POKEMON_QUIZ_GROUP_COUNT // 4
#define POKEMON_QUIZ_NO_PICK        0xFF

// Panel background fills (indices into the dialog text palette, BG pal 15).
// Index 10 is a duplicate white; it is recolored red while the panel is open.
#define QUIZ_FILL_HOVER   TEXT_COLOR_LIGHT_BLUE
#define QUIZ_FILL_CORRECT TEXT_COLOR_LIGHT_GREEN
#define QUIZ_FILL_WRONG   10
#define QUIZ_WRONG_RGB    RGB(31, 18, 18)

#define TAG_QUIZ_BADGE_PAL   0x7A00
#define TAG_QUIZ_BADGE_TILES 0x7A01 // + group index
#define QUIZ_BADGE_WIDTH     32
#define QUIZ_BADGE_HEIGHT    16
#define QUIZ_PANEL_SCREEN_LEFT 8 // panel window interior starts at tile column 1

static const u8 sQuizOptionSame[] = _("Same");
static const u8 sQuizOptionBlank[] = _(" ");
static const u8 sQuizSpeedSep[]   = _(" ");
static const u8 sQuizMultSep[]    = _(" x ");
static const u8 sQuizSameLabel[]  = _("Same total!");

// 2x2 group anchors aligned with the custom panel.
// Shifted +2 px right and -2 px up from the original layout per playtest.
static const s16 sQuizGroupAnchorX[POKEMON_QUIZ_GROUP_COUNT] = { 69, 69, 182, 182 };
static const s16 sQuizGroupAnchorY[POKEMON_QUIZ_GROUP_COUNT] = { 115, 138, 115, 138 };

// X offsets within a group for 1, 2, 3, or 4 icons. Icon sprites are 32x32
// but the visible Pokemon area is ~24x24. Groups of up to 3 sit 26 px apart;
// 4-mon groups are packed 22 px apart so the revealed speed badge fits to the
// left of every group without covering an icon.
static const s16 sQuizGroupOffsets[POKEMON_QUIZ_MAX_PER_GROUP][POKEMON_QUIZ_MAX_PER_GROUP] = {
    {   0,   0,   0,   0 }, // N = 1
    { -13, +13,   0,   0 }, // N = 2
    { -26,   0, +26,   0 }, // N = 3
    { -33, -11, +11, +33 }, // N = 4
};
#define QUIZ_ICON_HALF_WIDTH 12

// IMPORTANT: keep these in EWRAM, not the default BSS bucket. The qol
// branch's IWRAM is already at ~96% utilization; pushing additional BSS
// into IWRAM shifts existing IWRAM allocations and exposes a latent
// reshow-battle-screen bug that corrupts the BG palette on bag-return
// (cyan tint + magenta speckles). Routing this module's persistent state
// through EWRAM keeps IWRAM layout stable.
EWRAM_DATA static u8 sQuizQuestionsAsked = 0;
EWRAM_DATA static u8 sQuizScore = 0;
EWRAM_DATA static u8 sQuizCorrectChoice = 0;
EWRAM_DATA static u16 sQuizSpecies[POKEMON_QUIZ_GROUP_COUNT][POKEMON_QUIZ_MAX_PER_GROUP] = {0};
EWRAM_DATA static u8 sQuizGroupSize[POKEMON_QUIZ_GROUP_COUNT] = {0};
EWRAM_DATA static u16 sQuizGroupSpeed[POKEMON_QUIZ_GROUP_COUNT] = {0};
EWRAM_DATA static struct MenuAction sQuizMenuItems[POKEMON_QUIZ_GROUP_COUNT + 2] = {0}; // +1 for Same, +1 padding for 2x3 grid
EWRAM_DATA static u8 sQuizIconSpriteIds[POKEMON_QUIZ_GROUP_COUNT][POKEMON_QUIZ_MAX_PER_GROUP] = {0};
EWRAM_DATA static bool8 sQuizIconsLoaded = FALSE;
EWRAM_DATA static u8 sQuizPanelWindowId = 0;
EWRAM_DATA static bool8 sQuizPanelOpen = FALSE;
EWRAM_DATA static u8 sQuizPickedChoice = 0; // POKEMON_QUIZ_NO_PICK until the answer is revealed
EWRAM_DATA static u8 sQuizBadgeSpriteIds[POKEMON_QUIZ_GROUP_COUNT] = {0};

// Custom panel window: same horizontal extent as the standard dialog box but
// 4 tiles taller, sitting flush against the bottom of the screen.
// Standard dialog box uses tilemapTop=15, height=4 (rows 15..18).
// Ours uses tilemapTop=11, height=8 (rows 11..18) -> 4 tiles taller.
static const struct WindowTemplate sQuizPanelWindowTemplate = {
    .bg = 0,
    .tilemapLeft = 1,
    .tilemapTop = 11,
    .width = 28,
    .height = 8,
    .paletteNum = 15,
    .baseBlock = 0x40,
};

static const u8 sQuizPanelTitle[] = _("Fastest group?");

static u16 PickRandomQuizSpecies(void)
{
    u16 dexNum;
    u16 species;

    // Restrict to species 1..386 National Dex (excludes "??????" placeholders).
    do
    {
        dexNum = (Random() % NATIONAL_DEX_DEOXYS) + 1;
        species = NationalPokedexNumToSpecies(dexNum);
    } while (species == SPECIES_NONE || species == SPECIES_EGG);

    return species;
}

static void ResetQuizIconIds(void)
{
    u8 g, i;
    for (g = 0; g < POKEMON_QUIZ_GROUP_COUNT; g++)
    {
        for (i = 0; i < POKEMON_QUIZ_MAX_PER_GROUP; i++)
            sQuizIconSpriteIds[g][i] = MAX_SPRITES;
        sQuizBadgeSpriteIds[g] = MAX_SPRITES;
    }
}

void StartPokemonQuizSpeed(void)
{
    sQuizQuestionsAsked = 0;
    sQuizScore = 0;
    sQuizCorrectChoice = 0;
    sQuizPickedChoice = POKEMON_QUIZ_NO_PICK;
    sQuizIconsLoaded = FALSE;
    sQuizPanelOpen = FALSE;
    ResetQuizIconIds();
}

void OpenPokemonQuizPanel(void)
{
    static const u16 sWrongColor = QUIZ_WRONG_RGB;

    if (sQuizPanelOpen)
        return;

    LoadPalette(&sWrongColor, BG_PLTT_ID(15) + QUIZ_FILL_WRONG, PLTT_SIZEOF(1));
    sQuizPanelWindowId = AddWindow(&sQuizPanelWindowTemplate);
    sQuizPanelOpen = TRUE;
    DrawStdWindowFrame(sQuizPanelWindowId, FALSE);
    FillWindowPixelBuffer(sQuizPanelWindowId, PIXEL_FILL(1));

    // Title across the top. The cursor (drawn dynamically by
    // ShowPokemonQuizChoiceMenu) is the only other panel content.
    AddTextPrinterParameterized(sQuizPanelWindowId, FONT_NORMAL, sQuizPanelTitle, 4, 0, 0xFF, NULL);

    PutWindowTilemap(sQuizPanelWindowId);
    CopyWindowToVram(sQuizPanelWindowId, COPYWIN_FULL);
}

void ClosePokemonQuizPanel(void)
{
    if (!sQuizPanelOpen)
        return;

    ClearStdWindowAndFrame(sQuizPanelWindowId, TRUE);
    RemoveWindow(sQuizPanelWindowId);
    LoadPalette(&GetOverworldTextboxPalettePtr()[QUIZ_FILL_WRONG], BG_PLTT_ID(15) + QUIZ_FILL_WRONG, PLTT_SIZEOF(1));
    sQuizPanelOpen = FALSE;
}

static void DestroyQuizSpeedBadges(void)
{
    u8 g;

    for (g = 0; g < POKEMON_QUIZ_GROUP_COUNT; g++)
    {
        if (sQuizBadgeSpriteIds[g] != MAX_SPRITES)
        {
            DestroySprite(&gSprites[sQuizBadgeSpriteIds[g]]);
            FreeSpriteTilesByTag(TAG_QUIZ_BADGE_TILES + g);
            sQuizBadgeSpriteIds[g] = MAX_SPRITES;
        }
    }
    FreeSpritePaletteByTag(TAG_QUIZ_BADGE_PAL);
}

void DestroyPokemonQuizIcons(void)
{
    u8 g, i;

    DestroyQuizSpeedBadges();
    if (!sQuizIconsLoaded)
        return;

    for (g = 0; g < POKEMON_QUIZ_GROUP_COUNT; g++)
    {
        for (i = 0; i < POKEMON_QUIZ_MAX_PER_GROUP; i++)
        {
            if (sQuizIconSpriteIds[g][i] != MAX_SPRITES)
            {
                FreeAndDestroyMonIconSprite(&gSprites[sQuizIconSpriteIds[g][i]]);
                sQuizIconSpriteIds[g][i] = MAX_SPRITES;
            }
        }
    }
    FreeMonIconPalettes();
    sQuizIconsLoaded = FALSE;
}

void CreatePokemonQuizIcons(void)
{
    u8 g, i;

    DestroyPokemonQuizIcons();
    LoadMonIconPalettes();
    for (g = 0; g < POKEMON_QUIZ_GROUP_COUNT; g++)
    {
        u8 n = sQuizGroupSize[g];
        for (i = 0; i < n; i++)
        {
            s16 x = sQuizGroupAnchorX[g] + sQuizGroupOffsets[n - 1][i];
            s16 y = sQuizGroupAnchorY[g];
            // Subpriority 1 so the speed badges (subpriority 0) draw on top.
            sQuizIconSpriteIds[g][i] = CreateMonIcon(sQuizSpecies[g][i],
                                                    SpriteCallbackDummy,
                                                    x, y, 1, 0, FALSE);
            if (sQuizIconSpriteIds[g][i] != MAX_SPRITES)
                gSprites[sQuizIconSpriteIds[g][i]].oam.priority = 0;
        }
    }
    sQuizIconsLoaded = TRUE;
}

// Only the hovered group's icons animate; everything else holds still.
static void SetQuizIconAnimation(u8 activeGroup)
{
    u8 g, i;

    for (g = 0; g < POKEMON_QUIZ_GROUP_COUNT; g++)
    {
        for (i = 0; i < POKEMON_QUIZ_MAX_PER_GROUP; i++)
        {
            u8 spriteId = sQuizIconSpriteIds[g][i];
            if (spriteId == MAX_SPRITES)
                continue;
            gSprites[spriteId].callback = (g == activeGroup) ? SpriteCB_MonIcon : SpriteCallbackDummy;
        }
    }
}

// Screen X of the first and last columns of a group's visible icon art.
static s16 GetQuizGroupLeftEdge(u8 g)
{
    u8 n = sQuizGroupSize[g] ? sQuizGroupSize[g] : 1;
    return sQuizGroupAnchorX[g] + sQuizGroupOffsets[n - 1][0] - QUIZ_ICON_HALF_WIDTH;
}

static s16 GetQuizGroupRightEdge(u8 g)
{
    u8 n = sQuizGroupSize[g] ? sQuizGroupSize[g] : 1;
    return sQuizGroupAnchorX[g] + sQuizGroupOffsets[n - 1][n - 1] + QUIZ_ICON_HALF_WIDTH - 1;
}

static const struct OamData sOam_QuizBadge = {
    .shape = SPRITE_SHAPE(32x16),
    .size = SPRITE_SIZE(32x16),
    .priority = 0,
};

// Each badge is a small outlined label with the group's base Speed, rendered
// with the text engine into a scratch window and then uploaded as sprite tiles
// so it can sit on top of the mon icons. Window and 1D sprite tiles share the
// same row-major 4bpp layout, so the pixel buffer is copied as-is.
static void CreateQuizSpeedBadges(void)
{
    static const u8 sBadgeTextColors[3] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
    static const struct WindowTemplate sBadgeWindowTemplate = {
        .bg = 0,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = QUIZ_BADGE_WIDTH / 8,
        .height = QUIZ_BADGE_HEIGHT / 8,
        .paletteNum = 15,
        .baseBlock = 0, // never copied to VRAM
    };
    struct SpritePalette palette = {GetOverworldTextboxPalettePtr(), TAG_QUIZ_BADGE_PAL};
    struct SpriteTemplate template = {
        .tileTag = 0,
        .paletteTag = TAG_QUIZ_BADGE_PAL,
        .oam = &sOam_QuizBadge,
        .anims = gDummySpriteAnimTable,
        .images = NULL,
        .affineAnims = gDummySpriteAffineAnimTable,
        .callback = SpriteCallbackDummy,
    };
    u8 g;

    DestroyQuizSpeedBadges();
    if (LoadSpritePalette(&palette) == 0xFF)
        return;

    for (g = 0; g < POKEMON_QUIZ_GROUP_COUNT; g++)
    {
        u8 text[8];
        u8 windowId;
        u8 textWidth, boxWidth;
        s16 badgeLeft, minLeft;
        struct SpriteSheet sheet;

        if (sQuizGroupSize[g] == 0)
            continue;

        windowId = AddWindow(&sBadgeWindowTemplate);
        if (windowId == WINDOW_NONE)
            break;

        ConvertIntToDecimalStringN(text, sQuizGroupSpeed[g], STR_CONV_MODE_LEFT_ALIGN, 3);
        // Digit glyphs include a trailing blank column, so 1 px of left padding
        // plus the two outline columns gives a balanced box.
        textWidth = GetStringWidth(FONT_SMALL, text, 0);
        boxWidth = textWidth + 3;

        // FONT_SMALL glyph cells are 13 px tall: text rows 2..14, outline rows 1 and 15.
        // The box is left-aligned in the sprite; the rest stays transparent.
        FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
        FillWindowPixelRect(windowId, PIXEL_FILL(TEXT_COLOR_DARK_GRAY), 0, 1, boxWidth, 15);
        FillWindowPixelRect(windowId, PIXEL_FILL(TEXT_COLOR_WHITE), 1, 2, boxWidth - 2, 13);
        AddTextPrinterParameterized3(windowId, FONT_SMALL, 2, 2, sBadgeTextColors, TEXT_SKIP_DRAW, text);

        // Sit just left of the group's first icon, but never past the panel's
        // left edge or onto the left-column group sharing this row.
        badgeLeft = GetQuizGroupLeftEdge(g) - boxWidth;
        minLeft = (g >= 2) ? GetQuizGroupRightEdge(g - 2) + 1 : QUIZ_PANEL_SCREEN_LEFT;
        if (badgeLeft < minLeft)
            badgeLeft = minLeft;

        sheet.data = (const void *)GetWindowAttribute(windowId, WINDOW_TILE_DATA);
        sheet.size = QUIZ_BADGE_WIDTH * QUIZ_BADGE_HEIGHT / 2;
        sheet.tag = TAG_QUIZ_BADGE_TILES + g;
        if (LoadSpriteSheet(&sheet) != 0)
        {
            template.tileTag = sheet.tag;
            sQuizBadgeSpriteIds[g] = CreateSprite(&template, badgeLeft + QUIZ_BADGE_WIDTH / 2, sQuizGroupAnchorY[g], 0);
            if (sQuizBadgeSpriteIds[g] == MAX_SPRITES)
                FreeSpriteTilesByTag(sheet.tag);
        }
        RemoveWindow(windowId);
    }
}

// Builds 4 groups where every Pokemon within a group shares the same base
// Speed (mirroring the original Pokemon Quiz program). Group sizes shrink
// as the question count climbs: early questions favor 4-mon groups, the
// last question forces 1-mon groups.
//
// Answer rules:
//   - "Same" is the answer ONLY when ALL FOUR groups share the same base
//     Speed. We force this with ~20% probability per question.
//   - Otherwise the four groups are guaranteed to have FOUR DISTINCT base
//     Speeds, and the answer is the group with the highest of them.
void GeneratePokemonQuizQuestion(void)
{
    u8 g, i, s;
    u16 used[POKEMON_QUIZ_GROUP_COUNT * POKEMON_QUIZ_MAX_PER_GROUP];
    u8 usedCount = 0;
    u8 questionNum = sQuizQuestionsAsked + 1; // 1..10
    u16 maxSpeed;
    u8 maxIndex;
    bool8 makeSame = (Random() % 5) == 0; // ~20% chance the answer is "Same"
    u16 forcedSpeed = 0;

    if (makeSame)
    {
        // Find a Speed value that has enough species to populate every group.
        u8 attempts;
        for (attempts = 0; attempts < 32; attempts++)
        {
            u16 dex, sp;
            u8 count = 0;
            forcedSpeed = gSpeciesInfo[PickRandomQuizSpecies()].baseSpeed;
            for (dex = 1; dex <= NATIONAL_DEX_DEOXYS; dex++)
            {
                sp = NationalPokedexNumToSpecies(dex);
                if (sp != SPECIES_NONE && gSpeciesInfo[sp].baseSpeed == forcedSpeed)
                    count++;
            }
            if (count >= POKEMON_QUIZ_GROUP_COUNT)
                break;
        }
    }

    for (g = 0; g < POKEMON_QUIZ_GROUP_COUNT; g++)
    {
        u16 pool[64];
        u8 poolCount = 0;
        u16 targetSpeed = 0;
        u8 attempts;
        u8 maxSize;
        u8 offset;
        u32 weights[POKEMON_QUIZ_MAX_PER_GROUP];
        u32 total;
        u32 r;
        u8 chosenSize = 1;
        u16 dex, sp;

        sQuizGroupSize[g] = 0;
        sQuizGroupSpeed[g] = 0;

        // Pick a target Speed value, then build a pool of species sharing it.
        // For non-Same questions we reject any speed already used by an
        // earlier group so all four base Speeds are unique. As the quiz
        // progresses, groups 1+ are also constrained to a tighter window
        // around group 0's Speed so the speeds get closer together.
        for (attempts = 0; attempts < 32 && poolCount == 0; attempts++)
        {
            u8 g2;
            bool8 dupSpeed = FALSE;

            if (makeSame)
            {
                targetSpeed = forcedSpeed;
            }
            else
            {
                u16 seed = PickRandomQuizSpecies();
                targetSpeed = gSpeciesInfo[seed].baseSpeed;
                for (g2 = 0; g2 < g; g2++)
                {
                    if (sQuizGroupSpeed[g2] == targetSpeed)
                    {
                        dupSpeed = TRUE;
                        break;
                    }
                }
                if (dupSpeed)
                    continue;
                // For groups 1+ enforce a "speed window" around group 0
                // that shrinks linearly toward the final question.
                if (g > 0)
                {
                    // maxDelta: Q1 ~225, Q10 ~5. Smaller = closer speeds.
                    s32 maxDelta = (s32)(POKEMON_QUIZ_QUESTION_COUNT - questionNum + 1) * 25;
                    s32 diff = (s32)targetSpeed - (s32)sQuizGroupSpeed[0];
                    if (diff < 0)
                        diff = -diff;
                    if (diff > maxDelta)
                        continue;
                }
            }

            poolCount = 0;
            for (dex = 1; dex <= NATIONAL_DEX_DEOXYS && poolCount < 64; dex++)
            {
                u8 k;
                bool8 dup = FALSE;
                sp = NationalPokedexNumToSpecies(dex);
                if (sp == SPECIES_NONE)
                    continue;
                if (gSpeciesInfo[sp].baseSpeed != targetSpeed)
                    continue;
                for (k = 0; k < usedCount; k++)
                {
                    if (used[k] == sp)
                    {
                        dup = TRUE;
                        break;
                    }
                }
                if (dup)
                    continue;
                pool[poolCount++] = sp;
            }
        }

        if (poolCount == 0)
        {
            // Fall back to a single random species.
            sQuizSpecies[g][0] = PickRandomQuizSpecies();
            sQuizGroupSize[g] = 1;
            sQuizGroupSpeed[g] = gSpeciesInfo[sQuizSpecies[g][0]].baseSpeed;
            used[usedCount++] = sQuizSpecies[g][0];
            continue;
        }

        // Group size cap shrinks as the quiz progresses so the final
        // question always has 1-mon groups.
        // Cap = max(1, MAX - (q-1)/3): Q1-3=4, Q4-6=3, Q7-9=2, Q10=1.
        {
            u8 cap = (POKEMON_QUIZ_MAX_PER_GROUP > (questionNum - 1) / 3)
                     ? POKEMON_QUIZ_MAX_PER_GROUP - (questionNum - 1) / 3
                     : 1;
            if (cap < 1)
                cap = 1;
            maxSize = (poolCount < cap) ? poolCount : cap;
        }
        offset = 0; // Use weights [1,2,...,maxSize] favoring the cap.
        total = 0;
        for (s = 0; s < maxSize; s++)
        {
            s32 w = (s32)(s + 1) - (s32)offset;
            if (w < 0) w = 0;
            weights[s] = w;
            total += w;
        }
        if (total == 0)
        {
            weights[0] = 1;
            total = 1;
        }
        r = Random() % total;
        for (s = 0; s < maxSize; s++)
        {
            if (r < weights[s])
            {
                chosenSize = s + 1;
                break;
            }
            r -= weights[s];
        }

        // Partial Fisher-Yates shuffle to pick `chosenSize` species from pool.
        for (i = 0; i < chosenSize; i++)
        {
            u8 swapIdx = i + (Random() % (poolCount - i));
            u16 tmp = pool[i];
            pool[i] = pool[swapIdx];
            pool[swapIdx] = tmp;
            sQuizSpecies[g][i] = pool[i];
            used[usedCount++] = pool[i];
        }
        sQuizGroupSize[g] = chosenSize;
        sQuizGroupSpeed[g] = targetSpeed;
    }

    if (makeSame)
    {
        sQuizCorrectChoice = POKEMON_QUIZ_SAME_INDEX;
    }
    else
    {
        maxSpeed = sQuizGroupSpeed[0];
        maxIndex = 0;
        for (g = 1; g < POKEMON_QUIZ_GROUP_COUNT; g++)
        {
            if (sQuizGroupSpeed[g] > maxSpeed)
            {
                maxSpeed = sQuizGroupSpeed[g];
                maxIndex = g;
            }
        }
        sQuizCorrectChoice = maxIndex;
    }

    gStringVar1[0] = EOS;
    gStringVar2[0] = EOS;
}

// Cursor positions in panel-local coordinates for each selection.
// Placed on the leftmost icon of each group so the arrow points right
// into the rest of the group's sprites.
// 0=A (top-left), 1=B (bot-left), 2=C (top-right), 3=D (bot-right), 4=Same (hidden)
static const u8 sQuizCursorX[POKEMON_QUIZ_GROUP_COUNT + 1] = {   0,   0, 115, 115, 180 };
static const u8 sQuizCursorY[POKEMON_QUIZ_GROUP_COUNT + 1] = {  21,  46,  21,  46,   0 };
static const u8 sQuizCursorChar[] = _("{RIGHT_ARROW}");

EWRAM_DATA static u8 sQuizMenuSelection = 0;

// Panel-local bounds of the group highlight boxes. Group rows start below the
// 16 px title text cell; the window's origin is screen (8, 88).
#define QUIZ_PANEL_WIDTH     224
#define QUIZ_PANEL_ORIGIN_X  8
#define QUIZ_TOP_ROW_Y       16
#define QUIZ_TOP_ROW_END     39
#define QUIZ_BOTTOM_ROW_Y    40
#define QUIZ_BOTTOM_ROW_END  64
#define QUIZ_LEFT_COL_MIN_X  9   // clear of the A/B cursor
#define QUIZ_RIGHT_COL_MIN_X 124 // clear of the C/D cursor
#define QUIZ_SAME_TEXT_X     192
#define QUIZ_SAME_BOX_X      189
#define QUIZ_SAME_BOX_WIDTH  32
#define QUIZ_SAME_BOX_HEIGHT 16

static void FillQuizChoiceBox(u8 choice, u8 fill)
{
    s16 left, right, top, bottom;

    if (choice == POKEMON_QUIZ_SAME_INDEX)
    {
        FillWindowPixelRect(sQuizPanelWindowId, PIXEL_FILL(fill),
                            QUIZ_SAME_BOX_X, 0, QUIZ_SAME_BOX_WIDTH, QUIZ_SAME_BOX_HEIGHT);
        return;
    }
    if (choice >= POKEMON_QUIZ_GROUP_COUNT)
        return;

    // 1 px margin around the group's visible icon art.
    left = GetQuizGroupLeftEdge(choice) - QUIZ_PANEL_ORIGIN_X - 1;
    right = GetQuizGroupRightEdge(choice) - QUIZ_PANEL_ORIGIN_X + 2;
    if (choice >= 2)
    {
        if (left < QUIZ_RIGHT_COL_MIN_X)
            left = QUIZ_RIGHT_COL_MIN_X;
    }
    else if (left < QUIZ_LEFT_COL_MIN_X)
    {
        left = QUIZ_LEFT_COL_MIN_X;
    }
    if (right > QUIZ_PANEL_WIDTH)
        right = QUIZ_PANEL_WIDTH;

    // Groups are laid out A/B in the left column, C/D in the right.
    if (choice % 2 == 0)
    {
        top = QUIZ_TOP_ROW_Y;
        bottom = QUIZ_TOP_ROW_END;
    }
    else
    {
        top = QUIZ_BOTTOM_ROW_Y;
        bottom = QUIZ_BOTTOM_ROW_END;
    }
    FillWindowPixelRect(sQuizPanelWindowId, PIXEL_FILL(fill), left, top, right - left, bottom - top);
}

// While choosing, the hovered option gets a blue box. Once revealed, the
// correct option is green and a wrong pick is red.
static void RedrawQuizPanel(u8 selection)
{
    u8 sameColors[3] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};
    bool8 revealed = (sQuizPickedChoice != POKEMON_QUIZ_NO_PICK);

    FillWindowPixelBuffer(sQuizPanelWindowId, PIXEL_FILL(1));
    if (!revealed)
    {
        FillQuizChoiceBox(selection, QUIZ_FILL_HOVER);
        if (selection == POKEMON_QUIZ_SAME_INDEX)
            sameColors[0] = QUIZ_FILL_HOVER;
    }
    else
    {
        FillQuizChoiceBox(sQuizCorrectChoice, QUIZ_FILL_CORRECT);
        if (sQuizCorrectChoice == POKEMON_QUIZ_SAME_INDEX)
            sameColors[0] = QUIZ_FILL_CORRECT;
        if (sQuizPickedChoice != sQuizCorrectChoice)
        {
            FillQuizChoiceBox(sQuizPickedChoice, QUIZ_FILL_WRONG);
            if (sQuizPickedChoice == POKEMON_QUIZ_SAME_INDEX)
                sameColors[0] = QUIZ_FILL_WRONG;
        }
    }

    AddTextPrinterParameterized(sQuizPanelWindowId, FONT_NORMAL, sQuizPanelTitle, 4, 0, TEXT_SKIP_DRAW, NULL);
    // "Same" label sits at the top-right of the panel; its text background
    // matches whatever box is drawn behind it.
    AddTextPrinterParameterized3(sQuizPanelWindowId, FONT_NORMAL, QUIZ_SAME_TEXT_X, 0, sameColors, TEXT_SKIP_DRAW, sQuizOptionSame);
    if (!revealed)
        AddTextPrinterParameterized(sQuizPanelWindowId, FONT_NORMAL, sQuizCursorChar,
                                    sQuizCursorX[selection], sQuizCursorY[selection], TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(sQuizPanelWindowId, COPYWIN_FULL);
}

static void Task_QuizMenuInput(u8 taskId)
{
    u8 sel = sQuizMenuSelection;
    u8 newSel = sel;

    // Input lockout: ignore presses for the first 10 frames so any pre-buffered
    // button mashing during minigame load doesn't immediately confirm a choice
    // (which previously caused soft-locks).
    if (gTasks[taskId].data[0] < 10)
    {
        gTasks[taskId].data[0]++;
        return;
    }

    if (JOY_NEW(A_BUTTON))
    {
        gSpecialVar_Result = sel;
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }
    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        gSpecialVar_Result = MULTI_B_PRESSED;
        DestroyTask(taskId);
        ScriptContext_Enable();
        return;
    }

    // Layout grid:
    //   A  C   Same
    //   B  D
    if (JOY_NEW(DPAD_UP))
    {
        if (sel == 1) newSel = 0;        // B -> A
        else if (sel == 3) newSel = 2;   // D -> C
        else if (sel == 0 || sel == 2) newSel = 4; // A/C -> Same
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        if (sel == 0) newSel = 1;        // A -> B
        else if (sel == 2) newSel = 3;   // C -> D
        else if (sel == 4) newSel = 2;   // Same -> C
    }
    else if (JOY_NEW(DPAD_LEFT))
    {
        if (sel == 2) newSel = 0;        // C -> A
        else if (sel == 3) newSel = 1;   // D -> B
        else if (sel == 4) newSel = 2;   // Same -> C
    }
    else if (JOY_NEW(DPAD_RIGHT))
    {
        if (sel == 0) newSel = 2;        // A -> C
        else if (sel == 1) newSel = 3;   // B -> D
    }

    if (newSel != sel)
    {
        PlaySE(SE_SELECT);
        sQuizMenuSelection = newSel;
        SetQuizIconAnimation(newSel);
        RedrawQuizPanel(newSel);
    }
}

void ShowPokemonQuizChoiceMenu(void)
{
    u8 taskId;
    sQuizMenuSelection = 0;
    sQuizPickedChoice = POKEMON_QUIZ_NO_PICK;
    SetQuizIconAnimation(0);
    RedrawQuizPanel(0);
    taskId = CreateTask(Task_QuizMenuInput, 80);
    gTasks[taskId].data[0] = 0; // input lockout counter
}

// Holds the revealed answer on screen until the player presses A or B.
static void Task_QuizResultInput(u8 taskId)
{
    if (gTasks[taskId].data[0] < 20)
    {
        gTasks[taskId].data[0]++;
        return;
    }

    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        DestroyTask(taskId);
        ScriptContext_Enable();
    }
}

// Reveals the answer in the panel (colored boxes, speed badges, sound) and
// waits for a button press. Must be followed by `waitstate` in the script.
// VAR_RESULT = TRUE if the player's pick (passed in VAR_RESULT) was correct.
void CheckPokemonQuizAnswer(void)
{
    u8 picked = gSpecialVar_Result;
    bool8 correct = (picked == sQuizCorrectChoice);
    u8 taskId;

    sQuizPickedChoice = picked;
    SetQuizIconAnimation(POKEMON_QUIZ_NO_PICK);
    RedrawQuizPanel(picked);
    CreateQuizSpeedBadges();

    if (correct)
    {
        sQuizScore++;
        PlaySE(SE_DING_DONG);
    }
    else
    {
        PlaySE(SE_FAILURE);
    }
    sQuizQuestionsAsked++;

    gSpecialVar_Result = correct;
    taskId = CreateTask(Task_QuizResultInput, 80);
    gTasks[taskId].data[0] = 0; // input lockout counter
}
void GetPokemonQuizQuestionCount(void)
{
    gSpecialVar_Result = sQuizQuestionsAsked;
}

void GivePokemonQuizReward(void)
{
    static const u16 sVitamins[] = {
        ITEM_HP_UP,
        ITEM_PROTEIN,
        ITEM_IRON,
        ITEM_CARBOS,
        ITEM_CALCIUM,
        ITEM_ZINC,
    };

    gSpecialVar_0x8000 = sVitamins[Random() % ARRAY_COUNT(sVitamins)];

    ConvertIntToDecimalStringN(gStringVar1, sQuizScore, STR_CONV_MODE_LEFT_ALIGN, 2);
    ConvertIntToDecimalStringN(gStringVar2, POKEMON_QUIZ_QUESTION_COUNT, STR_CONV_MODE_LEFT_ALIGN, 2);
}

// Plays the SE the PC plays when you try to deposit your last Pokemon -
// scripted `playse SE_FAILURE` was inaudible in this context, calling
// PlaySE() from C works.
void PlayPokemonQuizWrongSE(void)
{
    PlaySE12WithPanning(SE_FAILURE, 0);
}

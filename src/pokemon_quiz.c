// Pokemon Speed Quiz minigame - 10 question speed-only mode.
// Hosted by the Trainer School woman in Viridian City. The player is shown
// four GROUPS of 1-3 random Pokemon (rendered as party icons) and must
// pick the group with the highest TOTAL base Speed. If two or more groups
// share the top total, the correct answer is "Same". Completing the quiz
// awards a random vitamin (HP Up / Protein / Iron / Carbos / Calcium / Zinc).

#include "global.h"
#include "data.h"
#include "event_data.h"
#include "main.h"
#include "menu.h"
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
#include "window.h"
#include "constants/characters.h"
#include "constants/items.h"
#include "constants/pokedex.h"
#include "constants/script_menu.h"
#include "constants/songs.h"
#include "constants/species.h"

#define POKEMON_QUIZ_QUESTION_COUNT 10
#define POKEMON_QUIZ_GROUP_COUNT    4
#define POKEMON_QUIZ_MAX_PER_GROUP  4
#define POKEMON_QUIZ_SAME_INDEX     POKEMON_QUIZ_GROUP_COUNT // 4

static const u8 sQuizOptionA[]    = _("A");
static const u8 sQuizOptionB[]    = _("B");
static const u8 sQuizOptionC[]    = _("C");
static const u8 sQuizOptionD[]    = _("D");
static const u8 sQuizOptionSame[] = _("Same");
static const u8 sQuizOptionBlank[] = _(" ");
static const u8 sQuizColon[]      = _(": ");
static const u8 sQuizSpeedSep[]   = _(" ");
static const u8 sQuizMultSep[]    = _(" x ");
static const u8 sQuizSameLabel[]  = _("Same total!");

// 2x2 group anchors aligned with the custom panel.
// Shifted +2 px right and -2 px up from the original layout per playtest.
static const s16 sQuizGroupAnchorX[POKEMON_QUIZ_GROUP_COUNT] = { 69, 69, 182, 182 };
static const s16 sQuizGroupAnchorY[POKEMON_QUIZ_GROUP_COUNT] = { 115, 138, 115, 138 };

// X offsets within a group for 1, 2, 3, or 4 icons. Icon sprites are 32x32
// but the visible Pokemon area is ~24x24, so spacing centers ~26 px apart
// leaves roughly a 1 px gap between rendered icons.
static const s16 sQuizGroupOffsets[POKEMON_QUIZ_MAX_PER_GROUP][POKEMON_QUIZ_MAX_PER_GROUP] = {
    {   0,   0,   0,   0 }, // N = 1
    { -13, +13,   0,   0 }, // N = 2
    { -26,   0, +26,   0 }, // N = 3
    { -39, -13, +13, +39 }, // N = 4
};

static u8 sQuizQuestionsAsked;
static u8 sQuizScore;
static u8 sQuizCorrectChoice;
static u16 sQuizSpecies[POKEMON_QUIZ_GROUP_COUNT][POKEMON_QUIZ_MAX_PER_GROUP];
static u8 sQuizGroupSize[POKEMON_QUIZ_GROUP_COUNT];
static u16 sQuizGroupSpeed[POKEMON_QUIZ_GROUP_COUNT];
static struct MenuAction sQuizMenuItems[POKEMON_QUIZ_GROUP_COUNT + 2]; // +1 for Same, +1 padding for 2x3 grid
static u8 sQuizIconSpriteIds[POKEMON_QUIZ_GROUP_COUNT][POKEMON_QUIZ_MAX_PER_GROUP];
static bool8 sQuizIconsLoaded;
static u8 sQuizPanelWindowId;
static bool8 sQuizPanelOpen;

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

static const u8 *GetLetterForIndex(u8 index)
{
    switch (index)
    {
    case 0: return sQuizOptionA;
    case 1: return sQuizOptionB;
    case 2: return sQuizOptionC;
    case 3: return sQuizOptionD;
    default: return sQuizOptionSame;
    }
}

static void ResetQuizIconIds(void)
{
    u8 g, i;
    for (g = 0; g < POKEMON_QUIZ_GROUP_COUNT; g++)
        for (i = 0; i < POKEMON_QUIZ_MAX_PER_GROUP; i++)
            sQuizIconSpriteIds[g][i] = MAX_SPRITES;
}

void StartPokemonQuizSpeed(void)
{
    sQuizQuestionsAsked = 0;
    sQuizScore = 0;
    sQuizCorrectChoice = 0;
    sQuizIconsLoaded = FALSE;
    sQuizPanelOpen = FALSE;
    ResetQuizIconIds();
}

void OpenPokemonQuizPanel(void)
{
    if (sQuizPanelOpen)
        return;

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
    sQuizPanelOpen = FALSE;
}

void DestroyPokemonQuizIcons(void)
{
    u8 g, i;

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
            sQuizIconSpriteIds[g][i] = CreateMonIcon(sQuizSpecies[g][i],
                                                    SpriteCallbackDummy,
                                                    x, y, 0, 0, FALSE);
            if (sQuizIconSpriteIds[g][i] != MAX_SPRITES)
                gSprites[sQuizIconSpriteIds[g][i]].oam.priority = 0;
        }
    }
    sQuizIconsLoaded = TRUE;
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

static u8 sQuizMenuSelection;

static void RedrawQuizPanel(u8 selection)
{
    FillWindowPixelBuffer(sQuizPanelWindowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(sQuizPanelWindowId, FONT_NORMAL, sQuizPanelTitle, 4, 0, 0xFF, NULL);
    // "Same" label sits at the top-right of the panel.
    AddTextPrinterParameterized(sQuizPanelWindowId, FONT_NORMAL, sQuizOptionSame, 192, 0, 0xFF, NULL);
    // Cursor follows the selection (including "Same").
    AddTextPrinterParameterized(sQuizPanelWindowId, FONT_NORMAL, sQuizCursorChar,
                                sQuizCursorX[selection], sQuizCursorY[selection], 0xFF, NULL);
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
        RedrawQuizPanel(newSel);
    }
}

void ShowPokemonQuizChoiceMenu(void)
{
    u8 taskId;
    sQuizMenuSelection = 0;
    RedrawQuizPanel(0);
    taskId = CreateTask(Task_QuizMenuInput, 80);
    gTasks[taskId].data[0] = 0; // input lockout counter
}

// Builds STR_VAR_1 = "<letter>: <baseSpeed>" for the winning group, or
// "Same" if the correct answer is a tie. Used for both right and wrong
// result text in the script.
void CheckPokemonQuizAnswer(void)
{
    u8 picked = gSpecialVar_Result;
    bool8 correct = (picked == sQuizCorrectChoice);
    u8 tmp[8];

    if (sQuizCorrectChoice == POKEMON_QUIZ_SAME_INDEX)
    {
        // For "Same" all groups share one base Speed; show that value too.
        ConvertIntToDecimalStringN(tmp, sQuizGroupSpeed[0], STR_CONV_MODE_LEFT_ALIGN, 4);
        StringCopy(gStringVar1, sQuizOptionSame);
        StringAppend(gStringVar1, sQuizColon);
        StringAppend(gStringVar1, tmp);
    }
    else
    {
        ConvertIntToDecimalStringN(tmp, sQuizGroupSpeed[sQuizCorrectChoice],
                                   STR_CONV_MODE_LEFT_ALIGN, 4);
        StringCopy(gStringVar1, GetLetterForIndex(sQuizCorrectChoice));
        StringAppend(gStringVar1, sQuizColon);
        StringAppend(gStringVar1, tmp);
    }

    // STR_VAR_2 = the player's pick, formatted the same way.
    if (picked == POKEMON_QUIZ_SAME_INDEX)
    {
        StringCopy(gStringVar2, sQuizOptionSame);
    }
    else if (picked < POKEMON_QUIZ_GROUP_COUNT)
    {
        ConvertIntToDecimalStringN(tmp, sQuizGroupSpeed[picked],
                                   STR_CONV_MODE_LEFT_ALIGN, 4);
        StringCopy(gStringVar2, GetLetterForIndex(picked));
        StringAppend(gStringVar2, sQuizColon);
        StringAppend(gStringVar2, tmp);
    }
    else
    {
        gStringVar2[0] = EOS;
    }

    if (correct)
        sQuizScore++;
    else
        PlaySE(SE_FAILURE);
    sQuizQuestionsAsked++;

    gSpecialVar_Result = correct;
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

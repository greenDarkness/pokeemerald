#include "global.h"
#include "event_data.h"
#include "event_scripts.h"
#include "field_effect.h"
#include "field_player_avatar.h"
#include "field_screen_effect.h"
#include "field_weather.h"
#include "fldeff.h"
#include "malloc.h"
#include "mirage_tower.h"
#include "palette.h"
#include "party_menu.h"
#include "pokemon.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "task.h"
#include "wild_encounter.h"
#include "new_moves_popup.h"
#include "pickup_item_popup.h"
#include "chain_reroll_popup.h"
#include "constants/field_effects.h"
#include "constants/moves.h"
#include "constants/rgb.h"
#include "constants/songs.h"

static void FieldCallback_SweetScent(void);
static void StartSweetScentFieldEffect(void);
static void TrySweetScentEncounter(u8 taskId);
static void FailSweetScentEncounter(u8 taskId);

static EWRAM_DATA u8 sSweetScentPartySlot = 0;

bool8 SetUpFieldMove_SweetScent(void)
{
    gFieldCallback2 = FieldCallback_PrepareFadeInFromMenu;
    gPostMenuFieldCallback = FieldCallback_SweetScent;
    return TRUE;
}

static void FieldCallback_SweetScent(void)
{
    sSweetScentPartySlot = GetCursorSelectionMonId();
    FieldEffectStart(FLDEFF_SWEET_SCENT);
    gFieldEffectArguments[0] = sSweetScentPartySlot;
}

bool8 FldEff_SweetScent(void)
{
    u8 taskId;

    SetWeatherScreenFadeOut();
    taskId = CreateFieldMoveTask();
    gTasks[taskId].data[8] = (u32)StartSweetScentFieldEffect >> 16;
    gTasks[taskId].data[9] = (u32)StartSweetScentFieldEffect;
    return FALSE;
}

static void StartSweetScentFieldEffect(void)
{
    u8 taskId;
    u32 palettes = ~(1 << (gSprites[GetPlayerAvatarSpriteId()].oam.paletteNum + 16) | (1 << 13) | (1 << 14) | (1 << 15));
    u32 *buffer = Alloc(PLTT_SIZE);

    PlaySE(SE_M_SWEET_SCENT);
    if (buffer) {
        CpuFastCopy(gPlttBufferUnfaded, buffer, PLTT_SIZE);
        CpuFastCopy(gPlttBufferFaded, gPlttBufferUnfaded, PLTT_SIZE);
    }
    BeginNormalPaletteFade(palettes, 4, 0, 8, RGB_RED);
    taskId = CreateTask(TrySweetScentEncounter, 0);
    gTasks[taskId].data[0] = 0;
    gTasks[taskId].data[1] = (u32) buffer >> 16;
    gTasks[taskId].data[2] = (u32) buffer;
    FieldEffectActiveListRemove(FLDEFF_SWEET_SCENT);
}

static void TrySweetScentEncounter(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        ClearMirageTowerPulseBlendEffect();
        BlendPalettes(0x00000040, 8, RGB_RED);
        if (gTasks[taskId].data[0] == 64)
        {
            gTasks[taskId].data[0] = 0;
            if (SweetScentWildEncounter() == TRUE)
            {
                u32 *buffer = (u32*)(((u16)gTasks[taskId].data[1] << 16) | (u16)gTasks[taskId].data[2]);
                TRY_FREE_AND_SET_NULL(buffer);
                DestroyTask(taskId);
            }
            else
            {
                gTasks[taskId].func = FailSweetScentEncounter;
                BeginNormalPaletteFade(~(1 << (gSprites[GetPlayerAvatarSpriteId()].oam.paletteNum + 16)), 4, 8, 0, RGB_RED);
                TryStartMirageTowerPulseBlendEffect();
            }
        }
        else
        {
            gTasks[taskId].data[0]++;
        }
    }
}

static void FailSweetScentEncounter(u8 taskId)
{
    if (!gPaletteFade.active)
    {
        u32 *buffer = (u32*)(((u16)gTasks[taskId].data[1] << 16) | (u16)gTasks[taskId].data[2]);
        if (buffer) {
            CpuFastCopy(buffer, gPlttBufferUnfaded, PLTT_SIZE);
            Free(buffer);
        }
        SetWeatherPalStateIdle();
        ScriptContext_SetupScript(EventScript_FailSweetScent);
        DestroyTask(taskId);
    }
}

void StartSweetScentFromScript(void)
{
    FieldEffectStart(FLDEFF_SWEET_SCENT);
    gFieldEffectArguments[0] = sSweetScentPartySlot;
}

void PartyCanUseSweetScent(void)
{
    u8 i;

    for (i = 0; i < PARTY_SIZE; i++)
    {
        if (!GetMonData(&gPlayerParty[i], MON_DATA_SPECIES))
            break;
        if (!GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG) && MonKnowsMove(&gPlayerParty[i], MOVE_SWEET_SCENT))
        {
            gSpecialVar_Result = TRUE;
            return;
        }
    }
    gSpecialVar_Result = FALSE;
}

static void Task_WaitForPopups(u8 taskId)
{
    if (!IsNewMovesPopupActive() && !IsPickupItemPopupActive() && !IsChainRerollPopupActive())
    {
        LockPlayerFieldControls();
        ScriptContext_Enable();
        DestroyTask(taskId);
    }
}

void WaitForPopups(void)
{
    if (IsNewMovesPopupActive() || IsPickupItemPopupActive() || IsChainRerollPopupActive())
    {
        UnlockPlayerFieldControls();
        CreateTask(Task_WaitForPopups, 1);
        gSpecialVar_Result = TRUE;
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}

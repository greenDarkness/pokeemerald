#include "global.h"
#include "ball_swap.h"
#include "item.h"
#include "item_menu.h"
#include "pokemon.h"
#include "event_data.h"
#include "overworld.h"
#include "constants/item.h"

// Check if the player has at least one Poke Ball in their bag
bool8 PlayerHasPokeBalls(void)
{
    return IsBagPocketNonEmpty(POCKET_POKE_BALLS);
}

// Callback for returning from bag menu after selecting a ball
static void CB2_BallSwapExitBagMenu(void)
{
    SetMainCallback2(CB2_ReturnToFieldContinueScript);
}

// Open the bag to the balls pocket for selecting a ball to swap to
void Script_BallSwapOpenBagMenu(void)
{
    GoToBagMenu(ITEMMENULOCATION_BALL_SWAP, BALLS_POCKET, CB2_BallSwapExitBagMenu);
}

// Swap the ball of the selected Pokemon
// VAR_0x8004 = party slot
// gSpecialVar_ItemId = new ball item ID
void SwapPartyMonBall(void)
{
    u8 partySlot = gSpecialVar_0x8004;
    u16 newBall = gSpecialVar_ItemId;
    u16 oldBall;
    
    if (partySlot < PARTY_SIZE && GetMonData(&gPlayerParty[partySlot], MON_DATA_SPECIES) != SPECIES_NONE)
    {
        // Get the current ball before swapping
        oldBall = GetMonData(&gPlayerParty[partySlot], MON_DATA_POKEBALL);
        
        // Remove one of the new ball from the bag
        if (RemoveBagItem(newBall, 1))
        {
            // Set the new ball on the Pokemon
            SetMonData(&gPlayerParty[partySlot], MON_DATA_POKEBALL, &newBall);
            
            // Return the old ball to the player's bag
            AddBagItem(oldBall, 1);
            
            gSpecialVar_Result = TRUE;
        }
        else
        {
            gSpecialVar_Result = FALSE;
        }
    }
    else
    {
        gSpecialVar_Result = FALSE;
    }
}

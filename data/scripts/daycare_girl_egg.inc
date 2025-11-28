@ Daycare Girl Egg Event Helper
@ Generates a new egg every 255 steps after the player has rescued Birch

DaycareGirl_CheckForNewEgg::
	@ Don't generate if she already has an egg waiting
	goto_if_set FLAG_DAYCARE_GIRL_HAS_EGG, DaycareGirl_CheckForNewEgg_End
	
	@ Don't start counting until player has rescued Birch
	goto_if_unset FLAG_RESCUED_BIRCH, DaycareGirl_CheckForNewEgg_End
	
	@ Increment step counter
	copyvar VAR_0x8000, VAR_DAYCARE_GIRL_EGG_STEP_COUNTER
	addvar VAR_0x8000, 1
	
	@ Check if reached 25 steps (changed from 255 for testing)
	compare VAR_0x8000, 25
	goto_if_ge DaycareGirl_GiveEgg
	
	@ Save incremented counter and exit
	copyvar VAR_DAYCARE_GIRL_EGG_STEP_COUNTER, VAR_0x8000
	return

DaycareGirl_GiveEgg:
	@ Reset step counter
	setvar VAR_DAYCARE_GIRL_EGG_STEP_COUNTER, 0
	@ Set flag that egg is available
	setflag FLAG_DAYCARE_GIRL_HAS_EGG
	return

DaycareGirl_CheckForNewEgg_End:
	return

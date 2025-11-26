@ Daycare Girl Egg Event Helper
@ Call this script after rival battles, gym badges, and E4 victories

DaycareGirl_CheckForNewEgg::
	goto_if_set FLAG_DAYCARE_GIRL_HAS_EGG, DaycareGirl_CheckForNewEgg_End
	
	@ Check current stage
	copyvar VAR_0x8000, VAR_DAYCARE_GIRL_EGG_STAGE
	
	@ Stage 0: After first rival battle (Route 103)
	compare VAR_0x8000, 0
	goto_if_eq DaycareGirl_CheckStage_Rival
	
	@ Stage 1-8: After each gym badge
	compare VAR_0x8000, 1
	goto_if_eq DaycareGirl_CheckStage_Badge1
	compare VAR_0x8000, 2
	goto_if_eq DaycareGirl_CheckStage_Badge2
	compare VAR_0x8000, 3
	goto_if_eq DaycareGirl_CheckStage_Badge3
	compare VAR_0x8000, 4
	goto_if_eq DaycareGirl_CheckStage_Badge4
	compare VAR_0x8000, 5
	goto_if_eq DaycareGirl_CheckStage_Badge5
	compare VAR_0x8000, 6
	goto_if_eq DaycareGirl_CheckStage_Badge6
	compare VAR_0x8000, 7
	goto_if_eq DaycareGirl_CheckStage_Badge7
	compare VAR_0x8000, 8
	goto_if_eq DaycareGirl_CheckStage_Badge8
	
	@ Stage 9+: After each Elite Four victory
	goto DaycareGirl_CheckStage_EliteFour

DaycareGirl_CheckStage_Rival:
	goto_if_set FLAG_SYS_POKEDEX_GET, DaycareGirl_GiveEgg
	return

DaycareGirl_CheckStage_Badge1:
	goto_if_set FLAG_BADGE01_GET, DaycareGirl_GiveEgg
	return

DaycareGirl_CheckStage_Badge2:
	goto_if_set FLAG_BADGE02_GET, DaycareGirl_GiveEgg
	return

DaycareGirl_CheckStage_Badge3:
	goto_if_set FLAG_BADGE03_GET, DaycareGirl_GiveEgg
	return

DaycareGirl_CheckStage_Badge4:
	goto_if_set FLAG_BADGE04_GET, DaycareGirl_GiveEgg
	return

DaycareGirl_CheckStage_Badge5:
	goto_if_set FLAG_BADGE05_GET, DaycareGirl_GiveEgg
	return

DaycareGirl_CheckStage_Badge6:
	goto_if_set FLAG_BADGE06_GET, DaycareGirl_GiveEgg
	return

DaycareGirl_CheckStage_Badge7:
	goto_if_set FLAG_BADGE07_GET, DaycareGirl_GiveEgg
	return

DaycareGirl_CheckStage_Badge8:
	goto_if_set FLAG_BADGE08_GET, DaycareGirl_GiveEgg
	return

DaycareGirl_CheckStage_EliteFour:
	@ For E4, check if game was cleared and increment counter
	@ This will give an egg each time they beat the E4
	goto_if_set FLAG_SYS_GAME_CLEAR, DaycareGirl_GiveEgg
	return

DaycareGirl_GiveEgg:
	setflag FLAG_DAYCARE_GIRL_HAS_EGG
	return

DaycareGirl_CheckForNewEgg_End:
	return

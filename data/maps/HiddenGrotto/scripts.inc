HiddenGrotto_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, HiddenGrotto_OnResume
	map_script MAP_SCRIPT_ON_TRANSITION, HiddenGrotto_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, HiddenGrotto_OnFrame
	.byte 0

HiddenGrotto_OnFrame:
	map_script_2 VAR_PETALBURG_CITY_STATE, 2, PetalburgCity_EventScript_WallyTutorialHiddenGrotto
	.2byte 0

@ A caught Pokemon is removed before the field fades back in (like the Regis); after
@ fleeing or defeating it, it vanishes once the battle script continues.
HiddenGrotto_OnResume:
	call_if_set FLAG_SYS_CTRL_OBJ_DELETE, HiddenGrotto_EventScript_TryRemoveCaughtMon
	end

HiddenGrotto_EventScript_TryRemoveCaughtMon::
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_ne VAR_RESULT, B_OUTCOME_CAUGHT, Common_EventScript_NopReturn
	removeobject VAR_LAST_TALKED
	return

HiddenGrotto_OnTransition:
	setflag FLAG_TEMP_3
	special SetupHiddenGrottoRoom
	call_if_eq VAR_PETALBURG_CITY_STATE, 2, HiddenGrotto_EventScript_SetupWallyTutorial
	end

@ Wally's catching tutorial (PetalburgCity_EventScript_WallyTutorial) brings him here to catch
@ a Ralts, whatever the grotto actually holds. FLAG_TEMP_3 hides Wally the rest of the time.
HiddenGrotto_EventScript_SetupWallyTutorial::
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_OW_MON
	setvar VAR_TEMP_4, SPECIES_RALTS
	clearflag FLAG_TEMP_1
	setflag FLAG_TEMP_2
	clearflag FLAG_TEMP_3
	return

HiddenGrotto_EventScript_Pokemon::
	lock
	faceplayer
	special GetHiddenGrottoPokemon
	goto_if_eq VAR_RESULT, FALSE, HiddenGrotto_EventScript_Vanish
	playmoncry VAR_0x8004, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	special StartHiddenGrottoBattle
	setflag FLAG_SYS_CTRL_OBJ_DELETE
	dowildbattle
	clearflag FLAG_SYS_CTRL_OBJ_DELETE
	removeobject VAR_LAST_TALKED
	release
	end

HiddenGrotto_EventScript_Item::
	special GetHiddenGrottoItem
	goto_if_eq VAR_RESULT, FALSE, HiddenGrotto_EventScript_Vanish
	finditem VAR_0x8000
	goto_if_ne VAR_0x8007, TRUE, HiddenGrotto_EventScript_End
	special ClearHiddenGrottoContents
HiddenGrotto_EventScript_End:
	end

HiddenGrotto_EventScript_Vanish::
	removeobject VAR_LAST_TALKED
	release
	end

HiddenGrotto_MapScripts::
	map_script MAP_SCRIPT_ON_RESUME, HiddenGrotto_OnResume
	map_script MAP_SCRIPT_ON_TRANSITION, HiddenGrotto_OnTransition
	.byte 0

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
	special SetupHiddenGrottoRoom
	end

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

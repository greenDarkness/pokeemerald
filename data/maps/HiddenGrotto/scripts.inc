HiddenGrotto_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, HiddenGrotto_OnTransition
	.byte 0

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
	dowildbattle
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

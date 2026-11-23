CeladonCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, CeladonCity_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

CeladonCity_PokemonCenter_1F_OnTransition::
	setrespawn HEAL_LOCATION_CELADON_CITY
	setflag FLAG_HIDE_CELADON_EUSINE
	call_if_ge VAR_SUICUNE_ENCOUNTERS, 8, CeladonCity_PokemonCenter_1F_EventScript_ShowEusine
	end

@ EUSINE is back in his hometown once SUICUNE has battled the player on ROUTE 25.
CeladonCity_PokemonCenter_1F_EventScript_ShowEusine::
	clearflag FLAG_HIDE_CELADON_EUSINE
	return

CeladonCity_PokemonCenter_1F_EventScript_Eusine::
	msgbox CeladonCity_PokemonCenter_1F_Text_Eusine, MSGBOX_NPC
	end

CeladonCity_PokemonCenter_1F_Text_Eusine:
	.string "EUSINE: Hi!\p"
	.string "I'm back visiting my hometown.\p"
	.string "It's been quite a while.$"

CeladonCity_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_CELADON_NURSE
	call Common_EventScript_KantoPkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

CeladonCity_PokemonCenter_1F_EventScript_Gentleman::
	msgbox CeladonCity_PokemonCenter_1F_Text_PokeFluteAwakensSleepingMons, MSGBOX_NPC
	end

CeladonCity_PokemonCenter_1F_EventScript_CooltrainerF::
	msgbox CeladonCity_PokemonCenter_1F_Text_RodeHereFromFuchsia, MSGBOX_NPC
	end

CeladonCity_PokemonCenter_1F_EventScript_Youngster::
	msgbox CeladonCity_PokemonCenter_1F_Text_GoToCyclingRoadIfIHadBike, MSGBOX_NPC
	end

CeladonCity_PokemonCenter_1F_Text_PokeFluteAwakensSleepingMons::
    .string "A POKé FLUTE awakens sleeping\n"
    .string "POKéMON. You know that.\p"
    .string "It does so with a sound that only\n"
    .string "they can hear.$"

CeladonCity_PokemonCenter_1F_Text_RodeHereFromFuchsia::
    .string "I rode here from FUCHSIA.\p"
    .string "It's an uphill ride on CYCLING\n"
    .string "ROAD, so I'm exhausted.$"

CeladonCity_PokemonCenter_1F_Text_GoToCyclingRoadIfIHadBike::
    .string "If I had a BIKE, I would go to\n"
    .string "CYCLING ROAD!$"


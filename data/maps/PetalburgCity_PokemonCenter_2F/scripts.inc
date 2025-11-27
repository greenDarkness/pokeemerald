PetalburgCity_PokemonCenter_2F_MapScripts::
	map_script MAP_SCRIPT_ON_FRAME_TABLE, CableClub_OnFrame
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, CableClub_OnWarp
	map_script MAP_SCRIPT_ON_LOAD, CableClub_OnLoad
	map_script MAP_SCRIPT_ON_TRANSITION, CableClub_OnTransition
	.byte 0

@ The below 3 are unused and leftover from RS
PetalburgCity_PokemonCenter_2F_EventScript_Colosseum::
	call CableClub_EventScript_Colosseum
	end

PetalburgCity_PokemonCenter_2F_EventScript_TradeCenter::
	call CableClub_EventScript_TradeCenter
	end

PetalburgCity_PokemonCenter_2F_EventScript_RecordCorner::
	call CableClub_EventScript_RecordCorner
	end

PetalburgCity_PokemonCenter_2F_EventScript_MintMaster::
	lock
	faceplayer
	goto_if_set FLAG_MINT_MASTER_INTRO, PetalburgCity_PokemonCenter_2F_EventScript_MintMaster_Main
	msgbox PetalburgCity_PokemonCenter_2F_Text_MintMasterIntro, MSGBOX_DEFAULT
	setflag FLAG_MINT_MASTER_INTRO
	goto PetalburgCity_PokemonCenter_2F_EventScript_MintMaster_Main

PetalburgCity_PokemonCenter_2F_EventScript_MintMaster_Main::
	special ChooseMonForNatureChange
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PetalburgCity_PokemonCenter_2F_EventScript_MintMaster_Decline
	multichoice 0, 0, MULTI_NATURE_MINT, FALSE
	switch VAR_RESULT
	case 0, PetalburgCity_PokemonCenter_2F_EventScript_SetAdamant
	case 1, PetalburgCity_PokemonCenter_2F_EventScript_SetJolly
	case 2, PetalburgCity_PokemonCenter_2F_EventScript_SetModest
	case 3, PetalburgCity_PokemonCenter_2F_EventScript_SetTimid
	case MULTI_B_PRESSED, PetalburgCity_PokemonCenter_2F_EventScript_MintMaster_Decline
	end

PetalburgCity_PokemonCenter_2F_EventScript_SetAdamant::
	setvar VAR_0x8005, NATURE_ADAMANT
	goto PetalburgCity_PokemonCenter_2F_EventScript_ConfirmNature

PetalburgCity_PokemonCenter_2F_EventScript_SetJolly::
	setvar VAR_0x8005, NATURE_JOLLY
	goto PetalburgCity_PokemonCenter_2F_EventScript_ConfirmNature

PetalburgCity_PokemonCenter_2F_EventScript_SetModest::
	setvar VAR_0x8005, NATURE_MODEST
	goto PetalburgCity_PokemonCenter_2F_EventScript_ConfirmNature

PetalburgCity_PokemonCenter_2F_EventScript_SetTimid::
	setvar VAR_0x8005, NATURE_TIMID
	goto PetalburgCity_PokemonCenter_2F_EventScript_ConfirmNature

PetalburgCity_PokemonCenter_2F_EventScript_ConfirmNature::
	special GetNatureName
	msgbox PetalburgCity_PokemonCenter_2F_Text_ConfirmNature, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PetalburgCity_PokemonCenter_2F_EventScript_MintMaster_Decline
	copyvar VAR_RESULT, VAR_0x8005
	special ChangePokemonNature
	playfanfare MUS_OBTAIN_ITEM
	msgbox PetalburgCity_PokemonCenter_2F_Text_NatureChanged, MSGBOX_DEFAULT
	waitmessage
	waitfanfare
	release
	end

PetalburgCity_PokemonCenter_2F_EventScript_MintMaster_Decline::
	msgbox PetalburgCity_PokemonCenter_2F_Text_ComeBackAnytime, MSGBOX_DEFAULT
	release
	end

PetalburgCity_PokemonCenter_2F_Text_MintMasterIntro:
	.string "Hello, I'm the Mint Master!\p"
	.string "I can help you change the nature of\n"
	.string "your POKéMON completely free!$"

PetalburgCity_PokemonCenter_2F_Text_ChangeNature:
	.string "Would you like me to change the\n"
	.string "nature of one of your POKéMON?$"

PetalburgCity_PokemonCenter_2F_Text_ConfirmNature:
	.string "Change to {STR_VAR_1} nature?$"

PetalburgCity_PokemonCenter_2F_Text_NatureChanged:
	.string "Done! Your POKéMON's nature has been\n"
	.string "changed to {STR_VAR_1}!$"

PetalburgCity_PokemonCenter_2F_Text_ComeBackAnytime:
	.string "Come back anytime!$"


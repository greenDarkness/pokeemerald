PalletTown_PlayersHouse_2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_PlayersHouse_2F_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, PalletTown_PlayersHouse_2F_OnWarp
	.byte 0

PalletTown_PlayersHouse_2F_OnTransition::
	call_if_eq VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 2, PalletTown_PlayersHouse_2F_EventScript_SetRespawn
	end

PalletTown_PlayersHouse_2F_EventScript_SetRespawn::
	setrespawn HEAL_LOCATION_PALLET_TOWN
	return

PalletTown_PlayersHouse_2F_OnWarp::
	map_script_2 VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 2, PalletTown_PlayersHouse_2F_FirstWarpIn
	.2byte 0

PalletTown_PlayersHouse_2F_FirstWarpIn::
	setvar VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 3
	setflag FLAG_HIDE_PALLET_TOWN_PLAYERS_HOUSE_VIGOROTH
	end

PalletTown_PlayersHouse_2F_EventScript_NES::
	msgbox PalletTown_PlayersHouse_2F_Text_PlayedWithNES, MSGBOX_SIGN
	end

PalletTown_PlayersHouse_2F_EventScript_Sign::
	msgbox PalletTown_PlayersHouse_2F_Text_PressLRForHelp, MSGBOX_SIGN
	end

PalletTown_PlayersHouse_2F_EventScript_CasualSneakersManual::
	msgbox PalletTown_PlayersHouse_2F_Text_CasualSneakersManual, MSGBOX_SIGN
	end

PalletTown_PlayersHouse_2F_EventScript_PC::
	goto_if_questlog EventScript_ReleaseEnd
	lockall
	setvar VAR_0x8004, HELPCONTEXT_BEDROOM_PC
	special Script_SetHelpContext
	setvar VAR_0x8004, 1
	special AnimatePcTurnOn
	playse SE_PC_ON
	msgbox Text_PlayerBootedUpPC
	special BedroomPC
	waitstate
	special SetHelpContextForMap
	releaseall
	end

EventScript_PalletTown_PlayersHouse_2F_ShutDownPC::
	setvar VAR_0x8004, 1
	playse SE_PC_OFF
	special AnimatePcTurnOff
	special SetHelpContextForMap
	releaseall
	end

PalletTown_PlayersHouse_2F_Text_PlayedWithNES::
    .string "{PLAYER} played with the NES.\p"
    .string "…Okay!\n"
    .string "It's time to go!$"

PalletTown_PlayersHouse_2F_Text_PressLRForHelp::
    .string "It's a posted notice…\p"
    .string "If you're confused, ask for HELP!\n"
    .string "Press the L or R Button!$"

PalletTown_PlayersHouse_2F_Text_CasualSneakersManual::
	.string "It's the instruction booklet for the\n"
	.string "CASUAL SNEAKERS.\p"
	.string "“Press the B Button to sneak by\n"
	.string "unnoticed while wearing your\l"
	.string "CASUAL SNEAKERS.\p"
	.string "“Lace up your CASUAL SNEAKERS and\n"
	.string "ACT CASUAL, trainer!”$"


PalletTown_PlayersHouse_2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_PlayersHouse_2F_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, PalletTown_PlayersHouse_2F_OnWarp
	map_script MAP_SCRIPT_ON_LOAD, PalletTown_PlayersHouse_2F_OnLoad
	.byte 0

PalletTown_PlayersHouse_2F_OnTransition::
	call_if_eq VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 2, PalletTown_PlayersHouse_2F_EventScript_SetRespawn
	end

PalletTown_PlayersHouse_2F_OnWarp::
	map_script_2 VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 2, PalletTown_PlayersHouse_2F_FirstWarpIn
	.2byte 0

PalletTown_PlayersHouse_2F_OnLoad::
	call_if_unset FLAG_HIDE_KANTO_HOUSE_GAS_POKEMON_BOX, PalletTown_PlayersHouse_2F_EventScript_ShowGasPokemonBox
	end

PalletTown_PlayersHouse_2F_EventScript_SetRespawn::
	setrespawn HEAL_LOCATION_PALLET_TOWN
	return

PalletTown_PlayersHouse_2F_FirstWarpIn::
	setvar VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 3
	setflag FLAG_HIDE_PALLET_TOWN_PLAYERS_HOUSE_VIGOROTH
	setflag FLAG_HIDDEN_ITEM_DELIAS_HOUSE_NEST_BALL
	end

PalletTown_PlayersHouse_2F_EventScript_ShowGasPokemonBox::
	goto_if_set FLAG_CHOSE_KOFFING, PalletTown_PlayersHouse_2F_EventScript_ShowKoffingPokemonBox
	goto_if_set FLAG_CHOSE_GASTLY, PalletTown_PlayersHouse_2F_EventScript_ShowGastlyPokemonBox
	setflag FLAG_HIDE_KANTO_HOUSE_GAS_POKEMON_BOX
	return
PalletTown_PlayersHouse_2F_EventScript_ShowKoffingPokemonBox::
	setmetatile 11, 7, 0x181, TRUE
	setmetatile 11, 8, 0x186, TRUE
	setmetatile 10, 8, 0x181, TRUE
	return
PalletTown_PlayersHouse_2F_EventScript_ShowGastlyPokemonBox::
	setmetatile 11, 7, 0x181, TRUE
	setmetatile 11, 8, 0x186, TRUE
	setmetatile 10, 8, 0x187, TRUE
	return

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

PalletTown_PlayersHouse_2F_EventScript_TruckEvent_GasPokemon::
	goto_if_set FLAG_RECEIVED_KANTO_TRUCK_STARTER, PalletTown_PlayersHouse_2F_EventScript_End
	goto_if_set FLAG_CHOSE_KOFFING, PalletTown_PlayersHouse_2F_EventScript_KoffingBox
	goto_if_set FLAG_CHOSE_GASTLY, PalletTown_PlayersHouse_2F_EventScript_GastlyBox
	end
PalletTown_PlayersHouse_2F_EventScript_KoffingBox::
	lock
	msgbox PalletTown_PlayersHouse_2F_Text_TakeKoffing, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PalletTown_PlayersHouse_2F_EventScript_LeavePokemon
	goto PalletTown_PlayersHouse_2F_EventScript_TakeKoffing
	end
PalletTown_PlayersHouse_2F_EventScript_TakeKoffing::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_KOFFING
	givemon SPECIES_KOFFING, 5
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PARTY, PalletTown_PlayersHouse_2F_EventScript_SendKoffingParty
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, PalletTown_PlayersHouse_2F_EventScript_SendKoffingPC
	goto Common_EventScript_NoMoreRoomForPokemon
	release
	end
PalletTown_PlayersHouse_2F_EventScript_SendKoffingParty::
	call PalletTown_PlayersHouse_2F_EventScript_ReceivedKoffingFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PalletTown_PlayersHouse_2F_EventScript_ReceivedGasPokemonEnd
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto PalletTown_PlayersHouse_2F_EventScript_ReceivedGasPokemonEnd
	end
PalletTown_PlayersHouse_2F_EventScript_SendKoffingPC::
	call PalletTown_PlayersHouse_2F_EventScript_ReceivedKoffingFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PalletTown_PlayersHouse_2F_EventScript_KoffingTransferredToPC
	call Common_EventScript_NameReceivedBoxMon
	goto PalletTown_PlayersHouse_2F_EventScript_KoffingTransferredToPC
	end
PalletTown_PlayersHouse_2F_EventScript_KoffingTransferredToPC::
	call Common_EventScript_TransferredToPC
	goto PalletTown_PlayersHouse_2F_EventScript_ReceivedGasPokemonEnd
	end
PalletTown_PlayersHouse_2F_EventScript_ReceivedKoffingFanfare::
	bufferspeciesname STR_VAR_2, SPECIES_KOFFING
	playfanfare MUS_OBTAIN_ITEM
	message PalletTown_PlayersHouse_2F_Text_ObtainedKoffing
	waitmessage
	waitfanfare
	bufferspeciesname STR_VAR_1, SPECIES_KOFFING
	return
PalletTown_PlayersHouse_2F_EventScript_GastlyBox::
	lock
	msgbox PalletTown_PlayersHouse_2F_Text_TakeGastly, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PalletTown_PlayersHouse_2F_EventScript_LeavePokemon
	goto PalletTown_PlayersHouse_2F_EventScript_TakeGastly
	end
PalletTown_PlayersHouse_2F_EventScript_TakeGastly::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_GASTLY
	givemon SPECIES_GASTLY, 5
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PARTY, PalletTown_PlayersHouse_2F_EventScript_SendGastlyParty
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, PalletTown_PlayersHouse_2F_EventScript_SendGastlyPC
	goto Common_EventScript_NoMoreRoomForPokemon
	release
	end
PalletTown_PlayersHouse_2F_EventScript_SendGastlyParty::
	call PalletTown_PlayersHouse_2F_EventScript_ReceivedGastlyFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PalletTown_PlayersHouse_2F_EventScript_ReceivedGasPokemonEnd
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto PalletTown_PlayersHouse_2F_EventScript_ReceivedGasPokemonEnd
	end
PalletTown_PlayersHouse_2F_EventScript_SendGastlyPC::
	call PalletTown_PlayersHouse_2F_EventScript_ReceivedGastlyFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PalletTown_PlayersHouse_2F_EventScript_GastlyTransferredToPC
	call Common_EventScript_NameReceivedBoxMon
	goto PalletTown_PlayersHouse_2F_EventScript_GastlyTransferredToPC
	end
PalletTown_PlayersHouse_2F_EventScript_GastlyTransferredToPC::
	call Common_EventScript_TransferredToPC
	goto PalletTown_PlayersHouse_2F_EventScript_ReceivedGasPokemonEnd
	end
PalletTown_PlayersHouse_2F_EventScript_ReceivedGastlyFanfare::
	bufferspeciesname STR_VAR_2, SPECIES_GASTLY
	playfanfare MUS_OBTAIN_ITEM
	message PalletTown_PlayersHouse_2F_Text_ObtainedGastly
	waitmessage
	waitfanfare
	bufferspeciesname STR_VAR_1, SPECIES_GASTLY
	return
PalletTown_PlayersHouse_2F_EventScript_ReceivedGasPokemonEnd::
	removeobject LOCALID_GAS_POKEMON
	setflag FLAG_RECEIVED_KANTO_TRUCK_STARTER
	setflag FLAG_HIDE_KANTO_HOUSE_GAS_POKEMON_BOX
	release
	end
PalletTown_PlayersHouse_2F_EventScript_LeavePokemon::
	msgbox PalletTown_PlayersHouse_2F_Text_LeftGasPokemon, MSGBOX_DEFAULT
	release
	end

PalletTown_PlayersHouse_2F_EventScript_End::
	release
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

PalletTown_PlayersHouse_2F_Text_TakeKoffing:
	.string "It's the box you left opened\n"
	.string "in the truck.\p"
	.string "It's... smoking?\p"
	.string "There is a strong odor\n"
	.string "coming from it.\p"
	.string "{PLAYER} checked the box...\p"
	.string "It contained the GAS POKéMON\n"
	.string "KOFFING! This one is quite weak.\p"
	.string "Will you take it with you?$"
PalletTown_PlayersHouse_2F_Text_ObtainedKoffing:
	.string "{PLAYER} obtained a KOFFING.$"
PalletTown_PlayersHouse_2F_Text_TakeGastly:
	.string "It's the box you taped up\n"
	.string "in the truck.\p"
	.string "It's... charred?\p"
	.string "Something inside it caused\n"
	.string "an EXPLOSION!\p"
	.string "{PLAYER} checked the box...\p"
	.string "It contained the GAS POKéMON\n"
	.string "GASTLY! This one is quite weak.\p"
	.string "Will you take it with you?$"
PalletTown_PlayersHouse_2F_Text_ObtainedGastly:
	.string "{PLAYER} obtained a GASTLY.$"
PalletTown_PlayersHouse_2F_Text_LeftGasPokemon:
	.string "{PLAYER} left the POKéMON where\n"
	.string "it was.$"
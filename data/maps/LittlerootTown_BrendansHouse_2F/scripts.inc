LittlerootTown_BrendansHouse_2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, LittlerootTown_BrendansHouse_2F_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, LittlerootTown_BrendansHouse_2F_OnWarp
	map_script MAP_SCRIPT_ON_LOAD, LittlerootTown_BrendansHouse_2F_OnLoad
	.byte 0

LittlerootTown_BrendansHouse_2F_OnTransition:
	call_if_lt VAR_LITTLEROOT_RIVAL_STATE, 2, LittlerootTown_BrendansHouse_2F_EventScript_CheckSetReadyToMeetBrendan
	call_if_ge VAR_LITTLEROOT_RIVAL_STATE, 3, LittlerootTown_BrendansHouse_2F_EventScript_CheckShouldUpdateBrendanPos
	call_if_eq VAR_LITTLEROOT_INTRO_STATE, 4, PlayersHouse_2F_EventScript_BlockStairsUntilClockIsSet
	call SecretBase_EventScript_SetDecorationFlags
	setvar VAR_SECRET_BASE_INITIALIZED, 0
	end

LittlerootTown_BrendansHouse_2F_OnLoad::
	call_if_set FLAG_HIDE_BRENDANSHOUSE_DOLL, LittlerootTown_BrendansHouse_2F_EventScript_HideDollBox
	end

LittlerootTown_BrendansHouse_2F_EventScript_CheckShouldUpdateBrendanPos::
	goto_if_set FLAG_MET_RIVAL_LILYCOVE, LittlerootTown_BrendansHouse_2F_EventScript_TryUpdateBrendanPos
	goto_if_ge VAR_BIRCH_LAB_STATE, 2, LittlerootTown_BrendansHouse_2F_EventScript_Ret
	goto LittlerootTown_BrendansHouse_2F_EventScript_TryUpdateBrendanPos

LittlerootTown_BrendansHouse_2F_EventScript_TryUpdateBrendanPos::
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, LittlerootTown_BrendansHouse_2F_EventScript_Ret
	@ Odd that the MaysHouse equivalent was used below instead
	goto_if_ge VAR_DEX_UPGRADE_JOHTO_STARTER_STATE, 2, LittlerootTown_MaysHouse_2F_EventScript_Ret
	setobjectxyperm LOCALID_RIVALS_HOUSE_2F_RIVAL, 0, 2
	setobjectmovementtype LOCALID_RIVALS_HOUSE_2F_RIVAL, MOVEMENT_TYPE_FACE_UP
	return

LittlerootTown_BrendansHouse_2F_EventScript_Ret::
	return

LittlerootTown_BrendansHouse_2F_EventScript_CheckSetReadyToMeetBrendan::
	checkplayergender
	goto_if_eq VAR_RESULT, FEMALE, LittlerootTown_BrendansHouse_2F_EventScript_SetReadyToMeetBrendan
	return

LittlerootTown_BrendansHouse_2F_EventScript_SetReadyToMeetBrendan::
	setvar VAR_LITTLEROOT_RIVAL_STATE, 2
	return

LittlerootTown_BrendansHouse_2F_OnWarp:
	map_script_2 VAR_SECRET_BASE_INITIALIZED, 0, LittlerootTown_BrendansHouse_2F_EventScript_CheckInitDecor
	.2byte 0

LittlerootTown_BrendansHouse_2F_EventScript_CheckInitDecor::
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, SecretBase_EventScript_InitDecorations
	end

LittlerootTown_BrendansHouse_2F_EventScript_RivalsPokeBall::
	lockall
	goto_if_eq VAR_LITTLEROOT_RIVAL_STATE, 2, LittlerootTown_BrendansHouse_2F_EventScript_MeetBrendan
	msgbox RivalsHouse_2F_Text_ItsRivalsPokeBall, MSGBOX_DEFAULT
	releaseall
	end

LittlerootTown_BrendansHouse_2F_EventScript_MeetBrendan::
	delay 10
	addobject LOCALID_RIVALS_HOUSE_2F_RIVAL
	applymovement LOCALID_RIVALS_HOUSE_2F_RIVAL, LittlerootTown_BrendansHouse_2F_Movement_BrendanEnters
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_RIVALS_HOUSE_2F_RIVAL, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_RIVALS_HOUSE_2F_RIVAL, Common_Movement_Delay48
	waitmovement 0
	delay 10
	playbgm MUS_ENCOUNTER_BRENDAN, TRUE
	call_if_eq VAR_FACING, DIR_NORTH, LittlerootTown_BrendansHouse_2F_EventScript_MeetBrendanNorth
	call_if_eq VAR_FACING, DIR_SOUTH, LittlerootTown_BrendansHouse_2F_EventScript_MeetBrendanSouth
	call_if_eq VAR_FACING, DIR_WEST, LittlerootTown_BrendansHouse_2F_EventScript_MeetBrendanWest
	call_if_eq VAR_FACING, DIR_EAST, LittlerootTown_BrendansHouse_2F_EventScript_MeetBrendanEast
	setvar VAR_LITTLEROOT_RIVAL_STATE, 3
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F_POKE_BALL
	clearflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_BEDROOM
	setvar VAR_LITTLEROOT_TOWN_STATE, 1
	savebgm MUS_DUMMY
	fadedefaultbgm
	releaseall
	end

LittlerootTown_BrendansHouse_2F_EventScript_MeetBrendanNorth::
	applymovement LOCALID_RIVALS_HOUSE_2F_RIVAL, LittlerootTown_BrendansHouse_2F_Movement_BrendanApproachPlayerNorth
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	msgbox RivalsHouse_2F_Text_BrendanWhoAreYou, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, LittlerootTown_BrendansHouse_2F_Movement_PlayerWatchBrendanNorth
	applymovement LOCALID_RIVALS_HOUSE_2F_RIVAL, LittlerootTown_BrendansHouse_2F_Movement_BrendanWalkToPCNorth
	waitmovement 0
	return

LittlerootTown_BrendansHouse_2F_EventScript_MeetBrendanSouth::
	applymovement LOCALID_RIVALS_HOUSE_2F_RIVAL, LittlerootTown_BrendansHouse_2F_Movement_BrendanApproachPlayerSouth
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	msgbox RivalsHouse_2F_Text_BrendanWhoAreYou, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, LittlerootTown_BrendansHouse_2F_Movement_PlayerWatchBrendanSouth
	applymovement LOCALID_RIVALS_HOUSE_2F_RIVAL, LittlerootTown_BrendansHouse_2F_Movement_BrendanWalkToPCSouth
	waitmovement 0
	return

LittlerootTown_BrendansHouse_2F_EventScript_MeetBrendanWest::
	applymovement LOCALID_RIVALS_HOUSE_2F_RIVAL, LittlerootTown_BrendansHouse_2F_Movement_BrendanApproachPlayerWest
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	msgbox RivalsHouse_2F_Text_BrendanWhoAreYou, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_PLAYER, LittlerootTown_BrendansHouse_2F_Movement_PlayerWatchBrendanWest
	applymovement LOCALID_RIVALS_HOUSE_2F_RIVAL, LittlerootTown_BrendansHouse_2F_Movement_BrendanWalkToPCWest
	waitmovement 0
	return

LittlerootTown_BrendansHouse_2F_EventScript_MeetBrendanEast::
	applymovement LOCALID_RIVALS_HOUSE_2F_RIVAL, LittlerootTown_BrendansHouse_2F_Movement_BrendanApproachPlayerEast
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	msgbox RivalsHouse_2F_Text_BrendanWhoAreYou, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_RIVALS_HOUSE_2F_RIVAL, LittlerootTown_BrendansHouse_2F_Movement_BrendanWalkToPCEast
	waitmovement 0
	return

LittlerootTown_BrendansHouse_2F_Movement_BrendanEnters:
	walk_down
	walk_down
	walk_in_place_faster_left
	step_end

LittlerootTown_BrendansHouse_2F_Movement_BrendanApproachPlayerNorth:
	walk_left
	walk_left
	walk_down
	walk_down
	walk_left
	step_end

LittlerootTown_BrendansHouse_2F_Movement_BrendanWalkToPCNorth:
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

LittlerootTown_BrendansHouse_2F_Movement_PlayerWatchBrendanNorth:
	delay_16
	walk_in_place_faster_up
	delay_16
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_left
	step_end

LittlerootTown_BrendansHouse_2F_Movement_BrendanApproachPlayerSouth:
	walk_left
	walk_left
	walk_left
	step_end

LittlerootTown_BrendansHouse_2F_Movement_BrendanWalkToPCSouth:
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

LittlerootTown_BrendansHouse_2F_Movement_PlayerWatchBrendanSouth:
	delay_16
	walk_in_place_faster_up
	delay_16
	delay_16
	walk_in_place_faster_left
	step_end

LittlerootTown_BrendansHouse_2F_Movement_BrendanApproachPlayerWest:
	walk_left
	walk_left
	walk_down
	walk_in_place_faster_left
	step_end

LittlerootTown_BrendansHouse_2F_Movement_BrendanWalkToPCWest:
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

LittlerootTown_BrendansHouse_2F_Movement_PlayerWatchBrendanWest:
	delay_8
	delay_16
	walk_in_place_faster_up
	delay_16
	delay_16
	walk_in_place_faster_left
	step_end

LittlerootTown_BrendansHouse_2F_Movement_BrendanApproachPlayerEast:
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_down
	step_end

LittlerootTown_BrendansHouse_2F_Movement_BrendanWalkToPCEast:
	walk_up
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

@ Unused, the player is already facing this direction so its unneeded
LittlerootTown_BrendansHouse_2F_Movement_PlayerWatchBrendanEast:
	delay_16
	delay_16
	walk_in_place_faster_left
	step_end

LittlerootTown_BrendansHouse_2F_EventScript_PC::
	lockall
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, LittlerootTown_BrendansHouse_2F_EventScript_CheckPlayersPC
	goto_if_eq VAR_RESULT, FEMALE, LittlerootTown_BrendansHouse_2F_EventScript_CheckRivalsPC
	end

LittlerootTown_BrendansHouse_2F_EventScript_CheckPlayersPC::
	setvar VAR_0x8004, PC_LOCATION_BRENDANS_HOUSE
	special DoPCTurnOnEffect
	playse SE_PC_ON
	msgbox gText_PlayerHouseBootPC, MSGBOX_DEFAULT
	special BedroomPC
	waitstate
	releaseall
	end

LittlerootTown_BrendansHouse_2F_EventScript_TurnOffPlayerPC::
	setvar VAR_0x8004, PC_LOCATION_BRENDANS_HOUSE
	playse SE_PC_OFF
	special DoPCTurnOffEffect
	releaseall
	end

LittlerootTown_BrendansHouse_2F_EventScript_CheckRivalsPC::
	msgbox gText_PokemonTrainerSchoolEmail, MSGBOX_DEFAULT
	releaseall
	end

PlayersHouse_2F_EventScript_Notebook::
	msgbox PlayersHouse_2F_Text_Notebook, MSGBOX_SIGN
	end

PlayersHouse_2F_EventScript_GameCube::
	msgbox PlayersHouse_2F_Text_ItsAGameCube, MSGBOX_SIGN
	end

LittlerootTown_BrendansHouse_2F_EventScript_HideDollBox::
	setmetatile 8, 6, METATILE_BrendansMaysHouse_NormalFloor, FALSE
	setmetatile 7, 7, METATILE_BrendansMaysHouse_NormalFloor, FALSE
	setmetatile 8, 7, METATILE_BrendansMaysHouse_NormalFloor, FALSE
	return

LittlerootTown_BrendansHouse_2F_EventScript_DollBox::	
	call_if_set FLAG_CHOSE_BALTOY, LittlerootTown_BrendansHouse_2F_EventScript_BaltoyDoll
	call_if_set FLAG_CHOSE_SHUPPET, LittlerootTown_BrendansHouse_2F_EventScript_ShuppetDoll
	end

@Baltoy Doll

LittlerootTown_BrendansHouse_2F_EventScript_BaltoyDoll::
	lockall
	msgbox LittlerootTown_BrendansHouse_2F_Text_TakeBallContainingBaltoy, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LittlerootTown_BrendansHouse_2F_EventScript_LeaveBaltoy
	goto LittlerootTown_BrendansHouse_2F_EventScript_GiveBaltoy
	end

LittlerootTown_BrendansHouse_2F_EventScript_LeaveBaltoy::
	msgbox LittlerootTown_BrendansHouse_2F_Text_LeftDollWhereItWas, MSGBOX_DEFAULT
	releaseall
	end

LittlerootTown_BrendansHouse_2F_EventScript_GiveBaltoy::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_BALTOY
	givemon SPECIES_BALTOY, 5
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PARTY, LittlerootTown_BrendansHouse_2F_EventScript_SendBaltoyParty
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, LittlerootTown_BrendansHouse_2F_EventScript_SendBaltoyPC
	goto Common_EventScript_NoMoreRoomForPokemon
	end

LittlerootTown_BrendansHouse_2F_EventScript_SendBaltoyParty::
	call LittlerootTown_BrendansHouse_2F_EventScript_ReceivedBaltoyFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LittlerootTown_BrendansHouse_2F_EventScript_ReceivedBaltoy
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto LittlerootTown_BrendansHouse_2F_EventScript_ReceivedBaltoy
	end

LittlerootTown_BrendansHouse_2F_EventScript_SendBaltoyPC::
	call LittlerootTown_BrendansHouse_2F_EventScript_ReceivedBaltoyFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LittlerootTown_BrendansHouse_2F_EventScript_BaltoyTransferredToPC
	call Common_EventScript_NameReceivedBoxMon
	goto LittlerootTown_BrendansHouse_2F_EventScript_BaltoyTransferredToPC
	end

LittlerootTown_BrendansHouse_2F_EventScript_BaltoyTransferredToPC::
	call Common_EventScript_TransferredToPC
	goto LittlerootTown_BrendansHouse_2F_EventScript_ReceivedBaltoy
	end

LittlerootTown_BrendansHouse_2F_EventScript_ReceivedBaltoyFanfare::
	bufferspeciesname STR_VAR_2, SPECIES_BALTOY
	removeobject LOCALID_BRENDAN_DOLL
	playfanfare MUS_OBTAIN_ITEM
	message LittlerootTown_BrendansHouse_2F_Text_ObtainedBaltoy
	waitmessage
	waitfanfare
	bufferspeciesname STR_VAR_1, SPECIES_BALTOY
	return

LittlerootTown_BrendansHouse_2F_EventScript_ReceivedBaltoy::
	setflag FLAG_HIDE_BRENDANSHOUSE_DOLL
	setflag FLAG_RECEIVED_BALTOY_DOLL
	releaseall
	end

LittlerootTown_BrendansHouse_2F_Text_TakeBallContainingBaltoy:
	.string "{PLAYER} checked the box...\p"
	.string "It contained the TOY POKEMON\n"
	.string "BALTOY.\p"
	.string "Take it?$"

LittlerootTown_BrendansHouse_2F_Text_ObtainedBaltoy:
	.string "{PLAYER} obtained a BALTOY.$"

@Shuppet Doll

LittlerootTown_BrendansHouse_2F_EventScript_ShuppetDoll::
	lockall
	msgbox LittlerootTown_BrendansHouse_2F_Text_TakeBallContainingShuppet, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LittlerootTown_BrendansHouse_2F_EventScript_LeaveShuppet
	goto LittlerootTown_BrendansHouse_2F_EventScript_GiveShuppet
	end

LittlerootTown_BrendansHouse_2F_EventScript_LeaveShuppet::
	msgbox LittlerootTown_BrendansHouse_2F_Text_LeftDollWhereItWas, MSGBOX_DEFAULT
	releaseall
	end

LittlerootTown_BrendansHouse_2F_EventScript_GiveShuppet::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_SHUPPET
	givemon SPECIES_SHUPPET, 5
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PARTY, LittlerootTown_BrendansHouse_2F_EventScript_SendShuppetParty
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, LittlerootTown_BrendansHouse_2F_EventScript_SendShuppetPC
	goto Common_EventScript_NoMoreRoomForPokemon
	end

LittlerootTown_BrendansHouse_2F_EventScript_SendShuppetParty::
	call LittlerootTown_BrendansHouse_2F_EventScript_ReceivedShuppetFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LittlerootTown_BrendansHouse_2F_EventScript_ReceivedShuppet
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto LittlerootTown_BrendansHouse_2F_EventScript_ReceivedShuppet
	end

LittlerootTown_BrendansHouse_2F_EventScript_SendShuppetPC::
	call LittlerootTown_BrendansHouse_2F_EventScript_ReceivedShuppetFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, LittlerootTown_BrendansHouse_2F_EventScript_ShuppetTransferredToPC
	call Common_EventScript_NameReceivedBoxMon
	goto LittlerootTown_BrendansHouse_2F_EventScript_ShuppetTransferredToPC
	end

LittlerootTown_BrendansHouse_2F_EventScript_ShuppetTransferredToPC::
	call Common_EventScript_TransferredToPC
	goto LittlerootTown_BrendansHouse_2F_EventScript_ReceivedShuppet
	end

LittlerootTown_BrendansHouse_2F_EventScript_ReceivedShuppetFanfare::
	bufferspeciesname STR_VAR_2, SPECIES_SHUPPET
	removeobject LOCALID_BRENDAN_DOLL
	playfanfare MUS_OBTAIN_ITEM
	message LittlerootTown_BrendansHouse_2F_Text_ObtainedShuppet
	waitmessage
	waitfanfare
	bufferspeciesname STR_VAR_1, SPECIES_SHUPPET
	return

LittlerootTown_BrendansHouse_2F_EventScript_ReceivedShuppet::
	setflag FLAG_HIDE_BRENDANSHOUSE_DOLL
	setflag FLAG_RECEIVED_SHUPPET_DOLL
	releaseall
	end

LittlerootTown_BrendansHouse_2F_Text_TakeBallContainingShuppet:
	.string "{PLAYER} checked the box...\p"
	.string "It contained the PUPPET POKEMON\n"
	.string "SHUPPET.\p"
	.string "Take it?$"

LittlerootTown_BrendansHouse_2F_Text_ObtainedShuppet:
	.string "{PLAYER} obtained a SHUPPET.$"

@ Unused
LittlerootTown_BrendansHouse_2F_Text_NoSpaceForAnotherMon:
	.string "There is no space for another POKéMON.$"

LittlerootTown_BrendansHouse_2F_Text_LeftDollWhereItWas:
	.string "{PLAYER} left the POKé BALL where\n"
	.string "it was.$"

PlayersHouse_2F_Text_ClockIsStopped:
	.string "The clock is stopped…\p"
	.string "Better set it and start it!$"

PlayersHouse_2F_Text_HowDoYouLikeYourRoom:
	.string "MOM: {PLAYER}, how do you like your\n"
	.string "new room?\p"
	.string "Good! Everything's put away neatly!\p"
	.string "They finished moving everything in\n"
	.string "downstairs, too.\p"
	.string "POKéMON movers are so convenient!\p"
	.string "Oh, you should make sure that\n"
	.string "everything's all there on your desk.$"

PlayersHouse_2F_Text_Notebook:
	.string "{PLAYER} flipped open the notebook.\p"
	.string "ADVENTURE RULE NO. 1\n"
	.string "Open the MENU with START.\p"
	.string "ADVENTURE RULE NO. 2\n"
	.string "Record your progress with SAVE.\p"
	.string "The remaining pages are blank…$"

Common_Text_LookCloserAtMap:
	.string "{PLAYER} took a closer look at the\n"
	.string "HOENN region map.$"

PlayersHouse_2F_Text_ItsAGameCube:
	.string "It's a Nintendo GameCube.\p"
	.string "A Game Boy Advance is connected to\n"
	.string "serve as the Controller.$"

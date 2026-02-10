OldaleShed_MapScripts::
	.byte 0

OldaleShed_EventScript_ShuppetPokeball::
	lockall
	msgbox OldaleShed_Text_TakeBallContainingShuppet, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleShed_EventScript_LeaveShuppet
	goto OldaleShed_EventScript_GiveShuppet
	end

OldaleShed_EventScript_LeaveShuppet::
	msgbox OldaleShed_Text_LeftPokeBallWhereItWas, MSGBOX_DEFAULT
	releaseall
	end

OldaleShed_EventScript_GiveShuppet::
	setvar VAR_TEMP_TRANSFERRED_SPECIES, SPECIES_SHUPPET
	givemon SPECIES_SHUPPET, 5
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PARTY, OldaleShed_EventScript_SendShuppetParty
	goto_if_eq VAR_RESULT, MON_GIVEN_TO_PC, OldaleShed_EventScript_SendShuppetPC
	goto Common_EventScript_NoMoreRoomForPokemon
	end

OldaleShed_EventScript_SendShuppetParty::
	call OldaleShed_EventScript_ReceivedShuppetFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleShed_EventScript_ReceivedShuppet
	call Common_EventScript_GetGiftMonPartySlot
	call Common_EventScript_NameReceivedPartyMon
	goto OldaleShed_EventScript_ReceivedShuppet
	end

OldaleShed_EventScript_SendShuppetPC::
	call OldaleShed_EventScript_ReceivedShuppetFanfare
	msgbox gText_NicknameThisPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleShed_EventScript_ShuppetTransferredToPC
	call Common_EventScript_NameReceivedBoxMon
	goto OldaleShed_EventScript_ShuppetTransferredToPC
	end

OldaleShed_EventScript_ShuppetTransferredToPC::
	call Common_EventScript_TransferredToPC
	goto OldaleShed_EventScript_ReceivedShuppet
	end

OldaleShed_EventScript_ReceivedShuppetFanfare::
	bufferspeciesname STR_VAR_2, SPECIES_SHUPPET
	removeobject LOCALID_OLDALESHED_BALL
	playfanfare MUS_OBTAIN_ITEM
	message OldaleShed_Text_ObtainedShuppet
	waitmessage
	waitfanfare
	bufferspeciesname STR_VAR_1, SPECIES_SHUPPET
	return

OldaleShed_EventScript_ReceivedShuppet::
	setflag FLAG_HIDE_OLDALESHED_SHUPPET_POKEBALL
	setflag FLAG_RECEIVED_SHUPPET_DOLL
	releaseall
	end

@ Debug NPC for testing Birch state
OldaleShed_EventScript_DebugBirch::
	lock
	faceplayer
	msgbox OldaleShed_Text_DebugBirchAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, OldaleShed_EventScript_DebugAdvanceDay
	goto OldaleShed_EventScript_DebugAdvanceBirch
	end

OldaleShed_EventScript_DebugAdvanceDay::
	special DebugAdvanceFakeRtcDay
	dotimebasedevents
	msgbox OldaleShed_Text_DebugDayAdvanced, MSGBOX_DEFAULT
	release
	end

OldaleShed_EventScript_DebugAdvanceBirch::
	addvar VAR_BIRCH_STATE, 1
	goto_if_lt VAR_BIRCH_STATE, 7, OldaleShed_EventScript_DebugBirchDone
	setvar VAR_BIRCH_STATE, 0
OldaleShed_EventScript_DebugBirchDone::
	msgbox OldaleShed_Text_DebugBirchAdvanced, MSGBOX_DEFAULT
	release
	end

OldaleShed_EventScript_GymFlagsPot::
	msgbox OldaleShed_Text_SetGymFlags, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, YES, OldaleShed_EventScript_SetGymFlags
	clearflag FLAG_DEFEATED_RUSTBORO_GYM
	clearflag FLAG_DEFEATED_DEWFORD_GYM
	clearflag FLAG_DEFEATED_MAUVILLE_GYM
	clearflag FLAG_DEFEATED_LAVARIDGE_GYM
	clearflag FLAG_DEFEATED_PETALBURG_GYM
	clearflag FLAG_DEFEATED_FORTREE_GYM
	clearflag FLAG_DEFEATED_MOSSDEEP_GYM
	clearflag FLAG_DEFEATED_SOOTOPOLIS_GYM
	clearflag FLAG_BADGE01_GET
	clearflag FLAG_BADGE02_GET
	clearflag FLAG_BADGE03_GET
	clearflag FLAG_BADGE04_GET
	clearflag FLAG_BADGE05_GET
	clearflag FLAG_BADGE06_GET
	clearflag FLAG_BADGE07_GET
	clearflag FLAG_BADGE08_GET
	clearflag NUM_BADGES  
	setflag FLAG_HIDE_HITMON_ATOLL_BRAWLY
	end

OldaleShed_EventScript_SetGymFlags::
	setflag FLAG_DEFEATED_RUSTBORO_GYM
	setflag FLAG_DEFEATED_DEWFORD_GYM
	setflag FLAG_DEFEATED_MAUVILLE_GYM
	setflag FLAG_DEFEATED_LAVARIDGE_GYM
	setflag FLAG_DEFEATED_PETALBURG_GYM
	setflag FLAG_DEFEATED_FORTREE_GYM
	setflag FLAG_DEFEATED_MOSSDEEP_GYM
	setflag FLAG_DEFEATED_SOOTOPOLIS_GYM
	setflag FLAG_BADGE01_GET
	setflag FLAG_BADGE02_GET
	setflag FLAG_BADGE03_GET
	setflag FLAG_BADGE04_GET
	setflag FLAG_BADGE05_GET
	setflag FLAG_BADGE06_GET
	setflag FLAG_BADGE07_GET
	setflag FLAG_BADGE08_GET
	setflag NUM_BADGES    
	end

OldaleShed_Text_ObtainedShuppet:
	.string "{PLAYER} obtained a SHUPPET.$"

@ Unused
OldaleShed_Text_NoSpaceForAnotherMon:
	.string "There is no space for another POKéMON.$"

OldaleShed_Text_LeftPokeBallWhereItWas:
	.string "{PLAYER} left the POKé BALL where\n"
	.string "it was.$"

OldaleShed_Text_TakeBallContainingShuppet:
	.string "{PLAYER} checked the POKé BALL.\p"
	.string "It contained the POKéMON\n"
	.string "SHUPPET.\p"
	.string "Take the POKé BALL?$"

OldaleShed_Text_DebugBirchAsk:
	.string "YES = Advance day\n"
	.string "NO = Advance Birch state$"

OldaleShed_Text_DebugDayAdvanced:
	.string "Day advanced by 1.$"

OldaleShed_Text_DebugBirchAdvanced:
	.string "Birch state advanced by 1.$"

OldaleShed_Text_SetGymFlags:
	.string "Set all 8 gym flags?\p"
	.string "Yes = set flags\n"
	.string "No = clear flags$"

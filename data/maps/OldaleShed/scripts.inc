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
	setflag FLAG_RECEIVED_SHUPPET
	releaseall
	end

	OldaleShed_Text_TakeBallContainingShuppet:
	.string "{PLAYER} checked the POKé BALL.\p"
	.string "It contained the POKéMON\n"
	.string "SHUPPET.\p"
	.string "Take the POKé BALL?$"

OldaleShed_Text_ObtainedShuppet:
	.string "{PLAYER} obtained a SHUPPET.$"

@ Unused
OldaleShed_Text_NoSpaceForAnotherMon:
	.string "There is no space for another POKéMON.$"

OldaleShed_Text_LeftPokeBallWhereItWas:
	.string "{PLAYER} left the POKé BALL where\n"
	.string "it was.$"
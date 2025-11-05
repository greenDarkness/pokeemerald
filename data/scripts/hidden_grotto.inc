@ Hidden Grotto entrances (see src/data/hidden_grottos.h). VAR_0x8004 = grotto slot.
HiddenGrotto_EventScript_Entrance::
	lockall
	msgbox HiddenGrotto_Text_SmallOpening, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HiddenGrotto_EventScript_CancelEntrance
	closemessage
	playse SE_EXIT
	special EnterHiddenGrotto
	end

HiddenGrotto_EventScript_CancelEntrance::
	closemessage
	releaseall
	end

HiddenGrotto_Text_SmallOpening:
	.string "There's a small opening hidden here.\n"
	.string "Would you like to go inside?$"

EventScript_SweetScentAgain::
	callnative PartyCanUseSweetScent
	compare VAR_RESULT, FALSE
	goto_if_eq EventScript_SweetScentAgain_End
	msgbox Text_SweetScentAgain, MSGBOX_YESNO
	closemessage
	compare VAR_RESULT, 0
	goto_if_eq EventScript_SweetScentAgain_End
	callnative StartSweetScentFromScript
EventScript_SweetScentAgain_End:
	end

Text_SweetScentAgain:
	.string "Use SWEET SCENT again?$"

EventScript_UseHoney::
	lockall
	msgbox Text_PlayerUsedHoney, MSGBOX_DEFAULT
	closemessage
	releaseall
	callnative StartSweetScentFromHoney
	end

EventScript_HoneyAgain::
	checkitem ITEM_HONEY
	goto_if_eq VAR_RESULT, FALSE, EventScript_HoneyAgain_End
	msgbox Text_HoneyAgain, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, NO, EventScript_HoneyAgain_End
	removeitem ITEM_HONEY
	bufferitemname STR_VAR_2, ITEM_HONEY
	goto EventScript_UseHoney
EventScript_HoneyAgain_End:
	end

Text_HoneyAgain:
	.string "Would you like to use another\n"
	.string "HONEY?$"

Text_PlayerUsedHoney:
	.string "{PLAYER} used the\n"
	.string "{STR_VAR_2}.$"

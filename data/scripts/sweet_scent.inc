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

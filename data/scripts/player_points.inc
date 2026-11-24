@ Gains of the player's own stats: max IP (item points) and max AP (action points). Both are
@ announced with the level-up fanfare.

@ Raises the player's max IP by VAR_0x8000 and tells them how much they gained (nothing, if max
@ IP is as high as it gets).
Common_EventScript_GainIP::
	callnative IncreasePlayerIP
	goto_if_eq VAR_0x8000, 0, Common_EventScript_NopReturn
	buffernumberstring STR_VAR_1, VAR_0x8000
	playfanfare MUS_LEVEL_UP
	message Common_Text_GainedIP
	waitmessage
	waitfanfare
	waitbuttonpress
	return

@ Max AP comes from the badges (GetPlayerAPMax): call this right after giving one, it tells the
@ player if that badge was worth an AP.
Common_EventScript_TryGainAP::
	specialvar VAR_RESULT, DidBadgeRaisePlayerAP
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_NopReturn
	playfanfare MUS_LEVEL_UP
	message Common_Text_GainedAP
	waitmessage
	waitfanfare
	waitbuttonpress
	return

Common_Text_GainedIP:
	.string "You gained {STR_VAR_1} IP!$"

Common_Text_GainedAP:
	.string "You gained 1 AP!$"

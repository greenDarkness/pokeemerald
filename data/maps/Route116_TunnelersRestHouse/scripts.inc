Route116_TunnelersRestHouse_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route116_TunnelersRestHouse_OnTransition
	.byte 0

Route116_TunnelersRestHouse_OnTransition:
	setflag FLAG_LANDMARK_TUNNELERS_REST_HOUSE
	end

Route116_TunnelersRestHouse_EventScript_Tunneler1::
	msgbox Route116_TunnelersRestHouse_Text_WeHadToStopBoring, MSGBOX_NPC
	end

Route116_TunnelersRestHouse_EventScript_Tunneler2::
	msgbox Route116_TunnelersRestHouse_Text_ManDiggingHisWayToVerdanturf, MSGBOX_NPC
	end

Route116_TunnelersRestHouse_EventScript_Tunneler3::
	lock
	faceplayer
	goto_if_set FLAG_RUSTURF_TUNNEL_OPENED, Route116_TunnelersRestHouse_EventScript_TunnelOpened
	msgbox Route116_TunnelersRestHouse_Text_GetToVerdanturfWithoutTunnel, MSGBOX_DEFAULT
	release
	end

Route116_TunnelersRestHouse_EventScript_TunnelOpened::
	msgbox Route116_TunnelersRestHouse_Text_TunnelHasGoneThrough, MSGBOX_DEFAULT
	release
	end

Route116_TunnelersRestHouse_EventScript_Trader::
	lock
	faceplayer
	goto_if_set FLAG_ROUTE116_NPC_TRADE_COMPLETED, Route116_TunnelersRestHouse_EventScript_TradeCompleted
	setvar VAR_0x8008, INGAME_TRADE_MR_MIME
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	copyvar VAR_0x8009, VAR_RESULT
	msgbox Route116_TunnelersRestHouse_Text_IllTradeIfYouWant, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, Route116_TunnelersRestHouse_EventScript_DeclineTrade
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, Route116_TunnelersRestHouse_EventScript_DeclineTrade
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	copyvar VAR_0x800B, VAR_RESULT
	goto_if_ne VAR_RESULT, VAR_0x8009, Route116_TunnelersRestHouse_EventScript_NotRequestedMon
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateInGameTradePokemon
	special DoInGameTradeScene
	waitstate
	msgbox Route116_TunnelersRestHouse_Text_PleaseBeGoodToMyPokemon, MSGBOX_DEFAULT
	setflag FLAG_ROUTE116_NPC_TRADE_COMPLETED
	release
	end

Route116_TunnelersRestHouse_EventScript_DeclineTrade::
	msgbox Route116_TunnelersRestHouse_Text_YouDontWantToThatsOkay, MSGBOX_DEFAULT
	release
	end

Route116_TunnelersRestHouse_EventScript_NotRequestedMon::
	bufferspeciesname STR_VAR_1, VAR_0x8009
	msgbox Route116_TunnelersRestHouse_Text_DoesntLookLikeMonToMe, MSGBOX_DEFAULT
	release
	end

Route116_TunnelersRestHouse_EventScript_TradeCompleted::
	msgbox Route116_TunnelersRestHouse_Text_AnyPokemonCanBeCute, MSGBOX_DEFAULT
	release
	end

Route116_TunnelersRestHouse_Text_WeHadToStopBoring:
	.string "That RUSTURF TUNNEL there…\p"
	.string "At first, we had a huge work crew boring\n"
	.string "through rock with the latest machinery.\l"
	.string "But, we had to stop.\p"
	.string "It turns out that we would have had\n"
	.string "a negative effect on wild POKéMON in\l"
	.string "the area.\p"
	.string "So, we've got nothing to do but loll\n"
	.string "around here doing nothing.$"

Route116_TunnelersRestHouse_Text_ManDiggingHisWayToVerdanturf:
	.string "There's a man digging his way to\n"
	.string "VERDANTURF all by his lonesome.\l"
	.string "He's desperate to get through.\p"
	.string "He says that if he digs little by little\n"
	.string "without using machines, he won't\l"
	.string "disturb POKéMON, and he'll avoid\l"
	.string "harming the natural environment.\p"
	.string "I wonder if he made it through yet.$"

Route116_TunnelersRestHouse_Text_GetToVerdanturfWithoutTunnel:
	.string "To get to VERDANTURF without using\n"
	.string "this TUNNEL, you'd have to cross the\l"
	.string "sea to DEWFORD, sail on to SLATEPORT,\l"
	.string "then travel through MAUVILLE.$"

Route116_TunnelersRestHouse_Text_TunnelHasGoneThrough:
	.string "Did you hear? The TUNNEL to VERDANTURF\n"
	.string "has gone through!\p"
	.string "Sometimes, if you hope strongly enough,\n"
	.string "dreams do come true.$"

Route116_TunnelersRestHouse_Text_IllTradeIfYouWant:
	.string "Rusturf Tunnel is full of noisy POKEMON,\n"
	.string "but my POKEMON's ability protects me.\p"
	.string "I could trade you one, if you find a\n"
	.string "rare POKEMON for me.\p"
	.string "I'll trade you my {STR_VAR_2} for\n"
	.string "a {STR_VAR_1}.$"

Route116_TunnelersRestHouse_Text_PleaseBeGoodToMyPokemon:
	.string "Hey there, SEEDOT. I think I'll call you\n"
	.string "DOTS...$"

Route116_TunnelersRestHouse_Text_DoesntLookLikeMonToMe:
	.string "Huh? That doesn't look anything like\n"
	.string "a {STR_VAR_1} to me.$"

Route116_TunnelersRestHouse_Text_YouDontWantToThatsOkay:
	.string "Oh, if you don't want to, that's okay.\n"
	.string "But my POKéMON is useful, you know…$"

Route116_TunnelersRestHouse_Text_AnyPokemonCanBeCute:
	.string "Any POKéMON can be useful if you find\n"
	.string "the right time to choose it.$"

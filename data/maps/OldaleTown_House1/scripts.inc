OldaleTown_House1_MapScripts::
	.byte 0

OldaleTown_House1_EventScript_Trader::
	lock
	faceplayer
	goto_if_set FLAG_OLDALE_NPC_TRADE_COMPLETED, OldaleTown_House1_EventScript_TradeCompleted
	setvar VAR_0x8008, INGAME_TRADE_POLIWRATH
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	copyvar VAR_0x8009, VAR_RESULT
	msgbox OldaleTown_House1_Text_IllTradeIfYouWant, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_House1_EventScript_DeclineTrade
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, OldaleTown_House1_EventScript_DeclineTrade
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	copyvar VAR_0x800B, VAR_RESULT
	goto_if_ne VAR_RESULT, VAR_0x8009, OldaleTown_House1_EventScript_NotRequestedMon
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateInGameTradePokemon
	special DoInGameTradeScene
	waitstate
	msgbox OldaleTown_House1_Text_PleaseBeGoodToMyPokemon, MSGBOX_DEFAULT
	setflag FLAG_OLDALE_NPC_TRADE_COMPLETED
	release
	end

OldaleTown_House1_EventScript_DeclineTrade::
	msgbox OldaleTown_House1_Text_YouDontWantToThatsOkay, MSGBOX_DEFAULT
	release
	end

OldaleTown_House1_EventScript_NotRequestedMon::
	bufferspeciesname STR_VAR_1, VAR_0x8009
	msgbox OldaleTown_House1_Text_DoesntLookLikeMonToMe, MSGBOX_DEFAULT
	release
	end

OldaleTown_House1_EventScript_TradeCompleted::
	msgbox OldaleTown_House1_Text_AnyPokemonCanBeCute, MSGBOX_DEFAULT
	release
	end

OldaleTown_House1_Text_IllTradeIfYouWant:
	.string "Sigh... the chores are piling up.\p"
	.string "I would ask my POKEMON for help, but it\n"
	.string "tends to be better at breaking things.\p"
	.string "I will trade it to you, if you have a\n"
	.string "POKEMON with a... quieter disposition.$"

OldaleTown_House1_Text_PleaseBeGoodToMyPokemon:
	.string "Welcome home, MR. MIME!\n"
	.string "Now let me show you the kitchen...$"

OldaleTown_House1_Text_DoesntLookLikeMonToMe:
	.string "Huh? That doesn't look like a helpful\n"
	.string "POKEMON to me.$"

OldaleTown_House1_Text_YouDontWantToThatsOkay:
	.string "Oh, if you don't want to, that's okay.\n"
	.string "But my POKéMON is strong, you know…$"

OldaleTown_House1_Text_AnyPokemonCanBeCute:
	.string "It feels like I retired after getting\n"
	.string "MR. MIME. Life has gotten much easier.\p"
	.string "Although its presence is still...\n"
	.string "unsettling.$"

OldaleTown_House1_Text_LeftPokemonGoesOutFirst:
	.string "When a POKéMON battle starts, the one\n"
	.string "at the left of the list goes out first.\p"
	.string "So, when you get more POKéMON in your\n"
	.string "party, try switching around the order\l"
	.string "of your POKéMON.\p"
	.string "It could give you an advantage.$"
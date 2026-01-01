DewfordTown_House1_MapScripts::
	.byte 0

DewfordTown_House1_EventScript_Man::
	msgbox DewfordTown_House1_Text_LotToBeSaidForLivingOnIsland, MSGBOX_NPC
	end

DewfordTown_House1_EventScript_Woman::
	msgbox DewfordTown_House1_Text_LifeGoesSlowlyOnIsland, MSGBOX_NPC
	end

DewfordTown_House1_EventScript_Zigzagoon::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_ZIGZAGOON, CRY_MODE_NORMAL
	msgbox DewfordTown_House1_Text_Zigzagoon, MSGBOX_DEFAULT
	waitmoncry
	release
	end

DewfordTown_House1_EventScript_HunterTrader::
	lock
	faceplayer
	goto_if_set FLAG_DEWFORD_HOUSE1_TRADE_COMPLETED, DewfordTown_House1_EventScript_TradeCompleted
	setvar VAR_0x8008, INGAME_TRADE_VIBRAVA
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	copyvar VAR_0x8009, VAR_RESULT
	msgbox DewfordTown_House1_Text_HunterTradeIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_House1_EventScript_DeclineTrade
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, DewfordTown_House1_EventScript_DeclineTrade
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	copyvar VAR_0x800B, VAR_RESULT
	goto_if_ne VAR_RESULT, VAR_0x8009, DewfordTown_House1_EventScript_NotRequestedMon
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateInGameTradePokemon
	special DoInGameTradeScene
	waitstate
	msgbox DewfordTown_House1_Text_HunterTradeCompleted, MSGBOX_DEFAULT
	setflag FLAG_DEWFORD_HOUSE1_TRADE_COMPLETED
	release
	end

DewfordTown_House1_EventScript_DeclineTrade::
	msgbox DewfordTown_House1_Text_HunterTradeCanceled, MSGBOX_DEFAULT
	release
	end

DewfordTown_House1_EventScript_NotRequestedMon::
	bufferspeciesname STR_VAR_1, VAR_0x8009
	msgbox DewfordTown_House1_Text_HunterTradeFailed, MSGBOX_DEFAULT
	release
	end

DewfordTown_House1_EventScript_TradeCompleted::
	msgbox DewfordTown_House1_Text_HunterTradeOver, MSGBOX_DEFAULT
	release
	end

DewfordTown_House1_Text_LotToBeSaidForLivingOnIsland:
	.string "There's a lot to be said for living on\n"
	.string "a small island like this in harmony with\l"
	.string "POKéMON and the family.$"

DewfordTown_House1_Text_LifeGoesSlowlyOnIsland:
	.string "I left the major port of SLATEPORT\n"
	.string "CITY when I married my husband here.\p"
	.string "Life goes by slowly on this little\n"
	.string "island. But being surrounded by the\l"
	.string "beautiful sea--that's happiness, too.$"

DewfordTown_House1_Text_Zigzagoon:
	.string "ZIGZAGOON: Guguuh!$"

DewfordTown_House1_Text_HunterTradeIntro:
	.string "There's a lot to be said for living on\n"
	.string "a small island like this in harmony with\l"
	.string "POKéMON and the family.\p"
	.string "For me, its the perfect setting to\n"
	.string "pursue my dream of training a team of\l"
	.string "FISH POKEMON.\p"
	.string "If you have a FISH POKEMON I haven't\n"
	.string "caught yet, I might trade my {STR_VAR_2}\l"
	.string "for it.$"

DewfordTown_House1_Text_HunterTradeCompleted:
	.string "MAGIKARP is one I was missing.\n"
	.string "Back home, its a lot more common.$"

DewfordTown_House1_Text_HunterTradeFailed:
	.string "Sorry, that is not a FISH POKEMON\n"
	.string "I am missing.$"

DewfordTown_House1_Text_HunterTradeCanceled:
	.string "Oh, if you don't want to, that's okay.\n"
	.string "But my {STR_VAR_2} might evolve soon,\l" 
	.string "you know…$"

DewfordTown_House1_Text_HunterTradeOver:
	.string "I was using {STR_VAR_2} for bait,\n"
	.string "but it got too strong.\p"
	.string "Now I'm back to using the common\n"
	.string "VENONAT.$"
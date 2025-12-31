PetalburgCity_House2_MapScripts::
	.byte 0

PetalburgCity_House2_EventScript_Woman::
	msgbox PetalburgCity_House2_Text_NormanBecameGymLeader, MSGBOX_NPC
	end

PetalburgCity_House2_EventScript_Trader::
	lock
	faceplayer
	goto_if_set FLAG_PETALBURG_NPC_TRADE_COMPLETED, PetalburgCity_House2_EventScript_TradeCompleted
	setvar VAR_0x8008, INGAME_TRADE_GASTLY
	copyvar VAR_0x8004, VAR_0x8008
	specialvar VAR_RESULT, GetInGameTradeSpeciesInfo
	copyvar VAR_0x8009, VAR_RESULT
	msgbox PetalburgCity_House2_Text_IllTradeIfYouWant, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PetalburgCity_House2_EventScript_DeclineTrade
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x800A, VAR_0x8004
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PetalburgCity_House2_EventScript_DeclineTrade
	copyvar VAR_0x8005, VAR_0x800A
	specialvar VAR_RESULT, GetTradeSpecies
	copyvar VAR_0x800B, VAR_RESULT
	goto_if_ne VAR_RESULT, VAR_0x8009, PetalburgCity_House2_EventScript_NotRequestedMon
	copyvar VAR_0x8004, VAR_0x8008
	copyvar VAR_0x8005, VAR_0x800A
	special CreateInGameTradePokemon
	special DoInGameTradeScene
	waitstate
	msgbox PetalburgCity_House2_Text_PleaseBeGoodToMyPokemon, MSGBOX_DEFAULT
	setflag FLAG_PETALBURG_NPC_TRADE_COMPLETED
	release
	end

PetalburgCity_House2_EventScript_DeclineTrade::
	msgbox PetalburgCity_House2_Text_YouDontWantToThatsOkay, MSGBOX_DEFAULT
	release
	end

PetalburgCity_House2_EventScript_NotRequestedMon::
	bufferspeciesname STR_VAR_1, VAR_0x8009
	msgbox PetalburgCity_House2_Text_DoesntLookLikeMonToMe, MSGBOX_DEFAULT
	release
	end

PetalburgCity_House2_EventScript_TradeCompleted::
	msgbox PetalburgCity_House2_Text_AnyPokemonCanBeCute, MSGBOX_DEFAULT
	release
	end

PetalburgCity_House2_Text_NormanBecameGymLeader:
	.string "NORMAN became our town's new\n"
	.string "GYM LEADER.\p"
	.string "I think he called his family over from\n"
	.string "somewhere far away.$"

PetalburgCity_House2_Text_BattledNormanOnce:
	.string "I battled NORMAN once, but, whew,\n"
	.string "he was way too strong.\p"
	.string "How would I put it?\p"
	.string "I just got the feeling that he\n"
	.string "lives for POKéMON.$"

PetalburgCity_House2_Text_IllTradeIfYouWant:
	.string "I battled NORMAN once, but, whew,\n"
	.string "he was way too strong.\p"
	.string "If I can get into a rhythm with\n"
	.string "the right FIGHTING-TYPE POKEMON,\p"
	.string "I know I can win.\p"
	.string "if you want, I'll trade you my {STR_VAR_2}\n"
	.string "for the right FIGHTING-TYPE.$"

PetalburgCity_House2_Text_PleaseBeGoodToMyPokemon:
	.string "Goodbye, THRILLER!\p"
	.string "Please be good to my spooky friend.$"

PetalburgCity_House2_Text_DoesntLookLikeMonToMe:
	.string "Hmm... I don't think this POKEMON can\n"
	.string "match my rhythm.$"

PetalburgCity_House2_Text_YouDontWantToThatsOkay:
	.string "Oh, if you don't want to, that's okay.\p"
	.string "But my POKéMON is useless against NORMAN,\n" 
	.string "you know…$"

PetalburgCity_House2_Text_AnyPokemonCanBeCute:
	.string "NORMAN's gonna be surprised when\n"
	.string "we uses BELLY DRUM next time!$"
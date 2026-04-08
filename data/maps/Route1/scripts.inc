Route1_MapScripts::
	.byte 0

Route1_EventScript_MartClerk::
	lock
	faceplayer
	goto_if_set FLAG_GOT_POKEDOLL_ON_ROUTE_1, Route1_EventScript_AlreadyGotPotion
	msgbox Route1_Text_WorkAtPokeMartTakeSample
	checkitemspace ITEM_POTION
	goto_if_eq VAR_RESULT, FALSE, EventScript_BagIsFull
	bufferitemname STR_VAR_2, ITEM_POKE_DOLL
	playfanfare MUS_OBTAIN_ITEM
	message Text_ObtainedTheX
	waitmessage
	waitfanfare
	additem ITEM_POKE_DOLL
	msgbox Route1_Text_PutPokedollAway
	call EventScript_RestorePrevTextColor
	setflag FLAG_GOT_POKEDOLL_ON_ROUTE_1
	release
	end

Route1_EventScript_AlreadyGotPotion::
	msgbox Route1_Text_ComeSeeUsIfYouNeedPokeBalls
	release
	end

Route1_EventScript_Boy::
	msgbox Route1_Text_CanJumpFromLedges, MSGBOX_NPC
	end

Route1_EventScript_RouteSign::
	msgbox Route1_Text_RouteSign, MSGBOX_SIGN
	end

Route1_Text_WorkAtPokeMartTakeSample::
    .string "Hi!\n"
    .string "I work at a POKéMON MART.\p"
    .string "It's part of a convenient chain\n"
    .string "selling all sorts of items.\p"
    .string "Please, visit us in VIRIDIAN CITY.\p"
    .string "I know, I'll give you a POKéDOLL\n"
    .string "as a promotional item!$"

Route1_Text_ComeSeeUsIfYouNeedPokeBalls::
    .string "Please come see us if you need\n"
    .string "POKé BALLS for catching POKéMON.$"

Route1_Text_PutPokedollAway::
    .string "{PLAYER} put the POKéDOLL away in\n"
    .string "the BAG's ITEMS POCKET.$"

Route1_Text_CanJumpFromLedges::
    .string "See those ledges along the road?\p"
    .string "It's a bit scary, but you can jump\n"
    .string "from them.\p"
    .string "You can get back to PALLET TOWN\n"
    .string "quicker that way.$"

Route1_Text_RouteSign::
    .string "ROUTE 1\n"
    .string "PALLET TOWN - VIRIDIAN CITY$"


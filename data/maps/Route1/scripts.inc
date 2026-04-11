Route1_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route1_OnTransition
	.byte 0

Route1_OnTransition:
	call_if_eq VAR_BIRCH_STATE, 1, Route1_EventScript_MoveMartEmployee
	call_if_eq VAR_BIRCH_STATE, 5, Route1_EventScript_MoveMartEmployee
	end

Route1_EventScript_MoveMartEmployee::
	call_if_unset FLAG_DAILY_RECEIVED_ROUTE1_POKEDOLL, Route1_EventScript_MoveMartEmployeeEnd
	return
Route1_EventScript_MoveMartEmployeeEnd::
	setobjectxyperm LOCALID_MART_EMPLOYEE_ROUTE1, 6, 28
	setobjectmovementtype LOCALID_MART_EMPLOYEE_ROUTE1, MOVEMENT_TYPE_FACE_DOWN_AND_RIGHT
	return

.equ MARTEMPLOYEE_EMOTE_ROUTE1,			FLAG_TEMP_3

Route1_EventScript_MartClerk::
	lock
	faceplayer
	goto_if_set FLAG_DAILY_RECEIVED_ROUTE1_POKEDOLL, Route1_EventScript_MartClerkEnd
	goto_if_set FLAG_GOT_POKEDOLL_ON_ROUTE_1, Route1_EventScript_MartEmployee_GiveAwayDay
	goto_if_unset FLAG_TEMP_1, Route1_EventScript_MartClerk_FreeSample
	goto Route1_EventScript_MartClerkEnd
	release
	end
Route1_EventScript_MartClerk_FreeSample::
	setflag FLAG_TEMP_1
	msgbox Route1_Text_WorkAtPokeMartTakeSample, MSGBOX_DEFAULT
	checkitemspace ITEM_POKE_DOLL
	goto_if_eq VAR_RESULT, FALSE, Route1_EventScript_BagIsFull
	bufferitemname STR_VAR_2, ITEM_POKE_DOLL
	playfanfare MUS_OBTAIN_ITEM
	message Text_ObtainedTheX
	waitmessage
	waitfanfare
	additem ITEM_POKE_DOLL
	msgbox Route1_Text_PutPokedollAway, MSGBOX_DEFAULT
	closemessage
	setflag FLAG_GOT_POKEDOLL_ON_ROUTE_1
	setflag FLAG_DAILY_RECEIVED_ROUTE1_POKEDOLL
	release
	end
Route1_EventScript_MartEmployee_GiveAwayDay::
	goto_if_eq VAR_BIRCH_STATE, 1, Route1_EventScript_MartEmployee_GiveAway
	goto_if_eq VAR_BIRCH_STATE, 5, Route1_EventScript_MartEmployee_GiveAway
	goto Route1_EventScript_MartClerkEnd
	release
	end
Route1_EventScript_MartEmployee_GiveAway::
	msgbox Route1_Text_MartEmployee_GiveAway, MSGBOX_DEFAULT
	giveitem ITEM_POKE_DOLL
	goto_if_eq VAR_RESULT, FALSE, Route1_EventScript_BagIsFull
	setflag FLAG_DAILY_RECEIVED_ROUTE1_POKEDOLL
	goto Route1_EventScript_MartClerkEnd
	release
	end
Route1_EventScript_BagIsFull::
	msgbox gText_TooBadBagIsFull, MSGBOX_DEFAULT
	closemessage
	release
	end
Route1_EventScript_MartClerkEnd::
	msgbox Route1_Text_ComeSeeUsIfYouNeedPokeDolls, MSGBOX_DEFAULT
	closemessage
	release
	end

@ Mart Employee Emote
Route1_EventScript_MartEmployee_Emote::
	goto_if_set MARTEMPLOYEE_EMOTE_ROUTE1, Route1_EventScript_MartEmployee_EmoteSet
	goto_if_set FLAG_DAILY_RECEIVED_ROUTE1_POKEDOLL, Route1_EventScript_MartEmployee_EmoteSet
	goto_if_eq VAR_BIRCH_STATE, 1, Route1_EventScript_MartEmployee_EmoteContinued
	goto_if_eq VAR_BIRCH_STATE, 5, Route1_EventScript_MartEmployee_EmoteContinued
	release
	end
Route1_EventScript_MartEmployee_EmoteContinued::
	setvar VAR_0x8004, LOCALID_MART_EMPLOYEE_ROUTE1
	special NpcFacePlayerEmoteItemBall
	playse SE_PIN
	setflag MARTEMPLOYEE_EMOTE_ROUTE1
	release
	end
Route1_EventScript_MartEmployee_EmoteSet::
	release
	end

Route1_EventScript_Boy::
	msgbox Route1_Text_CanJumpFromLedges, MSGBOX_NPC
	end

Route1_EventScript_RouteSign::
	msgbox Route1_Text_RouteSign, MSGBOX_SIGN
	end

Route1_Text_WorkAtPokeMartTakeSample:
    .string "Hi!\n"
    .string "I work at a POKéMON MART.\p"
    .string "It's part of a convenient chain\n"
    .string "selling all sorts of items.\p"
    .string "Please, visit us in VIRIDIAN CITY.\p"
    .string "I know, I'll give you a POKéDOLL\n"
    .string "as a promotional item!$"
Route1_Text_ComeSeeUsIfYouNeedPokeDolls:
    .string "Please come see us in VIRIDIAN\n"
    .string "CITY if you need more POKéDOLLs!$"
Route1_Text_PutPokedollAway:
    .string "{PLAYER} put the POKéDOLL away in\n"
    .string "the BAG's ITEMS POCKET.$"
Route1_Text_MartEmployee_GiveAway:
	.string "Hey, trainer! There are DANGEROUS\n"
	.string "POKéMON that can appear here in KANTO.\p"
	.string "If you throw out a POKéDOLL, you can\n"
	.string "distract it long enough to get away.\p"
	.string "You should take this promotional\n"
	.string "POKéDOLL item just to be safe!$"

Route1_Text_CanJumpFromLedges:
    .string "See those ledges along the road?\p"
    .string "It's a bit scary, but you can jump\n"
    .string "from them.\p"
    .string "You can get back to PALLET TOWN\n"
    .string "quicker that way.$"

Route1_Text_RouteSign:
    .string "ROUTE 1\n"
    .string "PALLET TOWN - VIRIDIAN CITY$"


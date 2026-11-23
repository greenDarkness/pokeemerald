.set LOCALID_ROUTE14_SUICUNE, 16
.set LOCALID_ROUTE14_EUSINE, 17

Route14_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route14_OnTransition
	.byte 0

Route14_OnTransition::
	setflag FLAG_HIDE_ROUTE14_EUSINE
	setflag FLAG_HIDE_ROUTE14_SUICUNE
	call_if_eq VAR_SUICUNE_ENCOUNTERS, 4, Route14_EventScript_ShowSuicune
	end

Route14_EventScript_ShowSuicune::
	clearflag FLAG_HIDE_ROUTE14_SUICUNE
	return

@ SUICUNE stands on the water east of the south end of the route. Both ways down the route cross
@ the trigger row; the player is walked to the shore next to it (20, 46).
Route14_EventScript_SuicuneTrigger::
	lockall
	playse SE_PIN
	applymovement LOCALID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 40
Route14_EventScript_SuicuneWalkToShore::
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_lt VAR_0x8005, 46, Route14_EventScript_SuicuneWalkDown
	goto_if_lt VAR_0x8004, 20, Route14_EventScript_SuicuneWalkRight
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	playmoncry SPECIES_SUICUNE, CRY_MODE_NORMAL
	applymovement LOCALID_ROUTE14_SUICUNE, Route14_Movement_SuicuneLeave
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	playse SE_LEDGE
	waitse
	turnobject LOCALID_PLAYER, DIR_NORTH
	playse SE_LEDGE
	delay 30
	playse SE_LEDGE
	delay 30
	playse SE_LEDGE
	delay 30
	playse SE_LEDGE
	delay 30
	playse SE_LEDGE
	delay 30
	waitmovement 0
	removeobject LOCALID_ROUTE14_SUICUNE
	clearflag FLAG_HIDE_ROUTE14_EUSINE
	addobject LOCALID_ROUTE14_EUSINE
	applymovement LOCALID_ROUTE14_EUSINE, Route14_Movement_EusineEnter
	delay 60
	turnobject LOCALID_PLAYER, DIR_WEST
	waitmovement 0
	msgbox Route14_Text_Eusine, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_ROUTE14_EUSINE, Route14_Movement_EusineLeave
	waitmovement 0
	removeobject LOCALID_ROUTE14_EUSINE
	setflag FLAG_HIDE_ROUTE14_EUSINE
	setflag FLAG_HIDE_ROUTE14_SUICUNE
	setvar VAR_SUICUNE_ENCOUNTERS, 5
	call_if_set FLAG_DEFEATED_MISTY, Route14_EventScript_SuicuneGoesToRoute25
	releaseall
	end

@ SUICUNE appears on ROUTE 25 once MISTY has been beaten.
Route14_EventScript_SuicuneGoesToRoute25::
	setvar VAR_SUICUNE_ENCOUNTERS, 6
	return

Route14_EventScript_SuicuneWalkDown::
	applymovement LOCALID_PLAYER, Route14_Movement_SuicuneWalkDown
	waitmovement 0
	goto Route14_EventScript_SuicuneWalkToShore

Route14_EventScript_SuicuneWalkRight::
	applymovement LOCALID_PLAYER, Route14_Movement_SuicuneWalkRight
	waitmovement 0
	goto Route14_EventScript_SuicuneWalkToShore

Route14_Movement_SuicuneWalkDown:
	walk_down
	step_end

Route14_Movement_SuicuneWalkRight:
	walk_right
	step_end

Route14_Movement_SuicuneLeave:
	jump_in_place_left
	jump_in_place_left
	jump_in_place_left
	jump_2_up
	jump_2_up
	jump_2_up
	jump_2_up
	jump_2_up
	jump_2_up
	step_end

Route14_Movement_EusineEnter:
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_up
	walk_up
	walk_slow_up
	delay_16
	delay_16
	delay_16
	delay_16
	delay_16
	walk_slow_down
	walk_slow_down
	face_right
	step_end

Route14_Movement_EusineLeave:
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	step_end

Route14_Text_Eusine:
	.string "EUSINE: {PLAYER}! Not again!\n"
	.string "I'll be there first next time!\p"
	.string "Having followed it here, I'm\n"
	.string "starting to understand what SUICUNE\l"
	.string "is after.\p"
	.string "To be honest, I would like to keep\n"
	.string "this information to myself.\p"
	.string "But I want to be an honest trainer\n"
	.string "in front of SUICUNE!\p"
	.string "That's why I am sharing a clue with\n"
	.string "you.\p"
	.string "It seems that...\n"
	.string "SUICUNE prefers a hilly place...\l"
	.string "Near water...\l"
	.string "Somewhere north...\p"
	.string "I don't know exactly where yet.\n"
	.string "It will just be you and me!\p"
	.string "Who'll find it first?\n"
	.string "I challenge you!$"

Route14_EventScript_RouteSign::
	msgbox Route14_Text_RouteSign, MSGBOX_SIGN
	end

Route14_Text_CarterIntro::
    .string "You need to use TMs to teach good\n"
    .string "moves to POKéMON.$"

Route14_Text_CarterDefeat::
    .string "Not good enough quite yet.$"

Route14_Text_CarterPostBattle::
    .string "You have some HMs, right? POKéMON\n"
    .string "can't forget those moves easily.$"

Route14_Text_MitchIntro::
    .string "My bird POKéMON should be ready\n"
    .string "for battle.$"

Route14_Text_MitchDefeat::
    .string "Not ready yet!$"

Route14_Text_MitchPostBattle::
    .string "My bird POKéMON need to learn\n"
    .string "better moves.$"

Route14_Text_BeckIntro::
    .string "They have TMs on sale at the\n"
    .string "CELADON DEPT. STORE.\p"
    .string "TMs aren't really rare, but not\n"
    .string "many people have HMs.$"

Route14_Text_BeckDefeat::
    .string "Aww, bummer!$"

Route14_Text_BeckPostBattle::
    .string "Try teaching POKéMON a move that's\n"
    .string "the same type as it.\p"
    .string "That apparently boosts the power\n"
    .string "of the move.$"

Route14_Text_MarlonIntro::
    .string "Have you taught your bird POKéMON\n"
    .string "how to FLY?\p"
    .string "You'll be able to soar with it into\n"
    .string "the sky!$"

Route14_Text_MarlonDefeat::
    .string "Shot down in flames!$"

Route14_Text_MarlonPostBattle::
    .string "Bird POKéMON are my one true love.\n"
    .string "I don't want to raise anything else.$"

Route14_Text_DonaldIntro::
    .string "Have you heard the legend of the\n"
    .string "winged mirages?$"

Route14_Text_DonaldDefeat::
    .string "Why?\n"
    .string "Why'd I lose?$"

Route14_Text_DonaldPostBattle::
    .string "Well, the winged mirages are the\n"
    .string "legendary bird POKéMON.\p"
    .string "There are three of them: ARTICUNO,\n"
    .string "ZAPDOS, and MOLTRES.$"

Route14_Text_BennyIntro::
    .string "I'm not into it, but okay.\n"
    .string "Let's go!$"

Route14_Text_BennyDefeat::
    .string "I knew it!$"

Route14_Text_BennyPostBattle::
    .string "Winning, losing… It's insignificant\n"
    .string "under this huge sky.$"

Route14_Text_LukasIntro::
    .string "C'mon, c'mon.\n"
    .string "Let's go, let's go, let's go!$"

Route14_Text_LukasDefeat::
    .string "Arrg!\n"
    .string "Lost! Get lost!$"

Route14_Text_LukasPostBattle::
    .string "What, what, what?\n"
    .string "What do you want still?$"

Route14_Text_IsaacIntro::
    .string "I need to burn some time.\n"
    .string "Shut up and battle.$"

Route14_Text_IsaacDefeat::
    .string "What?\n"
    .string "You!?$"

Route14_Text_IsaacPostBattle::
    .string "Raising POKéMON is a drag, man.$"

Route14_Text_GeraldIntro::
    .string "We ride out here because of the\n"
    .string "wide-open spaces.$"

Route14_Text_GeraldDefeat::
    .string "Wipeout!$"

Route14_Text_GeraldPostBattle::
    .string "It's cool you made your POKéMON so\n"
    .string "strong.\p"
    .string "Might is right!\n"
    .string "And you know it!$"

Route14_Text_MalikIntro::
    .string "POKéMON battle?\n"
    .string "Cool! Rumble!$"

Route14_Text_MalikDefeat::
    .string "Blown away!$"

Route14_Text_MalikPostBattle::
    .string "You know who'd win, you and me\n"
    .string "one-on-one!$"

Route14_Text_RouteSign::
    .string "ROUTE 14\n"
    .string "West to FUCHSIA CITY$"

Route14_Text_KiriIntro::
    .string "KIRI: JAN, let's try really,\n"
    .string "really hard together.$"

Route14_Text_KiriDefeat::
    .string "KIRI: Whimper…\n"
    .string "We lost, didn't we?$"

Route14_Text_KiriPostBattle::
    .string "KIRI: Did we lose because of me?$"

Route14_Text_KiriNotEnoughMons::
    .string "KIRI: We can battle if you have\n"
    .string "two POKéMON.$"

Route14_Text_JanIntro::
    .string "JAN: KIRI, here we go!\n"
    .string "We have to try hard!$"

Route14_Text_JanDefeat::
    .string "JAN: Eeeeh!\n"
    .string "No fair!$"

Route14_Text_JanPostBattle::
    .string "JAN: KIRI, don't cry!\n"
    .string "We'll just try harder next time.$"

Route14_Text_JanNotEnoughMons::
    .string "JAN: You want to battle?\n"
    .string "You don't have enough POKéMON.$"

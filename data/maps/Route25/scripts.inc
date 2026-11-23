.set LOCALID_ROUTE25_SUICUNE, 14
.set LOCALID_ROUTE25_EUSINE, 15

Route25_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route25_OnTransition
	.byte 0

@ SUICUNE waits east of the SEA COTTAGE once it has been met in VERMILION CITY and on ROUTE 14 and
@ MISTY has been beaten (VAR_SUICUNE_ENCOUNTERS 6, see include/constants/vars.h). After the battle
@ it is here again each time the player comes to the route, until it is caught.
Route25_OnTransition::
	setflag FLAG_HIDE_ROUTE25_SUICUNE
	setflag FLAG_HIDE_ROUTE25_EUSINE
	call_if_eq VAR_SUICUNE_ENCOUNTERS, 6, Route25_EventScript_ShowSuicune
	call_if_eq VAR_SUICUNE_ENCOUNTERS, 7, Route25_EventScript_ShowSuicuneAndEusine
	call_if_eq VAR_SUICUNE_ENCOUNTERS, 8, Route25_EventScript_ShowSuicune
	end

Route25_EventScript_ShowSuicune::
	clearflag FLAG_HIDE_ROUTE25_SUICUNE
	return

Route25_EventScript_ShowSuicuneAndEusine::
	clearflag FLAG_HIDE_ROUTE25_SUICUNE
	clearflag FLAG_HIDE_ROUTE25_EUSINE
	setobjectxyperm LOCALID_ROUTE25_EUSINE, 57, 6
	return

@ The player is walked to (57, 7), two steps from SUICUNE, and EUSINE catches up.
Route25_EventScript_SuicuneTriggerWest::
	lockall
	call Route25_EventScript_SuicuneNoticed
Route25_EventScript_SuicuneWalkToSuicune::
	getplayerxy VAR_0x8004, VAR_0x8005
	goto_if_lt VAR_0x8005, 7, Route25_EventScript_SuicuneWalkDown
	goto_if_gt VAR_0x8005, 7, Route25_EventScript_SuicuneWalkUp
	goto_if_lt VAR_0x8004, 57, Route25_EventScript_SuicuneWalkRight
	goto Route25_EventScript_EusineCatchesUp

@ Coming up the east side of the pond
Route25_EventScript_SuicuneTriggerEast::
	lockall
	call Route25_EventScript_SuicuneNoticed
	applymovement LOCALID_PLAYER, Route25_Movement_WalkAroundSuicune
	waitmovement 0
	goto Route25_EventScript_EusineCatchesUp

Route25_EventScript_SuicuneNoticed::
	playse SE_PIN
	applymovement LOCALID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 40
	playmoncry SPECIES_SUICUNE, CRY_MODE_NORMAL
	applymovement LOCALID_ROUTE25_SUICUNE, Route25_Movement_SuicuneJump
	playse SE_LEDGE
	waitse
	waitmovement 0
	return

Route25_EventScript_SuicuneWalkDown::
	applymovement LOCALID_PLAYER, Route25_Movement_SuicuneWalkDown
	waitmovement 0
	goto Route25_EventScript_SuicuneWalkToSuicune

Route25_EventScript_SuicuneWalkUp::
	applymovement LOCALID_PLAYER, Route25_Movement_SuicuneWalkUp
	waitmovement 0
	goto Route25_EventScript_SuicuneWalkToSuicune

Route25_EventScript_SuicuneWalkRight::
	applymovement LOCALID_PLAYER, Route25_Movement_SuicuneWalkRight
	waitmovement 0
	goto Route25_EventScript_SuicuneWalkToSuicune

Route25_EventScript_EusineCatchesUp::
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	clearflag FLAG_HIDE_ROUTE25_EUSINE
	addobject LOCALID_ROUTE25_EUSINE
	applymovement LOCALID_ROUTE25_EUSINE, Route25_Movement_EusineEnter
	waitmovement 0
	turnobject LOCALID_PLAYER, DIR_WEST
	msgbox Route25_Text_EusineNoMatchForYou, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_ROUTE25_EUSINE, Route25_Movement_EusineStepAside
	waitmovement 0
	turnobject LOCALID_PLAYER, DIR_EAST
	msgbox Route25_Text_EusineSuicuneIsWaiting, MSGBOX_DEFAULT
	closemessage
	setvar VAR_SUICUNE_ENCOUNTERS, 7
	releaseall
	end

Route25_EventScript_Eusine::
	msgbox Route25_Text_EusineSuicuneIsWaiting, MSGBOX_NPC
	end

Route25_EventScript_Suicune::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_SUICUNE, CRY_MODE_ENCOUNTER
	delay 40
	waitmoncry
	seteventmon SPECIES_SUICUNE, 40
	special BattleSetup_StartLegendaryBattle
	specialvar VAR_RESULT, GetBattleOutcome
	goto_if_eq VAR_RESULT, B_OUTCOME_RAN, Route25_EventScript_SuicuneStays
	goto_if_eq VAR_RESULT, B_OUTCOME_PLAYER_TELEPORTED, Route25_EventScript_SuicuneStays
	copyvar VAR_0x8004, VAR_RESULT
	fadescreenswapbuffers FADE_TO_BLACK
	removeobject LOCALID_ROUTE25_SUICUNE
	setflag FLAG_HIDE_ROUTE25_SUICUNE
	fadescreenswapbuffers FADE_FROM_BLACK
	@ EUSINE is only here the first time
	goto_if_ne VAR_SUICUNE_ENCOUNTERS, 7, Route25_EventScript_SuicuneBattleDone
	applymovement LOCALID_ROUTE25_EUSINE, Common_Movement_FacePlayer
	waitmovement 0
	turnobject LOCALID_PLAYER, DIR_WEST
	msgbox Route25_Text_EusineFarewell, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_ROUTE25_EUSINE, Route25_Movement_EusineLeave
	waitmovement 0
	removeobject LOCALID_ROUTE25_EUSINE
	setflag FLAG_HIDE_ROUTE25_EUSINE
Route25_EventScript_SuicuneBattleDone::
	setvar VAR_SUICUNE_ENCOUNTERS, 8
	call_if_eq VAR_0x8004, B_OUTCOME_CAUGHT, Route25_EventScript_CaughtSuicune
	release
	end

Route25_EventScript_CaughtSuicune::
	setvar VAR_SUICUNE_ENCOUNTERS, 9
	return

Route25_EventScript_SuicuneStays::
	release
	end

Route25_Movement_SuicuneWalkDown:
	walk_down
	step_end

Route25_Movement_SuicuneWalkUp:
	walk_up
	step_end

Route25_Movement_SuicuneWalkRight:
	walk_right
	step_end

Route25_Movement_WalkAroundSuicune:
	walk_up
	walk_left
	walk_left
	walk_left
	walk_up
	step_end

Route25_Movement_SuicuneJump:
	jump_in_place_left
	step_end

Route25_Movement_EusineEnter:
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_fast_right
	walk_right
	walk_slow_right
	step_end

Route25_Movement_EusineStepAside:
	walk_slow_up
	walk_slow_right
	step_end

Route25_Movement_EusineLeave:
	walk_left
	walk_left
	walk_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	step_end

Route25_Text_EusineNoMatchForYou:
	.string "EUSINE: Puff, puff...\n"
	.string "I am... no match for you.\p"
	.string "...As I predicted.\n"
	.string "..........................\l"
	.string "... Go ahead.\p"
	.string "Since I met you in the BURNED TOWER\n"
	.string "at ECRUTEAK CITY, I've sort of\l"
	.string "known that SUICUNE would choose\l"
	.string "you.$"

Route25_Text_EusineSuicuneIsWaiting:
	.string "Take a look at it!\p"
	.string "SUICUNE is waiting for you!\n"
	.string "It has been waiting for a battle\l"
	.string "with a worthy trainer to whom it\l"
	.string "can entrust itself.$"

Route25_Text_EusineFarewell:
	.string "{PLAYER}...\n"
	.string "You two are truly amazing...\p"
	.string "I have never seen a battle this\n"
	.string "intense!\l"
	.string "I have no regrets...\p"
	.string "Farewell, SUICUNE!\n"
	.string "Farewell... {PLAYER}!$"

Route25_EventScript_SeaCottageSign::
	msgbox Route25_Text_SeaCottageSign, MSGBOX_SIGN
	end

Route25_EventScript_Man::
	msgbox Route25_Text_AreYouHereAlone, MSGBOX_NPC
	end

Route25_Text_JoeyIntro::
    .string "Local TRAINERS come here to\n"
    .string "practice.$"

Route25_Text_JoeyDefeat::
    .string "You're decent.$"

Route25_Text_JoeyPostBattle::
    .string "All POKéMON have weaknesses.\n"
    .string "Even the strongest ones.\p"
    .string "That's why it's best to raise\n"
    .string "POKéMON of different types.$"

Route25_Text_DanIntro::
    .string "Dad took me to a great party on\n"
    .string "the S.S. ANNE at VERMILION CITY.$"

Route25_Text_DanDefeat::
    .string "I'm not mad!$"

Route25_Text_DanPostBattle::
    .string "On the S.S. ANNE, I saw TRAINERS\n"
    .string "from around the world.$"

Route25_Text_FlintIntro::
    .string "I'm a cool guy.\n"
    .string "I've got a girlfriend!$"

Route25_Text_FlintDefeat::
    .string "Aww, darn…$"

Route25_Text_FlintPostBattle::
    .string "Oh, well.\n"
    .string "My girlfriend will cheer me up.$"

Route25_Text_KelseyIntro::
    .string "Hi!\n"
    .string "My boyfriend is cool!$"

Route25_Text_KelseyDefeat::
    .string "My conditioning isn't the best…$"

Route25_Text_KelseyPostBattle::
    .string "I wish my boyfriend was as good\n"
    .string "as you.$"

Route25_Text_ChadIntro::
    .string "I had this feeling…\n"
    .string "I knew I had to battle you!$"

Route25_Text_ChadDefeat::
    .string "I knew I'd lose, too!$"

Route25_Text_ChadPostBattle::
    .string "If your POKéMON gets confused,\n"
    .string "switch it out.\p"
    .string "That's a good tactic.$"

Route25_Text_HaleyIntro::
    .string "My friend has many cute POKéMON.\n"
    .string "I'm so jealous!$"

Route25_Text_HaleyDefeat::
    .string "I'm not so jealous!$"

Route25_Text_HaleyPostBattle::
    .string "You came from MT. MOON?\n"
    .string "May I have a CLEFAIRY?$"

Route25_Text_FranklinIntro::
    .string "I just got down from MT. MOON,\n"
    .string "but I've still got gas in the tank!$"

Route25_Text_FranklinDefeat::
    .string "You worked hard!$"

Route25_Text_FranklinPostBattle::
    .string "Drat!\n"
    .string "A ZUBAT bit me back in that cave.$"

Route25_Text_NobIntro::
    .string "I'm off to see a POKéMANIAC's\n"
    .string "collection at the cape.$"

Route25_Text_NobDefeat::
    .string "You done got me, and real good,\n"
    .string "too!$"

Route25_Text_NobPostBattle::
    .string "The POKéMANIAC sure lives up to\n"
    .string "his name.\p"
    .string "His collection includes many rare\n"
    .string "species of POKéMON.$"

Route25_Text_WayneIntro::
    .string "You're going to see BILL?\n"
    .string "First, we battle!$"

Route25_Text_WayneDefeat::
    .string "You're something.$"

Route25_Text_WaynePostBattle::
    .string "The trail below is a shortcut to\n"
    .string "CERULEAN CITY.$"

Route25_Text_SeaCottageSign::
    .string "SEA COTTAGE\n"
    .string "BILL lives here!$"

Route25_Text_MistyHighHopesAboutThisPlace::
    .string "This cape is a famous date spot.\p"
    .string "MISTY, the GYM LEADER, has high\n"
    .string "hopes about this place.$"

Route25_Text_AreYouHereAlone::
    .string "Hello, are you here alone?\p"
    .string "If you're out at CERULEAN's cape…\n"
    .string "Well, it should be as a couple.$"


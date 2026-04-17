.equ FLAG_HIDE_SPRITES_MORNING,   				FLAG_TEMP_A
.equ FLAG_HIDE_SPRITES_DAY,   					FLAG_TEMP_B
.equ FLAG_HIDE_SPRITES_NIGHT,   				FLAG_TEMP_C
.equ FLAG_HIDE_SPRITES_NIGHT_AND_MORNING,       FLAG_TEMP_D
.equ FLAG_HIDE_SPRITES_MORNING_AND_DAY,   		FLAG_TEMP_E
.equ FLAG_HIDE_SPRITES_DAY_AND_NIGHT,   	    FLAG_TEMP_F


ViridianCity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, ViridianCity_OnTransition
	.byte 0

ViridianCity_OnTransition::
	setworldmapflag FLAG_WORLD_MAP_VIRIDIAN_CITY
	call ViridianCity_EventScript_ShowPokemon
	call_if_eq VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 0, ViridianCity_EventScript_SetOldManBlockingRoad
	call_if_eq VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 1, ViridianCity_EventScript_SetOldManStandingByRoad
	call_if_ge VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 2, ViridianCity_EventScript_SetOldManNormal
	call_if_eq VAR_MAP_SCENE_VIRIDIAN_CITY_GYM_DOOR, 0, ViridianCity_EventScript_TryUnlockGym
	end

ViridianCity_EventScript_ShowPokemon::
	call ViridianCity_EventScript_HidePokemon
	specialvar VAR_RESULT, GetScriptTimeOfDay
    goto_if_eq VAR_RESULT, SCRIPT_TIME_MORNING, ViridianCity_EventScript_ShowMorningPokemon
	goto_if_eq VAR_RESULT, SCRIPT_TIME_DAY, ViridianCity_EventScript_ShowDayPokemon
	goto_if_eq VAR_RESULT, SCRIPT_TIME_NIGHT, ViridianCity_EventScript_ShowNightPokemon
	return
ViridianCity_EventScript_ShowMorningPokemon::
	clearflag FLAG_HIDE_SPRITES_MORNING
	clearflag FLAG_HIDE_SPRITES_MORNING_AND_DAY
	clearflag FLAG_HIDE_SPRITES_NIGHT_AND_MORNING
	end
ViridianCity_EventScript_ShowDayPokemon::
	clearflag FLAG_HIDE_SPRITES_DAY
	clearflag FLAG_HIDE_SPRITES_MORNING_AND_DAY
	clearflag FLAG_HIDE_SPRITES_DAY_AND_NIGHT
	end
ViridianCity_EventScript_ShowNightPokemon::
	clearflag FLAG_HIDE_SPRITES_NIGHT
	clearflag FLAG_HIDE_SPRITES_DAY_AND_NIGHT
	clearflag FLAG_HIDE_SPRITES_NIGHT_AND_MORNING
	end
ViridianCity_EventScript_HidePokemon::
	setflag FLAG_HIDE_SPRITES_MORNING
	setflag FLAG_HIDE_SPRITES_DAY
	setflag FLAG_HIDE_SPRITES_NIGHT
	setflag FLAG_HIDE_SPRITES_MORNING_AND_DAY
	setflag FLAG_HIDE_SPRITES_DAY_AND_NIGHT
	setflag FLAG_HIDE_SPRITES_NIGHT_AND_MORNING
	return

ViridianCity_EventScript_SetOldManNormal::
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_OLD_MAN_1
	return

ViridianCity_EventScript_SetOldManStandingByRoad::
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_OLD_MAN_1
	setobjectxyperm LOCALID_TUTORIAL_MAN, 21, 8
	setobjectmovementtype LOCALID_TUTORIAL_MAN, MOVEMENT_TYPE_LOOK_AROUND
	return

ViridianCity_EventScript_SetOldManBlockingRoad::
	setvar VAR_OBJ_GFX_ID_0, OBJ_EVENT_GFX_OLD_MAN_LYING_DOWN
	setobjectxyperm LOCALID_TUTORIAL_MAN, 21, 11
	setobjectmovementtype LOCALID_TUTORIAL_MAN, MOVEMENT_TYPE_FACE_DOWN
	return

ViridianCity_EventScript_TryUnlockGym::
	goto_if_unset FLAG_BADGE02_GET, EventScript_Return
	goto_if_unset FLAG_BADGE03_GET, EventScript_Return
	goto_if_unset FLAG_BADGE04_GET, EventScript_Return
	goto_if_unset FLAG_BADGE05_GET, EventScript_Return
	goto_if_unset FLAG_BADGE06_GET, EventScript_Return
	goto_if_unset FLAG_BADGE07_GET, EventScript_Return
	setvar VAR_MAP_SCENE_VIRIDIAN_CITY_GYM_DOOR, 1
	return

ViridianCity_EventScript_GymDoorLocked::
	lockall
	textcolor NPC_TEXT_COLOR_NEUTRAL
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	delay 20
	msgbox ViridianCity_Text_GymDoorsAreLocked
	closemessage
	applymovement LOCALID_PLAYER, ViridianCity_Movement_JumpDownLedge
	waitmovement 0
	releaseall
	end

ViridianCity_Movement_JumpDownLedge::
	jump_2_down
	step_end

ViridianCity_EventScript_CitySign::
	msgbox ViridianCity_Text_CitySign, MSGBOX_SIGN
	end

ViridianCity_EventScript_TrainerTips1::
	msgbox ViridianCity_Text_CatchMonsForEasierBattles, MSGBOX_SIGN
	end

ViridianCity_EventScript_TrainerTips2::
	msgbox ViridianCity_Text_MovesLimitedByPP, MSGBOX_SIGN
	end

ViridianCity_EventScript_GymSign::
	msgbox ViridianCity_Text_GymSign, MSGBOX_SIGN
	end

ViridianCity_EventScript_GymDoor::
	msgbox ViridianCity_Text_GymDoorsAreLocked, MSGBOX_SIGN
	end

ViridianCity_EventScript_Boy::
	msgbox ViridianCity_Text_CanCarryMonsAnywhere, MSGBOX_NPC
	end

@ Other old man, not the tutorial old man
ViridianCity_EventScript_OldMan::
	lock
	faceplayer
	goto_if_eq VAR_MAP_SCENE_VIRIDIAN_CITY_GYM_DOOR, 1, ViridianCity_EventScript_OldManGymLeaderReturned
	msgbox ViridianCity_Text_GymClosedWonderWhoLeaderIs
	closemessage
	applymovement LOCALID_VIRIDIAN_OLD_MAN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

ViridianCity_EventScript_OldManGymLeaderReturned::
	msgbox ViridianCity_Text_ViridiansGymLeaderReturned
	release
	end

ViridianCity_EventScript_TutorialOldMan::
	lock
	faceplayer
	goto_if_set FLAG_BADGE01_GET, ViridianCity_EventScript_AskIfTeachyTVHelpful
	goto_if_ge VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 2, ViridianCity_EventScript_TutorialCompleted
	goto_if_eq VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 1, ViridianCity_EventScript_TutorialStart
	goto_if_eq VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 0, ViridianCity_EventScript_TutorialNotReady
	end

ViridianCity_EventScript_AskIfTeachyTVHelpful::
	msgbox ViridianCity_Text_HowsTeachyTVHelping, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ViridianCity_EventScript_TeachyTVNotHelpful
	msgbox ViridianCity_Text_MyGrandsonOnTheShow
	release
	end

ViridianCity_EventScript_TeachyTVNotHelpful::
	msgbox ViridianCity_Text_TooBusyForTeachyTV
	release
	end

ViridianCity_EventScript_TutorialCompleted::
	msgbox ViridianCity_Text_WeakenMonsFirstToCatch
	release
	end

ViridianCity_EventScript_TutorialStart::
	call ViridianCity_EventScript_DoTutorialBattle
	release
	end

@ Unused
ViridianCity_EventScript_WatchToLearnBasics::
	msgbox ViridianCity_Text_WatchThatToLearnBasics
	release
	end

@ Unused. Starts battle after the post battle text, which is odd.
ViridianCity_EventScript_TutorialUnused::
	msgbox ViridianCity_Text_ThatWasEducationalTakeThis
	goto_if_questlog EventScript_ReleaseEnd
	special QuestLog_CutRecording
	special StartOldManTutorialBattle_Kanto
	waitstate
	release
	end

ViridianCity_EventScript_TutorialNotReady::
	msgbox ViridianCity_Text_ThisIsPrivateProperty
	closemessage
	release
	end

ViridianCity_EventScript_Youngster::
	lock
	faceplayer
	msgbox ViridianCity_Text_WantToKnowAboutCaterpillarMons, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, ViridianCity_EventScript_YoungsterExplainCaterpillars
	goto_if_eq VAR_RESULT, NO, ViridianCity_EventScript_YoungsterDeclineExplanation
	end

ViridianCity_EventScript_YoungsterExplainCaterpillars::
	msgbox ViridianCity_Text_ExplainCaterpieWeedle
	release
	end

ViridianCity_EventScript_YoungsterDeclineExplanation::
	msgbox ViridianCity_Text_OhOkayThen
	release
	end

ViridianCity_EventScript_Woman::
	lock
	faceplayer
	goto_if_eq VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 0, ViridianCity_EventScript_WomanRoadBlocked
	msgbox ViridianCity_Text_GoShoppingInPewterOccasionally
	release
	end

ViridianCity_EventScript_WomanRoadBlocked::
	msgbox ViridianCity_Text_GrandpaHasntHadCoffeeYet
	closemessage
	applymovement LOCALID_VIRIDIAN_WOMAN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

ViridianCity_EventScript_DreamEaterTutor::
	goto EventScript_DreamEaterTutor
	end

ViridianCity_EventScript_RoadBlocked::
	lockall
	textcolor NPC_TEXT_COLOR_MALE
	msgbox ViridianCity_Text_ThisIsPrivateProperty
	closemessage
	applymovement LOCALID_PLAYER, ViridianCity_Movement_WalkDown
	waitmovement 0
	releaseall
	end

ViridianCity_Movement_WalkDown::
	walk_down
	step_end

ViridianCity_EventScript_TutorialTriggerLeft::
	lockall
	textcolor NPC_TEXT_COLOR_MALE
	applymovement LOCALID_TUTORIAL_MAN, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	call ViridianCity_EventScript_DoTutorialBattle
	release
	end

ViridianCity_EventScript_TutorialTriggerRight::
	lockall
	textcolor NPC_TEXT_COLOR_MALE
	applymovement LOCALID_TUTORIAL_MAN, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	call ViridianCity_EventScript_DoTutorialBattle
	release
	end

ViridianCity_EventScript_DoTutorialBattle::
	msgbox ViridianCity_Text_ShowYouHowToCatchMons
	closemessage
	goto_if_questlog EventScript_ReleaseEnd
	special QuestLog_CutRecording
	special StartOldManTutorialBattle_Kanto
	waitstate
	lock
	faceplayer
	msgbox ViridianCity_Text_ThatWasEducationalTakeThis
	setvar VAR_MAP_SCENE_VIRIDIAN_CITY_OLD_MAN, 2
	giveitem ITEM_TEACHY_TV
	msgbox ViridianCity_Text_WatchThatToLearnBasics
	return

ViridianCity_Text_CanCarryMonsAnywhere::
    .string "Those POKé BALLS at your waist!\n"
    .string "You have POKéMON, don't you?\p"
    .string "It's great that you can carry and\n"
    .string "use POKéMON anytime, anywhere.$"

ViridianCity_Text_GymClosedWonderWhoLeaderIs::
    .string "This POKéMON GYM is always closed.\p"
    .string "I wonder who the LEADER is?$"

ViridianCity_Text_ViridiansGymLeaderReturned::
    .string "VIRIDIAN GYM's LEADER returned!$"

ViridianCity_Text_WantToKnowAboutCaterpillarMons::
    .string "You want to know about the two\n"
    .string "kinds of caterpillar POKéMON?$"

ViridianCity_Text_OhOkayThen::
    .string "Oh, okay then!$"

ViridianCity_Text_ExplainCaterpieWeedle::
    .string "CATERPIE has no poison,\n"
    .string "but WEEDLE does.\p"
    .string "Watch that your POKéMON aren't\n"
    .string "stabbed by WEEDLE's POISON STING.$"

ViridianCity_Text_GrandpaHasntHadCoffeeYet::
    .string "Good grief, Grandpa!\n"
    .string "Sleeping here of all places!\p"
    .string "It can't be helped…\n"
    .string "We have to wait until he sobers up…$"

ViridianCity_Text_GoShoppingInPewterOccasionally::
    .string "I go shopping in PEWTER CITY\n"
    .string "occasionally.\p"
    .string "I have to take the winding trail in\n"
    .string "VIRIDIAN FOREST when I go.$"

ViridianCity_Text_ThisIsPrivateProperty::
    .string "Uugh…! Hic…hold it right there!\n"
    .string "Lissen to what I hafta say!\p"
    .string "…hey!\n"
    .string "Don' go, I said!$"

ViridianCity_Text_ShowYouHowToCatchMons::
    .string "Mmn…\n"
    .string "Looks like I was pretty drunk!\p"
    .string "My head hurts…\n"
    .string "Are you in a hurry…?\p"
    .string "Hm?\n"
    .string "What is that red box you have?\p"
    .string "Ah, so you're working on your\n"
    .string "POKéDEX.\p"
    .string "Then let me give you a word of\n"
    .string "advice.\p"
    .string "Whenever you catch a POKéMON,\n"
    .string "the POKéDEX automatically updates\l"
    .string "its data.\p"
    .string "…You don't know how to catch\n"
    .string "a POKéMON?\p"
    .string "I suppose I had better show you\n"
    .string "then!$"

ViridianCity_Text_ThatWasEducationalTakeThis::
    .string "There! Now tell me, that was\n"
    .string "educational, was it not?\p"
    .string "And here, take this, too.$"

ViridianCity_Text_WatchThatToLearnBasics::
    .string "If there's something you don't\n"
    .string "understand, watch that.\p"
    .string "It will teach you about the basics\n"
    .string "of being a POKéMON TRAINER.$"

ViridianCity_Text_WeakenMonsFirstToCatch::
    .string "Well, now, I've had my coffee, and\n"
    .string "that's what I need to get going!\p"
    .string "But I made it too strong.\n"
    .string "It gave me a headache…\p"
    .string "Incidentally, are you filling your\n"
    .string "POKéDEX?\p"
    .string "At first, focus on weakening the\n"
    .string "POKéMON before trying to catch it.$"

ViridianCity_Text_HowsTeachyTVHelping::
    .string "Well, now, I've had my coffee, and\n"
    .string "that's what I need to get going!\p"
    .string "But I made it too strong.\n"
    .string "It gave me a headache…\p"
    .string "Incidentally, is my old TEACHY TV\n"
    .string "helping you?$"

ViridianCity_Text_MyGrandsonOnTheShow::
    .string "Wahaha!\n"
    .string "It's my grandson on the show!\p"
    .string "Since he's the one doing the\n"
    .string "teaching, you're bound to learn.$"

ViridianCity_Text_TooBusyForTeachyTV::
    .string "Hm… You're too busy to not even\n"
    .string "have the time for TEACHY TV…\p"
    .string "Time is money, and neither should\n"
    .string "be ill spent…$"

ViridianCity_Text_CitySign::
    .string "VIRIDIAN CITY \n"
    .string "The Eternally Green Paradise$"

ViridianCity_Text_CatchMonsForEasierBattles::
    .string "TRAINER TIPS\p"
    .string "Catch POKéMON and expand your\n"
    .string "collection.\p"
    .string "The more you have, the easier it\n"
    .string "is to battle.$"

ViridianCity_Text_MovesLimitedByPP::
    .string "TRAINER TIPS\p"
    .string "The battle moves of POKéMON are\n"
    .string "limited by their POWER POINTS, PP.\p"
    .string "To replenish PP, rest your tired\n"
    .string "POKéMON at a POKéMON CENTER.$"

ViridianCity_Text_GymSign::
    .string "VIRIDIAN CITY POKéMON GYM$"

ViridianCity_Text_GymDoorsAreLocked::
    .string "VIRIDIAN GYM's doors are locked…$"


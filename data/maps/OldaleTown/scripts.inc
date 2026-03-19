OldaleTown_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, OldaleTown_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, OldaleTown_OnLoad
	.byte 0

OldaleTown_OnTransition:
	call Common_EventScript_SetupRivalGfxId
	call Mom_EventScript_UpdateLocation
	setflag FLAG_VISITED_OLDALE_TOWN
	call_if_unset FLAG_ADVENTURE_STARTED, OldaleTown_EventScript_BlockWestEntrance
	call_if_unset FLAG_RECEIVED_POTION_OLDALE, OldaleTown_EventScript_MoveMartEmployee
	call_if_set FLAG_ADVENTURE_STARTED, OldaleTown_EventScript_SetOldaleState
	call_if_unset FLAG_HIDE_LITTLEROOT_TOWN_MOM_OUTSIDE, OldaleTown_EventScript_HideOldaleMom
	call_if_unset FLAG_DAILY_MOM_ITEM_GIVEN, OldaleTown_EventScript_Mom_LookAround
	end

OldaleTown_EventScript_Mom_LookAround::
	setobjectmovementtype LOCALID_MOM_OLDALE, MOVEMENT_TYPE_LOOK_AROUND
	return

OldaleTown_EventScript_HideOldaleMom::
	setflag FLAG_HIDE_MOM_OLDALE
	return

OldaleTown_OnLoad:
	call_if_unset FLAG_OLDALESHED_OPENED, OldaleTown_EventScript_LockShed
	call_if_set FLAG_OLDALESHED_OPENED, OldaleTown_EventScript_UnlockShed
	end

@ This script seems pointless because nothing uses VAR_OLDALE_TOWN_STATE
OldaleTown_EventScript_SetOldaleState::
	setvar VAR_OLDALE_TOWN_STATE, 1
	return

.equ HAS_SHARP_BEAK,   					FLAG_TEMP_2
.equ HAS_QUICK_CLAWS,		 			FLAG_TEMP_3
.equ HAS_BEAK_AND_CLAWS, 	 			FLAG_TEMP_4
.equ OLDALE_TOWN_WINDTUTOR_FIRST_TALK,  FLAG_TEMP_5
.equ WINDTUTOR_EMOTE,  					FLAG_TEMP_6
.equ MOM_EMOTE_OLDALE,  				FLAG_TEMP_7


OldaleTown_EventScript_ShedDoor::
	lockall
	goto_if_set FLAG_OLDALESHED_OPENED, OldaleTown_EventScript_ShedDoorIsUnlocked
	checkitem ITEM_RUSTY_KEY
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_EventScript_ShedDoorIsLocked
	msgbox OldaleTown_Text_InsertedRustyKey, MSGBOX_DEFAULT
	playse SE_PIN
	removeitem ITEM_RUSTY_KEY
	setflag FLAG_OLDALESHED_OPENED
	call OldaleTown_EventScript_UnlockShed
	special DrawWholeMapView
	releaseall
	end

OldaleTown_EventScript_LockShed::
	setmetatile 32, 10, METATILE_Petalburg_Door_Oldale_Locked, TRUE
	return

OldaleTown_EventScript_UnlockShed::
	setmetatile 32, 10, METATILE_Petalburg_Door_Oldale, TRUE
	return

OldaleTown_EventScript_ShedDoorIsLocked::
	msgbox OldaleTown_Text_ShedDoorIsLocked, MSGBOX_DEFAULT
	releaseall
	end

OldaleTown_EventScript_ShedDoorIsUnlocked::
	msgbox OldaleTown_Text_ShedDoorIsOpen, MSGBOX_DEFAULT
	releaseall
	end

OldaleTown_EventScript_BlockWestEntrance::
	setobjectxyperm LOCALID_FOOTPRINTS_MAN, 1, 11
	setobjectmovementtype LOCALID_FOOTPRINTS_MAN, MOVEMENT_TYPE_FACE_LEFT
	return

OldaleTown_EventScript_MoveMartEmployee::
	setobjectxyperm LOCALID_OLDALE_MART_EMPLOYEE, 13, 14
	setobjectmovementtype LOCALID_OLDALE_MART_EMPLOYEE, MOVEMENT_TYPE_FACE_DOWN
	return

OldaleTown_EventScript_TownSign::
	msgbox OldaleTown_Text_TownSign, MSGBOX_SIGN
	end

OldaleTown_EventScript_Girl::
	msgbox OldaleTown_Text_SavingMyProgress, MSGBOX_NPC
	end

OldaleTown_EventScript_Grunt::
	lock
	goto_if_set FLAG_OLDALE_GRUNT_TAUGHT, OldaleTown_EventScript_Grunt_Taught
	msgbox OldaleTown_Text_GruntLine, MSGBOX_DEFAULT
	closemessage
	playse SE_PIN
	applymovement LOCALID_OLDALE_GRUNT, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_OLDALE_GRUNT, Common_Movement_Delay32
	waitmovement 0
	applymovement LOCALID_OLDALE_GRUNT, Common_Movement_FacePlayer
	waitmovement 0
	msgbox OldaleTown_Text_GruntTutorOffer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_EventScript_Grunt_Decline
	setvar VAR_0x8005, TUTOR_MOVE_COVET
	special ChooseMonForMoveTutor
	waitstate
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_EventScript_Grunt_Exit
	setflag FLAG_OLDALE_GRUNT_TAUGHT
	setflag FLAG_HIDE_OLDALE_GRUNT
	msgbox OldaleTown_Text_GruntTutorDone, MSGBOX_DEFAULT
	goto OldaleTown_EventScript_Grunt_Exit

OldaleTown_EventScript_Grunt_Taught::
	applymovement LOCALID_OLDALE_GRUNT, Common_Movement_FacePlayer
	waitmovement 0
	msgbox OldaleTown_Text_GruntTaughtStyled, MSGBOX_NPC
	applymovement LOCALID_OLDALE_GRUNT, Common_Movement_FaceLeft
	waitmovement 0
	goto OldaleTown_EventScript_Grunt_Exit

OldaleTown_EventScript_Grunt_Decline::
	msgbox OldaleTown_Text_GruntDecline, MSGBOX_DEFAULT
	goto OldaleTown_EventScript_Grunt_Exit

OldaleTown_EventScript_Grunt_Exit::
	applymovement LOCALID_OLDALE_GRUNT, Common_Movement_FaceLeft
	waitmovement 0
	release
	end

OldaleTown_EventScript_MartEmployee::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_POTION_OLDALE, OldaleTown_EventScript_ExplainPotion
	goto_if_set FLAG_TEMP_1, OldaleTown_EventScript_ExplainPotion
	setflag FLAG_TEMP_1
	playbgm MUS_FOLLOW_ME, FALSE
	msgbox OldaleTown_Text_IWorkAtPokemonMart, MSGBOX_DEFAULT
	closemessage
	switch VAR_FACING
	case DIR_SOUTH, OldaleTown_EventScript_GoToMartSouth
	case DIR_NORTH, OldaleTown_EventScript_GoToMartNorth
	case DIR_EAST, OldaleTown_EventScript_GoToMartEast
	end

OldaleTown_EventScript_GoToMartSouth::
	applymovement LOCALID_OLDALE_MART_EMPLOYEE, OldaleTown_Movement_EmployeeSouth
	applymovement LOCALID_PLAYER, OldaleTown_Movement_PlayerSouth
	waitmovement 0
	goto OldaleTown_EventScript_ExplainPokemonMart
	end

OldaleTown_EventScript_GoToMartNorth::
	applymovement LOCALID_OLDALE_MART_EMPLOYEE, OldaleTown_Movement_EmployeeNorth
	applymovement LOCALID_PLAYER, OldaleTown_Movement_PlayerNorth
	waitmovement 0
	goto OldaleTown_EventScript_ExplainPokemonMart
	end

OldaleTown_EventScript_GoToMartEast::
	applymovement LOCALID_PLAYER, OldaleTown_Movement_PlayerEast
	applymovement LOCALID_OLDALE_MART_EMPLOYEE, OldaleTown_Movement_EmployeeEast
	waitmovement 0
	goto OldaleTown_EventScript_ExplainPokemonMart
	end

OldaleTown_EventScript_ExplainPokemonMart::
	msgbox OldaleTown_Text_ThisIsAPokemonMart, MSGBOX_DEFAULT
	giveitem ITEM_POKE_DOLL
	goto_if_eq VAR_RESULT, FALSE, OldaleTown_EventScript_BagIsFull
	msgbox OldaleTown_Text_PotionExplanation, MSGBOX_DEFAULT
	setflag FLAG_RECEIVED_POTION_OLDALE
	fadedefaultbgm
	release
	end

OldaleTown_EventScript_ExplainPotion::
	msgbox OldaleTown_Text_PotionExplanation, MSGBOX_DEFAULT
	release
	end

OldaleTown_EventScript_BagIsFull::
	msgbox gText_TooBadBagIsFull, MSGBOX_DEFAULT
	fadedefaultbgm
	release
	end

OldaleTown_Movement_EmployeeEast:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_in_place_faster_down
	step_end

OldaleTown_Movement_EmployeeSouth:
	walk_left
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_in_place_faster_down
	step_end

OldaleTown_Movement_EmployeeNorth:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_in_place_faster_down
	step_end

OldaleTown_Movement_Unknown1:
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	delay_8
	walk_in_place_faster_down
	step_end

OldaleTown_Movement_PlayerEast:
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

OldaleTown_Movement_PlayerSouth:
	delay_16
	delay_16
	delay_16
	delay_16
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

OldaleTown_Movement_PlayerNorth:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

OldaleTown_Movement_Unknown2:
	walk_left
	walk_up
	walk_up
	walk_right
	walk_right
	walk_right
	walk_right
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

OldaleTown_Movement_WindTutor_FacePlayer:
	face_player
	step_end

OldaleTown_Movement_WindTutor_FaceRight:
	face_right
	step_end

OldaleTown_EventScript_FootprintsMan::
	lock
	faceplayer
	goto_if_set FLAG_ADVENTURE_STARTED, OldaleTown_EventScript_NotBlockingPath
	msgbox OldaleTown_Text_DiscoveredFootprints, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_FOOTPRINTS_MAN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

OldaleTown_EventScript_BlockedPath::
	lockall
	applymovement LOCALID_PLAYER, OldaleTown_Movement_PlayerStepBack
	applymovement LOCALID_FOOTPRINTS_MAN, OldaleTown_Movement_BackUp
	waitmovement 0
	msgbox OldaleTown_Text_WaitDontComeInHere, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_FOOTPRINTS_MAN, OldaleTown_Movement_ReturnToOriginalPosition
	waitmovement 0
	releaseall
	end

OldaleTown_EventScript_NotBlockingPath::
	msgbox OldaleTown_Text_FinishedSketchingFootprints, MSGBOX_DEFAULT
	release
	end

OldaleTown_EventScript_Rival::
	lockall
	applymovement LOCALID_OLDALE_RIVAL, Common_Movement_FacePlayer
	waitmovement 0
	setvar VAR_0x8009, 0
	goto OldaleTown_EventScript_ShowRivalMessage
	end

OldaleTown_EventScript_RivalTrigger1::
	lockall
	applymovement LOCALID_OLDALE_RIVAL, OldaleTown_Movement_RivalApproachPlayer1
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	setvar VAR_0x8009, 1
	goto OldaleTown_EventScript_ShowRivalMessage
	end

OldaleTown_EventScript_RivalTrigger2::
	lockall
	applymovement LOCALID_OLDALE_RIVAL, OldaleTown_Movement_RivalApproachPlayer2
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	setvar VAR_0x8009, 1
	goto OldaleTown_EventScript_ShowRivalMessage
	end

OldaleTown_EventScript_RivalTrigger3::
	lockall
	applymovement LOCALID_OLDALE_RIVAL, OldaleTown_Movement_RivalApproachPlayer3
	waitmovement 0
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterRight
	waitmovement 0
	setvar VAR_0x8009, 1
	goto OldaleTown_EventScript_ShowRivalMessage
	end

OldaleTown_EventScript_ShowRivalMessage::
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, OldaleTown_EventScript_ShowMayMessage
	goto_if_eq VAR_RESULT, FEMALE, OldaleTown_EventScript_ShowBrendanMessage
	end

OldaleTown_EventScript_ShowMayMessage::
	msgbox OldaleTown_Text_MayLetsGoBack, MSGBOX_DEFAULT
	goto OldaleTown_EventScript_RivalFinish
	end

OldaleTown_EventScript_ShowBrendanMessage::
	msgbox OldaleTown_Text_BrendanLetsGoBack, MSGBOX_DEFAULT
	goto OldaleTown_EventScript_RivalFinish
	end

OldaleTown_EventScript_RivalFinish::
	closemessage
	call_if_eq VAR_0x8009, 0, OldaleTown_EventScript_DoExitMovement1
	call_if_eq VAR_0x8009, 1, OldaleTown_EventScript_DoExitMovement2
	applymovement LOCALID_OLDALE_RIVAL, OldaleTown_Movement_RivalExit
	waitmovement 0
	removeobject LOCALID_OLDALE_RIVAL
	setvar VAR_OLDALE_RIVAL_STATE, 2
	setflag FLAG_HIDE_OLDALE_TOWN_RIVAL
	releaseall
	end

OldaleTown_EventScript_DoExitMovement1::
	goto_if_ne VAR_FACING, DIR_SOUTH, OldaleTown_EventScript_DoExitMovement2
	applymovement LOCALID_OLDALE_RIVAL, OldaleTown_Movement_RivalExit
	waitmovement 0
	return

OldaleTown_EventScript_DoExitMovement2::
	applymovement LOCALID_PLAYER, OldaleTown_Movement_WatchRivalExit
	applymovement LOCALID_OLDALE_RIVAL, OldaleTown_Movement_RivalExit
	waitmovement 0
	return

OldaleTown_Movement_RivalApproachPlayer1:
	walk_left
	walk_left
	step_end

OldaleTown_Movement_RivalApproachPlayer2:
	walk_left
	step_end

OldaleTown_Movement_RivalApproachPlayer3:
	face_left
	step_end

OldaleTown_Movement_RivalExit:
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	step_end

OldaleTown_Movement_WatchRivalExit:
	delay_8
	delay_4
	walk_in_place_faster_down
	step_end

OldaleTown_Movement_PlayerStepBack:
	delay_8
	walk_right
	step_end

OldaleTown_Movement_BackUp:
	walk_fast_up
	walk_in_place_faster_left
	lock_facing_direction
	walk_right
	unlock_facing_direction
	step_end

OldaleTown_Movement_ReturnToOriginalPosition:
	walk_down
	walk_left
	step_end


OldaleTown_EventScript_Despawn_MagmaGrunt1::
	fadescreen FADE_TO_BLACK
	setflag FLAG_HIDE_OLDALE_MAGMAGRUNT1
	removeobject 7
	fadescreen FADE_FROM_BLACK
	end

OldaleTown_EventScript_Despawn_MagmaGrunt2::
	fadescreen FADE_TO_BLACK
	setflag FLAG_HIDE_OLDALE_MAGMAGRUNT2
	setflag FLAG_HIDE_OLDALE_MAGMAGRUNT3
	addobject 8
	addobject 9
	removeobject 8
	removeobject 9
	fadescreen FADE_FROM_BLACK
	end

OldaleTown_EventScript_Despawn_MagmaGrunt3::
	fadescreen FADE_TO_BLACK
	setflag FLAG_HIDE_OLDALE_MAGMAGRUNT3
	addobject 8
	addobject 9
	removeobject 8
	removeobject 9
	fadescreen FADE_FROM_BLACK
	end

OldaleTown_EventScript_MagmaGrunt1::
	trainerbattle_single TRAINER_OLDALE_MAGMAGRUNT1, OldaleTown_Text_MagmaGrunt1Intro, OldaleTown_Text_MagmaGrunt1Defeat, OldaleTown_EventScript_Despawn_MagmaGrunt1
	end

OldaleTown_EventScript_MagmaGrunt2::
	trainerbattle_single TRAINER_OLDALE_MAGMAGRUNT2, OldaleTown_Text_MagmaGrunt2Intro, OldaleTown_Text_MagmaGrunt2Defeat, OldaleTown_EventScript_Despawn_MagmaGrunt2
	end

OldaleTown_EventScript_MagmaGrunt3::
	trainerbattle_single TRAINER_OLDALE_MAGMAGRUNT3, OldaleTown_Text_MagmaGrunt3Intro, OldaleTown_Text_MagmaGrunt3Defeat, OldaleTown_EventScript_Despawn_MagmaGrunt3
	end

OldaleTown_EventScript_WindTutor::
	lockall
	faceplayer
	applymovement LOCALID_WIND_TUTOR, OldaleTown_Movement_WindTutor_FacePlayer
	waitmovement 0
	goto_if_set OLDALE_TOWN_WINDTUTOR_FIRST_TALK, OldaleTown_EventScript_WindTutor_CheckPlayerHasBeakAndClaws
	goto_if_set WINDTUTOR_EMOTE, OldaleTown_EventScript_WindTutor_CheckPlayerHasBeakAndClaws
	msgbox OldaleTown_Text__WindTutorIntro, MSGBOX_NPC
	setflag OLDALE_TOWN_WINDTUTOR_FIRST_TALK
	applymovement LOCALID_WIND_TUTOR, OldaleTown_Movement_WindTutor_FaceRight
	waitmovement 0
	releaseall
	end
OldaleTown_EventScript_WindTutor_CheckPlayerHasBeakAndClaws::
	clearflag HAS_SHARP_BEAK
	clearflag HAS_QUICK_CLAWS
	clearflag HAS_BEAK_AND_CLAWS
	checkitem ITEM_SHARP_BEAK
	call_if_eq VAR_RESULT, TRUE, OldaleTown_EventScript_WindTutor_SetFlagSharpBeak
	checkitem ITEM_QUICK_CLAW, 2
	call_if_eq VAR_RESULT, TRUE, OldaleTown_EventScript_WindTutor_SetFlagQuickClaws
	goto_if_set HAS_SHARP_BEAK, OldaleTown_EventScript_WindTutor_CheckAlsoHasQuickClaws
	goto_if_set HAS_QUICK_CLAWS, OldaleTown_EventScript_WindTutor_CheckAlsoHasSharpBeak
	goto OldaleTown_EventScript_WindTutor_EndTutorMove
	end
OldaleTown_EventScript_WindTutor_CheckAlsoHasQuickClaws::
	call_if_set HAS_QUICK_CLAWS, OldaleTown_EventScript_WindTutor_SetHasBeakAndClaws
	goto OldaleTown_EventScript_WindTutor_AskTutorMon
	end
OldaleTown_EventScript_WindTutor_CheckAlsoHasSharpBeak::
	call_if_set HAS_SHARP_BEAK, OldaleTown_EventScript_WindTutor_SetHasBeakAndClaws
	goto OldaleTown_EventScript_WindTutor_AskTutorMon
	end
OldaleTown_EventScript_WindTutor_AskTutorMon::
	checkitem ITEM_QUICK_CLAW, 2
	goto_if_eq VAR_RESULT, TRUE, OldaleTown_EventScript_WantMeToTeachMoveClaws
	checkitem ITEM_SHARP_BEAK
	goto_if_eq VAR_RESULT, TRUE, OldaleTown_EventScript_WantMeToTeachMoveBeak
	applymovement LOCALID_WIND_TUTOR, OldaleTown_Movement_WindTutor_FaceRight
	waitmovement 0
	releaseall
	end
OldaleTown_EventScript_WantMeToTeachMoveBeak::
	msgbox OldaleTown_Text_WantMeToTeachMoveBeak, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_EventScript_WindTutor_EndTutorMove
	goto OldaleTown_EventScript_WindTutor_ChooseMonToTutor
	end
OldaleTown_EventScript_WantMeToTeachMoveClaws::
	msgbox OldaleTown_Text_WantMeToTeachMoveClaws, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, OldaleTown_EventScript_WindTutor_EndTutorMove
	goto OldaleTown_EventScript_WindTutor_ChooseMonToTutor
	end
OldaleTown_EventScript_WindTutor_ChooseMonToTutor::
	msgbox OldaleTown_Text_TutorWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8006, 3
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, OldaleTown_EventScript_WindTutor_EndTutorMove
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, OldaleTown_EventScript_WindTutor_CantTutorEgg
	goto_if_eq VAR_0x8005, 0, OldaleTown_EventScript_WindTutor_NoMoveToTutorMon
	goto OldaleTown_EventScript_WindTutor_ChooseMoveToTeach
	end
OldaleTown_EventScript_WindTutor_ChooseMoveToTeach::
	special TeachWindMoveTutorMove
	waitstate
	goto_if_eq VAR_0x8004, 0, OldaleTown_EventScript_WindTutor_ChooseMonToTutor
	goto_if_set HAS_BEAK_AND_CLAWS, OldaleTown_EventScript_WindTutor_GiveQuickClaws
	goto_if_set HAS_SHARP_BEAK, OldaleTown_EventScript_WindTutor_GiveSharpBeak
	goto_if_set HAS_QUICK_CLAWS, OldaleTown_EventScript_WindTutor_GiveQuickClaws
	applymovement LOCALID_WIND_TUTOR, OldaleTown_Movement_WindTutor_FaceRight
	waitmovement 0
	releaseall
	end
OldaleTown_EventScript_WindTutor_GiveSharpBeak::
	removeitem ITEM_SHARP_BEAK
	msgbox OldaleTown_Text_HandedOverOneSharpBeak, MSGBOX_DEFAULT
	applymovement LOCALID_WIND_TUTOR, OldaleTown_Movement_WindTutor_FaceRight
	waitmovement 0
	releaseall
	end
OldaleTown_EventScript_WindTutor_GiveQuickClaws::
	removeitem ITEM_QUICK_CLAW, 2
	msgbox OldaleTown_Text_HandedOverTwoQuickClaws, MSGBOX_DEFAULT
	applymovement LOCALID_WIND_TUTOR, OldaleTown_Movement_WindTutor_FaceRight
	waitmovement 0
	releaseall
	end
OldaleTown_EventScript_WindTutor_NoMoveToTutorMon::
	msgbox OldaleTown_Text_NoMoveICanTeachThatMon, MSGBOX_DEFAULT
	goto OldaleTown_EventScript_WindTutor_ChooseMonToTutor
	end
OldaleTown_EventScript_WindTutor_CantTutorEgg::
	msgbox OldaleTown_Text_CantTeachAnEgg, MSGBOX_DEFAULT
	goto OldaleTown_EventScript_WindTutor_ChooseMonToTutor
	end
OldaleTown_EventScript_WindTutor_EndTutorMove::
	msgbox OldaleTown_Text_NeedSomethingInspiring, MSGBOX_DEFAULT
	applymovement LOCALID_WIND_TUTOR, OldaleTown_Movement_WindTutor_FaceRight
	waitmovement 0
	releaseall
	end
OldaleTown_EventScript_WindTutor_SetFlagSharpBeak::
	setflag HAS_SHARP_BEAK
	return
OldaleTown_EventScript_WindTutor_SetFlagQuickClaws::
	setflag HAS_QUICK_CLAWS
	return
OldaleTown_EventScript_WindTutor_SetHasBeakAndClaws::
	setflag HAS_BEAK_AND_CLAWS
	return

OldaleTown_EventScript_WindTutor_Emote::
	checkitem ITEM_SHARP_BEAK, 1
	goto_if_eq VAR_RESULT, TRUE, OldaleTown_EventScript_WindTutor_EmoteHasItem
	checkitem ITEM_QUICK_CLAW, 2
	goto_if_eq VAR_RESULT, TRUE, OldaleTown_EventScript_WindTutor_EmoteHasItem
	release
	end
OldaleTown_EventScript_WindTutor_EmoteHasItem::
	goto_if_set WINDTUTOR_EMOTE, OldaleTown_EventScript_WindTutor_EmoteSet
	setvar VAR_0x8004, LOCALID_WIND_TUTOR
	special NpcFacePlayerEmoteTM
	playse SE_PIN
	setflag WINDTUTOR_EMOTE
	release
	end
OldaleTown_EventScript_WindTutor_EmoteSet::
	release
	end

OldaleTown_Text_MagmaGrunt1Intro:
	.string "Hold it.\p"
	.string "Did you think you could pass\n"
	.string "through freely?$"

OldaleTown_Text_MagmaGrunt1Defeat:
	.string "You got lucky, kid...$"

OldaleTown_Text_MagmaGrunt2Intro:
	.string "This is MAGMA territory, get lost!$"

OldaleTown_Text_MagmaGrunt2Defeat:
	.string "Stay off our turf, DEWGONG!$"

OldaleTown_Text_MagmaGrunt3Intro:
	.string "I could ask you to comply, but\n"
	.string "I prefer a more destructive method.$"

OldaleTown_Text_MagmaGrunt3Defeat:
	.string "How are you still standing?$"

OldaleTown_Text_GruntLine:
	.string "Oh yeah.\n"
	.string "A fresh shipment of POKEBALLS,\p"
	.string "right on time.$"

	OldaleTown_Text_GruntTutorOffer:
	.string "Back off!\n"
	.string "These balls belong to TEAM MAGMA!\p"
	.string "Wait, you mean you weren't coveting\n"
	.string "the POKEBALLS? \p"
	.string "Well, in that case\n"
	.string "I'll teach one of your POKéMON COVET!$"

OldaleTown_Text_GruntDecline:
	.string "Then scram!$"

OldaleTown_Text_GruntTutorDone:
	.string "There! I taught COVET to your\n"
	.string "POKéMON.\p"
	.string "So get lost!$"

OldaleTown_Text_GruntTaughtStyled:
	.string "Go find a different\n"
	.string "POKéMART!$"

OldaleTown_Text_SavingMyProgress:
	.string "I want to take a rest, so I'm saving my\n"
	.string "progress.$"

OldaleTown_Text_IWorkAtPokemonMart:
	.string "Hi!\n"
	.string "I work at a POKéMON MART.\p"
	.string "Can I get you to come with me?$"

OldaleTown_Text_ThisIsAPokemonMart:
	.string "This is a POKéMON MART.\n"
	.string "Just look for our blue roof.\p"
	.string "We sell a variety of goods including\n"
	.string "POKé BALLS for catching POKéMON.\p"
	.string "Here, I'd like you to have this as\n"
	.string "a promotional item.$"

OldaleTown_Text_PotionExplanation:
	.string "A POKé DOLL can be used to escape from\n"
	.string "wild POKéMON battles.\p"
	.string "Use it if you get into trouble!\p"
	.string "Be careful - tall grass often hides\n"
	.string "strong POKéMON. Stay prepared!$"

OldaleTown_Text_WaitDontComeInHere:
	.string "Aaaaah! Wait!\n"
	.string "Please don't come in here.\p"
	.string "I just discovered the footprints of\n"
	.string "a rare POKéMON!\p"
	.string "Wait until I finish sketching\n"
	.string "them, okay?$"

OldaleTown_Text_DiscoveredFootprints:
	.string "I just discovered the footprints of\n"
	.string "a rare POKéMON!\p"
	.string "Wait until I finish sketching\n"
	.string "them, okay?$"

OldaleTown_Text_FinishedSketchingFootprints:
	.string "I finished sketching the footprints of\n"
	.string "a rare POKéMON.\p"
	.string "But it turns out they were only my\n"
	.string "own footprints…$"

OldaleTown_Text_MayLetsGoBack:
	.string "MAY: {PLAYER}{KUN}!\n"
	.string "Over here!\l"
	.string "Let's hurry home!$"

OldaleTown_Text_BrendanLetsGoBack:
	.string "BRENDAN: I'm heading back to my dad's\n"
	.string "LAB now.\l"
	.string "{PLAYER}, you should hustle back, too.$"

OldaleTown_Text_TownSign:
	.string "OLDALE TOWN\n"
	.string "“Where things start off scarce.”$"

OldaleTown_Text_ShedDoorIsLocked:
	.string "The door is locked.\p"
	.string "“STORAGE SHED” is written on the door.$"

OldaleTown_Text_InsertedRustyKey:
	.string "{PLAYER} inserted and turned the\n"
	.string "RUSTY KEY.\p"
	.string "The inserted KEY snapped off in the\n"
	.string "lock, but the door opened.$"

OldaleTown_Text_ShedDoorIsOpen:
	.string "The door is open.$"

OldaleTown_Text__WindTutorIntro:
	.string "When I see POKéMON soaring,\n"
	.string "I find inspiration for my art.\p"
	.string "My art? By that I mean WIND-style\n"
	.string "moves.\p"
	.string "If you show me something that\n"
	.string "inspires me, I will share my\l"
	.string "art with you.$"
OldaleTown_Text_WantMeToTeachMoveBeak:
	.string "Hm! That SHARP BEAK...\n"
	.string "is inspiring!\p"
	.string "Do you want me to teach a WIND-style\n"
	.string "move to a POKéMON of yours?$"
OldaleTown_Text_WantMeToTeachMoveClaws:
	.string "Hm! Those QUICK CLAWs...\n"
	.string "are inspiring!\p"
	.string "Do you want me to teach a WIND-style\n"
	.string "move to a POKéMON of yours?$"
OldaleTown_Text_TutorWhichMon:
	.string "Which POKéMON needs tutoring?$"
OldaleTown_Text_TeachWhichMove:
	.string "Teach which move?$"
OldaleTown_Text_HandedOverOneSharpBeak:
	.string "For one SHARP BEAK, I'll\n"
	.string "share my art with you.$"
OldaleTown_Text_HandedOverTwoQuickClaws:
	.string "For two QUICK CLAWs, I'll\n"
	.string "share my art with you.$"
OldaleTown_Text_NoMoveICanTeachThatMon:
	.string "Sorry… There isn't a single move I can\n"
	.string "teach that POKéMON.$"
OldaleTown_Text_CantTeachAnEgg:
	.string "An EGG? There isn't a thing I can\n"
	.string "teach to an EGG…$"
OldaleTown_Text_NeedSomethingInspiring:
	.string "If you show me something that inspires\n"
	.string "me, I will share my art with you.$"
DewfordTown_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, DewfordTown_OnTransition
	.byte 0

DewfordTown_OnTransition:
	setflag FLAG_VISITED_DEWFORD_TOWN
	end

.equ HAS_LUCKY_PUNCH,   				 FLAG_TEMP_2
.equ HAS_BRIGHT_POWDERS,	 			 FLAG_TEMP_3
.equ HAS_PUNCH_AND_POWDERS, 	 		 FLAG_TEMP_4
.equ DEWFORD_TOWN_PUNCHTUTOR_FIRST_TALK, FLAG_TEMP_13
.equ HAS_CHOICE_BAND,   				 FLAG_TEMP_5
.equ HAS_FOCUS_BANDS,	 				 FLAG_TEMP_6
.equ HAS_CHOICE_AND_FOCUS_BANDS, 		 FLAG_TEMP_7
.equ DEWFORD_TOWN_KICKTUTOR_FIRST_TALK,  FLAG_TEMP_12

DewfordTown_Movement_PunchTutor_FacePlayer:
	face_player
	step_end
DewfordTown_Movement_PunchTutor_FaceDown:
	face_down
	step_end
DewfordTown_EventScript_PunchTutor::
	goto_if_unset FLAG_DEFEATED_DEWFORD_GYM, DewfordTown_EventScript_PunchTutorBusy
	lockall
	faceplayer
	applymovement LOCALID_PUNCH_TUTOR, DewfordTown_Movement_PunchTutor_FacePlayer
	waitmovement 0
	goto_if_set FLAG_DEWFORD_PUNCHTUTOR_INTRODUCED, DewfordTown_EventScript_PunchTutorIntroduced
	msgbox DewfordTown_Text_PunchTutorIntroduced, MSGBOX_NPC
	setflag FLAG_DEWFORD_PUNCHTUTOR_INTRODUCED
	releaseall
	end
DewfordTown_EventScript_PunchTutorIntroduced::
	goto_if_set DEWFORD_TOWN_PUNCHTUTOR_FIRST_TALK, DewfordTown_EventScript_PunchTutor_CheckPlayerHasPunchAndPowders
	msgbox DewfordTown_Text_PunchTutorIntro, MSGBOX_NPC
	setflag DEWFORD_TOWN_PUNCHTUTOR_FIRST_TALK
	applymovement LOCALID_PUNCH_TUTOR, DewfordTown_Movement_PunchTutor_FaceDown
	waitmovement 0
	releaseall
	end
DewfordTown_EventScript_PunchTutorBusy::
	msgbox DewfordTown_Text_PunchTutorBusy, MSGBOX_NPC
	applymovement LOCALID_PUNCH_TUTOR, DewfordTown_Movement_PunchTutor_FaceDown
	waitmovement 0
	releaseall
	end
DewfordTown_EventScript_PunchTutor_CheckPlayerHasPunchAndPowders::
	clearflag HAS_LUCKY_PUNCH
	clearflag HAS_BRIGHT_POWDERS
	clearflag HAS_PUNCH_AND_POWDERS
	checkitem ITEM_LUCKY_PUNCH
	call_if_eq VAR_RESULT, TRUE, DewfordTown_EventScript_PunchTutor_SetFlagLuckyPunch
	checkitem ITEM_BRIGHT_POWDER, 2
	call_if_eq VAR_RESULT, TRUE, DewfordTown_EventScript_PunchTutor_SetFlagBrightPowders
	goto_if_set HAS_LUCKY_PUNCH, DewfordTown_EventScript_PunchTutor_CheckAlsoHasBrightPowders
	goto_if_set HAS_BRIGHT_POWDERS, DewfordTown_EventScript_PunchTutor_CheckAlsoHasLuckyPunch
	goto DewfordTown_EventScript_PunchTutor_EndTutorMove
	end
DewfordTown_EventScript_PunchTutor_CheckAlsoHasBrightPowders::
	call_if_set HAS_BRIGHT_POWDERS, DewfordTown_EventScript_PunchTutor_SetHasPunchAndPowders
	goto DewfordTown_EventScript_PunchTutor_AskTutorMon
	end
DewfordTown_EventScript_PunchTutor_CheckAlsoHasLuckyPunch::
	call_if_set HAS_LUCKY_PUNCH, DewfordTown_EventScript_PunchTutor_SetHasPunchAndPowders
	goto DewfordTown_EventScript_PunchTutor_AskTutorMon
	end
DewfordTown_EventScript_PunchTutor_AskTutorMon::
	checkitem ITEM_BRIGHT_POWDER, 2
	goto_if_eq VAR_RESULT, TRUE, DewfordTown_EventScript_WantMeToTeachMovePowders
	checkitem ITEM_LUCKY_PUNCH
	goto_if_eq VAR_RESULT, TRUE, DewfordTown_EventScript_WantMeToTeachMovePunch
	applymovement LOCALID_PUNCH_TUTOR, DewfordTown_Movement_PunchTutor_FaceDown
	waitmovement 0
	releaseall
	end
DewfordTown_EventScript_WantMeToTeachMovePunch::
	msgbox DewfordTown_Text_WantMeToTeachMovePunch, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_EventScript_PunchTutor_EndTutorMove
	goto DewfordTown_EventScript_PunchTutor_ChooseMonToTutor
	end
DewfordTown_EventScript_WantMeToTeachMovePowders::
	msgbox DewfordTown_Text_WantMeToTeachMovePowders, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_EventScript_PunchTutor_EndTutorMove
	goto DewfordTown_EventScript_PunchTutor_ChooseMonToTutor
	end
DewfordTown_EventScript_PunchTutor_ChooseMonToTutor::
	msgbox DewfordTown_Text_TutorWhichMon, MSGBOX_DEFAULT
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, DewfordTown_EventScript_PunchTutor_EndTutorMove
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, DewfordTown_EventScript_PunchTutor_CantTutorEgg
	goto_if_eq VAR_0x8005, 0, DewfordTown_EventScript_PunchTutor_NoMoveToTutorMon
	goto DewfordTown_EventScript_PunchTutor_ChooseMoveToTeach
	end
DewfordTown_EventScript_PunchTutor_ChooseMoveToTeach::
	special TeachPunchMoveTutorMove
	waitstate
	goto_if_eq VAR_0x8004, 0, DewfordTown_EventScript_PunchTutor_ChooseMonToTutor
	goto_if_set HAS_PUNCH_AND_POWDERS, DewfordTown_EventScript_PunchTutor_GiveBrightPowders
	goto_if_set HAS_LUCKY_PUNCH, DewfordTown_EventScript_PunchTutor_GiveLuckyPunch
	goto_if_set HAS_BRIGHT_POWDERS, DewfordTown_EventScript_PunchTutor_GiveBrightPowders
	applymovement LOCALID_PUNCH_TUTOR, DewfordTown_Movement_PunchTutor_FaceDown
	waitmovement 0
	releaseall
	end
DewfordTown_EventScript_PunchTutor_GiveLuckyPunch::
	removeitem ITEM_LUCKY_PUNCH
	msgbox DewfordTown_Text_HandedOverOneLuckyPunch, MSGBOX_DEFAULT
	applymovement LOCALID_PUNCH_TUTOR, DewfordTown_Movement_PunchTutor_FaceDown
	waitmovement 0
	releaseall
	end
DewfordTown_EventScript_PunchTutor_GiveBrightPowders::
	removeitem ITEM_BRIGHT_POWDER, 2
	msgbox DewfordTown_Text_HandedOverTwoBrightPowders, MSGBOX_DEFAULT
	applymovement LOCALID_PUNCH_TUTOR, DewfordTown_Movement_PunchTutor_FaceDown
	waitmovement 0
	releaseall
	end
DewfordTown_EventScript_PunchTutor_NoMoveToTutorMon::
	msgbox DewfordTown_Text_NoMoveICanTeachThatMon, MSGBOX_DEFAULT
	goto DewfordTown_EventScript_PunchTutor_ChooseMonToTutor
	end
DewfordTown_EventScript_PunchTutor_CantTutorEgg::
	msgbox DewfordTown_Text_CantTeachAnEgg, MSGBOX_DEFAULT
	goto DewfordTown_EventScript_PunchTutor_ChooseMonToTutor
	end
DewfordTown_EventScript_PunchTutor_EndTutorMove::
	msgbox DewfordTown_Text_NeedSomethingInspiring, MSGBOX_DEFAULT
	applymovement LOCALID_PUNCH_TUTOR, DewfordTown_Movement_PunchTutor_FaceDown
	waitmovement 0
	releaseall
	end
DewfordTown_EventScript_PunchTutor_SetFlagLuckyPunch::
	setflag HAS_LUCKY_PUNCH
	return
DewfordTown_EventScript_PunchTutor_SetFlagBrightPowders::
	setflag HAS_BRIGHT_POWDERS
	return
DewfordTown_EventScript_PunchTutor_SetHasPunchAndPowders::
	setflag HAS_PUNCH_AND_POWDERS
	return

DewfordTown_EventScript_KickTutor::
	goto_if_unset FLAG_DEFEATED_DEWFORD_GYM, DewfordTown_EventScript_KickTutorBusy
	lock
	faceplayer
	goto_if_set FLAG_DEWFORD_KICKTUTOR_INTRODUCED, DewfordTown_EventScript_KickTutorIntroduced
	msgbox DewfordTown_Text_KickTutorIntroduced, MSGBOX_NPC
	setflag FLAG_DEWFORD_KICKTUTOR_INTRODUCED
	releaseall
	end
DewfordTown_EventScript_KickTutorIntroduced::
	goto_if_set DEWFORD_TOWN_KICKTUTOR_FIRST_TALK, DewfordTown_EventScript_KickTutor_CheckPlayerHasChoiceAndFocusBands
	msgbox DewfordTown_Text_KickTutorIntro, MSGBOX_NPC
	setflag DEWFORD_TOWN_KICKTUTOR_FIRST_TALK
	releaseall
	end
DewfordTown_EventScript_KickTutorBusy::
	msgbox DewfordTown_Text_KickTutorBusy, MSGBOX_NPC
	releaseall
	end
DewfordTown_EventScript_KickTutor_CheckPlayerHasChoiceAndFocusBands::
	clearflag HAS_CHOICE_BAND
	clearflag HAS_FOCUS_BANDS
	clearflag HAS_CHOICE_AND_FOCUS_BANDS
	checkitem ITEM_CHOICE_BAND
	call_if_eq VAR_RESULT, TRUE, DewfordTown_EventScript_KickTutor_SetFlagChoiceBand
	checkitem ITEM_FOCUS_BAND, 2
	call_if_eq VAR_RESULT, TRUE, DewfordTown_EventScript_KickTutor_SetFlagFocusBands
	goto_if_set HAS_CHOICE_BAND, DewfordTown_EventScript_KickTutor_CheckAlsoHasFocusBands
	goto_if_set HAS_FOCUS_BANDS, DewfordTown_EventScript_KickTutor_CheckAlsoHasChoiceBand
	goto DewfordTown_EventScript_KickTutor_EndTutorMoveNoItems
	end
DewfordTown_EventScript_KickTutor_CheckAlsoHasFocusBands::
	call_if_set HAS_FOCUS_BANDS, DewfordTown_EventScript_KickTutor_SetHasChoiceAndFocusBands
	goto DewfordTown_EventScript_KickTutor_AskTutorMon
	end
DewfordTown_EventScript_KickTutor_CheckAlsoHasChoiceBand::
	call_if_set HAS_CHOICE_BAND, DewfordTown_EventScript_KickTutor_SetHasChoiceAndFocusBands
	goto DewfordTown_EventScript_KickTutor_AskTutorMon
	end
DewfordTown_EventScript_KickTutor_AskTutorMon::
	checkitem ITEM_FOCUS_BAND, 2
	goto_if_eq VAR_RESULT, TRUE, DewfordTown_EventScript_WantMeToTeachMoveFocusBands
	checkitem ITEM_CHOICE_BAND
	goto_if_eq VAR_RESULT, TRUE, DewfordTown_EventScript_WantMeToTeachMoveChoiceBand
	release
	end
DewfordTown_EventScript_WantMeToTeachMoveChoiceBand::
	msgbox DewfordTown_Text_WantMeToTeachMoveChoiceBand, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_EventScript_KickTutor_EndTutorMove
	goto DewfordTown_EventScript_KickTutor_ChooseMonToTutor
	end
DewfordTown_EventScript_WantMeToTeachMoveFocusBands::
	msgbox DewfordTown_Text_WantMeToTeachMoveFocusBands, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, DewfordTown_EventScript_KickTutor_EndTutorMove
	goto DewfordTown_EventScript_KickTutor_ChooseMonToTutor
	end
DewfordTown_EventScript_KickTutor_ChooseMonToTutor::
	msgbox DewfordTown_Text_TutorWhichMon, MSGBOX_DEFAULT
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, DewfordTown_EventScript_KickTutor_EndTutorMove
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, DewfordTown_EventScript_KickTutor_CantTutorEgg
	goto_if_eq VAR_0x8005, 0, DewfordTown_EventScript_KickTutor_NoMoveToTutorMon
	goto DewfordTown_EventScript_KickTutor_ChooseMoveToTeach
	end
DewfordTown_EventScript_KickTutor_ChooseMoveToTeach::
	special TeachKickMoveTutorMove
	waitstate
	goto_if_eq VAR_0x8004, 0, DewfordTown_EventScript_KickTutor_EndTutorMove
	goto_if_set HAS_CHOICE_AND_FOCUS_BANDS, DewfordTown_EventScript_KickTutor_GiveFocusBands
	goto_if_set HAS_CHOICE_BAND, DewfordTown_EventScript_KickTutor_GiveChoiceBand
	goto_if_set HAS_FOCUS_BANDS, DewfordTown_EventScript_KickTutor_GiveFocusBands
	release
	end
DewfordTown_EventScript_KickTutor_GiveChoiceBand::
	removeitem ITEM_CHOICE_BAND
	release
	end
DewfordTown_EventScript_KickTutor_GiveFocusBands::
	removeitem ITEM_FOCUS_BAND, 2
	release
	end
DewfordTown_EventScript_KickTutor_NoMoveToTutorMon::
	msgbox DewfordTown_Text_NoMoveICanTeachThatMon, MSGBOX_DEFAULT
	goto DewfordTown_EventScript_KickTutor_EndTutorMove
	end
DewfordTown_EventScript_KickTutor_CantTutorEgg::
	msgbox DewfordTown_Text_CantTeachAnEgg, MSGBOX_DEFAULT
	goto DewfordTown_EventScript_KickTutor_EndTutorMove
	end
DewfordTown_EventScript_KickTutor_EndTutorMove::
	release
	end
DewfordTown_EventScript_KickTutor_EndTutorMoveNoItems::
	msgbox DewfordTown_Text_Kick_Tutor_SomethingInspiring, MSGBOX_DEFAULT
	release
	end
DewfordTown_EventScript_KickTutor_SetFlagChoiceBand::
	setflag HAS_CHOICE_BAND
	return
DewfordTown_EventScript_KickTutor_SetFlagFocusBands::
	setflag HAS_FOCUS_BANDS
	return
DewfordTown_EventScript_KickTutor_SetHasChoiceAndFocusBands::
	setflag HAS_CHOICE_AND_FOCUS_BANDS
	return

DewfordTown_EventScript_Briney::
	lock
	faceplayer
	goto_if_unset FLAG_DELIVERED_STEVEN_LETTER, DewfordTown_EventScript_ReturnToPetalburgPrompt
	message DewfordTown_Text_WhereAreWeBound
	waitmessage
	multichoicedefault 21, 6, MULTI_BRINEY_ON_DEWFORD, 2, FALSE
	switch VAR_RESULT
	case 0, DewfordTown_EventScript_ChoosePetalburg
	case 1, DewfordTown_EventScript_ChooseSlateport
	case 2, DewfordTown_EventScript_CancelSailSelect
	case MULTI_B_PRESSED, DewfordTown_EventScript_CancelSailSelect
	end

DewfordTown_EventScript_ChoosePetalburg::
	msgbox DewfordTown_Text_PetalburgWereSettingSail, MSGBOX_DEFAULT
	closemessage
	goto DewfordTown_EventScript_SailToPetalburg
	release
	end

DewfordTown_EventScript_ChooseSlateport::
	msgbox DewfordTown_Text_SlateportWereSettingSail, MSGBOX_DEFAULT
	closemessage
	goto DewfordTown_EventScript_SailToSlateport
	release
	end

DewfordTown_EventScript_CancelSailSelect::
	msgbox DewfordTown_Text_JustTellMeWhenYouNeedToSetSail, MSGBOX_DEFAULT
	closemessage
	release
	end

DewfordTown_EventScript_ReturnToPetalburgPrompt::
	msgbox DewfordTown_Text_SetSailBackToPetalburg, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, DewfordTown_EventScript_SailBackToPetalburg
	msgbox DewfordTown_Text_GoDeliverIllBeWaiting, MSGBOX_DEFAULT
	release
	end

DewfordTown_EventScript_SailBackToPetalburg::
	msgbox DewfordTown_Text_PetalburgWereSettingSail2, MSGBOX_DEFAULT
	closemessage
	goto DewfordTown_EventScript_SailToPetalburg
	end

DewfordTown_EventScript_Woman::
	msgbox DewfordTown_Text_TinyIslandCommunity, MSGBOX_NPC
	end

DewfordTown_EventScript_TownSign::
	msgbox DewfordTown_Text_TownSign, MSGBOX_SIGN
	clearflag FLAG_DEFEATED_DEWFORD_GYM
	end

DewfordTown_EventScript_GymSign::
	msgbox DewfordTown_Text_GymSign, MSGBOX_SIGN
	setflag FLAG_DEFEATED_DEWFORD_GYM
	end

DewfordTown_EventScript_HallSign::
	msgbox DewfordTown_Text_HallSign, MSGBOX_SIGN
	end

DewfordTown_EventScript_SailToPetalburg::
	call EventScript_BackupMrBrineyLocation
	setobjectsubpriority LOCALID_DEWFORD_BRINEY, MAP_DEWFORD_TOWN, 0
	setobjectsubpriority LOCALID_PLAYER, MAP_DEWFORD_TOWN, 0
	applymovement LOCALID_DEWFORD_BRINEY, DewfordTown_Movement_BrineyBoardBoat
	waitmovement 0
	removeobject LOCALID_DEWFORD_BRINEY
	applymovement LOCALID_PLAYER, DewfordTown_Movement_PlayerBoardBoat
	waitmovement 0
	hideobjectat LOCALID_PLAYER, MAP_DEWFORD_TOWN
	call Common_EventScript_PlayBrineysBoatMusic
	applymovement LOCALID_DEWFORD_BOAT, DewfordTown_Movement_SailToPetalburg
	applymovement LOCALID_PLAYER, DewfordTown_Movement_SailToPetalburg
	waitmovement 0
	showobjectat LOCALID_PLAYER, MAP_ROUTE104
	call Common_EventScript_StopBrineysBoatMusic
	applymovement LOCALID_PLAYER, DewfordTown_Movement_ExitBoatPetalburg
	waitmovement 0
	showobjectat LOCALID_PLAYER, MAP_ROUTE104
	clearflag FLAG_HIDE_BRINEYS_HOUSE_MR_BRINEY
	clearflag FLAG_HIDE_BRINEYS_HOUSE_PEEKO
	clearflag FLAG_HIDE_ROUTE_104_MR_BRINEY_BOAT
	setflag FLAG_HIDE_MR_BRINEY_BOAT_DEWFORD_TOWN
	hideobjectat LOCALID_DEWFORD_BOAT, MAP_DEWFORD_TOWN
	setvar VAR_BOARD_BRINEY_BOAT_STATE, 2
	resetobjectsubpriority LOCALID_PLAYER, MAP_DEWFORD_TOWN
	warp MAP_ROUTE104_MR_BRINEYS_HOUSE, 5, 4
	copyvar VAR_BRINEY_LOCATION, VAR_0x8008
	waitstate
	release
	end

DewfordTown_EventScript_SailToSlateport::
	call EventScript_BackupMrBrineyLocation
	setobjectsubpriority LOCALID_DEWFORD_BRINEY, MAP_DEWFORD_TOWN, 0
	setobjectsubpriority LOCALID_PLAYER, MAP_DEWFORD_TOWN, 1
	applymovement LOCALID_DEWFORD_BRINEY, DewfordTown_Movement_BrineyBoardBoat
	waitmovement 0
	removeobject LOCALID_DEWFORD_BRINEY
	applymovement LOCALID_PLAYER, DewfordTown_Movement_PlayerBoardBoat
	waitmovement 0
	hideobjectat LOCALID_PLAYER, MAP_DEWFORD_TOWN
	call Common_EventScript_PlayBrineysBoatMusic
	applymovement LOCALID_DEWFORD_BOAT, DewfordTown_Movement_SailToSlateport
	applymovement LOCALID_PLAYER, DewfordTown_Movement_SailToSlateport
	waitmovement 0
	call Common_EventScript_StopBrineysBoatMusic
	showobjectat LOCALID_PLAYER, MAP_ROUTE109
	applymovement LOCALID_PLAYER, DewfordTown_Movement_ExitBoatSlateport
	waitmovement 0
	setobjectxyperm LOCALID_ROUTE109_BRINEY, 21, 26
	addobject LOCALID_ROUTE109_BRINEY
	setobjectsubpriority LOCALID_ROUTE109_BRINEY, MAP_ROUTE109, 0
	applymovement LOCALID_ROUTE109_BRINEY, DewfordTown_Movement_BrineyExitBoat
	waitmovement 0
	clearflag FLAG_HIDE_ROUTE_109_MR_BRINEY
	addobject LOCALID_ROUTE109_BOAT
	clearflag FLAG_HIDE_ROUTE_109_MR_BRINEY_BOAT
	setflag FLAG_HIDE_MR_BRINEY_BOAT_DEWFORD_TOWN
	hideobjectat LOCALID_DEWFORD_BOAT, MAP_DEWFORD_TOWN
	call_if_unset FLAG_DELIVERED_DEVON_GOODS, DewfordTown_EventScript_LandedSlateportDeliverGoods
	call_if_set FLAG_DELIVERED_DEVON_GOODS, DewfordTown_EventScript_LandedSlateport
	closemessage
	copyvar VAR_BRINEY_LOCATION, VAR_0x8008
	resetobjectsubpriority LOCALID_PLAYER, MAP_DEWFORD_TOWN
	resetobjectsubpriority LOCALID_ROUTE109_BRINEY, MAP_ROUTE109
	copyobjectxytoperm LOCALID_ROUTE109_BRINEY
	release
	end

DewfordTown_EventScript_LandedSlateportDeliverGoods::
	msgbox DewfordTown_Text_BrineyLandedInSlateportDeliverGoods, MSGBOX_DEFAULT
	return

DewfordTown_EventScript_LandedSlateport::
	msgbox DewfordTown_Text_BrineyLandedInSlateport, MSGBOX_DEFAULT
	return

DewfordTown_Movement_SailToPetalburg:
	walk_up
	walk_up
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
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_left
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_left
	walk_fast_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_faster_left
	walk_fast_left
	walk_fast_left
	walk_fast_up
	walk_fast_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_fast_up
	walk_up
	walk_up
	walk_up
	walk_up
	step_end

DewfordTown_Movement_SailToSlateport:
	walk_right
	walk_fast_right
	walk_fast_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_faster_right
	walk_fast_right
	walk_fast_right
	walk_fast_up
	walk_fast_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
	walk_faster_up
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

DewfordTown_Movement_PlayerBoardBoat:
	walk_right
	walk_up
	step_end

DewfordTown_Movement_ExitBoatPetalburg:
	walk_up
	walk_up
	walk_up
	step_end

DewfordTown_Movement_ExitBoatSlateport:
	walk_up
	walk_up
	walk_up
	walk_in_place_faster_down
	step_end

DewfordTown_Movement_BrineyBoardBoat:
	walk_up
	step_end

DewfordTown_Movement_BrineyExitBoat:
	walk_up
	walk_up
	step_end

DewfordTown_EventScript_TrendyPhraseBoy::
	lock
	faceplayer
	call Common_EventScript_BufferTrendyPhrase
	msgbox DewfordTown_Text_XIsTheBiggestHappeningThingRight, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, DewfordTown_EventScript_ConfirmTrendyPhrase
	goto_if_eq VAR_RESULT, NO, DewfordTown_EventScript_RejectTrendyPhrase
	end

DewfordTown_EventScript_ConfirmTrendyPhrase::
	msgbox DewfordTown_Text_YeahDefinitionOfInRightNow, MSGBOX_DEFAULT
	release
	end

DewfordTown_EventScript_RejectTrendyPhrase::
	msgbox DewfordTown_Text_TellMeWhatsNewAndIn, MSGBOX_DEFAULT
	setvar VAR_0x8004, EASY_CHAT_TYPE_TRENDY_PHRASE
	call Common_ShowEasyChatScreen
	lock
	faceplayer
	goto_if_eq VAR_RESULT, TRUE, DewfordTown_EventScript_GiveNewTrendyPhrase
	goto_if_eq VAR_RESULT, FALSE, DewfordTown_EventScript_CancelNewTrendyPhrase
	end

DewfordTown_EventScript_GiveNewTrendyPhrase::
	incrementgamestat GAME_STAT_STARTED_TRENDS
	goto_if_eq VAR_0x8004, FALSE, DewfordTown_EventScript_PhraseNotTrendyEnough
	msgbox DewfordTown_Text_OfCourseIKnowAboutThat, MSGBOX_DEFAULT
	release
	end

DewfordTown_EventScript_CancelNewTrendyPhrase::
	msgbox DewfordTown_Text_HearOfAnyTrendsComeShareWithMe, MSGBOX_DEFAULT
	release
	end

DewfordTown_EventScript_PhraseNotTrendyEnough::
	msgbox DewfordTown_Text_XHuhIThinkYIsCool, MSGBOX_DEFAULT
	release
	end

DewfordTown_Text_TinyIslandCommunity:
	.string "DEWFORD is a tiny island community.\n"
	.string "If something gets trendy here,\l"
	.string "everyone picks up on it right away.$"

DewfordTown_Text_TownSign:
	.string "DEWFORD TOWN\n"
	.string "“A tiny island in the blue sea.”$"

DewfordTown_Text_GymSign:
	.string "DEWFORD TOWN POKéMON GYM\n"
	.string "LEADER: BRAWLY\l"
	.string "“A big wave in fighting!”$"

DewfordTown_Text_HallSign:
	.string "DEWFORD HALL\n"
	.string "“Everyone's information exchange!”$"

Route104_Text_LandedInDewfordDeliverLetter:
	.string "MR. BRINEY: Ahoy!\n"
	.string "We've hit land in DEWFORD.\p"
	.string "I suppose you're off to deliver that\n"
	.string "LETTER to, who was it now, STEVEN!$"

DewfordTown_Text_SetSailBackToPetalburg:
	.string "MR. BRINEY: Have you delivered your\n"
	.string "LETTER?\p"
	.string "Or were you meaning to sail back to\n"
	.string "PETALBURG?$"

DewfordTown_Text_PetalburgWereSettingSail2:
	.string "MR. BRINEY: PETALBURG it is, then!\p"
	.string "Anchors aweigh!\n"
	.string "PEEKO, we're setting sail, my darling!$"

DewfordTown_Text_GoDeliverIllBeWaiting:
	.string "MR. BRINEY: Then you go on and deliver\n"
	.string "the LETTER. I'll be waiting.$"

DewfordTown_Text_BrineyLandedInDewford:
	.string "MR. BRINEY: Ahoy!\n"
	.string "We've hit land in DEWFORD!\p"
	.string "You just go on and tell me whenever\n"
	.string "you want to set sail again!$"

DewfordTown_Text_WhereAreWeBound:
	.string "MR. BRINEY: Ahoy!\n"
	.string "For you, I'll go out to sea anytime!\p"
	.string "Now, my friend, where are we bound?$"

DewfordTown_Text_PetalburgWereSettingSail:
	.string "MR. BRINEY: PETALBURG, is it?\p"
	.string "Anchors aweigh!\n"
	.string "PEEKO, we're setting sail, my darling!$"

DewfordTown_Text_SlateportWereSettingSail:
	.string "MR. BRINEY: SLATEPORT, is it?\p"
	.string "Anchors aweigh!\n"
	.string "PEEKO, we're setting sail, my darling!$"

DewfordTown_Text_JustTellMeWhenYouNeedToSetSail:
	.string "MR. BRINEY: You just tell me whenever\n"
	.string "you need to set sail again!$"

DewfordTown_Text_XIsTheBiggestHappeningThingRight:
	.string "I like what's hip, happening, and trendy.\n"
	.string "I'm always checking it out.\p"
	.string "Listen, have you heard about this new\n"
	.string "“{STR_VAR_1}”?\p"
	.string "That's right!\n"
	.string "Of course you know!\p"
	.string "I mean, sheesh,\n"
	.string "“{STR_VAR_1}”…\l"
	.string "It's the hottest thing in cool!\p"
	.string "Wherever you're from,\n"
	.string "“{STR_VAR_1}”\l"
	.string "is the biggest happening thing, right?$"

DewfordTown_Text_TellMeWhatsNewAndIn:
	.string "Hunh?\n"
	.string "It's not the hip and happening thing?\p"
	.string "Well, hey, you have to tell me,\n"
	.string "what's new and what's “in”?$"

DewfordTown_Text_OfCourseIKnowAboutThat:
	.string "Hunh?\n"
	.string "“{STR_VAR_2}”?\p"
	.string "… …\p"
	.string "…Uh… Yeah! That's right!\n"
	.string "Yeah, I knew that! Knew it all along!\p"
	.string "Of course I know about that!\n"
	.string "“{STR_VAR_2},” right?\p"
	.string "Yeah, that's it, it's there!\n"
	.string "Isn't “{STR_VAR_2}”\l"
	.string "the coolest, or what?\p"
	.string "It's the hippest thing in hip.\n"
	.string "You think I'd not know about it?\p"
	.string "“{STR_VAR_1}”…\n"
	.string "It's, like, so five minutes ago.\p"
	.string "Now, “{STR_VAR_2}” is\n"
	.string "what's vital and in tune with the times!$"

DewfordTown_Text_XHuhIThinkYIsCool:
	.string "Hmm…\n"
	.string "“{STR_VAR_2},” huh?\p"
	.string "But personally, I think\n"
	.string "“{STR_VAR_1}”\l"
	.string "is what's real in cool.$"

DewfordTown_Text_HearOfAnyTrendsComeShareWithMe:
	.string "Well, if you hear of any happening new\n"
	.string "trends, come share them with me, okay?$"

DewfordTown_Text_YeahDefinitionOfInRightNow:
	.string "Yeah, absolutely right!\p"
	.string "“{STR_VAR_1}” is the\n"
	.string "definition of “in” right now.$"

DewfordTown_Text_PunchTutorIntroduced:
	.string "Hey...\n"
	.string "Is that really the KNUCKLE BADGE?\p"
	.string "I've seen you around, but I didn't\n"
	.string "realize you were that good...\p"
	.string "You might even be worth my time.\p"
	.string "I have a talent for boxing, so I\n"
	.string "know all the PUNCH-style moves.\p"
	.string "I could teach you something if you\n"
	.string "were interested- but you would have\l"
	.string "to prove your worth, first.$"
DewfordTown_Text_PunchTutorIntro:
	.string "Are you ready to run some errands-\p"
	.string "I mean, prove your worthy of\n"
	.string "learning from a master?\p"
	.string "Then bring me something a PUNCH-style\n"
	.string "fighter would appreciate, and I'll\l"
	.string "show you any PUNCH you like.$"
DewfordTown_Text_PunchTutorBusy:
	.string "...\p"
	.string "He is too focused training\n"
	.string "to notice you.$"
DewfordTown_Text_WantMeToTeachMovePunch:
	.string "Is that a LUCKY PUNCH?!\n"
	.string "You picked up a signature strategy!\p"
	.string "Do you want me to teach a PUNCH-style\n"
	.string "move to a POKéMON of yours?$"
DewfordTown_Text_WantMeToTeachMovePowders:
	.string "Is that BRIGHT POWDER?\p"
	.string "Enough for both gloves, too...\p"
	.string "Do you want me to teach a PUNCH-style\n"
	.string "move to a POKéMON of yours?$"
DewfordTown_Text_TutorWhichMon:
	.string "Which POKéMON needs tutoring?$"
DewfordTown_Text_TeachWhichMove:
	.string "Teach which move?$"
DewfordTown_Text_HandedOverOneLuckyPunch:
	.string "For one LUCKY PUNCH, I'll\n"
	.string "show you some real boxing.$"
DewfordTown_Text_HandedOverTwoBrightPowders:
	.string "For two BRIGHT POWDERs, I'll\n"
	.string "show you some real boxing.$"
DewfordTown_Text_NoMoveICanTeachThatMon:
	.string "Sorry… There isn't a single move I can\n"
	.string "teach that POKéMON.$"
DewfordTown_Text_CantTeachAnEgg:
	.string "An EGG? There isn't a thing I can\n"
	.string "teach to an EGG…$"
DewfordTown_Text_NeedSomethingInspiring:
	.string "Show me you know what it takes to\n"
	.string "win. Then I'll teach you some moves.$"

DewfordTown_Text_KickTutorIntroduced:
	.string "You beat BRAWLY!\p"
	.string "I watched the whole thing through\n"
	.string "the back window!\p"
	.string "That was one masterful battle.\p"
	.string "You know, I happen to be a master\n"
	.string "in my own field: KICK-style moves!\p"
	.string "I would be happy to offer you my\n"
	.string "services as a move tutor.\p"
	.string "I'll have your POKEMON kickin' butt!$"
DewfordTown_Text_KickTutorIntro:
	.string "So, you wanna learn a KICK-style move?\n"
	.string "First, some training is required.\p"
	.string "Did you know some POKEMON wear BANDs\n"
	.string "around their heads to improve focus?\p"
	.string "If you can snag one or two of those\n"
	.string "BANDs, I'll teach you a KICK-style move.$"
DewfordTown_Text_KickTutorBusy:
	.string "...\p"
	.string "She is too focused training\n"
	.string "to notice you.$"
DewfordTown_Text_WantMeToTeachMoveChoiceBand:
	.string "Is that a CHOICE BAND?!\n"
	.string "Impressive!\p"
	.string "Do you want me to teach a KICK-style\n"
	.string "move to a POKéMON of yours?$"
DewfordTown_Text_WantMeToTeachMoveFocusBands:
	.string "You have some FOCUS BANDs?\p"
	.string "Do you want me to teach a KICK-style\n"
	.string "move to a POKéMON of yours?$"
DewfordTown_Text_HandedOverOneChoiceBand:
	.string "For one CHOICE BAND, I'll\n"
	.string "teach you my best KICK.$"
DewfordTown_Text_HandedOverTwoFocusBands:
	.string "For two FOCUS BANDs, I'll\n"
	.string "teach you any KICK.$"
DewfordTown_Text_Kick_Tutor_SomethingInspiring:
	.string "If you want to learn a KICK-style\n"
	.string "move, snag some BANDs from a\l"
	.string "KICK-style POKEMON.$"
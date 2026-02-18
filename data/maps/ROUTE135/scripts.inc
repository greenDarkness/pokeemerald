ROUTE135_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, ROUTE135_OnLoad
	.byte 0

ROUTE135_OnLoad:
	goto_if_eq VAR_BIRCH_STATE, 0, ROUTE135_ShowBrawly
	goto ROUTE135_HideBrawly
	end

.equ DEWFORD_TOWN_BRAWLYTUTOR_FIRST_TALK,	FLAG_TEMP_3

ROUTE135_ShowBrawly::
	clearflag FLAG_HIDE_HITMON_ATOLL_BRAWLY
	clearflag FLAG_GAVE_BRAWLY_PROTEIN
	end
ROUTE135_HideBrawly::
	setflag FLAG_HIDE_HITMON_ATOLL_BRAWLY
	end

ROUTE135_EventScript_BrawlyTutor::
	lock
	faceplayer
	goto_if_set DEWFORD_TOWN_BRAWLYTUTOR_FIRST_TALK, ROUTE135_EventScript_BrawlyTutor_CheckIfProteinGiven
	msgbox ROUTE135_Text_BrawlyTutorIntro, MSGBOX_NPC
	setflag DEWFORD_TOWN_BRAWLYTUTOR_FIRST_TALK
	release
	end
ROUTE135_EventScript_BrawlyTutor_CheckIfProteinGiven::
	goto_if_set FLAG_GAVE_BRAWLY_PROTEIN, ROUTE135_EventScript_BrawlyTutor_ChooseMonToTutor
	goto ROUTE135_EventScript_BrawlyTutor_CheckPlayerHasProtein
	end
ROUTE135_EventScript_BrawlyTutor_CheckPlayerHasProtein::
	checkitem ITEM_PROTEIN, 2
	goto_if_eq VAR_RESULT, TRUE, ROUTE135_EventScript_BrawlyTutor_GiveBrawlyProtein
	goto ROUTE135_EventScript_BrawlyTutor_EndTutorMove
	end
ROUTE135_EventScript_BrawlyTutor_GiveBrawlyProtein::
	msgbox ROUTE135_Text_BrawlyTutor_GiveBrawlyProtein, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ROUTE135_EventScript_BrawlyTutor_End
	setflag FLAG_GAVE_BRAWLY_PROTEIN
	goto ROUTE135_EventScript_BrawlyTutor_ShowStrongestMoves
	end
ROUTE135_EventScript_BrawlyTutor_ShowStrongestMoves::
	msgbox ROUTE135_Text_BrawlyTutor_ShowStrongestMoves, MSGBOX_DEFAULT
	closemessage
	goto ROUTE135_EventScript_BrawlyTutor_ChooseMonToTutorNoMsg
	end
ROUTE135_EventScript_BrawlyTutor_ChooseMonToTutor::
	msgbox ROUTE135_Text_TutorWhichMon, MSGBOX_DEFAULT
	setvar VAR_0x8006, 7
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, ROUTE135_EventScript_BrawlyTutor_EndTutorMove
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, ROUTE135_EventScript_BrawlyTutor_CantTutorEgg
	goto_if_eq VAR_0x8005, 0, ROUTE135_EventScript_BrawlyTutor_NoMoveToTutorMon
	goto ROUTE135_EventScript_BrawlyTutor_ChooseMoveToTeach
	end
ROUTE135_EventScript_BrawlyTutor_ChooseMonToTutorNoMsg::
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, ROUTE135_EventScript_BrawlyTutor_EndTutorMove
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, ROUTE135_EventScript_BrawlyTutor_CantTutorEgg
	goto_if_eq VAR_0x8005, 0, ROUTE135_EventScript_BrawlyTutor_NoMoveToTutorMon
	goto ROUTE135_EventScript_BrawlyTutor_ChooseMoveToTeach
	end
ROUTE135_EventScript_BrawlyTutor_ChooseMoveToTeach::
	applymovement LOCALID_BRAWLY_TUTOR, DewfordTown_Movement_PunchTutor_FacePlayer
	waitmovement 0
	msgbox ROUTE135_Text_TeachWhichMove, MSGBOX_DEFAULT
	special TeachBrawlyMoveTutorMove
	waitstate
	goto_if_eq VAR_0x8004, 0, ROUTE135_EventScript_BrawlyTutor_ChooseMonToTutor
	goto ROUTE135_EventScript_BrawlyTutor_GiveProtein
	end
ROUTE135_EventScript_BrawlyTutor_GiveProtein::
	removeitem ITEM_PROTEIN, 2
	msgbox ROUTE135_Text_BrawlyTutor_KeepTraining, MSGBOX_DEFAULT
	release
	end
ROUTE135_EventScript_BrawlyTutor_NoMoveToTutorMon::
	msgbox ROUTE135_Text_NoMoveICanTeachThatMon, MSGBOX_DEFAULT
	goto ROUTE135_EventScript_BrawlyTutor_ChooseMonToTutor
	end
ROUTE135_EventScript_BrawlyTutor_CantTutorEgg::
	msgbox ROUTE135_Text_CantTeachAnEgg, MSGBOX_DEFAULT
	goto ROUTE135_EventScript_BrawlyTutor_ChooseMonToTutor
	end
ROUTE135_EventScript_BrawlyTutor_EndTutorMove::
	msgbox ROUTE135_Text_BrawlyTutor_KeepTraining, MSGBOX_DEFAULT
	release
	end
ROUTE135_EventScript_BrawlyTutor_End::
	release
	end

ROUTE135_Text_BrawlyTutorIntro:
	.string "Hey {PLAYER}, What are you\n"
	.string "doing here?\p"
	.string "Me? I come here once a week to train\n"
	.string "with the HITMONs.\p"
	.string "Have you come to train with us, too?$"
ROUTE135_Text_BrawlyTutor_GiveBrawlyProtein:
	.string "An energy drink would hit the spot\n"
	.string "right now. I normally bring my SPECIAL\p"
	.string "PROTEIN shake...\p"
	.string "Share your PROTEIN with BRAWLY?$"
ROUTE135_Text_BrawlyTutor_ShowStrongestMoves:
	.string "And here I thought I was the only one\n"
	.string "drinking POKEMON PROTEIN. Thanks!\p"
	.string "Watch and learn, I'll show you\n"
	.string "my strongest moves in return!$"
ROUTE135_Text_BrawlyTutor_KeepTraining:
	.string "Keep training, {PLAYER}.\p"
	.string "I'll be ready the next time you\n"
	.string "walk into my GYM!$"

@universal tutor messages
ROUTE135_Text_NoMoveICanTeachThatMon:
	.string "Sorry… There isn't a single move I can\n"
	.string "teach that POKéMON.$"
ROUTE135_Text_CantTeachAnEgg:
	.string "An EGG? There isn't a thing I can\n"
	.string "teach to an EGG…$"
ROUTE135_Text_TutorWhichMon:
	.string "Which POKéMON needs tutoring?$"
ROUTE135_Text_TeachWhichMove:
	.string "Teach which move?$"

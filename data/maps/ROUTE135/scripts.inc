ROUTE135_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, ROUTE135_OnLoad
	.byte 0

ROUTE135_OnLoad:
	goto_if_eq VAR_BIRCH_STATE, 0, ROUTE135_ShowBrawly
	end

.equ HAS_PROTEIN,  		 					FLAG_TEMP_2
.equ DEWFORD_TOWN_BRAWLYTUTOR_FIRST_TALK,	FLAG_TEMP_3

ROUTE135_ShowBrawly::
	clearflag FLAG_HIDE_HITMON_ATOLL_BRAWLY
	end

ROUTE135_EventScript_BrawlyTutor::
	lock
	faceplayer
	goto_if_set DEWFORD_TOWN_BRAWLYTUTOR_FIRST_TALK, ROUTE135_EventScript_BrawlyTutor_CheckPlayerHasProtein
	msgbox ROUTE135_Text_BrawlyTutorIntro, MSGBOX_NPC
	setflag DEWFORD_TOWN_BRAWLYTUTOR_FIRST_TALK
	release
	end
ROUTE135_EventScript_BrawlyTutor_CheckPlayerHasProtein::
	clearflag HAS_PROTEIN
	checkitem ITEM_PROTEIN, 2
	call_if_eq VAR_RESULT, TRUE, ROUTE135_EventScript_BrawlyTutor_SetFlagProtein
	goto_if_eq VAR_RESULT, TRUE, ROUTE135_EventScript_BrawlyTutor_ShowStrongestMoves
	goto ROUTE135_EventScript_BrawlyTutor_EndTutorMove
	release
	end
ROUTE135_EventScript_BrawlyTutor_ShowStrongestMoves::
	msgbox ROUTE135_Text_BrawlyTutor_GiveBrawlyProtein, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ROUTE135_EventScript_BrawlyTutor_End
	msgbox ROUTE135_Text_BrawlyTutor_ShowStrongestMoves, MSGBOX_DEFAULT
	closemessage
	goto ROUTE135_EventScript_BrawlyTutor_ChooseMonToTutor
	end
ROUTE135_EventScript_BrawlyTutor_ChooseMonToTutor::
	msgbox ROUTE135_Text_TutorWhichMon, MSGBOX_DEFAULT
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, ROUTE135_EventScript_BrawlyTutor_EndTutorMove
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, ROUTE135_EventScript_BrawlyTutor_CantTutorEgg
	goto_if_eq VAR_0x8005, 0, ROUTE135_EventScript_BrawlyTutor_NoMoveToTutorMon
	goto ROUTE135_EventScript_BrawlyTutor_ChooseMoveToTeach
	end
ROUTE135_EventScript_BrawlyTutor_ChooseMoveToTeach::
	msgbox ROUTE135_Text_TeachWhichMove, MSGBOX_DEFAULT
	special TeachPunchMoveTutorMove
	waitstate
	goto_if_eq VAR_0x8004, 0, ROUTE135_EventScript_BrawlyTutor_ChooseMonToTutor
	goto_if_set HAS_PROTEIN, ROUTE135_EventScript_BrawlyTutor_GiveProtein
	release
	end
ROUTE135_EventScript_BrawlyTutor_GiveProtein::
	removeitem ITEM_PROTEIN, 2
	msgbox ROUTE135_Text_BrawlyTutor_HandedOverOneProtein, MSGBOX_DEFAULT
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
ROUTE135_EventScript_BrawlyTutor_SetFlagProtein::
	setflag HAS_PROTEIN
	return
ROUTE135_EventScript_BrawlyTutor_EndTutorMove::
	msgbox ROUTE135_Text_BrawlyTutor_DontStopTraining, MSGBOX_DEFAULT
	releaseall
	end
ROUTE135_EventScript_BrawlyTutor_End::
	release
	end

ROUTE135_Text_BrawlyTutorIntro:
	.string "Hey PLAYER, What are you\n"
	.string "doing here?\p"
	.string "Me? I come here once a week to train\n"
	.string "with the HITMONs.\p"
	.string "Have you come to train with us, too?$"
ROUTE135_Text_BrawlyTutor_GiveBrawlyProtein:
	.string "You came prepared, that PROTEIN looks\n"
	.string "good right about now.\p"
	.string "Give a PROTEIN to BRAWLY?$"
ROUTE135_Text_BrawlyTutor_ShowStrongestMoves:
	.string "Thanks! I'm ready to go!\p"
	.string "Watch and learn, I'll show you\n"
	.string "my strongest moves!$"
ROUTE135_Text_BrawlyTutor_HandedOverOneProtein:
	.string "For one LUCKY PUNCH, I'll\n"
	.string "show you some real boxing.$"
ROUTE135_Text_BrawlyTutor_DontStopTraining:
	.string "Don't stop training, PLAYER.\p"
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

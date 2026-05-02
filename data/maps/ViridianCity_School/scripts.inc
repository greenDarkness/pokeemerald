ViridianCity_School_MapScripts::
	.byte 0

ViridianCity_School_EventScript_Lass::
	lock
	faceplayer
	msgbox ViridianCity_School_Text_TryingToMemorizeNotes
	closemessage
	applymovement LOCALID_SCHOOL_LASS, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	release
	end

ViridianCity_School_EventScript_Woman::
	lock
	faceplayer
	goto ViridianCity_School_EventScript_QuizIntro
	end

ViridianCity_School_EventScript_QuizIntro::
	msgbox ViridianCity_School_Text_QuizIntro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ViridianCity_School_EventScript_QuizDecline
	special StartPokemonQuizSpeed
	msgbox ViridianCity_School_Text_QuizBegin, MSGBOX_DEFAULT
	closemessage
	goto ViridianCity_School_EventScript_QuizAskQuestion

ViridianCity_School_EventScript_QuizDecline::
	msgbox ViridianCity_School_Text_QuizDecline, MSGBOX_DEFAULT
	applymovement LOCALID_SCHOOL_WOMAN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

ViridianCity_School_EventScript_QuizAskQuestion::
	closemessage
	special GeneratePokemonQuizQuestion
	special OpenPokemonQuizPanel
	special CreatePokemonQuizIcons
	special ShowPokemonQuizChoiceMenu
	waitstate
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, ViridianCity_School_EventScript_QuizQuit
	special CheckPokemonQuizAnswer
	goto_if_eq VAR_RESULT, TRUE, ViridianCity_School_EventScript_QuizCorrect
	delay 40
	special DestroyPokemonQuizIcons
	special ClosePokemonQuizPanel
	msgbox ViridianCity_School_Text_QuizWrong, MSGBOX_DEFAULT
	goto ViridianCity_School_EventScript_QuizFailed

ViridianCity_School_EventScript_QuizCorrect::
	playse SE_DING_DONG
	waitse
	special DestroyPokemonQuizIcons
	special ClosePokemonQuizPanel
	msgbox ViridianCity_School_Text_QuizRight, MSGBOX_DEFAULT
	goto ViridianCity_School_EventScript_QuizAfterAnswer

ViridianCity_School_EventScript_QuizFailed::
	msgbox ViridianCity_School_Text_QuizFailed, MSGBOX_DEFAULT
	applymovement LOCALID_SCHOOL_WOMAN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

ViridianCity_School_EventScript_QuizAfterAnswer::
	special GetPokemonQuizQuestionCount
	goto_if_lt VAR_RESULT, 10, ViridianCity_School_EventScript_QuizAskQuestion
	goto ViridianCity_School_EventScript_QuizFinish

ViridianCity_School_EventScript_QuizFinish::
	special GivePokemonQuizReward
	msgbox ViridianCity_School_Text_QuizFinished, MSGBOX_DEFAULT
	giveitem VAR_0x8000
	applymovement LOCALID_SCHOOL_WOMAN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

ViridianCity_School_EventScript_QuizQuit::
	special DestroyPokemonQuizIcons
	special ClosePokemonQuizPanel
	msgbox ViridianCity_School_Text_QuizQuit, MSGBOX_DEFAULT
	applymovement LOCALID_SCHOOL_WOMAN, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

ViridianCity_School_EventScript_Notebook::
	lockall
	msgbox ViridianCity_School_Text_NotebookFirstPage
	msgbox ViridianCity_School_Text_TurnThePage, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ViridianCity_School_EventScript_StopReadingNotebook
	msgbox ViridianCity_School_Text_NotebookSecondPage
	msgbox ViridianCity_School_Text_TurnThePage, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ViridianCity_School_EventScript_StopReadingNotebook
	msgbox ViridianCity_School_Text_NotebookThirdPage
	msgbox ViridianCity_School_Text_TurnThePage, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, ViridianCity_School_EventScript_StopReadingNotebook
	msgbox ViridianCity_School_Text_NotebookFourthPage
	applymovement LOCALID_SCHOOL_LASS, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	textcolor NPC_TEXT_COLOR_FEMALE
	msgbox ViridianCity_School_Text_HeyDontLookAtMyNotes
	releaseall
	end

ViridianCity_School_EventScript_StopReadingNotebook::
	releaseall
	end

ViridianCity_School_EventScript_Blackboard::
	lockall
	msgbox ViridianCity_School_Text_BlackboardListsStatusProblems
	goto ViridianCity_School_EventScript_ChooseBlackboardTopic
	end

ViridianCity_School_EventScript_ChooseBlackboardTopic::
	message ViridianCity_School_Text_ReadWhichTopic
	waitmessage
	multichoicegrid 7, 1, MULTICHOICE_TRAINER_SCHOOL_WHITEBOARD, 3, FALSE
	switch VAR_RESULT
	case 0, ViridianCity_School_EventScript_ReadSleep
	case 1, ViridianCity_School_EventScript_ReadPoison
	case 2, ViridianCity_School_EventScript_ReadParalysis
	case 3, ViridianCity_School_EventScript_ReadBurn
	case 4, ViridianCity_School_EventScript_ReadFreeze
	case 5, ViridianCity_School_EventScript_ExitBlackboard
	case 127, ViridianCity_School_EventScript_ExitBlackboard
	end

ViridianCity_School_EventScript_ReadSleep::
	msgbox ViridianCity_School_Text_ExplainSleep
	goto ViridianCity_School_EventScript_ChooseBlackboardTopic
	end

ViridianCity_School_EventScript_ReadBurn::
	msgbox ViridianCity_School_Text_ExplainBurn
	goto ViridianCity_School_EventScript_ChooseBlackboardTopic
	end

ViridianCity_School_EventScript_ReadPoison::
	msgbox ViridianCity_School_Text_ExplainPoison
	goto ViridianCity_School_EventScript_ChooseBlackboardTopic
	end

ViridianCity_School_EventScript_ReadFreeze::
	msgbox ViridianCity_School_Text_ExplainFreeze
	goto ViridianCity_School_EventScript_ChooseBlackboardTopic
	end

ViridianCity_School_EventScript_ReadParalysis::
	msgbox ViridianCity_School_Text_ExplainParalysis
	goto ViridianCity_School_EventScript_ChooseBlackboardTopic
	end

ViridianCity_School_EventScript_ExitBlackboard::
	releaseall
	end

ViridianCity_School_Text_TryingToMemorizeNotes::
    .string "Whew! I'm trying to memorize all my\n"
    .string "notes.$"

ViridianCity_School_Text_ReadBlackboardCarefully::
    .string "Okay!\p"
    .string "Be sure to read what's on the\n"
    .string "blackboard carefully!$"

ViridianCity_School_Text_QuizIntro::
    .string "Welcome to the VIRIDIAN CITY\n"
    .string "TRAINER school!\p"
    .string "Would you like to test your knowledge\n"
    .string "of POKéMON speed tiers?$"

ViridianCity_School_Text_QuizBegin::
    .string "Great! Let's get started!$"

ViridianCity_School_Text_QuizPromptQuestion::
    .string "Which group is fastest?$"

ViridianCity_School_Text_QuizPromptLabels::
    .string "A                                  C\n"
    .string "B                                  D$"

ViridianCity_School_Text_QuizDecline::
    .string "Come back whenever you'd like\n"
    .string "to test your skill!$"

ViridianCity_School_Text_QuizRight::
    .string "Correct!\p"
    .string "Answer: {STR_VAR_1}$"

ViridianCity_School_Text_QuizWrong::
    .string "Sorry!\p"
    .string "You picked {STR_VAR_2}.\n"
    .string "Answer: {STR_VAR_1}$"

ViridianCity_School_Text_QuizFailed::
    .string "A wrong answer ends the quiz.\p"
	.string "But don't give up!\p"
    .string "Believe in yourself and keep at it!$"

ViridianCity_School_Text_QuizFinished::
    .string "That's all ten questions!\p"
    .string "You scored {STR_VAR_1}/{STR_VAR_2}!\p"
    .string "Here, take this as a reward.$"

ViridianCity_School_Text_QuizQuit::
    .string "Maybe next time!$"

ViridianCity_School_Text_NotebookFirstPage::
    .string "Let's check out the notebook.\p"
    .string "First page…\p"
    .string "POKé BALLS are used to catch\n"
    .string "POKéMON.\p"
    .string "Up to six POKéMON can be carried\n"
    .string "in your party.\p"
    .string "People who raise and battle\n"
    .string "with POKéMON are called TRAINERS.$"

ViridianCity_School_Text_NotebookSecondPage::
    .string "Second page…\p"
    .string "A healthy POKéMON may be hard to\n"
    .string "catch, so weaken it first.\p"
    .string "Poison, burn, or cause another\n"
    .string "status problem to weaken it.$"

ViridianCity_School_Text_NotebookThirdPage::
    .string "Third page…\p"
    .string "POKéMON TRAINERS seek others to\n"
    .string "engage in POKéMON battles.\p"
    .string "To TRAINERS, the taste of victory\n"
    .string "is sweet, indeed.\p"
    .string "Battles are constantly waged at\n"
    .string "POKéMON GYMS everywhere.$"

ViridianCity_School_Text_NotebookFourthPage::
    .string "Fourth page…\p"
    .string "The ultimate goal for all POKéMON\n"
    .string "TRAINERS is simple.\p"
    .string "It is to defeat the top eight\n"
    .string "POKéMON GYM LEADERS.\p"
    .string "Do so to earn the right to face…\p"
    .string "The ELITE FOUR of the POKéMON\n"
    .string "LEAGUE!$"

ViridianCity_School_Text_TurnThePage::
    .string "Turn the page?$"

ViridianCity_School_Text_HeyDontLookAtMyNotes::
    .string "GIRL: Hey!\n"
    .string "Don't look at my notes!$"

ViridianCity_School_Text_BlackboardListsStatusProblems::
    .string "The blackboard lists POKéMON\n"
    .string "STATUS problems during battles.$"

ViridianCity_School_Text_ReadWhichTopic::
    .string "Which topic do you want to read?$"

ViridianCity_School_Text_ExplainSleep::
    .string "A POKéMON can't attack if it's\n"
    .string "asleep.\p"
    .string "POKéMON will stay asleep even\n"
    .string "after battles.\p"
    .string "Use AWAKENING to awaken one\n"
    .string "from sleep.$"

ViridianCity_School_Text_ExplainBurn::
    .string "A burn reduces ATTACK power.\n"
    .string "It also causes ongoing HP loss.\p"
    .string "A burn remains after a battle.\n"
    .string "Use BURN HEAL to cure a burn.$"

ViridianCity_School_Text_ExplainPoison::
    .string "When poisoned, a POKéMON's health\n"
    .string "steadily drops.\p"
    .string "Poison lingers after battles.\n"
    .string "Use an ANTIDOTE to cure poison!$"

ViridianCity_School_Text_ExplainFreeze::
    .string "A frozen POKéMON becomes\n"
    .string "helplessly immobile.\p"
    .string "It stays frozen even after the\n"
    .string "battle ends.\p"
    .string "Use ICE HEAL to thaw out the\n"
    .string "suffering POKéMON.$"

ViridianCity_School_Text_ExplainParalysis::
    .string "Paralysis reduces SPEED and may\n"
    .string "prevent the POKéMON from moving.\p"
    .string "Paralysis remains after battles.\n"
    .string "Use PARLYZ HEAL for treatment.$"


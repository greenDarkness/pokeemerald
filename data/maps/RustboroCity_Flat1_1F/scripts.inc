RustboroCity_Flat1_1F_MapScripts::
	.byte 0

.equ CHECKED_PEARLS,  						FLAG_TEMP_1
.equ HAS_BIG_PEARL,   						FLAG_TEMP_2
.equ HAS_PEARLS,	 						FLAG_TEMP_3
.equ HAS_BOTH_PEARLS,						FLAG_TEMP_4
.equ RUSTBORO_CITY_POWERTUTOR_FIRST_TALK,	FLAG_TEMP_13

RustboroCity_Flat1_1F_EventScript_Man::
	msgbox RustboroCity_Flat1_1F_Text_EveryPokemonHasAbility, MSGBOX_NPC
	end

@

RustboroCity_Flat1_1F_EventScript_PowerTutor::
	setobjectmovementtype 11, MOVEMENT_TYPE_NONE
	lockall
	faceplayer
	goto_if_set RUSTBORO_CITY_POWERTUTOR_FIRST_TALK, RustboroCity_Flat1_1F_EventScript_PowerTutor_MoveTutor
	msgbox RustboroCity_Flat1_1F_Text_HiddenItemsExplanation, MSGBOX_NPC
	setflag RUSTBORO_CITY_POWERTUTOR_FIRST_TALK
	releaseall
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_MoveTutor::
	lockall
	faceplayer
	clearflag HAS_BIG_PEARL
	clearflag HAS_PEARLS
	clearflag HAS_BOTH_PEARLS
	goto RustboroCity_Flat1_1F_EventScript_PowerTutor_CheckPlayerHasPearls
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_CheckPlayerHasPearls::
	checkitem ITEM_BIG_PEARL
	call_if_eq VAR_RESULT, TRUE, RustboroCity_Flat1_1F_EventScript_PowerTutor_SetHasBigPearl
	checkitem ITEM_PEARL, 2
	call_if_eq VAR_RESULT, TRUE, RustboroCity_Flat1_1F_EventScript_PowerTutor_SetHasPearls
	goto_if_set HAS_BIG_PEARL, RustboroCity_Flat1_1F_EventScript_PowerTutor_CheckAlsoHasPearls
	goto_if_set HAS_PEARLS, RustboroCity_Flat1_1F_EventScript_PowerTutor_CheckAlsoHasBigPearl
	goto RustboroCity_Flat1_1F_EventScript_PowerTutor_EndTutorMove
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_CheckAlsoHasPearls::
	call_if_set HAS_PEARLS, RustboroCity_Flat1_1F_EventScript_PowerTutor_SetHasBothPearls
	goto RustboroCity_Flat1_1F_EventScript_PowerTutor_AskTutorMon
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_CheckAlsoHasBigPearl::
	call_if_set HAS_BIG_PEARL, RustboroCity_Flat1_1F_EventScript_PowerTutor_SetHasBothPearls
	goto RustboroCity_Flat1_1F_EventScript_PowerTutor_AskTutorMon
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_AskTutorMon::
	lockall
	faceplayer
	msgbox RustboroCity_Flat1_1F_Text_WantMeToTeachMove, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_Flat1_1F_EventScript_PowerTutor_EndTutorMove
	goto RustboroCity_Flat1_1F_EventScript_PowerTutor_ChooseMonToTutor
	releaseall
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_ChooseMonToTutor::
	lockall
	faceplayer
	setobjectmovementtype 11, MOVEMENT_TYPE_NONE
	msgbox RustboroCity_Flat1_1F_Text_TutorWhichMon
	special ChooseMonForMoveRelearner
	waitstate
	goto_if_ge VAR_0x8004, PARTY_SIZE, RustboroCity_Flat1_1F_EventScript_PowerTutor_EndTutorMove
	special IsSelectedMonEgg
	goto_if_eq VAR_RESULT, TRUE, RustboroCity_Flat1_1F_EventScript_PowerTutor_CantTutorEgg
	goto_if_eq VAR_0x8005, 0, RustboroCity_Flat1_1F_EventScript_PowerTutor_NoMoveToTutorMon
	goto RustboroCity_Flat1_1F_EventScript_PowerTutor_ChooseMoveToTeach
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_ChooseMoveToTeach::
	lockall
	faceplayer
	special TeachPowerMoveTutorMove
	waitstate
	goto_if_eq VAR_0x8004, 0, RustboroCity_Flat1_1F_EventScript_PowerTutor_ChooseMonToTutor
	goto_if_set HAS_BOTH_PEARLS, RustboroCity_Flat1_1F_EventScript_PowerTutor_GivePearls
	goto_if_set HAS_BIG_PEARL, RustboroCity_Flat1_1F_EventScript_PowerTutor_GiveBigPearl
	goto_if_set HAS_PEARLS, RustboroCity_Flat1_1F_EventScript_PowerTutor_GivePearls
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_GiveBigPearl::
	removeitem ITEM_BIG_PEARL
	msgbox RustboroCity_Flat1_1F_Text_HandedOverOneBigPearl
	goto RustboroCity_Flat1_1F_EventScript_PowerTutor_EndTutorMove
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_GivePearls::
	removeitem ITEM_PEARL, 2
	msgbox RustboroCity_Flat1_1F_Text_HandedOverTwoPearls
	goto RustboroCity_Flat1_1F_EventScript_PowerTutor_EndTutorMove
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_NoMoveToTutorMon::
	lockall
	faceplayer
	msgbox RustboroCity_Flat1_1F_Text_NoMoveICanTeachThatMon
	goto RustboroCity_Flat1_1F_EventScript_PowerTutor_ChooseMonToTutor
	releaseall
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_CantTutorEgg::
	lockall
	faceplayer
	msgbox RustboroCity_Flat1_1F_Text_CantTeachAnEgg
	goto RustboroCity_Flat1_1F_EventScript_PowerTutor_ChooseMonToTutor
	releaseall
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_EndTutorMove::
	lockall
	faceplayer
	msgbox RustboroCity_Flat1_1F_Text_NeedPearlOrTwoToLearnMove
	releaseall
	end

RustboroCity_Flat1_1F_EventScript_PowerTutor_SetHasBigPearl::
	setflag HAS_BIG_PEARL
	return

RustboroCity_Flat1_1F_EventScript_PowerTutor_SetHasPearls::
	setflag HAS_PEARLS
	return

RustboroCity_Flat1_1F_EventScript_PowerTutor_SetHasBothPearls::
	setflag HAS_BOTH_PEARLS
	return

@

RustboroCity_Flat1_1F_Text_HiddenItemsExplanation:
	.string "Sometimes, there are things on the\n"
	.string "ground even if you can't see them.\p"
	.string "That's why I always check where I'm\n"
	.string "walking.\p"
	.string "Have you found any PEARLS?$"

RustboroCity_Flat1_1F_Text_WantMeToTeachMove:
	.string "Sniff, sniff…\n"
	.string "Hm! You smell of PEARLS!\p"
	.string "Do you want me to teach a POWER MOVE\n"
	.string "to a POKéMON of yours?$"

RustboroCity_Flat1_1F_Text_TutorWhichMon:
	.string "Which POKéMON needs tutoring?$"

RustboroCity_Flat1_1F_Text_TeachWhichMove:
	.string "Teach which move?$"

RustboroCity_Flat1_1F_Text_HandedOverOneBigPearl:
	.string "For one BIG PEARL, I'll teach\n"
	.string "a move to a POKéMON of yours.$"

RustboroCity_Flat1_1F_Text_HandedOverTwoPearls:
	.string "For two PEARLS, I'll teach\n"
	.string "a move to a POKéMON of yours.$"

RustboroCity_Flat1_1F_Text_NoMoveICanTeachThatMon:
	.string "Sorry… There isn't a single move I can\n"
	.string "teach that POKéMON.$"

RustboroCity_Flat1_1F_Text_CantTeachAnEgg:
	.string "An EGG? There isn't a thing I can\n"
	.string "teach to an EGG…$"

RustboroCity_Flat1_1F_Text_NeedPearlOrTwoToLearnMove:
	.string "If your POKéMON need to learn a MOVE,\n"
	.string "bring me a PEARL or two.$"

@

RustboroCity_Flat1_1F_EventScript_Woman::
	msgbox RustboroCity_Flat1_1F_Text_PokemonStrange, MSGBOX_NPC
	end

RustboroCity_Flat1_1F_Text_EveryPokemonHasAbility:
	.string "Every POKéMON has a special ability\n"
	.string "that it can use.$"

RustboroCity_Flat1_1F_Text_PokemonStrange:
	.string "POKéMON are such strange creatures.$"


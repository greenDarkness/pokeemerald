Route102_MapScripts::
	.byte 0

Route102_OnTransition:
	call ProfBirch_EventScript_UpdateLocation
	end

Route102_EventScript_PokerusKid::
	lock
	faceplayer
	setvar VAR_0x8004, 0
	special BufferMonNickname
	msgbox Route102_Text_PokerusKid_PlayWithYourPokemon, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Route102_EventScript_PokerusKid_ImNotVeryTall
	release
	end
Route102_EventScript_PokerusKid_ImNotVeryTall::
	msgbox Route102_Text_PokerusKid_ImNotVeryTall, MSGBOX_NPC
	fadescreen FADE_TO_BLACK
	delay 16
	playse SE_M_HEAL_BELL
	special GetPokerus
	delay 60
	fadescreen FADE_FROM_BLACK
	delay 60
	msgbox Route102_Text_PokerusKid_ItsAGoodVirus, MSGBOX_NPC
	release
	end

Route102_EventScript_RouteSignOldale::
	msgbox Route102_Text_RouteSignOldale, MSGBOX_SIGN
	end

Route102_EventScript_RouteSignPetalburg::
	msgbox Route102_Text_RouteSignPetalburg, MSGBOX_SIGN
	end

Route102_EventScript_Boy::
	msgbox Route102_Text_CatchWholeBunchOfPokemon, MSGBOX_NPC
	end

Route102_EventScript_Calvin::
	trainerbattle_single TRAINER_CALVIN_1, Route102_Text_CalvinIntro, Route102_Text_CalvinDefeated, Route102_EventScript_CalvinRegisterMatchCallAfterBattle
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route102_EventScript_CalvinRematch
	setvar VAR_0x8004, TRAINER_CALVIN_1
	specialvar VAR_RESULT, IsTrainerRegistered
	goto_if_eq VAR_RESULT, FALSE, Route102_EventScript_CalvinTryRegister
	msgbox Route102_Text_CalvinPostBattle, MSGBOX_DEFAULT
	release
	end

Route102_EventScript_CalvinRegisterMatchCallAfterBattle::
	special PlayerFaceTrainerAfterBattle
	waitmovement 0
	goto_if_set FLAG_HAS_MATCH_CALL, Route102_EventScript_CalvinRegisterMatchCall
	release
	end

Route102_EventScript_CalvinRegisterMatchCall::
	msgbox Route102_Text_CalvinRegisterShort, MSGBOX_DEFAULT
	register_matchcall TRAINER_CALVIN_1
	release
	end

Route102_EventScript_CalvinTryRegister::
	goto_if_set FLAG_HAS_MATCH_CALL, Route102_EventScript_CalvinRegister
	msgbox Route102_Text_CalvinPostBattle, MSGBOX_DEFAULT
	release
	end

Route102_EventScript_CalvinRegister::
	msgbox Route102_Text_CalvinRegister, MSGBOX_DEFAULT
	register_matchcall TRAINER_CALVIN_1
	release
	end

Route102_EventScript_CalvinRematch::
	trainerbattle_rematch TRAINER_CALVIN_1, Route102_Text_CalvinRematchIntro, Route102_Text_CalvinRematchDefeated
	msgbox Route102_Text_CalvinRematchPostBattle, MSGBOX_AUTOCLOSE
	end

Route102_EventScript_Rick::
	trainerbattle_single TRAINER_RICK, Route102_Text_RickIntro, Route102_Text_RickDefeated
	msgbox Route102_Text_RickPostBattle, MSGBOX_AUTOCLOSE
	end

Route102_EventScript_Tiana::
	trainerbattle_single TRAINER_TIANA, Route102_Text_TianaIntro, Route102_Text_TianaDefeated
	msgbox Route102_Text_TianaPostBattle, MSGBOX_AUTOCLOSE
	end

Route102_EventScript_Allen::
	trainerbattle_single TRAINER_ALLEN, Route102_Text_AllenIntro, Route102_Text_AllenDefeated
	msgbox Route102_Text_AllenPostBattle, MSGBOX_AUTOCLOSE
	end

Route102_Text_WatchMeCatchPokemon:
	.string "WALLY: {PLAYER}…\n"
	.string "POKéMON hide in tall grass like this,\l"
	.string "don't they?\p"
	.string "Please watch me and see if I can\n"
	.string "catch one properly.\p"
	.string "…Whoa!$"

Route102_Text_WallyIDidIt:
	.string "WALLY: I did it… It's my…\n"
	.string "My POKéMON!$"

Route102_Text_LetsGoBack:
	.string "{PLAYER}, thank you!\n"
	.string "Let's go back to the GYM!$"

Route102_Text_PokerusKid_ImNotVeryTall:
	.string "I'm…not very tall, so I sink right\n"
	.string "into tall grass.\p"
	.string "The grass goes up my nose and…\n"
	.string "Fwafwafwafwafwa…\p"
	.string "Fwatchoo!$"

Route102_Text_PokerusKid_PlayWithYourPokemon:
	.string "Hey, trainer- WOW, your {STR_VAR_1}\n"
	.string "is one of my favorites.\p"
	.string "Can our POKEMON play in the grass\n"
	.string "together, please?$"

Route102_Text_PokerusKid_ItsAGoodVirus:
	.string "My POKEMON have been sneezing, too-\n"
	.string "but not from the tall grass.\p"
	.string "The lady at the POKEMON CENTER said\n"	
	.string "not to worry- that it's a GOOD virus...\p"
	.string "I'm not lying, I promise!$"		

Route102_Text_CatchWholeBunchOfPokemon:
	.string "I'm going to catch a whole bunch of\n"
	.string "POKéMON!$"

Route102_Text_RouteSignOldale:
	.string "ROUTE 102\n"
	.string "{RIGHT_ARROW} OLDALE TOWN$"

Route102_Text_RouteSignPetalburg:
	.string "ROUTE 102\n"
	.string "{LEFT_ARROW} PETALBURG CITY$"

@ Debug NPC for testing Birch state
Route102_EventScript_DebugBirch::
	lock
	faceplayer
	msgbox Route102_Text_DebugBirchAsk, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Route102_EventScript_DebugAdvanceDay
	goto Route102_EventScript_DebugAdvanceBirch
	end

Route102_EventScript_DebugAdvanceDay::
	special DebugAdvanceFakeRtcDay
	dotimebasedevents
	msgbox Route102_Text_DebugDayAdvanced, MSGBOX_DEFAULT
	release
	end

Route102_EventScript_DebugAdvanceBirch::
	addvar VAR_BIRCH_STATE, 1
	goto_if_lt VAR_BIRCH_STATE, 7, Route102_EventScript_DebugBirchDone
	setvar VAR_BIRCH_STATE, 0
Route102_EventScript_DebugBirchDone::
	msgbox Route102_Text_DebugBirchAdvanced, MSGBOX_DEFAULT
	release
	end

Route102_Text_DebugBirchAsk:
	.string "YES = Advance day\n"
	.string "NO = Advance Birch state$"

Route102_Text_DebugDayAdvanced:
	.string "Day advanced by 1.$"

Route102_Text_DebugBirchAdvanced:
	.string "Birch state advanced by 1.$"

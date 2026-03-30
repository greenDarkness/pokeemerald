Route102_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route102_OnTransition
	.byte 0

Route102_OnTransition:
	call ProfBirch_EventScript_UpdateLocation
	call_if_unset FLAG_DAILY_ROUTE102_CATCHERBOY, Route102_EventScript_CatcherBoy_LookAround
	end

.equ CATCHERBOY_EMOTE,   					FLAG_TEMP_2

Route102_EventScript_CatcherBoy_LookAround::
	setobjectmovementtype LOCALID_CATCHERBOY, MOVEMENT_TYPE_LOOK_AROUND
	return

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

Route102_EventScript_CatcherBoy::
	lock
	faceplayer
	goto_if_set FLAG_DAILY_ROUTE102_CATCHERBOY, Route102_EventScript_CatcherBoy_Default
	call Route102_EventScript_CatcherBoy_SpeciesCheck
	msgbox Route102_Text_CatcherBoy_WannaHelp, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, YES, Route102_EventScript_CatcherBoy_WannaHelp
	setobjectmovementtype LOCALID_CATCHERBOY, MOVEMENT_TYPE_WANDER_AROUND
	release
	end
Route102_EventScript_CatcherBoy_WannaHelp::
	specialvar VAR_RESULT, CountPartyNonEggMons
	goto_if_eq VAR_RESULT, 1, Route102_EventScript_CatcherBoy_NotEnoughPokemon
	copyvar VAR_0x8004, VAR_TEMP_1
	specialvar VAR_RESULT, GetReleaseMonSpeciesInfo
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x8005, VAR_0x8004
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, Route102_EventScript_CatcherBoy_Close
	specialvar VAR_RESULT, GetReleaseMonSpecies
	goto_if_ne VAR_RESULT, VAR_TEMP_1, Route102_EventScript_CatcherBoy_WrongPokemon
	special ReleaseMonFromParty
	setflag FLAG_DAILY_ROUTE102_CATCHERBOY
	lock
	faceplayer
	msgbox Route102_Text_CatcherBoy_Helped, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, GAME_STAT_PLAYER_IP_MAX
	specialvar VAR_RESULT, GetGameStatValue 
	goto_if_lt VAR_RESULT, 100, Route102_EventScript_CatcherBoy_GainIP
	special GiveGroundMomItem
    msgbox Route102_Text_ReceiveItem, MSGBOX_DEFAULT
    waitfanfare
    closemessage
	setobjectmovementtype LOCALID_CATCHERBOY, MOVEMENT_TYPE_WANDER_AROUND
	release
	end
Route102_EventScript_CatcherBoy_GainIP::
	setvar VAR_0x8000, 1
	callnative IncreasePlayerIP
	playfanfare MUS_OBTAIN_SYMBOL
	msgbox Route102_Text_IPGain, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	setobjectmovementtype LOCALID_CATCHERBOY, MOVEMENT_TYPE_WANDER_AROUND
	release
	end
Route102_EventScript_CatcherBoy_Default::
	msgbox Route102_Text_CatcherBoyDefault, MSGBOX_DEFAULT
	closemessage
	release
	end
Route102_EventScript_CatcherBoy_NotEnoughPokemon::
	lock
	msgbox Route102_Text_CatcherBoy_NotEnoughPokemon, MSGBOX_DEFAULT
	closemessage
	release
	end
Route102_EventScript_CatcherBoy_WrongPokemon::
	lock
	msgbox Route102_Text_CatcherBoy_WrongPokemon, MSGBOX_DEFAULT
	closemessage
	release
	end
Route102_EventScript_CatcherBoy_Close::
	setobjectmovementtype LOCALID_CATCHERBOY, MOVEMENT_TYPE_WANDER_AROUND
	closemessage
	release
	end
Route102_EventScript_CatcherBoy_SpeciesCheck::
	call_if_eq VAR_BIRCH_STATE, 0, Route102_EventScript_CatcherBoy_LandEncounter
	call_if_eq VAR_BIRCH_STATE, 1, Route102_EventScript_CatcherBoy_LandEncounter
	call_if_eq VAR_BIRCH_STATE, 2, Route102_EventScript_CatcherBoy_FishingEncounter
	call_if_eq VAR_BIRCH_STATE, 3, Route102_EventScript_CatcherBoy_LandEncounter
	call_if_eq VAR_BIRCH_STATE, 4, Route102_EventScript_CatcherBoy_WaterEncounter
	call_if_eq VAR_BIRCH_STATE, 5, Route102_EventScript_CatcherBoy_LandEncounter
	call_if_eq VAR_BIRCH_STATE, 6, Route102_EventScript_CatcherBoy_LandEncounter
	return
Route102_EventScript_CatcherBoy_LandEncounter::
	setvar VAR_0x8006, 11
	specialvar VAR_0x8005, GetRandomEncounterSlot
	setvar VAR_0x8004, 0xFFFF
	specialvar VAR_RESULT, GetLandEncounter
	copyvar VAR_TEMP_1, VAR_RESULT
	return
Route102_EventScript_CatcherBoy_WaterEncounter::
	setvar VAR_0x8006, 5
	specialvar VAR_0x8005, GetRandomEncounterSlot
	setvar VAR_0x8004, 0xFFFF
	specialvar VAR_RESULT, GetWaterEncounter
	copyvar VAR_TEMP_1, VAR_RESULT
	return
Route102_EventScript_CatcherBoy_FishingEncounter::
	setvar VAR_0x8006, 10
	specialvar VAR_0x8005, GetRandomEncounterSlot
	setvar VAR_0x8004, 0xFFFF
	specialvar VAR_RESULT, GetFishingEncounter
	copyvar VAR_TEMP_1, VAR_RESULT
	return
Route102_EventScript_CatcherBoy_Emote::
	goto_if_set CATCHERBOY_EMOTE, Route102_EventScript_CatcherBoy_EmoteSet
	goto_if_set FLAG_DAILY_ROUTE102_CATCHERBOY, Route102_EventScript_CatcherBoy_EmoteSet
	setvar VAR_0x8004, LOCALID_CATCHERBOY
	special NpcFacePlayerEmoteQuest
	playse SE_PIN
	setflag CATCHERBOY_EMOTE
	release
	end
Route102_EventScript_CatcherBoy_EmoteSet::
	release
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

Route102_Text_CatcherBoyDefault:
	.string "I'm going to catch a whole bunch of\n"
	.string "POKEMON!$"
Route102_Text_CatcherBoy_WannaHelp:
	.string "I'm going to catch a whole bunch of\n"
	.string "{STR_VAR_1}!\p"
	.string "Wanna help? If you manage to catch any\n"
	.string "{STR_VAR_1}, I'll take it off your hands.$"
Route102_Text_CatcherBoy_Helped:
	.string "Your help today really motivates me...\p"
	.string "I'm going to catch even more\n"
	.string "POKEMON!$"
Route102_Text_CatcherBoy_WrongPokemon:
	.string "No, thanks. That's not the\n"
	.string "POKEMON I'm looking for today.$"
Route102_Text_CatcherBoy_NotEnoughPokemon:
	.string "If you only have one POKEMON with you,\n"
	.string "then you should keep it.\p"
	.string "A trainer always need a POKEMON\n"
	.string "in their party for protection.$"

Route102_Text_IPGain:
	.string "You gained 1 IP!$"
Route102_Text_ReceiveItem:
    .string "You got one {STR_VAR_3}!$"

Route102_Text_RouteSignOldale:
	.string "ROUTE 102\n"
	.string "{RIGHT_ARROW} OLDALE TOWN$"

Route102_Text_RouteSignPetalburg:
	.string "ROUTE 102\n"
	.string "{LEFT_ARROW} PETALBURG CITY$"

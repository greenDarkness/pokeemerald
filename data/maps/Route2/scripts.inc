.equ FLAG_HIDE_SPRITES_MORNING,   			FLAG_TEMP_A
.equ FLAG_HIDE_SPRITES_DAY,   				FLAG_TEMP_B
.equ FLAG_HIDE_SPRITES_NIGHT,   			FLAG_TEMP_C
.equ FLAG_HIDE_SPRITES_NIGHT_AND_MORNING,	FLAG_TEMP_D
.equ FLAG_HIDE_SPRITES_MORNING_AND_DAY,		FLAG_TEMP_E
.equ FLAG_HIDE_SPRITES_DAY_AND_NIGHT,		FLAG_TEMP_F
.equ ROUTE2_CATCHERBOY_EMOTE,               FLAG_TEMP_2


Route2_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route2_OnTransition
	.byte 0

Route2_OnTransition:
	call Route2_EventScript_ShowPokemon
	call_if_unset FLAG_DAILY_ROUTE2_CATCHERBOY, Route2_EventScript_CatcherBoy_LookAround
	end

Route2_EventScript_CatcherBoy_LookAround::
	setobjectmovementtype LOCALID_ROUTE2_CATCHERBOY, MOVEMENT_TYPE_LOOK_AROUND
	return

Route2_EventScript_ShowPokemon::
	call Route2_EventScript_HidePokemon
	specialvar VAR_RESULT, GetScriptTimeOfDay
    goto_if_eq VAR_RESULT, SCRIPT_TIME_MORNING, Route2_EventScript_ShowMorningPokemon
	goto_if_eq VAR_RESULT, SCRIPT_TIME_DAY, Route2_EventScript_ShowDayPokemon
	goto_if_eq VAR_RESULT, SCRIPT_TIME_NIGHT, Route2_EventScript_ShowNightPokemon
	return
Route2_EventScript_ShowMorningPokemon::
	clearflag FLAG_HIDE_SPRITES_MORNING
	clearflag FLAG_HIDE_SPRITES_MORNING_AND_DAY
	clearflag FLAG_HIDE_SPRITES_NIGHT_AND_MORNING
	return
Route2_EventScript_ShowDayPokemon::
	clearflag FLAG_HIDE_SPRITES_DAY
	clearflag FLAG_HIDE_SPRITES_MORNING_AND_DAY
	clearflag FLAG_HIDE_SPRITES_DAY_AND_NIGHT
	return
Route2_EventScript_ShowNightPokemon::
	clearflag FLAG_HIDE_SPRITES_NIGHT
	clearflag FLAG_HIDE_SPRITES_DAY_AND_NIGHT
	clearflag FLAG_HIDE_SPRITES_NIGHT_AND_MORNING
	return
Route2_EventScript_HidePokemon::
	setflag FLAG_HIDE_SPRITES_MORNING
	setflag FLAG_HIDE_SPRITES_DAY
	setflag FLAG_HIDE_SPRITES_NIGHT
	setflag FLAG_HIDE_SPRITES_NIGHT_AND_MORNING
	setflag FLAG_HIDE_SPRITES_MORNING_AND_DAY
	setflag FLAG_HIDE_SPRITES_DAY_AND_NIGHT
	return

Route2_EventScript_RouteSign::
	msgbox Route2_Text_RouteSign, MSGBOX_SIGN
	end

Route2_EventScript_DiglettsCaveSign::
	msgbox Route2_Text_DiglettsCave, MSGBOX_SIGN
	end

Route2_Text_RouteSign::
    .string "ROUTE 2\n"
    .string "VIRIDIAN CITY - PEWTER CITY$"

Route2_Text_DiglettsCave::
    .string "DIGLETT'S CAVE$"

Route2_EventScript_CatcherBoy::
	lock
	faceplayer
	goto_if_set FLAG_DAILY_ROUTE2_CATCHERBOY, Route2_EventScript_CatcherBoy_Default
	call Route2_EventScript_CatcherBoy_SpeciesCheck
	msgbox Route2_Text_CatcherBoy_WannaHelp, MSGBOX_YESNO
	closemessage
	goto_if_eq VAR_RESULT, YES, Route2_EventScript_CatcherBoy_WannaHelp
	setobjectmovementtype LOCALID_ROUTE2_CATCHERBOY, MOVEMENT_TYPE_WANDER_AROUND
	release
	end
Route2_EventScript_CatcherBoy_WannaHelp::
	specialvar VAR_RESULT, CountPartyNonEggMons
	goto_if_eq VAR_RESULT, 1, Route2_EventScript_CatcherBoy_NotEnoughPokemon
	copyvar VAR_0x8004, VAR_TEMP_1
	specialvar VAR_RESULT, GetReleaseMonSpeciesInfo
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x8005, VAR_0x8004
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, Route2_EventScript_CatcherBoy_Close
	specialvar VAR_RESULT, GetReleaseMonSpecies
	goto_if_ne VAR_RESULT, VAR_TEMP_1, Route2_EventScript_CatcherBoy_WrongPokemon
	special ReleaseMonFromParty
	setflag FLAG_DAILY_ROUTE2_CATCHERBOY
	lock
	faceplayer
	msgbox Route2_Text_CatcherBoy_Helped, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, GAME_STAT_PLAYER_IP_MAX
	specialvar VAR_RESULT, GetGameStatValue
	goto_if_lt VAR_RESULT, 100, Route2_EventScript_CatcherBoy_GainIP
	special GiveGroundMomItem
    msgbox Route2_Text_ReceiveItem, MSGBOX_DEFAULT
    waitfanfare
    closemessage
	setobjectmovementtype LOCALID_ROUTE2_CATCHERBOY, MOVEMENT_TYPE_WANDER_AROUND
	release
	end
Route2_EventScript_CatcherBoy_GainIP::
	setvar VAR_0x8000, 1
	callnative IncreasePlayerIP
	playfanfare MUS_OBTAIN_SYMBOL
	msgbox Route2_Text_IPGain, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	setobjectmovementtype LOCALID_ROUTE2_CATCHERBOY, MOVEMENT_TYPE_WANDER_AROUND
	release
	end
Route2_EventScript_CatcherBoy_Default::
	msgbox Route2_Text_CatcherBoyDefault, MSGBOX_DEFAULT
	closemessage
	release
	end
Route2_EventScript_CatcherBoy_NotEnoughPokemon::
	lock
	msgbox Route2_Text_CatcherBoy_NotEnoughPokemon, MSGBOX_DEFAULT
	closemessage
	release
	end
Route2_EventScript_CatcherBoy_WrongPokemon::
	lock
	msgbox Route2_Text_CatcherBoy_WrongPokemon, MSGBOX_DEFAULT
	closemessage
	release
	end
Route2_EventScript_CatcherBoy_Close::
	setobjectmovementtype LOCALID_ROUTE2_CATCHERBOY, MOVEMENT_TYPE_WANDER_AROUND
	closemessage
	release
	end
Route2_EventScript_CatcherBoy_SpeciesCheck::
	call_if_eq VAR_BIRCH_STATE, 0, Route2_EventScript_CatcherBoy_LandEncounter
	call_if_eq VAR_BIRCH_STATE, 1, Route2_EventScript_CatcherBoy_LandEncounter
	call_if_eq VAR_BIRCH_STATE, 2, Route2_EventScript_CatcherBoy_FishingEncounter
	call_if_eq VAR_BIRCH_STATE, 3, Route2_EventScript_CatcherBoy_LandEncounter
	call_if_eq VAR_BIRCH_STATE, 4, Route2_EventScript_CatcherBoy_WaterEncounter
	call_if_eq VAR_BIRCH_STATE, 5, Route2_EventScript_CatcherBoy_LandEncounter
	call_if_eq VAR_BIRCH_STATE, 6, Route2_EventScript_CatcherBoy_LandEncounter
	return
Route2_EventScript_CatcherBoy_LandEncounter::
	setvar VAR_0x8006, 11
	specialvar VAR_0x8005, GetRandomEncounterSlot
	setvar VAR_0x8004, 0xFFFF
	specialvar VAR_RESULT, GetLandEncounter
	copyvar VAR_TEMP_1, VAR_RESULT
	return
Route2_EventScript_CatcherBoy_WaterEncounter::
	setvar VAR_0x8006, 5
	specialvar VAR_0x8005, GetRandomEncounterSlot
	setvar VAR_0x8004, 0xFFFF
	specialvar VAR_RESULT, GetWaterEncounter
	copyvar VAR_TEMP_1, VAR_RESULT
	return
Route2_EventScript_CatcherBoy_FishingEncounter::
	setvar VAR_0x8006, 10
	specialvar VAR_0x8005, GetRandomEncounterSlot
	setvar VAR_0x8004, 0xFFFF
	specialvar VAR_RESULT, GetFishingEncounter
	copyvar VAR_TEMP_1, VAR_RESULT
	return
Route2_EventScript_CatcherBoy_Emote::
	goto_if_set ROUTE2_CATCHERBOY_EMOTE, Route2_EventScript_CatcherBoy_EmoteSet
	goto_if_set FLAG_DAILY_ROUTE2_CATCHERBOY, Route2_EventScript_CatcherBoy_EmoteSet
	setvar VAR_0x8004, LOCALID_ROUTE2_CATCHERBOY
	special NpcFacePlayerAndExclaim
	playse SE_PIN
	setflag ROUTE2_CATCHERBOY_EMOTE
	release
	end
Route2_EventScript_CatcherBoy_EmoteSet::
	release
	end

Route2_Text_CatcherBoy_WannaHelp:
	.string "Hey! Could you help me out?\p"
	.string "I'm trying to fill out my POKéDEX,\n"
	.string "and I just need one more {STR_VAR_1}!\p"
	.string "Would you be willing to release one\n"
	.string "of yours so I can catch it?$"

Route2_Text_CatcherBoy_NotEnoughPokemon:
	.string "Aw, you've only got one POKéMON.\p"
	.string "You shouldn't be giving any away!$"

Route2_Text_CatcherBoy_WrongPokemon:
	.string "Hey, that's not a {STR_VAR_1}!\n"
	.string "Maybe come back when you've got one.$"

Route2_Text_CatcherBoy_Helped:
	.string "Whoa, thanks a ton!\p"
	.string "I'll find that {STR_VAR_1} in no\n"
	.string "time!$"

Route2_Text_CatcherBoyDefault:
	.string "Thanks again for the help!\p"
	.string "Come back tomorrow if you're feeling\n"
	.string "generous.$"

Route2_Text_ReceiveItem:
	.string "Here, take this for your trouble!$"

Route2_Text_IPGain:
	.string "Your IP went up!$"


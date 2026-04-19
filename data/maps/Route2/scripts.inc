.equ FLAG_HIDE_SPRITES_MORNING,   			FLAG_TEMP_A
.equ FLAG_HIDE_SPRITES_DAY,   				FLAG_TEMP_B
.equ FLAG_HIDE_SPRITES_NIGHT,   			FLAG_TEMP_C
.equ FLAG_HIDE_SPRITES_NIGHT_AND_MORNING,	FLAG_TEMP_D
.equ FLAG_HIDE_SPRITES_MORNING_AND_DAY,		FLAG_TEMP_E
.equ FLAG_HIDE_SPRITES_DAY_AND_NIGHT,		FLAG_TEMP_F


Route2_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Route2_OnTransition
	.byte 0

Route2_OnTransition:
	call Route2_EventScript_ShowPokemon
	end

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


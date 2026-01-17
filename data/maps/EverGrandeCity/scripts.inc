EverGrandeCity_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, EverGrandeCity_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, EverGrandeCity_OnLoad
	.byte 0

EverGrandeCity_OnTransition:
	call_if_set FLAG_SYS_WEATHER_CTRL, Common_EventScript_SetAbnormalWeather
	end

EverGrandeCity_OnLoad:
	call_if_set FLAG_HIDE_EVERGRANDE_BIKEPATH, EverGrandeCity_EventScript_HideBikePath
	end

EverGrandeCity_EventScript_VictoryRoadSign::
	msgbox EverGrandeCity_Text_EnteringVictoryRoad, MSGBOX_SIGN
	end

EverGrandeCity_EventScript_CitySign::
	msgbox EverGrandeCity_Text_CitySign, MSGBOX_SIGN
	end

EverGrandeCity_EventScript_PokemonLeagueSign::
	msgbox EverGrandeCity_Text_EnteringPokemonLeague, MSGBOX_SIGN
	end

EverGrandeCity_EventScript_SetVisitedEverGrande::
	setflag FLAG_VISITED_EVER_GRANDE_CITY
	setvar VAR_TEMP_1, 1
	end
	
EverGrandeCity_EventScript_HideBikePath::
	setmetatile 29, 55, 0x075, TRUE
	setmetatile 30, 55, 0x075, TRUE
	setmetatile 31, 55, 0x075, TRUE
	end
EverGrandeCity_EventScript_BikePathAthlete::
	goto_if_unset FLAG_HIDE_EVERGRANDE_BIKEPATH, EverGrandeCity_EventScript_BikePathAthletePost
	msgbox EverGrandeCity_Text_BikePathAthlete, MSGBOX_DEFAULT
	closemessage
	release
	end
EverGrandeCity_EventScript_BikePathAthletePost::
	msgbox EverGrandeCity_Text_BikePathAthletePost, MSGBOX_DEFAULT
	closemessage
	release
	end

EverGrandeCity_Text_EnteringVictoryRoad:
	.string "ENTERING VICTORY ROAD$"

EverGrandeCity_Text_EnteringPokemonLeague:
	.string "ENTERING POKéMON LEAGUE\n"
	.string "CENTER GATE$"

EverGrandeCity_Text_CitySign:
	.string "EVER GRANDE CITY\p"
	.string "“The paradise of flowers, the sea,\n"
	.string "and POKéMON.”$"

EverGrandeCity_Text_BikePathAthlete:
	.string "EVERGRANDE has great terrain for\n"
	.string "mountain biking.\p"
	.string "I'm working on making a ramp to\n"
	.string "the top. Once I'm finished,\l"
	.string "This city will really be hoppin'!$"

EverGrandeCity_Text_BikePathAthletePost:
	.string "That's a pretty nice bike ramp, huh?\p"
	.string "No need to thank me, I built it for\n"
	.string "the thrills!$"

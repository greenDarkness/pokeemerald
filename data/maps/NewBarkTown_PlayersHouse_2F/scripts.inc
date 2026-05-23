NewBarkTown_PlayersHouse_2F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, NewBarkTown_PlayersHouse_2F_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, NewBarkTown_PlayersHouse_2F_OnFrame
	.byte 0

NewBarkTown_PlayersHouse_2F_OnTransition::
	call_if_unset FLAG_NEWBARK_ARRIVED, NewBarkTown_PlayersHouse_2F_FirstWarpIn
	.2byte 0

NewBarkTown_PlayersHouse_2F_OnFrame::
	map_script_2 VAR_NEWBARK_TOWN_STATE, 1, NewBarkTown_PlayersHouse_2F_Script_ClockSetupTrigger
	.2byte 0

NewBarkTown_PlayersHouse_2F_FirstWarpIn::
	setflag FLAG_HIDE_NEWBARK_TOWN_PLAYERS_HOUSE_VIGOROTH
	setflag FLAG_HIDDEN_ITEM_ARIANAS_HOUSE_NEST_BALL
	setrespawn HEAL_LOCATION_NEW_BARK_TOWN
	end

NewBarkTown_PlayersHouse_2F_Script_ClockSetupTrigger::
	goto_if_set FLAG_SET_WALL_CLOCK, NewBarkTown_PlayersHouse_2F_EventScript_End
    lock
	playse SE_PIN
	applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 60
    applymovement OBJ_EVENT_ID_PLAYER, NewBarkTown_PlayersHouse_2F_Movement_PlayerWalkToClock
	waitmovement 0
    msgbox PlayersHouse_2F_Text_ClockIsStopped, MSGBOX_DEFAULT
	checkplayergender
	call_if_eq VAR_RESULT, MALE, NewBarkTown_PlayersHouse_2F_Script_SetMaleGender
	call_if_eq VAR_RESULT, FEMALE, NewBarkTown_PlayersHouse_2F_Script_SetFemaleGender
	call PlayersHouse_2F_EventScript_SetWallClock
	delay 30
	setvar VAR_NEWBARK_TOWN_STATE, 2
	setflag FLAG_SET_WALL_CLOCK
	releaseall
	end

NewBarkTown_PlayersHouse_2F_Movement_PlayerWalkToClock:
	walk_right
	face_up
	delay_8
	step_end

NewBarkTown_PlayersHouse_2F_Script_SetMaleGender::
	setvar VAR_0x8004, MALE
	return
NewBarkTown_PlayersHouse_2F_Script_SetFemaleGender::
	setvar VAR_0x8004, FEMALE
	return

NewBarkTown_PlayersHouse_2F_Script_SetPlayerGender::
	checkplayergender

NewBarkTown_PlayersHouse_2F_EventScript_NES::
	msgbox NewBarkTown_PlayersHouse_2F_Text_NES, MSGBOX_SIGN
	end

NewBarkTown_PlayersHouse_2F_EventScript_End::
	release
	end

NewBarkTown_PlayersHouse_2F_Text_NES:
    .string "{PLAYER} played with the NES.\p"
    .string "…Okay!\n"
    .string "It's time to go!$"

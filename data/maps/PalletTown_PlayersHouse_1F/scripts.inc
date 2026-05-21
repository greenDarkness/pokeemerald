PalletTown_PlayersHouse_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_PlayersHouse_1F_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, PalletTown_PlayersHouse_1F_OnLoad
	map_script MAP_SCRIPT_ON_FRAME_TABLE, PalletTown_PlayersHouse_1F_OnFrame
	.byte 0

PalletTown_PlayersHouse_1F_OnTransition:
	call_if_eq VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 1, PalletTown_PlayersHouse_1F_EventScript_SetDeliaPos
	end

PalletTown_PlayersHouse_1F_OnLoad:
	call_if_eq VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 1, PalletTown_PlayersHouse_1F_ShowMovingBoxes
	end

PalletTown_PlayersHouse_1F_OnFrame:
	map_script_2 VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 1, PalletTown_PlayersHouse_1F_EventScript_EnterHouseMovingIn
	.2byte 0

PalletTown_PlayersHouse_1F_ShowMovingBoxes::
	setmetatile 11, 4, 0x181, TRUE
	setmetatile 12, 3, 0x181, TRUE
	setmetatile 12, 4, 0x186, TRUE
	return

PalletTown_PlayersHouse_1F_EventScript_EnterHouseMovingIn::
	delay 32
	applymovement LOCALID_PLAYER, PalletTown_PlayersHouse_1F_Movement_FaceRight
	waitmovement 0
	delay 16
	applymovement LOCALID_PLAYER, PalletTown_PlayersHouse_1F_Movement_WalkRight
	waitmovement 0
	delay 16
	applymovement LOCALID_DELIA_PLAYERS_HOUSE_1F, PalletTown_PlayersHouse_1F_Movement_WalkRight
	waitmovement 0
	delay 32
	applymovement LOCALID_PLAYER, PalletTown_PlayersHouse_1F_Movement_FaceUp
	waitmovement 0
	delay 16
	msgbox PalletTown_PlayersHouse_1F_Text_RememberThisPlace, MSGBOX_DEFAULT
	waitmessage
	closemessage
	delay 16
	applymovement LOCALID_PLAYER, PalletTown_PlayersHouse_1F_Movement_FaceLeft
	waitmovement 0
	delay 16
	msgbox PalletTown_PlayersHouse_1F_Text_CheckOutYourRoom, MSGBOX_DEFAULT
	closemessage
	setvar VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 2
	release
	end

PalletTown_PlayersHouse_1F_EventScript_SetDeliaPos::
	setobjectxyperm LOCALID_DELIA_PLAYERS_HOUSE_1F, 3, 8
	setobjectmovementtype LOCALID_DELIA_PLAYERS_HOUSE_1F, MOVEMENT_TYPE_FACE_RIGHT
	return

PalletTown_PlayersHouse_1F_Movement_WalkRight:
	walk_right
	step_end
PalletTown_PlayersHouse_1F_Movement_FaceLeft:
	face_left
	step_end
PalletTown_PlayersHouse_1F_Movement_FaceRight:
	face_right
	step_end
PalletTown_PlayersHouse_1F_Movement_FaceUp:
	face_up
	step_end

PalletTown_PlayersHouse_1F_EventScript_Delia::
	lock
	faceplayer
	goto_if_lt VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 3, PalletTown_PlayersHouse_1F_EventScript_IsntItNiceInHere_DeliaContinued
	goto_if_set FLAG_BEAT_RIVAL_IN_OAKS_LAB, PalletTown_PlayersHouse_1F_EventScript_DeliaHeal
	checkplayergender
	call_if_eq VAR_RESULT, MALE, PalletTown_PlayersHouse_1F_EventScript_DeliaOakLookingForYouMale
	call_if_eq VAR_RESULT, FEMALE, PalletTown_PlayersHouse_1F_EventScript_DeliaOakLookingForYouFemale
	closemessage
	applymovement LOCALID_DELIA_PLAYERS_HOUSE_1F, Common_Movement_FaceOriginalDirection
	waitmovement 0
	release
	end

PalletTown_PlayersHouse_1F_EventScript_IsntItNiceInHere_DeliaContinued::
	msgbox PalletTown_PlayersHouse_1F_Text_CheckOutYourRoom, MSGBOX_DEFAULT
	closemessage
	release
	End

PalletTown_PlayersHouse_1F_EventScript_DeliaOakLookingForYouMale::
	msgbox PalletTown_PlayersHouse_1F_Text_AllBoysLeaveOakLookingForYou
	return

PalletTown_PlayersHouse_1F_EventScript_DeliaOakLookingForYouFemale::
	msgbox PalletTown_PlayersHouse_1F_Text_AllGirlsLeaveOakLookingForYou
	return

PalletTown_PlayersHouse_1F_EventScript_DeliaHeal::
	msgbox PalletTown_PlayersHouse_1F_Text_YouShouldTakeQuickRest
	closemessage
	call Common_EventScript_OutOfCenterPartyHeal
	incrementgamestat GAME_STAT_RESTED_AT_HOME
	msgbox PalletTown_PlayersHouse_1F_Text_LookingGreatTakeCare
	release
	end

@ Displays special text if interacted with from side or back (which are normally inaccessible)
PalletTown_PlayersHouse_1F_EventScript_TV::
	lockall
	goto_if_eq VAR_FACING, DIR_NORTH, PalletTown_PlayersHouse_1F_EventScript_TVScreen
	msgbox PalletTown_PlayersHouse_1F_Text_OopsWrongSide
	releaseall
	end

PalletTown_PlayersHouse_1F_EventScript_TVScreen::
	checkplayergender
	call_if_eq VAR_RESULT, MALE, PalletTown_PlayersHouse_1F_EventScript_TVScreenMale
	call_if_eq VAR_RESULT, FEMALE, PalletTown_PlayersHouse_1F_EventScript_TVScreenFemale
	releaseall
	end

PalletTown_PlayersHouse_1F_EventScript_TVScreenMale::
	msgbox PalletTown_PlayersHouse_1F_Text_MovieOnTVFourBoysOnRailroad
	return

PalletTown_PlayersHouse_1F_EventScript_TVScreenFemale::
	msgbox PalletTown_PlayersHouse_1F_Text_MovieOnTVGirlOnBrickRoad
	return

PalletTown_PlayersHouse_1F_Text_AllBoysLeaveOakLookingForYou::
    .string "DELIA: …Right.\n"
    .string "All boys leave home someday.\l"
    .string "It said so on TV.\p"
    .string "Oh, yes. PROF. OAK, next door, was\n"
    .string "looking for you.$"

PalletTown_PlayersHouse_1F_Text_AllGirlsLeaveOakLookingForYou::
    .string "DELIA: …Right.\n"
    .string "All girls dream of traveling.\l"
    .string "It said so on TV.\p"
    .string "Oh, yes. PROF. OAK, next door, was\n"
    .string "looking for you.$"

PalletTown_PlayersHouse_1F_Text_YouShouldTakeQuickRest::
    .string "DELIA: {PLAYER}!\n"
    .string "You should take a quick rest.$"

PalletTown_PlayersHouse_1F_Text_LookingGreatTakeCare::
    .string "DELIA: Oh, good! You and your\n"
    .string "POKéMON are looking great.\l"
    .string "Take care now!$"

PalletTown_PlayersHouse_1F_Text_MovieOnTVFourBoysOnRailroad::
    .string "There's a movie on TV.\n"
    .string "Four boys are walking on railroad\l"
    .string "tracks.\p"
    .string "…I better go, too.$"

PalletTown_PlayersHouse_1F_Text_MovieOnTVGirlOnBrickRoad::
    .string "There's a movie on TV.\n"
    .string "A girl with her hair in pigtails is\l"
    .string "walking up a brick road.\p"
    .string "…I better go, too.$"

PalletTown_PlayersHouse_1F_Text_OopsWrongSide::
    .string "Oops, wrong side…$"

PalletTown_PlayersHouse_1F_Text_RememberThisPlace::
	.string "DELIA: So, {PLAYER}?\n"
	.string "Do you remember this place? You used\l"
	.string "to stay here when you were younger.$"
PalletTown_PlayersHouse_1F_Text_CheckOutYourRoom::
	.string "DELIA: The mover's POKéMON will do\n"
	.string "all the work of moving you in and\l"
	.string "cleaning up after.\p"
	.string "{PLAYER}, your room is upstairs.\n"
	.string "Go check it out!$"

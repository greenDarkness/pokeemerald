.set LOCALID_ARIANA, 1
.set LOCALID_ARIANAS_FRIEND, 2

NewBarkTown_PlayersHouse_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, NewBarkTown_PlayersHouse_1F_OnTransition
	map_script MAP_SCRIPT_ON_LOAD, NewBarkTown_PlayersHouse_1F_OnLoad
	map_script MAP_SCRIPT_ON_FRAME_TABLE, NewBarkTown_PlayersHouse_1F_OnFrame
	.byte 0

NewBarkTown_PlayersHouse_1F_OnTransition::
	call_if_eq VAR_NEWBARK_TOWN_STATE, 0, NewBarkTown_PlayersHouse_1F_EventScript_SetArianaPos
	call_if_eq VAR_NEWBARK_TOWN_STATE, 1, NewBarkTown_PlayersHouse_1F_SetArianaStandingAtTable
	call_if_eq VAR_NEWBARK_TOWN_STATE, 2, NewBarkTown_PlayersHouse_1F_SetArianaStandingAtTable
	call_if_eq VAR_NEWBARK_TOWN_STATE, 3, NewBarkTown_PlayersHouse_1F_SetArianaStandingAtTable
	end

NewBarkTown_PlayersHouse_1F_OnLoad:
	call_if_eq VAR_NEWBARK_TOWN_STATE, 0, NewBarkTown_PlayersHouse_1F_ShowMovingBoxes
	end

NewBarkTown_PlayersHouse_1F_OnFrame:
	map_script_2 VAR_NEWBARK_TOWN_STATE, 0, NewBarkTown_PlayersHouse_1F_EventScript_EnterHouseMovingIn
	.2byte 0

NewBarkTown_PlayersHouse_1F_EventScript_SetArianaPos::
	setobjectxyperm LOCALID_ARIANA, 10, 8
	setobjectmovementtype LOCALID_ARIANA, MOVEMENT_TYPE_FACE_LEFT
	return

NewBarkTown_PlayersHouse_1F_ShowMovingBoxes::
	setmetatile 11, 3, 0x09A, TRUE
	setmetatile 12, 3, 0x1D3, TRUE
	setmetatile 12, 4, 0x1DB, TRUE
	return

NewBarkTown_PlayersHouse_1F_SetArianaStandingAtTable::
	setobjectxyperm LOCALID_ARIANA, 8, 4
	setobjectmovementtype LOCALID_ARIANA, MOVEMENT_TYPE_FACE_LEFT

	@add to newgame later
	clearflag FLAG_HIDE_ARIANAS_FRIEND
	clearflag FLAG_ARIANA_VISITED
	return

NewBarkTown_PlayersHouse_1F_EventScript_EnterHouseMovingIn::
	delay 32
	applymovement LOCALID_PLAYER, NewBarkTown_PlayersHouse_1F_Movement_FaceLeft
	waitmovement 0
	delay 16
	applymovement LOCALID_PLAYER, NewBarkTown_PlayersHouse_1F_Movement_WalkLeft
	waitmovement 0
	delay 16
	applymovement LOCALID_ARIANA, NewBarkTown_PlayersHouse_1F_Movement_WalkLeft
	waitmovement 0
	delay 32
	applymovement LOCALID_PLAYER, NewBarkTown_PlayersHouse_1F_Movement_FaceUp
	waitmovement 0
	delay 16
	msgbox NewBarkTown_PlayersHouse_1F_Text_RememberThisPlace, MSGBOX_DEFAULT
	waitmessage
	closemessage
	delay 16
	applymovement LOCALID_PLAYER, NewBarkTown_PlayersHouse_1F_Movement_FaceRight
	waitmovement 0
	delay 16
	msgbox NewBarkTown_PlayersHouse_1F_Text_CheckOutYourRoom, MSGBOX_DEFAULT
	closemessage
	setvar VAR_NEWBARK_TOWN_STATE, 1
	release
	end

NewBarkTown_PlayersHouse_1F_EventScript_Ariana::
	lock
	faceplayer
	goto_if_unset FLAG_NEWBARK_ARRIVED, NewBarkTown_PlayersHouse_1F_EventScript_Ariana_Welcome
	goto_if_set FLAG_ARIANA_VISITED, NewBarkTown_PlayersHouse_1F_EventScript_Ariana_Regular
	goto_if_set FLAG_ADVENTURE_STARTED, NewBarkTown_PlayersHouse_1F_ArianaEventScript_GoodLuck
	msgbox NewBarkTown_PlayersHouse_1F_Text_ArianaHurryUpElmIsWaiting, MSGBOX_DEFAULT
	applymovement LOCALID_ARIANA, NewBarkTown_PlayersHouse_1F_MovementFaceLeft
	waitmovement 0
	release
	end

NewBarkTown_PlayersHouse_1F_EventScript_Ariana_Welcome::
	lock
	faceplayer
	msgbox NewBarkTown_PlayersHouse_1F_Text_CheckOutYourRoom, MSGBOX_DEFAULT
	closemessage
	release
	end

NewBarkTown_PlayersHouse_1F_EventScript_ArianasFriend::
	lock
	faceplayer
	msgbox NewBarkTown_PlayersHouse_1F_Text_ArianasFriend, MSGBOX_DEFAULT
	applymovement LOCALID_ARIANAS_FRIEND, NewBarkTown_PlayersHouse_1F_MovementFaceRight
	waitmovement 0
	release
	end

NewBarkTown_PlayersHouse_1F_ArianaEventScript_GoodLuck::
	msgbox NewBarkTown_PlayersHouse_1F_Text_ArianaLeaving, MSGBOX_DEFAULT
	setflag FLAG_ARIANA_VISITED
	clearflag FLAG_HIDE_ROUTE_30_NPCS
	setvar VAR_NEWBARK_TOWN_STATE, 5
	release
	end


NewBarkTown_PlayersHouse_1F_EventScript_Ariana_Regular::
	call PlayersHouse_1F_EventScript_MomHealsParty
	release
	end

NewBarkTown_PlayersHouse_1F_EventScript_ArianaIntroTrigger1::
	goto_if_unset FLAG_NEWBARK_ARRIVED, NewBarkTown_PlayersHouse_1F_EventScript_End
	lock
	applymovement OBJ_EVENT_ID_PLAYER, NewBarkTown_PlayersHouse_1F_PlayerIntroMovement1
	waitmovement 0
	goto NewBarkTown_PlayersHouse_1F_EventScript_ArianaIntro
	end

NewBarkTown_PlayersHouse_1F_EventScript_ArianaIntroTrigger2::
	goto_if_unset FLAG_NEWBARK_ARRIVED, NewBarkTown_PlayersHouse_1F_EventScript_End
	lock
	applymovement OBJ_EVENT_ID_PLAYER, NewBarkTown_PlayersHouse_1F_PlayerIntroMovement2
	waitmovement 0
	goto NewBarkTown_PlayersHouse_1F_EventScript_ArianaIntro
	end


NewBarkTown_PlayersHouse_1F_EventScript_ArianaIntro::
	playse SE_PIN
	applymovement LOCALID_ARIANA, Common_Movement_ExclamationMark
	delay 60
	applymovement LOCALID_ARIANA, NewBarkTown_PlayersHouse_1F_ArianaMovement_1
	waitmovement 0
	msgbox NewBarkTown_PlayersHouse_1F_Text_ElmsLookingForYou, MSGBOX_DEFAULT
	call NewBarkTown_PlayersHouse_1F_EventScript_GivePokeGear
	applymovement LOCALID_ARIANA, NewBarkTown_PlayersHouse_1F_ArianaMovement_2
	waitmovement 0
	setvar VAR_NEWBARK_TOWN_STATE, 2
	setvar VAR_NEWBARKTOWN_LABSTATE, 0
	setflag FLAG_HIDE_LAB_POLICEMAN
	setflag FLAG_VISITED_NEWBARK_TOWN

	@former flagheap location

	releaseall
	end



NewBarkTown_PlayersHouse_1F_EventScript_GivePokeGear::
	playfanfare MUS_OBTAIN_ITEM
	message NewBarkTown_PlayersHouse_1F_Text_ObtainedPokeGear
	waitfanfare
	setflag FLAG_SYS_POKENAV_GET
	setflag FLAG_RECEIVED_POKENAV
	setflag FLAG_ADDED_MATCH_CALL_TO_POKENAV
	setflag FLAG_REGISTER_RIVAL_POKENAV
	msgbox NewBarkTown_PlayersHouse_1F_Text_ExplainPokeGear, MSGBOX_DEFAULT
	closemessage
	delay 30
	return
	
NewBarkTown_PlayersHouse_1F_EventScript_GiveRunningShoes::
	msgbox NewBarkTown_PlayersHouse_1F_Text_WearTheseRunningShoes, MSGBOX_DEFAULT
	waitmessage
	closemessage
	playfanfare MUS_OBTAIN_ITEM
	msgbox NewBarkTown_PlayersHouse_1F_Text_SwitchShoesWithRunningShoes, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	setflag FLAG_RECEIVED_RUNNING_SHOES
	setflag FLAG_SYS_B_DASH
	msgbox NewBarkTown_PlayersHouse_1F_Text_ExplainRunningShoes, MSGBOX_DEFAULT
	closemessage
	delay 30
	return

NewBarkTown_PlayersHouse_1F_EventScript_End::
	release
	end

NewBarkTown_PlayersHouse_1F_Movement_WalkLeft:
	walk_left
	step_end
NewBarkTown_PlayersHouse_1F_Movement_FaceLeft:
	face_left
	step_end
NewBarkTown_PlayersHouse_1F_Movement_FaceRight:
	face_right
	step_end
NewBarkTown_PlayersHouse_1F_Movement_FaceUp:
	face_up
	step_end

NewBarkTown_PlayersHouse_1F_PlayerIntroMovement1:
	delay_8
	walk_down
	walk_right
	walk_down
	face_left
	delay_8
	step_end

NewBarkTown_PlayersHouse_1F_PlayerIntroMovement2:
	delay_8
	walk_down
	face_left
	delay_8
	step_end

NewBarkTown_PlayersHouse_1F_ArianaMovement_1:
	face_right
	walk_right
	step_end

NewBarkTown_PlayersHouse_1F_ArianaMovement_2:
	face_left
	walk_left
	delay_8
	step_end

NewBarkTown_PlayersHouse_1F_MovementFaceRight:
	face_right
	step_end

NewBarkTown_PlayersHouse_1F_MovementFaceLeft:
	face_left
	step_end

NewBarkTown_PlayersHouse_1F_Text_ElmsLookingForYou:
	.string "Oh, {PLAYER}…! Our neighbor,\n"
	.string "PROF.ELM, was looking for you.\p"
	.string "He said he wanted you to do\n"
	.string "something for him.\p"
	.string "Oh! I almost forgot! Your POKéGEAR\n"
	.string "is back from the repair shop.\p"
	.string "Here you go!$"

NewBarkTown_PlayersHouse_1F_Text_ObtainedPokeGear:
	.string "{PLAYER} obtained the POKéGEAR.$"

NewBarkTown_PlayersHouse_1F_Text_ExplainPokeGear:
	.string "ARIANA: Phone numbers are stored\n"
	.string "in memory.\p"
	.string "Just choose a name you want to call.\p"
	.string "Gee, isn't that convenient?$"

NewBarkTown_PlayersHouse_1F_Text_WearTheseRunningShoes:
	.string "Oh, and don't forget your\n"
	.string "RUNNING SHOES!$"

NewBarkTown_PlayersHouse_1F_Text_SwitchShoesWithRunningShoes:
	.string "{PLAYER} switched shoes with the\n"
	.string "RUNNING SHOES.$"

NewBarkTown_PlayersHouse_1F_Text_ExplainRunningShoes:
	.string "ARIANA: {PLAYER}, press the B Button\n"
	.string "while wearing these to run fast!$"

NewBarkTown_PlayersHouse_1F_Text_ArianaHurryUpElmIsWaiting:
	.string "ARIANA: PROF.ELM is waiting for you.\p"
	.string "Hurry up, baby!$"

NewBarkTown_PlayersHouse_1F_Text_ArianasFriend:
	.string "Hi, {PLAYER}!\p"
	.string "I'm visiting!\p"
	.string "Have you heard?\p"
	.string "My daughter is adamant about\n"
	.string "becoming PROF.ELM's assistant.\p"
	.string "She really loves POKéMON!$"

NewBarkTown_PlayersHouse_1F_Text_ArianaLeaving:
	.string "ARIANA: Wow, that's a cute POKéMON.\n"
	.string "Where did you get it?\p"
	.string "…\p"
	.string "So, you're leaving on an adventure...\n"
	.string "That does sound thrilling!!\p"
	.string "Just be careful.\p"
	.string "Your POKéMON will keep you safe,\n"
	.string "if you work as a team.\p"
	.string "Now, go on!\n"
	.string "I'm behind you all the way!$"

NewBarkTown_PlayersHouse_1F_Text_RememberThisPlace::
	.string "ARIANA: So, {PLAYER}?\n"
	.string "Do you remember this place? You used\l"
	.string "to stay here when you were younger.$"
NewBarkTown_PlayersHouse_1F_Text_CheckOutYourRoom::
	.string "ARIANA: The mover's POKéMON will do\n"
	.string "all the work of moving you in and\l"
	.string "cleaning up after.\p"
	.string "Your room is upstairs, baby.\n"
	.string "Go check it out!$"

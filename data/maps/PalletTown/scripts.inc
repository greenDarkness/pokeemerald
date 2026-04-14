.equ SIGN_LADY_READY, VAR_TEMP_2

PalletTown_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PalletTown_OnTransition
	map_script MAP_SCRIPT_ON_FRAME_TABLE, PalletTown_OnFrame
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, PalletTown_OnWarp
	.byte 0

PalletTown_OnTransition::
	setworldmapflag FLAG_WORLD_MAP_PALLET_TOWN
	call_if_set FLAG_PALLET_LADY_NOT_BLOCKING_SIGN, PalletTown_EventScript_TryReadySignLady
	call_if_eq VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 0, PalletTown_EventScript_SetSignLadyPos
	call_if_eq VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 1, PalletTown_EventScript_SetSignLadyDone
	end

PalletTown_EventScript_TryReadySignLady::
	goto_if_unset FLAG_OPENED_START_MENU, EventScript_Return
	goto_if_ge VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 1, EventScript_Return
	setvar VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 1
	return

PalletTown_EventScript_SetSignLadyDone::
	setvar VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 2
	return

PalletTown_EventScript_SetSignLadyPos::
	goto_if_set FLAG_PALLET_LADY_NOT_BLOCKING_SIGN, PalletTown_EventScript_MoveSignLadyToRouteEntrance
	setobjectxyperm LOCALID_PALLET_SIGN_LADY, 5, 15
	setobjectmovementtype LOCALID_PALLET_SIGN_LADY, MOVEMENT_TYPE_FACE_UP
	return

PalletTown_EventScript_MoveSignLadyToRouteEntrance::
	setobjectxyperm LOCALID_PALLET_SIGN_LADY, 12, 2
	setobjectmovementtype LOCALID_PALLET_SIGN_LADY, MOVEMENT_TYPE_FACE_DOWN
	setvar SIGN_LADY_READY, TRUE
	return

PalletTown_OnFrame:
	map_script_2 VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 0, PalletTown_EventScript_StepOffTruck
	map_script_2 VAR_MAP_SCENE_PALLET_TOWN_OAK, 2, PalletTown_EventScript_OakRatingScene
	.2byte 0

PalletTown_OnWarp:
	map_script_2 VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 0, PalletTown_EventScript_HidePlayerOnTruck
	.byte 0

PalletTown_EventScript_HidePlayerOnTruck::
	hideobjectat LOCALID_PLAYER, MAP_PALLET_TOWN
	end

PalletTown_EventScript_StepOffTruck::
	setvar VAR_0x8004, 6
	setvar VAR_0x8005, 7
	call PalletTown_EventScript_GoInsideWithDelia
	setflag FLAG_HIDE_PALLET_TOWN_TRUCK
	warpsilent MAP_PALLET_TOWN_PLAYERS_HOUSE_1F, 4, 8
	waitstate
	release
	end
PalletTown_EventScript_GoInsideWithDelia::
	delay 15
	showobjectat LOCALID_PLAYER, MAP_PALLET_TOWN
	playse SE_LEDGE
	applymovement LOCALID_PLAYER, PalletTown_Movement_PlayerStepOffTruck
	waitmovement 0
	opendoor VAR_0x8004, VAR_0x8005
	waitdooranim
	addobject LOCALID_AUNT_DELIA_OUTSIDE
	applymovement LOCALID_AUNT_DELIA_OUTSIDE, PalletTown_Movement_DeliaExitHouse
	waitmovement 0
	closedoor VAR_0x8004, VAR_0x8005
	waitdooranim
	delay 10
	applymovement LOCALID_AUNT_DELIA_OUTSIDE, PalletTown_Movement_DeliaApproachPlayerAtTruck
	waitmovement 0
	msgbox PalletTown_Text_OurNewHomeLetsGoInside, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_AUNT_DELIA_OUTSIDE, PalletTown_Movement_DeliaApproachDoor
	applymovement LOCALID_PLAYER, PalletTown_Movement_PlayerApproachDoor
	waitmovement 0
	opendoor VAR_0x8004, VAR_0x8005
	waitdooranim
	applymovement LOCALID_AUNT_DELIA_OUTSIDE, PalletTown_Movement_DeliaEnterHouse
	applymovement LOCALID_PLAYER, PalletTown_Movement_PlayerEnterHouse
	waitmovement 0
	setflag FLAG_HIDE_AUNT_DELIA_OUTSIDE
	setvar VAR_MAP_SCENE_PALLET_TOWN_MOVING_IN, 1
	hideplayer
	closedoor VAR_0x8004, VAR_0x8005
	waitdooranim
	clearflag FLAG_HIDE_MAP_NAME_POPUP
	return
	
PalletTown_Movement_DeliaExitHouse:
	walk_down
	step_end
PalletTown_Movement_DeliaApproachPlayerAtTruck:
	walk_down
	walk_down
	face_left
	step_end
PalletTown_Movement_DeliaApproachDoor:
	delay_16
	delay_8
	walk_up
	walk_up
	step_end
PalletTown_Movement_DeliaEnterHouse:
	walk_up
	set_invisible
	step_end
PalletTown_Movement_PlayerApproachDoor:
	delay_16
	delay_8
	walk_right
	walk_up
	step_end
PalletTown_Movement_PlayerEnterHouse:
	walk_up
	walk_up
	step_end
PalletTown_Movement_PlayerStepOffTruck:
	jump_right
	delay_16
	delay_16
	delay_16
	step_end

@ Oak approaches player after exiting their house post Elite Four to check if they should receieve the National Dex
PalletTown_EventScript_OakRatingScene::
	lockall
	textcolor NPC_TEXT_COLOR_MALE
	setobjectxyperm LOCALID_PALLET_PROF_OAK, 14, 14
	setobjectmovementtype LOCALID_PALLET_PROF_OAK, MOVEMENT_TYPE_FACE_LEFT
	addobject LOCALID_PALLET_PROF_OAK
	applymovement LOCALID_PALLET_PROF_OAK, PalletTown_Movement_OakWalkToPlayersDoor
	waitmovement 0
	goto_if_unset FLAG_WORLD_MAP_ONE_ISLAND, PalletTown_EventScript_NotBeenToOneIslandYet
	msgbox PalletTown_Text_OakLetMeSeePokedex
	setvar VAR_0x8004, 0
	specialvar VAR_RESULT, GetPokedexCount
	copyvar VAR_0x8008, VAR_0x8005
	copyvar VAR_0x8009, VAR_0x8006
	buffernumberstring STR_VAR_1, VAR_0x8008
	buffernumberstring STR_VAR_2, VAR_0x8009
	goto_if_lt VAR_0x8009, 60, PalletTown_EventScript_NotEnoughMonsForNationalDex
	msgbox PalletTown_Text_CaughtXImpressiveFollowMe
	closemessage
	playbgm MUS_FOLLOW_ME, 0
	applymovement LOCALID_PLAYER, PalletTown_Movement_PlayerWalkToLabFromHouse
	applymovement LOCALID_PALLET_PROF_OAK, PalletTown_Movement_OakWalkToLabFromHouse
	waitmovement 0
	opendoor 16, 13
	waitdooranim
	applymovement LOCALID_PALLET_PROF_OAK, PalletTown_Movement_OakEnterLab
	applymovement LOCALID_PLAYER, PalletTown_Movement_PlayerEnterLab
	waitmovement 0
	closedoor 16, 13
	waitdooranim
	setvar VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 7
	setvar VAR_MAP_SCENE_PALLET_TOWN_OAK, 3
	setflag FLAG_HIDE_OAK_IN_PALLET_TOWN
	warp MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB, 6, 12
	waitstate
	releaseall
	end

PalletTown_EventScript_EndOakRatingScene::
	closemessage
	applymovement LOCALID_PALLET_PROF_OAK, PalletTown_Movement_OakExit
	waitmovement 0
	removeobject LOCALID_PALLET_PROF_OAK
	setvar VAR_MAP_SCENE_PALLET_TOWN_OAK, 3
	releaseall
	end

PalletTown_EventScript_NotEnoughMonsForNationalDex::
	msgbox PalletTown_Text_CaughtXPuttingInHonestEffort
	goto PalletTown_EventScript_EndOakRatingScene
	end

PalletTown_EventScript_NotBeenToOneIslandYet::
	msgbox PalletTown_Text_OakYouEnjoyingTraveling
	goto PalletTown_EventScript_EndOakRatingScene
	end

PalletTown_Movement_OakWalkToPlayersDoor::
	walk_left
	walk_left
	walk_left
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	walk_left
	walk_left
	walk_left
	walk_left
	walk_left
	walk_in_place_faster_up
	step_end

PalletTown_Movement_OakExit::
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	step_end

PalletTown_Movement_OakWalkToLabFromHouse::
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_in_place_faster_up
	step_end

PalletTown_Movement_PlayerWalkToLabFromHouse::
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	walk_right
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	step_end

PalletTown_EventScript_OakTriggerLeft::
	lockall
	setvar VAR_TEMP_1, 0
	goto PalletTown_EventScript_OakTrigger
	end

PalletTown_EventScript_OakTriggerRight::
	lockall
	setvar VAR_TEMP_1, 1
	goto PalletTown_EventScript_OakTrigger
	end

PalletTown_EventScript_OakTrigger::
	famechecker FAMECHECKER_OAK, FCPICKSTATE_COLORED, UpdatePickStateFromSpecialVar8005
	textcolor NPC_TEXT_COLOR_MALE
	delay 30
	playbgm MUS_OAK, 0
	message PalletTown_Text_OakDontGoOut
	waitmessage
	delay 85
	closemessage
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterDown
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_PLAYER, Common_Movement_ExclamationMark
	waitmovement 0
	delay 30
	addobject LOCALID_PALLET_PROF_OAK
	call_if_eq VAR_TEMP_1, 0, PalletTown_EventScript_OakEnterLeft
	call_if_eq VAR_TEMP_1, 1, PalletTown_EventScript_OakEnterRight
	delay 30
	msgbox PalletTown_Text_OakGrassUnsafeNeedMon
	closemessage
	delay 30
	call_if_eq VAR_TEMP_1, 0, PalletTown_EventScript_OakLeadPlayerToLabLeft
	call_if_eq VAR_TEMP_1, 1, PalletTown_EventScript_OakLeadPlayerToLabRight
	opendoor 16, 13
	waitdooranim
	applymovement LOCALID_PALLET_PROF_OAK, PalletTown_Movement_OakEnterLab
	applymovement LOCALID_PLAYER, PalletTown_Movement_PlayerEnterLab
	waitmovement 0
	closedoor 16, 13
	waitdooranim
	setvar VAR_MAP_SCENE_PALLET_TOWN_PROFESSOR_OAKS_LAB, 1
	clearflag FLAG_HIDE_OAK_IN_HIS_LAB
	setvar VAR_MAP_SCENE_PALLET_TOWN_OAK, 1
	setflag FLAG_HIDE_OAK_IN_PALLET_TOWN
	setflag FLAG_DONT_TRANSITION_MUSIC
	warp MAP_PALLET_TOWN_PROFESSOR_OAKS_LAB, 6, 12
	waitstate
	releaseall
	end

PalletTown_EventScript_OakEnterLeft::
	applymovement LOCALID_PALLET_PROF_OAK, PalletTown_Movement_OakEnterLeft
	waitmovement 0
	return

PalletTown_EventScript_OakEnterRight::
	applymovement LOCALID_PALLET_PROF_OAK, PalletTown_Movement_OakEnterRight
	waitmovement 0
	return

PalletTown_EventScript_OakLeadPlayerToLabLeft::
	applymovement LOCALID_PALLET_PROF_OAK, PalletTown_Movement_OakWalkToLabLeft
	applymovement LOCALID_PLAYER, PalletTown_Movement_PlayerWalkToLabLeft
	waitmovement 0
	return

PalletTown_EventScript_OakLeadPlayerToLabRight::
	applymovement LOCALID_PALLET_PROF_OAK, PalletTown_Movement_OakWalkToLabRight
	applymovement LOCALID_PLAYER, PalletTown_Movement_PlayerWalkToLabRight
	waitmovement 0
	return

PalletTown_Movement_OakEnterLeft::
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	step_end

PalletTown_Movement_OakEnterRight::
	walk_right
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	step_end

	.macro walk_to_lab
	walk_left
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_down
	walk_right
	walk_right
	walk_right
	walk_right
	.endm

PalletTown_Movement_OakWalkToLabLeft::
	walk_down
	walk_to_lab
	walk_right
	walk_in_place_faster_up
	step_end

PalletTown_Movement_OakWalkToLabRight::
	walk_down
	walk_left
	walk_to_lab
	walk_right
	walk_in_place_faster_up
	step_end

PalletTown_Movement_OakEnterLab::
	walk_up
	set_invisible
	step_end

PalletTown_Movement_PlayerWalkToLabLeft::
	walk_down
	walk_down
	walk_to_lab
	step_end

PalletTown_Movement_PlayerWalkToLabRight::
	walk_down
	walk_down
	walk_left
	walk_to_lab
	step_end

PalletTown_Movement_PlayerEnterLab::
	walk_right
	walk_up
	set_invisible
	step_end

PalletTown_EventScript_SignLady::
	lock
	goto_if_eq VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 2, PalletTown_EventScript_SignLadyDone
	goto_if_eq VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 1, PalletTown_EventScript_SignLadyJustShowedSign
	goto_if_eq SIGN_LADY_READY, TRUE, PalletTown_EventScript_SignLadyStartShowSign
	goto_if_set FLAG_TEMP_2, PalletTown_EventScript_SignLadyGoReadSign
	msgbox PalletTown_Text_HmmIsThatRight
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_Delay48
	waitmovement 0
	msgbox PalletTown_Text_OhLookLook
	closemessage
	call_if_eq VAR_FACING, DIR_EAST, PalletTown_EventScript_SignLadyMoveOutOfWayRight
	call_if_ne VAR_FACING, DIR_EAST, PalletTown_EventScript_SignLadyMoveOutOfWayLeft
	copyobjectxytoperm LOCALID_PALLET_SIGN_LADY
	setflag FLAG_TEMP_2
	release
	end

PalletTown_EventScript_SignLadyMoveOutOfWayRight::
	applymovement LOCALID_PALLET_SIGN_LADY, PalletTown_Movement_SignLadyMoveOutOfWayRight
	waitmovement 0
	return

PalletTown_EventScript_SignLadyMoveOutOfWayLeft::
	applymovement LOCALID_PALLET_SIGN_LADY, PalletTown_Movement_SignLadyMoveOutOfWayLeft
	waitmovement 0
	return

PalletTown_EventScript_SignLadyDone::
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	msgbox PalletTown_Text_RaisingMonsToo
	release
	end

PalletTown_EventScript_SignLadyGoReadSign::
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	msgbox PalletTown_Text_ReadItReadIt
	release
	end

PalletTown_EventScript_SignLadyJustShowedSign::
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	msgbox PalletTown_Text_SignsAreUsefulArentThey
	release
	end

PalletTown_Movement_SignLadyMoveOutOfWayRight::
	walk_right
	walk_in_place_faster_left
	step_end

PalletTown_Movement_SignLadyMoveOutOfWayLeft::
	walk_left
	walk_in_place_faster_right
	step_end

PalletTown_EventScript_FatMan::
	msgbox PalletTown_Text_CanStoreItemsAndMonsInPC, MSGBOX_NPC
	end

PalletTown_EventScript_OaksLabSign::
	lockall
	famechecker FAMECHECKER_OAK, 0
	msgbox PalletTown_Text_OakPokemonResearchLab
	releaseall
	end

PalletTown_EventScript_PlayersHouseSign::
	msgbox PalletTown_Text_PlayersHouse, MSGBOX_SIGN
	end

PalletTown_EventScript_RivalsHouseSign::
	msgbox PalletTown_Text_RivalsHouse, MSGBOX_SIGN
	end

PalletTown_EventScript_TownSign::
	msgbox PalletTown_Text_TownSign, MSGBOX_SIGN
	end

PalletTown_EventScript_TrainerTips::
	lockall
	msgbox PalletTown_Text_PressStartToOpenMenu
	closemessage
	releaseall
	end

PalletTown_EventScript_SignLadyTrigger::
	goto_if_set FLAG_OPENED_START_MENU, PalletTown_EventScript_SignLadyShowSignEnd
	lockall
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_WalkInPlaceFasterRight
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	call PalletTown_EventScript_SignLadyShowSign
	releaseall
	end
PalletTown_EventScript_SignLadyShowSign::
	goto_if_set FLAG_OPENED_START_MENU, PalletTown_EventScript_SignLadyShowSignEnd
	msgbox PalletTown_Text_LookCopiedTrainerTipsSign
	checkitemspace ITEM_POTION
	goto_if_eq VAR_RESULT, FALSE, EventScript_BagIsFull
	bufferitemname STR_VAR_2, ITEM_POTION
	playfanfare MUS_OBTAIN_ITEM
	message Text_ObtainedTheX
	waitmessage
	waitfanfare
	additem ITEM_POTION
	msgbox PalletTown_Text_PutPotionAway
	closemessage
	setflag FLAG_OPENED_START_MENU
	setvar VAR_MAP_SCENE_PALLET_TOWN_SIGN_LADY, 1
	setvar SIGN_LADY_READY, FALSE
	release
	end
PalletTown_EventScript_SignLadyShowSignEnd::
	msgbox PalletTown_Text_PressStartToOpenMenuCopy
	closemessage
	release
	end

PalletTown_EventScript_SignLadyStartShowSign::
	applymovement LOCALID_PALLET_SIGN_LADY, Common_Movement_FacePlayer
	waitmovement 0
	call PalletTown_EventScript_SignLadyShowSign
	release
	end

PalletTown_EventScript_Snorlax::
	lock
	msgbox PalletTown_Text_MonSprawledOutInSlumber, MSGBOX_DEFAULT
	closemessage
	release
	end

PalletTown_Text_MonSprawledOutInSlumber::
    .string "Two POKéMON are sprawled out in\n"
    .string "a deep and comfortable slumber.$"

PalletTown_Text_OakDontGoOut::
    .string "OAK: Hey! Wait!\n"
    .string "Don't go out!$"

PalletTown_Text_OakGrassUnsafeNeedMon::
    .string "OAK: It's unsafe!\n"
    .string "Wild POKéMON live in tall grass!\p"
    .string "You need your own POKéMON for\n"
    .string "your protection.\p"
    .string "I know!\n"
    .string "Here, come with me!$"

PalletTown_Text_RaisingMonsToo::
    .string "I'm raising POKéMON, too.\p"
    .string "When they get strong, they can\n"
    .string "protect me.$"

PalletTown_Text_CanStoreItemsAndMonsInPC::
    .string "Technology is incredible!\p"
    .string "You can now store and recall items\n"
    .string "and POKéMON as data via PC.$"

PalletTown_Text_OakPokemonResearchLab::
    .string "OAK POKéMON RESEARCH LAB$"

PalletTown_Text_PlayersHouse::
    .string "DELIA's house$"

PalletTown_Text_RivalsHouse::
    .string "{RIVALK}'s house$"

PalletTown_Text_TownSign::
    .string "PALLET TOWN\n"
    .string "Shades of your journey await!$"

PalletTown_Text_OakLetMeSeePokedex::
    .string "OAK: Ah, {PLAYER}!\n"
    .string "You're back, are you?\p"
    .string "How much have you filled in your\n"
    .string "POKéDEX?\p"
    .string "May I see it?\p"
    .string "Let's see…$"

PalletTown_Text_CaughtXPuttingInHonestEffort::
    .string "You've caught {STR_VAR_2}…\p"
    .string "Hm, it looks as if you're putting\n"
    .string "in an honest effort.\p"
    .string "When you manage to fill it some\n"
    .string "more, come show me, please.$"

PalletTown_Text_CaughtXImpressiveFollowMe::
    .string "You've caught… {STR_VAR_2}!?\n"
    .string "Now, this is impressive!\p"
    .string "There's something I wanted to ask\n"
    .string "of you, {PLAYER}.\p"
    .string "Come.\n"
    .string "Follow me.$"

PalletTown_Text_OakYouEnjoyingTraveling::
    .string "OAK: Ah, {PLAYER}!\n"
    .string "You seem to be enjoying traveling.\p"
    .string "Knowing you, {PLAYER}, I can easily\n"
    .string "imagine you going out to even more\l"
    .string "exotic locales.\p"
    .string "Good for you, good for you.\n"
    .string "Hohoho.$"

PalletTown_Text_PressStartToOpenMenu::
	.string "TRAINER TIPS\p"
	.string "Press START to open the MENU!$"

PalletTown_Text_ReadItReadIt::
	.string "Read it, read it!$"
PalletTown_Text_SignsAreUsefulArentThey::
	.string "Signs are useful, aren't they?$"
PalletTown_Text_PressStartToOpenMenuCopy::
	.string "TRAINER TIPS!\p"
	.string "Press START to open the MENU!$"
PalletTown_Text_OhLookLook::
	.string "Oh!\n"
	.string "Look, look!$"
PalletTown_Text_HmmIsThatRight::
	.string "Hmm…\n"
	.string "Is that right…$"
PalletTown_Text_LookCopiedTrainerTipsSign::
	.string "Look, look!\p"
	.string "I copied what it said on one of\n"
	.string "those TRAINER TIPS signs!\p"
	.string "It said to press START, and that's\n"
	.string "how I was able to open my bag!\p"
	.string "Here, I'll share what I have\n"
	.string "in my bag with you!$"
PalletTown_Text_PutPotionAway::
	.string "{PLAYER} put the POTION away in\n"
    .string "the BAG's ITEMS POCKET.$"

PalletTown_Text_OurNewHomeLetsGoInside::
	.string "AUNT DELIA: {PLAYER}, You're here!\p"
	.string "It must be tiring riding with your things\n"
	.string "in the moving truck.\p"
	.string "Well, this is PALLET TOWN.\n"
	.string "Is it the same as you remember?\p"
	.string "{PLAYER}, I'm so happy you chose to visit\n"
	.string "KANTO as part of your POKEMON journey.\p"
	.string "When I got the call from CAROLINE,\n"
	.string "the first thing I did was set up\l"
	.string "the spare bedroom for you.\p"
	.string "I hope you like it! Let's go inside!$"
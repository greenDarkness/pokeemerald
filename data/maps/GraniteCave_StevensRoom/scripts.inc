GraniteCave_StevensRoom_MapScripts::
	.byte 0

GraniteCave_StevensRoom_EventScript_Steven::
	lock
	faceplayer
	msgbox GraniteCave_StevensRoom_Text_ImStevenLetterForMe, MSGBOX_DEFAULT
	setvar VAR_0x8004, ITEM_LETTER
	call Common_EventScript_PlayerHandedOverTheItem
	setflag FLAG_DELIVERED_STEVEN_LETTER
	msgbox GraniteCave_StevensRoom_Text_ThankYouTakeThis, MSGBOX_DEFAULT
	giveitem ITEM_TM_STEEL_WING
	call_if_eq VAR_RESULT, FALSE, GraniteCave_StevensRoom_EventScript_BagFull
	msgbox GraniteCave_StevensRoom_Text_CouldBecomeChampionLetsRegister, MSGBOX_DEFAULT
	closemessage
	delay 30
	playfanfare MUS_REGISTER_MATCH_CALL
	msgbox GraniteCave_StevensRoom_Text_RegisteredSteven, MSGBOX_DEFAULT
	waitfanfare
	closemessage
	delay 30
	setflag FLAG_REGISTERED_STEVEN_POKENAV
	msgbox GraniteCave_StevensRoom_Text_IveGotToHurryAlong, MSGBOX_DEFAULT
	closemessage
	call_if_eq VAR_FACING, DIR_NORTH, GraniteCave_StevensRoom_EventScript_StevenExitNorth
	call_if_eq VAR_FACING, DIR_SOUTH, GraniteCave_StevensRoom_EventScript_StevenExitSouth
	call_if_eq VAR_FACING, DIR_WEST, GraniteCave_StevensRoom_EventScript_StevenExitWestEast
	call_if_eq VAR_FACING, DIR_EAST, GraniteCave_StevensRoom_EventScript_StevenExitWestEast
	playse SE_EXIT
	removeobject LOCALID_GRANITE_CAVE_STEVEN
	release
	end

GraniteCave_StevensRoom_EventScript_StevenExitNorth::
	applymovement LOCALID_GRANITE_CAVE_STEVEN, GraniteCave_StevensRoom_Movement_StevenExit
	waitmovement 0
	return

GraniteCave_StevensRoom_EventScript_StevenExitWestEast::
	applymovement LOCALID_PLAYER, GraniteCave_StevensRoom_Movement_PlayerTurnTowardExit
	applymovement LOCALID_GRANITE_CAVE_STEVEN, GraniteCave_StevensRoom_Movement_StevenExit
	waitmovement 0
	return

GraniteCave_StevensRoom_EventScript_StevenExitSouth::
	applymovement LOCALID_PLAYER, GraniteCave_StevensRoom_Movement_PlayerTurnTowardExit
	applymovement LOCALID_GRANITE_CAVE_STEVEN, GraniteCave_StevensRoom_Movement_StevenExitSouth
	waitmovement 0
	return

GraniteCave_StevensRoom_EventScript_BagFull::
	msgbox GraniteCave_StevensRoom_Text_OhBagIsFull, MSGBOX_DEFAULT
	return

GraniteCave_StevensRoom_Movement_StevenExit:
	walk_up
	walk_up
	walk_up
	walk_up
	walk_up
	delay_8
	step_end

GraniteCave_StevensRoom_Movement_PlayerTurnTowardExit:
	delay_16
	delay_16
	delay_16
	walk_in_place_faster_up
	step_end

GraniteCave_StevensRoom_Movement_StevenExitSouth:
	walk_left
	walk_up
	walk_up
	walk_up
	walk_right
	walk_up
	walk_up
	delay_8
	step_end

@

GraniteCave_StevensRoom_EventScript_FireStone::
	lock
	faceplayer
	msgbox GraniteCave_StevensRoom_Text_TakeFireStone, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GraniteCave_StevensRoom_EventScript_LeaveFireStone
	giveitem ITEM_FIRE_STONE
	closemessage
	removeobject LOCALID_FIRE_STONE
	delay 30
	goto GraniteCave_StevensRoom_EventScript_Collapse
	end
GraniteCave_StevensRoom_EventScript_LeaveFireStone::
	msgbox GraniteCave_StevensRoom_Text_LeftFireStoneAlone, MSGBOX_DEFAULT
	release
	end

GraniteCave_StevensRoom_EventScript_ThunderStone::
	lock
	faceplayer
	msgbox GraniteCave_StevensRoom_Text_TakeThunderStone, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GraniteCave_StevensRoom_EventScript_LeaveThunderStone
	giveitem ITEM_THUNDER_STONE
	closemessage
	removeobject LOCALID_THUNDER_STONE
	delay 30
	goto GraniteCave_StevensRoom_EventScript_Collapse
	end
GraniteCave_StevensRoom_EventScript_LeaveThunderStone::
	msgbox GraniteCave_StevensRoom_Text_LeftThunderStoneAlone, MSGBOX_DEFAULT
	release
	end

GraniteCave_StevensRoom_EventScript_WaterStone::
	lock
	faceplayer
	msgbox GraniteCave_StevensRoom_Text_TakeWaterStone, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GraniteCave_StevensRoom_EventScript_LeaveFireStone
	giveitem ITEM_FIRE_STONE
	closemessage
	removeobject LOCALID_WATER_STONE
	delay 30
	goto GraniteCave_StevensRoom_EventScript_Collapse
	end
GraniteCave_StevensRoom_EventScript_LeaveWaterStone::
	msgbox GraniteCave_StevensRoom_Text_LeftWaterStoneAlone, MSGBOX_DEFAULT
	release
	end

GraniteCave_StevensRoom_EventScript_LeafStone::
	lock
	faceplayer
	msgbox GraniteCave_StevensRoom_Text_TakeLeafStone, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GraniteCave_StevensRoom_EventScript_LeaveFireStone
	giveitem ITEM_FIRE_STONE
	closemessage
	removeobject LOCALID_LEAF_STONE
	delay 30
	goto GraniteCave_StevensRoom_EventScript_Collapse
	end
GraniteCave_StevensRoom_EventScript_LeaveLeafStone::
	msgbox GraniteCave_StevensRoom_Text_LeftLeafStoneAlone, MSGBOX_DEFAULT
	release
	end

GraniteCave_StevensRoom_EventScript_SunStone::
	lock
	faceplayer
	msgbox GraniteCave_StevensRoom_Text_TakeSunStone, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GraniteCave_StevensRoom_EventScript_LeaveFireStone
	giveitem ITEM_FIRE_STONE
	closemessage
	removeobject LOCALID_SUN_STONE
	delay 30
	goto GraniteCave_StevensRoom_EventScript_Collapse
	end
GraniteCave_StevensRoom_EventScript_LeaveSunStone::
	msgbox GraniteCave_StevensRoom_Text_LeftSunStoneAlone, MSGBOX_DEFAULT
	release
	end

GraniteCave_StevensRoom_EventScript_MoonStone::
	lock
	faceplayer
	msgbox GraniteCave_StevensRoom_Text_TakeMoonStone, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, GraniteCave_StevensRoom_EventScript_LeaveFireStone
	giveitem ITEM_FIRE_STONE
	closemessage
	removeobject LOCALID_MOON_STONE
	delay 30
	goto GraniteCave_StevensRoom_EventScript_Collapse
	end
GraniteCave_StevensRoom_EventScript_LeaveMoonStone::
	msgbox GraniteCave_StevensRoom_Text_LeftMoonStoneAlone, MSGBOX_DEFAULT
	release
	end

GraniteCave_StevensRoom_EventScript_Collapse::
	setvar VAR_0x8004, 1   @ vertical pan
	setvar VAR_0x8005, 1   @ horizontal pan
	setvar VAR_0x8006, 32  @ num shakes
	setvar VAR_0x8007, 2   @ shake delay
	special ShakeCamera
	waitstate
	special DoMirageTowerCeilingCrumble
	fadescreen FADE_TO_BLACK
	setflag FLAG_HIDE_STEVENS_ROOM_FIRESTONE
	setflag FLAG_HIDE_STEVENS_ROOM_THUNDERSTONE
	setflag FLAG_HIDE_STEVENS_ROOM_WATERSTONE
	setflag FLAG_HIDE_STEVENS_ROOM_LEAFSTONE
	setflag FLAG_HIDE_STEVENS_ROOM_SUNSTONE
	setflag FLAG_HIDE_STEVENS_ROOM_MOONSTONE
	removeobject LOCALID_FIRE_STONE
	removeobject LOCALID_THUNDER_STONE
	removeobject LOCALID_WATER_STONE
	removeobject LOCALID_LEAF_STONE
	removeobject LOCALID_SUN_STONE
	removeobject LOCALID_MOON_STONE
	delay 30
	fadescreen FADE_FROM_BLACK
	waitstate
	release
	end

@

GraniteCave_StevensRoom_Text_ImStevenLetterForMe:
	.string "My name is STEVEN.\p"
	.string "I'm interested in rare stones,\n"
	.string "so I travel here and there.\p"
	.string "Oh?\n"
	.string "A LETTER for me?$"

GraniteCave_StevensRoom_Text_ThankYouTakeThis:
	.string "STEVEN: Okay, thank you.\p"
	.string "You went through all this trouble to\n"
	.string "deliver that. I need to thank you.\p"
	.string "Let me see…\n"
	.string "I'll give you this TM.\p"
	.string "It contains my favorite move,\n"
	.string "STEEL WING.$"

GraniteCave_StevensRoom_Text_CouldBecomeChampionLetsRegister:
	.string "STEVEN: Your POKéMON appear quite\n"
	.string "capable.\p"
	.string "If you keep training, you could even\n"
	.string "become the CHAMPION of the POKéMON\l"
	.string "LEAGUE one day. That's what I think.\p"
	.string "I know, since we've gotten to know each\n"
	.string "other, let's register one another in\l"
	.string "our POKéNAVS.\p"
	.string "… … … … … …$"

GraniteCave_StevensRoom_Text_RegisteredSteven:
	.string "Registered STEVEN\n"
	.string "in the POKéNAV.$"

GraniteCave_StevensRoom_Text_IveGotToHurryAlong:
	.string "Now, I've got to hurry along.$"

GraniteCave_StevensRoom_Text_OhBagIsFull:
	.string "Oh, your BAG is full…\n"
	.string "That's too bad, then.$"

GraniteCave_StevensRoom_Text_TakeFireStone:
	.string "You found a FIRE STONE.\p"
	.string "If this stone is taken, the ground\n"
	.string "around it will likely crumble away…\p"
	.string "Take the FIRE STONE anyway?$"
GraniteCave_StevensRoom_Text_LeftFireStoneAlone:
	.string "{PLAYER} left the FIRE STONE alone.$"

GraniteCave_StevensRoom_Text_TakeThunderStone:
	.string "You found a THUNDER STONE.\p"
	.string "If this stone is taken, the ground\n"
	.string "around it will likely crumble away…\p"
	.string "Take the THUNDER STONE anyway?$"
GraniteCave_StevensRoom_Text_LeftThunderStoneAlone:
	.string "{PLAYER} left the THUNDER STONE alone.$"

GraniteCave_StevensRoom_Text_TakeWaterStone:
	.string "You found a WATER STONE.\p"
	.string "If this stone is taken, the ground\n"
	.string "around it will likely crumble away…\p"
	.string "Take the WATER STONE anyway?$"
GraniteCave_StevensRoom_Text_LeftWaterStoneAlone:
	.string "{PLAYER} left the WATER STONE alone.$"

GraniteCave_StevensRoom_Text_TakeLeafStone:
	.string "You found a LEAF STONE.\p"
	.string "If this stone is taken, the ground\n"
	.string "around it will likely crumble away…\p"
	.string "Take the LEAF STONE anyway?$"
GraniteCave_StevensRoom_Text_LeftLeafStoneAlone:
	.string "{PLAYER} left the LEAF STONE alone.$"

GraniteCave_StevensRoom_Text_TakeSunStone:
	.string "You found a SUN STONE.\p"
	.string "If this stone is taken, the ground\n"
	.string "around it will likely crumble away…\p"
	.string "Take the SUN STONE anyway?$"
GraniteCave_StevensRoom_Text_LeftSunStoneAlone:
	.string "{PLAYER} left the SUN STONE alone.$"

GraniteCave_StevensRoom_Text_TakeMoonStone:
	.string "You found a MOON STONE.\p"
	.string "If this stone is taken, the ground\n"
	.string "around it will likely crumble away…\p"
	.string "Take the MOON STONE anyway?$"
GraniteCave_StevensRoom_Text_LeftMoonStoneAlone:
	.string "{PLAYER} left the MOON STONE alone.$"
InsideOfTruck_MapScripts::
	map_script MAP_SCRIPT_ON_LOAD, InsideOfTruck_OnLoad
	map_script MAP_SCRIPT_ON_RESUME, InsideOfTruck_OnResume
	.byte 0

InsideOfTruck_OnLoad:
	setmetatile 4, 1, METATILE_InsideOfTruck_ExitLight_Top, FALSE
	setmetatile 4, 2, METATILE_InsideOfTruck_ExitLight_Mid, FALSE
	setmetatile 4, 3, METATILE_InsideOfTruck_ExitLight_Bottom, FALSE
	end

InsideOfTruck_OnResume:
	setstepcallback STEP_CB_TRUCK
	end

InsideOfTruck_EventScript_SetIntroFlags::
	setflag FLAG_HIDE_MAP_NAME_POPUP
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, InsideOfTruck_EventScript_SetIntroFlagsMale
	goto_if_eq VAR_RESULT, FEMALE, InsideOfTruck_EventScript_SetIntroFlagsFemale
	end

InsideOfTruck_EventScript_SetIntroFlagsMale::
	setrespawn HEAL_LOCATION_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F
	setvar VAR_LITTLEROOT_INTRO_STATE, 1
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_TRUCK
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_RIVAL_SIBLING
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_2F_POKE_BALL
	setvar VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 1
	goto_if_set FLAG_TRUCKEVENT_KANTO, InsideOfTruck_EventScript_KantoWarp
	goto_if_set FLAG_TRUCKEVENT_JOHTO, InsideOfTruck_EventScript_JohtoWarp
	goto InsideOfTruck_EventScript_HoennWarpMale
	release
	end

InsideOfTruck_EventScript_SetIntroFlagsFemale::
	setrespawn HEAL_LOCATION_LITTLEROOT_TOWN_MAYS_HOUSE_2F
	setvar VAR_LITTLEROOT_INTRO_STATE, 2
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_BRENDANS_HOUSE_TRUCK
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_MOM
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_RIVAL_SIBLING
	setflag FLAG_HIDE_LITTLEROOT_TOWN_MAYS_HOUSE_2F_POKE_BALL
	setvar VAR_LITTLEROOT_HOUSES_STATE_MAY, 1
	goto_if_set FLAG_TRUCKEVENT_KANTO, InsideOfTruck_EventScript_KantoWarp
	goto_if_set FLAG_TRUCKEVENT_JOHTO, InsideOfTruck_EventScript_JohtoWarp
	goto InsideOfTruck_EventScript_HoennWarpFemale
	release
	end

InsideOfTruck_EventScript_KantoWarp::
	setdynamicwarp MAP_PALLET_TOWN, 4, 10
	release
	end
InsideOfTruck_EventScript_JohtoWarp::
	setdynamicwarp MAP_PALLET_TOWN, 4, 10
	release
	end
InsideOfTruck_EventScript_HoennWarpMale::
	setdynamicwarp MAP_LITTLEROOT_TOWN, 3, 10
	release
	end
InsideOfTruck_EventScript_HoennWarpFemale::
	setdynamicwarp MAP_LITTLEROOT_TOWN, 12, 10
	release
	end

InsideOfTruck_EventScript_MovingBox::
	msgbox InsideOfTruck_Text_BoxPrintedWithMonLogo, MSGBOX_SIGN
	end

@ KANTO/JOHTO starting region cabinet
InsideOfTruck_EventScript_KantoStart::
	lock
	msgbox InsideOfTruck_Text_KantoStart, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, InsideOfTruck_EventScript_KantoStart_SetFlag
	goto InsideOfTruck_EventScript_HoennStart
	end
InsideOfTruck_EventScript_JohtoStart::
	lock
	msgbox InsideOfTruck_Text_JohtoStart, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, InsideOfTruck_EventScript_JohtoStart_SetFlag
	goto InsideOfTruck_EventScript_HoennStart
	end
InsideOfTruck_EventScript_HoennStart::
	clearflag FLAG_TRUCKEVENT_KANTO
	clearflag FLAG_TRUCKEVENT_JOHTO
	setflag FLAG_TRUCKEVENT_HOENN
	msgbox InsideOfTruck_Text_HoennStart, MSGBOX_DEFAULT
	closemessage
	release
	end
InsideOfTruck_EventScript_KantoStart_SetFlag::
	clearflag FLAG_TRUCKEVENT_JOHTO
	clearflag FLAG_TRUCKEVENT_HOENN
	setflag FLAG_TRUCKEVENT_KANTO
	msgbox InsideOfTruck_Text_KantoStartEnd, MSGBOX_DEFAULT
	closemessage
	release
	end
InsideOfTruck_EventScript_JohtoStart_SetFlag::
	clearflag FLAG_TRUCKEVENT_KANTO
	clearflag FLAG_TRUCKEVENT_HOENN
	setflag FLAG_TRUCKEVENT_JOHTO
	msgbox InsideOfTruck_Text_JohtoStartEnd, MSGBOX_DEFAULT
	closemessage
	release
	end

InsideOfTruck_EventScript_ChooseDollPokemon::
	msgbox InsideOfTruck_Text_ChooseDollPokemon, MSGBOX_SIGN
	multichoice 1, 3, TRUCKEVENT_DOLL_POKEMON, TRUE
	switch VAR_RESULT
	case 0, InsideOfTruck_EventScript_ChooseBaltoyPokemon
	case 1, InsideOfTruck_EventScript_ChooseShuppetPokemon
	removeobject 4
	setflag FLAG_HIDE_TRUCKEVENT_CABINET
	end

InsideOfTruck_EventScript_ChooseBaltoyPokemon::
	setflag FLAG_CHOSE_BALTOY
	msgbox InsideOfTruck_Text_ChoosenDoll, MSGBOX_SIGN
	removeobject 4
	end

InsideOfTruck_EventScript_ChooseShuppetPokemon::
	setflag FLAG_CHOSE_SHUPPET
	msgbox InsideOfTruck_Text_ChoosenDoll, MSGBOX_SIGN
	removeobject 4
	end

InsideOfTruck_EventScript_DoHidingSpotSparkle::
	setvar VAR_0x8004, 1  @ x
	setvar VAR_0x8005, 3  @ y
	setvar VAR_0x8006, 4  @ z
	dofieldeffectsparkle VAR_0x8004, VAR_0x8005, VAR_0x8006
	waitfieldeffect FLDEFF_SPARKLE
	delay 10
	return

InsideOfTruck_Text_ChooseDollPokemon:
	.string "The cabinet was damaged during transit.\n"
	.string "It might not make it off the truck...\p"
	.string "In the drawer there are toys from your\n"
	.string "childhood.\p"
	.string "Your favorite toy is sitting\n"
	.string "right on top.$"

InsideOfTruck_Text_ChoosenDoll:
	.string "You stash the toy in a box with your\n"
	.string "personal belongings.\p"
	.string "It wouldn't feel right to leave\n"
	.string "it behind.$"

InsideOfTruck_Text_BoxPrintedWithMonLogo:
	.string "The box is printed with a POKéMON logo.\p"
	.string "It's a POKéMON brand moving and\n"
	.string "delivery service.$"

InsideOfTruck_Text_KantoStart:
	.string "Would you like to start in KANTO?$"
InsideOfTruck_Text_KantoStartEnd:
	.string "The starting region is set\n"
	.string "to KANTO.$"
InsideOfTruck_Text_JohtoStart:
	.string "Would you like to start in JOHTO?$"
InsideOfTruck_Text_JohtoStartEnd:
	.string "The starting region is set\n"
	.string "to JOHTO.$"
InsideOfTruck_Text_HoennStart:
	.string "The starting region is set\n"
	.string "to HOENN.$"

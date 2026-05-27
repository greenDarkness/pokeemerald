InsideOfBoat_MapScripts::
	.byte 0

InsideOfBoat_EventScript_SetIntroFlags::
	setflag FLAG_HIDE_MAP_NAME_POPUP
	checkplayergender
	goto_if_eq VAR_RESULT, MALE, InsideOfBoat_EventScript_SetIntroFlagsMale
	goto_if_eq VAR_RESULT, FEMALE, InsideOfBoat_EventScript_SetIntroFlagsFemale
	end
InsideOfBoat_EventScript_SetIntroFlagsMale::
	setrespawn HEAL_LOCATION_NEW_BARK_TOWN_PLAYERS_HOUSE_2F
	setvar VAR_LITTLEROOT_HOUSES_STATE_BRENDAN, 1
	release
	end
InsideOfBoat_EventScript_SetIntroFlagsFemale::
	setrespawn HEAL_LOCATION_NEW_BARK_TOWN_PLAYERS_HOUSE_2F
	setvar VAR_LITTLEROOT_HOUSES_STATE_MAY, 1
	release
	end

InsideOfBoat_EventScript_MovingBox::
	end

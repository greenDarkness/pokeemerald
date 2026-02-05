PetalburgCity_House1_MapScripts::
	.byte 0

PetalburgCity_House1_EventScript_TravelingMan::
	lock
	faceplayer
	goto_if_unset FLAG_PETALBURG_TRAVELINGMAN_COMPLETED, PetalburgCity_House1_EventScript_TravelingMan2
	msgbox PetalburgCity_House1_Text_TravelingIsWonderful, MSGBOX_NPC
	release
	end
PetalburgCity_House1_EventScript_TravelingMan2::
	msgbox PetalburgCity_House1_Text_TravelingBestPart, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, PetalburgCity_House1_EventScript_TravelingMan3Yes
	goto_if_eq VAR_RESULT, NO, PetalburgCity_House1_EventScript_TravelingMan3NO
	release
	end
PetalburgCity_House1_EventScript_TravelingMan3Yes::
	msgbox PetalburgCity_House1_Text_TravelingTechnique, MSGBOX_DEFAULT
	giveitem ITEM_MACHO_BRACE
	setflag FLAG_PETALBURG_TRAVELINGMAN_COMPLETED
	release
	end
PetalburgCity_House1_EventScript_TravelingMan3NO::
	msgbox PetalburgCity_House1_Text_TravelingTechnique, MSGBOX_DEFAULT
	giveitem ITEM_EXP_SHARE
	setflag FLAG_PETALBURG_TRAVELINGMAN_COMPLETED
	release
	end

PetalburgCity_House1_EventScript_Woman::
	msgbox PetalburgCity_House1_Text_GoOnAdventure, MSGBOX_NPC
	end

PetalburgCity_House1_Text_TravelingIsWonderful:
	.string "Traveling is wonderful!\p"
	.string "When I was young, I roamed the seas\n"
	.string "and the mountains!$"
PetalburgCity_House1_Text_TravelingBestPart:
	.string "Traveling is wonderful!\p"
	.string "When I was young, I roamed the seas\n"
	.string "and the mountains!\p"
	.string "I searched all over HOENN for places\n"
	.string "to train with my POKéMON.\p"
	.string "That is the best part, don't you think?$"
PetalburgCity_House1_Text_TravelingTechnique:
	.string "I could tell you felt that way\n"
	.string "when you walked in.\p"
	.string "You have that look.\p"
	.string "So, I dug this out of the\n"
	.string "closet.\p"
	.string "I want to pass-on my special\n"
	.string "training technique to someone\l"
	.string "who could use it!$"

PetalburgCity_House1_Text_GoOnAdventure:
	.string "Sigh…\p"
	.string "I wish I could go on an adventure\n"
	.string "with some POKéMON…\p"
	.string "Crawl through some damp grass…\n"
	.string "Climb rocky, rugged mountains…\p"
	.string "Cross the raging seas…\n"
	.string "Wander about in dark caves…\p"
	.string "And, sometimes, even get a little\n"
	.string "homesick…\p"
	.string "It must be fabulous to travel!$"

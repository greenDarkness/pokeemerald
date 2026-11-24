PetalburgCity_HerbShop_MapScripts::
	.byte 0

PetalburgCity_HerbShop_EventScript_Clerk::
	lock
	faceplayer
	message PetalburgCity_HerbShop_Text_WelcomeToHerbShop
	waitmessage
	pokemart PetalburgCity_HerbShop_Pokemart
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

	.align 2
PetalburgCity_HerbShop_Pokemart:
	.2byte ITEM_ENERGY_POWDER
	.2byte ITEM_ENERGY_ROOT
	.2byte ITEM_HEAL_POWDER
	.2byte ITEM_REVIVAL_HERB
	pokemartlistend

PetalburgCity_HerbShop_EventScript_Woman::
	msgbox PetalburgCity_HerbShop_Text_WeakWillGrowStronger, MSGBOX_NPC
	end

PetalburgCity_HerbShop_EventScript_Boy::
	msgbox PetalburgCity_HerbShop_Text_RepelIsUseful, MSGBOX_NPC
	end

PetalburgCity_HerbShop_EventScript_Man::
	msgbox PetalburgCity_HerbShop_Text_TakeSomeAntidotesWithYou, MSGBOX_NPC
	end

PetalburgCity_HerbShop_Text_WelcomeToHerbShop:
	.string "Welcome to the HERB SHOP, home of\n"
	.string "effective and inexpensive medicine!$"

PetalburgCity_HerbShop_Text_WeakWillGrowStronger:
	.string "Even if a POKéMON is weak now,\n"
	.string "it will grow stronger.\p"
	.string "The most important thing is love!\n"
	.string "Love for your POKéMON!$"

PetalburgCity_HerbShop_Text_RepelIsUseful:
	.string "Do you use REPEL?\n"
	.string "It keeps POKéMON away, so it's\l"
	.string "useful when you're in a hurry.$"

PetalburgCity_HerbShop_Text_TakeSomeAntidotesWithYou:
	.string "Do you have any ANTIDOTES with\n"
	.string "you?\p"
	.string "If you walk around with a poisoned\n"
	.string "POKéMON, it will lose HP until it faints.\l"
	.string "Take some ANTIDOTES with you.$"


OldaleTown_House2_MapScripts::
	.byte 0

OldaleTown_House2_EventScript_Woman::
	msgbox OldaleTown_House2_Text_PokemonLevelUp, MSGBOX_NPC
	end

OldaleTown_House2_EventScript_Man::
	msgbox OldaleTown_House2_Text_YoullGoFurtherWithStrongPokemon, MSGBOX_NPC
	end

OldaleTown_House2_EventScript_BrawlyFan::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_RUSTY_KEY_OLDALE, OldaleTown_House2_BrawlyFanComplete
	goto_if_set FLAG_DEFEATED_DEWFORD_GYM, OldaleTown_House2_BrawlyFanRustyKey
	msgbox OldaleTown_House2_Text_BrawlyFanIntro, MSGBOX_NPC
	release
	end

OldaleTown_House2_BrawlyFanRustyKey::
	msgbox OldaleTown_House2_Text_BrawlyFanRustyKey, MSGBOX_NPC
	giveitem ITEM_RUSTY_KEY
	setflag FLAG_RECEIVED_RUSTY_KEY_OLDALE
	release
	end

OldaleTown_House2_BrawlyFanComplete::
	msgbox OldaleTown_House2_Text_BrawlyFanComplete, MSGBOX_NPC
	release
	end

OldaleTown_House2_Text_PokemonLevelUp:
	.string "When POKéMON battle, they eventually\n"
	.string "level up and become stronger.$"

OldaleTown_House2_Text_YoullGoFurtherWithStrongPokemon:
	.string "If the POKéMON with you become\n"
	.string "stronger, you'll be able to go farther\l"
	.string "away from here.$"

OldaleTown_House2_Text_BrawlyFanIntro:
	.string "If the POKéMON with you become\n"
	.string "stronger, you'll be able to go farther\l"
	.string "away from here.\p"
	.string "...By the looks of it, you still have\n"
	.string "a way to go.$"

OldaleTown_House2_Text_BrawlyFanRustyKey:
	.string "If the POKéMON with you become\n"
	.string "stronger, you'll be able to go farther\l"
	.string "away from here...\p"
	.string "Is that really the DEWFORD GYM BADGE!?\n"
	.string "BRAWLY is my idol! you must be a great\l"
	.string "trainer to have traveled so far.\p"
	.string "I have always wanted to be in the good\n"
	.string "graces of a celebrity- I mean, here,\l"
	.string "take this.\p"
	.string "I haven't found where it goes yet, but\n"
	.string "maybe a great trainer can!$"

OldaleTown_House2_Text_BrawlyFanComplete:
	.string "If your POKéMON keep becoming stronger,\n"
	.string "you'll be able to go even farther\l"
	.string "away than DEWFORD.\p"
	.string "...but why would you want to?\n"
	.string "They have BRAWLY!$"
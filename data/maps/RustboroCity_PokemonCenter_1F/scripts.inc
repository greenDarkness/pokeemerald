RustboroCity_PokemonCenter_1F_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, RustboroCity_PokemonCenter_1F_OnTransition
	map_script MAP_SCRIPT_ON_RESUME, CableClub_OnResume
	.byte 0

RustboroCity_PokemonCenter_1F_OnTransition:
	setrespawn HEAL_LOCATION_RUSTBORO_CITY
	call Common_EventScript_UpdateBrineyLocation
	end

RustboroCity_PokemonCenter_1F_EventScript_Nurse::
	setvar VAR_0x800B, LOCALID_RUSTBORO_NURSE
	call Common_EventScript_PkmnCenterNurse
	waitmessage
	waitbuttonpress
	release
	end

RustboroCity_PokemonCenter_1F_EventScript_Man::
	msgbox RustboroCity_PokemonCenter_1F_Text_PokemonHavePersonalities, MSGBOX_NPC
	end

RustboroCity_PokemonCenter_1F_EventScript_Boy::
	lock
	faceplayer
	msgbox RustboroCity_PokemonCenter_1F_Text_MaleAndFemalePokemon, MSGBOX_DEFAULT
	msgbox RustboroCity_PokemonCenter_1F_Text_CheckPokemonGender, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RustboroCity_PokemonCenter_1F_EventScript_BoyDecline
	special ChooseMonForGenderChange
	waitstate
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, RustboroCity_PokemonCenter_1F_EventScript_BoyDecline
	specialvar VAR_0x8005, ScriptGetPartyMonSpecies
	bufferspeciesname STR_VAR_1, VAR_0x8005
	special ChangePokemonGender
	goto_if_eq VAR_RESULT, MON_MALE, RustboroCity_PokemonCenter_1F_EventScript_BoyChangedToMale
	goto_if_eq VAR_RESULT, MON_FEMALE, RustboroCity_PokemonCenter_1F_EventScript_BoyChangedToFemale
	msgbox RustboroCity_PokemonCenter_1F_Text_CantChangeGender, MSGBOX_DEFAULT
	release
	end

RustboroCity_PokemonCenter_1F_EventScript_BoyChangedToMale::
	msgbox RustboroCity_PokemonCenter_1F_Text_ActuallyMale, MSGBOX_DEFAULT
	release
	end

RustboroCity_PokemonCenter_1F_EventScript_BoyChangedToFemale::
	msgbox RustboroCity_PokemonCenter_1F_Text_ActuallyFemale, MSGBOX_DEFAULT
	release
	end

RustboroCity_PokemonCenter_1F_EventScript_BoyDecline::
	msgbox RustboroCity_PokemonCenter_1F_Text_MaybeLater, MSGBOX_DEFAULT
	release
	end

RustboroCity_PokemonCenter_1F_EventScript_Girl::
	msgbox RustboroCity_PokemonCenter_1F_Text_HMCutNextDoor, MSGBOX_NPC
	end

RustboroCity_PokemonCenter_1F_Text_PokemonHavePersonalities:
	.string "My POKéMON has a NAIVE nature, and my\n"
	.string "friend's has a JOLLY nature.\p"
	.string "It's fascinating how POKéMON have\n"
	.string "personalities!$"

RustboroCity_PokemonCenter_1F_Text_MaleAndFemalePokemon:
	.string "Just like people, there are male and\n"
	.string "female POKéMON.\p"
	.string "But no one seems to have any idea how\n"
	.string "they're different.$"

RustboroCity_PokemonCenter_1F_Text_CheckPokemonGender:
	.string "Say, can I take a closer look at one\n"
	.string "of your POKéMON?\p"
	.string "Maybe I can spot the differences!$"

RustboroCity_PokemonCenter_1F_Text_ActuallyMale:
	.string "Wait… That's not right.\p"
	.string "Your {STR_VAR_1} is actually male!$"

RustboroCity_PokemonCenter_1F_Text_ActuallyFemale:
	.string "Wait… That's not right.\p"
	.string "Your {STR_VAR_1} is actually female!$"

RustboroCity_PokemonCenter_1F_Text_CantChangeGender:
	.string "Hmm… I can't quite tell with\n"
	.string "this one.$"

RustboroCity_PokemonCenter_1F_Text_MaybeLater:
	.string "Oh, okay.\n"
	.string "Maybe some other time!$"

RustboroCity_PokemonCenter_1F_Text_HMCutNextDoor:
	.string "The man next door gave me an HM!\p"
	.string "I used it to teach my POKéMON how to\n"
	.string "CUT down skinny trees.$"


.equ FLAG_HIDE_SPRITES_MORNING,   				FLAG_TEMP_A
.equ FLAG_HIDE_SPRITES_DAY,   					FLAG_TEMP_B
.equ FLAG_HIDE_SPRITES_NIGHT,   				FLAG_TEMP_C
.equ FLAG_HIDE_SPRITES_NIGHT_AND_MORNING,       FLAG_TEMP_D
.equ FLAG_HIDE_SPRITES_MORNING_AND_DAY,   		FLAG_TEMP_E
.equ FLAG_HIDE_SPRITES_DAY_AND_NIGHT,   	    FLAG_TEMP_F

ViridianForest_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, ViridianForest_OnTransition
	.byte 0

ViridianForest_OnTransition::
	setworldmapflag FLAG_WORLD_MAP_VIRIDIAN_FOREST
    call ViridianForest_EventScript_ShowPokemon
	end

ViridianForest_EventScript_ShowPokemon::
	call ViridianForest_EventScript_HidePokemon
	specialvar VAR_RESULT, GetScriptTimeOfDay
    goto_if_eq VAR_RESULT, SCRIPT_TIME_MORNING, ViridianForest_EventScript_ShowMorningPokemon
	goto_if_eq VAR_RESULT, SCRIPT_TIME_DAY, ViridianForest_EventScript_ShowDayPokemon
	goto_if_eq VAR_RESULT, SCRIPT_TIME_NIGHT, ViridianForest_EventScript_ShowNightPokemon
	return
ViridianForest_EventScript_ShowMorningPokemon::
	clearflag FLAG_HIDE_SPRITES_MORNING
	clearflag FLAG_HIDE_SPRITES_MORNING_AND_DAY
	clearflag FLAG_HIDE_SPRITES_NIGHT_AND_MORNING
	end
ViridianForest_EventScript_ShowDayPokemon::
	clearflag FLAG_HIDE_SPRITES_DAY
	clearflag FLAG_HIDE_SPRITES_MORNING_AND_DAY
	clearflag FLAG_HIDE_SPRITES_DAY_AND_NIGHT
	end
ViridianForest_EventScript_ShowNightPokemon::
	clearflag FLAG_HIDE_SPRITES_NIGHT
	clearflag FLAG_HIDE_SPRITES_DAY_AND_NIGHT
	clearflag FLAG_HIDE_SPRITES_NIGHT_AND_MORNING
	end
ViridianForest_EventScript_HidePokemon::
	setflag FLAG_HIDE_SPRITES_MORNING
	setflag FLAG_HIDE_SPRITES_DAY
	setflag FLAG_HIDE_SPRITES_NIGHT
	setflag FLAG_HIDE_SPRITES_MORNING_AND_DAY
	setflag FLAG_HIDE_SPRITES_DAY_AND_NIGHT
	setflag FLAG_HIDE_SPRITES_NIGHT_AND_MORNING
	return

ViridianForest_EventScript_Youngster::
	msgbox ViridianForest_Text_FriendsItchingToBattle, MSGBOX_NPC
	end

ViridianForest_EventScript_Boy::
	msgbox ViridianForest_Text_RanOutOfPokeBalls, MSGBOX_NPC
	end

ViridianForest_EventScript_TrainerTips1::
	msgbox ViridianForest_Text_AvoidGrassyAreasWhenWeak, MSGBOX_SIGN
	end

ViridianForest_EventScript_TrainerTips2::
	msgbox ViridianForest_Text_UseAntidoteForPoison, MSGBOX_SIGN
	end

ViridianForest_EventScript_TrainerTips3::
	msgbox ViridianForest_Text_ContactOakViaPCToRatePokedex, MSGBOX_SIGN
	end

ViridianForest_EventScript_TrainerTips4::
	msgbox ViridianForest_Text_CantCatchOwnedMons, MSGBOX_SIGN
	end

ViridianForest_EventScript_TrainerTips5::
	msgbox ViridianForest_Text_WeakenMonsBeforeCapture, MSGBOX_SIGN
	end

ViridianForest_EventScript_ExitSign::
	msgbox ViridianForest_Text_LeavingViridianForest, MSGBOX_SIGN
	end

ViridianForest_EventScript_Rick::
	trainerbattle_single TRAINER_BUG_CATCHER_RICK, ViridianForest_Text_RickIntro, ViridianForest_Text_RickDefeat
	msgbox ViridianForest_Text_RickPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianForest_EventScript_Doug::
	trainerbattle_single TRAINER_BUG_CATCHER_DOUG, ViridianForest_Text_DougIntro, ViridianForest_Text_DougDefeat
	msgbox ViridianForest_Text_DougPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianForest_EventScript_Sammy::
	trainerbattle_single TRAINER_BUG_CATCHER_SAMMY, ViridianForest_Text_SammyIntro, ViridianForest_Text_SammyDefeat
	msgbox ViridianForest_Text_SammyPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianForest_EventScript_Anthony::
	trainerbattle_single TRAINER_BUG_CATCHER_ANTHONY, ViridianForest_Text_AnthonyIntro, ViridianForest_Text_AnthonyDefeat
	msgbox ViridianForest_Text_AnthonyPostBattle, MSGBOX_AUTOCLOSE
	end

ViridianForest_EventScript_Charlie::
	trainerbattle_single TRAINER_BUG_CATCHER_CHARLIE, ViridianForest_Text_CharlieIntro, ViridianForest_Text_CharlieDefeat
	msgbox ViridianForest_Text_CharliePostBattle, MSGBOX_AUTOCLOSE
	end

ViridianForest_EventScript_FishingGuru::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_OLD_ROD, ViridianForest_EventScript_AlreadyGotOldRod
	msgbox ViridianForest_Text_ImFishingGuruDoYouLikeToFish, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, ViridianForest_EventScript_GiveOldRod
	msgbox ViridianForest_Text_OhThatsSoDisappointing
	release
	end
ViridianForest_EventScript_AlreadyGotOldRod::
	msgbox ViridianForest_Text_HowAreTheFishBiting
	release
	end
ViridianForest_EventScript_GiveOldRod::
	checkitemspace ITEM_OLD_ROD
	goto_if_eq VAR_RESULT, FALSE, ViridianForest_EventScript_NoRoomForOldRod
	additem ITEM_OLD_ROD
    setflag FLAG_RECEIVED_OLD_ROD
	msgbox ViridianForest_Text_TakeThisAndFish
	msgreceiveditem ViridianForest_Text_ReceivedOldRodFromFishingGuru, ITEM_OLD_ROD
	msgbox ViridianForest_Text_FishingIsAWayOfLife
	release
	end
ViridianForest_EventScript_NoRoomForOldRod::
	msgbox ViridianForest_Text_NoRoomForNiceGift
	release
	end

ViridianForest_Text_ImFishingGuruDoYouLikeToFish::
    .string "I'm the FISHING GURU!\p"
    .string "I simply looove fishing!\n"
    .string "I can't bear to go without.\p"
    .string "Tell me, do you like to fish?$"
ViridianForest_Text_TakeThisAndFish::
    .string "Grand! I like your style.\n"
    .string "I think we can be friends.\p"
    .string "Take this and fish, young friend!$"
ViridianForest_Text_ReceivedOldRodFromFishingGuru::
    .string "{PLAYER} received an OLD ROD from\n"
    .string "the FISHING GURU.$"
ViridianForest_Text_FishingIsAWayOfLife::
    .string "Fishing is a way of life!\n"
    .string "It is like the finest poetry.\p"
    .string "From the seas to rivers, go out\n"
    .string "and land the big one, my friend.$"
ViridianForest_Text_OhThatsSoDisappointing::
    .string "Oh…\n"
    .string "That's so disappointing…$"
ViridianForest_Text_HowAreTheFishBiting::
    .string "Hello there, {PLAYER}!\p"
    .string "How are the fish biting?$"
ViridianForest_Text_NoRoomForNiceGift::
    .string "Oh, no!\p"
    .string "I had a nice gift for you, but you\n"
    .string "have no room for it!$"

ViridianForest_Text_FriendsItchingToBattle::
    .string "I came here with some friends to\n"
    .string "catch us some BUG POKéMON!\p"
    .string "They're all itching to get into\n"
    .string "some POKéMON battles!$"

ViridianForest_Text_RickIntro::
    .string "Hey! You have POKéMON!\n"
    .string "Come on!\l"
    .string "Let's battle 'em!$"

ViridianForest_Text_RickDefeat::
    .string "No!\nCATERPIE can't hack it!$"

ViridianForest_Text_RickPostBattle::
    .string "Ssh! You'll scare the bugs away.\n"
    .string "Another time, okay?$"

ViridianForest_Text_DougIntro::
    .string "Yo!\n"
    .string "You can't jam out if you're a\l"
    .string "POKéMON TRAINER!$"

ViridianForest_Text_DougDefeat::
    .string "Huh?\n"
    .string "I ran out of POKéMON!$"

ViridianForest_Text_DougPostBattle::
    .string "That totally stinks! I'm going to\n"
    .string "catch some stronger ones!$"

ViridianForest_Text_SammyIntro::
    .string "Hey, wait up!\n"
    .string "What's the hurry? Why the rush?$"

ViridianForest_Text_SammyDefeat::
    .string "I give!\n"
    .string "You're good at this!$"

ViridianForest_Text_SammyPostBattle::
    .string "Sometimes, you can find stuff on\n"
    .string "the ground.\p"
    .string "I'm looking for the stuff I\n"
    .string "dropped. Can you help me?$"

ViridianForest_Text_AnthonyIntro::
    .string "I might be little, but I won't like\n"
    .string "it if you go easy on me!$"

ViridianForest_Text_AnthonyDefeat::
    .string "Oh, boo.\n"
    .string "Nothing went right.$"

ViridianForest_Text_AnthonyPostBattle::
    .string "I lost some of my allowance…$"

ViridianForest_Text_CharlieIntro::
    .string "Did you know that POKéMON evolve?$"

ViridianForest_Text_CharlieDefeat::
    .string "Oh!\n"
    .string "I lost!$"

ViridianForest_Text_CharliePostBattle::
    .string "BUG POKéMON evolve quickly.\n"
    .string "They're a lot of fun!$"

ViridianForest_Text_RanOutOfPokeBalls::
    .string "I was throwing POKé BALLS to\n"
    .string "catch POKéMON, and I ran out.\p"
    .string "That's why you can never have too\n"
    .string "many POKé BALLS.$"

ViridianForest_Text_AvoidGrassyAreasWhenWeak::
    .string "TRAINER TIPS\p"
    .string "If your POKéMON are weak and you\n"
    .string "want to avoid battles, stay away\l"
    .string "from grassy areas!$"

ViridianForest_Text_UseAntidoteForPoison::
    .string "For poison, use ANTIDOTE!\n"
    .string "Get it at POKéMON MARTS!$"

ViridianForest_Text_ContactOakViaPCToRatePokedex::
    .string "TRAINER TIPS\p"
    .string "Contact PROF. OAK via a PC to\n"
    .string "get your POKéDEX evaluated!$"

ViridianForest_Text_CantCatchOwnedMons::
    .string "TRAINER TIPS\p"
    .string "You can't catch a POKéMON that\n"
    .string "belongs to someone else.\p"
    .string "Throw POKé BALLS only at wild\n"
    .string "POKéMON to catch them!$"

ViridianForest_Text_WeakenMonsBeforeCapture::
    .string "TRAINER TIPS\p"
    .string "Weaken POKéMON before attempting\n"
    .string "capture!\p"
    .string "When healthy, they may escape!$"

ViridianForest_Text_LeavingViridianForest::
    .string "LEAVING VIRIDIAN FOREST\n"
    .string "PEWTER CITY AHEAD$"


PetalburgCity_HerbShop_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, PetalburgCity_HerbShop_OnTransition
	map_script MAP_SCRIPT_ON_WARP_INTO_MAP_TABLE, PetalburgCity_HerbShop_OnWarp
	map_script MAP_SCRIPT_ON_FRAME_TABLE, PetalburgCity_HerbShop_OnFrame
	.byte 0

@ The owner's missions are in src/data/herb_shop.h (logic in src/herb_shop.c). The shop stays
@ closed, with only her in it, until the player has escorted her to the Hidden Grotto on
@ Route 101 (FLAG_HERB_SHOP_OPEN). After that she gives one mission a day.
@ HerbShop_SetupShop hides the objects with temp flags (FLAG_HIDE_HERB_SHOP_*): the clerk and
@ customers while the shop is closed, her while she waits on a route for an escort, and the
@ Shroomish and Paras until the player has brought them.
PetalburgCity_HerbShop_OnTransition:
	special HerbShop_SetupShop
	end

PetalburgCity_HerbShop_OnWarp:
	map_script_2 VAR_HERB_SHOP_ESCORT_RETURNED, 1, PetalburgCity_HerbShop_EventScript_TurnPlayerToOwner
	.2byte 0

PetalburgCity_HerbShop_EventScript_TurnPlayerToOwner::
	turnobject LOCALID_PLAYER, DIR_EAST
	end

PetalburgCity_HerbShop_OnFrame:
	map_script_2 VAR_HERB_SHOP_ESCORT_RETURNED, 1, PetalburgCity_HerbShop_EventScript_ReturnFromEscort
	.2byte 0

@ Back from the grotto (HerbShop_EventScript_EscortInGrotto), the player at (6,5) beside her.
PetalburgCity_HerbShop_EventScript_ReturnFromEscort::
	lockall
	setvar VAR_HERB_SHOP_ESCORT_RETURNED, 0
	msgbox PetalburgCity_HerbShop_Text_BackSafeAndSound, MSGBOX_DEFAULT
	specialvar VAR_0x8000, HerbShop_CompleteMission
	call PetalburgCity_HerbShop_EventScript_Reward
	goto_if_unset FLAG_HERB_SHOP_OPEN, PetalburgCity_HerbShop_EventScript_OpenShop
	msgbox PetalburgCity_HerbShop_Text_ComeBackTomorrow, MSGBOX_DEFAULT
	releaseall
	end

PetalburgCity_HerbShop_EventScript_OpenShop::
	msgbox PetalburgCity_HerbShop_Text_NowICanOpen, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	setflag FLAG_HERB_SHOP_OPEN
	clearflag FLAG_HIDE_HERB_SHOP_STAFF
	addobject LOCALID_HERB_SHOP_CLERK
	addobject LOCALID_HERB_SHOP_MAN
	addobject LOCALID_HERB_SHOP_BOY
	delay 30
	fadescreen FADE_FROM_BLACK
	msgbox PetalburgCity_HerbShop_Text_OpenForBusiness, MSGBOX_DEFAULT
	releaseall
	end

@ VAR_0x8000 = the mission's IP. With max IP as high as it gets she gives a herb instead.
PetalburgCity_HerbShop_EventScript_Reward::
	setvar VAR_0x8004, GAME_STAT_PLAYER_IP_MAX
	specialvar VAR_RESULT, GetGameStatValue
	goto_if_ge VAR_RESULT, 100, PetalburgCity_HerbShop_EventScript_RewardHerb
	call Common_EventScript_GainIP
	return

PetalburgCity_HerbShop_EventScript_RewardHerb::
	msgbox PetalburgCity_HerbShop_Text_NothingLeftToTeach, MSGBOX_DEFAULT
	giveitem ITEM_REVIVAL_HERB
	return

PetalburgCity_HerbShop_EventScript_Owner::
	lock
	faceplayer
	goto_if_unset FLAG_HERB_SHOP_OPEN, PetalburgCity_HerbShop_EventScript_ShopClosed
	specialvar VAR_RESULT, HerbShop_GetMission
	goto_if_eq VAR_RESULT, HERB_MISSION_CATCH, PetalburgCity_HerbShop_EventScript_CheckCatch
	goto_if_eq VAR_RESULT, HERB_MISSION_FETCH, PetalburgCity_HerbShop_EventScript_CheckFetch
	goto_if_set FLAG_DAILY_HERB_SHOP_MISSION, PetalburgCity_HerbShop_EventScript_DoneForToday
	msgbox PetalburgCity_HerbShop_Text_InterestedInMedicine, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PetalburgCity_HerbShop_EventScript_NotInterested
	specialvar VAR_RESULT, HerbShop_GiveMission
	goto_if_eq VAR_RESULT, HERB_MISSION_CATCH, PetalburgCity_HerbShop_EventScript_GiveCatch
	goto_if_eq VAR_RESULT, HERB_MISSION_FETCH, PetalburgCity_HerbShop_EventScript_GiveFetch
	msgbox PetalburgCity_HerbShop_Text_EscortMeToGrotto, MSGBOX_DEFAULT
	goto PetalburgCity_HerbShop_EventScript_LeaveForEscort
	end

PetalburgCity_HerbShop_EventScript_NotInterested::
	msgbox PetalburgCity_HerbShop_Text_IllBeHere, MSGBOX_DEFAULT
	release
	end

PetalburgCity_HerbShop_EventScript_DoneForToday::
	msgbox PetalburgCity_HerbShop_Text_ComeBackTomorrow, MSGBOX_DEFAULT
	release
	end

@ Before the shop opens: she asks for an escort to Route 101, again each time until it's a yes.
PetalburgCity_HerbShop_EventScript_ShopClosed::
	goto_if_set FLAG_HERB_SHOP_MET_OWNER, PetalburgCity_HerbShop_EventScript_AskAgain
	setflag FLAG_HERB_SHOP_MET_OWNER
	msgbox PetalburgCity_HerbShop_Text_WhateverAmIToDo, MSGBOX_YESNO
	goto PetalburgCity_HerbShop_EventScript_AnswerFirstEscort
	end

PetalburgCity_HerbShop_EventScript_AskAgain::
	msgbox PetalburgCity_HerbShop_Text_ChangedYourMind, MSGBOX_YESNO
PetalburgCity_HerbShop_EventScript_AnswerFirstEscort::
	goto_if_eq VAR_RESULT, NO, PetalburgCity_HerbShop_EventScript_DeclineFirstEscort
	special HerbShop_StartFirstEscort
	msgbox PetalburgCity_HerbShop_Text_IllMeetYouThere, MSGBOX_DEFAULT
	goto PetalburgCity_HerbShop_EventScript_LeaveForEscort
	end

PetalburgCity_HerbShop_EventScript_DeclineFirstEscort::
	msgbox PetalburgCity_HerbShop_Text_IUnderstand, MSGBOX_DEFAULT
	release
	end

@ She walks out of the door from (7,5), around the player if they stand below her.
PetalburgCity_HerbShop_EventScript_LeaveForEscort::
	closemessage
	call_if_eq VAR_FACING, DIR_NORTH, PetalburgCity_HerbShop_EventScript_OwnerLeaveAroundPlayer
	call_if_ne VAR_FACING, DIR_NORTH, PetalburgCity_HerbShop_EventScript_OwnerLeave
	playse SE_EXIT
	removeobject LOCALID_HERB_SHOP_OWNER
	release
	end

PetalburgCity_HerbShop_EventScript_OwnerLeave::
	applymovement LOCALID_HERB_SHOP_OWNER, PetalburgCity_HerbShop_Movement_OwnerLeave
	waitmovement 0
	return

PetalburgCity_HerbShop_EventScript_OwnerLeaveAroundPlayer::
	applymovement LOCALID_HERB_SHOP_OWNER, PetalburgCity_HerbShop_Movement_OwnerLeaveAroundPlayer
	waitmovement 0
	return

PetalburgCity_HerbShop_Movement_OwnerLeave:
	walk_down
	walk_left
	walk_left
	walk_left
	walk_down
	set_invisible
	step_end

PetalburgCity_HerbShop_Movement_OwnerLeaveAroundPlayer:
	walk_left
	walk_left
	walk_left
	walk_down
	walk_down
	set_invisible
	step_end

PetalburgCity_HerbShop_EventScript_GiveCatch::
	msgbox PetalburgCity_HerbShop_Text_CatchMeOne, MSGBOX_DEFAULT
	release
	end

PetalburgCity_HerbShop_EventScript_GiveFetch::
	msgbox PetalburgCity_HerbShop_Text_BringMeIngredients, MSGBOX_DEFAULT
	release
	end

@ A catch mission: STR_VAR_1 = the Pokemon, STR_VAR_2 = where to find one.
PetalburgCity_HerbShop_EventScript_CheckCatch::
	specialvar VAR_RESULT, HerbShop_CanCompleteMission
	goto_if_eq VAR_RESULT, FALSE, PetalburgCity_HerbShop_EventScript_StillNeedMon
	msgbox PetalburgCity_HerbShop_Text_MayIHaveIt, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PetalburgCity_HerbShop_EventScript_KeepMon
	special ChoosePartyMon
	lock
	faceplayer
	special HerbShop_GetMission
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, PetalburgCity_HerbShop_EventScript_KeepMon
	specialvar VAR_RESULT, HerbShop_GiveChosenMon
	goto_if_eq VAR_RESULT, HERB_GIVE_MON_WRONG, PetalburgCity_HerbShop_EventScript_WrongMon
	goto_if_eq VAR_RESULT, HERB_GIVE_MON_LAST, PetalburgCity_HerbShop_EventScript_LastMon
	callnative UpdateFollowingPokemon
	msgbox PetalburgCity_HerbShop_Text_IllTakeGoodCareOfIt, MSGBOX_DEFAULT
	closemessage
	@ The Pokemon joins the shop: VAR_0x8004 = its object, VAR_0x8005 = its species
	specialvar VAR_0x8000, HerbShop_CompleteMission
	fadescreen FADE_TO_BLACK
	addobject VAR_0x8004
	fadescreen FADE_FROM_BLACK
	playmoncry VAR_0x8005, CRY_MODE_NORMAL
	msgbox PetalburgCity_HerbShop_Text_LooksRightAtHome, MSGBOX_DEFAULT
	waitmoncry
	call PetalburgCity_HerbShop_EventScript_Reward
	goto PetalburgCity_HerbShop_EventScript_DoneForToday
	end

PetalburgCity_HerbShop_EventScript_StillNeedMon::
	msgbox PetalburgCity_HerbShop_Text_FoundOneYet, MSGBOX_DEFAULT
	release
	end

PetalburgCity_HerbShop_EventScript_KeepMon::
	msgbox PetalburgCity_HerbShop_Text_LetMeKnow, MSGBOX_DEFAULT
	release
	end

PetalburgCity_HerbShop_EventScript_WrongMon::
	msgbox PetalburgCity_HerbShop_Text_ThatIsntOne, MSGBOX_DEFAULT
	release
	end

PetalburgCity_HerbShop_EventScript_LastMon::
	msgbox PetalburgCity_HerbShop_Text_CouldntTakeYourOnlyMon, MSGBOX_DEFAULT
	release
	end

@ A fetch mission: STR_VAR_1 to 3 = the ingredients.
PetalburgCity_HerbShop_EventScript_CheckFetch::
	specialvar VAR_RESULT, HerbShop_CanCompleteMission
	goto_if_eq VAR_RESULT, FALSE, PetalburgCity_HerbShop_EventScript_StillNeedIngredients
	msgbox PetalburgCity_HerbShop_Text_MayIHaveIngredients, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, PetalburgCity_HerbShop_EventScript_KeepIngredients
	special HerbShop_GiveIngredients
	msgbox PetalburgCity_HerbShop_Text_HandedOverIngredients, MSGBOX_DEFAULT
	msgbox PetalburgCity_HerbShop_Text_WatchClosely, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playse SE_M_HEAL_BELL
	delay 60
	fadescreen FADE_FROM_BLACK
	specialvar VAR_0x8000, HerbShop_CompleteMission
	call PetalburgCity_HerbShop_EventScript_Reward
	goto PetalburgCity_HerbShop_EventScript_DoneForToday
	end

PetalburgCity_HerbShop_EventScript_StillNeedIngredients::
	msgbox PetalburgCity_HerbShop_Text_StillNeedIngredients, MSGBOX_DEFAULT
	release
	end

PetalburgCity_HerbShop_EventScript_KeepIngredients::
	msgbox PetalburgCity_HerbShop_Text_LetMeKnow, MSGBOX_DEFAULT
	release
	end

@ The Pokemon the player caught for her
PetalburgCity_HerbShop_EventScript_Shroomish::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_SHROOMISH, CRY_MODE_NORMAL
	msgbox PetalburgCity_HerbShop_Text_Shroomish, MSGBOX_DEFAULT
	waitmoncry
	release
	end

PetalburgCity_HerbShop_EventScript_Paras::
	lock
	faceplayer
	waitse
	playmoncry SPECIES_PARAS, CRY_MODE_NORMAL
	msgbox PetalburgCity_HerbShop_Text_Paras, MSGBOX_DEFAULT
	waitmoncry
	release
	end

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

PetalburgCity_HerbShop_EventScript_Boy::
	msgbox PetalburgCity_HerbShop_Text_RepelIsUseful, MSGBOX_NPC
	end

PetalburgCity_HerbShop_EventScript_Man::
	msgbox PetalburgCity_HerbShop_Text_TakeSomeAntidotesWithYou, MSGBOX_NPC
	end

PetalburgCity_HerbShop_Text_WhateverAmIToDo:
	.string "Oh, I'm sorry, dear. I'm the owner\n"
	.string "of this HERB SHOP, but I can't\l"
	.string "open the store today.\p"
	.string "My shipment of herbs never\n"
	.string "arrived… Whatever am I to do?\p"
	.string "I could gather herbs myself from\n"
	.string "the hidden grotto on ROUTE 101…\p"
	.string "But I don't have any POKéMON,\n"
	.string "and the tall grass is no place\l"
	.string "to walk alone.\p"
	.string "You're a TRAINER, aren't you?\n"
	.string "Would you be my escort?$"

PetalburgCity_HerbShop_Text_ChangedYourMind:
	.string "Oh, it's you, dear!\n"
	.string "Have you changed your mind?\p"
	.string "Will you escort me to the hidden\n"
	.string "grotto on ROUTE 101?$"

PetalburgCity_HerbShop_Text_IUnderstand:
	.string "I understand, dear…\p"
	.string "But I just don't know what I'm\n"
	.string "going to do about the shop…$"

PetalburgCity_HerbShop_Text_IllMeetYouThere:
	.string "Oh, thank you so much, dear!\p"
	.string "I'll wait for you on {STR_VAR_1},\n"
	.string "{STR_VAR_2}.$"

PetalburgCity_HerbShop_Text_BackSafeAndSound:
	.string "Back safe and sound, and it's\n"
	.string "all thanks to you, dear!\p"
	.string "I hope you picked up a thing or\n"
	.string "two about herbs along the way.$"

PetalburgCity_HerbShop_Text_NothingLeftToTeach:
	.string "There's nothing left that I could\n"
	.string "teach you about medicine, dear.\p"
	.string "Please take this instead.$"

PetalburgCity_HerbShop_Text_NowICanOpen:
	.string "With these herbs, I can finally\n"
	.string "open the shop!$"

PetalburgCity_HerbShop_Text_OpenForBusiness:
	.string "The HERB SHOP is open for business!\p"
	.string "If you're interested in medicine,\n"
	.string "come and talk to me, dear.\p"
	.string "You could learn a thing or two\n"
	.string "by helping out around here.$"

PetalburgCity_HerbShop_Text_InterestedInMedicine:
	.string "Hello, dear! Are you interested\n"
	.string "in medicine?\p"
	.string "You could learn a thing or two\n"
	.string "by helping out around here.\p"
	.string "Will you lend me a hand today?$"

PetalburgCity_HerbShop_Text_IllBeHere:
	.string "That's all right, dear. I'll be\n"
	.string "here if you change your mind.$"

PetalburgCity_HerbShop_Text_ComeBackTomorrow:
	.string "Thank you for your help today.\p"
	.string "Come back tomorrow, dear. I'm sure\n"
	.string "I'll have something else for you.$"

PetalburgCity_HerbShop_Text_EscortMeToGrotto:
	.string "Wonderful! I'm running low on\n"
	.string "{STR_VAR_3}, you see.\p"
	.string "There's a hidden grotto where I\n"
	.string "can gather some, but I'll need\l"
	.string "an escort.\p"
	.string "I'll wait for you on {STR_VAR_1},\n"
	.string "{STR_VAR_2}.$"

PetalburgCity_HerbShop_Text_CatchMeOne:
	.string "Wonderful! I've always wanted a\n"
	.string "{STR_VAR_1} for the shop.\p"
	.string "Its spores make for excellent\n"
	.string "medicine, you see.\p"
	.string "Could you catch one and bring it\n"
	.string "to me?\p"
	.string "{STR_VAR_2}$"

PetalburgCity_HerbShop_Text_FoundOneYet:
	.string "Have you found a {STR_VAR_1}\n"
	.string "for me yet, dear?\p"
	.string "{STR_VAR_2}$"

PetalburgCity_HerbShop_Text_MayIHaveIt:
	.string "Oh! You have a {STR_VAR_1}\n"
	.string "with you!\p"
	.string "May I have it for the shop?$"

PetalburgCity_HerbShop_Text_LetMeKnow:
	.string "Oh… Well, let me know if you\n"
	.string "change your mind, dear.$"

PetalburgCity_HerbShop_Text_ThatIsntOne:
	.string "Hm? That isn't a {STR_VAR_1},\n"
	.string "dear.$"

PetalburgCity_HerbShop_Text_CouldntTakeYourOnlyMon:
	.string "Oh, but you'd have no POKéMON\n"
	.string "left to keep you safe!\p"
	.string "I couldn't possibly take it.$"

PetalburgCity_HerbShop_Text_IllTakeGoodCareOfIt:
	.string "Thank you, dear!\n"
	.string "I'll take good care of it.$"

PetalburgCity_HerbShop_Text_LooksRightAtHome:
	.string "{STR_VAR_1} looks right at home!$"

PetalburgCity_HerbShop_Text_BringMeIngredients:
	.string "Wonderful! I'm mixing a new\n"
	.string "remedy, but I'm missing a few\l"
	.string "ingredients.\p"
	.string "Could you bring me these?\p"
	.string "{STR_VAR_1},\n"
	.string "{STR_VAR_2} and\l"
	.string "{STR_VAR_3}.$"

PetalburgCity_HerbShop_Text_StillNeedIngredients:
	.string "I still need these for my remedy:\p"
	.string "{STR_VAR_1},\n"
	.string "{STR_VAR_2} and\l"
	.string "{STR_VAR_3}.$"

PetalburgCity_HerbShop_Text_MayIHaveIngredients:
	.string "Oh, you've found everything!\n"
	.string "May I have the ingredients?$"

PetalburgCity_HerbShop_Text_HandedOverIngredients:
	.string "{PLAYER} handed over the\n"
	.string "ingredients.$"

PetalburgCity_HerbShop_Text_WatchClosely:
	.string "Now watch closely, dear. This is\n"
	.string "how a remedy is made…$"

PetalburgCity_HerbShop_Text_Shroomish:
	.string "SHROOMISH: Shroo shroo!$"

PetalburgCity_HerbShop_Text_Paras:
	.string "PARAS: Kshh, kshh!$"

PetalburgCity_HerbShop_Text_WelcomeToHerbShop:
	.string "Welcome to the HERB SHOP, home of\n"
	.string "effective and inexpensive medicine!$"

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

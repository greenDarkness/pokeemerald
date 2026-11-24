@ Escort missions of the Petalburg Herb Shop's owner (src/herb_shop.c, src/data/herb_shop.h).
@ She waits on a route (her object's flag is FLAG_HIDE_HERB_SHOP_ESCORT, updated by
@ special HerbShop_UpdateEscortNpc in the map's on-transition script), leads the player to the
@ Hidden Grotto there, finds her item inside and takes the player back to the shop, where
@ PetalburgCity_HerbShop_EventScript_ReturnFromEscort gives the reward.

HerbShop_EventScript_EscortOwner::
	lock
	faceplayer
	specialvar VAR_RESULT, HerbShop_UpdateEscortNpc
	goto_if_eq VAR_RESULT, FALSE, HerbShop_EventScript_EscortCalledOff
	msgbox HerbShop_Text_ThanksForComing, MSGBOX_DEFAULT
	closemessage
	hidefollower
	setvar VAR_0x8004, HERB_ESCORT_MOVE_WALK
	special HerbShop_EscortMove
	waitmovement VAR_LAST_TALKED
	waitmovement LOCALID_PLAYER
	faceplayer
	msgbox HerbShop_Text_GrottoIsRightHere, MSGBOX_DEFAULT
	closemessage
	setvar VAR_0x8004, HERB_ESCORT_MOVE_OWNER_ENTER
	special HerbShop_EscortMove
	waitmovement VAR_LAST_TALKED
	playse SE_EXIT
	removeobject VAR_LAST_TALKED
	setvar VAR_0x8004, HERB_ESCORT_MOVE_PLAYER_ENTER
	special HerbShop_EscortMove
	waitmovement LOCALID_PLAYER
	special GetFacingHiddenGrotto
	playse SE_EXIT
	special EnterHiddenGrotto
	releaseall
	end

@ The day ended while she was waiting: the mission is over (missions only last a day).
HerbShop_EventScript_EscortCalledOff::
	msgbox HerbShop_Text_HaveToGetBack, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	removeobject VAR_LAST_TALKED
	fadescreen FADE_FROM_BLACK
	release
	end

@ Continues the escort inside the grotto, from HiddenGrotto's on-frame table. The room is set
@ up by special HerbShop_SetupGrottoEscort: she is at (5,6), her item at (5,4), whatever the
@ grotto actually holds, and the player arrives on the exit at (5,7).
HerbShop_EventScript_EscortInGrotto::
	lockall
	setvar VAR_GROTTO_HERB_SHOP_ESCORT, 0
	special HerbShop_GetMission
	applymovement LOCALID_PLAYER, Common_Movement_WalkInPlaceFasterUp
	waitmovement 0
	playse SE_PIN
	applymovement LOCALID_GROTTO_HERB_SHOP_OWNER, Common_Movement_ExclamationMark
	waitmovement 0
	applymovement LOCALID_GROTTO_HERB_SHOP_OWNER, Common_Movement_Delay48
	waitmovement 0
	msgbox HerbShop_Text_LookOverThere, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_GROTTO_HERB_SHOP_OWNER, HerbShop_Movement_OwnerApproachItem
	applymovement LOCALID_PLAYER, HerbShop_Movement_PlayerApproachItem
	waitmovement 0
	waitmovement LOCALID_GROTTO_HERB_SHOP_OWNER
	delay 20
	playse SE_SUCCESS
	removeobject LOCALID_GROTTO_ITEM
	msgbox HerbShop_Text_ExactlyWhatINeeded, MSGBOX_DEFAULT
	closemessage
	applymovement LOCALID_GROTTO_HERB_SHOP_OWNER, Common_Movement_WalkInPlaceFasterLeft
	waitmovement 0
	msgbox HerbShop_Text_LetsHeadBack, MSGBOX_DEFAULT
	closemessage
	special HerbShop_FinishGrottoEscort
	@ Next to her in the shop: PetalburgCity_HerbShop_EventScript_ReturnFromEscort
	warp MAP_PETALBURG_CITY_HERB_SHOP, 6, 5
	waitstate
	releaseall
	end

@ She walks from (5,6) up to the item, the player from (5,7) to her left side at (4,5).
HerbShop_Movement_OwnerApproachItem:
	walk_up
	step_end

HerbShop_Movement_PlayerApproachItem:
	walk_up
	walk_left
	walk_up
	walk_in_place_faster_right
	step_end

HerbShop_Text_ThanksForComing:
	.string "Thank you for coming, dear!\p"
	.string "The hidden grotto isn't far.\n"
	.string "Stay close to me, please!$"

HerbShop_Text_GrottoIsRightHere:
	.string "Here we are! It's easy to miss\n"
	.string "if you don't know where to look.$"

HerbShop_Text_HaveToGetBack:
	.string "Oh my, look at the time!\p"
	.string "I'm sorry, dear. I have to get\n"
	.string "back to the shop.\p"
	.string "Perhaps another day!$"

HerbShop_Text_LookOverThere:
	.string "Oh! Look over there!$"

HerbShop_Text_ExactlyWhatINeeded:
	.string "{STR_VAR_3}!\p"
	.string "These are exactly what I needed!$"

HerbShop_Text_LetsHeadBack:
	.string "Thank you for keeping me safe.\n"
	.string "Let's head back to the shop!$"

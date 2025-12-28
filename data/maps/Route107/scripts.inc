Route107_MapScripts::
	.byte 0

Route107_EventScript_Darrin::
	trainerbattle_single TRAINER_DARRIN, Route107_Text_DarrinIntro, Route107_Text_DarrinDefeated
	msgbox Route107_Text_DarrinPostBattle, MSGBOX_AUTOCLOSE
	end

Route107_EventScript_Tony::
	trainerbattle_single TRAINER_TONY_1, Route107_Text_TonyIntro, Route107_Text_TonyDefeated, Route107_EventScript_TonyRegisterMatchCallAfterBattle
	specialvar VAR_RESULT, ShouldTryRematchBattle
	goto_if_eq VAR_RESULT, TRUE, Route107_EventScript_TonyRematch
	msgbox Route107_Text_TonyPostBattle, MSGBOX_DEFAULT
	release
	end

Route107_EventScript_TonyRegisterMatchCallAfterBattle::
	special PlayerFaceTrainerAfterBattle
	waitmovement 0
	msgbox Route107_Text_TonyRegister, MSGBOX_DEFAULT
	register_matchcall TRAINER_TONY_1
	release
	end

Route107_EventScript_TonyRematch::
	trainerbattle_rematch TRAINER_TONY_1, Route107_Text_TonyRematchIntro, Route107_Text_TonyRematchDefeated
	msgbox Route107_Text_TonyRematchPostBattle, MSGBOX_AUTOCLOSE
	end

Route107_EventScript_Denise::
	trainerbattle_single TRAINER_DENISE, Route107_Text_DeniseIntro, Route107_Text_DeniseDefeated
	msgbox Route107_Text_DenisePostBattle, MSGBOX_AUTOCLOSE
	end

Route107_EventScript_Beth::
	trainerbattle_single TRAINER_BETH, Route107_Text_BethIntro, Route107_Text_BethDefeated
	msgbox Route107_Text_BethPostBattle, MSGBOX_AUTOCLOSE
	end

Route107_EventScript_Lisa::
	trainerbattle_double TRAINER_LISA_AND_RAY, Route107_Text_LisaIntro, Route107_Text_LisaDefeated, Route107_Text_LisaNotEnoughPokemon
	msgbox Route107_Text_LisaPostBattle, MSGBOX_AUTOCLOSE
	end

Route107_EventScript_Ray::
	trainerbattle_double TRAINER_LISA_AND_RAY, Route107_Text_RayIntro, Route107_Text_RayDefeated, Route107_Text_RayNotEnoughPokemon
	msgbox Route107_Text_RayPostBattle, MSGBOX_AUTOCLOSE
	end

Route107_EventScript_Camron::
	trainerbattle_single TRAINER_CAMRON, Route107_Text_CamronIntro, Route107_Text_CamronDefeated
	msgbox Route107_Text_CamronPostBattle, MSGBOX_AUTOCLOSE
	end

Route107_EventScript_GoodRodFisherman::
	lock
	faceplayer
	goto_if_set FLAG_RECEIVED_GOOD_ROD, Route107_EventScript_ReceivedGoodRod
	msgbox Route107_Text_YouAgreeGoodRodIsGood, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, Route107_EventScript_ReceiveGoodRod
	goto_if_eq VAR_RESULT, NO, Route107_EventScript_DeclineGoodRod
	end

Route107_EventScript_ReceiveGoodRod::
	msgbox Route107_Text_IdenticalMindsTakeThis, MSGBOX_DEFAULT
	giveitem ITEM_GOOD_ROD
	setflag FLAG_RECEIVED_GOOD_ROD
	msgbox Route107_Text_TryYourLuckFishing, MSGBOX_DEFAULT
	release
	end

Route107_EventScript_DeclineGoodRod::
	msgbox Route107_Text_DontYouLikeToFish, MSGBOX_DEFAULT
	release
	end

Route107_EventScript_ReceivedGoodRod::
	msgbox Route107_Text_TryCatchingMonWithGoodRod, MSGBOX_DEFAULT
	release
	end


Route107_Text_YouAgreeGoodRodIsGood:
	.string "Hmm!\n"
	.string "A GOOD ROD is really good!\p"
	.string "Wouldn't you agree?$"

Route107_Text_IdenticalMindsTakeThis:
	.string "Hmm!\n"
	.string "We're of identical minds!\p"
	.string "Hmm!\n"
	.string "Take this GOOD ROD!$"

Route107_Text_TryYourLuckFishing:
	.string "Wherever there's water, try your luck\n"
	.string "at fishing.$"

Route107_Text_DontYouLikeToFish:
	.string "Don't you like to fish?$"

Route107_Text_TryCatchingMonWithGoodRod:
	.string "Try catching all sorts of POKéMON\n"
	.string "with your GOOD ROD.$"



Common_EventScript_MintMaster::
	lock
	faceplayer
	goto_if_set FLAG_MINT_MASTER_INTRO, Common_EventScript_MintMaster_Repeat
	msgbox Common_Text_MintMasterIntro, MSGBOX_DEFAULT
	setflag FLAG_MINT_MASTER_INTRO
	goto Common_EventScript_MintMaster_Main

Common_EventScript_MintMaster_Repeat::
	msgbox Common_Text_WhichPokemon, MSGBOX_DEFAULT
	goto Common_EventScript_MintMaster_Main

Common_EventScript_MintMaster_Main::
	special ChooseMonForNatureChange
	waitstate
	lock
	faceplayer
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, Common_EventScript_MintMaster_Decline
	goto Common_EventScript_CategoryMenu

Common_EventScript_CategoryMenu::
	multichoice 0, 0, MULTI_NATURE_CATEGORY, FALSE
	switch VAR_RESULT
	case 0, Common_EventScript_AttackMenu
	case 1, Common_EventScript_DefenseMenu
	case 2, Common_EventScript_SpeedMenu
	case 3, Common_EventScript_SpAtkMenu
	case 4, Common_EventScript_SpDefMenu
	case 5, Common_EventScript_NeutralMenu
	case MULTI_B_PRESSED, Common_EventScript_MintMaster_Main
	end

Common_EventScript_AttackMenu::
	multichoice 0, 0, MULTI_NATURE_ATTACK, FALSE
	switch VAR_RESULT
	case 0, Common_EventScript_SetAdamant
	case 1, Common_EventScript_SetBrave
	case 2, Common_EventScript_SetLonely
	case 3, Common_EventScript_SetNaughty
	case MULTI_B_PRESSED, Common_EventScript_CategoryMenu
	end

Common_EventScript_DefenseMenu::
	multichoice 0, 0, MULTI_NATURE_DEFENSE, FALSE
	switch VAR_RESULT
	case 0, Common_EventScript_SetBold
	case 1, Common_EventScript_SetImpish
	case 2, Common_EventScript_SetLax
	case 3, Common_EventScript_SetRelaxed
	case MULTI_B_PRESSED, Common_EventScript_CategoryMenu
	end

Common_EventScript_SpeedMenu::
	multichoice 0, 0, MULTI_NATURE_SPEED, FALSE
	switch VAR_RESULT
	case 0, Common_EventScript_SetHasty
	case 1, Common_EventScript_SetJolly
	case 2, Common_EventScript_SetNaive
	case 3, Common_EventScript_SetTimid
	case MULTI_B_PRESSED, Common_EventScript_CategoryMenu
	end

Common_EventScript_SpAtkMenu::
	multichoice 0, 0, MULTI_NATURE_SPATK, FALSE
	switch VAR_RESULT
	case 0, Common_EventScript_SetMild
	case 1, Common_EventScript_SetModest
	case 2, Common_EventScript_SetQuiet
	case 3, Common_EventScript_SetRash
	case MULTI_B_PRESSED, Common_EventScript_CategoryMenu
	end

Common_EventScript_SpDefMenu::
	multichoice 0, 0, MULTI_NATURE_SPDEF, FALSE
	switch VAR_RESULT
	case 0, Common_EventScript_SetCalm
	case 1, Common_EventScript_SetCareful
	case 2, Common_EventScript_SetGentle
	case 3, Common_EventScript_SetSassy
	case MULTI_B_PRESSED, Common_EventScript_CategoryMenu
	end

Common_EventScript_NeutralMenu::
	multichoice 0, 0, MULTI_NATURE_NEUTRAL, FALSE
	switch VAR_RESULT
	case 0, Common_EventScript_SetBashful
	case 1, Common_EventScript_SetDocile
	case 2, Common_EventScript_SetHardy
	case 3, Common_EventScript_SetQuirky
	case 4, Common_EventScript_SetSerious
	case MULTI_B_PRESSED, Common_EventScript_CategoryMenu
	end

Common_EventScript_SetAdamant::
	setvar VAR_0x8005, NATURE_ADAMANT
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetBrave::
	setvar VAR_0x8005, NATURE_BRAVE
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetLonely::
	setvar VAR_0x8005, NATURE_LONELY
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetNaughty::
	setvar VAR_0x8005, NATURE_NAUGHTY
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetBold::
	setvar VAR_0x8005, NATURE_BOLD
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetImpish::
	setvar VAR_0x8005, NATURE_IMPISH
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetLax::
	setvar VAR_0x8005, NATURE_LAX
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetRelaxed::
	setvar VAR_0x8005, NATURE_RELAXED
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetHasty::
	setvar VAR_0x8005, NATURE_HASTY
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetJolly::
	setvar VAR_0x8005, NATURE_JOLLY
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetNaive::
	setvar VAR_0x8005, NATURE_NAIVE
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetTimid::
	setvar VAR_0x8005, NATURE_TIMID
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetMild::
	setvar VAR_0x8005, NATURE_MILD
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetModest::
	setvar VAR_0x8005, NATURE_MODEST
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetQuiet::
	setvar VAR_0x8005, NATURE_QUIET
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetRash::
	setvar VAR_0x8005, NATURE_RASH
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetCalm::
	setvar VAR_0x8005, NATURE_CALM
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetCareful::
	setvar VAR_0x8005, NATURE_CAREFUL
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetGentle::
	setvar VAR_0x8005, NATURE_GENTLE
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetSassy::
	setvar VAR_0x8005, NATURE_SASSY
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetBashful::
	setvar VAR_0x8005, NATURE_BASHFUL
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetDocile::
	setvar VAR_0x8005, NATURE_DOCILE
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetHardy::
	setvar VAR_0x8005, NATURE_HARDY
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetQuirky::
	setvar VAR_0x8005, NATURE_QUIRKY
	goto Common_EventScript_ConfirmNature

Common_EventScript_SetSerious::
	setvar VAR_0x8005, NATURE_SERIOUS
	goto Common_EventScript_ConfirmNature

Common_EventScript_ConfirmNature::
	bufferpartymonnick STR_VAR_3, VAR_0x8004
	special GetNatureName
	copyvar VAR_RESULT, VAR_0x8005
	special ChangePokemonNature
	delay 30
	playfanfare MUS_OBTAIN_ITEM
	msgbox Common_Text_NatureChanged, MSGBOX_DEFAULT
	waitfanfare
	release
	end

Common_EventScript_MintMaster_Decline::
	msgbox Common_Text_ComeBackAnytime, MSGBOX_DEFAULT
	release
	end

Common_Text_MintMasterIntro:
	.string "Hello, I'm the Mint Master!\p"
	.string "I can feed your POKéMON an exotic mint\n"
	.string "that will alter its nature!$"

Common_Text_WhichPokemon:
	.string "Which POKéMON's nature needs\n"
	.string "adjusting?$"

Common_Text_NatureChanged:
	.string "Done! {STR_VAR_3}'s nature has been\n"
	.string "changed to {STR_VAR_1}!$"

Common_Text_ComeBackAnytime:
	.string "Come back anytime!$"

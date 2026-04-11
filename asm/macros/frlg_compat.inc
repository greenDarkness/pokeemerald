@ Compatibility macros and constants for FRLG (pokeorigin) map scripts
@ These stubs allow Kanto map scripts to assemble in pokeemerald.

@ ===== FRLG Constants =====

@ Seagallop destinations
.equiv SEAGALLOP_VERMILION_CITY,  0
.equiv SEAGALLOP_ONE_ISLAND,      1
.equiv SEAGALLOP_TWO_ISLAND,      2
.equiv SEAGALLOP_THREE_ISLAND,    3
.equiv SEAGALLOP_FOUR_ISLAND,     4
.equiv SEAGALLOP_FIVE_ISLAND,     5
.equiv SEAGALLOP_SIX_ISLAND,      6
.equiv SEAGALLOP_SEVEN_ISLAND,    7
.equiv SEAGALLOP_CINNABAR_ISLAND, 8
.equiv SEAGALLOP_NAVEL_ROCK,      9
.equiv SEAGALLOP_BIRTH_ISLAND,    10
.equiv SEAGALLOP_MORE,            254

@ In-game trade IDs (only define those not already in pokeemerald constants/trade.h)
@ INGAME_TRADE_MR_MIME already defined in pokeemerald as 4
.equiv INGAME_TRADE_JYNX,     1
.equiv INGAME_TRADE_NIDORAN,   2
.equiv INGAME_TRADE_FARFETCHD, 3
.equiv INGAME_TRADE_NIDORINOA, 4
.equiv INGAME_TRADE_LICKITUNG, 5
.equiv INGAME_TRADE_ELECTRODE, 6
.equiv INGAME_TRADE_TANGELA,   7
.equiv INGAME_TRADE_SEEL,      8

@ List menu IDs
.equiv LISTMENU_SILPHCO_FLOORS, 1
.equiv LISTMENU_BERRY_POWDER,   5

@ Help system context
.equiv HELPCONTEXT_BEDROOM_PC, 32

@ Menu cancel constant
.equiv SCR_MENU_CANCEL, 127

@ Trainer Tower challenge status (FRLG values, different from Emerald's battle frontier)
.equiv CHALLENGE_STATUS_UNK,    1
.equiv CHALLENGE_STATUS_NORMAL, 2

@ Multichoice IDs (FRLG-specific, won't display correct options in Emerald)
.equiv MULTICHOICE_YES_NO_INFO,         16
.equiv MULTICHOICE_YES_NO_INFO_2,       18
.equiv MULTICHOICE_ROOFTOP_B1F,         20
.equiv MULTICHOICE_TRAINER_TOWER_MODE,  64

@ NPC text color constants
.equiv NPC_TEXT_COLOR_MALE,    0
.equiv NPC_TEXT_COLOR_FEMALE,  1
.equiv NPC_TEXT_COLOR_MON,     2
.equiv NPC_TEXT_COLOR_NEUTRAL, 3
.equiv NPC_TEXT_COLOR_DEFAULT, 255

@ Wonder News reward types
.equiv NEWS_REWARD_NONE,       0
.equiv NEWS_REWARD_RECV_SMALL, 1
.equiv NEWS_REWARD_RECV_BIG,   2
.equiv NEWS_REWARD_WAITING,    3
.equiv NEWS_REWARD_SENT_SMALL, 4
.equiv NEWS_REWARD_SENT_BIG,   5
.equiv NEWS_REWARD_AT_MAX,     6

@ Fame Checker persons
.equiv FAMECHECKER_OAK,       0
.equiv FAMECHECKER_DAISY,     1
.equiv FAMECHECKER_BROCK,     2
.equiv FAMECHECKER_MISTY,     3
.equiv FAMECHECKER_LTSURGE,   4
.equiv FAMECHECKER_ERIKA,     5
.equiv FAMECHECKER_KOGA,      6
.equiv FAMECHECKER_SABRINA,   7
.equiv FAMECHECKER_BLAINE,    8
.equiv FAMECHECKER_LORELEI,   9
.equiv FAMECHECKER_BRUNO,     10
.equiv FAMECHECKER_AGATHA,    11
.equiv FAMECHECKER_LANCE,     12
.equiv FAMECHECKER_GIOVANNI,  13
.equiv FAMECHECKER_BILL,      14
.equiv FAMECHECKER_MRFUJI,    15

@ Fame Checker pick states
.equiv FCPICKSTATE_NO_DRAW,    0
.equiv FCPICKSTATE_SILHOUETTE, 1
.equiv FCPICKSTATE_COLORED,    2

@ Trainer battle type (early rival) - not supported in Emerald, stub maps to single
.equiv TRAINER_BATTLE_EARLY_RIVAL, 0

@ FRLG-specific constants
.equiv RIVAL_BATTLE_TUTORIAL, 0
.equiv LISTMENU_BADGES, 0

@ Event script commands not present in Emerald
.equiv STD_RECEIVED_ITEM, 0
.equiv STD_PUT_ITEM_AWAY, 0

@ Trainer Tower function constants (not supported)
.equiv TRAINER_TOWER_FUNC_START_CHALLENGE, 0
.equiv TRAINER_TOWER_FUNC_SHOULD_WARP_TO_COUNTER, 0
.equiv TRAINER_TOWER_FUNC_RESUME_TIMER, 0
.equiv TRAINER_TOWER_FUNC_GET_NUM_FLOORS, 0
.equiv TRAINER_TOWER_FUNC_GET_CHALLENGE_STATUS, 0
.equiv TRAINER_TOWER_FUNC_GET_BEAT_CHALLENGE, 0

@ ===== FRLG Script Macro Stubs =====

@ famechecker - FRLG fame checker system; no-op in Emerald
	.macro famechecker person:req, index:req, function=0
	.endm

@ goto_if_questlog - FRLG quest log check; no-op in Emerald (no questlog)
	.macro goto_if_questlog dest:req
	.endm

@ giveitem_msg - give item with custom message; maps to giveitem in Emerald
	.macro giveitem_msg msg:req, item:req, amount=1, fanfare=MUS_LEVEL_UP
	setorcopyvar VAR_0x8000, \item
	setorcopyvar VAR_0x8001, \amount
	callstd STD_OBTAIN_ITEM
	.endm

@ msgreceiveditem - show received item message; maps to giveitem in Emerald
	.macro msgreceiveditem msg:req, item:req, amount=1, fanfare=MUS_LEVEL_UP
	setorcopyvar VAR_0x8000, \item
	setorcopyvar VAR_0x8001, \amount
	callstd STD_OBTAIN_ITEM
	.endm

@ putitemaway - remove item from bag; maps to removeitem in Emerald
	.macro putitemaway item:req, amount=1
	removeitem \item, \amount
	.endm

@ set_gym_trainers - FRLG gym trainer activation; no-op in Emerald
	.macro set_gym_trainers gym:req
	.endm

@ trainerbattle_earlyrival - FRLG early rival battle
@ Uses CONTINUE_SCRIPT_NO_INTRO: no intro text, continues script after battle (win or lose)
@ Script handles encounter music (MUS_ENCOUNTER_RIVAL), victory music is skipped for this class
	.macro trainerbattle_earlyrival trainer:req, flags:req, defeat_text:req, victory_text:req
	trainerbattle TRAINER_BATTLE_CONTINUE_SCRIPT_NO_INTRO, \trainer, 0, \defeat_text
	.endm

@ braillemessage_wait - show braille with wait; simplified for Emerald
	.macro braillemessage_wait text:req
	braillemessage \text
	waitbuttonpress
	closemessage
	.endm

@ ===== FRLG Movement Macros =====

@ walk_slower variants - alias to walk_slow in Emerald
	.macro walk_slower_down
	walk_slow_down
	.endm

	.macro walk_slower_up
	walk_slow_up
	.endm

	.macro walk_slower_left
	walk_slow_left
	.endm

	.macro walk_slower_right
	walk_slow_right
	.endm

@ ===== Trainer Tower Macros (not supported) =====

	.macro ttower_startchallenge
	.endm

	.macro ttower_shouldexit
	.endm

	.macro ttower_resumetimer
	.endm

	.macro ttower_getnumfloors
	.endm

	.macro ttower_getchallengestatus
	.endm

	.macro ttower_getbeatchallenge
	.endm

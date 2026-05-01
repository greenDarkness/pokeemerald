@ Script

FieldResearch_EventScript_Kanto::
	lock
	faceplayer
	goto_if_unset FLAG_SYS_POKEDEX_GET, FieldResearch_EventScript_Kanto_StudyAsOaksAide
    goto_if_set FLAG_FIELD_RESEARCH_INTRO_KANTO, FieldResearch_EventScript_Start
    goto_if_set FLAG_FIELD_RESEARCH_INTRO_JOHTO, FieldResearch_EventScript_Kanto_Intro
    goto_if_set FLAG_FIELD_RESEARCH_INTRO_HOENN, FieldResearch_EventScript_Kanto_Intro
	msgbox FieldResearch_Text_FirstIntro, MSGBOX_NPC
    closemessage
	setflag FLAG_FIELD_RESEARCH_INTRO_KANTO
	release
	end
FieldResearch_EventScript_Kanto_Intro::
    msgbox FieldResearch_Text_Kanto_Intro, MSGBOX_NPC
    closemessage
	setflag FLAG_FIELD_RESEARCH_INTRO_KANTO
	release
	end
FieldResearch_EventScript_Kanto_StudyAsOaksAide::
	msgbox FieldResearch_Text_Kanto_StudyAsOaksAide, MSGBOX_NPC
	closemessage
	release
	end

FieldResearch_EventScript_Johto::
	lock
	faceplayer
    goto_if_set FLAG_FIELD_RESEARCH_INTRO_JOHTO, FieldResearch_EventScript_Start
    goto_if_set FLAG_FIELD_RESEARCH_INTRO_KANTO, FieldResearch_EventScript_Johto_Intro
    goto_if_set FLAG_FIELD_RESEARCH_INTRO_HOENN, FieldResearch_EventScript_Johto_Intro
	msgbox FieldResearch_Text_FirstIntro, MSGBOX_NPC
    closemessage
	setflag FLAG_FIELD_RESEARCH_INTRO_JOHTO
	release
	end
FieldResearch_EventScript_Johto_Intro::
    msgbox FieldResearch_Text_Johto_Intro, MSGBOX_NPC
    closemessage
	setflag FLAG_FIELD_RESEARCH_INTRO_JOHTO
	release
	end
    
FieldResearch_EventScript_Hoenn::
	lock
	faceplayer
    goto_if_set FLAG_FIELD_RESEARCH_INTRO_HOENN, FieldResearch_EventScript_Start
    goto_if_set FLAG_FIELD_RESEARCH_INTRO_KANTO, FieldResearch_EventScript_Hoenn_Intro
    goto_if_set FLAG_FIELD_RESEARCH_INTRO_JOHTO, FieldResearch_EventScript_Hoenn_Intro
	msgbox FieldResearch_Text_FirstIntro, MSGBOX_NPC
    closemessage
	setflag FLAG_FIELD_RESEARCH_INTRO_HOENN
	release
	end
FieldResearch_EventScript_Hoenn_Intro::
    msgbox FieldResearch_Text_Hoenn_Intro, MSGBOX_NPC
    closemessage
	setflag FLAG_FIELD_RESEARCH_INTRO_HOENN
	release
	end

FieldResearch_EventScript_Start::
    goto_if_unset FLAG_EXPALL_RECEIVED, FieldResearch_EventScript_ExpAll
    goto_if_unset FLAG_GEODUD_RECEIVED, FieldResearch_EventScript_Geodud
    goto_if_unset FLAG_SMOKE_CLOAK_RECEIVED, FieldResearch_EventScript_SmokeCloak
	goto_if_unset FLAG_REPELLENT_FOG_RECEIVED, FieldResearch_EventScript_RepellentFog
    goto_if_unset FLAG_PAYROLL_RECEIVED, FieldResearch_EventScript_Payroll
    goto_if_unset FLAG_BONDING_CHIME_RECEIVED, FieldResearch_EventScript_BondingChime
    goto_if_unset FLAG_MACHO_GEAR_RECEIVED, FieldResearch_EventScript_MachoGear
    goto_if_unset FLAG_LUCKY_DOZEN_RECEIVED, FieldResearch_EventScript_LuckyDozen
    msgbox FieldResearch_Text_ResearchCompleted, MSGBOX_NPC
	closemessage
	release
	end

@ Research Task I

FieldResearch_EventScript_ExpAll::
    msgbox FieldResearch_Text_NeedExpShare, MSGBOX_NPC
	checkitem ITEM_EXP_SHARE, 6
	goto_if_eq VAR_RESULT, TRUE, FieldResearch_EventScript_ExpAll_Collected_Materials
    closemessage
	release
	end
FieldResearch_EventScript_ExpAll_Collected_Materials::
	call FieldResearch_EventScript_ExclaimIfYouHaveItem
	msgbox FieldResearch_Text_ExpAll_Collected_Materials, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, FieldResearch_EventScript_ExpAll_CompletedJob
	release
	end
FieldResearch_EventScript_ExpAll_CompletedJob::
	msgbox FieldResearch_Text_ExpAll_CompletedJob, MSGBOX_NPC
	removeitem ITEM_EXP_SHARE, 6
	call FieldResearch_EventScript_FaceAwayBuildDevice
	playfanfare MUS_OBTAIN_SYMBOL
	message FieldResearch_Text_ExpAll_Legendary
	waitmessage
	waitfanfare
	msgbox FieldResearch_Text_ExpAll_Received, MSGBOX_NPC
	waitmessage
	giveitem ITEM_EXP_ALL, 1
	setflag FLAG_EXPALL_RECEIVED
    closemessage
    release
	end

@ Research Task II

FieldResearch_EventScript_Geodud::
	goto_if_set FLAG_RDAIDE_GEODUD_CONTINUED, FieldResearch_EventScript_Geodud_Continued
	msgbox FieldResearch_Text_Geodud_NeedGeodude, MSGBOX_NPC
	closemessage
	checkpartyspecies SPECIES_GEODUDE
	goto_if_eq VAR_RESULT, TRUE, FieldResearch_EventScript_Geodud_Collected_Geodude
	closemessage
	release
	end
FieldResearch_EventScript_Geodud_Collected_Geodude::
	call FieldResearch_EventScript_ExclaimIfYouHaveItem
	setvar VAR_0x8004, SPECIES_GEODUDE
	specialvar VAR_RESULT, GetReleaseMonSpeciesInfo
	msgbox FieldResearch_Text_Geodud_Collected_Geodude, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, FieldResearch_EventScript_Geodud_End
	closemessage
	special ChoosePartyMon
	waitstate
	copyvar VAR_0x8005, VAR_0x8004
	lock
	faceplayer
	goto_if_eq VAR_0x8004, PARTY_NOTHING_CHOSEN, FieldResearch_EventScript_Geodud_End
	specialvar VAR_RESULT, GetReleaseMonSpecies
	goto_if_ne VAR_RESULT, SPECIES_GEODUDE, FieldResearch_EventScript_Geodud_WrongPokemon
	specialvar VAR_RESULT, GetReleaseMonLevel
	goto_if_lt VAR_RESULT, 25, FieldResearch_EventScript_Geodud_WrongLevel
	special ReleaseMonFromParty
	setflag FLAG_RDAIDE_GEODUD_CONTINUED
	msgbox FieldResearch_Text_CompletedJob, MSGBOX_DEFAULT
	closemessage
	goto FieldResearch_EventScript_Geodud_Continued
	end

FieldResearch_EventScript_Geodud_Continued::
	msgbox FieldResearch_Text_Geodud_NeedEverstone, MSGBOX_NPC
	closemessage
	checkitem ITEM_EVERSTONE, 25
	goto_if_eq VAR_RESULT, TRUE, FieldResearch_EventScript_Geodud_Collected_Everstones
	closemessage
	release
	end
FieldResearch_EventScript_Geodud_Collected_Everstones::
	call FieldResearch_EventScript_ExclaimIfYouHaveItem
	msgbox FieldResearch_Text_Geodud_Collected_Everstone, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, FieldResearch_EventScript_Geodud_CompletedJob
	closemessage
	release
	end

FieldResearch_EventScript_Geodud_CompletedJob::
	checkitem ITEM_EVERSTONE, 25
	goto_if_eq VAR_RESULT, FALSE, FieldResearch_EventScript_Geodud_NotEnoughEverstones
	removeitem ITEM_EVERSTONE, 25
	msgbox FieldResearch_Text_Geodud_CompletedJob, MSGBOX_NPC
	closemessage
	call FieldResearch_EventScript_FaceAwayBuildDevice
	playfanfare MUS_OBTAIN_SYMBOL
	message FieldResearch_Text_Geodud_Wondrous
	waitmessage
	waitfanfare
	msgbox FieldResearch_Text_Geodud_Received, MSGBOX_NPC
	waitmessage
	giveitem ITEM_GEO_DUD, 1
	setflag FLAG_GEODUD_RECEIVED
    closemessage
    release
	end

FieldResearch_EventScript_Geodud_WrongPokemon::
	msgbox FieldResearch_Text_Geodud_WrongPokemon, MSGBOX_DEFAULT
	closemessage
	release
	end
FieldResearch_EventScript_Geodud_WrongLevel::
	msgbox FieldResearch_Text_Geodud_WrongLevel, MSGBOX_DEFAULT
	closemessage
	release
	end
FieldResearch_EventScript_Geodud_NotEnoughEverstones::
	msgbox FieldResearch_Text_Geodud_NotEnoughEverstones, MSGBOX_DEFAULT
	closemessage
	release
	end
FieldResearch_EventScript_Geodud_End::
	closemessage
	release
	end

@ Research Task III

FieldResearch_EventScript_SmokeCloak::
	giveitem ITEM_SMOKE_CLOAK, 1
    setflag FLAG_SMOKE_CLOAK_RECEIVED
    release
	end

@ Research Task IV

FieldResearch_EventScript_RepellentFog::
	giveitem ITEM_REPELLENT_FOG, 1
    setflag FLAG_REPELLENT_FOG_RECEIVED
    release
	end

@ Research Task V

FieldResearch_EventScript_Payroll::
	giveitem ITEM_PAY_ROLL, 1
    setflag FLAG_PAYROLL_RECEIVED
    release
	end

@ Research Task VI

FieldResearch_EventScript_BondingChime::
	giveitem ITEM_BONDING_CHIME, 1
    setflag FLAG_BONDING_CHIME_RECEIVED
    release
	end

@ Research Task VII

FieldResearch_EventScript_MachoGear::
	giveitem ITEM_MACHO_GEAR, 1
    setflag FLAG_MACHO_GEAR_RECEIVED
    release
	end

@ Research Task VIII

FieldResearch_EventScript_LuckyDozen::
	giveitem ITEM_LUCKY_DOZEN, 1
    setflag FLAG_LUCKY_DOZEN_RECEIVED
    release
	end

@ Miscellaneous Scripts

FieldResearch_EventScript_ExclaimIfYouHaveItem::
	setvar VAR_0x8004, LOCALID_OAKS_LAB_AIDE2
	special NpcFacePlayerAndExclaim
	playse SE_PIN
	lock
	faceplayer
	delay 32
	return
FieldResearch_EventScript_FaceAwayBuildDevice::
	applymovement LOCALID_OAKS_LAB_AIDE2, FieldResearch_Movement_FaceAwayPlayer
	waitmovement 0
	playse SE_DEX_SEARCH
	waitse
	delay 32
	applymovement LOCALID_OAKS_LAB_AIDE2, FieldResearch_Movement_FacePlayer
	waitmovement 0
	delay 32
	return

@ Movement

FieldResearch_Movement_FaceAwayPlayer:
	face_away_player
	step_end
FieldResearch_Movement_FacePlayer:
	face_player
	step_end
	
@ Text

FieldResearch_Text_FirstIntro:
	.string "Oh? What was your name again?\n"
	.string "You're a new face among\l"
	.string "these white coats.\p"
	.string "Here at the lab, I work under\n"
	.string "the professor as the head of\l"
	.string "the R&D division.\p"
	.string "...I see, so you're a trainer\n"
	.string "who has been issued a POKEDEX.\p"
	.string "That means you are eligible to\n"
	.string "participate in the interregional\l"
	.string "field research program!\p"
    .string "Whenever you visit a regional lab,\n"
	.string "check in with the local R&D division.\p"
	.string "If there is work to be done outside\n"
	.string "the lab, scientists rely on experienced\l"
	.string "trainers to be our agents in the field.\p"
	.string "In other words, your help can bring about\n"
    .string "the development of new technologies!\p"
	.string "If you're interested in field work,\n"
    .string "I'm the guy to talk to.\p"
	.string "I'll have you out in the field\n"
    .string "in no time!$"
FieldResearch_Text_Kanto_Intro:
	.string "Oh? What was your name again?\n"
	.string "You're a new face among\l"
	.string "these white coats.\p"
	.string "Here at the lab, I work under\n"
	.string "the professor as the head of\l"
	.string "the R&D division.\p"
	.string "...I see, you're already a member\n"
    .string "of the interregional\l"
	.string "field research program!\p"
    .string "Great to meet you, {PLAYER}!\n"
    .string "If you're interested in work,\l"
    .string "I'm the guy to talk to.\p"
	.string "We'll have you out in the field\n"
    .string "in no time!$"
FieldResearch_Text_Johto_Intro:
	.string "Oh? What was your name again?\n"
	.string "You're a new face among\l"
	.string "these white coats.\p"
	.string "Here at the lab, I work under\n"
	.string "the professor as the head of\l"
	.string "the R&D division.\p"
	.string "...I see, you're already a member\n"
    .string "of the interregional\l"
	.string "field research program!\p"
    .string "Great to meet you, {PLAYER}!\n"
    .string "If you're interested in work,\l"
    .string "I'm the guy to talk to.\p"
	.string "We'll have you out in the field\n"
    .string "in no time!$"
FieldResearch_Text_Hoenn_Intro:
	.string "Oh? What was your name again?\n"
	.string "You're a new face among\l"
	.string "these white coats.\p"
	.string "Here at the lab, I work under\n"
	.string "the professor as the head of\l"
	.string "the R&D division.\p"
	.string "...I see, you're already a member\n"
    .string "of the interregional\l"
	.string "field research program!\p"
    .string "Great to meet you, {PLAYER}!\n"
    .string "If you're interested in work,\l"
    .string "I'm the guy to talk to.\p"
	.string "We'll have you out in the field\n"
    .string "in no time!$"
FieldResearch_Text_Kanto_StudyAsOaksAide:
    .string "I study POKéMON as PROF. OAK's\n"
    .string "AIDE.$"
FieldResearch_Text_ResearchCompleted:
    .string "{PLAYER}, You have been a\n"
    .string "tremendous help.\p"
    .string "When we start our next project,\n"
    .string "you know we will be counting on you!$"

FieldResearch_Text_NeedExpShare:
	.string "R&D AIDE: SO, are you ready for\n"
    .string "some field work?\p"
    .string "For your first assignment, I need your\n"
    .string "help developing a device that can\l"
	.string "improve your training regimen.\p"
	.string "In order to fabricate the device,\n"
	.string "I need you to bring me 6 EXP SHARE\l"
	.string "to use as materials.\p"
    .string "Unfortunately, the cost of those items\n"
    .string "is not in our budget.\p"
    .string "That is the challenge of this task.\n"
    .string "I know you will overcome it!$"
FieldResearch_Text_ExpAll_Collected_Materials:
	.string "R&D AIDE: Were you able to obtain\n"
	.string "6 EXP SHARE?$"
FieldResearch_Text_ExpAll_CompletedJob:
	.string "R&D AIDE: You did it!\n"
	.string "But how did you afford it?\p"
    .string "...on second thought, I don't\n"
	.string "need to know. Now hand them over!$"
FieldResearch_Text_ExpAll_Received:
    .string "It allows your entire party to gain\n"
	.string "EXP after battle, without occupying\l"
	.string "a POKéMON's held item slot.\p"
	.string "Let me know how it feels in the field!$"
	FieldResearch_Text_ExpAll_Legendary:
	.string "R&D AIDE: The legendary EXP ALL!$"

FieldResearch_Text_Geodud_NeedGeodude:
	.string "R&D AIDE: SO, are you ready for\n"
    .string "some more field work?\p"
    .string "For your next assignment, I need your\n"
    .string "help developing a device that\l"
	.string "nullifies evolution.\p"
	.string "In order to fabricate the device,\n"
	.string "I need you to bring me a GEODUDE\l"
	.string "that is at least level 25.\p"
    .string "Past that level, GEODUDE must\n"
    .string "wrestle with the decision to evolve.\p"
    .string "A GEODUDE in this state is critical\n"
    .string "for our next invention!$"
FieldResearch_Text_Geodud_NeedEverstone:
	.string "Let's move on to the next\n"
	.string "research assignment.\p"
	.string "Did you know that GEODUDE collect\n"
	.string "EVERSTONES? Could it be they are\l"
	.string "reluctant to evolve?\p"
    .string "Well, according to our research, a\n"
	.string "GEODUDE like the one you brought us\l"
	.string "may undergo a transformation\l"
	.string "in the presence of many EVERSTONES.\p"
	.string "If you help us collect 25 EVERSTONES,\n"
	.string "we can test the theory once and for all!$"
FieldResearch_Text_Geodud_Collected_Geodude:
	.string "R&D AIDE: Were you able to obtain\n"
	.string "a GEODUDE that is level 25 or higher?$"
FieldResearch_Text_Geodud_Collected_Everstone:
	.string "R&D AIDE: Were you able to collect\n"
	.string "25 EVERSTONES?$"
FieldResearch_Text_Geodud_CompletedJob:
	.string "R&D AIDE: Perfect! That makes 25!\n"
	.string "Now I'll just place these in\l"
    .string "proximity to the GEODUDE...$"
FieldResearch_Text_Geodud_Wondrous:
	.string "R&D AIDE: The wondrous GEODUD!$"
FieldResearch_Text_Geodud_Received:
    .string "It prevents your entire party from\n"
	.string "evolving after battle, without\l"
	.string "occupying a POKéMON's held item slot.\p"
	.string "Let me know how it feels in the field!$"
FieldResearch_Text_Geodud_WrongPokemon:
	.string "For this experiment, it has to be\n"
	.string "a GEODUDE. That won't work.$"
FieldResearch_Text_Geodud_WrongLevel:
	.string "For this experiment, it has to be\n"
	.string "LVL 25 or higher. That won't work.$"
FieldResearch_Text_Geodud_NotEnoughEverstones:
	.string "I need 25 EVERSTONEs.\l"
	.string "Come back once you have enough.$"

FieldResearch_Text_CompletedJob:
	.string "R&D AIDE: You did it! Great job!\n"
	.string "That fulfills the first half\l"
	.string "of the requirements.$"
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

FieldResearch_EventScript_ExpAll::
    msgbox FieldResearch_Text_NeedExpShare, MSGBOX_NPC
	checkitem ITEM_EXP_SHARE, 6
	goto_if_eq VAR_RESULT, TRUE, FieldResearch_EventScript_ExpAll_Collected_Materials
    closemessage
	release
	end
FieldResearch_EventScript_ExpAll_Collected_Materials::
	msgbox FieldResearch_Text_ExpAll_Collected_Materials, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, FieldResearch_EventScript_ExpAll_CompletedJob
	release
	end
FieldResearch_EventScript_ExpAll_CompletedJob::
	msgbox FieldResearch_Text_ExpAll_CompletedJob, MSGBOX_NPC
    removeitem ITEM_EXP_SHARE, 6
	giveitem ITEM_EXP_ALL, 1
	setflag FLAG_EXPALL_RECEIVED
	msgbox FieldResearch_Text_ExpAll_Received, MSGBOX_NPC
    closemessage
    release
	end

FieldResearch_EventScript_Geodud::
	giveitem ITEM_GEO_DUD, 1
    setflag FLAG_GEODUD_RECEIVED
    release
	end
FieldResearch_EventScript_SmokeCloak::
	giveitem ITEM_SMOKE_CLOAK, 1
    setflag FLAG_SMOKE_CLOAK_RECEIVED
    release
	end
FieldResearch_EventScript_RepellentFog::
	giveitem ITEM_REPELLENT_FOG, 1
    setflag FLAG_REPELLENT_FOG_RECEIVED
    release
	end
FieldResearch_EventScript_Payroll::
	giveitem ITEM_PAY_ROLL, 1
    setflag FLAG_PAYROLL_RECEIVED
    release
	end
FieldResearch_EventScript_BondingChime::
	giveitem ITEM_BONDING_CHIME, 1
    setflag FLAG_BONDING_CHIME_RECEIVED
    release
	end
FieldResearch_EventScript_MachoGear::
	giveitem ITEM_MACHO_GEAR, 1
    setflag FLAG_MACHO_GEAR_RECEIVED
    release
	end
FieldResearch_EventScript_LuckyDozen::
	giveitem ITEM_LUCKY_DOZEN, 1
    setflag FLAG_LUCKY_DOZEN_RECEIVED
    release
	end

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
    .string "For your first assignment, I want your\n"
    .string "help developing a device that should\l"
	.string "improve your training regimen.\p"
	.string "6 EXP SHARE should cover the\n"
	.string "material requirements.\p"
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
	.string "R&D AIDE: The legendary EXP ALL!\p"
    .string "It allows your entire party to gain\n"
	.string "EXP after battle, without occupying a\l"
	.string "POKéMON's held item slot.\p"
	.string "Let me know how it feels in the field!$"

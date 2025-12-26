EventScript_DebtGrunt::
    msgbox gText_DebtGrunt_Intro, MSGBOX_DEFAULT
    closemessage
    trainerbattle_single TRAINER_GRUNT_MAGMA_DEBT, gText_DebtGrunt_Intro, gText_DebtGrunt_Defeat
    msgbox gText_DebtGrunt_PostBattle, MSGBOX_AUTOCLOSE
    release
    end

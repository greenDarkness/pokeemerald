#ifndef GUARD_MOVE_RELEARNER_H
#define GUARD_MOVE_RELEARNER_H

void TeachMoveRelearnerMove(void);
void TeachEggMoveTutorMove(void);
void TeachPowerMoveTutorMove(void);
void SetMoveRelearnerExitCallback(void (*callback)(void));
void MoveRelearnerShowHideHearts(s32 move);

#endif //GUARD_MOVE_RELEARNER_H

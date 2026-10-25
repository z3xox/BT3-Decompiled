#ifndef BATTLE_BTL_ACT_J_H
#define BATTLE_BTL_ACT_J_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * End of the fighter action decision code (src/battle/btl_act_decide.c, 0x203168..0x204E78): transformation, fusion,
 * member switch, technique (blast 2 / ultimate) and skill (blast 1) requests, the hit reaction -> action table,
 * the story mode forced actions, and a handful of "which action id" selectors used by the handlers.
 *
 * Every "Check..." / "Queue..." function here only writes the follow-up queue (BtlAct_SetQueue) and returns how
 * many slots it wrote (0 = nothing); the caller makes the request with BtlAct_Request(chr, BtlAct_GetQueued(chr)).
 *
 * All structures are partial views local to this module; the fighter is BtlFighter (battle/btl_fighter.h).
 */

s32 BtlAct_QueueComboFinish(BtlFighter *chr);
s32 BtlAct_CanTransform(BtlFighter *chr, u32 index, s32 needBlast, s32 needAllowed);
s32 BtlAct_CheckTransformInput(BtlFighter *chr);
s32 BtlAct_QueueTransform(BtlFighter *chr, s32 index);
s32 BtlAct_CanFuse(BtlFighter *chr, s32 index, s32 needBlast, s32 needAllowed, s32 *partner);
s32 BtlAct_CheckFusionInput(BtlFighter *chr);
s32 BtlAct_QueueFusion(BtlFighter *chr, s32 index, s32 partner);
s32 BtlAct_CanSwitch(BtlFighter *chr, s32 needGauge, s32 needAllowed);
s32 BtlAct_CheckSwitchInput(BtlFighter *chr);
s32 BtlAct_QueueSwitch(BtlFighter *chr);
s32 BtlAct_GetTechniqueAction(BtlFighter *chr, s32 slot);
s32 BtlAct_CanUseTechnique(BtlFighter *chr, s32 slot);
s32 BtlAct_CheckTechniqueInput(BtlFighter *chr);
s32 BtlAct_GetSkillAction(BtlFighter *chr, s32 slot);
s32 BtlAct_CanUseSkill(BtlFighter *chr, u32 slot);
s32 BtlAct_CheckSkillInput(BtlFighter *chr);
s32 BtlAct_QueueReaction(BtlFighter *chr, u32 reaction);
s32 BtlAct_GetLandingAction(BtlFighter *chr, s32 slot, u32 kind);
s32 BtlAct_QueueFlag21Action(BtlFighter *chr);
s32 BtlAct_QueueFlag21ActionB(BtlFighter *chr);
s32 BtlAct_CheckRecoveryInput(BtlFighter *chr, s32 held);
s32 BtlAct_CheckStoryForced(BtlFighter *chr);
s32 BtlAct_GetDownAction(BtlFighter *chr);
s32 BtlAct_GetEvasionAttack(BtlFighter *chr);
s32 BtlAct_GetChainAction(BtlFighter *chr, u32 kind);
s32 BtlAct_GetIdleFollowUp(BtlFighter *chr);
s32 BtlAct_GetAction14or16(BtlFighter *chr);
s32 BtlAct_GetAction11or15(BtlFighter *chr);

#endif

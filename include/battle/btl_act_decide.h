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
 * All structures are partial views local to this module.
 */

/* Pose block (chr + 0x10, BtlChar_GetPos). */
typedef BtlFighterPose BtlActJPose;

/* First words of a member entry (BtlMember_Get / BtlMember_GetActive). */
typedef BtlMember BtlActJMember;

/* Gauge block of a member (BtlMember_GetGauge / BtlMember_GetActiveGauge). */
typedef BtlMemberGauge BtlActJGauge;

/* Form-change request (chr + 0x12CC), the same block btl_char_action.h calls BtlActForm. */
typedef BtlFighterForm BtlActJForm;

/* Fighter (0x1600 bytes). */
typedef BtlFighter BtlActJChr;

s32 BtlAct_QueueComboFinish(BtlActJChr *chr);
s32 BtlAct_CanTransform(BtlActJChr *chr, u32 index, s32 needBlast, s32 needAllowed);
s32 BtlAct_CheckTransformInput(BtlActJChr *chr);
s32 BtlAct_QueueTransform(BtlActJChr *chr, s32 index);
s32 BtlAct_CanFuse(BtlActJChr *chr, s32 index, s32 needBlast, s32 needAllowed, s32 *partner);
s32 BtlAct_CheckFusionInput(BtlActJChr *chr);
s32 BtlAct_QueueFusion(BtlActJChr *chr, s32 index, s32 partner);
s32 BtlAct_CanSwitch(BtlActJChr *chr, s32 needGauge, s32 needAllowed);
s32 BtlAct_CheckSwitchInput(BtlActJChr *chr);
s32 BtlAct_QueueSwitch(BtlActJChr *chr);
s32 BtlAct_GetTechniqueAction(BtlActJChr *chr, s32 slot);
s32 BtlAct_CanUseTechnique(BtlActJChr *chr, s32 slot);
s32 BtlAct_CheckTechniqueInput(BtlActJChr *chr);
s32 BtlAct_GetSkillAction(BtlActJChr *chr, s32 slot);
s32 BtlAct_CanUseSkill(BtlActJChr *chr, u32 slot);
s32 BtlAct_CheckSkillInput(BtlActJChr *chr);
s32 BtlAct_QueueReaction(BtlActJChr *chr, u32 reaction);
s32 BtlAct_GetLandingAction(BtlActJChr *chr, s32 slot, u32 kind);
s32 BtlAct_QueueFlag21Action(BtlActJChr *chr);
s32 BtlAct_QueueFlag21ActionB(BtlActJChr *chr);
s32 BtlAct_CheckRecoveryInput(BtlActJChr *chr, s32 held);
s32 BtlAct_CheckStoryForced(BtlActJChr *chr);
s32 BtlAct_GetDownAction(BtlActJChr *chr);
s32 BtlAct_GetEvasionAttack(BtlActJChr *chr);
s32 BtlAct_GetChainAction(BtlActJChr *chr, u32 kind);
s32 BtlAct_GetIdleFollowUp(BtlActJChr *chr);
s32 BtlAct_GetAction14or16(BtlActJChr *chr);
s32 BtlAct_GetAction11or15(BtlActJChr *chr);

#endif

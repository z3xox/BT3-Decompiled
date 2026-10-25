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
typedef struct BtlActJPose {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ Vec4 rot;       /* y = model yaw */
} BtlActJPose;

/* First words of a member entry (BtlMember_Get / BtlMember_GetActive). */
typedef struct BtlActJMember {
    /* 0x00 */ s32 chara;
    /* 0x04 */ s32 costume;
} BtlActJMember;

/* Gauge block of a member (BtlMember_GetGauge / BtlMember_GetActiveGauge). */
typedef struct BtlActJGauge {
    /* 0x00 */ s32 health;
    /* 0x04 */ u8 unk4[0x20 - 0x4];
    /* 0x20 */ s32 variant;
    /* 0x24 */ u8 unk24[0x30 - 0x24];
    /* 0x30 */ s32 bodyChanged;        /* non-zero: this member cannot be fused with */
    /* 0x34 */ s32 fused;
    /* 0x38 */ s32 skillLock[2]; /* > 0: skill slot 0 / 1 (blast 1) with flag 0x10 is locked */
    /* 0x40 */ u8 unk40[0x4C - 0x40];
    /* 0x4C */ s32 techLock[5];  /* indexed by technique slot (2..4 -> + 0x54..0x5C): > 0 = a flag-0x2000 technique is locked */
} BtlActJGauge;

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

#ifndef BATTLE_BTL_ACT_A_H
#define BATTLE_BTL_ACT_A_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, first group (src/battle/btl_act_1.c, 0x1E3158..0x1E6CC0).
 * The structures are partial views local to this module.
 */

/* Pose block (chr + 0x10, what BtlChar_GetPos returns). */
typedef BtlFighterPose BtlActAPose;

/* Latched attack parameters (chr + 0xCF8); same layout as BtlActAttack in btl_char_action.h. */
typedef BtlFighterAttack BtlActAAttack;

/* Battle object (what BtlChar_GetObj returns). */
typedef struct BtlActAObj {
    /* 0x000 */ u8 unk0[0xBDC];
    /* 0xBDC */ f32 unkBDC;    /* a length in frames at 60 Hz, added to the vanish attack's lead */
    /* 0xBE0 */ u8 unkBE0[0xCAC - 0xBE0];
    /* 0xCAC */ s8 hitCount;
    /* 0xCAD */ s8 hitNo;
} BtlActAObj;

/* Fighter (0x1600 bytes). */
typedef BtlFighter BtlActAChr;

s32 BtlAct_WaitHandler(BtlActAChr *chr, s32 phase);
f32 BtlAct_GetChargeRate(BtlActAChr *chr);
s32 BtlAct_StepCharge(BtlActAChr *chr);
void BtlAct_SetChargeFromAnim(BtlActAChr *chr);
void BtlAct_SetChargeAnimDuration(BtlActAChr *chr);
void BtlAct_SetChargeAnimStep(BtlActAChr *chr);
void BtlAct_CancelOnOppC9(BtlActAChr *chr, f32 after);
void BtlAct_PlayPickedVoice(BtlActAChr *chr);
s32 BtlAct_StartSub19B(BtlActAChr *chr);
void BtlAct_DecideAttackFollow(BtlActAChr *chr, BtlActAAttack *atk);
void BtlAct_RequestAttackEnd(BtlActAChr *chr, BtlActAAttack *atk);
s32 BtlAct_RushHandler(BtlActAChr *chr, s32 phase);
s32 BtlAct_RushAutoHandler(BtlActAChr *chr, s32 phase);
s32 BtlAct_RushRapidHandler(BtlActAChr *chr, s32 phase);
s32 BtlAct_SmashChargeHandler(BtlActAChr *chr, s32 phase);
s32 BtlAct_SmashFullHandler(BtlActAChr *chr, s32 phase);
s32 BtlAct_DashSmashHandler(BtlActAChr *chr, s32 phase);
s32 BtlAct_FlyingKickHandler(BtlActAChr *chr, s32 phase);
s32 BtlAct_LiftStrikeHandler(BtlActAChr *chr, s32 phase);
s32 BtlAct_SmashVanishHandler(BtlActAChr *chr, s32 phase);
s32 BtlAct_RushFinishHandler(BtlActAChr *chr, s32 phase);


/* ======== formerly btl_act_1_part2.h ======== */


#include "types.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, second slice (src/battle/btl_act_b.c, 0x1E6CC0..0x1EA5F8).
 * The handler protocol is described in include/battle/btl_char_action.h. The structures are
 * partial views local to this module: only the fields this file touches.
 */

/* Pose block (chr + 0x10, what BtlChar_GetPos returns); the same layout as BtlMovePose. */
typedef BtlFighterPose BtlActBPose;

/* Active member's gauge block (BtlMember_GetActiveGauge). */
typedef BtlMemberGauge BtlActBGauge;

/* Latched attack parameters (chr + 0xCF8); BtlActAttack in btl_char_action.h. */
typedef BtlFighterAttack BtlActBAttack;

/* The fighter's battle object (BtlChar_GetObj). */
typedef struct BtlActBObjSub {
    /* 0x00000 */ u8 unk0[0x18060];
    /* 0x18060 */ s32 contactFlags;   /* bit 0x40 tested when a launched / blown away fighter meets a wall */
} BtlActBObjSub;

typedef struct BtlActBObj {
    /* 0x0000 */ u8 unk0[0x1660];
    /* 0x1660 */ BtlActBObjSub *work;
} BtlActBObj;

/* Fighter (0x1600 bytes). */
typedef BtlFighter BtlActBChr;

/* Helpers other slices call. */
s32 BtlActB_NextVariant(BtlActBChr *chr, s32 n);
void BtlActB_SetReactionFlags(BtlActBChr *chr);
s32 BtlActB_PickDownAnim(BtlActBChr *chr, s32 cur);
void BtlActB_UpdateMashRate(BtlActBChr *chr, f32 *rate, s32 scaled);
s32 BtlActB_TickMemberChange(BtlActBChr *chr, s32 *frames);


#endif

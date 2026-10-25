#ifndef BATTLE_BTL_ACT_A_H
#define BATTLE_BTL_ACT_A_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, first group (src/battle/btl_act_1.c, 0x1E3158..0x1E6CC0).
 * The structures are partial views local to this module; the fighter is BtlFighter (battle/btl_fighter.h).
 */

/* Battle object (what BtlChar_GetObj returns). */
typedef struct BtlActAObj {
    /* 0x000 */ u8 unk0[0xBDC];
    /* 0xBDC */ f32 unkBDC;    /* a length in frames at 60 Hz, added to the vanish attack's lead */
    /* 0xBE0 */ u8 unkBE0[0xCAC - 0xBE0];
    /* 0xCAC */ s8 hitCount;
    /* 0xCAD */ s8 hitNo;
} BtlActAObj;

s32 BtlAct_WaitHandler(BtlFighter *chr, s32 phase);
f32 BtlAct_GetChargeRate(BtlFighter *chr);
s32 BtlAct_StepCharge(BtlFighter *chr);
void BtlAct_SetChargeFromAnim(BtlFighter *chr);
void BtlAct_SetChargeAnimDuration(BtlFighter *chr);
void BtlAct_SetChargeAnimStep(BtlFighter *chr);
void BtlAct_CancelOnOppC9(BtlFighter *chr, f32 after);
void BtlAct_PlayPickedVoice(BtlFighter *chr);
s32 BtlAct_StartSub19B(BtlFighter *chr);
void BtlAct_DecideAttackFollow(BtlFighter *chr, BtlFighterAttack *atk);
void BtlAct_RequestAttackEnd(BtlFighter *chr, BtlFighterAttack *atk);
s32 BtlAct_RushHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_RushAutoHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_RushRapidHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_SmashChargeHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_SmashFullHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_DashSmashHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_FlyingKickHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_LiftStrikeHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_SmashVanishHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_RushFinishHandler(BtlFighter *chr, s32 phase);


/* ======== formerly btl_act_1_part2.h ======== */


#include "types.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, second slice (src/battle/btl_act_b.c, 0x1E6CC0..0x1EA5F8).
 * The handler protocol is described in include/battle/btl_char_action.h. The structures are
 * partial views local to this module: only the fields this file touches. The fighter is BtlFighter.
 */

/* The fighter's battle object (BtlChar_GetObj). */
typedef struct BtlActBObjSub {
    /* 0x00000 */ u8 unk0[0x18060];
    /* 0x18060 */ s32 contactFlags;   /* bit 0x40 tested when a launched / blown away fighter meets a wall */
} BtlActBObjSub;

typedef struct BtlActBObj {
    /* 0x0000 */ u8 unk0[0x1660];
    /* 0x1660 */ BtlActBObjSub *work;
} BtlActBObj;

/* Helpers other slices call. */
s32 BtlActB_NextVariant(BtlFighter *chr, s32 n);
void BtlActB_SetReactionFlags(BtlFighter *chr);
s32 BtlActB_PickDownAnim(BtlFighter *chr, s32 cur);
void BtlActB_UpdateMashRate(BtlFighter *chr, f32 *rate, s32 scaled);
s32 BtlActB_TickMemberChange(BtlFighter *chr, s32 *frames);


#endif

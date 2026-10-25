#ifndef BATTLE_BTL_ACT_C_H
#define BATTLE_BTL_ACT_C_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, third slice (src/battle/btl_act_2.c, 0x1EA5F8..0x1EE058).
 * Handler protocol: include/battle/btl_char_action.h. The structures below are partial views
 * local to this module; the fighter is BtlFighter (battle/btl_fighter.h).
 */

s32 BtlAct_DownHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_GetUpHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_ActionE6(BtlFighter *chr, s32 phase);
s32 BtlAct_ActionE7(BtlFighter *chr, s32 phase);
s32 BtlAct_ActionE8E9(BtlFighter *chr, s32 phase);
s32 BtlAct_ActionEA(BtlFighter *chr, s32 phase);
s32 BtlAct_ActionEB(BtlFighter *chr, s32 phase);
s32 BtlAct_StunHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_AirStunHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_GrowSearchWindow(BtlFighter *chr, f32 limit);
s32 BtlAct_IsOppInSearchWindow(BtlFighter *chr);
s32 BtlAct_SearchHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_ChargeHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_GuardHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_Action39(BtlFighter *chr, s32 phase);
s32 BtlAct_Action3A(BtlFighter *chr, s32 phase);
s32 BtlAct_Action3B(BtlFighter *chr, s32 phase);
s32 BtlAct_Action3C(BtlFighter *chr, s32 phase);
s32 BtlAct_Action3D(BtlFighter *chr, s32 phase);
s32 BtlAct_Action3E(BtlFighter *chr, s32 phase);
s32 BtlAct_Action3F(BtlFighter *chr, s32 phase);
s32 BtlAct_Action40(BtlFighter *chr, s32 phase);
s32 BtlAct_Action41(BtlFighter *chr, s32 phase);
s32 BtlAct_Action42(BtlFighter *chr, s32 phase);
s32 BtlAct_Action43(BtlFighter *chr, s32 phase);
s32 BtlAct_PlaySubAnim(BtlFighter *chr, s32 anim);
s32 BtlAct_KiBlastHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_ChargedKiBlastHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_DashKiBlastHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_DashChargedKiBlastHandler(BtlFighter *chr, s32 phase);


/* ======== formerly btl_act_2_part2.h ======== */


#include "types.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, 0x1EE058..0x1F1930 (src/battle/btl_act_d.c).
 * The structures are partial views local to this module.
 */

s32 BtlAct_KiBlastB2(BtlFighter *chr, s32 phase);
s32 BtlAct_KiBlastChargeB3(BtlFighter *chr, s32 phase);
s32 BtlAct_KiVolley95(BtlFighter *chr, s32 phase);
void BtlAct_PlayLandFx(BtlFighter *chr, s32 hard);
s32 BtlAct_GetIdleMotion(BtlFighter *chr);
s32 BtlAct_NeutralHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_IdleOnceHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_MoveHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_CloseMoveHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_DashMoveHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_JumpStartHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_JumpAirHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_AscendHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_DescendHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_HopHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_FastAscendHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_FastDescendHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_HomingDashHandler(BtlFighter *chr, s32 phase);



/* ======== formerly btl_act_2_part3.h ======== */


#include "types.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, actions 0x1A..0x35 and 0xFA..0xFC (src/battle/btl_act_e.c,
 * 0x1F1930..0x1F5460). The structures are partial views local to this module.
 * Handler protocol: see include/battle/btl_char_action.h.
 */

/* Battle object of a fighter (BtlChar_GetObj). */
typedef struct BtlActEObj {
    /* 0x000 */ u8 unk0[0xBDC];
    /* 0xBDC */ f32 subLength;
} BtlActEObj;

/* Clash C state (roster + 0x7C; see BtlClashC in btl_char_flag.h). */
typedef struct BtlActEClashC {
    /* 0x00 */ s32 level;
    /* 0x04 */ s32 count;
    /* 0x08 */ s32 path;
    /* 0x0C */ s32 point;
    /* 0x10 */ s32 hold;
    /* 0x14 */ s32 pick;      /* 0..3: which of the four buttons (bits 27..30) is asked for */
    /* 0x18 */ s32 prevPick;
} BtlActEClashC;

/* Fighter roster (gBtlChars). */
typedef struct BtlActERoster {
    /* 0x00 */ s32 count;
    /* 0x04 */ BtlFighter *chars;
    /* 0x08 */ u8 unk8[0x7C - 0x8];
    /* 0x7C */ BtlActEClashC clashC;
} BtlActERoster;

s32 BtlAct_DragonDashHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_StepHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_StepInHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_VanishStepHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_VanishBehindHandler(BtlFighter *chr, s32 phase);
void BtlAct_RecoverHandler(BtlFighter *chr, s32 phase);
f32 BtlAct_PlayAttackPart(BtlFighter *chr, BtlFighterAttack *atk, s32 frame, f32 time);
void BtlAct_VanishAttackHandler(BtlFighter *chr, s32 phase);
void BtlAct_VanishAttackQuickHandler(BtlFighter *chr, s32 phase);
void BtlAct_WarpBehindAttackHandler(BtlFighter *chr, s32 phase);
void BtlAct_WarpAheadAttackHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_SlideToAttackHandler(BtlFighter *chr, s32 phase);
void BtlAct_RushToAttackHandler(BtlFighter *chr, s32 phase);
void BtlAct_HopBackAttackHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_CircleDashHandler(BtlFighter *chr, s32 phase);
void BtlAct_RushDashHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_VanishDashHandler(BtlFighter *chr, s32 phase);
void BtlAct_ClashBlowsHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_ClashVanishHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_ClashExchangeHandler(BtlFighter *chr, s32 phase);


#endif

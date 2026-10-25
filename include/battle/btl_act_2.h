#ifndef BATTLE_BTL_ACT_C_H
#define BATTLE_BTL_ACT_C_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, third slice (src/battle/btl_act_2.c, 0x1EA5F8..0x1EE058).
 * Handler protocol: include/battle/btl_char_action.h. The structures below are partial views
 * local to this module.
 */

/* Pose block (chr + 0x10, what BtlChar_GetPos returns). */
typedef BtlFighterPose BtlActCPose;

/* Active member's gauge block (what BtlMember_GetActiveGauge returns). */
typedef struct BtlActCGauge {
    /* 0x00 */ s32 hp;
    /* 0x04 */ s32 hpMax;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 ki;
    /* 0x10 */ s32 kiMax;
    /* 0x14 */ s32 blast;
    /* 0x18 */ s32 blastMax;
    /* 0x1C */ s32 maxPower;
    /* 0x20 */ s32 variant;     /* non-zero: getting up (0xE1) plays voice 0x20 / 0x21 once */
    /* 0x24 */ s32 lowHealth;
    /* 0x28 */ s32 lowHealthIdle;     /* non-zero: a downed fighter must mash to recover */
    /* 0x2C */ s32 variantVoiced;     /* that voice was played */
    /* 0x30 */ s32 bodyChanged;
} BtlActCGauge;

/* Member entry (what BtlMember_GetActive returns). */
typedef struct BtlActCMember {
    /* 0x00 */ u8 unk0[0xA0];
    /* 0xA0 */ s32 idleTalkCount;     /* counts action 0x43 */
} BtlActCMember;

/* Fighter (0x1600 bytes). */
typedef BtlFighter BtlActCChr;

s32 BtlAct_DownHandler(BtlActCChr *chr, s32 phase);
s32 BtlAct_GetUpHandler(BtlActCChr *chr, s32 phase);
s32 BtlAct_ActionE6(BtlActCChr *chr, s32 phase);
s32 BtlAct_ActionE7(BtlActCChr *chr, s32 phase);
s32 BtlAct_ActionE8E9(BtlActCChr *chr, s32 phase);
s32 BtlAct_ActionEA(BtlActCChr *chr, s32 phase);
s32 BtlAct_ActionEB(BtlActCChr *chr, s32 phase);
s32 BtlAct_StunHandler(BtlActCChr *chr, s32 phase);
s32 BtlAct_AirStunHandler(BtlActCChr *chr, s32 phase);
s32 BtlAct_GrowSearchWindow(BtlActCChr *chr, f32 limit);
s32 BtlAct_IsOppInSearchWindow(BtlActCChr *chr);
s32 BtlAct_SearchHandler(BtlActCChr *chr, s32 phase);
s32 BtlAct_ChargeHandler(BtlActCChr *chr, s32 phase);
s32 BtlAct_GuardHandler(BtlActCChr *chr, s32 phase);
s32 BtlAct_Action39(BtlActCChr *chr, s32 phase);
s32 BtlAct_Action3A(BtlActCChr *chr, s32 phase);
s32 BtlAct_Action3B(BtlActCChr *chr, s32 phase);
s32 BtlAct_Action3C(BtlActCChr *chr, s32 phase);
s32 BtlAct_Action3D(BtlActCChr *chr, s32 phase);
s32 BtlAct_Action3E(BtlActCChr *chr, s32 phase);
s32 BtlAct_Action3F(BtlActCChr *chr, s32 phase);
s32 BtlAct_Action40(BtlActCChr *chr, s32 phase);
s32 BtlAct_Action41(BtlActCChr *chr, s32 phase);
s32 BtlAct_Action42(BtlActCChr *chr, s32 phase);
s32 BtlAct_Action43(BtlActCChr *chr, s32 phase);
s32 BtlAct_PlaySubAnim(BtlActCChr *chr, s32 anim);
s32 BtlAct_KiBlastHandler(BtlActCChr *chr, s32 phase);
s32 BtlAct_ChargedKiBlastHandler(BtlActCChr *chr, s32 phase);
s32 BtlAct_DashKiBlastHandler(BtlActCChr *chr, s32 phase);
s32 BtlAct_DashChargedKiBlastHandler(BtlActCChr *chr, s32 phase);


/* ======== formerly btl_act_2_part2.h ======== */


#include "types.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, 0x1EE058..0x1F1930 (src/battle/btl_act_d.c).
 * The structures are partial views local to this module.
 */

/* Pose block (chr + 0x10, what BtlChar_GetPos returns); names as in btl_char_move.h. */
typedef BtlFighterPose BtlActDPose;

/* Latched attack parameters (chr + 0xCF8), see BtlActAttack in btl_char_action.h. */
typedef BtlFighterAttack BtlActDAttack;

/* Active member's gauge block (BtlMember_GetActiveGauge). */
typedef struct BtlActDGauge {
    /* 0x00 */ s32 hp;
    /* 0x04 */ s32 hpMax;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 ki;
    /* 0x10 */ s32 kiMax;
    /* 0x14 */ u8 unk14[0x28 - 0x14];
    /* 0x28 */ s32 lowHealthIdle;      /* non-zero: second idle motion, flag 0x96 every frame */
    /* 0x2C */ s32 variantVoiced;
    /* 0x30 */ s32 bodyChanged;
} BtlActDGauge;

/* Member entry (BtlMember_GetActive). */
typedef struct BtlActDMember {
    /* 0x00 */ u8 unk0[0xA0];
    /* 0xA0 */ s32 idleTalkCount;      /* <= 0 lets the neutral state time out into action 0x43 */
} BtlActDMember;

/* Fighter (0x1600 bytes). */
typedef BtlFighter BtlActDChr;

s32 BtlAct_KiBlastB2(BtlActDChr *chr, s32 phase);
s32 BtlAct_KiBlastChargeB3(BtlActDChr *chr, s32 phase);
s32 BtlAct_KiVolley95(BtlActDChr *chr, s32 phase);
void BtlAct_PlayLandFx(BtlActDChr *chr, s32 hard);
s32 BtlAct_GetIdleMotion(BtlActDChr *chr);
s32 BtlAct_NeutralHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_IdleOnceHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_MoveHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_CloseMoveHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_DashMoveHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_JumpStartHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_JumpAirHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_AscendHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_DescendHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_HopHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_FastAscendHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_FastDescendHandler(BtlActDChr *chr, s32 phase);
s32 BtlAct_HomingDashHandler(BtlActDChr *chr, s32 phase);



/* ======== formerly btl_act_2_part3.h ======== */


#include "types.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, actions 0x1A..0x35 and 0xFA..0xFC (src/battle/btl_act_e.c,
 * 0x1F1930..0x1F5460). The structures are partial views local to this module.
 * Handler protocol: see include/battle/btl_char_action.h.
 */

/* Pose block (chr + 0x10, what BtlChar_GetPos returns). */
typedef BtlFighterPose BtlActEPose;

/* Active member's gauge block (BtlMember_GetActiveGauge). */
typedef struct BtlActEGauge {
    /* 0x00 */ s32 hp;
    /* 0x04 */ s32 hpMax;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 ki;
    /* 0x10 */ s32 kiMax;
    /* 0x14 */ u8 unk14[0x28 - 0x14];
    /* 0x28 */ s32 lowHealthIdle;
} BtlActEGauge;

/* Attack parameter block (see BtlActAttack in btl_char_action.h). */
typedef BtlFighterAttack BtlActEAttack;

/* Battle object of a fighter (BtlChar_GetObj). */
typedef struct BtlActEObj {
    /* 0x000 */ u8 unk0[0xBDC];
    /* 0xBDC */ f32 unkBDC;
} BtlActEObj;

/* Fighter (0x1600 bytes). */
typedef BtlFighter BtlActEChr;

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
    /* 0x04 */ BtlActEChr *chars;
    /* 0x08 */ u8 unk8[0x7C - 0x8];
    /* 0x7C */ BtlActEClashC clashC;
} BtlActERoster;

s32 BtlAct_DragonDashHandler(BtlActEChr *chr, s32 phase);
s32 BtlAct_StepHandler(BtlActEChr *chr, s32 phase);
s32 BtlAct_StepInHandler(BtlActEChr *chr, s32 phase);
s32 BtlAct_VanishStepHandler(BtlActEChr *chr, s32 phase);
s32 BtlAct_VanishBehindHandler(BtlActEChr *chr, s32 phase);
void BtlAct_RecoverHandler(BtlActEChr *chr, s32 phase);
f32 BtlAct_PlayAttackPart(BtlActEChr *chr, BtlActEAttack *atk, s32 frame, f32 time);
void BtlAct_VanishAttackHandler(BtlActEChr *chr, s32 phase);
void BtlAct_VanishAttackQuickHandler(BtlActEChr *chr, s32 phase);
void BtlAct_WarpBehindAttackHandler(BtlActEChr *chr, s32 phase);
void BtlAct_WarpAheadAttackHandler(BtlActEChr *chr, s32 phase);
s32 BtlAct_SlideToAttackHandler(BtlActEChr *chr, s32 phase);
void BtlAct_RushToAttackHandler(BtlActEChr *chr, s32 phase);
void BtlAct_HopBackAttackHandler(BtlActEChr *chr, s32 phase);
s32 BtlAct_CircleDashHandler(BtlActEChr *chr, s32 phase);
void BtlAct_RushDashHandler(BtlActEChr *chr, s32 phase);
s32 BtlAct_VanishDashHandler(BtlActEChr *chr, s32 phase);
void BtlAct_ClashBlowsHandler(BtlActEChr *chr, s32 phase);
s32 BtlAct_ClashVanishHandler(BtlActEChr *chr, s32 phase);
s32 BtlAct_ClashExchangeHandler(BtlActEChr *chr, s32 phase);


#endif

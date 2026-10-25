#ifndef BATTLE_BTL_ACT_F_H
#define BATTLE_BTL_ACT_F_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Blast 2 / Ultimate Blast actions, first part (src/battle/btl_act_super.c, 0x1F5460..0x1F8C00): the helpers shared
 * by all class 2..4 technique actions and the handlers of actions 0x106..0x11A. The structures are partial views
 * local to this module: only the fields this file touches. Names are guesses unless the comment gives the reason.
 */

/* Fighter flags this module sets or tests (numbers verified, meanings from how they are used here). */
#define BTL_SUPER_FLAG_FIRE     0xA7 /* held; raised by the effect when the start cinematic is over: fire */
#define BTL_SUPER_FLAG_END      0xA8 /* held; raised by the effect when the beam is over: leave the firing loop */
#define BTL_SUPER_FLAG_END2     0xA9 /* held; the same for the second firing stage */
#define BTL_SUPER_FLAG_QUICK    0xA4 /* quick form: the damage reader BtlSuper_GetDamage gives 30% */
#define BTL_SUPER_TIMEOUT       150  /* frames after which the handlers raise those three flags themselves */

/* Pose block (chr + 0x10, BtlChar_GetPos). */
typedef BtlFighterPose BtlSuperPose;

/* Animation data of one layer of the object. */
typedef struct BtlSuperAnimData {
    /* 0x0 */ u16 unk0;
    /* 0x2 */ u16 frames;      /* length in frames */
} BtlSuperAnimData;

/* Battle object of a fighter (BtlChar_GetObj). */
typedef struct BtlSuperObj {
    /* 0x000 */ u8 unk0[0xA40];
    /* 0xA40 */ u32 flags;     /* bit 0x40000: set while BtlObj_SetColorMode bit 2 is on (inferred) */
    /* 0xA44 */ u8 unkA44[0xBD8 - 0xA44];
    /* 0xBD8 */ BtlSuperAnimData *subAnim; /* animation of layer 1 (BtlAnim_PlaySub), NULL when not loaded */
} BtlSuperObj;

/* Active member's gauge block (BtlMember_GetActiveGauge). */
typedef BtlMemberGauge BtlSuperGauge;

/* Fighter (0x1600 bytes): the fields this module touches. */
typedef BtlFighter BtlSuperChr;

void BtlSuper_RequestCut(BtlSuperChr *chr, s32 cls);
void BtlSuper_SetStartFx(BtlSuperChr *chr, s32 cls);
void BtlSuper_SetClassFlag(BtlSuperChr *chr, s32 cls, s32 force);
void BtlSuper_RequestHitStop(BtlSuperChr *chr, s32 light);
void BtlSuper_SetFiringFlags(BtlSuperChr *chr);
void BtlSuper_SetClass(BtlSuperChr *chr, s32 cls);
void BtlSuper_SetRushFlags(BtlSuperChr *chr);
void BtlSuper_ShakeOnEvent(BtlSuperChr *chr, s32 cls, s32 kind);
void BtlSuper_FitAnimToEvent(BtlSuperChr *chr, f32 seconds);
void BtlSuper_AddSway(BtlSuperChr *chr);
void BtlSuper_VibrateCharge(BtlSuperChr *chr);
void BtlSuper_Recoil(BtlSuperChr *chr, s32 cls, s32 loop);
void BtlSuper_MeasureRushStep(BtlSuperChr *chr, s32 cls, s32 step);
void BtlSuper_Begin(BtlSuperChr *chr, s32 cls, s32 force);
void BtlSuper_PlayStartSound(BtlSuperChr *chr, s32 cls, s32 force);
void BtlSuper_Leave(BtlSuperChr *chr, s32 cls);
void BtlSuper_SetupRushDamage(BtlSuperChr *chr, s32 cls, s32 fromAnim);
void BtlSuper_Finish(BtlSuperChr *chr, s32 cls);
void BtlAct_SuperBeamHandler(BtlSuperChr *chr, s32 phase);
void BtlAct_SuperWarpBeamHandler(BtlSuperChr *chr, s32 phase);
void BtlAct_SuperLongBeamHandler(BtlSuperChr *chr, s32 phase);
void BtlAct_SuperChargeHandler(BtlSuperChr *chr, s32 phase);
void BtlAct_SuperRepeatHandler(BtlSuperChr *chr, s32 phase);
void BtlAct_SuperQuickBeamHandler(BtlSuperChr *chr, s32 phase);
void BtlAct_SuperQuickLongBeamHandler(BtlSuperChr *chr, s32 phase);


/* ======== formerly btl_act_super_part2.h ======== */


#include "types.h"
#include "sys/math3d.h"

/*
 * Blast 2 / Ultimate Blast actions, second part (src/battle/btl_act_g.c, 0x1F8C00..0x1FC2B0): the rush techniques
 * (dash, strike, scripted sequence, both sides), the clash struggle, and two helpers of the throw handlers.
 * The structures are partial views local to this module: only the fields this file touches. Blocks documented
 * elsewhere keep their layout (BtlCollThrow, HitReact, BtlMemberQueue, BtlActForm, BtlMovePose).
 */

/* Pose block of a fighter (chr + 0x10, BtlChar_GetPos). Same layout as BtlMovePose; only what this file reads. */
typedef BtlFighterPose ActGPose;

/* Rush / throw description shared by the two fighters (chr + 0xE90). Same layout as BtlCollThrow. */
typedef BtlFighterThrow ActGThrow;

/* Hit reaction block (chr + 0xFB0). Same layout as HitReact in btl_char_hit.h. */
typedef BtlFighterReact ActGReact;

/* Form-change request (chr + 0x12CC). Same layout as BtlActForm. */
typedef BtlFighterForm ActGForm;

/* Gauge block of the active member (BtlMember_GetActiveGauge). */
typedef BtlMemberGauge ActGGauge;

/* Damage queue (chr + 0xD94). Same layout as BtlMemberQueue in btl_char_member.h, plus the word after it. */
typedef BtlFighterQueue ActGQueue;

/* Root rotation track of an animation. */
typedef struct ActGTrack {
    /* 0x00 */ u16 flags;      /* bit 0: the first key is stored at +4, else at +0x14 */
    /* 0x02 */ u16 count;
    /* 0x04 */ u32 key[6];     /* packed quaternions, 64 bits each */
} ActGTrack;

typedef struct ActGAnim {
    /* 0x00 */ u8 unk0[6];
    /* 0x06 */ u16 rootTrack;  /* offset of the root rotation track in words, 0 = none */
} ActGAnim;

/* Battle object (BtlChar_GetObj). */
typedef struct ActGObj {
    /* 0x000 */ u8 unk0[0xB40];
    /* 0xB40 */ ActGAnim *anim;
} ActGObj;

/* Fighter (0x1600 bytes). */
typedef BtlFighter ActGChr;

void BtlAct_SuperRushDashHandler(ActGChr *chr, s32 phase);
void BtlAct_SuperRushStrikeHandler(ActGChr *chr, s32 phase);
void BtlAct_SuperRushSequenceHandler(ActGChr *chr, s32 phase);
void BtlAct_SuperRushFlyHandler(ActGChr *chr, s32 phase);
void BtlAct_SuperRushFollowHandler(ActGChr *chr, s32 phase);
void BtlAct_SuperRushFinishHandler(ActGChr *chr, s32 phase);
void BtlAct_SuperRushCaughtHandler(ActGChr *chr, s32 phase);
s32 BtlAct_UltimateStartHandler(ActGChr *chr, s32 phase);
void BtlAct_ClashStruggleHandler(ActGChr *chr, s32 phase);
void BtlAct_ClashLostHandler(ActGChr *chr, s32 phase);
s32 BtlAct_SuperLaunchedUpHandler(ActGChr *chr, s32 phase);
s32 BtlAct_ChangeRandomCharaHandler(ActGChr *chr, s32 phase);
void BtlActThrow_SetupDamage(ActGChr *chr, s32 extraHit);
void BtlActThrow_TurnByRootYaw(ActGChr *chr);


#endif

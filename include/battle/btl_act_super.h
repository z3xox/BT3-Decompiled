#ifndef BATTLE_BTL_ACT_F_H
#define BATTLE_BTL_ACT_F_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Blast 2 / Ultimate Blast actions, first part (src/battle/btl_act_super.c, 0x1F5460..0x1F8C00): the helpers shared
 * by all class 2..4 technique actions and the handlers of actions 0x106..0x11A. The structures are partial views
 * local to this module: only the fields this file touches. Names are guesses unless the comment gives the reason.
 * The fighter is BtlFighter (battle/btl_fighter.h).
 */

/* Fighter flags this module sets or tests (numbers verified, meanings from how they are used here). */
#define BTL_SUPER_FLAG_FIRE     0xA7 /* held; raised by the effect when the start cinematic is over: fire */
#define BTL_SUPER_FLAG_END      0xA8 /* held; raised by the effect when the beam is over: leave the firing loop */
#define BTL_SUPER_FLAG_END2     0xA9 /* held; the same for the second firing stage */
#define BTL_SUPER_FLAG_QUICK    0xA4 /* quick form: the damage reader BtlSuper_GetDamage gives 30% */
#define BTL_SUPER_TIMEOUT       150  /* frames after which the handlers raise those three flags themselves */

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

void BtlSuper_RequestCut(BtlFighter *chr, s32 cls);
void BtlSuper_SetStartFx(BtlFighter *chr, s32 cls);
void BtlSuper_SetClassFlag(BtlFighter *chr, s32 cls, s32 force);
void BtlSuper_RequestHitStop(BtlFighter *chr, s32 light);
void BtlSuper_SetFiringFlags(BtlFighter *chr);
void BtlSuper_SetClass(BtlFighter *chr, s32 cls);
void BtlSuper_SetRushFlags(BtlFighter *chr);
void BtlSuper_ShakeOnEvent(BtlFighter *chr, s32 cls, s32 kind);
void BtlSuper_FitAnimToEvent(BtlFighter *chr, f32 seconds);
void BtlSuper_AddSway(BtlFighter *chr);
void BtlSuper_VibrateCharge(BtlFighter *chr);
void BtlSuper_Recoil(BtlFighter *chr, s32 cls, s32 loop);
void BtlSuper_MeasureRushStep(BtlFighter *chr, s32 cls, s32 step);
void BtlSuper_Begin(BtlFighter *chr, s32 cls, s32 force);
void BtlSuper_PlayStartSound(BtlFighter *chr, s32 cls, s32 force);
void BtlSuper_Leave(BtlFighter *chr, s32 cls);
void BtlSuper_SetupRushDamage(BtlFighter *chr, s32 cls, s32 fromAnim);
void BtlSuper_Finish(BtlFighter *chr, s32 cls);
void BtlAct_SuperBeamHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperWarpBeamHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperLongBeamHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperChargeHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperRepeatHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperQuickBeamHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperQuickLongBeamHandler(BtlFighter *chr, s32 phase);


/* ======== formerly btl_act_super_part2.h ======== */


#include "types.h"
#include "sys/math3d.h"

/*
 * Blast 2 / Ultimate Blast actions, second part (src/battle/btl_act_g.c, 0x1F8C00..0x1FC2B0): the rush techniques
 * (dash, strike, scripted sequence, both sides), the clash struggle, and two helpers of the throw handlers.
 * The structures are partial views local to this module: only the fields this file touches. The fighter is
 * BtlFighter (battle/btl_fighter.h).
 */

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

void BtlAct_SuperRushDashHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperRushStrikeHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperRushSequenceHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperRushFlyHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperRushFollowHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperRushFinishHandler(BtlFighter *chr, s32 phase);
void BtlAct_SuperRushCaughtHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_UltimateStartHandler(BtlFighter *chr, s32 phase);
void BtlAct_ClashStruggleHandler(BtlFighter *chr, s32 phase);
void BtlAct_ClashLostHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_SuperLaunchedUpHandler(BtlFighter *chr, s32 phase);
s32 BtlAct_ChangeRandomCharaHandler(BtlFighter *chr, s32 phase);
void BtlActThrow_SetupDamage(BtlFighter *chr, s32 extraHit);
void BtlActThrow_TurnByRootYaw(BtlFighter *chr);


#endif

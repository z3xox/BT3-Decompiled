#ifndef BATTLE_BTL_ACT_F_H
#define BATTLE_BTL_ACT_F_H

#include "types.h"
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
typedef struct BtlSuperPose {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ Vec4 rot;       /* rot.y = model yaw */
    /* 0x20 */ u8 unk20[0x98 - 0x20];
    /* 0x98 */ f32 speed;      /* zeroed with velY when a technique starts */
    /* 0x9C */ f32 velY;       /* vertical speed, positive = down (`fallSpeed` in the other pose views, ActGPose included) */
} BtlSuperPose;

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
typedef struct BtlSuperGauge {
    /* 0x00 */ s32 health;
    /* 0x04 */ u8 unk4[0x20 - 0x4];
    /* 0x20 */ s32 variant;
    /* 0x24 */ u8 unk24[0x38 - 0x24];
    /* 0x38 */ s32 used[5];    /* [class]: times a technique of that class was started (only 2..4 written) */
    /* 0x4C */ s32 fired[5];   /* [class]: times a self-destruct technique (flag B 0x2000) was fired */
} BtlSuperGauge;

/* Fighter (0x1600 bytes): the fields this module touches. */
typedef struct BtlSuperChr {
    /* 0x0000 */ s32 player;
    /* 0x0004 */ u8 unk4[0x3D0 - 0x4];
    /* 0x03D0 */ s32 work[0x14];   /* per-action scratch, zeroed on every action switch */
    /* 0x0420 */ u8 unk420[0x964 - 0x420];
    /* 0x0964 */ s32 actionFrame;  /* frames in the current action */
    /* 0x0968 */ u8 unk968[0xD94 - 0x968];
    /* The damage queue against the opponent (BtlMemberQueue in btl_char_member.h), filled by
       BtlSuper_SetupRushDamage. */
    /* 0x0D94 */ s32 dmgTotalStart;      /* the total again (1 with technique flag B 0x400 and + 0xEC4 set) */
    /* 0x0D98 */ s32 dmgTotal;     /* total damage, rounded up to 10 */
    /* 0x0D9C */ s32 dmgPerHit;    /* total / (hits + 5), at most 15000, rounded up to 10 */
    /* 0x0DA0 */ s32 dmgHits;      /* number of 0x200000000 events in the animation(s) */
    /* 0x0DA4 */ s32 dmgFlags;     /* BTL_DMG_*: 0x180 plus 0x800000 / 0x1000000 / 0x2000000 by class */
    /* 0x0DA8 */ s32 dmgDrainHealthStart;     /* the health drain again */
    /* 0x0DAC */ s32 drainHealth;  /* health still to drain */
    /* 0x0DB0 */ s32 drainHealthStep; /* per frame: drain / max((to - from) / 2, 1) + 1 */
    /* 0x0DB4 */ s32 dmgDrainKiStart;
    /* 0x0DB8 */ s32 drainKi;
    /* 0x0DBC */ s32 drainKiStep;
    /* 0x0DC0 */ s32 drainFrom;    /* first frame of the drain window, -1 = none */
    /* 0x0DC4 */ s32 drainTo;      /* last frame of the drain window */
    /* 0x0DC8 */ u8 unkDC8[0xE40 - 0xDC8];
    /* 0x0E40 */ s32 cooldown;     /* frames, from the technique (BtlSuper_GetCooldownFrames) when its action is left */
    /* 0x0E44 */ f32 charge;       /* charge level of a chargeable technique, 0..1 */
    /* 0x0E48 */ s32 chargeFull;   /* frames the charge has been full (the sounds play on the first) */
    /* 0x0E4C */ u8 unkE4C[0xE5C - 0xE4C];
    /* 0x0E5C */ s32 skillCount3;       /* counter (full at 3, fighter.md); zeroed when technique 0x268 ends */
    /* 0x0E60 */ s32 unkE60;       /* counter 0..10 (boostStock in btl_act_change.h): + 5 per technique 0x2EF, zeroed when technique 0x2CD ends; + 1 per throw by character 0x6E (btl_act_change.c) */
    /* 0x0E64 */ u8 unkE64[0xEC4 - 0xE64];
    /* 0x0EC4 */ s32 thrFailed;       /* non-zero: a flag B 0x400 technique deals 1 damage, undefended and exact */
    /* 0x0EC8 */ u8 unkEC8[0xEE8 - 0xEC8];
    /* 0x0EE8 */ s32 thrHalfDamage;       /* non-zero: rush damage is halved */
    /* 0x0EEC */ u8 unkEEC[0xEF8 - 0xEEC];
    /* Measured from a rush technique's animation chain by BtlSuper_MeasureRushStep. */
    /* 0x0EF8 */ s32 rushHits;     /* 0x200000000 events in all steps */
    /* 0x0EFC */ u8 unkEFC[0xF10 - 0xEFC];
    /* 0x0F10 */ s32 rushFrames;   /* total length: frames of every step, + 1 between steps */
    /* 0x0F14 */ s32 rushFrameA;   /* frame, counted over the chain, of the first 0x20000 event */
    /* 0x0F18 */ s32 rushFrameB;   /* the same for the 0x40000 event (its last frame) */
    /* 0x0F1C */ s32 rushStepA;    /* step that holds the 0x20000 event, -1 = not found yet */
    /* 0x0F20 */ s32 rushStepB;    /* step that holds the 0x40000 event */
    /* 0x0F24 */ u8 unkF24[0x12D0 - 0xF24];
    /* The form-change request (BtlActForm at + 0x12CC), filled by BtlAct_SetFormCurrent. */
    /* 0x12D0 */ s32 formChara;
    /* 0x12D4 */ s32 formCostume;
    /* 0x12D8 */ u8 unk12D8[0x12E0 - 0x12D8];
    /* 0x12E0 */ s32 formVariant;       /* passed to BtlChange_RequestChara as variant, animChara, anim1Chara, voiceChara */
    /* 0x12E4 */ s32 formAnimChara;
    /* 0x12E8 */ s32 form1C;
    /* 0x12EC */ s32 formVoiceChara;
    /* 0x12F0 */ u8 unk12F0[0x1594 - 0x12F0];
    /* 0x1594 */ s32 inputClass;   /* class whose button command is watched; -1 again every frame */
    /* 0x1598 */ u8 unk1598[0x1600 - 0x1598];
} BtlSuperChr; /* size 0x1600 */

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
typedef struct ActGPose {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ Vec4 rot;
    /* 0x20 */ Vec4 dispOfs;
    /* 0x30 */ Vec4 vel;
    /* 0x40 */ Vec4 moved;      /* whole movement of the previous frame */
    /* 0x50 */ Vec4 rootPos;
    /* 0x60 */ Vec4 rootRot;
    /* 0x70 */ Vec4 impulse;
    /* 0x80 */ Vec4 dir;
    /* 0x90 */ f32 pitch;       /* heading pitch */
    /* 0x94 */ f32 yaw;         /* heading yaw */
    /* 0x98 */ f32 speed;
    /* 0x9C */ f32 fallSpeed;
} ActGPose;

/* Rush / throw description shared by the two fighters (chr + 0xE90). Same layout as BtlCollThrow. */
typedef struct ActGThrow {
    /* 0x00 */ s32 tech;
    /* 0x04 */ s32 atkSide;
    /* 0x08 */ s32 defSide;
    /* 0x0C */ s32 slot;
    /* 0x10 */ s32 stepCount;
    /* 0x14 */ s32 landingKind;
    /* 0x18 */ s32 partnerStep;
    /* 0x1C */ s32 lastStep;
    /* 0x20 */ s32 partnerChara;
    /* 0x24 */ s32 partnerCostume;
    /* 0x28 */ s32 partnerVariant;
    /* 0x2C */ s32 partnerSlot;
    /* 0x30 */ s32 turnVictim;
    /* 0x34 */ s32 failed;
    /* 0x38 */ s32 unk38;
    /* 0x3C */ s32 faceAttacker;
    /* 0x40 */ s32 koSkipLanding;
    /* 0x44 */ s32 caughtLoop;
    /* 0x48 */ s32 atkReload;
    /* 0x4C */ s32 defReload;
    /* 0x50 */ s32 placeFirstStep;
    /* 0x54 */ s32 unk54;
    /* 0x58 */ s32 halfDamage;
    /* 0x5C */ s32 raiseAtEnd;
    /* 0x60 */ f32 turnYaw;
    /* 0x64 */ f32 turnPitch;
} ActGThrow; /* size 0x68 */

/* Hit reaction block (chr + 0xFB0). Same layout as HitReact in btl_char_hit.h. */
typedef struct ActGReact {
    /* 0x00 */ s32 reaction;
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 animCycle;
    /* 0x0C */ s32 back;
    /* 0x10 */ s32 noBlend;
    /* 0x14 */ s32 silent;
    /* 0x18 */ f32 scale;
    /* 0x1C */ f32 yaw;
    /* 0x20 */ f32 turnYaw;     /* yaw the fighter is turned to under flag 0x94 */
    /* 0x24 */ f32 turnPitch;
    /* 0x28 */ f32 launchA;
    /* 0x2C */ f32 launchB;
    /* 0x30 */ s32 stun;
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 shakeTimer;
    /* 0x3C */ s32 damage;      /* deferred damage */
    /* 0x40 */ s32 unk40;
    /* 0x44 */ s32 slot;
    /* 0x48 */ s32 unk48;
} ActGReact; /* size 0x4C */

/* Form-change request (chr + 0x12CC). Same layout as BtlActForm. */
typedef struct ActGForm {
    /* 0x00 */ s32 index;
    /* 0x04 */ s32 chara;
    /* 0x08 */ s32 costume;
    /* 0x0C */ s32 cost;
    /* 0x10 */ s32 kind;
    /* 0x14 */ s32 variant;       /* model variant requested (1 from BtlAct_SetFormCurrent) */
    /* 0x18 */ s32 animChara;
    /* 0x1C */ s32 unk1C;
    /* 0x20 */ s32 voiceChara;
    /* 0x24 */ s32 objId;
    /* 0x28 */ s32 objCostume;
    /* 0x2C */ s32 partner;
} ActGForm;

/* Gauge block of the active member (BtlMember_GetActiveGauge). */
typedef struct ActGGauge {
    /* 0x00 */ s32 health;
    /* 0x04 */ s32 healthMax;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 ki;
    /* 0x10 */ s32 kiMax;
    /* 0x14 */ s32 blast;
    /* 0x18 */ s32 blastMax;
    /* 0x1C */ s32 maxPower;
    /* 0x20 */ s32 variant;     /* model variant in use; 0 = undamaged */
    /* 0x24 */ u8 unk24[0x4C - 0x24];
    /* 0x4C */ s32 fired[5];    /* by technique slot: counts self-destruct techniques (flag B 0x2000) used */
} ActGGauge;

/* Damage queue (chr + 0xD94). Same layout as BtlMemberQueue in btl_char_member.h, plus the word after it. */
typedef struct ActGQueue {
    /* 0x00 */ s32 totalStart;            /* the total again */
    /* 0x04 */ s32 total;           /* health damage still to deal */
    /* 0x08 */ s32 perHit;          /* amount per trigger while more than one hit is left */
    /* 0x0C */ s32 hits;            /* triggers left */
    /* 0x10 */ s32 flags;           /* BTL_DMG_* */
    /* 0x14 */ s32 drainHealthStart;           /* the health drain again */
    /* 0x18 */ s32 drainHealth;     /* health still to drain */
    /* 0x1C */ s32 drainHealthStep; /* per frame */
    /* 0x20 */ s32 drainKiStart;           /* the ki drain again */
    /* 0x24 */ s32 drainKi;         /* ki still to drain */
    /* 0x28 */ s32 drainKiStep;     /* per frame */
    /* 0x2C */ s32 drainFrom;       /* animation frame of the 0x20000 event, -1 = no drain */
    /* 0x30 */ s32 drainTo;         /* animation frame of the 0x40000 event */
} ActGQueue; /* size 0x34 */

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
typedef struct ActGChr {
    /* 0x0000 */ s32 player;
    /* 0x0004 */ s32 pad;
    /* 0x0008 */ s32 side;
    /* 0x000C */ s32 objId;
    /* 0x0010 */ u8 unk10[0x3D0 - 0x10];
    /* 0x03D0 */ s32 work[0x14];    /* per-action scratch, zeroed when an action starts */
    /* 0x0420 */ u8 unk420[0x964 - 0x420];
    /* 0x0964 */ s32 actionFrame;   /* frames spent in the current action */
    /* 0x0968 */ u8 unk968[0x970 - 0x968];
    /* 0x0970 */ s32 holdAction;    /* set in a run phase: skip the next forced-action check */
    /* 0x0974 */ u8 unk974[0xD94 - 0x974];
    /* 0x0D94 */ ActGQueue queue;   /* damage queue (BtlMemberQueue) */
    /* 0x0DC8 */ u8 unkDC8[0xE4C - 0xDC8];
    /* 0x0E4C */ s32 clashPower;    /* clash struggle score: technique value / 20, ability bonus, + 1 per frame of input 51 */
    /* 0x0E50 */ u8 unkE50[0xE90 - 0xE50];
    /* 0x0E90 */ ActGThrow thr;     /* rush / throw description (BtlCollThrow) */
    /* 0x0EF8 */ s32 rushHits;      /* written by BtlSuper_MeasureRushStep */
    /* 0x0EFC */ f32 stepFrame[5];  /* animation frame reached in each step of the rush sequence */
    /* 0x0F10 */ u8 unkF10[0xFB0 - 0xF10];
    /* 0x0FB0 */ ActGReact react;   /* hit reaction block (HitReact) */
    /* 0x0FFC */ u8 unkFFC[0x12CC - 0xFFC];
    /* 0x12CC */ ActGForm form;     /* form-change request (BtlActForm) */
    /* 0x12FC */ u8 unk12FC[0x1600 - 0x12FC];
} ActGChr;

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

#ifndef BATTLE_BTL_ACT_A_H
#define BATTLE_BTL_ACT_A_H

#include "types.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, first group (src/battle/btl_act_1.c, 0x1E3158..0x1E6CC0).
 * The structures are partial views local to this module.
 */

/* Pose block (chr + 0x10, what BtlChar_GetPos returns). */
typedef struct BtlActAPose {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ Vec4 rot;
    /* 0x20 */ u8 unk20[0x80 - 0x20];
    /* 0x80 */ Vec4 dir;       /* unit travel direction */
    /* 0x90 */ f32 pitch;      /* heading pitch */
    /* 0x94 */ f32 yaw;        /* heading yaw */
    /* 0x98 */ f32 speed;
    /* 0x9C */ f32 vspeed;     /* vertical speed, positive = down */
} BtlActAPose;

/* Latched attack parameters (chr + 0xCF8); same layout as BtlActAttack in btl_char_action.h. */
typedef struct BtlActAAttack {
    /* 0x00 */ s32 flags;
    /* 0x04 */ s16 motion;
    /* 0x06 */ s16 motion2;
    /* 0x08 */ s8 parts;
    /* 0x09 */ u8 level;
    /* 0x0A */ s8 cut;
    /* 0x0B */ s8 follow[2];
    /* 0x0D */ u8 padD[3];
    /* 0x10 */ f32 rate;
    /* 0x14 */ f32 cancelAt;   /* animation progress after which the queued follow-up starts */
    /* 0x18 */ f32 leadTime;
    /* 0x1C */ f32 speed;
    /* 0x20 */ s32 chained;    /* a follow-up attack (not a generic cancel) is queued */
} BtlActAAttack; /* size 0x24 */

/* Battle object (what BtlChar_GetObj returns). */
typedef struct BtlActAObj {
    /* 0x000 */ u8 unk0[0xBDC];
    /* 0xBDC */ f32 unkBDC;    /* a length in frames at 60 Hz, added to the vanish attack's lead */
    /* 0xBE0 */ u8 unkBE0[0xCAC - 0xBE0];
    /* 0xCAC */ s8 hitCount;
    /* 0xCAD */ s8 hitNo;
} BtlActAObj;

/* Fighter (0x1600 bytes). */
typedef struct BtlActAChr {
    /* 0x0000 */ s32 player;
    /* 0x0004 */ s32 pad;
    /* 0x0008 */ s32 side;
    /* 0x000C */ s32 objId;
    /* 0x0010 */ u8 unk10[0x3D0 - 0x10];
    /* 0x03D0 */ s32 work[6];       /* per-action scratch, zeroed when an action starts: [0] bit flags, [2] step, [3] frame count, [4] animation */
    /* 0x03E8 */ f32 workF[14];     /* [0] animation progress after which the queued action starts */
    /* 0x0420 */ u8 unk420[0x4A8 - 0x420];
    /* 0x04A8 */ f32 camSide;       /* ChrCam side: sign picks the left / right camera cut */
    /* 0x04AC */ u8 unk4AC[0x964 - 0x4AC];
    /* 0x0964 */ s32 actionFrame;   /* frames spent in the current action */
    /* 0x0968 */ u8 unk968[0xCF8 - 0x968];
    /* 0x0CF8 */ BtlActAAttack attack;
    /* 0x0D1C */ u8 unkD1C[0xD60 - 0xD1C];
    /* 0x0D60 */ s32 rushStep;        /* rush chain step 0..9 (animation 0x37 + step) */
    /* 0x0D64 */ s32 rushRapidStep;        /* rapid rush step 0..4 (animation 0x3C + step) */
    /* 0x0D68 */ s32 dashCount;        /* > 0: the dash smash shows the charge effect at once */
    /* 0x0D6C */ s32 dashLimit;
    /* 0x0D70 */ s32 vanishCount;        /* counts vanishing attacks */
    /* 0x0D74 */ s32 vanishLimit;
    /* 0x0D78 */ f32 charge;        /* attack charge level 0..1 */
    /* 0x0D7C */ f32 chargeTimer;        /* attack charge timer 0..1 */
    /* 0x0D80 */ s32 chargeGauge;        /* gauge 0..100000 that blends the two charge rates */
    /* 0x0D84 */ s32 chargeFullFrames;        /* frames the charge has been full */
    /* 0x0D88 */ s32 evasionCount;
    /* 0x0D8C */ s32 unkD8C;
    /* 0x0D90 */ s32 attackId;      /* set to the action id by the vanishing attack */
    /* 0x0D94 */ u8 unkD94[0x1354 - 0xD94];
    /* 0x1354 */ s32 burstNode;       /* model node for the effect of action 0x66 */
    /* 0x1358 */ u8 unk1358[0x1600 - 0x1358];
} BtlActAChr; /* size 0x1600 */

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
typedef struct BtlActBPose {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ Vec4 rot;       /* y = model yaw */
    /* 0x20 */ Vec4 dispOffset;
    /* 0x30 */ Vec4 velocity;     /* movement of the previous frame ("my velocity" in btl_char_move.h) */
    /* 0x40 */ Vec4 moved;     /* whole movement of the previous frame: its length is the real speed */
    /* 0x50 */ u8 unk50[0x70 - 0x50];
    /* 0x70 */ Vec4 impulse;   /* knock-back velocity, metres per frame */
    /* 0x80 */ Vec4 dir;       /* unit direction of travel */
    /* 0x90 */ f32 pitch;      /* heading pitch, positive = up */
    /* 0x94 */ f32 yaw;        /* heading yaw */
    /* 0x98 */ f32 speed;      /* metres per frame along dir */
    /* 0x9C */ f32 fallSpeed;  /* vertical speed, positive = down */
    /* 0xA0 */ f32 leanX;
    /* 0xA4 */ f32 leanZ;
    /* 0xA8 */ u8 unkA8[0xB0 - 0xA8];
    /* 0xB0 */ Vec4 ground;    /* ground point below the fighter */
    /* 0xC0 */ Vec4 groundNormal;
    /* 0xD0 */ s32 groundFlags; /* bit 0x40 with pos.y below BtlStage_GetBottom(): landed out of the arena (action 0xD9) */
} BtlActBPose;

/* Active member's gauge block (BtlMember_GetActiveGauge). */
typedef struct BtlActBGauge {
    /* 0x00 */ u8 unk0[0x28];
    /* 0x28 */ s32 lowHealthIdle;      /* non-zero: a downed fighter has to mash before it may act (chr + 0x1000) */
} BtlActBGauge;

/* Latched attack parameters (chr + 0xCF8); BtlActAttack in btl_char_action.h. */
typedef struct BtlActBAttack {
    /* 0x00 */ s32 flags;      /* 0x40 end the camera cut on contact, 0x80 raise flag 0xCD, 0x400 end in action 0xC */
    /* 0x04 */ s16 motion[2];  /* one animation per part */
    /* 0x08 */ s8 parts;
    /* 0x09 */ s8 level;       /* 1..3: effect request 0xD at the start */
    /* 0x0A */ s8 cut;         /* camera cut, < 0 = none */
    /* 0x0B */ s8 follow[2];
    /* 0x0D */ u8 padD[3];
    /* 0x10 */ f32 rate;       /* animation step = rate * 2 while a further part follows */
    /* 0x14 */ f32 cancelAt;
    /* 0x18 */ f32 leadTime;
    /* 0x1C */ f32 speed;      /* advance speed, metres per frame (0 = stand still) */
    /* 0x20 */ s32 chained;
} BtlActBAttack; /* size 0x24 */

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
typedef struct BtlActBChr {
    /* 0x0000 */ s32 player;        /* passed to BtlEvent_Raise by action 0xD8 */
    /* 0x0004 */ s32 pad;
    /* 0x0008 */ s32 side;
    /* 0x000C */ s32 objId;
    /* 0x0010 */ u8 unk10[0x3D0 - 0x10];
    /* 0x03D0 */ s32 work[0x14];    /* per-action scratch, zeroed when an action starts; uses are listed per handler
                                       in btl_act_b.c ([0] bits, [1] done, [2] counter, [3] / [4] counters, [6] float) */
    /* 0x0420 */ u8 unk420[0x964 - 0x420];
    /* 0x0964 */ s32 actionFrame;   /* frames spent in the current action */
    /* 0x0968 */ u8 unk968[0xCF8 - 0x968];
    /* 0x0CF8 */ BtlActBAttack attack;
    /* 0x0D1C */ u8 unkD1C[0xD68 - 0xD1C];
    /* 0x0D68 */ s32 dashCount;        /* > 0: actions 0x6B..0x6F request camera cut 0x20 */
    /* 0x0D6C */ u8 unkD6C[0xD78 - 0xD6C];
    /* 0x0D78 */ f32 charge;        /* set to 1.0 every frame by the attack handler and by action 0x6A */
    /* 0x0D7C */ u8 unkD7C[0xFB8 - 0xD7C];
    /* 0x0FB8 */ s32 animCycle;     /* stagger animation variant counter (BtlActB_NextVariant) */
    /* 0x0FBC */ u8 unkFBC[0xFC8 - 0xFBC];
    /* 0x0FC8 */ f32 reactScale;        /* reaction strength 0..1 (inferred): scales launch speed, knock-back time and
                                       how much mashing helps */
    /* 0x0FCC */ u8 unkFCC[0xFE0 - 0xFCC];
    /* 0x0FE0 */ s32 stunTimer;        /* countdown; > 0 = action 0xE0 */
    /* 0x0FE4 */ u8 unkFE4[0xFF0 - 0xFE4];
    /* 0x0FF0 */ s32 unkFF0;        /* countdown, set to 15 when a stagger action is left */
    /* 0x0FF4 */ u8 unkFF4[0x1000 - 0xFF4];
    /* 0x1000 */ s32 mashLeft;       /* presses still owed before a downed fighter may act, while gauge +0x28 is set */
    /* 0x1004 */ u8 unk1004[0x1600 - 0x1004];
} BtlActBChr; /* size 0x1600 */

/* Helpers other slices call. */
s32 BtlActB_NextVariant(BtlActBChr *chr, s32 n);
void BtlActB_SetReactionFlags(BtlActBChr *chr);
s32 BtlActB_PickDownAnim(BtlActBChr *chr, s32 cur);
void BtlActB_UpdateMashRate(BtlActBChr *chr, f32 *rate, s32 scaled);
s32 BtlActB_TickMemberChange(BtlActBChr *chr, s32 *frames);


#endif

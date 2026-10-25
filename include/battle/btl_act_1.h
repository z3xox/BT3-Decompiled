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

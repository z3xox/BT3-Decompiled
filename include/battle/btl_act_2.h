#ifndef BATTLE_BTL_ACT_C_H
#define BATTLE_BTL_ACT_C_H

#include "types.h"
#include "sys/math3d.h"

/*
 * Fighter action handlers, third slice (src/battle/btl_act_2.c, 0x1EA5F8..0x1EE058).
 * Handler protocol: include/battle/btl_char_action.h. The structures below are partial views
 * local to this module.
 */

/* Pose block (chr + 0x10, what BtlChar_GetPos returns). */
typedef struct BtlActCPose {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ Vec4 rot;
    /* 0x20 */ u8 unk20[0x40 - 0x20];
    /* 0x40 */ Vec4 move;      /* whole movement of the previous frame (`moved` in BtlActBPose); its length is the real speed */
    /* 0x50 */ u8 unk50[0x70 - 0x50];
    /* 0x70 */ Vec4 impulse;
    /* 0x80 */ Vec4 vel;       /* unit direction of travel (`dir` in the other pose views) */
    /* 0x90 */ f32 pitch;      /* heading pitch */
    /* 0x94 */ f32 facing;     /* heading yaw (`yaw` in the other pose views) */
    /* 0x98 */ f32 speed;      /* along the travel direction, per frame */
    /* 0x9C */ f32 fallSpeed;
    /* 0xA0 */ f32 leanX;
    /* 0xA4 */ f32 leanZ;
    /* 0xA8 */ f32 bobPhase;
    /* 0xAC */ f32 unkAC;
    /* 0xB0 */ f32 groundX;
    /* 0xB4 */ f32 groundY;
} BtlActCPose;

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
typedef struct BtlActCChr {
    /* 0x0000 */ s32 player;
    /* 0x0004 */ s32 pad;
    /* 0x0008 */ s32 side;
    /* 0x000C */ s32 objId;
    /* 0x0010 */ u8 unk10[0x3D0 - 0x10];
    /* 0x03D0 */ s32 work[0x14];   /* per-action scratch: [0] bit 0 = dash blast steers at the opponent; [2] = counters of 0x38 / 0x3B, block of 0xEB */
    /* 0x0420 */ u8 unk420[0x4A8 - 0x420];
    /* 0x04A8 */ f32 camSide;       /* ChrCam side: its sign picks the camera cut of action 0x41 */
    /* 0x04AC */ u8 unk4AC[0x964 - 0x4AC];
    /* 0x0964 */ s32 actionFrame;   /* frames in the current action */
    /* 0x0968 */ u8 unk968[0xD44 - 0x968];
    /* 0x0D44 */ s32 comboHits;        /* must reach 10 before a downed fighter may recover */
    /* 0x0D48 */ u8 unkD48[0xD58 - 0xD48];
    /* 0x0D58 */ f32 searchAngle;        /* search window half angle (0.5 .. pi) */
    /* 0x0D5C */ f32 searchRange;        /* search window range (500 .. 1000000) */
    /* 0x0D60 */ u8 unkD60[0xD8C - 0xD60];
    /* 0x0D8C */ s32 unkD8C;        /* action 0x41: which of the three animations is next */
    /* 0x0D90 */ u8 unkD90[0xDD0 - 0xD90];
    /* 0x0DD0 */ Vec4 aimDir;       /* ki blast aim direction */
    /* 0x0DE0 */ s32 blastShots;        /* ki blast counter */
    /* 0x0DE4 */ s32 blastResetTimer;        /* set to 30 by every ki blast (a countdown, see btl_char_action.h) */
    /* 0x0DE8 */ s32 blastAltHand;        /* ki blast: 1 = the next one uses the other hand */
    /* 0x0DEC */ f32 blastCharge;        /* charged ki blast: progress of the hold animation */
    /* 0x0DF0 */ s32 blastNode;        /* charged ki blast: model node it leaves from */
    /* 0x0DF4 */ u8 unkDF4[0xFE0 - 0xDF4];
    /* 0x0FE0 */ s32 stunTimer;        /* stun countdown */
    /* 0x0FE4 */ u8 unkFE4[0x1000 - 0xFE4];
    /* 0x1000 */ s32 mashLeft;       /* downed: button presses still needed before recovering */
    /* 0x1004 */ u8 unk1004[0x1600 - 0x1004];
} BtlActCChr; /* size 0x1600 */

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
typedef struct BtlActDPose {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ Vec4 rot;
    /* 0x20 */ u8 unk20[0x40 - 0x20];
    /* 0x40 */ Vec4 moved;     /* whole movement of the previous frame (btl_char_move.h); its length is the real speed */
    /* 0x50 */ u8 unk50[0x80 - 0x50];
    /* 0x80 */ Vec4 dir;       /* unit direction of travel */
    /* 0x90 */ f32 pitch;      /* heading pitch */
    /* 0x94 */ f32 yaw;        /* heading yaw */
    /* 0x98 */ f32 speed;      /* along dir, per frame */
    /* 0x9C */ f32 fallSpeed;  /* vertical speed, positive = down */
    /* 0xA0 */ f32 leanX;
    /* 0xA4 */ f32 leanZ;
    /* 0xA8 */ f32 bobPhase;
    /* 0xAC */ f32 unkAC;
    /* 0xB0 */ f32 groundX;
    /* 0xB4 */ f32 groundY;
    /* 0xB8 */ u8 unkB8[0xD0 - 0xB8];
    /* 0xD0 */ s32 groundFlags;      /* bit word: 0x40 blocks the landing of action 0x15; 0x40000000 without 0x01000000
                                  keeps the fast descend going near the ground */
} BtlActDPose;

/* Latched attack parameters (chr + 0xCF8), see BtlActAttack in btl_char_action.h. */
typedef struct BtlActDAttack {
    /* 0x00 */ s32 flags;
    /* 0x04 */ s16 motion;
    /* 0x06 */ s16 motion2;
    /* 0x08 */ s8 parts;
    /* 0x09 */ s8 level;
    /* 0x0A */ s8 cut;
    /* 0x0B */ s8 follow[2];
    /* 0x0D */ u8 padD[3];
    /* 0x10 */ f32 rate;
    /* 0x14 */ f32 cancelAt;
    /* 0x18 */ f32 leadTime;
    /* 0x1C */ f32 speed;
    /* 0x20 */ s32 chained;
} BtlActDAttack; /* size 0x24 */

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
typedef struct BtlActDChr {
    /* 0x0000 */ s32 player;
    /* 0x0004 */ s32 pad;
    /* 0x0008 */ s32 side;
    /* 0x000C */ s32 objId;
    /* 0x0010 */ u8 unk10[0x3D0 - 0x10];
    /* 0x03D0 */ s32 work[0x14];    /* per-action scratch, zeroed when an action starts */
    /* 0x0420 */ u8 unk420[0x948 - 0x420];
    /* 0x0948 */ s32 action;
    /* 0x094C */ s32 request;
    /* 0x0950 */ s32 prev;
    /* 0x0954 */ s32 queue[4];
    /* 0x0964 */ s32 actionFrame;
    /* 0x0968 */ u8 unk968[0xCF8 - 0x968];
    /* 0x0CF8 */ BtlActDAttack attack;
    /* 0x0D1C */ u8 unkD1C[0xD68 - 0xD1C];
    /* 0x0D68 */ s32 dashCount;       /* counter: homing dashes started out of an attack or actions 0x47..0x57, 0x5A..0x5D, 0x6B..0x6F */
    /* 0x0D6C */ u8 unkD6C[0xDD0 - 0xD6C];
    /* 0x0DD0 */ Vec4 aimDir;      /* direction a ki blast is fired in */
    /* 0x0DE0 */ s32 blastShots;       /* ki blasts fired: += the shots of motion 0x7A, or 1 */
    /* 0x0DE4 */ s32 blastResetTimer;       /* set to 30 with it (countdown in BtlAct_UpdateTimers) */
    /* 0x0DE8 */ s32 blastAltHand;
    /* 0x0DEC */ f32 blastCharge;       /* progress of the charge motion 0x8E */
    /* 0x0DF0 */ s32 blastNode;       /* result of BtlAct_PlaySubAnim */
    /* 0x0DF4 */ u8 unkDF4[0xFF0 - 0xDF4];
    /* 0x0FF0 */ s32 unkFF0;       /* countdown (BtlAct_UpdateTimers); > 0 selects the quick start of actions 0x14, 0x15, 0x17, 0x18; cleared by a jump */
    /* 0x0FF4 */ u8 unkFF4[0x1600 - 0xFF4];
} BtlActDChr; /* size 0x1600 */

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
typedef struct BtlActEPose {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ Vec4 rot;
    /* 0x20 */ u8 unk20[0x40 - 0x20];
    /* 0x40 */ Vec4 moved;     /* whole movement of the previous frame: its length is compared with a speed */
    /* 0x50 */ u8 unk50[0x80 - 0x50];
    /* 0x80 */ Vec4 vel;       /* unit direction of travel (`dir` in the other pose views) */
    /* 0x90 */ f32 pitch;      /* heading pitch */
    /* 0x94 */ f32 facing;     /* heading yaw (`yaw` in the other pose views) */
    /* 0x98 */ f32 speed;      /* forward speed */
    /* 0x9C */ f32 fallSpeed;  /* vertical speed, positive = down */
} BtlActEPose;

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
typedef struct BtlActEAttack {
    /* 0x00 */ s32 flags;
    /* 0x04 */ s16 motion[2];  /* one motion per part */
    /* 0x08 */ s8 parts;
    /* 0x09 */ u8 unk9[7];
    /* 0x10 */ f32 rate;
    /* 0x14 */ f32 cancelAt;
    /* 0x18 */ f32 leadTime;      /* lead time in frames, built up by BtlAct_PlayAttackPart */
    /* 0x1C */ f32 speed;
    /* 0x20 */ s32 chained;
} BtlActEAttack; /* size 0x24 */

/* Battle object of a fighter (BtlChar_GetObj). */
typedef struct BtlActEObj {
    /* 0x000 */ u8 unk0[0xBDC];
    /* 0xBDC */ f32 unkBDC;
} BtlActEObj;

/* Fighter (0x1600 bytes). */
typedef struct BtlActEChr {
    /* 0x0000 */ s32 player;
    /* 0x0004 */ s32 pad;
    /* 0x0008 */ s32 side;
    /* 0x000C */ s32 objId;
    /* 0x0010 */ u8 unk10[0x3D0 - 0x10];
    /* 0x03D0 */ s32 work[0x14];    /* per-action scratch, zeroed when an action starts */
    /* 0x0420 */ u8 unk420[0x964 - 0x420];
    /* 0x0964 */ s32 actionFrame;   /* frames spent in the current action */
    /* 0x0968 */ u8 unk968[0xCF8 - 0x968];
    /* 0x0CF8 */ BtlActEAttack attack;     /* latched when the attack action starts */
    /* 0x0D1C */ BtlActEAttack nextAttack; /* prepared when the attack is queued */
    /* 0x0D40 */ u8 unkD40[0xD48 - 0xD40];
    /* 0x0D48 */ s32 comboTimer;        /* countdown (see btl_char_action.h); held at 30 during clash B */
    /* 0x0D4C */ u8 unkD4C[0xE50 - 0xD4C];
    /* 0x0E50 */ s32 clashCountB;   /* inputs counted in clashes B and C */
    /* 0x0E54 */ s32 clashExchanges;        /* consecutive clash C exchanges: picks the strike animation */
    /* 0x0E58 */ s32 clashAnswerFrame;        /* clash C answer: -1 none yet, -2 wrong button, else the frame it was pressed */
    /* 0x0E5C */ u8 unkE5C[0xFF0 - 0xE5C];
    /* 0x0FF0 */ s32 unkFF0;
    /* 0x0FF4 */ u8 unkFF4[0x1278 - 0xFF4];
    /* 0x1278 */ s32 injected;      /* input is injected (CPU) */
    /* 0x127C */ u8 unk127C[0x1600 - 0x127C];
} BtlActEChr; /* size 0x1600 */

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

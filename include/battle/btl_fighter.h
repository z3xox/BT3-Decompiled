#ifndef BATTLE_BTL_FIGHTER_H
#define BATTLE_BTL_FIGHTER_H

#include "types.h"
#include "sys/math3d.h"
#include "battle/btl_cam.h"

/*
 * The fighter object (0x1600 bytes, BtlChar_Get) and the structures it is made of.
 */

#define BTL_BUTTON_BITS   31 /* entries of BtlCharInput.mask; bit 31 is never set */
#define BTL_INPUT_RING    8
#define BTL_MEMBER_SLOTS  5
#define BTL_STAT_COUNT    4  /* stats with modifiers (BTL_STAT_*, battle/btl_stats.h) */
#define BTL_STAT_SLOTS    2  /* a character has two skill slots; every modifier is kept per slot */
#define BTL_FLAG_COUNT    0x13D
#define BTL_FLAG_BYTES    0x28
#define BTL_VOICE_KINDS   57
#define BTL_SNAP_COUNT    6

/* A Vec4 aligned to 8 bytes, as the stage ground query stores it: copies of it move doublewords. */
typedef struct BtlGroundVec {
    f32 x, y, z, w;
} __attribute__((aligned(8))) BtlGroundVec;

/* The per-frame input of one fighter: 16 bytes at chr+0x938. This is everything the simulation reads from
   the controller (after BtlInput_Fetch's gating), and the replay recorder stores exactly
   {buttons, stickX, stickY} of it and rebuilds commands on playback. */
typedef struct BtlInputRecord {
    /* 0x00 */ u8 tag[4];    /* 'O', 'P', 'R', 'T' */
    /* 0x04 */ u8 player;    /* low byte of chr->player */
    /* 0x05 */ u8 unk5;      /* always 0 */
    /* 0x06 */ u8 stickX;    /* quantised left stick, BTL_STICK_CENTER = neutral; 0 / 0xFF when a d-pad bit is down */
    /* 0x07 */ u8 stickY;
    /* 0x08 */ u32 buttons;  /* BTLB_* */
    /* 0x0C */ u32 commands; /* BTLC_* */
} BtlInputRecord; /* 0x10 */

/* Five saturating (0..BTL_COUNT_MAX) frame counters for each of the 32 bits of a word (BtlInput_TickCounters). */
typedef struct BtlInputCountRow {
    s8 n[32];
} BtlInputCountRow;

typedef struct BtlInputCounters {
    /* 0x00 */ BtlInputCountRow held;           /* frames the bit has been set (0 while clear) */
    /* 0x20 */ BtlInputCountRow notHeld;        /* frames the bit has been clear (0 while set) */
    /* 0x40 */ BtlInputCountRow sincePress;     /* frames since the bit last went 0 -> 1 (0 on that frame) */
    /* 0x60 */ BtlInputCountRow sinceRelease;   /* frames since the bit last went 1 -> 0 */
    /* 0x80 */ BtlInputCountRow prevSincePress; /* sincePress as it was before this frame's update: on a press
                                                   frame, the distance to the previous press (double taps) */
} BtlInputCounters; /* 0xA0 */

/* The input block of a fighter, chr+0x570..0x948. Offsets in brackets are fighter offsets. */
typedef struct BtlCharInput {
    /* 0x000 [0x570] */ u32 rawHeld;       /* gPad[chr->pad].held (PAD_* bits) sampled by BtlInput_Sample */
    /* 0x004 [0x574] */ u8 rawStickX;      /* gPad left stick, quantised */
    /* 0x005 [0x575] */ u8 rawStickY;
    /* 0x008 [0x578] */ u32 mask[BTL_BUTTON_BITS]; /* PAD_* mask of each battle button bit */
    /* 0x084 [0x5F4] */ s32 padStatus;     /* Pad.status; double taps are only made when it is 0 or 1 */
    /* 0x088 [0x5F8] */ s32 padLastStatus; /* Pad.lastStatus */
    /* 0x08C [0x5FC] */ BtlInputCounters recCount;    /* counters of the record's buttons (before the ring) */
    /* 0x12C [0x69C] */ BtlInputCounters recCmdCount; /* counters of the record's commands */
    /* 0x1CC [0x73C] */ u32 held;          /* buttons fetched this frame (after ring and gating) */
    /* 0x1D0 [0x740] */ u32 prev;          /* held of the previous frame */
    /* 0x1D4 [0x744] */ u32 pressed;       /* held & ~prev */
    /* 0x1D8 [0x748] */ u32 released;      /* prev & ~held */
    /* 0x1DC [0x74C] */ u32 cmdHeld;       /* the same four for the command word */
    /* 0x1E0 [0x750] */ u32 cmdPrev;
    /* 0x1E4 [0x754] */ u32 cmdPressed;
    /* 0x1E8 [0x758] */ u32 cmdReleased;
    /* 0x1EC [0x75C] */ f32 stick[2];      /* smoothed stick x, y (-1..1, clamped to the unit circle) */
    /* 0x1F4 [0x764] */ f32 stickPrev[2];  /* stick of the previous frame */
    /* 0x1FC [0x76C] */ f32 stickRaw[2];   /* the fetched stick bytes as floats */
    /* 0x204 [0x774] */ f32 stickRawPrev[2];
    /* 0x20C [0x77C] */ f32 stickTurn;     /* angle between stickRaw and stickRawPrev, wrapped by BtlUtil_WrapAngle; 0 when either is neutral */
    /* 0x210 [0x780] */ BtlInputCounters count;    /* counters of held */
    /* 0x2B0 [0x820] */ BtlInputCounters cmdCount; /* counters of cmdHeld */
    /* 0x350 [0x8C0] */ u32 ringButtons[BTL_INPUT_RING];
    /* 0x370 [0x8E0] */ u32 ringCommands[BTL_INPUT_RING];
    /* 0x390 [0x900] */ u8 ringStick[BTL_INPUT_RING][2];
    /* 0x3A0 [0x910] */ s32 ringWrite;     /* next slot to write; the ring is full when (write + 1) % 8 == read */
    /* 0x3A4 [0x914] */ s32 ringRead;      /* next slot to read */
    /* 0x3A8 [0x918] */ u8 frameBits[30];  /* two interleaved 120-bit sets (byte = (bit / 8) * 2 + set), cleared at the end of
                                              every BtlInput_Update; BtlInput_TestAction(chr, id, arg) sets bit id of set (arg != 0) */
    /* 0x3C6 [0x936] */ u8 unk3C6[2];
    /* 0x3C8 [0x938] */ BtlInputRecord rec; /* this frame's record */
} BtlCharInput; /* 0x3D8 */

/* The camera of one fighter, fighter + 0x420. Offsets in brackets are fighter offsets. */
typedef struct ChrCam {
    /* 0x00 [0x420] */ Vec4 eye;      /* wanted camera position, smoothed, before the stage collision test */
    /* 0x10 [0x430] */ Vec4 pos;      /* camera position after BtlCam_TraceStage: what the views get */
    /* 0x20 [0x440] */ Vec4 rot;      /* Euler angles (pitch, yaw, roll), each wrapped to -pi..pi; w forced to 0 */
    /* 0x30 [0x450] */ Vec4 target;   /* point looked at (the trace starts here) */
    /* 0x40 [0x460] */ Vec4 bodyPos;    /* copy of *(Vec4 *)(obj + 0xFA0), refreshed by ChrCam_UpdateInput; read by BtlCharApi_GetCamBodyPos */
    /* 0x50 [0x470] */ CamShake shake;
    /* 0x70 [0x490] */ s32 hitZone;    /* BtlCam_TraceStage: the stage zone that was traced (the one hit, else the last one tried); not an object */
    /* 0x74 [0x494] */ s32 hit;       /* BtlCam_TraceStage result (1 = the stage is between target and eye); returned by BtlCharApi_GetCamPose, read by 0x1D16F8 */
    /* 0x78 [0x498] */ f32 hitFrac;   /* BtlCam_TraceStage: fraction of the segment at the hit */
    /* 0x7C [0x49C] */ s32 distMode;  /* 0..2, ChrCam_GetOffset preset; BtlChar_Reset: BattleSide_GetOptionA(side); SELECT steps it */
    /* 0x80 [0x4A0] */ f32 yaw;       /* yaw of the follow camera: the reference direction of stick movement */
    /* 0x84 [0x4A4] */ f32 yawStep;   /* last yaw change made by ChrCam_UpdateLockOnYaw */
    /* 0x88 [0x4A8] */ f32 side;      /* lock-on camera: signed angle of the camera around the fighter, relative to the line to the opponent */
    /* 0x8C [0x4AC] */ f32 yawOfs;    /* what ChrCam_CalcFixed / ChrCam_CalcLockOn took off the yaw for rot.y; no reader found */
    /* 0x90 [0x4B0] */ f32 bob;       /* phase of the vertical swing added by ChrCam_GetBob */
    /* 0x94 [0x4B4] */ f32 rate;      /* interpolation factor eye/rot -> wanted pose (ChrCam_GetRate) */
    /* 0x98 [0x4B8] */ s32 shakeOn;     /* non-zero enables shake requests and the swing; BtlChar_Reset: BattleSide_GetOptionB(side) == 0 */
    /* 0x9C [0x4BC] */ s32 unk9C;
} ChrCam; /* size 0xA0 */

/* A camera cut: six vectors and six scalars as (start, delta) pairs, blended over `total` frames.
   Fighter + 0x4C0. */
typedef struct ChrCamCut {
    /* 0x00 [0x4C0] */ s32 reqTable;       /* the two arguments of ChrCam_RequestCut */
    /* 0x04 [0x4C4] */ s32 reqIndex;
    /* 0x08 [0x4C8] */ u8 unk8[8];
    /* 0x10 [0x4D0] */ Vec4 vecA;      /* start values ... */
    /* 0x20 [0x4E0] */ Vec4 vecADelta; /* ... and what is added over the cut */
    /* 0x30 [0x4F0] */ Vec4 vecB;
    /* 0x40 [0x500] */ Vec4 vecBDelta;
    /* 0x50 [0x510] */ Vec4 vecC;
    /* 0x60 [0x520] */ Vec4 vecCDelta;
    /* 0x70 [0x530] */ f32 valA;
    /* 0x74 [0x534] */ f32 valADelta;
    /* 0x78 [0x538] */ f32 valB;
    /* 0x7C [0x53C] */ f32 valBDelta;
    /* 0x80 [0x540] */ f32 valC;
    /* 0x84 [0x544] */ f32 valCDelta;
    /* 0x88 [0x548] */ s32 nodeA;
    /* 0x8C [0x54C] */ s32 nodeA2;
    /* 0x90 [0x550] */ s32 nodeC;
    /* 0x94 [0x554] */ s32 nodeC2;
    /* 0x98 [0x558] */ s32 timer;      /* frames left; > 0 means a cut is running */
    /* 0x9C [0x55C] */ s32 total;      /* length in frames */
    /* 0xA0 [0x560] */ s32 flags;      /* CHRCUT_F_* */
    /* 0xA4 [0x564] */ u8 unkA4[0xC];
} ChrCamCut; /* size 0xB0 */

/* Action bookkeeping (chr + 0x948). The nesting is needed for BtlAct_SetQueue to match. */
typedef struct BtlActState {
    /* 0x00 */ s32 current;   /* current action id */
    /* 0x04 */ s32 request;   /* action to switch to at the next dispatch, -1 = none */
    /* 0x08 */ s32 prev;      /* read by BtlAct_GetPrev; nothing stores it */
    /* 0x0C */ s32 queue[4];  /* follow-up actions, -1 = empty */
} BtlActState; /* size 0x1C */

/* The animation ids of a fighter: fighter + 0x974. */
typedef struct BtlAnimIds {
    /* 0x0 */ s32 cur;  /* animation playing on the main layer: index into the roster's flag table */
    /* 0x4 */ s32 next; /* requested animation, started by BtlAnim_FlushRequest; -1 once started */
    /* 0x8 */ s32 prev; /* last frame's cur (written by the manager) */
    /* 0xC */ s32 sub;  /* animation on the second layer; -1 when the object is bound */
} BtlAnimIds; /* size 0x10 */

/* Gauges of one member: member entry + 0x40. */
typedef struct BtlMemberGauge {
    /* 0x00 */ s32 health;     /* 10000 = one bar */
    /* 0x04 */ s32 healthMax;  /* table value +/- 10000..30000 by ability, at least 10000 */
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 ki;         /* 0..kiMax */
    /* 0x10 */ s32 kiMax;      /* always 100000 */
    /* 0x14 */ s32 blast;      /* 0..blastMax */
    /* 0x18 */ s32 blastMax;   /* table stock count * 100000 */
    /* 0x1C */ s32 maxPower;   /* 0..30000; zeroed whenever ki is spent or drained */
    /* 0x20 */ s32 variant;    /* BattleMember.variant, the model variant */
    /* 0x24 */ s32 lowHealth;      /* low-health flags, see btl_char_mgr.h */
    /* 0x28 */ s32 lowHealthIdle;
    /* 0x2C */ s32 unk2C;
    /* 0x30 */ s32 bodyChanged;      /* cleared by a full parameter load */
    /* 0x34 */ s32 fused;      /* non-zero: healthMax is kept as it is by a parameter load */
} BtlMemberGauge; /* size 0x38 */

/* One team member: fighter + 0x9A4 + n * 0xA4. */
typedef struct BtlMember {
    /* 0x00 */ s32 chara;
    /* 0x04 */ s32 costume;
    /* 0x08 */ s32 present;    /* 1 when the slot holds a member */
    /* 0x0C */ s32 bonus[7];   /* BattleMember.bonus[1..7] */
    /* 0x28 */ s32 ability[4]; /* 128 ability bits */
    /* 0x38 */ s32 cpuLevel;
    /* 0x3C */ s32 aiType;
    /* 0x40 */ BtlMemberGauge gauge;
    /* 0x78 */ u8 unk78[0xA4 - 0x78];
} BtlMember; /* size 0xA4 */

/* Combo / damage counters of the fighter being hit: fighter + 0xD40. */
typedef struct BtlMemberCombo {
    /* 0x00 */ s32 damage;     /* health lost in the current combo */
    /* 0x04 */ s32 hits;       /* hits in the current combo; from the 5th on damage is scaled down */
    /* 0x08 */ s32 timer;      /* 30 + damage * 0.003 frames, at most 90 */
    /* 0x0C */ s32 newHit;     /* 1 when a counted hit landed */
    /* 0x10 */ s32 changed;    /* 1 when the total changed */
    /* 0x14 */ s32 drain;      /* the last damage had BTL_DMG_DRAIN */
} BtlMemberCombo;

/* Modifiers of one stat: fighter + 0xF50 + stat * 0x18. All values are levels, added up by BtlStat_GetMod. */
typedef struct BtlStatMod {
    /* 0x00 */ s8 base[BTL_STAT_SLOTS];    /* BtlStat_SetBase */
    /* 0x02 */ s8 timed[BTL_STAT_SLOTS];   /* BtlStat_SetTimed */
    /* 0x04 */ s8 untilA[BTL_STAT_SLOTS];  /* BtlStat_SetKind2 */
    /* 0x06 */ s8 untilB[BTL_STAT_SLOTS];  /* BtlStat_SetKind3 */
    /* 0x08 */ s8 untilC[BTL_STAT_SLOTS];  /* BtlStat_SetKind4 */
    /* 0x0A */ s8 penalty[BTL_STAT_SLOTS]; /* BtlStat_SetPenalty; not counted while the fighter has flag 6 */
    /* 0x0C */ s8 frame;                   /* BtlStat_SetFrameMod; cleared every frame by BtlStat_ClearFrameMods */
    /* 0x0D */ u8 unkD[3];
    /* 0x10 */ s32 timer[BTL_STAT_SLOTS];  /* frames left for timed[] */
} BtlStatMod; /* size 0x18 */

/* Two more bit sets (fighter +0x1538). That they form one structure is what makes BtlChar_SetOnceBit match. */
typedef struct BtlFlagBits {
    /* 0x0 [0x1538] */ u64 frame;  /* per-frame bits, cleared each frame by the manager */
    /* 0x8 [0x1540] */ u8 once[8]; /* never cleared here; bit 0 = "first clash event raised"; length unknown */
} BtlFlagBits;

/* Voice cooldowns of a fighter (chr + 0x1370). */
typedef struct BtlCharVoice {
    /* 0x00 */ s32 last[BTL_VOICE_KINDS];  /* line played last for each kind */
    /* 0xE4 */ s32 timer[BTL_VOICE_KINDS]; /* frames until the kind may play again */
} BtlCharVoice; /* size 0x1C8 */

/* Pad vibration request of a fighter (chr + 0x15D0). */
typedef struct BtlCharVib {
    /* 0x00 */ s32 enabled;   /* set at fighter reset from gSaveData->flags & (2 << chr->pad) */
    /* 0x04 */ f32 power;     /* 0..1, big motor */
    /* 0x08 */ s32 time;      /* frames left for the big motor */
    /* 0x0C */ s32 smallTime; /* frames left for the small motor */
    /* 0x10 */ s32 phase;     /* frames the big motor has run (modulates the power) */
    /* 0x14 */ s32 toggle;    /* small motor is on every other frame */
} BtlCharVib; /* size 0x18 */

#endif /* BATTLE_BTL_FIGHTER_H */

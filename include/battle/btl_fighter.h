#ifndef BATTLE_BTL_FIGHTER_H
#define BATTLE_BTL_FIGHTER_H

#include "types.h"
#include "sys/math3d.h"
#include "battle/btl_cam.h"

/*
 * The fighter object (0x1600 bytes, BtlChar_Get) and the structures it is made of.
 *
 * Every module used to declare its own partial view of the fighter (BtlCtlChr, BtlMgrChr, HitChr, ... 30 of them)
 * and of its parts (nine pose types, four reaction blocks, two throw blocks, ...). They were merged into the
 * structures below; docs/names/fighter/canonical.tsv lists, for every field, the names the views gave it and why
 * the name here was chosen. Where views read the same bytes with different types, the code was checked to compile
 * the same with one type; the exceptions are noted at the field (work[], pose.groundFlags, the ground vectors).
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
    /* 0x08 */ s32 prev;      /* the action of the frame before: the manager copies current here (btl_char_mgr.c);
                                 read by BtlAct_GetPrev */
    /* 0x0C */ s32 queue[4];  /* follow-up actions, -1 = empty */
} BtlActState; /* size 0x1C */

/* The animation ids of a fighter: fighter + 0x974. */
typedef struct BtlAnimIds {
    /* 0x0 */ s32 cur;  /* animation playing on the main layer: index into the roster's flag table */
    /* 0x4 */ s32 next; /* requested animation, started by BtlAnim_FlushRequest; -1 once started */
    /* 0x8 */ s32 prev; /* last frame's cur (written by the manager) */
    /* 0xC */ s32 sub;  /* animation on the second layer; -1 when the object is bound */
} BtlAnimIds; /* size 0x10 */

/* Gauges and use counters of one member: member entry + 0x40 to its end (BtlMember_GetActiveGauge). */
typedef struct BtlMemberGauge {
    /* 0x00 */ s32 health;        /* 10000 = one bar */
    /* 0x04 */ s32 healthMax;     /* table value +/- 10000..30000 by ability, at least 10000 */
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 ki;            /* 0..kiMax; set to kiMax by technique bit 4 and ability 0x43 */
    /* 0x10 */ s32 kiMax;         /* always 100000 */
    /* 0x14 */ s32 blast;         /* 0..blastMax; set to blastMax by ability 0x61 */
    /* 0x18 */ s32 blastMax;      /* table stock count * 100000 */
    /* 0x1C */ s32 maxPower;      /* 0..30000; zeroed whenever ki is spent or drained; 30000 together with held flag 6 */
    /* 0x20 */ s32 variant;       /* BattleMember.variant, the model variant in use, 0 = undamaged; non-zero: BtlObj +
                                     0xA40 bit 30 set, getting up (action 0xE1) plays voice 0x20 / 0x21 once */
    /* 0x24 */ s32 lowHealth;     /* 1 while health < 10000 */
    /* 0x28 */ s32 lowHealthIdle; /* lowHealth and nothing forbids it (see BtlChar_PostScene): a downed fighter must
                                     mash to recover (fighter + 0x1000) */
    /* 0x2C */ s32 variantVoiced; /* the voice of the changed variant was played */
    /* 0x30 */ s32 bodyChanged;   /* BtlChar_IsBodyChanged() = (bodyChanged != 0); set to 1 with flag 0xA6, cleared by a
                                     full parameter load; non-zero: this member cannot be fused with */
    /* 0x34 */ s32 fused;         /* 1 after a fusion: healthMax is kept as it is by a parameter load */
    /* 0x38 */ s32 used[5];       /* by move slot (skills 0..1, technique classes 2..4): times it was used; > 0 locks a
                                     skill with flag 0x10 */
    /* 0x4C */ s32 fired[5];      /* by slot: self-destruct techniques (flag B 0x2000) fired; > 0 locks a flag-0x2000
                                     technique */
    /* 0x60 */ s32 idleTalkCount; /* counts action 0x43; <= 0 lets the neutral state time out into it */
} BtlMemberGauge; /* size 0x64 */

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

/* pose.groundFlags as BtlCharApi_TestPoseBit80 reads bit 0x80: a byte bit-field through a 4-aligned view. Read
   straight from the 8-aligned pose, the bit-field would be loaded as a doubleword. */
typedef union BtlGroundFlagBits {
    s32 flags;
    struct {
        u8 low : 7;
        u8 bit80 : 1;
    };
} BtlGroundFlagBits;

/* The pose block, fighter + 0x10 (BtlChar_GetPos returns it); fighter + 0x100 holds last frame's copy. Offsets in
   brackets are fighter offsets. The ground vectors make it 8-aligned: the manager copies it whole with doublewords. */
typedef struct BtlFighterPose {
    /* 0x00 [0x10] */ Vec4 pos;         /* world position; w forced to 1 after a move */
    /* 0x10 [0x20] */ Vec4 rot;         /* model rotation as Euler angles; y = model yaw */
    /* 0x20 [0x30] */ Vec4 dispOfs;     /* display offset rebuilt every frame: hover bob (y) and shake (x, z); added to pos
                                           when the pose goes to the object, subtracted on the way back; y is also the
                                           distance from the origin to the feet (btl_hit_reaction.c) */
    /* 0x30 [0x40] */ Vec4 vel;         /* movement of the previous frame between snapshots 0 and 1 (BtlChar_GetSnapDelta at
                                           the end of the frame); zeroed by a placement */
    /* 0x40 [0x50] */ Vec4 move;        /* whole movement of the previous frame (BtlChar_GetMoveSince): its length is the
                                           real speed; zeroed by a placement */
    /* 0x50 [0x60] */ Vec4 rootPos;     /* translation of model node 0 (animation root motion) */
    /* 0x60 [0x70] */ Vec4 rootRot;     /* rotation of model node 0 as Euler angles; rootRot.y is added to rot.y by
                                           BtlHit_IsFromBehind */
    /* 0x70 [0x80] */ Vec4 impulse;     /* knock-back velocity, metres per frame; zeroed by action 4 */
    /* 0x80 [0x90] */ Vec4 dir;         /* unit direction of travel, w = 0; (sin yaw, 0, cos yaw, 0) at placement */
    /* 0x90 [0xA0] */ f32 pitch;        /* heading pitch, positive = up; scaled by cos(rootRot.y) in BtlChar_ApplyRootYaw */
    /* 0x94 [0xA4] */ f32 yaw;          /* heading yaw: the way the fighter faces; rot.y at placement */
    /* 0x98 [0xA8] */ f32 speed;        /* metres per frame along dir */
    /* 0x9C [0xAC] */ f32 fallSpeed;    /* vertical speed, positive = down: zeroed on landing, halved when flag 0x11 rises */
    /* 0xA0 [0xB0] */ f32 leanX;        /* model lean about x (BtlMove_SetLeanX), negated into the x angle of the object's
                                           quaternion at +0xD20; cleared each frame by the manager */
    /* 0xA4 [0xB4] */ f32 leanZ;        /* model lean about z (BtlMove_SetLeanZ) */
    /* 0xA8 [0xB8] */ f32 bobPhase;     /* hover bob phase */
    /* 0xAC [0xBC] */ f32 dashBoost;    /* forced to 1.0 except in actions 0xF, 0x3E, 0x58, 0xB0, 0xB1 (kept >= 1.0);
                                           multiplies technique speeds 4 and 5 and turn rate 0 */
    /* 0xB0 [0xC0] */ BtlGroundVec groundPos; /* ground point below the fighter: copy of the stage ground query */
    /* 0xC0 [0xD0] */ BtlGroundVec groundNrm; /* ground normal, valid with flag 0xF */
    /* 0xD0 [0xE0] */ s32 groundFlags;  /* ground query bits: 0x40 = landed out of the arena below BtlStage_GetBottom()
                                           (action 0xD9) and blocks the landing of action 0x15; 0x40000000 without
                                           0x01000000 keeps the fast descend going near the ground; any of 0x30000065
                                           cancels the ground effects; bit 0x80: see BtlGroundFlagBits */
    /* 0xD4 [0xE4] */ f32 steerYaw;     /* yaw steering offset of the homing dash */
    /* 0xD8 [0xE8] */ f32 steerPitch;   /* pitch steering offset of the homing dash */
    /* 0xDC [0xEC] */ f32 orbitNear;    /* BtlMove_RequestOrbit */
    /* 0xE0 [0xF0] */ f32 orbitFar;
    /* 0xE4 [0xF4] */ u8 unkE4[0xF0 - 0xE4];
} BtlFighterPose; /* size 0xF0 */

/* Attack parameters latched from the attack record, fighter + 0xCF8 (the running attack) and + 0xD1C (the queued
   one, prepared by BtlAct_PrepareAttack). */
typedef struct BtlFighterAttack {
    /* 0x00 */ s32 flags;      /* record +0x14 (u16): 0x40 ends the camera cut on contact, 0x80 raises flag 0xCD, 0x400
                                  ends in action 0xC */
    /* 0x04 */ s16 motion[2];  /* animation of each part: record +0x06, and + 2 for a second part (else -1) */
    /* 0x08 */ s8 parts;       /* max(record +0x04, 1) */
    /* 0x09 */ s8 level;       /* record +0x01: 1..3 = effect request 0xD at the start */
    /* 0x0A */ s8 cut;         /* camera cut passed to ChrCam_RequestCut(chr, 1, cut); < 0 = none */
    /* 0x0B */ s8 follow[2];   /* attack record indices of the follow-ups: record +0x0D.. or +0x0F.. */
    /* 0x0D */ u8 padD[3];
    /* 0x10 */ f32 rate;       /* record +0x0C * 0.1, or 1.0; the animation step is rate * 2 while a further part follows */
    /* 0x14 */ f32 cancelAt;   /* record +0x08: animation progress after which the queued follow-up starts */
    /* 0x18 */ f32 leadTime;   /* lead time in frames, built up by BtlAct_PlayAttackPart; 0 from BtlAct_PrepareAttack,
                                  15.0 from BtlAct_ResetAttack */
    /* 0x1C */ f32 speed;      /* record +0x00 * 10 / 1080: advance speed, metres per frame (0 = stand still) */
    /* 0x20 */ s32 chained;    /* a follow-up attack (not a generic cancel) is queued */
} BtlFighterAttack; /* size 0x24 */

/* The damage queue against the opponent, fighter + 0xD94, filled by BtlSuper_SetupRushDamage. */
typedef struct BtlFighterQueue {
    /* 0x00 [0xD94] */ s32 totalStart;       /* the total again (1 with technique flag B 0x400 and thr.failed set) */
    /* 0x04 [0xD98] */ s32 total;            /* health damage still to deal, rounded up to 10 */
    /* 0x08 [0xD9C] */ s32 perHit;           /* amount per trigger while more than one hit is left: total / (hits + 5), at
                                                most 15000, rounded up to 10 */
    /* 0x0C [0xDA0] */ s32 hits;             /* triggers left: the 0x200000000 events of the animation(s) */
    /* 0x10 [0xDA4] */ s32 flags;            /* BTL_DMG_*: 0x180 plus 0x800000 / 0x1000000 / 0x2000000 by class */
    /* 0x14 [0xDA8] */ s32 drainHealthStart; /* the health drain again */
    /* 0x18 [0xDAC] */ s32 drainHealth;      /* health still to drain */
    /* 0x1C [0xDB0] */ s32 drainHealthStep;  /* per frame: drain / max((to - from) / 2, 1) + 1 */
    /* 0x20 [0xDB4] */ s32 drainKiStart;     /* the ki drain again */
    /* 0x24 [0xDB8] */ s32 drainKi;          /* ki still to drain */
    /* 0x28 [0xDBC] */ s32 drainKiStep;      /* per frame */
    /* 0x2C [0xDC0] */ s32 drainFrom;        /* animation frame of the 0x20000 event: the drain runs while the frame is past
                                                it; -1 = no drain */
    /* 0x30 [0xDC4] */ s32 drainTo;          /* animation frame of the 0x40000 event: the last frame of the drain window */
} BtlFighterQueue; /* size 0x34 */

/* The rush / throw description, fighter + 0xE90, filled on both fighters by BtlColl_StartThrow. */
typedef struct BtlFighterThrow {
    /* 0x00 [0xE90] */ s32 tech;            /* BtlSuper_GetId(attacker, slot) */
    /* 0x04 [0xE94] */ s32 atkSide;         /* player index of the attacker */
    /* 0x08 [0xE98] */ s32 defSide;         /* player index of the victim */
    /* 0x0C [0xE9C] */ s32 slot;            /* technique slot 0..2; added to the actions of reactions 29, 30 and 34 */
    /* 0x10 [0xEA0] */ s32 stepCount;       /* BtlSuper_GetStepCount */
    /* 0x14 [0xEA4] */ s32 landingKind;     /* BtlSuper_GetLandingKind */
    /* 0x18 [0xEA8] */ s32 partnerStep;     /* BtlSuper_GetThrowPartnerStep, or -1 */
    /* 0x1C [0xEAC] */ s32 defFormStep;     /* BtlSuper_GetLastStep, or -1: the step in which the victim changes form */
    /* 0x20 [0xEB0] */ s32 partnerChara;    /* BtlSuper_GetThrowChara; -1 in the victim */
    /* 0x24 [0xEB4] */ s32 partnerCostume;  /* BtlSuper_GetThrowCostume; 0 in the victim */
    /* 0x28 [0xEB8] */ s32 partnerVariant;  /* BtlSuper_GetThrowGauge20; 0 in the victim */
    /* 0x2C [0xEBC] */ s32 partnerSlot;     /* BtlSuper_GetThrowObjectSlot; 1 in the victim */
    /* 0x30 [0xEC0] */ s32 useAngles;       /* technique bit 0x400000: at the end the victim is turned by angleYaw /
                                               anglePitch */
    /* 0x34 [0xEC4] */ s32 failed;          /* technique bit 0x400 and BtlChar_FrameMod(2) != 0: the victim ends in the
                                               neutral action 0xB, and a flag B 0x400 technique deals 1 damage */
    /* 0x38 [0xEC8] */ s32 holdVictim;      /* technique bit 0x100000: flags 0x8B and 9 during the throw */
    /* 0x3C [0xECC] */ s32 faceAttacker;    /* technique bit 0x800000 */
    /* 0x40 [0xED0] */ s32 koSkipLanding;   /* BtlSuper_GetFlagsA bit 0x2000000: a dead victim goes to action 0xEB */
    /* 0x44 [0xED4] */ s32 caughtLoop;      /* 1 when the catch reaction is 0x1E */
    /* 0x48 [0xED8] */ s32 atkFormChange;   /* technique bit 0x2000 and the attacker's gauge variant == 0: the attacker
                                               changes form in the last step */
    /* 0x4C [0xEDC] */ s32 defFormChange;   /* technique bit 0x80000 and the victim's gauge variant == 0: the victim
                                               changes form in step defFormStep */
    /* 0x50 [0xEE0] */ s32 placeFirstStep;  /* technique bit 0x800: the pair is placed in step 0 as well */
    /* 0x54 [0xEE4] */ s32 unk54;           /* technique bit 0x10000; passed to BtlObjAnim_PlayFrom when
                                               BtlCharApi_IsInRushSequence(objId) is set */
    /* 0x58 [0xEE8] */ s32 halfDamage;      /* the victim's animation has table bit 0x800: rush damage is halved */
    /* 0x5C [0xEEC] */ s32 raiseAtEnd;      /* technique bit 0x4000000: when the action ends the attacker is raised by its
                                               own height (BtlAct_GetHeight) */
    /* 0x60 [0xEF0] */ f32 angleYaw;        /* BtlSuper_GetThrowAngleA: yaw added to the victim's at the end */
    /* 0x64 [0xEF4] */ f32 anglePitch;      /* BtlSuper_GetThrowAngleB */
} BtlFighterThrow; /* size 0x68 */

/* Hit status, fighter + 0xF30. */
typedef struct BtlFighterHitStatus {
    /* 0x00 [0xF30] */ Vec4 carryMove; /* movement of the fighter holding this one: BtlChar_PostScene writes the holder's
                                          snapshot delta here, and it is added to the position under flag 0x8B */
    /* 0x10 [0xF40] */ s32 target;     /* player index this fighter's attack hits this frame, -1 = none */
    /* 0x14 [0xF44] */ s32 lastHitNo;  /* HitObj.hitNo of the previous frame */
    /* 0x18 [0xF48] */ s32 contact;    /* frames flag 0x65 has been seen by BtlHit_TestHit */
} BtlFighterHitStatus; /* size 0x1C */

/* What the last hit does to this fighter, fighter + 0xFB0 (BtlHit_ApplyHit fills it on the defender). */
typedef struct BtlFighterReact {
    /* 0x00 [0xFB0] */ s32 reaction;   /* pending reaction id from the attack record (1..49): 1 = none, 2 = no flinch,
                                          0x22 = caught; >= 3 = cannot act */
    /* 0x04 [0xFB4] */ s32 reactSub;   /* attack record + 0x20, or 0x13 when the attacker's member flag unk30 is set;
                                          passed to BtlAct_QueueReaction when a thrown fighter is released */
    /* 0x08 [0xFB8] */ s32 animCycle;  /* stagger animation variant counter (BtlActB_NextVariant) */
    /* 0x0C [0xFBC] */ s32 back;       /* hit from behind (the hit travels the way the fighter faces); for launches
                                          attack flag 0x1000 instead; 0 for a grab; for a throw technique bit 0x10000000 */
    /* 0x10 [0xFC0] */ s32 noBlend;    /* attack 0x55: bit 19 of the attacker's parameter flags; the throw animations start
                                          without blending */
    /* 0x14 [0xFC4] */ s32 silent;     /* attack flag 0x400: no hurt voice */
    /* 0x18 [0xFC8] */ f32 scale;      /* reaction strength 0..1: the attacker's charge (+0xD78), times the defender's
                                          parameter float + 0x60 unless flag 0x2000000; scales launch speed, knock-back
                                          time and how much mashing helps */
    /* 0x1C [0xFCC] */ f32 yaw;        /* direction the hit pushes */
    /* 0x20 [0xFD0] */ f32 turnYaw;    /* yaw the fighter is turned to under flag 0x94 (attack flag 0x100) */
    /* 0x24 [0xFD4] */ f32 turnPitch;
    /* 0x28 [0xFD8] */ f32 launchA;    /* launch reactions: attack record + 0x15 as an angle; yaw offset of a throw's
                                          release direction */
    /* 0x2C [0xFDC] */ f32 launchB;    /* attack record + 0x16 as an angle; pitch of the release direction */
    /* 0x30 [0xFE0] */ s32 stunTimer;  /* countdown shortened by mashing; > 0 = not free (BtlChar_IsFree); set from
                                          BtlSkill_GetFrames for reactions 0x17..0x1C; cleared on a KO */
    /* 0x34 [0xFE4] */ s32 unk34;      /* countdown: flag 0x96; 30 after damage from the queue */
    /* 0x38 [0xFE8] */ s32 shakeTimer; /* countdown: the model shakes sideways, the side alternating with bit 0; 3 with
                                          reaction 2 */
    /* 0x3C [0xFEC] */ s32 damage;     /* reactions 0x14, 0x16, 0x25: the damage, applied later by other code */
    /* 0x40 [0xFF0] */ s32 unk40;      /* countdown: 15 when a stagger action is left, cleared by a jump or without flag
                                          0x13; > 0 selects the quick start of actions 0x14, 0x15, 0x17, 0x18 */
    /* 0x44 [0xFF4] */ s32 slot;       /* slot of the rush attack that hit; 0..2, added to the action of reaction 41 */
    /* 0x48 [0xFF8] */ s32 blindTimer; /* countdown (flags 0x93, 0x137, 0x96); a grab sets it to BtlSkill_GetFrames(attacker,
                                          skill slot) */
} BtlFighterReact; /* size 0x4C */

/* One-frame effect request bits, fighter + 0x1262 (and last frame's at + 0x126B). */
typedef struct BtlFighterFxBits {
    u8 bits[9];
} BtlFighterFxBits;

/* Form-change request, fighter + 0x12CC: a transformation, a fusion or a member switch (BtlAct_SetFormCurrent). */
typedef struct BtlFighterForm {
    /* 0x00 [0x12CC] */ s32 index;      /* transformation index 0..3 / fusion index 0..2 */
    /* 0x04 [0x12D0] */ s32 chara;      /* character to become: the arguments of BtlChange_RequestChara */
    /* 0x08 [0x12D4] */ s32 costume;    /* transformation: the active member's costume; fusion: 0 */
    /* 0x0C [0x12D8] */ s32 cost;       /* blast gauge cost (stocks * 100000), taken when the model is in memory */
    /* 0x10 [0x12DC] */ s32 kind;       /* transformation: parameter byte + 0xA4; fusion: 5; 3 / 4 select flag 0x133 /
                                           0x132 in action 0xEE; 0..11: kind of the effect requested by fx bits 0..2 */
    /* 0x14 [0x12E0] */ s32 variant;    /* model variant: the active gauge's + 0x20, or 0 when parameter flag
                                           (0x10 << index) is set; 1 from BtlAct_SetFormCurrent */
    /* 0x18 [0x12E4] */ s32 animChara;  /* = chara */
    /* 0x1C [0x12E8] */ s32 anim1Chara; /* = chara: the character of the second animation file */
    /* 0x20 [0x12EC] */ s32 voiceChara; /* = chara */
    /* 0x24 [0x12F0] */ s32 objId;      /* extra object (BtlChange_RequestObject), the partner or a prop: parameter byte
                                           + 0xA8 (transformation) / + 0xB7 (fusion) */
    /* 0x28 [0x12F4] */ s32 objCostume; /* transformation: 0, or 2 when the active character is 0x6A; fusion: the
                                           partner's costume */
    /* 0x2C [0x12F8] */ s32 partner;    /* fusion: member index of the partner; transformation: -1 */
    /* 0x30 [0x12FC] */ s32 objVariant; /* fusion only */
    /* 0x34 [0x1300] */ s32 allowed;    /* BattleSide.changeAllowed: 0 = transformations, fusions and switches are refused
                                           (unless flag 0x10F) */
} BtlFighterForm; /* size 0x38 */

/* The partner object (a second character model), fighter + 0x1330. */
typedef struct BtlFighterPartner {
    /* 0x0 [0x1330] */ s32 active;   /* 1 once the object load job has attached it */
    /* 0x4 [0x1334] */ s32 res;      /* resource slot, released with BtlRes_Free */
    /* 0x8 [0x1338] */ s32 objId;    /* battle object, released with BtlObj_Destroy */
} BtlFighterPartner; /* size 0xC */

/* Head tracking state, fighter + 0x15A0. */
typedef struct BtlFighterLook {
    /* 0x00 [0x15A0] */ Vec4 offset;   /* smoothed offset added to the direction of the target */
    /* 0x10 [0x15B0] */ Vec4 dir;      /* smoothed unit direction to look along; w doubles as the first argument of a
                                          Vec4_Set */
    /* 0x20 [0x15C0] */ f32 height;
    /* 0x24 [0x15C4] */ s32 enabled;   /* BtlChar_SetLookEnabled */
    /* 0x28 [0x15C8] */ s32 tracking;  /* the target is inside the limits */
    /* 0x2C [0x15CC] */ s32 snap;      /* non-zero: no smoothing */
} BtlFighterLook; /* size 0x30 */

struct BtlTechPlaceTbl; /* battle/btl_tech.h */

/* The fighter (0x1600 bytes): gBtlChars->chars[n], BtlChar_Get(n). */
typedef struct BtlFighter {
    /* 0x0000 */ s32 player;            /* roster index = battle side 0 / 1: key-config index (SaveData.key[]), replay track,
                                           sound channel base, voice handle; the opponent is BtlChar_Get(player == 0) */
    /* 0x0004 */ s32 pad;               /* index into gPad (BtlChar_GetPad); BattleSide.pad */
    /* 0x0008 */ s32 side;              /* roster index again (BtlChars_Reset sets both): what BtlChar_FindBySide compares;
                                           the view whose screen filter BtlFx_UpdateScreenFilter drives */
    /* 0x000C */ s32 objId;             /* BtlObj_Get index of the fighter's battle object; BattleSide.objId */
    /* 0x0010 */ BtlFighterPose pose;
    /* 0x0100 */ BtlFighterPose prevPose; /* the pose as the manager copied it at the start of the frame */
    /* 0x01F0 */ u8 unk1F0[0x2E0 - 0x1F0]; /* 0x10..0x2E0 is cleared when the character is swapped */
    /* 0x02E0 */ Vec4 snapPos[BTL_SNAP_COUNT + 1]; /* position at each BtlChars_Snapshot(n) of this frame; [6] = the latest */
    /* 0x0350 */ Vec4 snapRot[BTL_SNAP_COUNT + 1]; /* the same for the rotation */
    /* 0x03C0 */ u32 snapMask;          /* bit n: snapshot n was taken this frame; bit 6: any */
    /* 0x03C4 */ u8 unk3C4[0x3D0 - 0x3C4];
    /* 0x03D0 */ s32 work[0x14];        /* per-action scratch, zeroed when an action starts; the use of each word is listed
                                           per handler in the action files. Some handlers keep floats in it (btl_act_1.c:
                                           [6] is the animation progress after which a queued action starts) and take
                                           the word's address as an f32 *: an f32 member in a union with work[] would
                                           compile differently */
    /* 0x0420 */ ChrCam cam;            /* the fighter camera */
    /* 0x04C0 */ ChrCamCut cut;         /* its camera cut */
    /* 0x0570 */ BtlCharInput input;
    /* 0x0948 */ BtlActState act;
    /* 0x0964 */ s32 actionFrame;       /* frames spent in the current action; does not advance while time is stopped */
    /* 0x0968 */ s32 prevActionFrame;   /* last frame's actionFrame (the manager copies it) */
    /* 0x096C */ f32 pitch;             /* pitch toward the opponent, radians (BtlAct_SetPitchMotion) */
    /* 0x0970 */ s32 holdAction;        /* set by a handler's run phase: skip next frame's forced-action check */
    /* 0x0974 */ BtlAnimIds anim;       /* anim.cur indexes the roster's motion flag table gBtlChars->tbl[0] (BtlAnim_GetId,
                                           BtlAnim_GetFlags) */
    /* 0x0984 */ f32 blendTime;         /* seconds; valid with flag 0x32 */
    /* 0x0988 */ f32 scale;             /* model scale, copied to the object (+0xA20) by BtlChar_PoseToObj: the uniform scale
                                           of its matrix; BtlAnim_SetRate writes it, the manager sets 1.0 every frame */
    /* 0x098C */ s32 stall;             /* animation advances in a row that left the frame unchanged */
    /* 0x0990 */ s32 prevStall;         /* stall before the last advance; > 0 turns the attribute test off
                                           (BtlAnim_TestAttr, BtlCharApi_ObjTestAttr) */
    /* 0x0994 */ s32 curMember;         /* index of the member that is fighting (BtlMember_GetActive) */
    /* 0x0998 */ s32 memberCount;       /* BattleSide.memberCount */
    /* 0x099C */ s32 switchGauge;       /* 0..100000: a member switch needs it full; 100000 at once when the current member
                                           has ability 0x1B */
    /* 0x09A0 */ s32 switchTarget;      /* member a switch would bring in */
    /* 0x09A4 */ BtlMember members[BTL_MEMBER_SLOTS];
    /* 0x0CD8 */ s32 switchMember;      /* member being switched to (actions 0xF3..0xF8) */
    /* 0x0CDC */ s32 switchChara;       /* member switch target: the arguments of BtlChange_RequestChara */
    /* 0x0CE0 */ s32 switchCostume;
    /* 0x0CE4 */ s32 switchVariant;
    /* 0x0CE8 */ s32 switchAnimChara;
    /* 0x0CEC */ s32 switchAnim1Chara;  /* the character of the second animation file */
    /* 0x0CF0 */ s32 switchVoiceChara;
    /* 0x0CF4 */ s32 switchEnabled;     /* BattleSide.switchEnabled */
    /* 0x0CF8 */ BtlFighterAttack attack;     /* latched when the attack action starts */
    /* 0x0D1C */ BtlFighterAttack nextAttack; /* prepared when the attack is queued */
    /* 0x0D40 */ BtlMemberCombo combo;  /* the combo this fighter is receiving; the manager keeps the maxima of damage and
                                           hits in BattleResult.maxComboDamage / maxComboHits[opponent] */
    /* 0x0D58 */ f32 searchAngle;       /* search window half angle (0.5 .. pi); 0.5 after a round reset */
    /* 0x0D5C */ f32 searchRange;       /* search window range (500 .. 1000000); 500.0 after a round reset */
    /* 0x0D60 */ s32 rushStep;          /* rush chain step 0..9 (animation 0x37 + step): < 5 picks action 0x4B over 0x4C;
                                           (rushStep + 4) % 5 indexes parameter +0x84 */
    /* 0x0D64 */ s32 rushRapidStep;     /* rapid rush step 0..4 (animation 0x3C + step) */
    /* 0x0D68 */ s32 dashCount;         /* homing dashes started out of an attack or actions 0x47..0x57, 0x5A..0x5D,
                                           0x6B..0x6F; non-zero selects attack ids 0x3E..0x42 instead of 0x34..0x3D */
    /* 0x0D6C */ s32 dashLimit;         /* per-frame level: 1 (or BtlParam_GetPoweredDashLimit with flag 6) + ability 1 / 0
                                           bonus; action 0x19 from the attack table needs dashCount < dashLimit */
    /* 0x0D70 */ s32 vanishCount;       /* vanishing attacks: below vanishLimit actions 0x5A..0x5D, else the combo finisher */
    /* 0x0D74 */ s32 vanishLimit;       /* per-frame level: 1 (or BtlParam_GetPoweredVanishLimit with flag 6) + ability 3 / 2
                                           bonus */
    /* 0x0D78 */ f32 charge;            /* attack charge level 0..1; copied into react.scale of whoever this fighter hits */
    /* 0x0D7C */ f32 chargeTimer;       /* attack charge timer 0..1 */
    /* 0x0D80 */ s32 chargeGauge;       /* 0..100000, blends the two charge rates: + attack record + 0x10 per hit, + 50000
                                           per guard result 1, drains 400 per frame down to a floor */
    /* 0x0D84 */ s32 chargeFullFrames;  /* frames the charge has been full */
    /* 0x0D88 */ s32 evasionCount;      /* + 1 per frame with flag 8 in actions 0x89..0x8C; kept through 0x2B / 0x2D / 0x2E,
                                           else 0; read as a count (BtlOpp_GetEvasionCount, modulo 3) */
    /* 0x0D8C */ s32 unkD8C;            /* action 0x41: which of the three animations is next */
    /* 0x0D90 */ s32 attackId;          /* id BtlAct_PrepareAttack was last called with; the action id for the vanishing
                                           attack */
    /* 0x0D94 */ BtlFighterQueue queue; /* damage queue against the opponent */
    /* 0x0DC8 */ u8 unkDC8[0xDD0 - 0xDC8];
    /* 0x0DD0 */ Vec4 aimDir;           /* ki blast aim direction (BtlCharApi_CalcAimDir) */
    /* 0x0DE0 */ s32 blastShots;        /* ki blasts fired in a row (+= the shots of motion 0x7A, or 1): below
                                           BtlParam_GetBlastLimitA to fire another */
    /* 0x0DE4 */ s32 blastResetTimer;   /* set to 30 by every ki blast; counts down (BtlAct_UpdateTimers) */
    /* 0x0DE8 */ s32 blastAltHand;      /* 1 = the next ki blast uses the other hand */
    /* 0x0DEC */ f32 blastCharge;       /* 0..1: progress of the charge motion 0x8E of a charged ki blast */
    /* 0x0DF0 */ s32 blastNode;         /* model node the charged ki blast leaves from (BtlAct_PlaySubAnim); effect request
                                           0x17 passes it on as the effect's node */
    /* 0x0DF4 */ u8 unkDF4[0xE00 - 0xDF4];
    /* 0x0E00 */ s32 dodges;            /* automatic evasions left (side step, flags 0x74..0x76): the stack count 0..3 of
                                           skill ids 0x0C / 0x32 (1 on use, + 1 when the skill stacks; refused at 3) */
    /* 0x0E04 */ s32 dodgesB;           /* the same of the second kind (flag 0x77), skill id 0x37 */
    /* 0x0E08 */ s32 skillTimer;        /* skill id 0x33: frames left; flag 0x56 every frame while > 0 */
    /* 0x0E0C */ s32 skillSlot;         /* slot 0 / 1 of the skill that set one of the fields above: which of flags 0xDE /
                                           0xDF a used evasion raises; added to the damage flags of attacks 0x61 / 0x62 */
    /* 0x0E10 */ s32 skillKiRate;       /* skill id 0x18: ki maximum / duration */
    /* 0x0E14 */ s32 skillTimerC;       /* frames left of a kind 1 skill with attribute 0x800 (held at 1 while kind2On /
                                           kind3On); > 0 adds one armour level and forbids lowHealthIdle */
    /* 0x0E18 */ s32 noFlinch;          /* frames left of a skill with attribute 0x400: > 0 turns every non-launch reaction
                                           into 2 (no flinch) */
    /* 0x0E1C */ s32 skillTimerE;       /* frames left for skillValE */
    /* 0x0E20 */ s32 skillValE;         /* BtlSkill_GetValE when positive; 0 when its timer runs out */
    /* 0x0E24 */ s32 skillTimerF;       /* frames left for rateBonus */
    /* 0x0E28 */ s32 rateBonus;         /* BtlSkill_GetValF when positive: per second, added to the ki charge rate
                                           (BtlParam_GetRateA / B) */
    /* 0x0E2C */ s32 unkE2C;
    /* 0x0E30 */ s32 kind2On;           /* 1 when a kind 2 skill with attribute 0x800 was used; 0 when its modifiers end
                                           (BtlStat_EndKind2) */
    /* 0x0E34 */ s32 kind3On;           /* the same for kind 3 */
    /* 0x0E38 */ s32 slotOn[BTL_STAT_SLOTS]; /* 1 while the slot's skill (non-kept modifier) is active */
    /* 0x0E40 */ s32 techDelay;         /* countdown: no technique input while > 0 (hides the technique prompt); frames from
                                           the technique (BtlSuper_GetCooldownFrames) when its action is left; cleared by
                                           BtlCtrl_UseTechnique */
    /* 0x0E44 */ f32 techCharge;        /* charge level 0..1 of a chargeable technique */
    /* 0x0E48 */ s32 techChargeFull;    /* frames techCharge has been full (the sounds play on the first) */
    /* 0x0E4C */ s32 clashCountA;       /* clash A struggle score, compared between the two fighters every frame: technique
                                           value / 20, ability bonus, + 1 per frame of input 51 */
    /* 0x0E50 */ s32 clashCountB;       /* inputs counted in clashes B and C, compared to pick the winner */
    /* 0x0E54 */ s32 clashExchanges;    /* consecutive clash C exchanges: picks the strike animation */
    /* 0x0E58 */ s32 clashAnswerFrame;  /* clash C answer: -1 none yet, -2 wrong button, else the frame it was pressed */
    /* 0x0E5C */ s32 skillCount3;       /* skill id 0x41: + 1, clamped to 0..3; scales technique 0x268, zeroed when it ends */
    /* 0x0E60 */ s32 boostStock;        /* 0..10: + 1 per throw by character 0x6E (btl_act_change.c), + 5 per technique 0x2EF,
                                           zeroed when technique 0x2CD ends, which it scales by 20 % each (btl_act_super.c) */
    /* 0x0E64 */ u8 unkE64[0xE70 - 0xE64];
    /* 0x0E70 */ Vec4 relPos;           /* offsets for the flag 0xF6 placement (BtlChar_RequestPlaceRelative) */
    /* 0x0E80 */ Vec4 relRot;
    /* 0x0E90 */ BtlFighterThrow thr;   /* rush / throw description */
    /* 0x0EF8 */ s32 rushHits;          /* measured from a rush technique's animation chain by BtlSuper_MeasureRushStep:
                                           0x200000000 events in all steps */
    /* 0x0EFC */ f32 stepFrame[5];      /* animation frame reached in each step of the rush sequence (summed by
                                           BtlCharApi_GetRushSequenceFrame) */
    /* 0x0F10 */ s32 rushFrames;        /* total length: frames of every step, + 1 between steps */
    /* 0x0F14 */ s32 rushFrameA;        /* frame, counted over the chain, of the first 0x20000 event */
    /* 0x0F18 */ s32 rushFrameB;        /* the same for the 0x40000 event (its last frame) */
    /* 0x0F1C */ s32 rushStepA;         /* step that holds the 0x20000 event, -1 = not found yet */
    /* 0x0F20 */ s32 rushStepB;         /* step that holds the 0x40000 event */
    /* 0x0F24 */ u8 unkF24[0xF30 - 0xF24];
    /* 0x0F30 */ BtlFighterHitStatus status;
    /* 0x0F4C */ u8 unkF4C[0xF50 - 0xF4C];
    /* 0x0F50 */ BtlStatMod stat[BTL_STAT_COUNT]; /* modifiers of the four stats (battle/btl_stats.h) */
    /* 0x0FB0 */ BtlFighterReact react;
    /* 0x0FFC */ s32 blindLevel;        /* follows react.blindTimer * 2 by 15 per frame (BtlAct_UpdateTimers);
                                           BtlCharApi_GetBlindRatio gives it as a ratio of 90 */
    /* 0x1000 */ s32 mashLeft;          /* downed: button presses still owed before recovering (while gauge +0x28 is set) */
    /* 0x1004 */ s8 reactCount[0x32];   /* per reaction id: times it was taken, saturating at 100 */
    /* 0x1036 */ s8 reactTimer[0x32];   /* per reaction id: zeroed when the reaction is taken, counts up to 100 */
    /* 0x1068 */ s32 dodgeWindow;       /* defence windows (BtlMove_UpdateDefenseTimers), levels: > 0 = open. This one (1
                                           with attack flag 0x4000000) avoids the first hit of a flag-1 attack */
    /* 0x106C */ s32 blastDodgeWindow;  /* > 0: a dodge without using a counter; zeroed by a rush hit */
    /* 0x1070 */ s32 counterWindow;     /* > 0 counters (guard result 8) from a 0x31 animation */
    /* 0x1074 */ s32 deflectWindow;
    /* 0x1078 */ s32 throwBreakWindow;  /* > 0 breaks a throw (attack 0x55) */
    /* 0x107C */ s32 rushBreakWindow;   /* > 0 breaks / evades a rush */
    /* 0x1080 */ s32 unk1080;           /* a seventh window: > 0 needed (with flag 0xB3) for action 0x24 */
    /* 0x1084 */ u8 stage;              /* current stage of the frame (battle/btl_char_flag.h) */
    /* 0x1085 */ u8 held[BTL_FLAG_BYTES];      /* fighter flags (BtlChar_TestFlag and friends) */
    /* 0x10AD */ u8 pulse[BTL_FLAG_BYTES];
    /* 0x10D5 */ u8 prevHeld[BTL_FLAG_BYTES];
    /* 0x10FD */ u8 prevPulse[BTL_FLAG_BYTES];
    /* 0x1125 */ u8 stamp[BTL_FLAG_COUNT];
    /* 0x1262 */ BtlFighterFxBits fxBits;     /* one-frame effect request bits */
    /* 0x126B */ BtlFighterFxBits prevFxBits; /* last frame's (the manager copies them) */
    /* 0x1274 */ u8 unk1274[0x1278 - 0x1274];
    /* 0x1278 */ s32 injectOn;          /* BattleSide_IsCpu(side): the input record is built from the three fields below
                                           (written by the AI), not from the pad; no vibration */
    /* 0x127C */ u32 injectButtons;     /* BTLB_* */
    /* 0x1280 */ f32 injectStickX;      /* -1..1 */
    /* 0x1284 */ f32 injectStickY;
    /* 0x1288 */ u64 actBits;           /* classes of the actions started this frame (BtlColl_AddActionBit) */
    /* 0x1290 */ s32 storyAiForce;
    /* 0x1294 */ s32 framesLeftOverride; /* >= 0: overrides the technique progress the AI sees */
    /* 0x1298 */ u8 unk1298[0x12A0 - 0x1298];
    /* 0x12A0 */ Vec4 savedPos;         /* placement restored by flag 0xF5 */
    /* 0x12B0 */ Vec4 savedRot;
    /* 0x12C0 */ s32 savedArea;
    /* 0x12C4 */ s32 savedFlagF;        /* fighter flag 0xF at the time of the save */
    /* 0x12C8 */ s32 savedFlagE;        /* fighter flag 0xE */
    /* 0x12CC */ BtlFighterForm form;   /* form-change request */
    /* 0x1304 */ u8 unk1304[0x1310 - 0x1304];
    /* 0x1310 */ Vec4 bodyWarpPos;      /* copy of *obj->bodyPos; passed to BtlObjBody_Warp with flag 0x55 */
    /* 0x1320 */ s32 freeze;            /* hit-stop, > 0: every phase skips this fighter (BtlChar_IsFrozen), no camera shake
                                           from BtlCharApi_ShakeCamsNear; counts down per frame */
    /* 0x1324 */ s32 freezeNext;        /* pending hit-stop: the freeze to start once freezeDelay has run out */
    /* 0x1328 */ s32 freezeDelay;
    /* 0x132C */ s32 secTimer;          /* 0..29: one "second" tick every 30 frames */
    /* 0x1330 */ BtlFighterPartner partner;
    /* 0x133C */ s32 unk133C;
    /* 0x1340 */ f32 filter;            /* screen filter strength 0..1 */
    /* 0x1344 */ s32 unk1344;
    /* 0x1348 */ s32 charaFxOn;         /* BtlFx_UpdateCharaFx: its effect is running */
    /* 0x134C */ s32 trigger1E;         /* frame counter 0..3 of effect request 0x1E */
    /* 0x1350 */ s32 trigger1F;         /* frame counter 0..3 of effect request 0x1F */
    /* 0x1354 */ s32 burstNode;         /* model node for the effect of action 0x66 (effect request 0x28) */
    /* 0x1358 */ u8 unk1358[0x1360 - 0x1358];
    /* 0x1360 */ Vec4 deflectDir;       /* direction a deflected blast is sent in; second vector of the flash of effect
                                           request 0x2E */
    /* 0x1370 */ BtlCharVoice voice;
    /* 0x1538 */ BtlFlagBits bits;      /* bits.frame: cleared each frame by the manager, BtlChar_SetFrameBits ORs bits in */
    /* 0x1548 */ u8 unk1548[0x1550 - 0x1548];
    /* 0x1550 */ s32 scriptMotion;      /* animation of action 4 (scripted motion) */
    /* 0x1554 */ f32 scriptMotionBlend; /* its blend time */
    /* 0x1558 */ s32 scriptMotionLoop;  /* its end mode: 0 play once, 1 loop */
    /* 0x155C */ s32 unk155C;
    /* 0x1560 */ Vec4 warpPos;          /* taken when held flag 0xFC is seen */
    /* 0x1570 */ Vec4 warpRot;          /* taken when held flag 0xFD is seen */
    /* 0x1580 */ s32 inputOffTimer;     /* mode 1 only: frames of flag 0x11E (input off) */
    /* 0x1584 */ u8 unk1584[0x1590 - 0x1584];
    /* 0x1590 */ s32 dirHeld;           /* -1 each frame, or the technique class 2..4 by the direction held: -1 (bit 16), 2
                                           (bit 18), 3 (bit 19), 4 (bit 17) */
    /* 0x1594 */ s32 techClass;         /* -1 each frame, or the technique class 2..4 whose button command is watched this
                                           frame: >= 2 enables BTLC_SWITCH; passed to BtlSuper_GetPromptRow */
    /* 0x1598 */ u8 unk1598[0x15A0 - 0x1598];
    /* 0x15A0 */ BtlFighterLook look;   /* head tracking */
    /* 0x15D0 */ BtlCharVib vib;        /* pad vibration (BtlChar_SetVibration / BtlChar_SetSmallVibration,
                                           BtlChar_UpdateVibration); vib.enabled = (SaveData.flags & (2 << pad)) != 0 */
    /* 0x15E8 */ struct BtlTechPlaceTbl *place[3]; /* by slot - 2: the placement table inside each of the object's three
                                                      files */
    /* 0x15F4 */ u8 unk15F4[0x1600 - 0x15F4];
} BtlFighter; /* size 0x1600 */

#endif /* BATTLE_BTL_FIGHTER_H */

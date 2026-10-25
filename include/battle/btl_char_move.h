#ifndef BATTLE_BTL_CHAR_MOVE_H
#define BATTLE_BTL_CHAR_MOVE_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Fighter movement (src/battle/btl_char_move.c, 0x1DC9A0..0x1E0290).
 * The structures are partial views local to this module: only the fields this file touches. The fighter is
 * BtlFighter (battle/btl_fighter.h).
 *
 * The movement model (everything here is verified by the matching C unless marked):
 *
 *   heading   yaw (+0x94) and pitch (+0x90) of the direction the fighter wants to travel. Yaw 0 is +Z, yaw grows
 *             towards +X (dir = (sin yaw, 0, cos yaw)); a positive pitch points up (dir.y = -sin pitch, +Y is down).
 *   dir       unit vector (+0x80) built from the heading, or straight from the stick, by BtlMove_SetDirection.
 *   speed     scalar (+0x98), metres per frame along dir. BtlMove_Advance: speed approaches its target by `accel`
 *             per frame, then pos += dir * speed. There is no separate velocity vector and no time step: one call
 *             is one 30 Hz frame.
 *   vertical  a second scalar (+0x9C, positive = down) added to pos.y: gravity adds 50 km/h per frame up to
 *             3000 km/h; in flight (flag 0xE) it is braked to 0 by 100 km/h per frame instead.
 *   impulse   a vector (+0x70) added to pos every frame and shortened by a fixed amount (knock-back).
 *   model     rot.y (+0x14) is the yaw the model is drawn with; it follows the heading yaw at a limited rate.
 *
 *   Stick to heading: target yaw = atan2f(stickX, -stickY) + chr->camYaw (BtlMove_TurnYaw mode 0), or the same
 *   rotation written out with sin / cos of -camYaw (BtlMove_SetDirection modes 0 / 1). The stick is the smoothed
 *   pair at chr+0x75C / +0x760 (BtlInput_GetStickX / Y) and is only used while a direction bit (0xF0) is held.
 *
 *   Units (inferred from the constants): one macro form, km/h * 1000 / 3600 * (1 / 30), reproduces six different
 *   constants bit for bit (10, 50, 100, 500, 800 and 3000 km/h), so lengths are metres and speeds metres per frame.
 *   Angles written in degrees and converted (1.8, 6, 18, 36 degrees).
 */

/* What obj + 0xFA0 points to. */
typedef struct BtlMoveObjBody {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ f32 radius;  /* subtracted from the stage radius */
} BtlMoveObjBody;

/* Battle object of a fighter (BtlChar_GetObj). */
typedef struct BtlMoveObj {
    /* 0x0000 */ u8 unk0[0xFA0];
    /* 0x0FA0 */ BtlMoveObjBody *bodySphere;
    /* 0x0FA4 */ u8 unkFA4[0x1660 - 0xFA4];
    /* 0x1660 */ u8 *work;   /* stage contact work buffer */
} BtlMoveObj;

/* Second stage contact of the object, at obj->work + 0x18060. */
typedef struct BtlMoveContact {
    /* 0x00 */ Vec4 unk0;
    /* 0x10 */ Vec4 normal;
} BtlMoveContact;

/* Segment for the stage line test (ColSeg_Set builds it, StgCol_TraceSegment tests it, StgCol_GetHitPos gives the hit). */
typedef struct BtlMoveSeg {
    /* 0x00 */ Vec4 a;
    /* 0x10 */ Vec4 b;
} BtlMoveSeg;

/* One record of the blast list (EftHit_GetList: 64 records and a count at +0x6400). */
typedef struct BtlMoveBlastDef {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ s16 ownerId;  /* object id of the fighter that fired it */
    /* 0x12 */ u8 unk12[0x18 - 0x12];
    /* 0x18 */ s16 kind;     /* 0, 4, 8 form one class, everything else the other */
} BtlMoveBlastDef;

typedef struct BtlMoveBlastSrc {
    /* 0x0 */ s32 ownerId;
    /* 0x4 */ s32 slot;      /* 0 / 1: not counted as incoming */
} BtlMoveBlastSrc;

typedef struct BtlMoveBlastRec {
    /* 0x00 */ u8 unk0[0xC];
    /* 0x0C */ s32 type;     /* 0: described by def, otherwise by src */
    /* 0x10 */ u8 unk10[0x10];
    /* 0x20 */ Vec4 pos;
    /* 0x30 */ Vec4 prevPos;
    /* 0x40 */ u8 unk40[0x14];
    /* 0x54 */ s32 unk54;    /* 1: ignored */
    /* 0x58 */ u8 unk58[0xC];
    /* 0x64 */ BtlMoveBlastSrc *src;
    /* 0x68 */ BtlMoveBlastDef *atk;
    /* 0x6C */ u8 unk6C[0x190 - 0x6C];
} BtlMoveBlastRec; /* size 0x190 */

typedef struct BtlMoveBlastList {
    /* 0x0000 */ BtlMoveBlastRec rec[64];
    /* 0x6400 */ s32 count;
} BtlMoveBlastList;

/* Modes of BtlMove_TurnYaw. */
#define BTL_YAW_STICK        0 /* atan2f(stickX, -stickY) + camYaw, while a direction is held */
#define BTL_YAW_STICK_RATE   1 /* yaw + maxStep * stickX */
#define BTL_YAW_OPPONENT     2 /* lock-on: towards the opponent */
#define BTL_YAW_AWAY         3 /* lock-on: away from the opponent */
#define BTL_YAW_OPPONENT2    4 /* same as 2 */
#define BTL_YAW_KEEP         6 /* 5 and 6: unchanged */
#define BTL_YAW_CENTRE       7 /* towards the stage centre */
#define BTL_YAW_FROM_CENTRE  8 /* away from the stage centre */
#define BTL_YAW_AS_OPPONENT  9 /* the opponent's yaw */

/* Modes of BtlMove_SetDirection. */
#define BTL_DIR_STICK        0 /* stick rotated by camYaw, horizontal */
#define BTL_DIR_STICK_PITCH  1 /* the same, tilted by the heading pitch */
#define BTL_DIR_YAW          2 /* heading yaw, horizontal */
#define BTL_DIR_HEADING      3 /* heading yaw and pitch */
#define BTL_DIR_BACK_YAW     4 /* reverse of 2 */
#define BTL_DIR_BACK_HEADING 5 /* reverse of 3 */
#define BTL_DIR_KEEP_FLAT    6 /* keep, y removed */
#define BTL_DIR_KEEP         7 /* keep */

void BtlMove_UpdateAnimVoice(BtlFighter *chr);
void BtlMove_ResetAnimStep(BtlFighter *chr);
void BtlMove_UpdateAction(BtlFighter *chr);
s32 BtlMove_TestContact(BtlFighter *chr, s32 floor, f32 angle);
s32 BtlMove_IsHeadingIntoFloor(BtlFighter *chr, f32 angle);
s32 BtlMove_IsHeadingIntoWall(BtlFighter *chr, f32 angle);
s32 BtlMove_CanFireBlast(BtlFighter *chr, s32 mode, s32 *outCount);
s32 BtlMove_IsBlastIncoming(BtlFighter *chr, s32 uncharged, s32 otherBlasts, s32 techniques, f32 limit);
s32 BtlMove_CountOwnBeams(BtlFighter *chr);
s32 BtlMove_ClipToRadius(Vec4 *out, Vec4 *from, Vec4 *to, f32 radius);
void BtlMove_TurnYaw(BtlFighter *chr, s32 mode, f32 maxStep);
void BtlMove_TurnPitch(BtlFighter *chr, s32 mode, f32 maxStep);
void BtlMove_SteerAtOpponent(BtlFighter *chr, f32 closeSpeed, f32 yawAccel, f32 pitchAccel, f32 yawMax, f32 pitchMax, f32 maxStep);
void BtlMove_TurnModelYaw(BtlFighter *chr, f32 maxStep, f32 rate);
void BtlMove_SetHeading(BtlFighter *chr, f32 yaw, f32 pitch);
void BtlMove_TurnToPoint(BtlFighter *chr, Vec4 *target, f32 yawStep, f32 pitchStep);
void BtlMove_SetDirection(BtlFighter *chr, s32 mode);
void BtlMove_Advance(BtlFighter *chr, f32 speed, f32 accel);
void BtlMove_Step(BtlFighter *chr, s32 yawMode, s32 pitchMode, s32 dirMode, f32 speed, f32 accel);
void BtlMove_WarpAheadOfOpponent(BtlFighter *chr, f32 lead);
void BtlMove_WarpBehindOpponent(BtlFighter *chr, f32 gap);
void BtlMove_WarpInFrontOfOpponent(BtlFighter *chr, f32 gap);
void BtlMove_SnapToOpponent(BtlFighter *chr);
s32 BtlMove_CircleOpponent(BtlFighter *chr, s32 side, s32 lead, f32 baseYaw, f32 speed);
void BtlMove_CalcApproachPoint(BtlFighter *chr, Vec4 *out, Vec4 *outTarget, f32 dist, f32 angle, f32 distScale, f32 lead);
void BtlMove_Fall(BtlFighter *chr);
void BtlMove_MoveVertical(BtlFighter *chr, f32 speed, f32 accel);
void BtlMove_BrakeVertical(BtlFighter *chr);
void BtlMove_ApplyGravity(BtlFighter *chr);
void BtlMove_SetLeanX(BtlFighter *chr, f32 angle);
void BtlMove_SetLeanZ(BtlFighter *chr, f32 angle);
s32 BtlMove_IsBlockedByOpponent(BtlFighter *chr);
void BtlMove_ClampToStage(BtlFighter *chr);
void BtlMove_RequestOrbit(BtlFighter *chr, f32 near, f32 far);
void BtlMove_ApplyOrbit(BtlFighter *chr);
s32 BtlMove_PushOut(BtlFighter *chr);
f32 BtlMove_CalcVerticalSpeed(f32 height, f32 extra);
void BtlMove_AimVerticalAtOpponent(BtlFighter *chr, s32 lead);
void BtlMove_BeginRiseToOpponent(BtlFighter *chr);
f32 BtlMove_CalcJumpSpeed(f32 height);
void BtlMove_SetImpulse(BtlFighter *chr, Vec4 *vel);
void BtlMove_SetImpulseDir(BtlFighter *chr, Vec4 *dir, f32 speed);
void BtlMove_SetImpulseYaw(BtlFighter *chr, f32 yaw, f32 speed);
void BtlMove_DeflectAtStageLimit(BtlFighter *chr, s32 back);
s32 BtlMove_ApplyImpulse(BtlFighter *chr);
void BtlMove_UpdateHoverOffset(BtlFighter *chr);
void BtlMove_UpdateDefenseTimers(BtlFighter *chr);

#endif

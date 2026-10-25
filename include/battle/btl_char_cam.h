#ifndef BATTLE_BTL_CHAR_CAM_H
#define BATTLE_BTL_CHAR_CAM_H

#include "types.h"
#include "sys/math3d.h"
#include "battle/btl_cam.h"
#include "battle/btl_fighter.h"

/* The fighter's own camera, src/battle/btl_char_cam.c = 0x1C4BF8..0x1C7B30. All names are guesses.
 *
 * Every fighter carries a camera (fighter + 0x420, ChrCam) and a "cut" block (fighter + 0x4C0, ChrCamCut)
 * with which scripted moves take the camera over. BtlCharApi_GetCamPose / BtlCam_UpdateView (btl_cam.c) only
 * copy ChrCam.pos / ChrCam.rot out into the battle views.
 *
 * == Per frame, per fighter ==
 * BtlChar_UpdateCamera (0x1C2218, control phase):
 *     ChrCam_StartCut      a cut requested with ChrCam_RequestCut (fighter flag 0xDB) is set up from its table entry
 *     ChrCam_UpdateDemo    flags 0xD7..0xDA start / stop the scripted (demo) camera
 *     ChrCam_UpdateInput   SELECT steps the distance preset; then the follow yaw:
 *                            lock-on (flag 5):  ChrCam_UpdateLockOnYaw: yaw turns towards the opponent (a quarter of
 *                                               the way per frame, all the way with flag 0xCD); `side` follows right
 *                                               stick left / right or drifts with the yaw change
 *                            otherwise:         ChrCam_UpdateFreeYaw: flag 0xC9: stick x and actions 0x61 / 0x62
 *                                               turn the yaw; flag 0xCA: the yaw follows the fighter's facing
 * BtlChar_PostScene (0x1C2318, after the scene update):
 *     ChrCam_Update        picks one wanted pose (eye, rot, target):
 *                            flag 0xCC:           keep last frame's pose
 *                            ChrCam_IsCutActive:  ChrCam_CalcCut (steps the cut's timer)
 *                            flag 0xDC or 0xB8:   ChrCam_CalcFixed
 *                            flag 5 (lock-on):    ChrCam_CalcLockOn
 *                            else:                ChrCam_CalcFree
 *                          then: CamShake_Calc (rand() x 5 while a shake runs) added to the eye, CamShake_Tick,
 *                          eye and target pulled inside the stage's horizontal limit, eye smoothed towards the
 *                          wanted eye by ChrCam_GetRate (not with flag 0xCD or last frame's 0xCE), the eye kept
 *                          behind the fighter's head (follow cameras), BtlCam_TraceStage(target -> eye) giving
 *                          pos / hit / hitFrac, a turn towards the opponent when the stage blocks the view, rot
 *                          smoothed the same way plus the shake rotation.
 *
 * == What the rest of the fighter code reads back ==
 * cam->yaw (fighter + 0x4A0) is the reference direction of stick movement: 0x1DD258 and 0x1DDD50 turn the stick
 * into a heading with atan2(stickX, -stickY) + yaw, 0x1E0670 / 0x1E06A0 / 0x1E06D0 give the fighter's rotation,
 * facing and velocity direction relative to it, 0x204748 and 0x1D09A0 compare directions with it.
 * cam->side (+0x4A8) picks left / right variants (0x1E0D68, 0x1E3D88, 0x1E3F48, 0x1ECC38, 0x1FD2F8).
 * cam->hit (+0x494) is read by 0x1D16F8. Nothing but BtlCharApi_GetCamPose reads pos / rot (+0x430 / +0x440).
 */

/* Distance presets cycled with SELECT (ChrCam_GetOffset). */
#define CHRCAM_DIST_COUNT 3

/* ChrCamCut.flags */
#define CHRCUT_F_SNAP      0x001 /* ChrCam_SetCut raises fighter flag 0xCD (no smoothing this frame) */
#define CHRCUT_F_SNAP_RUN  0x002 /* ChrCam_CalcCut raises flag 0xCD every frame of the cut */
#define CHRCUT_F_SNAP_END  0x004 /* when the timer runs out: flag 0xCE (ChrCam_CalcCut), then flag 0xCD (ChrCam_EndCut) */
#define CHRCUT_F_RATE      0x008 /* ChrCam_SetCut raises flag 0xD1 (rate 0.2) */
#define CHRCUT_F_RATE_RUN  0x010 /* ChrCam_CalcCut raises flag 0xD1 every frame */
#define CHRCUT_F_RATE_END  0x020 /* ChrCam_EndCut raises flag 0xD1 */
#define CHRCUT_F_PRIORITY  0x040 /* ChrCam_CalcCut raises flag 0xD3: this camera takes the whole screen (BtlCam_GetPriorityView) */
#define CHRCUT_F_HOLD      0x080 /* the cut stays active after its timer has run out */
#define CHRCUT_F_SET_SIDE  0x100 /* ChrCam_CalcCut sets cam->side from where the camera is relative to the fighter's facing */

/* Node ids in ChrCamCut.unk88..unk94: a model node index of the fighter, or with one of these bits; -1 = none. */
#define CHRCUT_NODE_OPP 0x40000000 /* node of the opponent's model */
#define CHRCUT_NODE_MID 0x20000000 /* midpoint of that node on fighter 0 and fighter 1 */
#define CHRCUT_NODE_MASK 0x9FFFFFFF

/* ChrCamCutDef.flags */
#define CHRCUTDEF_A_TRACK  0x0001 /* vecA / vecADelta follow nodeA / nodeA2 every frame instead of being sampled once */
#define CHRCUTDEF_OPP      0x0002 /* nodeA, yaw and scale come from the opponent */
#define CHRCUTDEF_C_TRACK  0x0004
#define CHRCUTDEF_C_OPP    0x0008
#define CHRCUTDEF_C2_TRACK 0x0010
#define CHRCUTDEF_C2_OPP   0x0020
#define CHRCUTDEF_BLEND    0x0040 /* start from the current camera (ChrCam_BlendToCut) */
#define CHRCUTDEF_MID      0x0080 /* nodeA is the midpoint of both fighters; scale is the larger one */
#define CHRCUTDEF_C_MID    0x0100
#define CHRCUTDEF_C2_MID   0x0200
#define CHRCUTDEF_MIN_SCALE 0x0400 /* scale is at least 15 */

/* One entry of a cut table (the character's at obj + 0x938, the common one at gBtlChars + 0x24). */
typedef struct ChrCamCutDef {
    /* 0x00 */ u8 nodeA;      /* model node the camera orbits ... */
    /* 0x01 */ u8 nodeA2;     /* ... moving towards this one */
    /* 0x02 */ u8 nodeC;      /* model node looked at ... */
    /* 0x03 */ u8 nodeC2;     /* ... moving towards this one */
    /* 0x04 */ f32 yaw;       /* degrees, relative to the fighter's facing */
    /* 0x08 */ f32 yawDelta;
    /* 0x0C */ f32 pitch;     /* degrees */
    /* 0x10 */ f32 pitchDelta;
    /* 0x14 */ f32 dist;      /* in body scales */
    /* 0x18 */ f32 distDelta;
    /* 0x1C */ f32 seconds;
    /* 0x20 */ u16 cutFlags;  /* CHRCUT_F_* */
    /* 0x22 */ u16 flags;     /* CHRCUTDEF_* */
} ChrCamCutDef; /* size 0x24 */

/* gBtlChars (the fighter manager): only what this file reads. */
typedef struct ChrCamMgr {
    /* 0x000 */ u8 unk0[0x24];
    /* 0x024 */ ChrCamCutDef *cutDefs; /* common cut table */
    /* 0x028 */ u8 unk28[0x274 - 0x28];
    /* 0x274 */ s32 timeStopped;            /* BtlChars_IsTimeStopped: non-zero forces rate 1 and no swing */
} ChrCamMgr;

/* The fighter's body block, fighter + 0x10 (BtlChar_GetPos returns it): only what this file reads. */
typedef struct ChrCamBody {
    /* 0x00 [0x10] */ Vec4 pos;      /* world position */
    /* 0x10 [0x20] */ u8 unk10[0x84]; /* +0x14 [0x24]: rotation y; +0x80 / +0x88 [0x90 / 0x98]: x / z of a direction (0x1E06D0) */
    /* 0x94 [0xA4] */ f32 yaw;       /* facing */
} ChrCamBody;

/* Partial view of a fighter (0x1600 bytes, BtlChar_Get): only what this file touches. */
typedef struct ChrCamChr {
    /* 0x000 */ s32 side;      /* 0 / 1 (btl_input.h: player); the opponent is BtlChar_Get(side == 0); passed to DemoCam_PlayCharAnim0..2 */
    /* 0x004 */ s32 pad;
    /* 0x008 */ s32 index;
    /* 0x00C */ s32 objId;     /* BtlObj_Get index of the model object */
    /* 0x010 */ ChrCamBody body;
    /* 0x0A8 */ u8 unkA8[0x420 - 0xA8];
    /* 0x420 */ ChrCam cam;
    /* 0x4C0 */ ChrCamCut cut;
    /* 0x570 */ u8 unk570[0x974 - 0x570];
    /* 0x974 */ s32 motion;    /* BtlAnim_GetId */
    /* 0x978 */ u8 unk978[0x1600 - 0x978];
} ChrCamChr; /* size 0x1600 */

f32 ChrCam_GetOffset(ChrCamChr *chr, Vec4 *out, s32 mode);
f32 ChrCam_CalcRate(ChrCamChr *chr, Vec4 *eye, Vec4 *target);
f32 ChrCam_GetRate(ChrCamChr *chr, Vec4 *eye, Vec4 *target);
f32 ChrCam_GetBob(ChrCamChr *chr);
void ChrCam_CalcCut(ChrCamChr *chr, Vec4 *eye, Vec4 *rot, Vec4 *target);
void ChrCam_CalcFixed(ChrCamChr *chr, Vec4 *eye, Vec4 *rot, Vec4 *target);
void ChrCam_CalcLockOn(ChrCamChr *chr, Vec4 *eye, Vec4 *rot, Vec4 *target);
void ChrCam_CalcFree(ChrCamChr *chr, Vec4 *eye, Vec4 *rot, Vec4 *target);
void ChrCam_KeepBehindHead(ChrCamChr *chr, Vec4 *eye, Vec4 *rot);
void ChrCam_UpdateLockOnYaw(ChrCamChr *chr);
void ChrCam_UpdateFreeYaw(ChrCamChr *chr);
void ChrCam_TurnToOpponent(ChrCamChr *chr, Vec4 *rot);
void ChrCam_UpdateInput(ChrCamChr *chr);
void ChrCam_Update(ChrCamChr *chr);
f32 ChrCam_GetSideLimit(ChrCamChr *chr);
void ChrCam_AddShake(ChrCamChr *chr, f32 strength, f32 time);
void ChrCam_UpdateDemo(ChrCamChr *chr);
void ChrCam_SetCut(ChrCamChr *chr, Vec4 *vecA, Vec4 *vecADelta, Vec4 *vecB, Vec4 *vecBDelta, Vec4 *vecC,
                   Vec4 *vecCDelta, s32 nodeA, f32 valA, f32 valADelta, f32 valB, f32 valBDelta, f32 valC,
                   f32 valCDelta, s32 nodeA2, s32 nodeC, s32 nodeC2, s32 time, s32 flags);
void ChrCam_BlendToCut(ChrCamChr *chr, Vec4 *vecADelta, Vec4 *vecBDelta, Vec4 *vecCDelta, s32 nodeA2, s32 nodeC2,
                       f32 valADelta, f32 valBDelta, f32 valCDelta, s32 time, s32 flags);
void ChrCam_RequestCut(ChrCamChr *chr, s32 table, s32 index);
void ChrCam_StartCut(ChrCamChr *chr);
void ChrCam_EndCut(ChrCamChr *chr);
s32 ChrCam_IsCutActive(ChrCamChr *chr);

#endif

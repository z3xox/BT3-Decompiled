#ifndef BATTLE_BTL_CHAR_ACTION_H
#define BATTLE_BTL_CHAR_ACTION_H

#include "types.h"
#include "sys/math3d.h"
#include "battle/btl_fighter.h"

/*
 * Fighter action state machine, core part (src/battle/btl_char_action.c, 0x1E0290..0x1E3158).
 * The structures are partial views local to this module: only the fields this file touches.
 *
 * An action is a number 0..0x13B. gBtlActHandlers[id] is `s32 handler(chr, phase)` (NULL for 0):
 *   phase 0  enter      called once, on the frame the action becomes current
 *   phase 1  run        called every frame (also on the enter frame, after phase 0)
 *   phase 2  decide     called every frame before the forced-action check; requests the next action
 *   phase 3  leave      called once on the old action, when a request is accepted
 * The handlers are int functions that fall off their end (they are not compiled as void: their last
 * call is not a tail call), and the dispatcher ignores the value. A handler ends its action by
 * calling BtlAct_Request(chr, next); BtlAct_Update makes the switch on the next frame's dispatch.
 */

#define BTLACT_COUNT 0x13C

#define BTLACT_PHASE_ENTER  0
#define BTLACT_PHASE_RUN    1
#define BTLACT_PHASE_DECIDE 2
#define BTLACT_PHASE_LEAVE  3

typedef s32 (*BtlActHandler)(BtlFighter *chr, s32 phase);

/* Pose block (chr + 0x10, what BtlChar_GetPos returns). */
typedef BtlFighterPose BtlActPose;

/* Attack parameters built from an attack table record (0x24 bytes; pending at chr + 0xD1C, latched at + 0xCF8). */
typedef BtlFighterAttack BtlActAttack;

/* Attack table record (roster + 0x2C, index = action id - 0x70). */
typedef struct BtlActAttackRec {
    /* 0x00 */ u8 speed;
    /* 0x01 */ u8 leadIn;
    /* 0x02 */ u8 unk2[2];
    /* 0x04 */ s8 parts;
    /* 0x05 */ s8 kind;        /* 0..8, switched on by the queue builder BtlDecide_QueueAttack */
    /* 0x06 */ u16 motion;
    /* 0x08 */ f32 cancelAt;
    /* 0x0C */ s8 rate;
    /* 0x0D */ s8 follow[2];   /* attack record indices (follow-ups, inferred) */
    /* 0x0F */ s8 followAlt[2]; /* used instead in the powered-up mode (flag 6) when that record is available */
    /* 0x11 */ s8 cut;         /* camera cut when the camera sits on the negative side (or by frame parity) */
    /* 0x12 */ s8 cutAlt;      /* camera cut for the other side; < 0 = always use cut */
    /* 0x13 */ s8 needSkill;   /* bit number tested by BtlAct_TestAttackSkill; < 0 = always available */
    /* 0x14 */ u16 flags;      /* 0x20 = pick the cut from the camera side instead of the frame parity */
    /* 0x16 */ u8 unk16[2];
} BtlActAttackRec; /* size 0x18 */

/* Member entry (chr + 0x9A4 + i * 0xA4). */
typedef BtlMemberGauge BtlActVitals;

typedef BtlMember BtlActMember;

/* Form-change request (chr + 0x12CC). */
typedef BtlFighterForm BtlActForm;

/* Fighter (0x1600 bytes). */
typedef BtlFighter BtlActChr;

/* Fighter roster (gBtlChars). */
typedef struct BtlActRoster {
    /* 0x00 */ s32 count;
    /* 0x04 */ BtlActChr *chars;
    /* 0x08 */ u8 unk8[0x20 - 0x8];
    /* 0x20 */ u32 *motionFlags;         /* 0x19E words, by motion id */
    /* 0x24 */ void *cutTbl;
    /* 0x28 */ void *voiceTbl;
    /* 0x2C */ BtlActAttackRec *attackTbl; /* by action id - 0x70 */
} BtlActRoster;

void BtlAct_Request(BtlActChr *chr, s32 id);
void BtlAct_SetQueue(BtlActChr *chr, u32 slot, s32 id);
void BtlAct_ClearQueue(BtlActChr *chr);
s32 BtlAct_HasQueued(BtlActChr *chr);
s32 BtlAct_GetCurrent(BtlActChr *chr);
s32 BtlAct_GetRequested(BtlActChr *chr);
s32 BtlAct_GetPrev(BtlActChr *chr);
s32 BtlAct_GetQueued(BtlActChr *chr);
void BtlAct_PopQueue(BtlActChr *chr);
s32 BtlAct_IsTechniqueId(s32 id);
s32 BtlAct_GetIdClass(BtlActChr *chr, s32 id);
s32 BtlAct_GetCurrentClass(BtlActChr *chr);
s32 BtlAct_GetPrevClass(BtlActChr *chr);
s32 BtlAct_GetMotionLevel(BtlActChr *chr, s32 motion);
f32 BtlAct_GetGroundY(BtlActChr *chr);
f32 BtlAct_GetHeight(BtlActChr *chr);
f32 BtlAct_GetHeightRatio(BtlActChr *chr);
f32 BtlAct_GetFramesToGround(BtlActChr *chr);
f32 BtlAct_GetRotYRelCam(BtlActChr *chr);
f32 BtlAct_GetFacingRelCam(BtlActChr *chr);
f32 BtlAct_GetVelDirRelCam(BtlActChr *chr);
f32 BtlAct_ScaleSpeedByApproach(BtlActChr *chr, f32 speed, f32 range);
f32 BtlAct_ScaleSpeedByDist(BtlActChr *chr, f32 speed, f32 range, f32 frames);
void BtlAct_SetPitchMotion(BtlActChr *chr, s32 motionUp, s32 motionDown, s32 recalc);
s32 BtlAct_IsAirMotion(BtlActChr *chr, s32 useSaved);
void BtlAct_PrepareSwitch(BtlActChr *chr);
void BtlAct_PushAngle(BtlActChr *chr, f32 angle, f32 speed, f32 max);
void BtlAct_PushDir(BtlActChr *chr, Vec4 *dir, f32 speed, f32 max);
void BtlAct_AddSway(BtlActChr *chr, f32 add, f32 max);
s32 BtlAct_IsAttackId(s32 id);
s32 BtlAct_TestAttackSkill(BtlActChr *chr, s32 attack);
void BtlAct_PrepareAttack(BtlActChr *chr, s32 id);
void BtlAct_ResetAttack(BtlActChr *chr);
void BtlAct_LatchAttack(BtlActChr *chr);
s32 BtlAct_TestPoweredSkill(BtlActChr *chr, u32 mask);
void BtlAct_SetFormCurrent(BtlActChr *chr);
void BtlAct_SetFormRandom(BtlActChr *chr);
void BtlAct_CountAndMarkOpponent(BtlActChr *chr);
void BtlAct_EndFlag98(BtlActChr *chr);
void BtlAct_ApplyFlag94(BtlActChr *chr);
void BtlAct_CheckForced(BtlActChr *chr);
void BtlAct_UpdateGauges(BtlActChr *chr);
void BtlAct_UpdateTimers(BtlActChr *chr);
void BtlAct_Update(BtlActChr *chr);
s32 BtlAct_Action01(BtlActChr *chr, s32 phase);
s32 BtlAct_Action02(BtlActChr *chr, s32 phase);
s32 BtlAct_Action03(BtlActChr *chr, s32 phase);
s32 BtlAct_Action04(BtlActChr *chr, s32 phase);
s32 BtlAct_Action05(BtlActChr *chr, s32 phase);

#endif

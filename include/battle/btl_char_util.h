#ifndef BATTLE_BTL_CHAR_GET_H
#define BATTLE_BTL_CHAR_GET_H

#include "types.h"
#include "sys/math3d.h"
#include "sys/pad.h"
#include "battle/btl_fighter.h"

/*
 * Small battle helpers and fighter accessors (src/battle/btl_char_util.c, 0x1DBF20..0x1DC9A0).
 * The structures are partial views local to this module: only the fields this file touches. The fighter is
 * BtlFighter (battle/btl_fighter.h).
 */

/* Entry of the voice table (roster + 0x28). */
typedef struct BtlVoiceEntry {
    /* 0x0 */ s16 first;    /* first line */
    /* 0x2 */ s16 count;    /* number of lines */
    /* 0x4 */ f32 interval; /* cooldown in seconds */
} BtlVoiceEntry; /* size 0x8 */

/* Fighter roster (gBtlChars, 0x280 bytes). */
typedef struct BtlCharGetRoster {
    /* 0x00 */ s32 count;
    /* 0x04 */ BtlFighter *chars;
    /* 0x08 */ u8 unk8[0x14 - 0x8];
    /* 0x14 */ s32 frame;       /* frames simulated since BtlChar_ResetAll, 30 bits */
    /* 0x18 */ u32 randState;   /* BtlChar_Rand; 0 at BtlChar_ResetAll */
    /* 0x1C */ s32 stageTimer;  /* BtlChar_AddStageTimer; counts down to 0 */
    /* 0x20 */ u8 unk20[0x28 - 0x20];
    /* 0x28 */ BtlVoiceEntry *voiceTbl;
} BtlCharGetRoster;

f32 BtlUtil_WrapAngle(f32 a);
void BtlUtil_WrapAngles(Vec4 *dst, Vec4 *src);
f32 BtlUtil_LengthXZ(Vec4 *v);
f32 BtlUtil_ApproachF(f32 cur, f32 target, f32 step);
s32 BtlUtil_Approach(s32 cur, s32 target, s32 step);
f32 BtlUtil_ClampF(f32 v, f32 lo, f32 hi);
f32 BtlUtil_MinF(f32 a, f32 b);
f32 BtlUtil_MaxF(f32 a, f32 b);
s32 BtlUtil_Clamp(s32 v, s32 lo, s32 hi);
s32 BtlUtil_Min(s32 a, s32 b);
s32 BtlUtil_Max(s32 a, s32 b);
s32 BtlUtil_RoundUp(s32 n, s32 unit);
s32 BtlUtil_AngleToSector(f32 a);
s32 BtlChar_GetCount(void);
BtlFighter *BtlChar_Get(s32 i);
BtlFighter *BtlChar_FindByObjId(s32 objId);
BtlFighter *BtlChar_FindBySide(s32 side);
void *BtlChar_GetObj(BtlFighter *chr);
Vec4 *BtlChar_GetPos(BtlFighter *chr);
Pad *BtlChar_GetPad(BtlFighter *chr);
s32 BtlChar_IsFrozen(BtlFighter *chr);
s32 BtlChar_IsFree(BtlFighter *chr);
s32 BtlChar_IsDead(BtlFighter *chr);
s32 BtlChar_IsBodyChanged(BtlFighter *chr);
s32 BtlChar_GetFrame(void);
s32 BtlChar_FrameMod(s32 n);
s32 BtlChar_Rand(void);
f32 BtlChar_RandF(void);
s32 BtlChar_IsStage4Or27(void);
void BtlChar_AddStageTimer(f32 seconds);
void BtlChar_SetVibration(BtlFighter *chr, f32 power, f32 seconds);
void BtlChar_SetSmallVibration(BtlFighter *chr, f32 seconds);
void BtlChar_Vibrate(BtlFighter *chr, f32 power, f32 seconds);
void BtlChar_UpdateVibration(BtlFighter *chr);
BtlVoiceEntry *BtlChar_GetVoiceEntry(s32 kind);
void BtlChar_PlayVoice(BtlFighter *chr, s32 kind);
void BtlChar_StopVoiceOnFlag(BtlFighter *chr);
void BtlChar_TickVoiceTimers(BtlFighter *chr);
void BtlChar_PlayVoiceSingle(BtlFighter *chr, s32 kind);

#endif

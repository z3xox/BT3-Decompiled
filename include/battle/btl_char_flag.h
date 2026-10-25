#ifndef BATTLE_BTL_CHAR_FLAG_H
#define BATTLE_BTL_CHAR_FLAG_H

#include "types.h"
#include "sys/math3d.h"
#include "battle/btl_fighter.h"

/*
 * Fighter clash sequences, fighter sound, fighter flags and opponent helpers, 0x1D8750..0x1DBF20:
 *   src/battle/btl_clash.c  0x1D8750..0x1D9A20  clash sequences
 *   src/battle/btl_char_sound.c    0x1D9A20..0x1DA8B8
 *   src/battle/btl_char_flag.c        0x1DA8B8..0x1DB048
 *   src/battle/btl_opponent.c    0x1DB048..0x1DBF20
 * The structures are partial views local to these files: only the fields they touch. The fighter is BtlFighter
 * (battle/btl_fighter.h).
 *
 * ---- Fighter flags ----
 * A fighter has BTL_FLAG_COUNT (0x13D = 317) flags, numbered from 0, kept as bit arrays of 0x28 bytes
 * (bit n is bit n & 7 of byte n >> 3). There are four arrays and one byte per flag:
 *   held[]      (+0x1085) "held" flags: stay up until BtlChar_ClearFlag
 *   pulse[]     (+0x10AD) plain flags: stay up for one frame (see below)
 *   prevHeld[]  (+0x10D5) the held array as it was one frame ago
 *   prevPulse[] (+0x10FD) the plain array as it was one frame ago
 *   stamp[]     (+0x1125) per flag: the stage number current when the flag was last written
 * BtlChar_TestFlag answers held | pulse, so a reader cannot tell the two kinds apart.
 *
 * The frame is cut into "stages": BtlChar_SetStage(chr, n) is called by the fighter manager at the start
 * of each of its per-fighter passes (1 reset, 2 sample, 3 input, 6..10 the main passes, 11 events, 12 end)
 * and stores n at +0x1084. Every write of a flag stamps it with the stage it was written in. When the
 * same stage comes round again one frame later, BtlChar_SetStage "promotes" every flag carrying that
 * stamp: prevHeld := held, prevPulse := pulse, pulse := 0. So
 *   - a plain flag set in stage S is visible to every stage from S of this frame up to stage S - 1 of
 *     the next: exactly one frame, whoever reads it, whatever the order of the passes;
 *   - "previous" means "the value this flag had when its writer's stage last began";
 *   - a flag written again before its stage comes round (stage < stamp: the frame has wrapped) takes
 *     its own snapshot first, so the previous value is not lost.
 * When the stage number goes down (a new frame starts) stage 1 is promoted as well, whatever the new
 * stage is; that covers flags written in stage 1 when the reset pass does not run.
 * Flags stamped 0 (written before the first BtlChar_SetStage) or with a stage that is never entered
 * (4, 5) are never promoted.
 */


/* One playing sound of a side (roster +0x8 / +0xC point at one BtlFlagSoundSet per side). */
typedef struct BtlFlagSound {
    /* 0x0 */ s32 handle; /* Snd_PlaySeEx handle, < 0 = free slot */
    /* 0x4 */ s32 id;     /* sound id inside the bank */
    /* 0x8 */ u8 kind;    /* request kind: index into gBtlSndBankMask */
} BtlFlagSound; /* size 0xC */

typedef struct BtlFlagSoundSet {
    /* 0x00 */ BtlFlagSound slot[4];
    /* 0x30 */ s32 next;  /* ring index of the slot tried first */
} BtlFlagSoundSet; /* size 0x34 */

/* What to play (second half of a request). */
typedef struct BtlFlagSndParam {
    /* 0x00 [0x10] */ f32 near;  /* full volume up to this distance from the camera */
    /* 0x04 [0x14] */ f32 far;   /* silent from this distance */
    /* 0x08 [0x18] */ s32 id;    /* sound id, stream id or voice line */
    /* 0x0C [0x1C] */ s8 owner;  /* fighter index, -1 = none (played as side 0) */
    /* 0x0D [0x1D] */ u8 kind;   /* 0..3 sample banks 4 / 8 / 0x10 / 0x20, 4 stream, 5 6 character voice, 7 voice by id */
} BtlFlagSndParam; /* size 0x10 */

/* A positional sound request (roster +0xA0, four of them). */
typedef struct BtlFlagSndReq {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ BtlFlagSndParam p;
} BtlFlagSndReq; /* size 0x20 */

typedef struct BtlFlagSndReqList {
    /* 0x00 */ BtlFlagSndReq req[4];
    /* 0x80 */ s32 count;
} BtlFlagSndReqList;

/* Clash A state (roster +0x50). */
typedef struct BtlClashA {
    /* 0x00 */ s32 timer;
    /* 0x04 */ s32 winner; /* player index of the winner */
    /* 0x08 */ s32 lead;   /* balance: > 0 fighter 0 is ahead */
    /* 0x0C */ f32 bias;   /* lead squashed into -0.8..0.8 */
    /* 0x10 */ Vec4 mid;   /* point between the two fighters' node 0x11, weighted by bias */
} BtlClashA; /* size 0x20 */

/* Clash B state (roster +0x70). */
typedef struct BtlClashB {
    /* 0x0 */ s32 timer;
    /* 0x4 */ s32 winner;
} BtlClashB;

/* Clash C state (roster +0x7C); also read by the action 0xFC code and by BtlChar_PlaceOnPath. */
typedef struct BtlClashC {
    /* 0x00 [0x7C] */ s32 level;  /* 1, or gProgress +0x3C in mode 1; written here, read elsewhere */
    /* 0x04 [0x80] */ s32 count;  /* exchanges so far */
    /* 0x08 [0x84] */ s32 path;   /* stage path the pair is placed on, 1 .. paths - 1 */
    /* 0x0C [0x88] */ s32 point;  /* point of that path */
    /* 0x10 [0x8C] */ s32 hold;   /* exchanges left before the path changes */
    /* 0x14 [0x90] */ s32 pick;   /* 0..3, drawn each exchange */
    /* 0x18 [0x94] */ s32 prevPick;
} BtlClashC;

/* A stage path as returned by BtlStage_GetPath (the same thing as BtlCtlPath in btl_char_ctl.h). */
typedef struct BtlClashPath {
    /* 0x0 */ s32 end;      /* one past the last usable point */
    /* 0x4 */ f32 *points;  /* 0x30 bytes each: x y z ? rx ry ... */
    /* 0x8 */ s32 first;    /* first usable point */
} BtlClashPath;

/* Fighter roster (gBtlChars, 0x280 bytes). */
typedef struct BtlFlagRoster {
    /* 0x00 */ s32 count;
    /* 0x04 */ BtlFighter *chars;
    /* 0x08 */ BtlFlagSoundSet *sounds;     /* one-shot sounds, one set per side */
    /* 0x0C */ BtlFlagSoundSet *loopSounds; /* looping sounds, one set per side */
    /* 0x10 */ u8 unk10[0x40 - 0x10];
    /* 0x40 */ s32 clashState;              /* 0 none, 1..5 clash A, 6..8 clash B, 9..11 clash C */
    /* 0x44 */ u8 unk44[0x50 - 0x44];
    /* 0x50 */ BtlClashA clashA;
    /* 0x70 */ BtlClashB clashB;
    /* 0x78 */ u8 unk78[0x7C - 0x78];
    /* 0x7C */ BtlClashC clashC;
    /* 0x98 */ u8 unk98[0xA0 - 0x98];
    /* 0xA0 */ BtlFlagSndReqList snd;
} BtlFlagRoster;

/* btl_clash.c */
s32 BtlClash_BothInClashA(void);
void BtlClash_HoldClashA(s32 player);
s32 BtlClash_BothInActionFA(void);
void BtlClash_EndActionFA(s32 player);
s32 BtlClash_BothInActionFB(void);
s32 BtlClash_BothInActionFC(void);
void BtlClash_EndActionFC(s32 player);
void BtlClash_SetOrbitCut(s32 player, s32 type);
void BtlClash_SetMidCut(Vec4 *mid, f32 bias);
void BtlClash_SetPathCut(s32 n);
f32 BtlClash_CalcBias(s32 lead);
s32 BtlClash_UpdateA(s32 state);
s32 BtlClash_UpdateB(s32 state);
s32 BtlClash_UpdateC(s32 state);
void BtlClash_Update(void);

/* btl_char_sound.c */
BtlFlagSoundSet *BtlCharSnd_GetSet(BtlFighter *chr);
BtlFlagSoundSet *BtlCharSnd_GetLoopSet(BtlFighter *chr);
s32 BtlCharSnd_IsLoopPlaying(s32 player, s32 kind, s32 id);
s32 BtlCharSnd_IsRequested(s32 owner, s32 kind, s32 id);
void BtlCharSnd_Request(Vec4 *pos, s32 owner, s32 kind, f32 near, f32 far, s32 id);
void BtlCharSnd_StoreHandle(BtlFlagSoundSet *set, s32 handle, s32 player, s32 kind, s32 id, s32 force);
void BtlCharSnd_StopUnrequestedLoops(BtlFighter *chr);
void BtlCharSnd_CalcVolPan(Vec4 *pos, s32 *vol, s32 *pan, f32 near, f32 far);
void BtlCharSnd_Init(void);
void BtlCharSnd_ClearRequests(void);
void BtlCharSnd_PlayRequests(void);
void BtlCharSnd_RequestAt(Vec4 *pos, s32 kind, s32 id, f32 near, f32 far);
void BtlCharSnd_PlayCommon(BtlFighter *chr, s32 id);
void BtlCharSnd_PlayCommonFar(BtlFighter *chr, s32 id);
void BtlCharSnd_PlayBank8(BtlFighter *chr, s32 id);
void BtlCharSnd_PlayOwn(BtlFighter *chr, s32 id);
void BtlCharSnd_PlayStream(BtlFighter *chr, s32 id);
void BtlCharSnd_PlayVoice(BtlFighter *chr, s32 line);
void BtlCharSnd_PlayTechSounds(BtlFighter *chr);
void BtlCharSnd_Stop(BtlFighter *chr, s32 kind, s32 id);
void BtlCharSnd_StopCommon(BtlFighter *chr, s32 id);
void BtlCharSnd_StopOwn(BtlFighter *chr, s32 id);
s32 BtlCharSnd_GetBankMask(s32 kind);

/* btl_char_flag.c */
void BtlChar_PromoteFlags(BtlFighter *chr, u8 stage);
void BtlChar_SetStage(BtlFighter *chr, u8 stage);
u32 BtlChar_GetStage(BtlFighter *chr);
void BtlChar_SetHeldFlag(BtlFighter *chr, u32 n);
void BtlChar_ClearFlag(BtlFighter *chr, u32 n);
void BtlChar_ClearFlagRange(BtlFighter *chr, u32 first, u32 last);
void BtlChar_ToggleHeldFlag(BtlFighter *chr, u32 n);
void BtlChar_SetFlag(BtlFighter *chr, u32 n);
s32 BtlChar_TestFlag(BtlFighter *chr, u32 n);
s32 BtlChar_TestPrevFlag(BtlFighter *chr, u32 n);
s32 BtlChar_IsFlagRaised(BtlFighter *chr, u32 n);
s32 BtlChar_IsFlagDropped(BtlFighter *chr, u32 n);
void BtlChar_UpdateLockOn(BtlFighter *chr);
void BtlChars_UpdateSightFlag(void);
void BtlChar_SetOnceBit(BtlFighter *chr, s32 n);
s32 BtlChar_TestOnceBit(BtlFighter *chr, s32 n);
void BtlChar_SetFrameBits(BtlFighter *chr, u64 bits);
void BtlChar_RaiseFirstClash(BtlFighter *chr);

/* btl_opponent.c */
void BtlOpp_GetTargetPos(BtlFighter *chr, Vec4 *out);
void BtlOpp_GetSnapPos(BtlFighter *chr, Vec4 *out, s32 slot);
void BtlOpp_GetTargetRot(BtlFighter *chr, Vec4 *out);
void BtlOpp_GetSnapRot(BtlFighter *chr, Vec4 *out, s32 slot);
f32 BtlOpp_GetTargetYaw(BtlFighter *chr);
void BtlOpp_GetDelta(BtlFighter *chr, Vec4 *out);
void BtlOpp_GetPoseVec30(BtlFighter *chr, Vec4 *out);
void BtlOpp_GetVelocity(BtlFighter *chr, Vec4 *out);
f32 BtlOpp_GetDistance(BtlFighter *chr);
f32 BtlOpp_GetDistanceXZ(BtlFighter *chr);
f32 BtlOpp_GetGapXZ(BtlFighter *chr);
f32 BtlOpp_GetYaw(BtlFighter *chr);
f32 BtlOpp_GetYawFromFacing(BtlFighter *chr);
f32 BtlOpp_GetYawFromItsFacing(BtlFighter *chr);
f32 BtlOpp_GetPitch(BtlFighter *chr);
f32 BtlOpp_GetPitchAdjusted(BtlFighter *chr);
f32 BtlOpp_GetHalfHeightDiff(BtlFighter *chr);
f32 BtlOpp_GetRadius(BtlFighter *chr);
s32 BtlOpp_GetParamByte2(BtlFighter *chr);
f32 BtlOpp_GetHeight(BtlFighter *chr);
f32 BtlOpp_GetCenterHeight(BtlFighter *chr);
s32 BtlOpp_GetParamWord0(BtlFighter *chr);
s32 BtlOpp_GetPlayer(BtlFighter *chr);
s32 BtlOpp_GetObjId(BtlFighter *chr);
void *BtlOpp_GetObj(BtlFighter *chr);
s32 BtlOpp_GetSeenAction(BtlFighter *chr);
s32 BtlOpp_GetSeenActionFrame(BtlFighter *chr);
f32 BtlOpp_GetObjUnkFF8(BtlFighter *chr);
void BtlOpp_GetLookOffset(BtlFighter *chr, Vec4 *out);
void BtlOpp_GetNodePos(BtlFighter *chr, s32 node, Vec4 *out);
void BtlOpp_GetObjVecFA0(BtlFighter *chr, Vec4 *out);
s32 BtlOpp_GetEvasionCount(BtlFighter *chr);
f32 BtlOpp_GetClosingTime(BtlFighter *chr);
s32 BtlOpp_HasAbility(BtlFighter *chr, s32 ability);
f32 BtlOpp_GetUnk20F318(BtlFighter *chr);
s32 BtlOpp_GetClashCountB(BtlFighter *chr);
void BtlOpp_MirrorFlags(BtlFighter *chr);

#endif

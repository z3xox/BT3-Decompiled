#ifndef BATTLE_BTL_CHAR_STATUS_H
#define BATTLE_BTL_CHAR_STATUS_H

#include "types.h"
#include "battle/btl_fighter.h"

/*
 * Fighter stat modifiers and fighter animation control:
 *   src/battle/btl_stats.c       0x1C2FF0..0x1C3CA8  stat modifiers and the stat curves
 *   src/battle/btl_anim.c  0x1C3CA8..0x1C4BF8  animation playback of the fighter's battle object
 * The structures are partial views local to this module: only the fields these files touch. The fighter is
 * BtlFighter (battle/btl_fighter.h).
 */

/* ---- stat modifiers ------------------------------------------------------------------------------------------ */

/* The four modified stats: index of BtlFighter.stat[]. Meanings inferred from the curve rows each one feeds. */
#define BTL_STAT_0     0 /* with bonus[1]: rows 4..6 (multipliers 0.5..1..3) */
#define BTL_STAT_1     1 /* with bonus[2]: row 7 (multiplier 1.1875..1..0.25: lower is better, damage taken) */
#define BTL_STAT_2     2 /* with bonus[0] and bonus[3]: rows 0..2, 8, 11, 12 */
#define BTL_STAT_3     3 /* with bonus[4..6]: rows 3, 9, 10 */

/* How long a skill's modifier lasts: the byte at (object +0x930) + 0x66 + slot (BtlSkill_GetStatKind). */
#define BTL_STAT_KIND_KEEP   0 /* base[]: never removed */
#define BTL_STAT_KIND_TIMED  1 /* timed[] with timer[]: removed when the timer runs out */
#define BTL_STAT_KIND_ACTION 2 /* untilA[]: removed when the action leaves 0x105..0x132 (BtlStat_Update) */
#define BTL_STAT_KIND_3      3 /* untilB[]: removed by BtlStat_EndKind3 (caller 0x1E16C0) */
#define BTL_STAT_KIND_4      4 /* untilC[]: removed by BtlStat_EndKind4 (caller 0x1E1208) */

/* gBtlStatCurve[row][...]: the value of a derived quantity at level 80, 0 and -20 (f32[13][3] at 0x2EE7C0). */
#define BTL_CURVE_MAX  0 /* at level 80 */
#define BTL_CURVE_BASE 1 /* at level 0 */
#define BTL_CURVE_MIN  2 /* at level -20 */

#define BTL_STAT_CURVE_ROWS 13

/* ---- animation ----------------------------------------------------------------------------------------------- */

/* Bits of an animation's word in the roster table (gBtlChars + 0x20, 0x19E entries). */
#define BTL_ANIM_F_2        0x00000002 /* with F_80000000: BtlObjAnim_ZeroRootAxes(handle, 1, 0, 0) after BtlObjAnim_RebaseRoot */
#define BTL_ANIM_F_10       0x00000010 /* tested by the manager (btl_char_mgr.c) */
#define BTL_ANIM_F_20000000 0x20000000 /* started with BtlObjAnim_PlayFrom (needs the opponent's object) */
#define BTL_ANIM_F_40000000 0x40000000 /* BtlObjAnim_ZeroRootAxes(handle, 1, 1, 1) */
#define BTL_ANIM_F_80000000 0x80000000 /* BtlObjAnim_RebaseRoot(handle) */
#define BTL_ANIM_COUNT      0x19E

/* Fighter flags owned by the animation code. */
#define BTL_ANIM_FLAG_NEW        0x2B /* one frame: an animation was started on the main layer (BtlAnim_Play) */
#define BTL_ANIM_FLAG_NEW_KEEP   0x2C /* one frame: started by BtlAnim_PlayKeep */
#define BTL_ANIM_FLAG_REQ        0x2D /* held: BtlAnim_Request is pending */
#define BTL_ANIM_FLAG_REQ_KEEP   0x2E /* held: BtlAnim_RequestKeep is pending */
#define BTL_ANIM_FLAG_REQ_SUB    0x2F /* held: BtlAnim_RequestSubToMain is pending */
#define BTL_ANIM_FLAG_30         0x30 /* cleared by BtlAnim_Play */
#define BTL_ANIM_FLAG_END        0x31 /* held: the animation reached its last frame */
#define BTL_ANIM_FLAG_BLEND_REQ  0x32 /* held: fighter +0x984 holds a blend time to apply */
#define BTL_ANIM_FLAG_BLENDING   0x34 /* held: the blend counter of the object is still running */
#define BTL_ANIM_FLAG_NO_BLEND   0x37 /* tested: forces the blend time to 0 */

/* Animation player inside the battle object: object + 0xB40. Layer handles are 0x98 bytes apart. */
typedef struct BtlAnimObj {
    /* 0x000 */ void *handle;    /* main layer (what BtlObjAnim_ZeroRootAxes / BtlObjAnim_RebaseRoot / BtlObjAnim_ClearNodeRot take) */
    /* 0x004 */ f32 length;      /* last frame of the current animation */
    /* 0x008 */ u8 unk8[0x98 - 0x8];
    /* 0x098 */ void *subHandle; /* second layer (object + 0xBD8) */
    /* 0x09C */ u8 unk9C[0x138 - 0x9C];
    /* 0x138 */ f32 frame;       /* object + 0xC78: current frame */
    /* 0x13C */ f32 prevFrame;   /* object + 0xC7C: frame before the last advance */
    /* 0x140 */ f32 step;        /* object + 0xC80: frames added per advance */
    /* 0x144 */ f32 blend;       /* object + 0xC84: blend counter, runs down to 0 */
    /* 0x148 */ f32 blendStep;   /* object + 0xC88 */
    /* 0x14C */ f32 mix;      /* object + 0xC8C: BtlAnim_SetSubMix */
    /* 0x150 */ u8 unk150[0x178 - 0x150];
    /* 0x178 */ f32 rate;        /* object + 0xCB8: BtlAnim_SetObjRate, 1.0 every frame */
} BtlAnimObj;

/* The fighter's battle object (BtlChar_GetObj): only the fields used here. */
typedef struct BtlStatObj {
    /* 0x000 */ u8 unk0[0xC];
    /* 0x00C */ s32 model;       /* 0x78, 0x98, 0x99 select nodes to switch off (BtlAnim_HideModelNodes) */
    /* 0x010 */ u8 unk10[0xA40 - 0x10];
    /* 0xA40 */ u32 flags;       /* bits 0x20 and 0x40 are cleared when an animation starts */
    /* 0xA44 */ u8 unkA44[0xB40 - 0xA44];
    /* 0xB40 */ BtlAnimObj anim;
} BtlStatObj;

/* ---- fighter ------------------------------------------------------------------------------------------------- */

/* Fighter roster (gBtlChars): only the animation flag table. */
typedef struct BtlStatRoster {
    /* 0x00 */ u8 unk0[0x20];
    /* 0x20 */ u32 *animFlags; /* tbl[0]: one word per animation id, BTL_ANIM_COUNT entries */
} BtlStatRoster;

/* btl_stats.c */
void BtlStat_ClearSlotOn(BtlFighter *chr, s32 slot, s32 kind);
void BtlStat_EndKind2(BtlFighter *chr);
void BtlStat_EndKind3(BtlFighter *chr);
void BtlStat_EndKind4(BtlFighter *chr);
void BtlStat_ClearPenalty(BtlFighter *chr);
void BtlStat_ClearFrameMods(BtlFighter *chr);
void BtlStat_Reset(BtlFighter *chr);
void BtlStat_Update(BtlFighter *chr);
void BtlStat_SetBase(BtlFighter *chr, s32 stat, s32 slot, s32 val, s32 add);
void BtlStat_SetFrameMod(BtlFighter *chr, s32 stat, s32 val, s32 add);
void BtlStat_SetTimed(BtlFighter *chr, s32 stat, s32 slot, s32 val, s32 add, s32 time);
void BtlStat_SetKind2(BtlFighter *chr, s32 stat, s32 slot, s32 val, s32 add);
void BtlStat_SetKind3(BtlFighter *chr, s32 stat, s32 slot, s32 val, s32 add);
void BtlStat_SetKind4(BtlFighter *chr, s32 stat, s32 slot, s32 val, s32 add);
void BtlStat_SetPenalty(BtlFighter *chr, s32 stat, s32 slot, s32 val);
s32 BtlStat_GetMod(BtlFighter *chr, s32 stat);
s32 BtlStat_GetLevel0(BtlFighter *chr);
s32 BtlStat_GetLevel1(BtlFighter *chr);
s32 BtlStat_GetLevel2(BtlFighter *chr);
s32 BtlStat_GetLevel3(BtlFighter *chr);
s32 BtlStat_GetLevel4(BtlFighter *chr);
s32 BtlStat_GetLevel5(BtlFighter *chr);
s32 BtlStat_GetLevel6(BtlFighter *chr);
f32 BtlStat_EvalCurve(s32 level, s32 row);
s32 BtlStat_GetKiChargeBonus(BtlFighter *chr);
s32 BtlStat_GetKiRegenBonus(BtlFighter *chr);
s32 BtlStat_GetKiRecoverBonus(BtlFighter *chr);
s32 BtlStat_GetBlastGainBonus(BtlFighter *chr);
f32 BtlStat_GetMeleeDamageScale(BtlFighter *chr);
f32 BtlStat_GetGuardKiCostScale(BtlFighter *chr);
f32 BtlStat_GetKiBlastDamageScale(BtlFighter *chr);
f32 BtlStat_GetScale7(BtlFighter *chr);
f32 BtlStat_GetSpeedScale(BtlFighter *chr);
f32 BtlStat_GetBlast2DamageScale(BtlFighter *chr);
f32 BtlStat_GetUltimateDamageScale(BtlFighter *chr);
f32 BtlStat_GetMaxPowerChargeScale(BtlFighter *chr);
f32 BtlStat_GetMaxPowerExtraTime(BtlFighter *chr);

/* btl_anim.c */
void BtlAnim_ApplyTableFlags(void *handle, s32 anim);
void BtlAnim_HideModelNodes(void *handle, s32 model);
void BtlAnim_HideModelNode10(void *handle, s32 model);
void BtlAnim_Play(BtlFighter *chr, s32 anim, f32 blend);
void BtlAnim_PlayKeep(BtlFighter *chr, s32 anim, f32 blend);
void BtlAnim_SubToMain(BtlFighter *chr);
void BtlAnim_Request(BtlFighter *chr, s32 anim, f32 blend);
void BtlAnim_RequestKeep(BtlFighter *chr, s32 anim, f32 blend);
void BtlAnim_RequestSubToMain(BtlFighter *chr);
void BtlAnim_SetBlend(BtlFighter *chr, f32 blend);
void BtlAnim_SetNoBlend(BtlFighter *chr);
void BtlAnim_FlushRequest(BtlFighter *chr);
void BtlAnim_FlushRequestKeep(BtlFighter *chr);
void BtlAnim_FlushSubToMain(BtlFighter *chr);
void BtlAnim_ApplyBlend(BtlFighter *chr);
void BtlAnim_PlaySub(BtlFighter *chr, s32 anim);
void BtlAnim_SetSubMix(BtlFighter *chr, f32 v);
void BtlAnim_SetStep(BtlFighter *chr, f32 step);
void BtlAnim_SetDuration(BtlFighter *chr, f32 seconds);
void BtlAnim_SetObjRate(BtlFighter *chr, f32 rate);
void BtlAnim_SetScale(BtlFighter *chr, f32 scale);
void BtlAnim_JumpToEnd(BtlFighter *chr);
void BtlAnim_EnableHandle(BtlFighter *chr);
s32 BtlAnim_GetId(BtlFighter *chr);
s32 BtlAnim_GetNextId(BtlFighter *chr);
s32 BtlAnim_GetPrevId(BtlFighter *chr);
u32 BtlAnim_GetFlags(u32 anim);
f32 BtlAnim_GetFrame(BtlFighter *chr);
f32 BtlAnim_GetProgress(BtlFighter *chr);
f32 BtlAnim_GetLength(BtlFighter *chr);
f32 BtlAnim_GetStep(BtlFighter *chr);
f32 BtlAnim_GetObjRate(BtlFighter *chr);
s32 BtlAnim_TestAttr(BtlFighter *chr, u64 mask);
s32 BtlAnim_Advance(BtlFighter *chr, s32 flags);
s32 BtlAnim_AdvanceThen(BtlFighter *chr, s32 next, f32 blend, s32 flags);
void BtlAnim_AdvanceLoop(BtlFighter *chr, s32 flags);
s32 BtlAnim_PassedRatio(BtlFighter *chr, f32 ratio);
s32 BtlAnim_PassedFrame(BtlFighter *chr, f32 frame);
s32 BtlAnim_InRatioRange(BtlFighter *chr, f32 lo, f32 hi);
s32 BtlAnim_InFrameRange(BtlFighter *chr, f32 lo, f32 hi);
s32 BtlAnim_IsNew(BtlFighter *chr);
s32 BtlAnim_WasNew(BtlFighter *chr);

#endif

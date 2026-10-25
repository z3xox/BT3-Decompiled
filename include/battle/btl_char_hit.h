#ifndef BATTLE_BTL_CHAR_HIT_H
#define BATTLE_BTL_CHAR_HIT_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Fighter hit resolution (src/battle/btl_char_hit.c, 0x1C7B30..0x1CA520): does an attack connect, is it avoided,
 * guarded or traded, and what it does to the defender. Called once per frame from the collision step at 0x1CA520
 * (the next module), in this order:
 *
 *   for each fighter: BtlHit_BeginFrame
 *   BtlHit_CheckClash        (returns 1: stop)
 *   BtlHit_CheckRush         (returns 1: stop)
 *   BtlHit_CheckProximityAll
 *   for each fighter: BtlColl_UpdateFrozen, BtlHit_TestHit
 *   BtlHit_ResolveTrades
 *   for each fighter: BtlHit_ApplyHit
 *
 * A hit is not an object. It is: the attacker's current attack id (BtlAtk_GetId, -1 = none), the attack record that
 * id selects (0x30 bytes at fighter object + 0x920, read through the func_0020Dxxx accessors, not decompiled), the
 * pending target HitStatus.target of the attacker, and the HitReact block that BtlHit_ApplyHit fills on the defender.
 *
 * All structures are partial views local to this module. Names are guesses unless the comment gives the evidence.
 */

/* Results passed to BtlHit_ApplyGuard. */
#define BTL_GUARD_PLAIN      0 /* attack record guard result 0 */
#define BTL_GUARD_FLAG40     1 /* defender flag 0x40, attack id below 11 */
#define BTL_GUARD_TIMED_A    2 /* defender flag 0x41, input 0x10 held: defender flag 0x68 */
#define BTL_GUARD_TIMED_B    3 /* input 0x20 held: defender flag 0x69 */
#define BTL_GUARD_TIMED_C    4 /* neither: defender flag 0x6A */
#define BTL_GUARD_KI         5 /* guard result 1: defender pays ki, flag 0xBE when it runs out */
#define BTL_GUARD_PUSH       6 /* guard result 2: attacker flag 0x6F, defender + 50000 blast */
#define BTL_GUARD_PUSH_STOP  7 /* guard result 4: the same with hit-stop and voice 0x12 */
#define BTL_GUARD_COUNTER    8 /* defender level at + 0x1070: flag 0x7B, hit-stop, no chip damage */

/* Reaction ids (HitReact.reaction) this module tests by value. */
#define BTL_REACT_NONE       1    /* nothing happens; also the value BtlHit_BeginFrame writes every frame */
#define BTL_REACT_NO_FLINCH  2    /* the hit lands and hurts but the defender keeps its action */
#define BTL_REACT_GRAB       9    /* BtlHit_CheckGrab */
#define BTL_REACT_GRAB_B     0xE  /* BtlHit_CheckGrab, defender animation flag 0x800 */

/* The pose block, fighter + 0x10 (BtlChar_GetPos). */
typedef BtlFighterPose HitPose;

/* The fighter object's work buffer (BtlObj + 0x1660). */
typedef struct HitObjWork {
    /* 0x00000 */ u8 unk0[0x18020];
    /* 0x18020 */ u32 hitFlags; /* & 0xF000000: the model has active hit volumes this frame */
} HitObjWork;

/* The fighter's battle object (BtlChar_GetObj). */
typedef struct HitObj {
    /* 0x0000 */ u8 unk0[0xCAC];
    /* 0x0CAC */ s8 hitCount;    /* when hitNo == ~hitCount the animation's last hit has been reached (inferred) */
    /* 0x0CAD */ s8 hitNo;     /* number of the animation's current hit, negative when none */
    /* 0x0CAE */ u8 unkCAE[0x1660 - 0xCAE];
    /* 0x1660 */ HitObjWork *work;
} HitObj;

/* First words of a member entry (BtlMember_GetActive). */
typedef struct HitMember {
    /* 0x00 */ s32 chara;
    /* 0x04 */ s32 costume;
} HitMember;

/* Gauge block of the active member (BtlMember_GetActiveGauge). */
typedef struct HitGauge {
    /* 0x00 */ s32 health;
    /* 0x04 */ s32 healthMax;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 ki;
    /* 0x10 */ s32 kiMax;
    /* 0x14 */ s32 blast;
    /* 0x18 */ s32 blastMax;
    /* 0x1C */ s32 maxPower;
    /* 0x20 */ s32 variant;
} HitGauge;

/* fighter + 0xF30 */
typedef BtlFighterHitStatus HitStatus;

/* fighter + 0xFB0: what the last hit does to this fighter. Read by the action code (not decompiled). */
typedef BtlFighterReact HitReact;

/* Partial view of a fighter (0x1600 bytes). */
typedef BtlFighter HitChr;

s32 BtlHit_IsFromBehind(HitChr *atk, HitChr *def);
s32 BtlHit_GetVoiceKind(s32 reaction);
s32 BtlHit_GetHitNo(HitChr *chr);
void BtlHit_ApplyGuard(HitChr *atk, HitChr *def, u32 result);
s32 BtlHit_CheckNear(HitChr *a, HitChr *b);
s32 BtlHit_CheckGrab(HitChr *a, HitChr *b);
s32 BtlHit_Stub(void);
s32 BtlHit_CheckTouch(HitChr *a, HitChr *b);
s32 BtlHit_IsVoid(HitChr *atk, HitChr *def);
s32 BtlHit_CheckDodge(HitChr *atk, HitChr *def);
s32 BtlHit_CheckArmor(HitChr *atk, HitChr *def);
s32 BtlHit_CheckRepel(HitChr *atk, HitChr *def);
s32 BtlHit_CheckGuard(HitChr *atk, HitChr *def, s32 *full);
s32 BtlHit_CheckThrowBreak(HitChr *atk, HitChr *def);
void BtlHit_BeginFrame(HitChr *chr);
s32 BtlHit_CheckClash(void);
s32 BtlHit_CheckRush(void);
void BtlHit_CheckProximityAll(void);
void BtlHit_TestHit(HitChr *chr);
void BtlHit_ResolveTrades(void);
void BtlHit_ApplyHit(HitChr *atk);

/* 0x1CA520..0x1CA6D0, the tail of the same object (include/battle/btl_char_coll.h declares them with its own view). */
void BtlColl_Update(void);
void BtlColl_UpdateFrozen(HitChr *chr);
s32 BtlColl_GetGuardKind(HitChr *chr);
s32 BtlColl_IsGuarding(HitChr *chr);

#endif

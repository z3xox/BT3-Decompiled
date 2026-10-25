#ifndef BATTLE_BTL_CHAR_COLL_H
#define BATTLE_BTL_CHAR_COLL_H

#include "types.h"
#include "sys/math3d.h"
#include "battle/btl_fighter.h"

/*
 * Fighter side of hit resolution (0x1CA520..0x1CA6D0, the tail of src/battle/btl_char_hit.c, and
 * src/battle/btl_hit_reaction.c 0x1CA6D0..0x1CDCA8): what a fighter does when the hit detection of the effect scene (0x1AF740..0x1B10F0)
 * reports that a hit record overlaps it. The structures are partial views local to this module: only the fields
 * these files touch. Names are guesses unless the comment says how they are known.
 */

/* Kind of hit record (BtlCollHit.type). */
#define BTL_HIT_BLAST 0 /* a projectile / energy attack; its parameters are BtlCollHit.atk */
#define BTL_HIT_TECH  1 /* a technique of a fighter; BtlCollHit.owner says whose and which slot */

/* Reaction ids stored in BtlCollReact.id that these files write or compare. */
#define BTL_REACT_NOFLINCH 2    /* the hit is taken without a reaction (damage still applies) */
#define BTL_REACT_CAUGHT   0x22 /* victim of a throw (BtlColl_StartThrow) */

/* Who launched a BTL_HIT_TECH record (BtlCollHit.owner). */
typedef struct BtlCollOwner {
    /* 0x0 */ s32 side; /* roster index of the attacker */
    /* 0x4 */ s32 slot; /* technique slot: 0, 1 ("strike": readers at 0x212080..0x212760) or 2..4 ("rush": readers
                           at 0x210D48..0x211AF0); the same number BtlAct_GetCurrentClass returns */
} BtlCollOwner;

/* Parameters of a blast (BtlCollHit.atk). */
typedef struct BtlCollAtk {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ s16 side;   /* roster index of the fighter that fired it */
    /* 0x12 */ u8 unk12[0x18 - 0x12];
    /* 0x18 */ s16 kind;   /* 1..3 raise event 0x36 on a hit; 0, 4, 8 need flag 0x50 to be deflected, others 0x51 */
    /* 0x1A */ u8 unk1A[0x41 - 0x1A];
    /* 0x41 */ u8 hits;    /* number of hits the damage is spread over (divisor when >= 2) */
} BtlCollAtk;

/* One hit record of the effect scene's list (the 0x190-byte "blast record" of btl_scene.h, 64 of them, emptied
   every frame). The hit detection (0x1B0030) passes the record and the target (owner ^ 1) to this module. */
typedef struct BtlCollHit {
    /* 0x00 */ s32 ownerId;    /* roster index / object id of the attacker (BtlChar_Get index) */
    /* 0x04 */ s32 unk4;
    /* 0x08 */ s32 level;       /* compared with BtlSuper_GetClashPower(chr, class) in BtlColl_TryDodge */
    /* 0x0C */ s32 type;       /* BTL_HIT_* */
    /* 0x10 */ u8 unk10[0x10];
    /* 0x20 */ Vec4 pos;       /* where it hit: sounds, camera shake and rumble are placed here */
    /* 0x30 */ u8 unk30[0x10];
    /* 0x40 */ Vec4 vel;       /* travel direction of the hit */
    /* 0x50 */ s32 flags;      /* bit 0x10: second variant of a rush hit (other reaction table, sound and a push) */
    /* 0x54 */ s32 unk54;      /* non-zero: direction taken from the hit position instead of vel; 1: cannot be
                                  deflected, reflected or absorbed */
    /* 0x58 */ u8 unk58[0xC];
    /* 0x64 */ BtlCollOwner *owner;
    /* 0x68 */ BtlCollAtk *atk;
} BtlCollHit;

/* Active member's gauge block (BtlMember_GetActiveGauge). */
typedef struct BtlCollGauge {
    /* 0x00 */ s32 health;
    /* 0x04 */ u8 unk4[0x20 - 0x4];
    /* 0x20 */ s32 variant;
} BtlCollGauge;

typedef BtlFighterPose BtlCollPose;

/* Result of the stage query under the fighter, inside the object's work buffer. */
typedef struct BtlCollGround {
    /* 0x00 */ BtlGroundVec pos;
    /* 0x10 */ BtlGroundVec nrm;
    /* 0x20 */ s32 flags;
} BtlCollGround;

/* Work buffer of a fighter's battle object (BtlObj + 0x1660, 0x1A0C0 bytes). */
typedef struct BtlCollObjWork {
    /* 0x00000 */ u8 unk0[0x18020];
    /* 0x18020 */ s32 hitFlags;    /* stage contact bits: 0x20, 0x40, 0x80, 0x100, 0x200 */
    /* 0x18024 */ u8 unk18024[0xC];
    /* 0x18030 */ BtlCollGround ground;
    /* 0x18058 */ u8 unk18058[0x8];
    /* 0x18060 */ s32 contactFlags;    /* bits 0xC0 */
} BtlCollObjWork;

/* Battle object of a fighter. */
typedef struct BtlCollObj {
    /* 0x0000 */ u8 unk0[0xCAC];
    /* 0x0CAC */ s8 hitCount;
    /* 0x0CAD */ s8 hitNo;
    /* 0x0CAE */ u8 unkCAE[0x1660 - 0xCAE];
    /* 0x1660 */ BtlCollObjWork *work;
} BtlCollObj;

/* Throw / catch description shared by the two fighters of a throw (fighter + 0xE90). The attacker fills its own
   copy and the fields are then copied into the victim's. */
typedef BtlFighterThrow BtlCollThrow;

/* Pending hit reaction of a fighter (fighter + 0xFB0). */
typedef BtlFighterReact BtlCollReact;

/* Fighter (0x1600 bytes). */
typedef BtlFighter BtlCollChr;

void BtlColl_Update(void);
void BtlColl_UpdateFrozen(BtlCollChr *chr);
s32 BtlColl_GetGuardKind(BtlCollChr *chr);
s32 BtlColl_IsGuarding(BtlCollChr *chr);
s32 BtlColl_StartThrow(BtlCollChr *atk, BtlCollChr *def, s32 slot, u32 techFlags);
s32 BtlColl_IsFacing(BtlCollChr *chr, f32 yaw);
f32 BtlColl_GetHitDir(BtlCollChr *chr, BtlCollHit *hit, Vec4 *outDir);
void BtlColl_PlayHitSound(BtlCollChr *chr, BtlCollHit *hit, u32 kind);
s32 BtlColl_GuardBlast(BtlCollChr *chr, BtlCollHit *hit);
s32 BtlColl_GuardStrike(BtlCollChr *chr, BtlCollHit *hit);
s32 BtlColl_GuardRush(BtlCollChr *chr, BtlCollHit *hit);
s32 BtlColl_HitByBlast(BtlCollChr *chr, BtlCollHit *hit);
s32 BtlColl_HitByStrike(BtlCollChr *chr, BtlCollHit *hit);
void BtlColl_ApplyRushHit(BtlCollChr *chr, BtlCollChr *atk, BtlCollHit *hit, s32 react, f32 yaw, s32 front, Vec4 *dir);
void BtlColl_ApplyRushNoFlinch(BtlCollChr *chr, BtlCollChr *atk, BtlCollHit *hit, f32 yaw, s32 front);
s32 BtlColl_StartRushCatch(BtlCollChr *chr, BtlCollChr *atk, BtlCollHit *hit, s32 react, f32 yaw);
s32 BtlColl_HitByRush(BtlCollChr *chr, BtlCollHit *hit);
s32 BtlColl_TryDodge(s32 objId, BtlCollHit *hit);
s32 BtlColl_TryDeflect(s32 objId, BtlCollHit *hit);
s32 BtlColl_TryReflect(s32 objId, BtlCollHit *hit);
s32 BtlColl_TryAbsorb(s32 objId, BtlCollHit *hit);
s32 BtlColl_TryGuard(s32 objId, BtlCollHit *hit);
s32 BtlColl_Hit(s32 objId, BtlCollHit *hit);
void BtlColl_PlayContactSound(BtlCollHit *hit);
void BtlColl_UpdateGround(BtlCollChr *chr);
void BtlColl_ClearActionBits(BtlCollChr *chr);
void BtlColl_AddActionBit(BtlCollChr *chr, s32 action);
void BtlColl_SetFramesLeftOverride(BtlCollChr *chr, s32 frames);
void BtlColl_SetFramesToReach(BtlCollChr *chr, f32 scale);
void BtlColl_NextPoolMember(BtlCollChr *chr); /* 0x1CDCA8: first function of src/battle/btl_char_member.c */

#endif

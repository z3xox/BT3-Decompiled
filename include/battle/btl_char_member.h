#ifndef BATTLE_BTL_CHAR_MEMBER_H
#define BATTLE_BTL_CHAR_MEMBER_H

#include "types.h"
#include "sys/math3d.h"
#include "battle/btl_fighter.h"

/*
 * Team members, parameters and gauges of a fighter (src/battle/btl_char_member.c, 0x1CDCA8..0x1D00D8).
 * The structures are this module's own partial views of shared objects (roster, battle setup); the fighter and
 * its member entries are BtlFighter and BtlMember (battle/btl_fighter.h).
 */

/* Units. */
#define BTL_HEALTH_BAR      10000  /* one health bar */
#define BTL_KI_MAX          100000 /* the ki gauge of every character */
#define BTL_BLAST_STOCK     100000 /* one blast stock */
#define BTL_MAXPOWER_MAX    30000  /* the +0x1C gauge */

/* BtlMember_Damage flags (bits verified, meanings as far as this module shows). */
#define BTL_DMG_NO_DEFENSE   0x1       /* skip both defense multipliers */
#define BTL_DMG_EXACT        0x2       /* do not round up to a multiple of 10 */
#define BTL_DMG_DRAIN        0x4       /* drain: abilities 0x3F / 0x3E apply, cannot kill; stored in combo.drain */
#define BTL_DMG_NO_HIT       0x8       /* does not count as a combo hit */
#define BTL_DMG_NO_TOTAL     0x10      /* (with 0x8) not added to the combo total */
#define BTL_DMG_NO_TIMER     0x20      /* (with 0x8) does not restart the combo display timer */
#define BTL_DMG_ANY_STATE    0x40      /* applies even when fighter flag 3 (fight state) is not set */
#define BTL_DMG_NO_HALVE     0x80      /* skip the "state bit 0x800 halves damage" rule */
#define BTL_DMG_NO_SCALING   0x100     /* skip the combo scaling */
#define BTL_DMG_FORCE        0x200     /* run the bookkeeping even for 0 damage */
#define BTL_DMG_NO_KILL      0x400     /* leaves at least 1 health */
#define BTL_DMG_KO_EV53_A    0x200000  /* on a KO raise event 0x53 for the opponent */
#define BTL_DMG_KO_EV53_B    0x400000
#define BTL_DMG_KO_EV52_A    0x800000  /* on a KO raise event 0x52 */
#define BTL_DMG_KO_EV52_B    0x1000000
#define BTL_DMG_KO_EV51      0x2000000 /* on a KO raise event 0x51 */
#define BTL_DMG_GUARD_MASK   0x3800000 /* with fighter state 0x4C the second defense multiplier is skipped */

/* Character parameters: gBtlChars->paramTbl[chara] (roster + 0x30, a table inside common file 2). */
typedef struct BtlMemberParam {
    /* 0x0 */ s32 healthMax;   /* 10000 per bar */
    /* 0x4 */ s32 kiStart;     /* ki at the start of a battle */
    /* 0x8 */ s32 unk8;
    /* 0xC */ s32 blastStocks; /* number of blast stocks */
} BtlMemberParam; /* size 0x10 */

/* Roster view. */
typedef struct BtlMemberRoster {
    /* 0x00 */ u8 unk0[0x30];
    /* 0x30 */ BtlMemberParam *paramTbl;
} BtlMemberRoster;

/* BattleMember (battle/battle.h) as this module reads it. */
typedef struct BtlMemberSetup {
    /* 0x00 */ s32 chara;
    /* 0x04 */ s32 costume;
    /* 0x08 */ s32 variant;
    /* 0x0C */ s32 cpuLevel;
    /* 0x10 */ f32 health;     /* percent of the maximum */
    /* 0x14 */ u8 unk14[0x24 - 0x14];
    /* 0x24 */ s32 bonus[8];
    /* 0x44 */ s32 aiType;
    /* 0x48 */ s32 ability[4];
} BtlMemberSetup;

/* The fighter's battle object, as BtlChar_SpawnFxBits3C / BtlChar_SpawnHitFx read it. */
typedef struct BtlMemberObj {
    /* 0x000 */ u8 unk0[0xA24];
    /* 0xA24 */ s32 area;
    /* 0xA28 */ u8 unkA28[0xC9C - 0xA28];
    /* 0xC9C */ u32 nodeMask;
} BtlMemberObj;

/* Request built by BtlChar_SpawnFxBits3C for EftShot_Request. */
typedef struct BtlMemberAuraReq {
    /* 0x00 */ s32 objId;
    /* 0x04 */ s32 kind;       /* 0..4 */
    /* 0x08 */ s32 area;       /* object + 0xA24 */
    /* 0x0C */ f32 unkC;
    /* 0x10 */ f32 time;
    /* 0x14 */ f32 speed;
    /* 0x18 */ f32 turnRate;
} BtlMemberAuraReq;

/* Request built by BtlChar_SpawnFxBits0 for EftTransform_Start. */
typedef struct BtlMemberFx0Req {
    /* 0x00 */ s32 objId;
    /* 0x04 */ s32 kind;       /* 0..10 */
} BtlMemberFx0Req;

/* Request built by BtlChar_SpawnHitFx for EftImpact_SpawnHit. */
typedef struct BtlMemberHitFxReq {
    /* 0x00 */ Vec4 pos;       /* world position, w = 1 */
    /* 0x10 */ Vec4 dir;       /* unit direction, w = 0 */
    /* 0x20 */ s32 kind;       /* 0..13 */
    /* 0x24 */ s32 objId;
    /* 0x28 */ s32 unk28;      /* 0 */
} BtlMemberHitFxReq;

/* Register use: a0 chr, a1 member, a2 init, a3 variant, f12 healthPct. */
void BtlMember_LoadParams(BtlFighter *chr, s32 member, s32 init, f32 healthPct, s32 variant);
BtlMember *BtlMember_Get(BtlFighter *chr, s32 member);
BtlMember *BtlMember_GetActive(BtlFighter *chr);
BtlMember *BtlMember_FindPresent(BtlFighter *chr, s32 chara);
s32 BtlMember_FindPresentIndex(BtlFighter *chr, s32 chara);
BtlMemberGauge *BtlMember_GetGauge(BtlFighter *chr, s32 member);
BtlMemberGauge *BtlMember_GetActiveGauge(BtlFighter *chr);
s32 BtlMember_Replace(BtlFighter *chr, BtlMemberSetup *src);
s32 BtlMember_FindIndex(BtlFighter *chr, s32 chara);
void BtlMember_SetActiveIndex(BtlFighter *chr, s32 member);
s32 BtlMember_CountAlive(BtlFighter *chr);
s32 BtlMember_GetActiveIndex(BtlFighter *chr);
s32 BtlMember_GetSwitchTarget(BtlFighter *chr);
s32 BtlMember_SetSwitchTarget(BtlFighter *chr, s32 member);
void BtlMember_NextSwitchTarget(BtlFighter *chr);
void BtlMember_PrevSwitchTarget(BtlFighter *chr);
s32 BtlMember_Damage(BtlFighter *chr, s32 amount, s32 flags);
void BtlMember_AddHealth(BtlFighter *chr, s32 amount);
s32 BtlMember_HasHealth(BtlFighter *chr, s32 amount);
s32 BtlMember_AddKi(BtlFighter *chr, s32 amount);
s32 BtlMember_HasKi(BtlFighter *chr, s32 amount);
s32 BtlMember_SpendKi(BtlFighter *chr, s32 amount, s32 force);
s32 BtlMember_DrainKi(BtlFighter *chr, s32 amount);
void BtlMember_AddBlast(BtlFighter *chr, s32 amount);
s32 BtlMember_HasBlast(BtlFighter *chr, s32 amount);
void BtlMember_SubBlast(BtlFighter *chr, s32 amount);
void BtlMember_AddMaxPower(BtlFighter *chr, s32 amount);
void BtlMember_SubMaxPower(BtlFighter *chr, s32 amount);
void BtlMember_SetMaxPower(BtlFighter *chr, s32 value);
f32 BtlMember_GetHealthRatio(BtlFighter *chr);
f32 BtlMember_GetTeamHealthRatio(BtlFighter *chr);
f32 BtlMember_GetKiRatio(BtlFighter *chr);
s32 BtlMember_IsHealthFull(BtlFighter *chr);
s32 BtlMember_IsKiFull(BtlFighter *chr);
s32 BtlMember_IsMaxPowerFull(BtlFighter *chr);
s32 BtlMember_IsHealthEmpty(BtlFighter *chr);
s32 BtlMember_IsKiEmpty(BtlFighter *chr);
s32 BtlMember_IsMaxPowerEmpty(BtlFighter *chr);
s32 BtlMember_HasAbility(BtlFighter *chr, s32 ability);
s32 BtlMember_HasAbilityOf(BtlFighter *chr, s32 member, s32 ability);
s32 BtlMember_HasAnyListedAbility(BtlFighter *chr);
void BtlMembers_UpdateQueuedDamage(void);
void BtlChar_SetFxBit(BtlFighter *chr, s32 bit);
void BtlChar_ClearFxBit(BtlFighter *chr, s32 bit);
s32 BtlChar_TestFxBit(BtlFighter *chr, s32 bit);
s32 BtlChar_TestPrevFxBit(BtlFighter *chr, s32 bit);
s32 BtlChar_IsFxBitNew(BtlFighter *chr, s32 bit);
s32 BtlChar_IsFxBitEnded(BtlFighter *chr, s32 bit);
s32 BtlChar_GetHitSoundLine(s32 kind);
void BtlChar_SpawnFxBits3C(BtlFighter *chr);
void BtlChar_SpawnFxBits0(BtlFighter *chr);
void BtlChar_SpawnHitFx(BtlFighter *chr);

#endif

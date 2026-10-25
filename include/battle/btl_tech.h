#ifndef BATTLE_BTL_TECH_B_H
#define BATTLE_BTL_TECH_B_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Character data readers, src/battle/btl_tech.c (0x20F0E8..0x2129C8): the second half of the accessor module
 * that btl_param.c starts. Nothing here keeps state; every function reads the character's data through the
 * fighter's battle object and applies the character's abilities and stat levels.
 *
 * A fighter's battle object carries pointers into the character's parameter file:
 *   obj + 0x91C  general parameters                      BtlParam in btl_param.h; its tail is BtlTechBParam here
 *   obj + 0x920  attack records (melee)                  btl_param.c
 *   obj + 0x924  13 ki blast records of 0x34 bytes       BtlKiBlastData
 *   obj + 0x928  movement speeds, turn rates, ki costs   BtlMoveData
 *   obj + 0x92C  the techniques of slots 2..4            BtlSuperData
 *   obj + 0x930  the skills of slots 0..1                BtlSkillData
 * The last two are structures of arrays: every field is an array with one element per slot.
 *
 * Technique slot (the "class" BtlAct_GetIdClass gives an action, the number BtlCollOwner.slot carries):
 *   0, 1  skills      ("Blast 1"): cost blast stocks; damage and knock-back are not scaled by anything
 *   2, 3  techniques  ("Blast 2"): cost ki; damage scaled by stat curve row 9; +20 % in the powered-up state
 *   4     technique   ("Ultimate Blast"): costs ki; damage scaled by stat curve row 10; +20 % with ability 0x6F
 * The game terms are a reading of the code (blast stocks for 0..1, ki for 2..4, slot 4 the one the powered-up
 * state does not boost); the layout and the arithmetic are what the matching C verifies.
 *
 * Units: speeds in the file are in 10 km/h (x * 10 * 1000 / 3600 / 30 gives units per frame), turn rates in
 * degrees per second, times in seconds (x * 30 gives frames), damage in health points (10000 per bar), ki in
 * gauge units (100000 = a full ki gauge), blast cost in stocks (100000 gauge units each).
 */

/* Technique slots. */
#define BTL_SLOT_SKILL_A  0
#define BTL_SLOT_SKILL_B  1
#define BTL_SLOT_TECH_A   2
#define BTL_SLOT_TECH_B   3
#define BTL_SLOT_ULTIMATE 4

/* Ki blast kinds: index of the 13 records (BtlKiBlast_GetCurrentKind). Kinds 0, 4 and 8 are the uncharged shots;
   the three kinds after each are its charged shot at charge < 0.3, < 0.9 and above. */
#define BTL_KIBLAST_COUNT    13
#define BTL_KIBLAST_A        0  /* animations 0x71, 0x72 */
#define BTL_KIBLAST_A_CHARGE 1  /* 1..3: animations 0x7B, 0x7C (always level 1), 0x7D (by charge) */
#define BTL_KIBLAST_B        4  /* animations 0x77..0x79 */
#define BTL_KIBLAST_B_CHARGE 5  /* 5..7: animations 0x84, 0x85, 0x87, 0x88, 0x8A, 0x8B (level 1), 0x86, 0x89, 0x8C */
#define BTL_KIBLAST_C        8  /* animation 0x7A */
#define BTL_KIBLAST_C_CHARGE 9  /* 9..11: animations 0x8D, 0x8E (level 1), 0x8F */
#define BTL_KIBLAST_ACT95    12 /* action 0x95 */

/* obj + 0x91C: general parameters of the character. The first 0x5C bytes are BtlParam in btl_param.h. */
typedef struct BtlTechBParam {
    /* 0x00 */ u8 unk0[0x5C];
    /* 0x5C */ f32 unk5C;
    /* 0x60 */ f32 hitReactScale; /* multiplies the reaction scale of a hit taken (BtlHit_ApplyHit) */
    /* 0x64 */ f32 gauge99CTime;  /* seconds the fighter +0x99C gauge takes to fill */
    /* 0x68 */ f32 stepCancelRatio;
    /* 0x6C */ f32 backStepCancelRatio;
    /* 0x70 */ s16 poweredDashLimit;         /* copied to fighter +0xD6C every frame */
    /* 0x72 */ s16 poweredVanishLimit;         /* copied to fighter +0xD74 every frame */
    /* 0x74 */ f32 chargeRateA;   /* attack charge rate at gauge +0xD80 == 0 (BtlAct_GetChargeRate) */
    /* 0x78 */ f32 chargeRateB;   /* attack charge rate at gauge +0xD80 == 100000 */
    /* 0x7C */ f32 damageTaken;   /* multiplier of the damage the character takes (BtlMember_Damage) */
    /* 0x80 */ u8 unk80[0x8F - 0x80];
    /* 0x8F */ u8 comboFinish[0xC7 - 0x8F]; /* BtlParam_GetComboFinish indexes it; from 0x98 on BtlParam has other fields */
    /* 0xC7 */ u8 unkC7;          /* percent; BtlParam_GetUnkC7Scale */
    /* 0xC8 */ f32 unkC8;
} BtlTechBParam;

/* obj + 0x924: one ki blast record. */
typedef struct BtlKiBlastData {
    /* 0x00 */ u16 flags;       /* 1: cannot be guarded (except by guard kind 6); 2: cannot be deflected, reflected
                                   or absorbed; 0x10: no flinch against a tougher victim or one whose state has bit
                                   0x100; 0x20: copied into the spawn request. Bits 1 and 2 are dropped while the
                                   owner's BtlChar_IsBodyChanged is set. (Uses are in btl_hit_reaction.c.) */
    /* 0x02 */ s8 type;         /* spark module, sound class, deflect rules */
    /* 0x03 */ s8 unk3;
    /* 0x04 */ s32 damage;      /* total over all hits */
    /* 0x08 */ s32 guardDamage; /* total over all hits when guarded */
    /* 0x0C */ s32 ki;          /* ki the shot costs */
    /* 0x10 */ f32 speed;       /* 10 km/h */
    /* 0x14 */ f32 turn;        /* degrees per second: homing turn rate */
    /* 0x18 */ f32 time;        /* seconds */
    /* 0x1C */ f32 push;        /* knock-back speed, 10 km/h, total over all hits */
    /* 0x20 */ f32 guardPush;
    /* 0x24 */ s8 react;        /* reaction id */
    /* 0x25 */ s8 reactS800;    /* victim state bit 0x800; 0: react */
    /* 0x26 */ s8 reactS4000;   /* victim state bit 0x4000 or action 0xDF; 0: react */
    /* 0x27 */ s8 reactS200;    /* victim state bit 0x200; 0: react */
    /* 0x28 */ f32 radius;
    /* 0x2C */ f32 unk2C;
    /* 0x30 */ s8 hits;         /* number of hits the damage and the knock-back are spread over */
    /* 0x31 */ s8 spreadMode;
    /* 0x32 */ u8 unk32[2];
} BtlKiBlastData; /* size 0x34 */

/* obj + 0x928: movement parameters. */
typedef struct BtlMoveData {
    /* 0x00 */ f32 speed[20];    /* 10 km/h; BtlMoveParam_GetSpeed */
    /* 0x50 */ f32 accel[2];     /* 10 km/h per second */
    /* 0x58 */ f32 angle58;      /* degrees */
    /* 0x5C */ f32 height[2];    /* times 10 */
    /* 0x64 */ s32 kiCost[4];    /* ki per second */
    /* 0x74 */ f32 turnRate[3];  /* degrees per second */
    /* 0x80 */ f32 turnAccel[4]; /* degrees per second (per frame after the conversion) */
    /* 0x90 */ f32 turnMax[4];   /* degrees */
} BtlMoveData;

/* obj + 0x92C: the techniques of slots 2..4; every array is indexed by slot - 2. */
typedef struct BtlSuperData {
    /* 0x000 */ u32 flagsA[3];        /* BtlSuper_GetFlagsA */
    /* 0x00C */ u32 flags[3];         /* BtlSuper_GetFlags */
    /* 0x018 */ s16 id[3];            /* technique id; 0x268, 0x27F, 0x2CD, 0x2D5 are special-cased here */
    /* 0x01E */ s16 clashPower[3];
    /* 0x024 */ f32 shotTime[3];      /* seconds */
    /* 0x030 */ u8 unk30[0x54 - 0x30];
    /* 0x054 */ f32 shotSpeed[3];     /* 10 km/h */
    /* 0x060 */ f32 shotTurn[3];      /* degrees per second */
    /* 0x06C */ f32 shotScale[3];
    /* 0x078 */ u8 unk78[0x90 - 0x78];
    /* 0x090 */ s8 unk90[3];
    /* 0x093 */ s8 shots[3];          /* hit factor 1: number of shots; < 1 counts as 1 */
    /* 0x096 */ s8 hitsB[3];          /* hit factor 2; < 1 counts as 1 */
    /* 0x099 */ s8 hitShape[3];
    /* 0x09C */ s8 hitsC[3];          /* hit factor 3, less 1 */
    /* 0x09F */ u8 unk9F[0x13B - 0x9F];
    /* 0x13B */ s8 hitDirKind[3];
    /* 0x13E */ s8 type[3];           /* BtlSuper_GetType */
    /* 0x141 */ s8 react[3];          /* reaction id */
    /* 0x144 */ s8 reactAlt[3];       /* hit flag 0x10; <= 0: react */
    /* 0x147 */ s8 reactS800[3];      /* victim state bit 0x800 */
    /* 0x14A */ s8 reactAltS800[3];   /* both; <= 0: reactS800 */
    /* 0x14D */ s8 landingKind[3];
    /* 0x150 */ s8 guardKind[3];
    /* 0x153 */ s8 hitSound[2][3];    /* [hit flag 0x10][slot - 2] */
    /* 0x159 */ s8 steps[3];          /* steps of a rush; < 1 counts as 1 */
    /* 0x15C */ u8 unk15C[0x165 - 0x15C];
    /* 0x165 */ s8 aiKind[3];
    /* 0x168 */ s8 lastStep[3];       /* < 0: steps - 1 */
    /* 0x16B */ u8 unk16B;
    /* 0x16C */ f32 push[3];          /* knock-back speed, 10 km/h */
    /* 0x178 */ f32 guardPush[3];
    /* 0x184 */ s32 damage[3];        /* total over all hits */
    /* 0x190 */ s32 guardDamage[3];   /* total over all hits when guarded */
    /* 0x19C */ s32 ki[3];            /* ki the technique costs */
    /* 0x1A8 */ s32 recoil[3];        /* damage to the user */
    /* 0x1B4 */ s32 drain[3];
    /* 0x1C0 */ f32 cooldown[3];       /* seconds */
    /* 0x1CC */ f32 chargeLimit[3];       /* seconds */
    /* 0x1D8 */ f32 chargeScale[3];   /* damage multiplier at full charge (fighter +0xE44 == 1) */
    /* 0x1E4 */ f32 throwAngleA[3];   /* degrees */
    /* 0x1F0 */ f32 throwAngleB[3];   /* degrees */
    /* 0x1FC */ f32 rushSpeed[3];     /* 10 km/h */
    /* 0x208 */ s16 throwChara[3];    /* < 0x100: a character the throw brings in */
    /* 0x20E */ s16 throwCharaAlt[3];
    /* 0x214 */ u8 unk214[0x220 - 0x214];
    /* 0x220 */ s8 throwPartnerStep[3];
    /* 0x223 */ s8 stageFxEndStep[3];         /* < 0: 999 */
    /* 0x226 */ s8 throwObjectSlot[3];
    /* 0x229 */ s8 promptRow[3];      /* row of the roster's prompt table (+0x34) */
    /* 0x22C */ f32 time22C[3];       /* seconds */
    /* 0x238 */ f32 time238[3];       /* seconds */
} BtlSuperData;

/* obj + 0x930: the skills of slots 0..1; every array is indexed by slot. */
typedef struct BtlSkillData {
    /* 0x00 */ u32 flagsA[2];
    /* 0x08 */ u32 flags[2];       /* 0x100: stat levels are added to the current ones; 0x200: enters the
                                      powered-up state; others in btl_act_i.c / btl_act_decide.c */
    /* 0x10 */ s16 id[2];          /* skill id */
    /* 0x14 */ s16 clashPower[2];
    /* 0x18 */ f32 shotTime[2];
    /* 0x20 */ f32 shotSpeed[2];   /* 10 km/h */
    /* 0x28 */ f32 shotTurn[2];    /* degrees per second */
    /* 0x30 */ f32 shotScale[2];
    /* 0x38 */ s8 unk38[2];
    /* 0x3A */ s8 shots[2];        /* hit factor 1; < 1 counts as 1 */
    /* 0x3C */ s8 hitsB[2];        /* hit factor 2; < 1 counts as 1 */
    /* 0x3E */ s8 hitShape[2];
    /* 0x40 */ s8 hitsC[2];        /* hit factor 3, less 1 */
    /* 0x42 */ u8 unk42[0x62 - 0x42];
    /* 0x62 */ s8 hitDirKind[2];
    /* 0x64 */ s8 sequence[2];     /* which action block runs the skill */
    /* 0x66 */ s8 statKind[2];     /* BTL_STAT_KIND_*: how long the stat levels last */
    /* 0x68 */ s8 react[2];        /* reaction id */
    /* 0x6A */ s8 reactS800[2];    /* victim state bit 0x800; <= 0: react */
    /* 0x6C */ s8 hitSound[2];
    /* 0x6E */ u8 unk6E[0x74 - 0x6E];
    /* 0x74 */ f32 push[2];        /* knock-back, total over all hits (used as stored) */
    /* 0x7C */ f32 guardPush[2];
    /* 0x84 */ s32 damage[2];      /* total over all hits */
    /* 0x8C */ s32 guardDamage[2];
    /* 0x94 */ s8 stocks[2];       /* blast stocks the skill costs */
    /* 0x96 */ s8 statLevel0[2];   /* level added to stat 0 */
    /* 0x98 */ s8 statLevel2[2];   /* level added to stat 2 */
    /* 0x9A */ s8 statLevel1[2];   /* level added to stat 1 */
    /* 0x9C */ s8 statLevel3[2];   /* level added to stat 3 */
    /* 0x9E */ s8 aiKind[2];
    /* 0xA0 */ f32 time[2];        /* seconds: duration of the effect / of the stun */
    /* 0xA8 */ s32 healthPct[2];   /* percent of the maximum health given (negative: taken) */
    /* 0xB0 */ s32 ki[2];          /* ki given (negative: taken) */
    /* 0xB8 */ s32 valF[2];
    /* 0xC0 */ s32 valE[2];
} BtlSkillData;

/* The fighter's battle object. */
typedef struct BtlTechBObj {
    /* 0x000 */ u8 unk0[0xC];
    /* 0x00C */ s32 chara;       /* compared with BtlSuperData.throwChara */
    /* 0x010 */ u8 unk10[0x91C - 0x10];
    /* 0x91C */ BtlTechBParam *param;
    /* 0x920 */ void *atk;
    /* 0x924 */ BtlKiBlastData *kiBlast;
    /* 0x928 */ BtlMoveData *move;
    /* 0x92C */ BtlSuperData *super;
    /* 0x930 */ BtlSkillData *skill;
} BtlTechBObj;

/* The shot behind a hit record (BtlCollAtk in btl_char_coll.h). */
typedef struct BtlTechBShot {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ s16 side;         /* roster index of the fighter that fired it */
    /* 0x12 */ u8 unk12[0x18 - 0x12];
    /* 0x18 */ s16 kind;         /* ki blast kind */
} BtlTechBShot;

/* A hit record of the effect scene (BtlCollHit in btl_char_coll.h). */
typedef struct BtlTechBHit {
    /* 0x00 */ u8 unk0[0x68];
    /* 0x68 */ BtlTechBShot *shot;
} BtlTechBHit;

/* Where a technique's cutscene is staged: five entries per stage. */
typedef struct BtlTechPlace {
    /* 0x0 */ f32 x, y, z;
    /* 0xC */ f32 yaw;           /* degrees */
} BtlTechPlace; /* size 0x10 */

typedef struct BtlTechPlaceTbl {
    /* 0x0 */ BtlTechPlace rec[5]; /* indexed [stage * 5 + n]: the stages follow each other */
} BtlTechPlaceTbl;

/* Roster table +0x34 (BtlTechPromptRow in btl_param.h). */
typedef struct BtlTechBPromptRow {
    /* 0x00 */ s8 kind[2];
    /* 0x02 */ s8 dir;           /* the byte BtlInput_TestAction's "switch" test reads */
    /* 0x03 */ u8 unk3[0x14 - 3];
} BtlTechBPromptRow; /* size 0x14 */

typedef struct BtlTechBRoster {
    /* 0x00 */ u8 unk0[0x34];
    /* 0x34 */ BtlTechBPromptRow *promptRows;
} BtlTechBRoster;

/* ---- general parameters ---- */
f32 BtlParam_GetUnk5C(BtlFighter *chr);
f32 BtlParam_GetStepCancelRatio(BtlFighter *chr);
f32 BtlParam_GetBackStepCancelRatio(BtlFighter *chr);
f32 BtlParam_GetHitReactScale(BtlFighter *chr);
s32 BtlParam_GetPoweredDashLimit(BtlFighter *chr);
s32 BtlParam_GetPoweredVanishLimit(BtlFighter *chr);
f32 BtlParam_GetChargeRateA(BtlFighter *chr);
f32 BtlParam_GetChargeRateB(BtlFighter *chr);
f32 BtlParam_GetDamageTakenScale(BtlFighter *chr);
f32 BtlParam_GetGauge99CTime(BtlFighter *chr);
f32 BtlParam_GetUnkC7Scale(BtlFighter *chr);
f32 BtlParam_GetUnkC8(BtlFighter *chr);
s32 BtlParam_GetComboFinish(BtlFighter *chr, s32 n);
s32 BtlParam_GetDashSound(BtlFighter *chr);
s32 BtlParam_GetChargeStartSound(BtlFighter *chr);
s32 BtlParam_GetChargeLoopSound(BtlFighter *chr);
s32 BtlParam_GetMaxPowerSound(BtlFighter *chr);
s32 BtlParam_CanFly(BtlFighter *chr);

/* ---- ki blasts: X(chr) reads the blast being fired, XOfHit(hit) the one behind a hit record, XOf(chr, kind) a
   given kind ---- */
s32 BtlKiBlast_ScaleDamageByAbility(BtlFighter *chr, s32 damage, s32 kind);
f32 BtlKiBlast_ScaleSpeedByAbility(BtlFighter *chr, s32 kind, f32 speed);
s32 BtlKiBlast_ScaleKiByAbility(BtlFighter *chr, s32 ki, s32 kind);
s32 BtlKiBlast_MapReact(BtlFighter *chr, s32 react, s32 kind);
s32 BtlKiBlast_MapReactS800(BtlFighter *chr, s32 react, s32 kind);
s32 BtlKiBlast_GetCurrentKind(BtlFighter *chr);
BtlKiBlastData *BtlKiBlast_GetCurrent(BtlFighter *chr, s32 *outKind);
BtlKiBlastData *BtlKiBlast_GetOfHit(BtlTechBHit *hit, BtlFighter **outChr, s32 *outKind);
BtlKiBlastData *BtlKiBlast_Get(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetFlags(BtlFighter *chr);
s32 BtlKiBlast_GetFlagsOfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetFlagsOf(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetDamage(BtlFighter *chr);
s32 BtlKiBlast_GetDamageOfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetDamageOf(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetGuardDamage(BtlFighter *chr);
s32 BtlKiBlast_GetGuardDamageOfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetGuardDamageOf(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetKiCost(BtlFighter *chr);
s32 BtlKiBlast_GetKiCostOfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetKiCostOf(BtlFighter *chr, u32 kind);
f32 BtlKiBlast_GetSpeed(BtlFighter *chr);
f32 BtlKiBlast_GetSpeedOfHit(BtlTechBHit *hit);
f32 BtlKiBlast_GetSpeedOf(BtlFighter *chr, u32 kind);
f32 BtlKiBlast_GetTurnRate(BtlFighter *chr);
f32 BtlKiBlast_GetTurnRateOfHit(BtlTechBHit *hit);
f32 BtlKiBlast_GetTurnRateOf(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetFrames(BtlFighter *chr);
s32 BtlKiBlast_GetFramesOfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetFramesOf(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetSpreadMode(BtlFighter *chr);
s32 BtlKiBlast_GetSpreadModeOfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetSpreadModeOf(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetReact(BtlFighter *chr);
s32 BtlKiBlast_GetReactOfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetReactOf(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetReactS800(BtlFighter *chr);
s32 BtlKiBlast_GetReactS800OfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetReactS800Of(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetReactS4000(BtlFighter *chr);
s32 BtlKiBlast_GetReactS4000OfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetReactS4000Of(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetReactS200(BtlFighter *chr);
s32 BtlKiBlast_GetReactS200OfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetReactS200Of(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetType(BtlFighter *chr);
s32 BtlKiBlast_GetTypeOfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetTypeOf(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetUnk3(BtlFighter *chr);
s32 BtlKiBlast_GetUnk3OfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetUnk3Of(BtlFighter *chr, u32 kind);
s32 BtlKiBlast_GetHits(BtlFighter *chr);
s32 BtlKiBlast_GetHitsOfHit(BtlTechBHit *hit);
s32 BtlKiBlast_GetHitsOf(BtlFighter *chr, u32 kind);
f32 BtlKiBlast_GetRadius(BtlFighter *chr);
f32 BtlKiBlast_GetRadiusOfHit(BtlTechBHit *hit);
f32 BtlKiBlast_GetRadiusOf(BtlFighter *chr, u32 kind);
f32 BtlKiBlast_GetUnk2C(BtlFighter *chr);
f32 BtlKiBlast_GetUnk2COfHit(BtlTechBHit *hit);
f32 BtlKiBlast_GetUnk2COf(BtlFighter *chr, u32 kind);
f32 BtlKiBlast_GetPush(BtlFighter *chr);
f32 BtlKiBlast_GetPushOfHit(BtlTechBHit *hit);
f32 BtlKiBlast_GetPushOf(BtlFighter *chr, u32 kind);
f32 BtlKiBlast_GetGuardPush(BtlFighter *chr);
f32 BtlKiBlast_GetGuardPushOfHit(BtlTechBHit *hit);
f32 BtlKiBlast_GetGuardPushOf(BtlFighter *chr, u32 kind);

/* ---- movement ---- */
f32 BtlMoveParam_GetSpeed(BtlFighter *chr, s32 n);
f32 BtlMoveParam_GetAccel(BtlFighter *chr, s32 n);
f32 BtlMoveParam_GetAngle58(BtlFighter *chr);
f32 BtlMoveParam_GetHeight(BtlFighter *chr, s32 n);
s32 BtlMoveParam_GetKiCost(BtlFighter *chr, s32 n);
f32 BtlMoveParam_GetTurnRate(BtlFighter *chr, s32 n);
f32 BtlMoveParam_GetTurnAccel(BtlFighter *chr, s32 n);
f32 BtlMoveParam_GetTurnMax(BtlFighter *chr, s32 n);

/* ---- techniques, slot 2..4 ---- */
s32 BtlSuper_GetHitCount(BtlFighter *chr, s32 slot);
u32 BtlSuper_GetFlagsA(BtlFighter *chr, s32 slot);
u32 BtlSuper_GetFlags(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetId(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetClashPower(BtlFighter *chr, s32 slot);
f32 BtlSuper_GetShotTime(BtlFighter *chr, s32 slot);
f32 BtlSuper_GetShotSpeed(BtlFighter *chr, s32 slot);
f32 BtlSuper_GetShotTurnRate(BtlFighter *chr, s32 slot);
f32 BtlSuper_GetShotUnk6C(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetUnk90(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetShots(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetHitsB(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetHitsC(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetHitDirKind(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetType(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetReact(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetReactAlt(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetReactS800(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetReactAltS800(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetLandingKind(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetGuardKind(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetHitSound(BtlFighter *chr, s32 slot, s32 alt);
s32 BtlSuper_GetStepCount(BtlFighter *chr, s32 slot);
f32 BtlSuper_GetPush(BtlFighter *chr, s32 slot);
f32 BtlSuper_GetGuardPush(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetBaseDamage(BtlFighter *chr, s32 slot, s32 total);
s32 BtlSuper_GetBaseGuardDamage(BtlFighter *chr, s32 slot, s32 total);
s32 BtlSuper_GetKiCost(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetRecoilDamage(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetDrain(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetCooldownFrames(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetChargeLimitFrames(BtlFighter *chr, s32 slot);
f32 BtlSuper_GetChargeScale(BtlFighter *chr, s32 slot);
f32 BtlSuper_GetThrowAngleA(BtlFighter *chr, s32 slot);
f32 BtlSuper_GetThrowAngleB(BtlFighter *chr, s32 slot);
f32 BtlSuper_GetRushSpeed(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetThrowChara(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetThrowCostume(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetThrowGauge20(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetThrowPartnerStep(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetStageFxEndStep(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetThrowObjectSlot(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetPromptRowIndex(BtlFighter *chr, s32 slot);
BtlTechBPromptRow *BtlSuper_GetPromptRow(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetAiKind(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetLastStep(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetFrames22C(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetFrames238(BtlFighter *chr, s32 slot);
s32 BtlSuper_GetDamage(BtlFighter *chr, s32 slot, s32 guard, s32 total);
void BtlSuper_GetStagePlacement(BtlFighter *chr, s32 slot, s32 n, Vec4 *pos, Vec4 *rot);
s32 BtlSuper_IsThrow(BtlFighter *chr, s32 slot);

/* ---- skills, slot 0..1 ---- */
s32 BtlSkill_GetHitCount(BtlFighter *chr, s32 slot);
u32 BtlSkill_GetFlagsA(BtlFighter *chr, s32 slot);
u32 BtlSkill_GetFlags(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetId(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetUnk14(BtlFighter *chr, s32 slot);
f32 BtlSkill_GetShotTime(BtlFighter *chr, s32 slot);
f32 BtlSkill_GetShotSpeed(BtlFighter *chr, s32 slot);
f32 BtlSkill_GetShotTurnRate(BtlFighter *chr, s32 slot);
f32 BtlSkill_GetShotUnk30(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetUnk38(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetShots(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetHitsB(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetHitsC(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetHitDirKind(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetSequence(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetAiKind(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetStatKind(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetReact(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetReactS800(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetHitSound(BtlFighter *chr, s32 slot);
f32 BtlSkill_GetPush(BtlFighter *chr, s32 slot);
f32 BtlSkill_GetGuardPush(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetDamage(BtlFighter *chr, s32 slot, s32 total);
s32 BtlSkill_GetGuardDamage(BtlFighter *chr, s32 slot, s32 total);
s32 BtlSkill_GetBlastCost(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetStatLevel0(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetStatLevel2(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetStatLevel1(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetStatLevel3(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetFrames(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetHealthChange(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetKiChange(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetValF(BtlFighter *chr, s32 slot);
s32 BtlSkill_GetValE(BtlFighter *chr, s32 slot);

/* ---- battle-level wrappers (HUD, battle sequence) ---- */
void BtlGame_Reset(void);
void BtlGame_Init(void);
void BtlGame_Term(void);
void BtlGame_PreUpdate(void);
s32 BtlGame_Update(void);

#endif

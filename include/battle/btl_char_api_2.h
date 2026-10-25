#ifndef BATTLE_BTL_CHAR_API_H
#define BATTLE_BTL_CHAR_API_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Fighter interface, 0x207020..0x208430 (src/battle/btl_char_api_2.c): what the rest of the battle code (camera, effect
 * scene, effect modules, HUD, sound, AI) calls to read or poke a fighter without knowing its layout. Arguments named
 * `objId` are battle object ids (BtlObj_Get index; a fighter's own id is at fighter +0xC); every function is safe on
 * an id that is not a fighter and then returns 0 / 0.0f / does nothing.
 *
 * The structs below are this module's partial views (rule: no shared fighter header yet). Only the fields this file
 * touches, or that its direct callees show, are named.
 */

/* Active member's vitals, the block BtlMember_GetActiveGauge(chr) returns: chr + 0x9A4 + chr->member * 0xA4 + 0x40. */
typedef struct BtlCharApiVitals {
    /* 0x00 */ s32 hp;     /* < 1 = down (BtlChar_IsDead) */
    /* 0x04 */ s32 hpMax;
    /* 0x08 */ u8 unk8[0x20 - 0x8];
    /* 0x20 */ s32 variant;  /* member +0x60 */
    /* 0x24 */ u8 unk24[0x30 - 0x24];
    /* 0x30 */ s32 bodyChanged;  /* member +0x70; non-zero makes the win / lose talk use character 0x56 (btl_seq.c) */
} BtlCharApiVitals;

/* Fighter (0x1600 bytes, BtlChar_Get / BtlChar_FindByObjId). */
typedef BtlFighter BtlCharApiChr;

/* Battle object (BtlObj_Get): only the fields read here. */
typedef struct BtlCharApiObj {
    /* 0x000 */ u8 unk0[0xA40];
    /* 0xA40 */ union {
        u64 flags64;                /* the tests of two bits read all 64 bits */
        s32 flags;                  /* the single-bit accessors read and write the low word */
    } u;
    /* 0xA48 */ u8 unkA48[0xC78 - 0xA48];
    /* 0xC78 */ f32 animFrame;
    /* 0xC7C */ u8 unkC7C[0xC80 - 0xC7C];
    /* 0xC80 */ f32 animStep;
    /* 0xC84 */ u8 unkC84[0xCAC - 0xC84];
    /* 0xCAC */ s8 hitCount;
    /* 0xCAD */ s8 hitIndex;          /* expected to be ~hitCount */
    /* 0xCAE */ s8 eventCount;       /* eventCount in btl_obj_anim.h: number of animation events (BtlObjAnim_TestEvent) */
} BtlCharApiObj;

/* One playing sound of a side (0xC bytes). BtlCharSnd_StoreHandle fills a slot, BtlCharSnd_StopUnrequestedLoops stops it with Snd_StopHandle. */
typedef struct BtlCharApiSound {
    /* 0x0 */ s32 handle;           /* sound handle, < 0 = free slot */
    /* 0x4 */ s32 id;
    /* 0x8 */ u8 kind;              /* index into the table at 0x2EF290 (BtlCharSnd_GetBankMask) */
} BtlCharApiSound;

typedef struct BtlCharApiSoundSet {
    /* 0x00 */ BtlCharApiSound slot[4];
    /* 0x30 */ s32 next;            /* ring index of the next slot to try (BtlCharSnd_StoreHandle) */
} BtlCharApiSoundSet; /* size 0x34 */

/* Fighter manager (gBtlChars, 0x280 bytes). */
typedef struct BtlCharApiMgr {
    /* 0x00 */ s32 count;
    /* 0x04 */ BtlCharApiChr *chars;
    /* 0x08 */ void *oneShotSounds;            /* the one-shot sound sets; the looping sets follow */
    /* 0x0C */ BtlCharApiSoundSet *loopSounds; /* one per side */
    /* 0x10 */ u8 unk10[0x1C - 0x10];
    /* 0x1C */ s32 unk1C;               /* counter, full at 90 */
    /* 0x20 */ void **motionFlags;            /* 0x19E pointers, indexed by fighter +0x974 */
    /* 0x24 .. 0x280: not touched here. Seen in callees: +0xA0 four 0x20-byte sound requests {Vec4 pos, f32 near,
       f32 far, s32 id, s8 owner, s8 kind} with their count at +0x120 (BtlCharSnd_Request); +0x130 / +0x134 (BtlReplay_GetViewSide). */
} BtlCharApiMgr;

s32 BtlCharApi_ObjHasFlags22(s32 objId);
s32 BtlCharApi_ObjHasFlags42(s32 objId);
s32 BtlCharApi_AnyHasFlag128(void);
s32 BtlCharApi_IsHpEmpty(s32 objId);
s32 BtlCharApi_TestFlagA4(s32 objId);
s32 BtlCharApi_GetMemberVariant(s32 objId);
s32 BtlCharApi_IsFlag8Action104(s32 objId);
s32 BtlCharApi_IsAction103OrFlagA6(s32 objId);
s32 BtlCharApi_IsMemberBodyChanged(s32 objId);
void BtlCharApi_SetHeldFlagA7(s32 objId);
void BtlCharApi_SetHeldFlagA8(s32 objId);
void BtlCharApi_SetHeldFlagA9(s32 objId);
void BtlCharApi_SetHeldFlagAA(s32 objId);
void BtlCharApi_SetHeldFlagAB(s32 objId);
void BtlCharApi_PlaySoundAt(Vec4 *pos, s32 kind, s32 id, f32 near, f32 far);
s32 BtlCharApi_PlayTechniqueSound(s32 objId, u32 n); /* result undefined, see the source */
s32 BtlCharApi_GetSoundCount(s32 side);
void BtlCharApi_GetSound(s32 side, s32 n, s32 *handle, s32 *bankMask, s32 *id);
void BtlCharApi_ObjClearMaskBit3(s32 objId);
void BtlCharApi_ObjSetMaskBit3(s32 objId);
s32 BtlCharApi_ObjTestFlagBit21(s32 objId);
f32 BtlCharApi_GetTechCharge(s32 objId);
f32 BtlCharApi_GetSkillCount3Ratio(s32 objId);
f32 BtlCharApi_GetUnkE60Ratio(s32 objId);
void BtlCharApi_ObjSetFlag100(s32 objId, s32 on);
s32 BtlCharApi_ObjHasFlags102(s32 objId);
s32 BtlCharApi_IsTargetBelowHalfHp(s32 objId, s32 targetId);
s32 BtlCharApi_CanTechniqueFinish(s32 objId, s32 targetId);
f32 BtlCharApi_GetMgrTimerRatio(void);
void BtlCharApi_RumbleNear(Vec4 *pos, f32 near, f32 far, f32 power, f32 time);
s32 BtlCharApi_ObjTestAttr(s32 objId, u64 mask);
s32 BtlCharApi_ObjGetAttrValue(s32 objId, u64 mask);
s32 BtlCharApi_ObjGetAttrKind(s32 objId, u64 mask);
f32 BtlCharApi_GetRushSequenceFrame(s32 objId);
f32 BtlCharApi_ObjGetAnimFrame(s32 objId);
f32 BtlCharApi_ObjGetAnimStep(s32 objId);
s32 BtlCharApi_ObjQueryAnimEvent(s32 objId, s32 mask, s32 what);
s32 BtlCharApi_GetAnimId(s32 objId);
void *BtlCharApi_GetAnimFlags(s32 objId);
s32 BtlCharApi_TestFlag2B(s32 objId);
s32 BtlCharApi_GetCamPose(s32 objId, Vec4 *pos, Vec4 *rot);
f32 BtlCharApi_GetCamYaw(s32 objId);
s32 BtlCharApi_HasCamPriority(s32 objId);
void BtlCharApi_ShakeCamsNear(Vec4 *pos, f32 near, f32 far, f32 strength, f32 time);
s32 BtlCharApi_IsCamShown(s32 objId);
void BtlCharApi_GetCamBodyPos(s32 objId, Vec4 *out);
s32 BtlCharApi_GetReplayViewSide(void);
s32 BtlCharApi_AnyCamPriority(void);
s32 BtlCharApi_IsInputInjected(s32 objId);
void BtlCharApi_SetInjectedInput(s32 objId, u32 buttons, f32 stickX, f32 stickY);
s32 BtlCharApi_TestFlag0F(s32 objId);
s32 BtlCharApi_TestFlag05(s32 objId);
s32 BtlCharApi_GetParamByte84(s32 objId, u32 n);
s32 BtlCharApi_GetParamByte8A(s32 objId);
s32 BtlCharApi_GetParamByte8D(s32 objId);
u64 BtlCharApi_GetActionBits(s32 objId);
s32 BtlCharApi_IsAttackHitPending(s32 objId);
s32 BtlCharApi_TestFlag60(s32 objId);


/* ======== formerly btl_char_api_2_part2.h ======== */


#include "types.h"
#include "sys/math3d.h"

/*
 * Fighter interface, second part, 0x208430..0x20BA80 (src/battle/btl_capi_b.c). It continues btl_char_api_2.c:
 *   0x208430..0x209EE8  accessors keyed by battle object id (BtlChar_FindByObjId), nearly all for the AI
 *   0x209EE8..0x20B200  control of a fighter keyed by player index (BtlChar_Get): the sequence's poses and the
 *                       battle event script's commands (through btl_facade.c)
 *   0x20B200..0x20B418  the character-change request as the loader sees it, the partner attach
 *   0x20B418            key-config lookup for the HUD
 *   0x20B4A8..0x20BA80  accessors keyed by side (BtlChar_FindBySide) for the HUD and the AI, and by player index
 *                       for the sequence
 * Every function is safe on an id that is not a fighter and then returns 0 / -1 / does nothing.
 *
 * The structs are this module's partial views (no shared fighter header yet): only fields this file touches, or
 * that its direct callees show, are named.
 */

/* Gauge block of a team member (BtlMember_GetGauge(chr, n) / BtlMember_GetActiveGauge(chr)): member entry + 0x40. */
typedef struct BtlCapiBGauge {
    /* 0x00 */ s32 hp;        /* 10000 per bar; < 1 = down */
    /* 0x04 */ s32 hpMax;
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 ki;        /* 20000 per bar */
    /* 0x10 */ s32 kiMax;
    /* 0x14 */ s32 blast;     /* blast stock, 100000 per stock */
    /* 0x18 */ s32 blastMax;
    /* 0x1C */ s32 maxPower;  /* 0..30000, the timer of the powered-up mode (fighter flag 6) */
} BtlCapiBGauge;

/* Team member entry (BtlMember_GetActive): 0xA4 bytes at fighter +0x9A4. */
typedef struct BtlCapiBMember {
    /* 0x00 */ u8 unk0[0x38];
    /* 0x38 */ s32 cpuLevel;
    /* 0x3C */ s32 aiType;
    /* 0x40 */ BtlCapiBGauge gauge;
} BtlCapiBMember;

/* Pose block (fighter +0x10, BtlChar_GetPos). */
typedef BtlFighterPose BtlCapiBPose;

/* Fighter (0x1600 bytes). */
typedef BtlFighter BtlCapiBChr;

/* Character parameter block (battle object +0x91C); read here only through BtlCharApi_GetParamByte2 / Flags /
   Unk14 / Unk18. The rest is what the callees of this file read (0x20DF80..0x20F3C8). */
typedef struct BtlCapiBParam {
    /* 0x00 */ u16 charaFlags;            /* bit 0x80: BtlCharApi_HasParamBit80 */
    /* 0x02 */ s8 sizeClass;
    /* 0x03 */ u8 unk3[0x10 - 0x3];
    /* 0x10 */ s32 flags;           /* bits 4 / 8: armour level of BtlCharApi_GetArmorBreakLevel */
    /* 0x14 */ s32 flags2;
    /* 0x18 */ s32 flags3;
    /* 0x1C */ u8 unk1C[0x80 - 0x1C];
    /* 0x80 */ s16 blastLimit;      /* class-0 blasts alive at once (BtlParam_GetBlastLimitA) */
    /* 0x82 */ s16 blastLimitB;     /* class-1 (BtlParam_GetBlastLimitB) */
    /* 0x84 */ u8 unk84[0x8F - 0x84];
    /* 0x8F */ u8 comboFinish[9];         /* BtlParam_GetComboFinish(chr, n) */
    /* 0x98 */ u8 transformId[4];   /* 0xFF = none (BtlParam_GetSlotId) */
    /* 0x9C */ u8 transformCost[4]; /* blast stocks (BtlParam_GetSlotCost) */
    /* 0xA0 */ u8 unkA0[0xAC - 0xA0];
    /* 0xAC */ s8 transformSlot;    /* the slot the AI uses, -1 none (BtlParam_GetDefaultSlot) */
    /* 0xAD */ u8 formFlags;
    /* 0xAE */ u8 fusionCost[3];    /* blast stocks (BtlParam_GetCostAE) */
    /* 0xB1 */ u8 unkB1[3];
    /* 0xB4 */ u8 fusionId[3];      /* (BtlParam_GetFusionResult) */
} BtlCapiBParam;

/* Technique table (battle object +0x92C, BtlCharApi_GetSkillTable). Arrays are indexed by technique class
   (BtlAct_GetCurrentClass: 0 / 1 = the two move slots, 2..4 = the three skill slots); the AI's view
   (AiThChrSkills) indexes the same arrays by skill slot = class - 2. */
typedef struct BtlCapiBSkills {
    /* 0x000 */ u32 state[3];       /* by skill slot */
    /* 0x00C */ u32 flags[3];       /* by skill slot (BtlSuper_GetFlags(chr, cls)): 1, 2, 4 tested here */
    /* 0x018 */ u8 unk18[0x13C - 0x18];
    /* 0x13C */ s8 type[5];       /* by class (BtlSuper_GetType) */
    /* 0x141 */ u8 unk141[0x163 - 0x141];
    /* 0x163 */ s8 kind[5];         /* by class (BtlSuper_GetAiKind) */
    /* 0x168 */ u8 unk168[0x194 - 0x168];
    /* 0x194 */ s32 cost[5];        /* by class: ki cost (BtlSuper_GetKiCost) */
    /* 0x1A8 */ u8 unk1A8[0x227 - 0x1A8];
    /* 0x227 */ s8 promptRow[5];       /* BtlSuper_GetPromptRowIndex(chr, n): index into the roster table +0x34 */
} BtlCapiBSkills;

/* Move table (battle object +0x930, BtlCharApi_GetMoveTable): the two move slots. */
typedef struct BtlCapiBMoves {
    /* 0x00 */ u8 unk0[0x8];
    /* 0x08 */ u32 flags[2];        /* BtlSkill_GetFlags: bit 1 tested by BtlCtrl_UseTechnique, 0x100 by IsMoveFlag100 */
    /* 0x10 */ s16 id[2];
    /* 0x14 */ u8 unk14[0x94 - 0x14];
    /* 0x94 */ s8 stock[2];         /* blast stocks the move costs (BtlSkill_GetBlastCost) */
    /* 0x96 */ u8 unk96[0x9E - 0x96];
    /* 0x9E */ s8 kind[2];          /* BtlSkill_GetAiKind */
} BtlCapiBMoves;

/* Battle object of a fighter (BtlChar_GetObj). */
typedef struct BtlCapiBObj {
    /* 0x000 */ u8 unk0[0x91C];
    /* 0x91C */ BtlCapiBParam *param;
    /* 0x920 */ u8 unk920[0x92C - 0x920];
    /* 0x92C */ BtlCapiBSkills *skills;
    /* 0x930 */ BtlCapiBMoves *moves;
    /* 0x934 */ u8 unk934[0xCAC - 0x934];
    /* 0xCAC */ s8 hitCount;
    /* 0xCAD */ s8 hitIndex;          /* expected to be ~hitCount */
} BtlCapiBObj;

/* Blast list (EftHit_GetList): 64 records of 0x190 bytes and a count. */
typedef struct BtlCapiBBlastDef {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ s16 ownerId;
    /* 0x12 */ u8 unk12[0x18 - 0x12];
    /* 0x18 */ s16 kind;
} BtlCapiBBlastDef;

typedef struct BtlCapiBBlastSrc {
    /* 0x0 */ s32 ownerId;
} BtlCapiBBlastSrc;

typedef struct BtlCapiBBlastRec {
    /* 0x000 */ s32 objId;          /* fighter that fired it */
    /* 0x004 */ u8 unk4[0xC - 0x4];
    /* 0x00C */ s32 type;           /* 0: described by def, 1: by src */
    /* 0x010 */ u8 unk10[0x20 - 0x10];
    /* 0x020 */ u64 pos[2];         /* a Vec4; copied by value as two doublewords */
    /* 0x030 */ Vec4 prevPos;
    /* 0x040 */ u8 unk40[0x54 - 0x40];
    /* 0x054 */ s32 unk54;          /* 1: ignored */
    /* 0x058 */ u8 unk58[0x64 - 0x58];
    /* 0x064 */ BtlCapiBBlastSrc *src;
    /* 0x068 */ BtlCapiBBlastDef *def;
    /* 0x06C */ u8 unk6C[0x180 - 0x6C];
    /* 0x180 */ s32 seen;           /* set by BtlCharApi_MarkIncomingBlast: the AI has reacted to it */
    /* 0x184 */ u8 unk184[0x190 - 0x184];
} BtlCapiBBlastRec; /* size 0x190 */

typedef struct BtlCapiBBlastList {
    /* 0x0000 */ BtlCapiBBlastRec rec[64];
    /* 0x6400 */ s32 count;
} BtlCapiBBlastList;

/* Character-change request (roster +0x258 points at the one being served; btl_char_ctl.h has the queue). */
typedef struct BtlCapiBChange {
    /* 0x00 */ s32 player;
    /* 0x04 */ s32 type;            /* 0 / 1 */
    /* 0x08 */ s32 arg[7];          /* the words BtlChange_GetArgs copies out */
} BtlCapiBChange;

typedef struct BtlCapiBChangeQueue {
    /* 0x000 */ u8 unk0[0x120];
    /* 0x120 */ BtlCapiBChange *cur; /* roster +0x258 */
    /* 0x124 */ u8 unk124[0x12C - 0x124];
    /* 0x12C */ s32 state;           /* roster +0x264: 1 = waiting for the loader, 4 = ready */
} BtlCapiBChangeQueue;

/* One entry of the roster table +0x34 (0x14 bytes), indexed by BtlCapiBSkills.unk227[]. */
typedef struct BtlCapiBPrompt {
    /* 0x0 */ u8 unk0[2];
    /* 0x2 */ s8 button;            /* 0..3: which button the prompt wants */
    /* 0x3 */ u8 unk3[0x14 - 0x3];
} BtlCapiBPrompt;

/* Fighter roster (gBtlChars). */
typedef struct BtlCapiBMgr {
    /* 0x000 */ u8 unk0[0x34];
    /* 0x034 */ BtlCapiBPrompt *prompts;
    /* 0x038 */ u8 unk38[0x138 - 0x38];
    /* 0x138 */ BtlCapiBChangeQueue change;
} BtlCapiBMgr;

/* Battle work: only the flag word. */
typedef struct BtlCapiBWork {
    /* 0x0000 */ u8 unk0[0x19F0];
    /* 0x19F0 */ u64 flags;         /* 0x100 paused, 0x2000 loading */
} BtlCapiBWork;

/* --- by object id --- */
s32 BtlCharApi_IsAttackHitsDone(s32 objId);
s32 BtlCharApi_GetAction(s32 objId);
s32 BtlCharApi_GetBlastShots(s32 objId);
s32 BtlCharApi_GetBlastRoom(s32 objId);
s32 BtlCharApi_HasBlastLimit(s32 objId);
s32 BtlCharApi_GetBlastRoomB(s32 objId);
s32 BtlCharApi_TestFlag66(s32 objId);
s32 BtlCharApi_SkipBlastRec(s32 objId, BtlCapiBBlastRec *rec, s32 mode);
s32 BtlCharApi_FindIncomingBlast(s32 objId, s32 mode);
s32 BtlCharApi_IsBlastPassing(s32 objId);
void BtlCharApi_MarkIncomingBlast(s32 objId);
f32 BtlCharApi_GetTechniqueFramesLeft(s32 objId);
BtlCapiBSkills *BtlCharApi_GetSkillTable(s32 objId);
BtlCapiBMoves *BtlCharApi_GetMoveTable(s32 objId);
s32 BtlCharApi_GetAttackAttr(s32 objId);
s32 BtlCharApi_GetHp(s32 objId);
s32 BtlCharApi_GetHpMax(s32 objId);
s32 BtlCharApi_GetKi(s32 objId);
s32 BtlCharApi_GetBlast(s32 objId);
s32 BtlCharApi_GetBlastMax(s32 objId);
s32 BtlCharApi_GetMaxPower(s32 objId);
s32 BtlCharApi_GetSwitchGauge(s32 objId);
s32 BtlCharApi_IsBlockedByOpponent(s32 objId);
s32 BtlCharApi_HasParamBit80(s32 objId);
s32 BtlCharApi_CanTransform(s32 objId);
s32 BtlCharApi_CanFuse(s32 objId);
s32 BtlCharApi_CanSwitch(s32 objId);
s32 BtlCharApi_GetCpuLevel(s32 objId);
s32 BtlCharApi_GetAiType(s32 objId);
s32 BtlCharApi_IsCounterWindowBusy(s32 objId);
s32 BtlCharApi_GetTransformCost(s32 objId);
s32 BtlCharApi_PickFusionSlot(s32 objId);
s32 BtlCharApi_PickTransformSlot(s32 objId);
s32 BtlCharApi_IsChangingForm(s32 objId);
s32 BtlCharApi_GetParamFlags2(s32 objId);
s32 BtlCharApi_GetParamFlags(s32 objId);
s32 BtlCharApi_GetParamFlags3(s32 objId);
s32 BtlCharApi_GetMemberCount(s32 objId);
s32 BtlCharApi_GetActiveMember(s32 objId);
s32 BtlCharApi_GetSwitchTarget(s32 objId);
s32 BtlCharApi_GetMemberHpPercent(s32 objId, s32 member);
s32 BtlCharApi_GetMemberKiPercent(s32 objId, s32 member);
s32 BtlCharApi_GetOppTechniqueKind(s32 objId);
s32 BtlCharApi_IsOppSkillFlag4(s32 objId);
s32 BtlCharApi_TestOppSkillFlags(s32 objId);
s32 BtlCharApi_GetOppSkillClass(s32 objId);
s32 BtlCharApi_GetOppSkillKind(s32 objId);
s32 BtlCharApi_GetClashCountB(s32 objId);
s32 BtlCharApi_GetClashCountA(s32 objId);
s32 BtlCharApi_IsMoveSlotActive(s32 objId, u32 slot);
s32 BtlCharApi_IsMoveFlag100(s32 objId, u32 slot);
s32 BtlCharApi_GetStunTimer(s32 objId);
s32 BtlCharApi_GetPromptButtons(s32 objId);
s32 BtlCharApi_GetStoryAiForce(s32 objId);
s32 BtlCharApi_IsDodgeWindowReady(s32 objId);
s32 BtlCharApi_GetArmorBreakLevel(s32 objId);
s32 BtlCharApi_GetParamByte2(s32 objId);
f32 BtlCharApi_GetTechChargeB(s32 objId);
s32 BtlCharApi_GetVanishStrikesLeft(s32 objId);
s32 BtlCharApi_IsAnimFlag2800(s32 objId);
s32 BtlCharApi_TestPoseBit80(s32 objId, s32 always);
s32 BtlCharApi_TestFlag98(s32 objId);
s32 BtlCharApi_TestFlagBE(s32 objId);
s32 BtlCharApi_GetTechniqueCooldown(s32 objId);
s32 BtlCharApi_GetParamByte8F(s32 objId, s32 n);

/* --- by player index: sequence poses --- */
void BtlCtrl_StartEntrance(s32 player);
void BtlCtrl_EndEntrance(s32 player);
void BtlCtrl_StartWinPose(s32 player);
void BtlCtrl_StartLosePose(s32 player);
s32 BtlCtrl_IsPoseReached(s32 player);

/* --- by player index: the battle event script (BtlCtrl_* names already in btl_scene.txt are kept) --- */
void BtlCtrl_PlayMotion(s32 player, s32 motion, s32 loop, f32 blend);
void BtlCtrl_SetPos(s32 player, Vec4 *pos);
void BtlCtrl_GetPos(s32 player, Vec4 *out);
void BtlCtrl_SetRot(s32 player, Vec4 *rot);
void BtlCtrl_GetRot(s32 player, Vec4 *out);
s32 BtlCtrl_GetActiveMember(s32 player);
s32 BtlCtrl_IsAction4(s32 player);
s32 BtlCtrl_IsMotionPlaying(s32 player);
void BtlCtrl_StopMotion(s32 player);
void BtlCtrl_SetAuraOn(s32 player);
void BtlCtrl_SetAuraOff(s32 player);
s32 BtlCtrl_SetChargeFx(s32 player, s32 on);  /* no result: see the source */
void BtlCtrl_BurstChargeFx(s32 player);
void BtlCtrl_ClearHidden(s32 player);
void BtlCtrl_SetHidden(s32 player);
void BtlCtrl_ReloadMemberBonus(s32 player, u32 member);
void BtlCtrl_ReloadMemberAbilities(s32 player, u32 member);
void BtlCtrl_SetFlag10D(s32 player);
void BtlCtrl_ClearFlag10D(s32 player);
s32 BtlCtrl_AddHp(s32 player, s32 member, f32 percent);  /* no result: see the source */
void BtlCtrl_RaiseHp(s32 player, s32 member, f32 percent);
void BtlCtrl_LowerHp(s32 player, s32 member, f32 percent);
s32 BtlCtrl_AddKi(s32 player, s32 member, s32 bars);  /* no result: see the source */
void BtlCtrl_RaiseKi(s32 player, s32 member, s32 bars);
void BtlCtrl_LowerKi(s32 player, s32 member, s32 bars);
s32 BtlCtrl_AddBlast(s32 player, s32 member, s32 stocks);  /* no result: see the source */
void BtlCtrl_RaiseBlast(s32 player, s32 member, s32 stocks);
void BtlCtrl_LowerBlast(s32 player, s32 member, s32 stocks);
s32 BtlCtrl_SetMaxPower(s32 player, s32 on);  /* no result: see the source */
s32 BtlCtrl_Transform(s32 player, s32 id);
s32 BtlCtrl_Fuse(s32 player, s32 id);
s32 BtlCtrl_ChangeMember(s32 player, s32 member);
s32 BtlCtrl_ForceFlag11x(s32 player, s32 kind);
s32 BtlCtrl_UseTechnique(s32 player, s32 kind);
s32 BtlCtrl_ForceReaction(s32 player, u32 kind);
s32 BtlCtrl_IsInterruptible(s32 player);
s32 BtlCtrl_CanAct(s32 player);

/* --- the character-change request, for the loader --- */
s32 BtlChange_IsPendingType0(s32 player);
s32 BtlChange_IsPendingType1(s32 player);
void BtlChange_GetArgs(s32 *chara, s32 *costume, s32 *variant, s32 *animChara, s32 *animChara2, s32 *voiceChara, s32 *slot);
void BtlChange_NotifyTaken(void);
void BtlChange_NotifyLoaded(void);
s32 BtlChange_IsReady(void);
void BtlCtrl_AttachPartner(s32 player, s32 slot, s32 objId);
s32 BtlCharApi_GetButtonIcon(BtlCapiBChr *chr, u32 bit);

/* --- by side --- */
s32 BtlSide_GetHp(s32 side);
s32 BtlSide_GetKi(s32 side);
s32 BtlSide_GetBlast(s32 side);
s32 BtlSide_GetBlastMax(s32 side);
s32 BtlSide_GetMaxPower(s32 side);
s32 BtlSide_GetSwitchGauge(s32 side);
s32 BtlSide_GetActiveMember(s32 side);
s32 BtlSide_GetSwitchTarget(s32 side);
s32 BtlSide_CountAlive(s32 side);
s32 BtlSide_GetSwitchTargetHp(s32 side);
s32 BtlSide_GetSwitchTargetHpMax(s32 side);
s32 BtlSide_GetKiRecoverGoal(s32 side);
s32 BtlSide_IsPoweredUp(s32 side);
s32 BtlSide_TestFlagBE(s32 side);
s32 BtlCtrl_IsActiveDead(s32 player);
s32 BtlCtrl_IsTeamDead(s32 player);
s32 BtlCtrl_TestFlag7(s32 player);
s32 BtlSide_GetComboHits(s32 side);
s32 BtlSide_GetComboDamage(s32 side);
s32 BtlSide_IsComboShown(s32 side);
s32 BtlSide_IsComboHitNew(s32 side);


#endif

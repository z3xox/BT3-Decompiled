#ifndef BATTLE_BTL_CHAR_MGR_H
#define BATTLE_BTL_CHAR_MGR_H

#include "types.h"
#include "battle/btl_fighter.h"

/*
 * Fighter manager (src/battle/btl_char_mgr.c, 0x1C0058..0x1C2FF0): the roster gBtlChars, the per-fighter
 * reset, and the whole-roster phases Battle_Loop / Battle_Update call every frame. The phase order and what
 * each phase runs per fighter is described at the top of btl_char_mgr.c.
 *
 * The structures below are this module's partial views. The fighter object (0x1600 bytes) and the fighter's
 * BtlObj are shared with many modules that are not decompiled: the fields listed are only the ones this file
 * reads or writes. All names are guesses unless the comment says how they are known.
 */

#define BTL_CHR_MEMBER_MAX 5

/* Sequence stage passed to BtlChar_SetStage (stored in the byte at fighter + 0x1084): which phase the fighter is in. */
#define BTL_CHR_STAGE_RESET    1
#define BTL_CHR_STAGE_SAMPLE   2
#define BTL_CHR_STAGE_INPUT    3
#define BTL_CHR_STAGE_6        6
#define BTL_CHR_STAGE_7        7
#define BTL_CHR_STAGE_8        8
#define BTL_CHR_STAGE_9        9
#define BTL_CHR_STAGE_10       10
#define BTL_CHR_STAGE_EVENTS   11
#define BTL_CHR_STAGE_END      12

/* BtlCharMgr.flags */
#define BTL_CHARS_STARTED 1 /* set the first frame the sequence is in Ready (2) or Fight (3) */

/* Gauges of one team member: member entry + 0x40 (BtlMember_GetActiveGauge returns this for the current member). */
typedef BtlMemberGauge BtlMgrGauge;

/* One team member inside the fighter: fighter + 0x9A4 + n * 0xA4 (BtlMember_Get). */
typedef BtlMember BtlMgrMember;

/* The pose block at fighter + 0x10 (BtlChar_GetPos); copied whole to fighter + 0x100 at the start of a frame. */
typedef BtlFighterPose BtlMgrPose;

/* Nine bytes at fighter + 0x1262, copied to + 0x126B and cleared at the start of a frame. */
typedef BtlFighterFxBits BtlMgrBytes9;

/* The fighter's display / collision object (BtlObj_Get(objId)): only the fields used here. */
typedef struct BtlMgrObj {
    /* 0x0000 */ u8 unk0[0xA4];
    /* 0x00A4 */ u32 *file[3];   /* three loaded data files; word 3 of each is the byte offset of a table */
    /* 0x00B0 */ u8 unkB0[0xA40 - 0xB0];
    /* 0x0A40 */ u32 flags;      /* bit 1: drawn in front / highlighted (see BtlChars_UpdateObjFlag2),
                                    bit 30: BtlMgrGauge.unk20 */
    /* 0x0A44 */ u8 unkA44[0x1660 - 0xA44];
    /* 0x1660 */ u8 *charaWork;    /* block whose word at + 0x18028 mirrors BtlChar_IsBodyChanged() */
} BtlMgrObj;

/* Partial view of a fighter (0x1600 bytes, BtlChar_Get). */
typedef BtlFighter BtlMgrChr;

/* The roster: gBtlChars -> 0x280 bytes. */
typedef struct BtlCharMgr {
    /* 0x000 */ s32 count;       /* number of fighters (Battle_Init passes 2) */
    /* 0x004 */ BtlMgrChr *chars; /* count * 0x1600 */
    /* 0x008 */ void *sounds;      /* count * 0x34 bytes: per side, four 0xC-byte slots (BtlCharSnd_GetSet) */
    /* 0x00C */ void *loopSounds;      /* count * 0x34 bytes: per side, four sound handle slots (BtlCharSnd_GetLoopSet) */
    /* 0x010 */ s32 flags;       /* BTL_CHARS_* */
    /* 0x014 */ s32 frame;       /* frames simulated since the start, 30 bits */
    /* 0x018 */ s32 randState;
    /* 0x01C */ s32 stageTimer;       /* counts down to 0 on every simulated frame */
    /* 0x020 */ u32 *tbl[6];     /* tables inside common file 2: header words 3, 5, 6, 7, 8, 9 */
    /* 0x038 */ u8 unk38[8];
    /* 0x040 */ u8 clash[0x60];  /* cleared by a reset */
    /* 0x0A0 */ u8 snd[0x90];  /* cleared by a reset; four 0x20-byte sound request records from +0xA0 on, their count at +0x120 */
    /* 0x130 */ u64 viewer;      /* cleared by a reset */
    /* 0x138 */ u8 change[0x140]; /* cleared by a reset; +0x274 is BtlChars_IsTimeStopped() */
    /* 0x278 */ u8 unk278[8];
} BtlCharMgr; /* 0x280 */

extern BtlCharMgr *gBtlChars;

void BtlChar_BindObject(BtlMgrChr *chr);
void BtlChar_CopyMemberBonus(BtlMgrChr *chr, s32 idx);
void BtlChar_CopyAllBonus(BtlMgrChr *chr);
void BtlChar_CopyMemberAbility(BtlMgrChr *chr, s32 idx);
void BtlChar_CopyAllAbility(BtlMgrChr *chr);
void BtlChar_Reset(BtlMgrChr *chr);
void BtlChar_OnModelLoaded(BtlMgrChr *chr);
void BtlChar_OnStageLoaded(BtlMgrChr *chr);
void BtlChar_ResetRound(BtlMgrChr *chr);
void BtlChar_ResetObjWork(BtlMgrChr *chr);
void BtlChars_UpdateFreeze(void);
void BtlChars_UpdateObjFlag2(void);
void BtlChars_ClearObjFlag2(void);
void BtlChars_UpdateCollision(void);
void BtlChar_RaiseEvents(BtlMgrChr *chr);
void BtlChars_CheckRoundReset(void);
void BtlChar_ApplyBonusRequests(BtlMgrChr *chr);
void BtlChar_ApplyAbilityRequests(BtlMgrChr *chr);
void BtlChar_SampleInput(BtlMgrChr *chr);
void BtlChar_BeginFrame(BtlMgrChr *chr);
void BtlChar_UpdateSeqFlags(BtlMgrChr *chr);
void BtlChar_UpdateMotion(BtlMgrChr *chr);
void BtlChar_UpdateStage6(BtlMgrChr *chr);
void BtlChar_UpdateStage7(BtlMgrChr *chr);
void BtlChar_UpdateStage8(BtlMgrChr *chr);
void BtlChar_UpdateStage9(BtlMgrChr *chr);
void BtlChar_UpdateCamera(BtlMgrChr *chr);
void BtlChar_UpdateStage10(BtlMgrChr *chr);
void BtlChar_PostScene(BtlMgrChr *chr);
void BtlChar_UpdateLate(BtlMgrChr *chr);
void BtlChar_EndFrame(BtlMgrChr *chr);
void BtlChar_AllocAll(s32 count);
void BtlChar_FreeAll(void);
void BtlChar_ResetAll(void);
void BtlChars_OnModelLoaded(s32 side);
void BtlChars_OnStageLoaded(void);
void BtlChars_SampleInput(void);
s32 BtlChars_CheckStart(void);
void BtlChars_UpdateInput(void);
void BtlChars_UpdateMain(void);
void BtlChars_PostScene(void);
void BtlChars_EndFrame(void);

#endif

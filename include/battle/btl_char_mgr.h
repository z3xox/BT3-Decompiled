#ifndef BATTLE_BTL_CHAR_MGR_H
#define BATTLE_BTL_CHAR_MGR_H

#include "types.h"
#include "battle/btl_fighter.h"

/*
 * Fighter manager (src/battle/btl_char_mgr.c, 0x1C0058..0x1C2FF0): the roster gBtlChars, the per-fighter
 * reset, and the whole-roster phases Battle_Loop / Battle_Update call every frame. The phase order and what
 * each phase runs per fighter is described at the top of btl_char_mgr.c.
 *
 * The structures below are this module's partial views; the fighter object (0x1600 bytes) is BtlFighter
 * (battle/btl_fighter.h). The fighter's BtlObj is shared with many modules: the fields listed are only the
 * ones this file reads or writes. All names are guesses unless the comment says how they are known.
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

/* The fighter's display / collision object (BtlObj_Get(objId)): only the fields used here. */
typedef struct BtlMgrObj {
    /* 0x0000 */ u8 unk0[0xA4];
    /* 0x00A4 */ u32 *file[3];   /* three loaded data files; word 3 of each is the byte offset of a table */
    /* 0x00B0 */ u8 unkB0[0xA40 - 0xB0];
    /* 0x0A40 */ u32 flags;      /* bit 1: drawn in front / highlighted (see BtlChars_UpdateObjFlag2),
                                    bit 30: BtlMemberGauge.variant != 0 */
    /* 0x0A44 */ u8 unkA44[0x1660 - 0xA44];
    /* 0x1660 */ u8 *charaWork;    /* block whose word at + 0x18028 mirrors BtlChar_IsBodyChanged() */
} BtlMgrObj;

/* The roster: gBtlChars -> 0x280 bytes. */
typedef struct BtlCharMgr {
    /* 0x000 */ s32 count;       /* number of fighters (Battle_Init passes 2) */
    /* 0x004 */ BtlFighter *chars; /* count * 0x1600 */
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

void BtlChar_BindObject(BtlFighter *chr);
void BtlChar_CopyMemberBonus(BtlFighter *chr, s32 idx);
void BtlChar_CopyAllBonus(BtlFighter *chr);
void BtlChar_CopyMemberAbility(BtlFighter *chr, s32 idx);
void BtlChar_CopyAllAbility(BtlFighter *chr);
void BtlChar_Reset(BtlFighter *chr);
void BtlChar_OnModelLoaded(BtlFighter *chr);
void BtlChar_OnStageLoaded(BtlFighter *chr);
void BtlChar_ResetRound(BtlFighter *chr);
void BtlChar_ResetObjWork(BtlFighter *chr);
void BtlChars_UpdateFreeze(void);
void BtlChars_UpdateObjFlag2(void);
void BtlChars_ClearObjFlag2(void);
void BtlChars_UpdateCollision(void);
void BtlChar_RaiseEvents(BtlFighter *chr);
void BtlChars_CheckRoundReset(void);
void BtlChar_ApplyBonusRequests(BtlFighter *chr);
void BtlChar_ApplyAbilityRequests(BtlFighter *chr);
void BtlChar_SampleInput(BtlFighter *chr);
void BtlChar_BeginFrame(BtlFighter *chr);
void BtlChar_UpdateSeqFlags(BtlFighter *chr);
void BtlChar_UpdateMotion(BtlFighter *chr);
void BtlChar_UpdateStage6(BtlFighter *chr);
void BtlChar_UpdateStage7(BtlFighter *chr);
void BtlChar_UpdateStage8(BtlFighter *chr);
void BtlChar_UpdateStage9(BtlFighter *chr);
void BtlChar_UpdateCamera(BtlFighter *chr);
void BtlChar_UpdateStage10(BtlFighter *chr);
void BtlChar_PostScene(BtlFighter *chr);
void BtlChar_UpdateLate(BtlFighter *chr);
void BtlChar_EndFrame(BtlFighter *chr);
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

#ifndef BATTLE_BTL_CHAR_FX_H
#define BATTLE_BTL_CHAR_FX_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Fighter effect requests, animation events and the partner object
 * (src/battle/btl_char_fx_1.c, btl_char_fx_b.c, btl_char_fx_2.c: 0x1D00D8..0x1D3B40).
 * The structures are partial views local to this module: only the fields these files touch. The fighter is
 * BtlFighter (battle/btl_fighter.h).
 */

/* An action record of a battle object (obj + 0x91C points at the current one): only the attribute word. */
typedef struct FxObjAction {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ u32 attr;   /* 0x8000 and 0x200000 are tested here */
} FxObjAction;

/* Header of the animation an object is playing. */
typedef struct FxAnimHead {
    /* 0x0 */ u16 unk0;
    /* 0x2 */ u16 length;    /* 0: the partner is hidden while this animation plays */
} FxAnimHead;

/* Animation playback of a battle object: obj + 0xB40. */
typedef struct FxObjAnim {
    /* 0x000 [0xB40] */ FxAnimHead *head;
    /* 0x004 [0xB44] */ f32 end;       /* last frame */
    /* 0x008 [0xB48] */ u8 unk8[0x130 - 0x8];
    /* 0x130 [0xC70] */ s32 manual;    /* set to 1 when the partner starts an animation */
    /* 0x134 [0xC74] */ s32 mode;
    /* 0x138 [0xC78] */ f32 frame;
    /* 0x13C [0xC7C] */ f32 prevFrame;
    /* 0x140 [0xC80] */ f32 step;
} FxObjAnim;

/* Bits of FxObj.flags as this module uses them. */
#define FX_OBJ_DRAWN        0x2        /* required for flag 0x80 and for the impact flash; cleared to hide the partner */
#define FX_OBJ_FLAG10       0x10
#define FX_OBJ_FLAG20       0x20       /* set / cleared by animation events 0x20 / 0x40 */
#define FX_OBJ_FLAG40       0x40       /* set / cleared by animation event bits 43 / 44 */
#define FX_OBJ_FLAG80       0x80
#define FX_OBJ_BYTE2        0xFF0000   /* request 3 writes 8 when it starts and 1 when it ends */
#define FX_OBJ_FULL_AURA    0x40000    /* forces the aura level to 1 */
#define FX_OBJ_OCCLUDED     0x2000000  /* the stage is between this fighter and its camera */

/* A battle object (BtlChar_GetObj / BtlObj_Get): only the fields used here. */
typedef struct FxObj {
    /* 0x000 */ u8 unk0[0xC];
    /* 0x00C */ s32 chara;       /* character number (voice lines are chara * 100 + line) */
    /* 0x010 */ s32 id;          /* object id */
    /* 0x014 */ u8 unk14[0x91C - 0x14];
    /* 0x91C */ FxObjAction *action;
    /* 0x920 */ u8 unk920[0x950 - 0x920];
    /* 0x950 */ Vec4 pos;
    /* 0x960 */ Vec4 rot;
    /* 0x970 */ u8 unk970[0x9A0 - 0x970];
    /* 0x9A0 */ u8 mtx[0x40];    /* matrix the model part offsets of event 8 are multiplied by */
    /* 0x9E0 */ u8 unk9E0[0xA24 - 0x9E0];
    /* 0xA24 */ s32 area;      /* copied into the damage spark argument */
    /* 0xA28 */ u8 unkA28[0xA40 - 0xA28];
    /* 0xA40 */ u32 flags;       /* FX_OBJ_ bits. The original tests "0x2 and 0x80" with one 64-bit load, which is
                                    what two adjacent bit-field tests compile to. */
    /* 0xA44 */ u8 unkA44[0xB40 - 0xA44];
    /* 0xB40 */ FxObjAnim anim;
} FxObj;

/* Argument of EftImpact_SpawnHit / EftImpact_SpawnHitScaled (the "flash" effects). */
typedef struct FxPosArg {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ Vec4 pos2;
    /* 0x20 */ s32 kind;
    /* 0x24 */ s32 objId;
    /* 0x28 */ s32 unk28;
    /* 0x2C */ s32 unk2C;
} FxPosArg;

/* Argument of the hit sparks of request 0x17. */
typedef struct FxHitArg {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ s32 objId;
    /* 0x14 */ s32 kind;
    /* 0x18 */ s32 texVariant;
    /* 0x1C */ f32 scale;
} FxHitArg;

/* Launch argument of a ki blast (EftKiBlast_Fire and its four variants); pos carries the launch direction. */
typedef struct FxHitArg2 {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ u16 objId;
    /* 0x12 */ u16 objId2;
    /* 0x14 */ u16 node;     /* model node of the animation event (BtlObjAnim_MaskToNode): the firing hand */
    /* 0x16 */ u16 area;    /* obj->unkA24 */
    /* 0x18 */ s16 level;    /* BtlKiBlast_GetCurrentKind: 0..11, its low two bits pick the size */
    /* 0x1A */ u8 type;      /* BtlKiBlast_GetType: picks the spark module (5, 4, 2, 3, other) */
    /* 0x1B */ u8 texVariant;
    /* 0x1C */ s16 life;
    /* 0x20 */ f32 speed;
    /* 0x24 */ f32 turn;
    /* 0x28 */ f32 radius;
    /* 0x2C */ f32 unk2C;
    /* 0x30 */ s32 size;     /* 0..3 */
    /* 0x34 */ s32 unk34;
    /* 0x38 */ s32 canHit;
    /* 0x3C */ s32 unk3C;
    /* 0x40 */ u8 unk40;
    /* 0x41 */ u8 shotCount;
    /* 0x42 */ u8 unk42[0xE];
} FxHitArg2; /* size 0x50 */

/* Argument of EftBodyFx_Start. */
typedef struct FxArg3 {
    /* 0x0 */ s32 objId;
    /* 0x4 */ s32 pack;
    /* 0x8 */ f32 scale;
    /* 0xC */ s32 unkC;
} FxArg3;

/* Argument of EftAbsorb_Start / EftAbsorb_StartHands / EftCharaFx_Start. */
typedef struct FxArg2 {
    /* 0x0 */ s32 objId;
    /* 0x4 */ f32 scale;
    /* 0x8 */ s32 unk8[2];
} FxArg2;

/* Argument of EftRushBurst_Start. */
typedef struct FxDirArg {
    /* 0x00 */ Vec4 dir;
    /* 0x10 */ s32 objId;
    /* 0x14 */ s32 node;
    /* 0x18 */ f32 scale;
    /* 0x1C */ s32 unk1C;
} FxDirArg;

/* Argument of EftRay_CreateByValue (the speed lines of request 0x15). The original's block is 0x60 bytes. */
typedef struct FxLineArg {
    /* 0x00 */ Vec4 pos;      /* (0, 0, 0, 1) */
    /* 0x10 */ s32 r;         /* colour: r, then g, b, alpha. The callee reads s32 color[4]; the split (a scalar and
                                 an array of three) is what the initialiser's store order needs to match */
    /* 0x14 */ s32 gba[3];
    /* 0x20 */ f32 life;     /* 0.3 + 0.5 * |angle to the opponent, relative to the camera yaw| / pi */
    /* 0x24 */ f32 length;
    /* 0x28 */ f32 width;
    /* 0x2C */ f32 inner;     /* -20 - distance * 0.1, not below -50 */
    /* 0x30 */ f32 jitter;
    /* 0x34 */ s32 mode;
    /* 0x38 */ s32 count;
    /* 0x3C */ s32 objId;
    /* 0x40 */ s32 blend;
    /* 0x44 */ s32 space;
    /* 0x48 */ s32 delay;
    /* 0x4C */ s32 fadeFrames;
    /* 0x50 */ s32 autoKill;
} __attribute__((aligned(16))) FxLineArg; /* size 0x60 */

/* The part of the save block read here. */
typedef struct FxSave {
    /* 0x0000 */ u8 unk0[0x1608];
    /* 0x1608 */ s32 flags;   /* bit 0: alternate voices (voice line base 0xCC32 instead of 0x8D4E) */
} FxSave;

/* btl_char_fx_1.c */
void BtlFx_SpawnFlashReq20(BtlFighter *chr);
void BtlFx_SpawnFlashReq2B(BtlFighter *chr);
void BtlFx_SpawnHitSparkReq17(BtlFighter *chr);
s32 BtlFx_UpdateAura(BtlFighter *chr);
void BtlFx_UpdateChargeFx(BtlFighter *chr);
s32 BtlFx_UpdatePowerUpLook(BtlFighter *chr);
void BtlFx_SpawnReq6(BtlFighter *chr);
void BtlFx_UpdateScreenFilter(BtlFighter *chr);
void BtlFx_SpawnBurstReqC(BtlFighter *chr);
void BtlFx_SpawnFastMoveFx(BtlFighter *chr);
void BtlFx_SpawnSpeedLines(BtlFighter *chr);

/* btl_char_fx_b.c */
void BtlFx_SpawnReq18(BtlFighter *chr);
void BtlFx_UpdateGroundFx(BtlFighter *chr);
void BtlFx_UpdateWaterFx(BtlFighter *chr);
void BtlFx_UpdateAimedFxReq28(BtlFighter *chr);
void BtlFx_UpdateReq1A(BtlFighter *chr);
void BtlFx_UpdateReq38(BtlFighter *chr);
void BtlFx_SpawnReq37(BtlFighter *chr);
void BtlFx_SpawnFlashReq2E(BtlFighter *chr);
void BtlFx_UpdateCharaFx(BtlFighter *chr);
void BtlFx_SpawnClashFlash(BtlFighter *chr);
void BtlFx_TriggerObjReq1D(BtlFighter *chr);
void BtlFx_TriggerObjReq1E(BtlFighter *chr);
void BtlFx_TriggerObjReq1F(BtlFighter *chr);
void BtlFx_UpdateObjFlagReq3(BtlFighter *chr);
void BtlFx_UpdateOccludedFlag(BtlFighter *chr);
void BtlFx_UpdateObjCmdFlag137(BtlFighter *chr);
void BtlFx_UpdateSubStateFlag96(BtlFighter *chr);
void BtlFx_UpdateObjFlag80(BtlFighter *chr);
void BtlFx_FireKiBlast(BtlFighter *chr);

/* btl_char_fx_2.c */
void BtlFx_HandleImpactEvents(BtlFighter *chr);
void BtlFx_HandleEvent1(BtlFighter *chr);
void BtlFx_HandleFlag30Events(BtlFighter *chr);
void BtlFx_PlaySwingSound(BtlFighter *chr);
void BtlFx_SpawnPartFxEvent8(BtlFighter *chr);
void BtlFx_SpawnPartFlashes(BtlFighter *chr);
s32 BtlFx_PushBackOnEvent400(BtlFighter *chr);
s32 BtlFx_SetObjMaskOnEvent400(BtlFighter *chr);
void BtlFx_ShakeCamOnEvent(BtlFighter *chr);
void BtlFx_VibrateOnEvent(BtlFighter *chr);
void BtlFxObj_HandleFlagEvents(FxObj *obj);
void BtlFxObj_PlayEventSounds(FxObj *obj);
void BtlFxObj_HandleSubStateEvents(FxObj *obj);
void BtlFx_UpdateObjEvents(BtlFighter *chr);
void BtlFx_UpdateAll(BtlFighter *chr);
void BtlFx_UpdateAfterHits(BtlFighter *chr);
void BtlFx_UpdatePostScene(BtlFighter *chr);
void BtlFx_UpdateLate(BtlFighter *chr);
void BtlFx_UpdateStageFxRequest(BtlFighter *chr);
void BtlFx_UpdateStageFx(void);
void BtlFx_UpdateRoster(void);
void BtlFxObj_Place(FxObj *obj, Vec4 *pos, Vec4 *rot);
void BtlPartner_HandleEvents(FxObj *obj, BtlFighter *chr);
s32 BtlPartner_IsActive(BtlFighter *chr);
void BtlPartner_Attach(BtlFighter *chr, s32 res, s32 objId);
void BtlPartner_Release(BtlFighter *chr);
void BtlPartner_Update(BtlFighter *chr);
void BtlPartner_UpdateEvents(BtlFighter *chr);
void BtlPartner_PlayAnim(BtlFighter *chr, s32 anim);
s32 BtlPartner_StepAnim(BtlFighter *chr);
void BtlPartner_SetFlag10(BtlFighter *chr, s32 on);
void BtlPartner_LinkToOwner(BtlFighter *chr);
FxObj *BtlPartner_GetObj(BtlFighter *chr);
void BtlPartner_PushAngle(BtlFighter *chr, f32 yaw, f32 speed, f32 max);
void BtlPartner_PushDir(BtlFighter *chr, Vec4 *dir, f32 speed, f32 max);
void BtlPartner_AddSway(BtlFighter *chr, f32 add, f32 max);
void BtlPartner_SetFlag80(BtlFighter *chr, s32 on);

#endif

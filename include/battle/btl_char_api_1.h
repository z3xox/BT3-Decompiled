#ifndef BATTLE_BTL_CAPI_A_H
#define BATTLE_BTL_CAPI_A_H

#include "types.h"
#include "battle/btl_fighter.h"
#include "sys/math3d.h"

/*
 * Fighter interface, first part: 0x204E78..0x207020 (src/battle/btl_char_api_1.c), 81 functions. The same original
 * source file continues in btl_char_api_2.c (0x207020) and btl_capi_b.c: see the header of btl_char_api_2.c for the
 * evidence. This is what the AI, the effect scene and its modules, the camera and the HUD call to read a fighter
 * without knowing its layout.
 *
 * Arguments named `objId` are battle object ids (BtlObj_Get index; a fighter's own id is at fighter +0xC, and in a
 * battle the two fighters are objects 0 and 1); arguments named `player` are roster indices (BtlChar_Get). Every
 * function is safe on an id that is not a fighter, or not an object, and then returns 0 / 0.0f / a zero vector /
 * an identity, except where the comment in the source gives another default.
 *
 * The structs are this module's partial views; the fighter is BtlFighter (battle/btl_fighter.h). Fields are named
 * where another module's matching code or this file's callers say what they are; the rest are unkXX.
 */

/* General parameters of the character (object +0x91C); BtlParam / BtlTechBParam in btl_param.h / btl_tech.h. */
typedef struct BtlCapiParam {
    /* 0x00 */ u16 charaFlags;     /* bits tested by callers: 0x4, 0x8, 0x80 */
    /* 0x02 */ u8 sizeClass;
    /* 0x03 */ s8 auraType;
    /* 0x04 */ u8 unk4[0xC - 0x4];
    /* 0x0C */ f32 radius;     /* body radius; <= 0: object +0xFF0 * 1.8 is used */
    /* 0x10 */ s32 flags;      /* 0x40 / 0x80: see BtlCharApi_HasWeaponOut; 0x800: see BtlCharApi_TestStageBreakA */
    /* 0x14 */ u8 unk14[0xC8 - 0x14];
    /* 0xC8 */ f32 unkC8;
} BtlCapiParam;

/* One ki blast record (object +0x924, 13 of them); BtlKiBlastData in btl_tech.h. */
typedef struct BtlCapiKiBlast {
    /* 0x00 */ u8 unk0[2];
    /* 0x02 */ s8 type;
    /* 0x03 */ u8 unk3[0x34 - 3];
} BtlCapiKiBlast; /* size 0x34 */

/* Model node, as returned by BtlObj_GetNode(obj, node). */
typedef struct BtlCapiNode {
    /* 0x00 */ u8 unk0[0x10];
    /* 0x10 */ Mtx44 mtx;      /* world matrix; its translation (+0x40) is the node position */
    /* 0x50 */ Mtx44 parent;
    /* 0x90 */ Vec4 pos;
    /* 0xA0 */ Quat rot;
} BtlCapiNode;

/* What BtlObj_FindBound(obj, n) returns. */
typedef struct BtlCapiPart {
    /* 0x00 */ u8 unk0[0x5C];
    /* 0x5C */ f32 sizeFactor;
} BtlCapiPart;

/* Battle object (BtlObj in btl_obj.h): only the fields read here. */
typedef struct BtlCapiObj {
    /* 0x000 */ u8 unk0[0xC];
    /* 0x00C */ s32 chara;          /* character id */
    /* 0x010 */ u8 unk10[0x58 - 0x10];
    /* 0x058 */ s32 charPack;
    /* 0x05C */ u8 unk5C[0x9C - 0x5C];
    /* 0x09C */ s32 effectPacks[5];       /* five pointers of the model data block */
    /* 0x0B0 */ u8 unkB0[0x91C - 0xB0];
    /* 0x91C */ BtlCapiParam *param;
    /* 0x920 */ u8 unk920[4];
    /* 0x924 */ BtlCapiKiBlast *kiBlasts;
    /* 0x928 */ u8 unk928[4];
    /* 0x92C */ void *supers;       /* BtlSuperData (btl_tech.h) */
    /* 0x930 */ void *skills;       /* BtlSkillData */
    /* 0x934 */ u8 unk934[0x960 - 0x934];
    /* 0x960 */ Vec4 rot;
    /* 0x970 */ Vec4 pos;
    /* 0x980 */ u8 unk980[0xA40 - 0x980];
    /* 0xA40 */ union {
        u64 flags64;                /* the tests of two bits read all 64 bits */
        s32 flags;                  /* 2 drawn; 0x20, 0x40 see BtlCharApi_HasWeaponOut; 0x40000 forces aura type 3 */
    } u;
    /* 0xA48 */ u8 unkA48[0xFA0 - 0xA48];
    /* 0xFA0 */ Vec4 *bodyPos;      /* -> {Vec4 pos; f32 radius} (BtlMoveObjBody in btl_char_move.h) */
    /* 0xFA4 */ u8 unkFA4[0xFF0 - 0xFA4];
    /* 0xFF0 */ f32 unkFF0;         /* from the model's body data; * 1.8 = default radius */
    /* 0xFF4 */ f32 height;         /* model header +0x18 (BtlObjBody_Init copies the four) */
    /* 0xFF8 */ f32 unkFF8;         /* model header +0x1C */
    /* 0xFFC */ f32 centerHeight;   /* model header +0x20 */
    /* 0x1000 */ f32 camSideSlope;       /* model header +0x24 */
} BtlCapiObj;

/* Roster (gBtlChars): only the field read here. */
typedef struct BtlCapiMgr {
    /* 0x00 */ u8 unk0[0x5C];
    /* 0x5C */ f32 clashBias;       /* BtlClashA.bias (clash A state is at roster +0x50) */
} BtlCapiMgr;

s32 BtlCharApi_GetChara(s32 objId);
f32 BtlCharApi_GetHeight(s32 objId);
f32 BtlCharApi_GetBodyUnkFF8(s32 objId);
f32 BtlCharApi_GetCenterHeight(s32 objId);
f32 BtlCharApi_GetCamSideSlope(s32 objId);
s32 BtlCharApi_ObjGetParamFlags0(s32 objId);
s32 BtlCharApi_ObjGetParamFlags(s32 objId);
s32 BtlCharApi_GetAuraType(s32 objId);
f32 BtlCharApi_GetParamUnkC8(s32 objId);
s32 BtlCharApi_GetCostume(s32 objId);
s32 BtlCharApi_HasKiBlastType2(s32 objId);
s32 BtlCharApi_HasKiBlastType3(s32 objId);
s32 BtlCharApi_IsFighter(s32 objId);
s32 BtlCharApi_GetObjIdOfPlayer(s32 player);
s32 BtlCharApi_GetPlayer(s32 objId);
s32 BtlCharApi_GetOpponentObjId(s32 objId);
s32 BtlCharApi_GetPartnerObjId(s32 objId);
s32 BtlCharApi_GetPlayerEffectPack(s32 player, u32 n);
void *BtlCharApi_GetPlayerSuperData(s32 player);
void *BtlCharApi_GetPlayerSkillData(s32 player);
s32 BtlCharApi_GetPlayerCharPack(s32 player);
void BtlCharApi_GetPos(s32 objId, Vec4 *out);
void BtlCharApi_GetBasePos(s32 objId, Vec4 *out);
void BtlCharApi_GetRot(s32 objId, Vec4 *out);
void BtlCharApi_GetModelRot(s32 objId, Vec4 *out);
f32 BtlCharApi_GetYaw(s32 objId);
void BtlCharApi_GetVelocity(s32 objId, Vec4 *out);
void BtlCharApi_GetFrameMove(s32 objId, Vec4 *out);
void BtlCharApi_GetDir(s32 objId, Vec4 *out);
f32 BtlCharApi_GetSpeed(s32 objId);
f32 BtlCharApi_GetFallSpeed(s32 objId);
f32 BtlCharApi_GetGroundY(s32 objId);
f32 BtlCharApi_GetAltitude(s32 objId);
void BtlCharApi_GetNodePos(s32 objId, s32 node, Vec4 *out);
void BtlCharApi_GetNodeLocalPos(s32 objId, s32 node, Vec4 *out);
void BtlCharApi_GetNodeMtx(s32 objId, s32 node, Mtx44 *out);
void BtlCharApi_GetNodeParentMtx(s32 objId, s32 node, Mtx44 *out);
void BtlCharApi_GetNodeRot(s32 objId, s32 node, Vec4 *out);
void BtlCharApi_GetNodeQuat(s32 objId, s32 node, Quat *out);
f32 BtlCharApi_GetNodeBoundSize(s32 objId, s32 node);
f32 BtlCharApi_GetCloseRange(s32 objId);
f32 BtlCharApi_ObjExistsF(s32 objId);
s32 BtlCharApi_IsClose(s32 objId);
s32 BtlCharApi_IsSightBlocked(s32 objId);
s32 BtlCharApi_IsOnGround(s32 objId);
s32 BtlCharApi_IsInWater(s32 objId);
s32 BtlCharApi_IsHidden(s32 objId);
s32 BtlCharApi_IsFrozen(s32 objId);
s32 BtlCharApi_AnyFrozen(void);
s32 BtlCharApi_TestStageBreakA(s32 objId);
s32 BtlCharApi_TestStageBreakB(s32 objId);
f32 BtlCharApi_GetChargeRate(s32 objId);
s32 BtlCharApi_GetActionFxKind(s32 objId);
f32 BtlCharApi_GetClashBias(void);
f32 BtlCharApi_GetBlindRatio(s32 objId);
s32 BtlCharApi_GetRushFinishPhase(s32 objId);
void BtlCharApi_ObjPushYaw(s32 objId, f32 yaw, f32 len, f32 max);
void BtlCharApi_ObjPushDir(s32 objId, Vec4 *dir, f32 len, f32 max);
f32 BtlCharApi_GetRadius(s32 objId);
void BtlCharApi_GetBodyPos(s32 objId, Vec4 *out);
void BtlCharApi_CalcAimDir(s32 objId, s32 node, Vec4 *from, Vec4 *out, f32 minPitch, f32 maxPitch);
void BtlCharApi_CalcAimDir45(s32 objId, Vec4 *from, Vec4 *out);
void BtlCharApi_GetDeflectDir(s32 objId, Vec4 *out);
s32 BtlCharApi_IsLockedOn(s32 objId);
s32 BtlCharApi_IsChargingKiBlast(s32 objId);
s32 BtlCharApi_IsInTechnique(s32 objId);
s32 BtlCharApi_EnteredTechnique(s32 objId);
s32 BtlCharApi_IsRushConnected(s32 objId);
s32 BtlCharApi_IsInRushSequence(s32 objId);
s32 BtlCharApi_IsInSkill(s32 objId);
s32 BtlCharApi_IsSkillStart(s32 objId);
s32 BtlCharApi_IsSkillApplied(s32 objId);
s32 BtlCharApi_IsInClashA(s32 objId);
s32 BtlCharApi_IsInClashBC(s32 objId);
s32 BtlCharApi_IsChargingKi(s32 objId);
s32 BtlCharApi_IsMaxPowerStart(s32 objId);
s32 BtlCharApi_IsChanging(s32 objId);
s32 BtlCharApi_TestAnimFlag10(s32 objId);
s32 BtlCharApi_IsCircleDash(s32 objId);
s32 BtlCharApi_IsModelNew(s32 objId);
s32 BtlCharApi_HasWeaponOut(s32 objId);

#endif

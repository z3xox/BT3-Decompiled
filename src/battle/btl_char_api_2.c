#include "common.h"
#include "battle/btl_char_api_2.h"
#include "battle/btl_capi_b.h"

/*
 * Fighter interface, 0x207020..0x208430: 58 accessors keyed by battle object id (or by nothing), used by the camera,
 * the effect scene and effect modules, the sequence, sound and the AI. See include/battle/btl_char_api_2.h.
 *
 * This is a slice of a larger original object, not a whole one. Evidence for the object's extent:
 *   - the same accessor style (BtlObj_Get / BtlChar_FindByObjId on the first argument, 0 for a non-fighter) starts at
 *     0x204E78, right after the last fighter state handler (0x204E28, which takes the fighter pointer), and runs to
 *     0x209EE8, where the script-control functions keyed by side (BtlCtrl_*, BtlChar_Get(side)) begin;
 *   - the float pool is one run: 0x2FE07C (BtlCharApi_ObjPushDir) .. 0x2FE0D4 (BtlCharApi_TestOppSkillFlags); 0x2FE078 belongs to
 *     BtlAct_CheckRecoveryInput before it and 0x2FE0E4 to BtlAtk_GetId after it;
 *   - calls inside it are compiled as same-file calls (BtlCharApi_IsCamShown -> BtlCharApi_IsInClashA / BtlCharApi_IsInClashBC here,
 *     BtlCharApi_GetTechniqueProgress -> 0x207C60 / 0x207C88 / 0x207CB0, BtlCharApi_FindIncomingBlast -> BtlCharApi_GetPos).
 * So the object starts at 0x204E78 and ends at 0x209EE8 or later (0x20B4A8 if the BtlCtrl functions are part of it).
 * The slice decompiled here uses no float pool entry, string or jump table, so it can be linked as its own file.
 *
 * Callees are named by address; what each does is in the comment next to its declaration (read from its code).
 */

extern BtlCharApiMgr *gBtlChars;

extern s32 BtlChar_GetCount(void);
extern BtlCharApiChr *BtlChar_Get(s32 idx);
extern BtlCharApiChr *BtlChar_FindByObjId(s32 objId);
extern BtlCharApiObj *BtlObj_Get(s32 objId);
extern s32 BtlChar_TestFlag(BtlCharApiChr *chr, s32 flag);
extern void BtlChar_SetHeldFlag(BtlCharApiChr *chr, s32 flag);

extern void Vec4_Copy(Vec4 *dst, Vec4 *src);
extern void Vec4_Sub(Vec4 *dst, Vec4 *a, Vec4 *b);
extern f32 Vec3_Length(Vec4 *v);
extern void Vec4_SetZeroW1(Vec4 *dst);                    /* dst = 0 */
extern void Vec4_SetZero(Vec4 *dst);                    /* dst = 0 */
extern f32 Vec3_Dist(Vec4 *a, Vec4 *b);              /* distance between two points */

extern f32 BtlAnim_GetFrame(BtlCharApiChr *chr);            /* BtlObj_Get(chr->objId)->unkC78 */
extern s32 BtlAnim_GetId(BtlCharApiChr *chr);            /* chr->unk974 */
extern void *BtlAnim_GetFlags(s32 idx);                     /* gBtlChars->unk20[idx], idx < 0x19E */
extern s32 BtlAnim_TestAttr(BtlCharApiChr *chr, u64 mask);  /* 0 while chr->unk990 > 0, else BtlObjAnim_TestEvent on its object */
extern void ChrCam_AddShake(BtlCharApiChr *chr, f32 a, f32 b); /* CamShake_Add(&chr->camShake, a, b) if chr->camShakeOn */
extern s32 ChrCam_IsCutActive(BtlCharApiChr *chr);
extern BtlCharApiVitals *BtlMember_GetActiveGauge(BtlCharApiChr *chr); /* the active member's vitals */
extern s32 BtlReplay_GetViewSide(void);
extern void BtlCharSnd_RequestAt(Vec4 *pos, s32 kind, s32 id, f32 near, f32 far); /* sound request, owner -1 */
extern void BtlCharSnd_PlayOwn(BtlCharApiChr *chr, s32 id);  /* sound request at the fighter: kind side + 2, 200 / 1500 */
extern s32 BtlCharSnd_GetBankMask(s32 idx);                       /* table of words at 0x2EF290 */
extern s32 BtlOpp_GetObjId(BtlCharApiChr *chr);            /* object id of the other side's fighter */
extern f32 BtlUtil_ClampF(f32 v, f32 lo, f32 hi);         /* clamp */
extern BtlCharApiObj *BtlChar_GetObj(BtlCharApiChr *chr); /* BtlObj_Get(chr->objId) */
extern Vec4 *BtlChar_GetPos(BtlCharApiChr *chr);          /* &chr->pos */
extern s32 BtlChar_IsFrozen(BtlCharApiChr *chr);            /* chr->unk1320 > 0 */
extern s32 BtlChar_IsDead(BtlCharApiChr *chr);            /* vitals->hp < 1 */
extern s32 BtlChar_TestMemberUnk70(BtlCharApiChr *chr);            /* vitals->unk30 != 0 */
extern void BtlChar_SetVibration(BtlCharApiChr *chr, f32 power, f32 time);
extern void BtlChar_SetSmallVibration(BtlCharApiChr *chr, f32 time);
extern s32 BtlAct_GetCurrent(BtlCharApiChr *chr);            /* chr->action */
extern s32 BtlAct_IsTechniqueId(s32 action);                    /* action == 0x105 or in 0x106..0x132 */
extern s32 BtlAct_GetCurrentClass(BtlCharApiChr *chr);            /* technique slot of the current action, or -1 */
extern s32 BtlAct_GetMotionLevel(BtlCharApiChr *chr, s32 action);
extern s32 BtlCharApi_IsInRushSequence(s32 objId);                     /* action id in 0x12D..0x12F or 0x139..0x13B */
extern s32 BtlCharApi_IsInClashA(s32 objId);                     /* action id in 0x130..0x132 */
extern s32 BtlCharApi_IsInClashBC(s32 objId);                     /* action id 0xFA or 0xFC */
extern s32 BtlParam_GetUnk84(BtlCharApiChr *chr, u32 n);
extern s32 BtlParam_GetUnk8A(BtlCharApiChr *chr, u32 n);
extern s32 BtlSuper_GetFlags(BtlCharApiChr *chr, s32 slot);  /* attribute word of technique `slot` */
extern s32 BtlSuper_IsThrow(BtlCharApiChr *chr, s32 slot);
extern s32 BtlObjAnim_TestEvent(BtlCharApiObj *obj, u64 mask);
extern s32 BtlObjAnim_GetEventArg(BtlCharApiObj *obj, u64 mask);
extern s32 BtlObjAnim_MaskToNode(s32 bits);
extern s32 BtlObjAnim_QueryEvent(BtlCharApiObj *obj, u64 arg1, s32 arg2, s32 arg3);
extern void BtlObj_SetColorMode(BtlCharApiObj *obj, s32 bit, s32 on);

extern s32 DemoCam_IsActive(void);
extern s32 Battle_IsSplitScreen(void);
extern s32 BattleReplay_IsActive(void);
extern s32 BtlCam_GetDefaultView(void);

/* Whether the object has both bits 0x02 and 0x20 of its flag word. */
s32 BtlCharApi_ObjHasFlags22(s32 objId) {
    BtlCharApiObj *obj = BtlObj_Get(objId);

    if (obj != NULL) {
        return (obj->u.flags64 & 0x22) == 0x22;
    }
    return 0;
}

/* Whether the object has both bits 0x02 and 0x40 of its flag word. */
s32 BtlCharApi_ObjHasFlags42(s32 objId) {
    BtlCharApiObj *obj = BtlObj_Get(objId);

    if (obj != NULL) {
        return (obj->u.flags64 & 0x42) == 0x42;
    }
    return 0;
}

/* Whether any fighter has flag 0x128 (the battle-end check waits while one does). */
s32 BtlCharApi_AnyHasFlag128(void) {
    s32 i;

    if (gBtlChars == NULL) {
        return 0;
    }
    for (i = 0; i < BtlChar_GetCount(); i++) {
        if (BtlChar_TestFlag(BtlChar_Get(i), 0x128)) {
            return 1;
        }
    }
    return 0;
}

/* Whether the fighter's active member has no health left. */
s32 BtlCharApi_IsHpEmpty(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_IsDead(chr);
    }
    return 0;
}

/* Fighter flag 0xA4. */
s32 BtlCharApi_TestFlagA4(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 0xA4);
    }
    return 0;
}

/* Word +0x60 of the fighter's active member block. */
s32 BtlCharApi_GetMemberUnk60(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->unk20;
    }
    return 0;
}

/* Whether the fighter has flag 8 and is in action 0x104. */
s32 BtlCharApi_IsFlag8Action104(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        if (BtlChar_TestFlag(chr, 8) && BtlAct_GetCurrent(chr) == 0x104) {
            return 1;
        }
        return 0;
    }
    return 0;
}

/* Whether the fighter is in action 0x103 or has flag 0xA6. */
s32 BtlCharApi_IsAction103OrFlagA6(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        if (BtlAct_GetCurrent(chr) == 0x103) {
            return 1;
        }
        return BtlChar_TestFlag(chr, 0xA6) != 0;
    }
    return 0;
}

/* Whether word +0x70 of the fighter's active member block is set. */
s32 BtlCharApi_HasMemberUnk70(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_TestMemberUnk70(chr);
    }
    return 0;
}

/* Sets held flag 0xA7 on the fighter. */
void BtlCharApi_SetHeldFlagA7(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        BtlChar_SetHeldFlag(chr, 0xA7);
    }
}

/* Sets held flag 0xA8 on the fighter. */
void BtlCharApi_SetHeldFlagA8(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        BtlChar_SetHeldFlag(chr, 0xA8);
    }
}

/* Sets held flag 0xA9 on the fighter. */
void BtlCharApi_SetHeldFlagA9(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        BtlChar_SetHeldFlag(chr, 0xA9);
    }
}

/* Sets held flag 0xAA on the fighter. */
void BtlCharApi_SetHeldFlagAA(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        BtlChar_SetHeldFlag(chr, 0xAA);
    }
}

/* Sets held flag 0xAB on the fighter (the effect scene does it to the target of a finishing technique). */
void BtlCharApi_SetHeldFlagAB(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        BtlChar_SetHeldFlag(chr, 0xAB);
    }
}

/* Queues a positioned sound with no owner in the fighter manager's 4-entry request list (played by BtlCharSnd_PlayRequests). */
void BtlCharApi_PlaySoundAt(Vec4 *pos, s32 kind, s32 id, f32 near, f32 far) {
    BtlCharSnd_RequestAt(pos, kind, id, near, far);
}

/* Queues sound 0x25 + n or 0x29 + n at the fighter, by the class (0 / 1) of the technique it is performing.
   Declared int with no return statement: the original calls BtlCharSnd_PlayOwn with jal and falls into the epilogue
   instead of tail-calling it, which a void function would not do. No caller uses a result. */
s32 BtlCharApi_PlayTechniqueSound(s32 objId, u32 n) {
    s32 base = -1;
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL && n < 4) {
        switch (BtlAct_GetCurrentClass(chr)) {
        case 0:
            base = 0x25;
            break;
        case 1:
            base = 0x29;
            break;
        }
        if (base >= 0) {
            BtlCharSnd_PlayOwn(chr, base + n);
        }
    }
}

/* Number of live sound handles of a side (4 slots). */
s32 BtlCharApi_GetSoundCount(s32 side) {
    s32 count = 0;
    s32 i;

    BtlCharApiSound *e;

    if (gBtlChars == NULL) {
        return 0;
    }
    e = gBtlChars->sounds[side].slot;
    for (i = 0; i < 4; i++, e++) {
        if (e->handle >= 0) {
            count++;
        }
    }
    return count;
}

/* The n-th live sound of a side: its handle, the table word for its byte +8, its word +4. */
void BtlCharApi_GetSound(s32 side, s32 n, s32 *handle, s32 *out3, s32 *out4) {
    s32 count = 0;
    s32 i;
    BtlCharApiSound *e;

    if (gBtlChars == NULL) {
        return;
    }
    e = gBtlChars->sounds[side].slot;
    for (i = 0; i < 4; i++, e++) {
        if (e->handle >= 0) {
            if (count == n) {
                if (handle != NULL) {
                    *handle = e->handle;
                }
                if (out3 != NULL) {
                    *out3 = BtlCharSnd_GetBankMask(e->unk8);
                }
                if (out4 != NULL) {
                    *out4 = e->unk4;
                }
                return;
            }
            count++;
        }
    }
}

/* Clears bit 3 of the object's mask at +0xB34. */
void BtlCharApi_ObjClearMaskBit3(s32 objId) {
    BtlCharApiObj *obj = BtlObj_Get(objId);

    if (obj != NULL) {
        BtlObj_SetColorMode(obj, 3, 0);
    }
}

/* Sets bit 3 of the object's mask at +0xB34. */
void BtlCharApi_ObjSetMaskBit3(s32 objId) {
    BtlCharApiObj *obj = BtlObj_Get(objId);

    if (obj != NULL) {
        BtlObj_SetColorMode(obj, 3, 1);
    }
}

/* Bit 21 of the object's flag word. */
s32 BtlCharApi_ObjTestFlagBit21(s32 objId) {
    BtlCharApiObj *obj = BtlObj_Get(objId);

    if (obj != NULL) {
        return (obj->u.flags >> 21) & 1;
    }
    return 0;
}

/* Float at fighter +0xE44. */
f32 BtlCharApi_GetUnkE44(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->unkE44;
    }
    return 0.0f;
}

/* Fighter counter +0xE5C as a 0..1 ratio of 3. */
f32 BtlCharApi_GetUnkE5CRatio(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);


    if (chr == NULL) {
        return 0.0f;
    }
    return BtlUtil_ClampF((f32)chr->unkE5C / 3.0f, 0.0f, 1.0f);
}

/* Fighter counter +0xE60 as a 0..1 ratio of 5. */
f32 BtlCharApi_GetUnkE60Ratio(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr == NULL) {
        return 0.0f;
    }
    return BtlUtil_ClampF((f32)chr->unkE60 / 5.0f, 0.0f, 1.0f);
}

/* Sets or clears bit 0x100 of the object's flag word. */
void BtlCharApi_ObjSetFlag100(s32 objId, s32 on) {
    BtlCharApiObj *obj = BtlObj_Get(objId);

    if (obj != NULL) {
        if (on) {
            obj->u.flags |= 0x100;
        } else {
            obj->u.flags &= ~0x100;
        }
    }
}

/* Whether the object has both bits 0x02 and 0x100 of its flag word. */
s32 BtlCharApi_ObjHasFlags102(s32 objId) {
    BtlCharApiObj *obj = BtlObj_Get(objId);

    if (obj != NULL) {
        return (obj->u.flags64 & 0x102) == 0x102;
    }
    return 0;
}

/* Whether both fighters still have health and the target is below half of its maximum. */
s32 BtlCharApi_IsTargetBelowHalfHp(s32 objId, s32 targetId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);
    BtlCharApiChr *target = BtlChar_FindByObjId(targetId);

    s32 result = 0;

    if (chr != NULL && target != NULL) {
        if (BtlChar_IsDead(chr)) {
            return 0;
        }
        if (BtlChar_IsDead(target)) {
            return 0;
        }
        return BtlMember_GetActiveGauge(target)->hp < BtlMember_GetActiveGauge(target)->hpMax / 2;
    }
    return result;
}

/* Whether the fighter's current technique has attribute 0x80000, passes BtlSuper_IsThrow and the target's member word +0x60 is 0. */
s32 BtlCharApi_CanTechniqueFinish(s32 objId, s32 targetId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);
    BtlCharApiChr *target = BtlChar_FindByObjId(targetId);
    s32 slot;
    s32 result = 0;

    if (chr != NULL && target != NULL) {
        if (!BtlAct_IsTechniqueId(BtlAct_GetCurrent(chr))) {
            return 0;
        }
        slot = BtlAct_GetCurrentClass(chr);
        if (!(BtlSuper_GetFlags(chr, slot) & 0x80000)) {
            return 0;
        }
        if (BtlSuper_IsThrow(chr, slot)) {
            return 0;
        }
        return BtlMember_GetActiveGauge(target)->unk20 == 0;
    }
    return result;
}

/* Fighter manager counter +0x1C as a 0..1 ratio of 90. */
f32 BtlCharApi_GetMgrTimerRatio(void) {
    if (gBtlChars == NULL) {
        return 0.0f;
    }
    return BtlUtil_ClampF((f32)gBtlChars->unk1C / 90.0f, 0.0f, 1.0f);
}

/* Starts the block at +0x15D0 (strength, frames) on every fighter within `far` of pos, scaled by closeness. */
void BtlCharApi_RumbleNear(Vec4 *pos, f32 near, f32 far, f32 power, f32 time) {
    Vec4 diff;
    s32 i;
    BtlCharApiChr *chr;
    f32 rate;
    f32 t;

    if (gBtlChars == NULL) {
        return;
    }
    for (i = 0; i < BtlChar_GetCount(); i++) {
        chr = BtlChar_Get(i);
        Vec4_Sub(&diff, BtlChar_GetPos(chr), pos);
        rate = 1.0f - (Vec3_Length(&diff) - near) / (far - near);
        if (rate < 0.0f) {
            continue;
        }
        if (rate > 1.0f) {
            rate = 1.0f;
        }
        t = time * rate;
        BtlChar_SetVibration(chr, power * rate, t);
        if (rate > 0.5f) {
            BtlChar_SetSmallVibration(chr, t);
        }
    }
}

/* Whether any attribute word of the object (for a fighter: unless +0x990 > 0) has a bit of mask. */
s32 BtlCharApi_ObjTestAttr(s32 objId, u64 mask) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);
    BtlCharApiObj *obj;

    if (chr != NULL) {
        return BtlAnim_TestAttr(chr, mask);
    }
    obj = BtlObj_Get(objId);
    if (obj != NULL) {
        return BtlObjAnim_TestEvent(obj, mask);
    }
    return 0;
}

/* OR of the values of the object's attribute words that have a bit of mask. */
s32 BtlCharApi_ObjGetAttrValue(s32 objId, u64 mask) {
    BtlCharApiObj *obj = BtlObj_Get(objId);

    if (obj != NULL) {
        return BtlObjAnim_GetEventArg(obj, mask);
    }
    return 0;
}

/* The same value mapped to a kind code by BtlObjAnim_MaskToNode, 0 when there is none. */
s32 BtlCharApi_ObjGetAttrKind(s32 objId, u64 mask) {
    BtlCharApiObj *obj = BtlObj_Get(objId);
    s32 bits;
    s32 kind;

    if (obj != NULL) {
        bits = BtlObjAnim_GetEventArg(obj, mask);
        kind = 0;
        if (bits != 0) {
            kind = BtlObjAnim_MaskToNode(bits);
        }
        return kind;
    }
    return 0;
}

/* Object float +0xC78, raised by the fighter's +0xEFC entries while it is in actions 0x12D..0x12F / 0x139..0x13B. */
f32 BtlCharApi_GetChargedUnkC78(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);
    f32 value;
    s32 n;
    s32 i;

    if (chr == NULL) {
        return 0.0f;
    }
    value = BtlAnim_GetFrame(chr);
    if (BtlCharApi_IsInRushSequence(objId)) {
        n = BtlAct_GetMotionLevel(chr, BtlAnim_GetId(chr));
        for (i = 0; i < n; i++) {
            value += chr->unkEFC[i] + 1.0f;
        }
    }
    return value;
}

/* Object float +0xC78. */
f32 BtlCharApi_ObjGetUnkC78(s32 objId) {
    BtlCharApiObj *obj = BtlObj_Get(objId);

    if (obj != NULL) {
        return obj->unkC78;
    }
    return 0.0f;
}

/* Object float +0xC80. */
f32 BtlCharApi_ObjGetUnkC80(s32 objId) {
    BtlCharApiObj *obj = BtlObj_Get(objId);

    if (obj != NULL) {
        return obj->unkC80;
    }
    return 0.0f;
}

/* BtlObjAnim_QueryEvent(obj, arg1, 0, arg2) on the object. */
s32 BtlCharApi_ObjQuery24D610(s32 objId, s32 arg1, s32 arg2) {
    BtlCharApiObj *obj = BtlObj_Get(objId);

    if (obj != NULL) {
        return BtlObjAnim_QueryEvent(obj, arg1, 0, arg2);
    }
    return 0;
}

/* Fighter word +0x974. */
s32 BtlCharApi_GetUnk974(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlAnim_GetId(chr);
    }
    return 0;
}

/* The manager's table entry for fighter word +0x974. */
void *BtlCharApi_GetUnk974Data(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlAnim_GetFlags(BtlAnim_GetId(chr));
    }
    return NULL;
}

/* Fighter flag 0x2B. */
s32 BtlCharApi_TestFlag2B(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 0x2B);
    }
    return 0;
}

/* Copies the fighter camera's position and rotation (zeroes them for a non-fighter); returns fighter +0x494. */
s32 BtlCharApi_GetCamPose(s32 objId, Vec4 *pos, Vec4 *rot) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        Vec4_Copy(pos, &chr->camPos);
        Vec4_Copy(rot, &chr->camRot);
        return chr->camUnk494;
    }
    Vec4_SetZeroW1(pos);
    Vec4_SetZero(rot);
    return 0;
}

/* Float at fighter +0x4A0. */
f32 BtlCharApi_GetCamUnk4A0(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->camUnk4A0;
    }
    return 0.0f;
}

/* Fighter flag 0xD3 as 0 / 1: this fighter's camera wants the whole screen. */
s32 BtlCharApi_HasCamPriority(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 0xD3) != 0;
    }
    return 0;
}

/* Shakes the camera of every fighter whose +0x420 point is within `far` of pos, scaled by closeness. */
void BtlCharApi_ShakeCamsNear(Vec4 *pos, f32 near, f32 far, f32 arg3, f32 arg4) {
    s32 i;
    BtlCharApiChr *chr;
    f32 dist;
    f32 rate;

    if (gBtlChars == NULL) {
        return;
    }
    for (i = 0; i < BtlChar_GetCount(); i++) {
        chr = BtlChar_Get(i);
        if (BtlChar_IsFrozen(chr)) {
            continue;
        }
        dist = Vec3_Dist(&chr->camUnk420, pos);
        if (dist < far) {
            rate = BtlUtil_ClampF(1.0f - (dist - near) / (far - near), 0.0f, 1.0f);
            ChrCam_AddShake(chr, arg3 * rate, arg4 * rate);
        }
    }
}

/* Whether this fighter's camera is the one on screen. */
s32 BtlCharApi_IsCamShown(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);
    s32 mine;
    s32 other;

    if (chr == NULL) {
        return 0;
    }
    if (BtlCharApi_IsInClashA(objId)) {
        return 0;
    }
    if (BtlCharApi_IsInClashBC(objId)) {
        return 0;
    }
    if (DemoCam_IsActive()) {
        return 1;
    }
    mine = BtlCharApi_HasCamPriority(objId);
    other = BtlCharApi_HasCamPriority(BtlOpp_GetObjId(chr));
    if (other < mine) {
        return 1;
    }
    if (mine < other) {
        return 0;
    }
    if (Battle_IsSplitScreen()) {
        if (!BattleReplay_IsActive()) {
            return 1;
        }
    }
    return chr->side == BtlCam_GetDefaultView();
}

/* Copies the vector at fighter +0x460. */
void BtlCharApi_GetCamUnk460(s32 objId, Vec4 *out) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        Vec4_Copy(out, &chr->camUnk460);
    }
}

/* Manager word +0x134 while +0x130 is 0, else 0 (the default view during a replay). */
s32 BtlCharApi_GetMgrUnk134(void) {
    return BtlReplay_GetViewSide();
}

/* Whether any fighter passing ChrCam_IsCutActive has flag 0xD3. */
s32 BtlCharApi_AnyCamPriority(void) {
    s32 i;
    BtlCharApiChr *chr;

    if (gBtlChars == NULL) {
        return 0;
    }
    for (i = 0; i < gBtlChars->count; i++) {
        chr = BtlChar_Get(i);
        if (ChrCam_IsCutActive(chr) && BtlChar_TestFlag(chr, 0xD3)) {
            return 1;
        }
    }
    return 0;
}

/* Whether the fighter's input is injected (CPU) instead of read from a pad. */
s32 BtlCharApi_IsInputInjected(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->injectOn;
    }
    return 0;
}

/* Stores the buttons and stick the fighter's input is built from when it is injected (the AI calls this). */
void BtlCharApi_SetInjectedInput(s32 objId, u32 buttons, f32 stickX, f32 stickY) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        chr->injectButtons = buttons;
        chr->injectStickX = stickX;
        chr->injectStickY = stickY;
    }
}

/* Fighter flag 0xF. */
s32 BtlCharApi_TestFlag0F(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 0xF);
    }
    return 0;
}

/* Fighter flag 5. */
s32 BtlCharApi_TestFlag05(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 5);
    }
    return 0;
}

/* Byte 0x84 + n (n < 4) of the fighter's parameter block. */
s32 BtlCharApi_GetParamByte84(s32 objId, u32 n) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlParam_GetUnk84(chr, n);
    }
    return 0;
}

/* Byte 0x8A of the fighter's parameter block. */
s32 BtlCharApi_GetParamByte8A(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlParam_GetUnk8A(chr, 0);
    }
    return 0;
}

/* Byte 0x8D of the fighter's parameter block. */
s32 BtlCharApi_GetParamByte8D(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlParam_GetUnk8A(chr, 3);
    }
    return 0;
}

/* The 64-bit word at fighter +0x1288. */
u64 BtlCharApi_GetUnk1288(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->unk1288;
    }
    return 0;
}

/* 1 if object byte +0xCAD is not the complement of +0xCAC; else 0 with flag 0x60; else whether +0xCAD is >= 0. */
s32 BtlCharApi_CheckUnkCAC(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);
    BtlCharApiObj *obj;

    if (chr == NULL) {
        return 0;
    }
    obj = BtlChar_GetObj(chr);
    if (obj == NULL) {
        return 0;
    }
    if (obj->unkCAD != ~obj->unkCAC) {
        return 1;
    }
    if (BtlChar_TestFlag(chr, 0x60) == 1) {
        return 0;
    }
    return obj->unkCAD >= 0;
}

/* Fighter flag 0x60. */
s32 BtlCharApi_TestFlag60(s32 objId) {
    BtlCharApiChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 0x60);
    }
    return 0;
}


/* ======== merged from src/battle/btl_capi_b.c ======== */


/*
 * Fighter interface, second part, 0x208430..0x20BA80. See include/battle/btl_capi_b.h for the groups.
 *
 * This continues src/battle/btl_char_api_2.c (same accessor style, and the float pool 0x2FE0CC..0x2FE0E0 used here
 * follows that file's run). Callees outside the file are named by address when they have no name yet; what each does
 * is in the comment next to its declaration (read from its code).
 */

#define gBtlChars (*(BtlCapiBMgr* *)&gBtlChars)

#define BtlChar_Get ((BtlCapiBChr *(*)(s32 player))BtlChar_Get)
#define BtlChar_FindByObjId ((BtlCapiBChr *(*)(s32 objId))BtlChar_FindByObjId)
extern BtlCapiBChr *BtlChar_FindBySide(s32 side);
#define BtlChar_GetObj ((BtlCapiBObj *(*)(BtlCapiBChr *chr))BtlChar_GetObj)
#define BtlChar_GetPos ((BtlCapiBPose *(*)(BtlCapiBChr *chr))BtlChar_GetPos)
#define BtlChar_TestFlag ((s32 (*)(BtlCapiBChr *chr, s32 flag))BtlChar_TestFlag)
extern void BtlChar_SetFlag(BtlCapiBChr *chr, s32 flag);
#define BtlChar_SetHeldFlag ((void (*)(BtlCapiBChr *chr, s32 flag))BtlChar_SetHeldFlag)
extern void BtlChar_ClearFlag(BtlCapiBChr *chr, s32 flag);
#define BtlChar_IsDead ((s32 (*)(BtlCapiBChr *chr))BtlChar_IsDead)
#define BtlChar_IsFrozen ((s32 (*)(BtlCapiBChr *chr))BtlChar_IsFrozen)
extern s32 BtlChar_IsStage4Or27(void);
extern s32 BtlUtil_Clamp(s32 v, s32 lo, s32 hi);
extern s32 BtlOpp_GetPlayer(BtlCapiBChr *chr);             /* player index of the opponent */

#define BtlAct_GetCurrent ((s32 (*)(BtlCapiBChr *chr))BtlAct_GetCurrent)            /* action id (+0x948) */
extern s32 BtlAct_IsTechniqueId(s32 action);                  /* action in 0x105..0x132: a technique */
#define BtlAct_GetCurrentClass ((s32 (*)(BtlCapiBChr *chr))BtlAct_GetCurrentClass)       /* technique class of the current action, 0..4 */
extern s32 BtlAct_TestPoweredSkill(BtlCapiBChr *chr, u32 mask);
#define BtlAnim_GetId ((s32 (*)(BtlCapiBChr *chr))BtlAnim_GetId)
#define BtlAnim_GetFlags ((s32 (*)(s32 anim))BtlAnim_GetFlags)

extern BtlCapiBMember *BtlMember_GetActive(BtlCapiBChr *chr);
extern BtlCapiBGauge *BtlMember_GetGauge(BtlCapiBChr *chr, s32 member);
#define BtlMember_GetActiveGauge ((BtlCapiBGauge *(*)(BtlCapiBChr *chr))BtlMember_GetActiveGauge)
extern s32 BtlMember_CountAlive(BtlCapiBChr *chr);
extern s32 BtlMember_GetActiveIndex(BtlCapiBChr *chr);
extern s32 BtlMember_GetSwitchTarget(BtlCapiBChr *chr);
extern s32 BtlMember_SetSwitchTarget(BtlCapiBChr *chr, s32 n);
extern s32 BtlMember_Damage(BtlCapiBChr *chr, s32 amount, s32 flags);
extern void BtlMember_AddHealth(BtlCapiBChr *chr, s32 amount);
extern void BtlMember_AddKi(BtlCapiBChr *chr, s32 amount);
extern s32 BtlMember_DrainKi(BtlCapiBChr *chr, s32 amount);
extern void BtlMember_AddBlast(BtlCapiBChr *chr, s32 amount);
extern void BtlMember_SubBlast(BtlCapiBChr *chr, s32 amount);
extern void BtlMember_AddMaxPower(BtlCapiBChr *chr, s32 amount);
extern s32 BtlMember_HasAbility(BtlCapiBChr *chr, s32 ability);

extern s32 BtlMove_CanFireBlast(BtlCapiBChr *chr, s32 cls, s32 *outCount);
extern s32 BtlMove_IsBlockedByOpponent(BtlCapiBChr *chr);
extern void BtlChange_SetTaken(void);
extern void BtlChange_SetLoaded(void);
extern void BtlPartner_Attach(BtlCapiBChr *chr, s32 slot, s32 objId);

extern BtlCapiBWork *Battle_GetWork(void);
extern s32 Battle_GetMode(void);
extern s32 Rand_Range(s32 n);

extern void Vec4_Copy(Vec4 *dst, Vec4 *src);
extern void Vec3_Sub(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void Vec3_Normalize(Vec4 *dst, Vec4 *src);
extern f32 Vec3_Dot(Vec4 *a, Vec4 *b);
extern f32 Vec3_Length(Vec4 *v);

extern BtlCapiBBlastList *EftHit_GetList(void);             /* the blast list */
extern s32 BtlAct_CanTransform(BtlCapiBChr *chr, u32 slot, s32 needBlast, s32 needAllowed);
extern s32 BtlAct_CanFuse(BtlCapiBChr *chr, s32 slot, s32 needBlast, s32 needAllowed, s32 *partner);
extern s32 BtlAct_CanSwitch(BtlCapiBChr *chr, s32 needGauge, s32 needAllowed);
extern void BtlCharApi_GetPos(s32 objId, Vec4 *out);           /* position of an object */
extern f32 BtlCharApi_GetRadius(s32 objId);                       /* body radius */
extern u32 BtlAtk_GetId(BtlCapiBChr *chr);                /* attack id in use */
extern s32 BtlAtk_GetFlags(BtlCapiBChr *chr);                /* its attribute word */
extern s32 BtlAtk_GetUnk2COf(BtlCapiBChr *chr, s32 level);     /* s8 +0x2C of the record of attack `level` (BtlAtk_GetRecordOf) */
extern s32 BtlParam_GetFlags(BtlCapiBChr *chr);                /* param->flags */
extern s32 BtlParam_GetUnk0(BtlCapiBChr *chr);                /* param->unk0 */
extern s32 BtlParam_GetUnkAC(BtlCapiBChr *chr);                /* param->transformSlot */
extern s32 BtlParam_GetSlotId(BtlCapiBChr *chr, s32 slot);      /* param->transformId[slot] */
extern s32 BtlParam_GetSlotCost(BtlCapiBChr *chr, s32 slot);      /* param->transformCost[slot] * 100000 */
extern s32 BtlParam_CountSlots(BtlCapiBChr *chr);                /* number of transformId[] entries != 0xFF */
extern s32 BtlParam_GetUnkB4(BtlCapiBChr *chr, s32 slot);      /* param->fusionId[slot] */
extern s32 BtlParam_GetCostAE(BtlCapiBChr *chr, s32 slot);      /* param->fusionCost[slot] * 100000 */
extern s32 BtlParam_GetCount80(BtlCapiBChr *chr);                /* param->blastLimit less abilities 0x11 / 0x10 / 0xF */
extern s32 BtlParam_GetUnk82(BtlCapiBChr *chr);                /* param->blastLimitB */
extern s32 BtlParam_GetGaugeB(BtlCapiBChr *chr);                /* param word +0x2C */
extern s32 BtlParam_GetUnk8F(BtlCapiBChr *chr, s32 n);         /* param->unk8F[n] */
extern s32 BtlKiBlast_GetFlagsOfHit(BtlCapiBBlastRec *rec);           /* flag word of the blast's definition */
#define BtlSuper_GetFlags ((s32 (*)(BtlCapiBChr *chr, s32 cls))BtlSuper_GetFlags)       /* skills->flags[cls - 2] */
extern s32 BtlSuper_GetType(BtlCapiBChr *chr, s32 cls);       /* skills->unk13C[cls] */
extern s32 BtlSuper_GetKiCost(BtlCapiBChr *chr, s32 cls);       /* skills->cost[cls], halved with ability 0x2B */
extern s32 BtlSuper_GetPromptRowIndex(BtlCapiBChr *chr, s32 n);         /* skills->unk227[n] */
extern s32 BtlSuper_GetUnk165(BtlCapiBChr *chr, s32 cls);       /* skills->kind[cls] */
extern s32 BtlSkill_GetFlags(BtlCapiBChr *chr, s32 slot);      /* moves->flags[slot] */
extern s32 BtlSkill_GetUnk9E(BtlCapiBChr *chr, s32 slot);      /* moves->kind[slot] */
extern s32 BtlSkill_GetBlastCost(BtlCapiBChr *chr, s32 slot);      /* moves->stock[slot] (-1 with ability 0x15, min 1) * 100000 */

extern s32 BtlCharApi_ObjQuery24D610(s32 objId, s32 arg1, s32 arg2);
extern f32 BtlCharApi_ObjGetUnkC78(s32 objId);
extern f32 BtlCharApi_ObjGetUnkC80(s32 objId);

/* Whether object byte +0xCAD is the complement of +0xCAC. */
s32 BtlCharApi_IsUnkCACPaired(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBObj *obj;

    if (chr != NULL) {
        obj = BtlChar_GetObj(chr);
        if (obj != NULL) {
            return obj->unkCAD == ~obj->unkCAC;
        }
        return 0;
    }
    return 0;
}

/* The fighter's action id. */
s32 BtlCharApi_GetAction(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlAct_GetCurrent(chr);
    }
    return 0;
}

/* Fighter word +0xDE0: blasts fired in the current volley. */
s32 BtlCharApi_GetBlastShots(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->blastShots;
    }
    return 0;
}

/* How many more class-0 blasts the fighter may fire: the smaller of limit - shots and limit - blasts alive. */
s32 BtlCharApi_GetBlastRoom(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    s32 count;
    s32 left;
    s32 room;

    if (chr != NULL) {
        left = BtlParam_GetCount80(chr) - chr->blastShots;
        BtlMove_CanFireBlast(chr, 0, &count);
        room = BtlParam_GetCount80(chr) - count;
        if (left < room) {
            room = left;
        }
        return room;
    }
    return 0;
}

/* Whether the character can fire class-0 blasts at all (limit > 0). */
s32 BtlCharApi_HasBlastLimit(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlParam_GetCount80(chr) > 0;
    }
    return 0;
}

/* The same for class-1 blasts: the smaller of limit - shots and class-1 limit - class-1 blasts alive. */
s32 BtlCharApi_GetBlastRoomB(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    s32 count;
    s32 left;
    s32 room;

    if (chr != NULL) {
        left = BtlParam_GetCount80(chr) - chr->blastShots;
        BtlMove_CanFireBlast(chr, 1, &count);
        room = BtlParam_GetUnk82(chr) - count;
        if (left < room) {
            room = left;
        }
        return room;
    }
    return 0;
}

/* Fighter flag 0x66. */
s32 BtlCharApi_TestFlag66(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 0x66);
    }
    return 0;
}

/* 1 when a blast record is not a threat to the fighter: wrong type for the mode, its own, ignored or already seen. */
s32 BtlCharApi_SkipBlastRec(s32 objId, BtlCapiBBlastRec *rec, s32 mode) {
    if (mode < 2) {
        if (rec->type != 0) {
            return 1;
        }
        if (rec->def == NULL) {
            return 1;
        }
        if (objId == rec->objId) {
            return 1;
        }
    } else {
        if (rec->type != 1) {
            return 1;
        }
        if (rec->src == NULL) {
            return 1;
        }
        if (objId == rec->src->ownerId) {
            return 1;
        }
    }
    if (rec->unk54 == 1) {
        return 1;
    }
    return rec->seen == 1;
}

/* Index of the first blast flying at the fighter and close enough to matter, -1 if none. Mode 0 / 1 look at blasts
   (type 0, reach = 5 frames of travel), mode 2 at type 1 records (reach = 1.9 frames). Mode 1 returns a class instead:
   0 for definition kinds 0, 4, 8, else 2 if bit 0 of the blast's flag word is set, else 1. */
s32 BtlCharApi_FindIncomingBlast(s32 objId, s32 mode) {
    Vec4 pos;
    Vec4 toBlast;
    u64 blastPos[2];
    Vec4 dir;
    Vec4 diff;
    Vec4 step;
    BtlCapiBBlastList *list;
    BtlCapiBBlastRec *rec;
    s32 i;
    f32 dist;
    f32 reach;

    if (mode < 2) {
        reach = 5.0f;
    } else {
        reach = 1.9f;
    }
    BtlCharApi_GetPos(objId, &pos);
    list = EftHit_GetList();
    for (i = 0; i < list->count; i++) {
        rec = &list->rec[i];
        if (BtlCharApi_SkipBlastRec(objId, rec, mode) == 1) {
            continue;
        }
        blastPos[0] = rec->pos[0];
        blastPos[1] = rec->pos[1];
        Vec3_Sub(&toBlast, (Vec4 *)rec->pos, &pos);
        Vec3_Normalize(&toBlast, &toBlast);
        Vec3_Sub(&step, (Vec4 *)rec->pos, &rec->prevPos);
        Vec3_Normalize(&dir, &step);
        if (0.0f < Vec3_Dot(&toBlast, &dir)) {
            continue;
        }
        Vec3_Sub(&diff, (Vec4 *)blastPos, &pos);
        dist = Vec3_Length(&diff) - BtlCharApi_GetRadius(objId) - 10.0f;
        if (Vec3_Length(&step) * reach < dist) {
            continue;
        }
        if (mode != 1) {
            return i;
        }
        switch (rec->def->kind) {
        case 0:
        case 4:
        case 8:
            return 0;
        }
        return (BtlKiBlast_GetFlagsOfHit(rec) & 1) ? 2 : 1;
    }
    return -1;
}

/* 1 when some blast that is not the fighter's own is level with it or moving away from it (no caller). */
s32 BtlCharApi_IsBlastPassing(s32 objId) {
    Vec4 pos;
    Vec4 toBlast;
    Vec4 dir;
    Vec4 step;
    BtlCapiBBlastList *list;
    BtlCapiBBlastRec *rec;
    s32 i;

    list = EftHit_GetList();
    BtlCharApi_GetPos(objId, &pos);
    for (i = 0; i < list->count; i++) {
        rec = &list->rec[i];
        if (rec->type == 0) {
            if (objId == rec->def->ownerId) {
                continue;
            }
        } else {
            if (objId == rec->src->ownerId) {
                continue;
            }
        }
        if (rec->unk54 == 1) {
            continue;
        }
        Vec3_Sub(&toBlast, (Vec4 *)rec->pos, &pos);
        Vec3_Normalize(&toBlast, &toBlast);
        Vec3_Sub(&step, (Vec4 *)rec->pos, &rec->prevPos);
        Vec3_Normalize(&dir, &step);
        if (!(0.0f < Vec3_Dot(&toBlast, &dir))) {
            return 1;
        }
    }
    return 0;
}

/* Marks the blast BtlCharApi_FindIncomingBlast(objId, 0) finds as seen, so it is not reported again. */
void BtlCharApi_MarkIncomingBlast(s32 objId) {
    BtlCapiBBlastList *list = EftHit_GetList();
    s32 i = BtlCharApi_FindIncomingBlast(objId, 0);
    BtlCapiBBlastRec *rec;

    if (i >= 0) {
        rec = list->rec;
        rec += i;
        rec->seen = 1;
    }
}

/* Progress of the technique the fighter is performing: fighter +0x1294 when it is >= 0, else
   (BtlCharApi_ObjQuery24D610(objId, 1, 1) - object +0xC78) / object +0xC80; -1 when there is none. */
f32 BtlCharApi_GetTechniqueProgress(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    s32 frame;
    f32 start;
    f32 len;

    if (chr == NULL) {
        return 0.0f;
    }
    if (chr->unk1294 >= 0) {
        return chr->unk1294;
    }
    frame = BtlCharApi_ObjQuery24D610(objId, 1, 1);
    if (frame < 0) {
        return -1.0f;
    }
    start = BtlCharApi_ObjGetUnkC78(objId);
    len = BtlCharApi_ObjGetUnkC80(objId);
    if (0.01f < len) {
        return ((f32)frame - start) / len;
    }
    return -1.0f;
}

/* The fighter's technique table (battle object +0x92C). */
BtlCapiBSkills *BtlCharApi_GetSkillTable(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBObj *obj;

    if (chr != NULL) {
        obj = BtlChar_GetObj(chr);
        if (obj != NULL) {
            return obj->skills;
        }
        return NULL;
    }
    return NULL;
}

/* The fighter's move table (battle object +0x930). */
BtlCapiBMoves *BtlCharApi_GetMoveTable(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBObj *obj;

    if (chr != NULL) {
        obj = BtlChar_GetObj(chr);
        if (obj != NULL) {
            return obj->moves;
        }
        return NULL;
    }
    return NULL;
}

/* Attribute word of the attack in use (attack id below 0xA3), else 0. */
s32 BtlCharApi_GetAttackAttr(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        if (BtlAtk_GetId(chr) < 0xA3) {
            return BtlAtk_GetFlags(chr);
        }
        return 0;
    }
    return 0;
}

/* Health of the active member. */
s32 BtlCharApi_GetHp(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->hp;
    }
    return 0;
}

/* Its maximum. */
s32 BtlCharApi_GetHpMax(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->hpMax;
    }
    return 0;
}

/* Ki of the active member (no caller). */
s32 BtlCharApi_GetKi(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->ki;
    }
    return 0;
}

/* Blast stock of the active member (no caller). */
s32 BtlCharApi_GetBlast(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->blast;
    }
    return 0;
}

/* Its maximum (no caller). */
s32 BtlCharApi_GetBlastMax(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->blastMax;
    }
    return 0;
}

/* Gauge +0x1C of the active member (the powered-up mode's timer). */
s32 BtlCharApi_GetMaxPower(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->maxPower;
    }
    return 0;
}

/* The side's switch gauge, fighter +0x99C (no caller). */
s32 BtlCharApi_GetSwitchGauge(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->switchGauge;
    }
    return 0;
}

/* BtlMove_IsBlockedByOpponent of the fighter (no caller). */
s32 BtlCharApi_IsBlockedByOpponent(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMove_IsBlockedByOpponent(chr);
    }
    return 0;
}

/* Bit 0x80 of the first word of the character's parameter block. */
s32 BtlCharApi_HasParamBit80(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return (BtlParam_GetUnk0(chr) >> 7) & 1;
    }
    return 0;
}

/* Whether the fighter can transform now into the form of its parameter block's AI slot. */
s32 BtlCharApi_CanTransform(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    s32 slot;

    if (chr != NULL) {
        slot = BtlParam_GetUnkAC(chr);
        if (slot < 0) {
            return 0;
        }
        return BtlAct_CanTransform(chr, slot, 1, 1) != 0;
    }
    return 0;
}

/* Whether the fighter can fuse now with any of its three fusion slots. */
s32 BtlCharApi_CanFuse(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    s32 i;

    if (chr == NULL) {
        return 0;
    }
    for (i = 0; i < 3; i++) {
        if (BtlAct_CanFuse(chr, i, 1, 1, NULL)) {
            return 1;
        }
    }
    return 0;
}

/* Whether the fighter can switch to another team member now. */
s32 BtlCharApi_CanSwitch(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlAct_CanSwitch(chr, 1, 1) != 0;
    }
    return 0;
}

/* CPU level of the active member. */
s32 BtlCharApi_GetCpuLevel(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMember_GetActive(chr)->cpuLevel;
    }
    return 0;
}

/* AI type of the active member. */
s32 BtlCharApi_GetAiType(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMember_GetActive(chr)->aiType;
    }
    return 0;
}

/* Whether fighter word +0x1070 is not -30. */
s32 BtlCharApi_IsUnk1070Set(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->unk1070 != -30;
    }
    return 0;
}

/* Blast stock the AI slot's transformation costs; 0 when the character has none. */
s32 BtlCharApi_GetTransformCost(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    s32 slot;

    if (chr != NULL) {
        if (BtlParam_CountSlots(chr) != 0) {
            slot = BtlParam_GetUnkAC(chr);
            if (slot == -1) {
                return 0;
            }
            return BtlParam_GetSlotCost(chr, slot);
        }
        return 0;
    }
    return 0;
}

/* Picks a fusion slot the side can pay for: the only one, a random one of two, or of three slot 0 / 1 / 2 with
   35 / 20 / 45 %; -1 if none (no caller). Draws Rand_Range(100) always and Rand_Range(2) for two candidates. */
s32 BtlCharApi_PickFusionSlot(s32 objId) {
    s32 list[3];
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    s32 n = 0;
    s32 blast = BtlSide_GetBlast(objId);
    s32 r = Rand_Range(100);
    s32 i;
    s32 cost;

    if (chr == NULL) {
        return 0;
    }
    for (i = 0; i < 3; i++) {
        cost = BtlParam_GetCostAE(chr, i);
        if (cost != 0 && !(blast < cost)) {
            list[n] = i;
            n++;
        }
    }
    switch (n) {
    case 1:
        return list[0];
    case 0:
        return -1;
    case 2:
        return list[Rand_Range(2)];
    case 3:
        if (r < 35) {
            return 0;
        }
        return r < 55 ? 1 : 2;
    }
    return -1;
}

/* Picks a transformation slot the side can pay for (slot 3 also counts with cost 0, and is then taken with 2 %);
   -1 if none (no caller). Draws Rand_Range(100) always and Rand_Range(2) for two candidates without slot 3. */
s32 BtlCharApi_PickTransformSlot(s32 objId) {
    s32 list[4];
    s32 n = 0;
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    s32 has3 = 0;
    s32 blast = BtlSide_GetBlast(objId);
    s32 r = Rand_Range(100);
    s32 i;
    s32 cost;

    if (chr == NULL) {
        return 0;
    }
    for (i = 0; i < 4; i++) {
        cost = BtlParam_GetSlotCost(chr, i);
        if (cost == 0 && i != 3) {
            continue;
        }
        if (blast < cost) {
            continue;
        }
        list[n] = i;
        if (i == 3) {
            has3 = 1;
        }
        n++;
    }
    switch (n) {
    case 0:
        break;
    case 1:
        return list[0];
    case 2:
        if (has3) {
            if (r < 2) {
                return 3;
            }
            return list[0];
        }
        return list[Rand_Range(2)];
    case 3:
        if (has3) {
            if (r < 2) {
                return 3;
            }
            if (r < 60) {
                return list[0];
            }
            return list[1];
        }
        if (r < 60) {
            return list[0];
        }
        if (r < 90) {
            return list[1];
        }
        return list[2];
    case 4:
        if (has3) {
            if (r < 2) {
                return 3;
            }
            if (r < 60) {
                return list[0];
            }
            if (r < 90) {
                return list[1];
            }
            return list[2];
        }
        break;
    }
    return -1;
}

/* Whether the fighter is transforming or fusing (action 0xEC..0xF2). */
s32 BtlCharApi_IsChangingForm(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    s32 action;

    if (chr != NULL) {
        action = BtlAct_GetCurrent(chr);
        switch (action) {
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
        case 0xF0:
        case 0xF1:
        case 0xF2:
            return 1;
        }
        return 0;
    }
    return 0;
}

/* Word +0x14 of the character's parameter block. */
s32 BtlCharApi_GetParamUnk14(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_GetObj(chr)->param->unk14;
    }
    return 0;
}

/* Word +0x10 of the character's parameter block (its flags). */
s32 BtlCharApi_GetParamFlags(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_GetObj(chr)->param->flags;
    }
    return 0;
}

/* Word +0x18 of the character's parameter block. */
s32 BtlCharApi_GetParamUnk18(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_GetObj(chr)->param->unk18;
    }
    return 0;
}

/* Number of team members of the fighter's side (no caller). */
s32 BtlCharApi_GetMemberCount(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->memberCount;
    }
    return 0;
}

/* Index of the member that is fighting (no caller). */
s32 BtlCharApi_GetActiveMember(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMember_GetActiveIndex(chr);
    }
    return 0;
}

/* Index of the member a switch would bring in (no caller). */
s32 BtlCharApi_GetSwitchTarget(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlMember_GetSwitchTarget(chr);
    }
    return 0;
}

/* Health of a member as a percentage, at least 1 while it has any (no caller). */
s32 BtlCharApi_GetMemberHpPercent(s32 objId, s32 member) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBGauge *g;
    s32 pct;

    if (chr != NULL) {
        g = BtlMember_GetGauge(chr, member);
        if (g->hpMax > 0) {
            pct = (f32)g->hp / (f32)g->hpMax * 100.0f;
            if (pct <= 0 && g->hp > 0) {
                pct = 1;
            }
            return pct;
        }
        return 0;
    }
    return 0;
}

/* Ki of a member as a percentage, at least 1 while it has any (no caller). */
s32 BtlCharApi_GetMemberKiPercent(s32 objId, s32 member) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBGauge *g;
    s32 pct;

    if (chr != NULL) {
        g = BtlMember_GetGauge(chr, member);
        if (g->kiMax > 0) {
            pct = (f32)g->ki / (f32)g->kiMax * 100.0f;
            if (pct <= 0 && g->ki > 0) {
                pct = 1;
            }
            return pct;
        }
        return 0;
    }
    return 0;
}

/* Kind byte of the technique the opponent is performing, -1 if it is not in a technique action. */
s32 BtlCharApi_GetOppSkillKind(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBChr *opp;

    if (chr == NULL) {
        return -1;
    }
    opp = BtlChar_Get(BtlOpp_GetPlayer(chr));
    if (!BtlAct_IsTechniqueId(BtlAct_GetCurrent(opp))) {
        return -1;
    }
    return BtlSuper_GetUnk165(opp, BtlAct_GetCurrentClass(opp));
}

/* Bit 4 of the flag word of the technique the opponent is performing. */
s32 BtlCharApi_IsOppSkillFlag4(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBChr *opp;

    if (chr != NULL) {
        opp = BtlChar_Get(BtlOpp_GetPlayer(chr));
        if (BtlAct_IsTechniqueId(BtlAct_GetCurrent(opp))) {
            return (BtlSuper_GetFlags(opp, BtlAct_GetCurrentClass(opp)) >> 2) & 1;
        }
        return 0;
    }
    return 0;
}

/* Whether the opponent's technique has flag bit 1, or bit 2 while this fighter's +0xE44 is above 0.9, or byte
   +0x13C of its class is 1 (meaning not known; the AI's condition code reads it). */
s32 BtlCharApi_TestOppSkillFlags(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBChr *opp;
    s32 cls;
    s32 flags;
    s32 unk;

    if (chr != NULL) {
        opp = BtlChar_Get(BtlOpp_GetPlayer(chr));
        if (BtlAct_IsTechniqueId(BtlAct_GetCurrent(opp))) {
            cls = BtlAct_GetCurrentClass(opp);
            flags = BtlSuper_GetFlags(opp, cls);
            unk = BtlSuper_GetType(opp, cls);
            if (flags & 1) {
                return 1;
            }
            if ((flags & 2) && 0.9f < chr->unkE44) {
                return 1;
            }
            return unk == 1;
        }
        return 0;
    }
    return 0;
}

/* Class (0..4) of the technique the opponent is performing, -1 if none. */
s32 BtlCharApi_GetOppSkillClass(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBChr *opp;

    if (chr == NULL) {
        return -1;
    }
    opp = BtlChar_Get(BtlOpp_GetPlayer(chr));
    if (!BtlAct_IsTechniqueId(BtlAct_GetCurrent(opp))) {
        return -1;
    }
    return BtlAct_GetCurrentClass(opp);
}

/* Kind byte of the move the opponent is performing (action 0xFD..0x102), -1 if none. */
s32 BtlCharApi_GetOppMoveKind(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBChr *opp;

    if (chr == NULL) {
        return -1;
    }
    opp = BtlChar_Get(BtlOpp_GetPlayer(chr));
    if ((u32)(BtlAct_GetCurrent(opp) - 0xFD) >= 6) {
        return -1;
    }
    return BtlSkill_GetUnk9E(opp, BtlAct_GetCurrentClass(opp));
}

/* Fighter word +0xE50 (button presses counted in clashes B and C). */
s32 BtlCharApi_GetClashCountB(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->clashCountB;
    }
    return 0;
}

/* Fighter word +0xE4C (the same for clash A). */
s32 BtlCharApi_GetClashCountA(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->clashCountA;
    }
    return 0;
}

/* Whether the effect of move slot 0 / 1 is running: its automatic evasions are left, or its modifier is active. */
s32 BtlCharApi_IsMoveSlotActive(s32 objId, u32 slot) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    s32 n;

    if (chr == NULL) {
        return 0;
    }
    if (slot >= 2) {
        return 0;
    }
    n = slot != 0;
    if (chr->dodgeKind == slot) {
        if (chr->dodges > 0) {
            return 1;
        }
        if (chr->dodgesB > 0) {
            return 1;
        }
    }
    return chr->slotOn[n];
}

/* Bit 0x100 of the flag word of move slot 0 / 1. */
s32 BtlCharApi_IsMoveFlag100(s32 objId, u32 slot) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr == NULL) {
        return 0;
    }
    if (slot >= 2) {
        return 0;
    }
    return (BtlSkill_GetFlags(chr, slot) >> 8) & 1;
}

/* Fighter word +0xFE0: frames of stun left. */
s32 BtlCharApi_GetStunTimer(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->stunTimer;
    }
    return 0;
}

/* Buttons the open prompt (fighter +0x1594 >= 2) wants, as bits of the fighter's button word: 4 with flag 0xA3, else
   0x08 plus the direction bit 0x10 / 0x20 / 0x40 / 0x80 of the prompt's entry; 0 with flag 0xA2 or no prompt. */
s32 BtlCharApi_GetPromptButtons(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBPrompt *p;

    if (chr == NULL) {
        return 0;
    }
    if (chr->switchPrompt < 2) {
        return 0;
    }
    p = &gBtlChars->prompts[BtlSuper_GetPromptRowIndex(chr, chr->switchPrompt)];
    if (BtlChar_TestFlag(chr, 0xA2)) {
        return 0;
    }
    if (BtlChar_TestFlag(chr, 0xA3)) {
        return 4;
    }
    switch (p->button) {
    case 0:
        return 0x18;
    case 1:
        return 0x28;
    case 2:
        return 0x48;
    case 3:
        return 0x88;
    }
    return 0;
}

/* Fighter word +0x1290 in story mode (Battle_GetMode() == 1), else 0. */
s32 BtlCharApi_GetUnk1290(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        if (Battle_GetMode() != 1) {
            return 0;
        }
        return chr->unk1290;
    }
    return 0;
}

/* Whether fighter word +0x106C is below -29. */
s32 BtlCharApi_IsUnk106CLow(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->unk106C < -29;
    }
    return 0;
}

/* Which of the fighter's attacks 0..4 is the first strong enough for the opponent's armour. An armour level is
   built from both characters' parameter flags 4 / 8, powered skills (0x40 on the opponent, 0x100 on the fighter),
   abilities (0x46 on the opponent; 0x47, or 0x70 with flag 6, on the fighter) and the opponent's +0xE14 timer; the
   first attack record whose byte +0x2C is not below it gives 0 (attacks 0, 1) or its id - 1; 4 when none is. */
s32 BtlCharApi_GetArmorBreakLevel(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    BtlCapiBChr *opp;
    s32 level;
    s32 i;

    if (chr == NULL) {
        return 4;
    }
    opp = BtlChar_Get(BtlOpp_GetPlayer(chr));
    level = 0;
    if (BtlParam_GetFlags(opp) & 4) {
        level = 1;
    }
    if (BtlParam_GetFlags(opp) & 8) {
        level--;
    }
    if (BtlParam_GetFlags(chr) & 4) {
        level--;
    }
    if (BtlParam_GetFlags(chr) & 8) {
        level++;
    }
    if (BtlAct_TestPoweredSkill(opp, 0x40)) {
        level++;
    }
    level += opp->unkE14 > 0;
    if (BtlMember_HasAbility(opp, 0x46)) {
        level++;
    }
    if (BtlAct_TestPoweredSkill(chr, 0x100)) {
        level--;
    }
    if (BtlMember_HasAbility(chr, 0x47)) {
        level--;
    } else if (BtlMember_HasAbility(chr, 0x70)) {
        if (BtlChar_TestFlag(chr, 6)) {
            level--;
        }
    }
    for (i = 0; i < 5; i++) {
        if (level - BtlAtk_GetUnk2COf(chr, i) <= 0) {
            if (i < 2) {
                return 0;
            }
            return i - 1;
        }
    }
    return 4;
}

/* Byte +2 of the character's parameter block. */
s32 BtlCharApi_GetParamByte2(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_GetObj(chr)->param->unk2;
    }
    return 0;
}

/* Float at fighter +0xE44 (a second copy of BtlCharApi_GetUnkE44). */
f32 BtlCharApi_GetUnkE44B(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->unkE44;
    }
    return 0.0f;
}

/* Fighter +0xD74 minus +0xD70, or -1 when +0xD74 is 0 (no caller). */
s32 BtlCharApi_GetUnkD74Diff(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        if (chr->unkD74 == 0) {
            return -1;
        }
        return chr->unkD74 - chr->unkD70;
    }
    return 0;
}

/* Whether the fighter's animation has flag 0x2000 or 0x800 and not 0x8000. */
s32 BtlCharApi_IsAnimFlag2800(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);
    s32 flags;

    if (chr != NULL) {
        flags = BtlAnim_GetFlags(BtlAnim_GetId(chr));
        if (!(flags & 0x2800)) {
            return 0;
        }
        if (flags & 0x8000) {
            return 0;
        }
        return 1;
    }
    return 0;
}

/* Top bit of pose byte +0xD0 (fighter +0xE0); with always == 0 only on stages 4 and 27. */
s32 BtlCharApi_TestPoseBit80(s32 objId, s32 always) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr == NULL) {
        return 0;
    }
    if (always == 0 && !BtlChar_IsStage4Or27()) {
        return 0;
    }
    return BtlChar_GetPos(chr)->unkD0_7;
}

/* Fighter flag 0x98. */
s32 BtlCharApi_TestFlag98(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 0x98);
    }
    return 0;
}

/* Fighter flag 0xBE (out of ki: the state that leads to action 0xEA). */
s32 BtlCharApi_TestFlagBE(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 0xBE);
    }
    return 0;
}

/* Fighter word +0xE40. */
s32 BtlCharApi_GetUnkE40(s32 objId) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return chr->unkE40;
    }
    return 0;
}

/* Byte 0x8F + n of the character's parameter block. */
s32 BtlCharApi_GetParamByte8F(s32 objId, s32 n) {
    BtlCapiBChr *chr = BtlChar_FindByObjId(objId);

    if (chr != NULL) {
        return BtlParam_GetUnk8F(chr, n);
    }
    return 0;
}

/* Sequence poses. Asks for the entrance pose: one-frame flag 0xEF forces action 1 (animations 0x180 -> 0x181). */
void BtlCtrl_StartEntrance(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_SetFlag(chr, 0xEF);
    }
}

/* Ends the entrance pose: flag 0xF0 sends action 1 to action 0xB. */
void BtlCtrl_EndEntrance(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_SetFlag(chr, 0xF0);
    }
}

/* Asks for the win pose: flag 0xF1 forces action 2 (animations 0x182 -> 0x183). */
void BtlCtrl_StartWinPose(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_SetFlag(chr, 0xF1);
    }
}

/* Asks for the lose pose: flag 0xF2 forces action 3 (animation 0x184). */
void BtlCtrl_StartLosePose(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_SetFlag(chr, 0xF2);
    }
}

/* Whether the pose has settled: animation 0x181 or 0x183 (the loops), or 0x184 played to its end (flag 0x31). */
s32 BtlCtrl_IsPoseReached(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr == NULL) {
        return 0;
    }
    switch (BtlAnim_GetId(chr)) {
    case 0x181:
    case 0x183:
        return 1;
    case 0x184:
        if (BtlChar_TestFlag(chr, 0x31)) {
            return 1;
        }
        break;
    }
    return 0;
}

/* Scripted motion: stores the animation, blend time and loop mode of action 4 and holds flag 0xFA, which makes the
   fighter enter (or restart) action 4. */
void BtlCtrl_PlayMotion(s32 player, s32 motion, s32 loop, f32 blend) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        chr->motion = motion;
        chr->motionBlend = blend;
        chr->motionLoop = loop;
        BtlChar_SetHeldFlag(chr, 0xFA);
    }
}

/* Stores a position to warp to and holds flag 0xFC (taken by the placement code). */
void BtlCtrl_SetPos(s32 player, Vec4 *pos) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        Vec4_Copy(&chr->warpPos, pos);
        BtlChar_SetHeldFlag(chr, 0xFC);
    }
}

/* Copies the fighter's position (no caller). */
void BtlCtrl_GetPos(s32 player, Vec4 *out) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        Vec4_Copy(out, &BtlChar_GetPos(chr)->pos);
    }
}

/* Stores a rotation to take and holds flag 0xFD. */
void BtlCtrl_SetRot(s32 player, Vec4 *rot) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        Vec4_Copy(&chr->warpRot, rot);
        BtlChar_SetHeldFlag(chr, 0xFD);
    }
}

/* Copies the fighter's rotation (no caller). */
void BtlCtrl_GetRot(s32 player, Vec4 *out) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        Vec4_Copy(out, &BtlChar_GetPos(chr)->rot);
    }
}

/* Index of the side's team member that is fighting. */
s32 BtlCtrl_GetActiveMember(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        return BtlMember_GetActiveIndex(chr);
    }
    return 0;
}

/* Whether the fighter is in action 4, the scripted motion. */
s32 BtlCtrl_IsAction4(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        return BtlAct_GetCurrent(chr) == 4;
    }
    return 0;
}

/* 1 while the scripted motion has NOT reached its end: always for a looping one, else while flag 0x31 is clear. */
s32 BtlCtrl_IsMotionPlaying(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        if (chr->motionLoop == 1) {
            return 1;
        }
        return BtlChar_TestFlag(chr, 0x31) == 0;
    }
    return 0;
}

/* Ends the scripted motion: drops the request flag 0xFA and raises 0xFB, on which action 4 leaves. */
void BtlCtrl_StopMotion(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_ClearFlag(chr, 0xFA);
        BtlChar_SetFlag(chr, 0xFB);
    }
}

/* Holds flag 0xFE and drops 0xFF: action 4 then sets effect request 0x13 every frame (aura at full level). */
void BtlCtrl_SetAuraOn(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_SetHeldFlag(chr, 0xFE);
        BtlChar_ClearFlag(chr, 0xFF);
    }
}

/* Drops flag 0xFE and holds 0xFF: action 4 then sets effect request 0x14 every frame (aura off). */
void BtlCtrl_SetAuraOff(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_ClearFlag(chr, 0xFE);
        BtlChar_SetHeldFlag(chr, 0xFF);
    }
}

/* Holds or drops flag 0x101: while held, action 4 sets effect request 7 (keeps the charge effect) and plays common
   sound BtlParam_GetChargeLoopSound every frame. Declared int with no return statement: the original does not tail-call. */
s32 BtlCtrl_SetChargeFx(s32 player, s32 on) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        if (on) {
            BtlChar_SetHeldFlag(chr, 0x101);
        } else {
            BtlChar_ClearFlag(chr, 0x101);
        }
    }
}

/* Holds flag 0x102: action 4 then drops 0x101 / 0x102 and sets effect requests 7 and 8 (the burst) with sound
   BtlParam_GetMaxPowerSound, once. */
void BtlCtrl_BurstChargeFx(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_SetHeldFlag(chr, 0x102);
    }
}

/* Drops flag 0x100. While it is held action 4 raises fighter flag 0x30, which clears bit 2 of the object's flags
   (the same flag the animation events 0x2000000 / 0x4000000 set and clear: the model is hidden). */
void BtlCtrl_ClearHidden(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_ClearFlag(chr, 0x100);
    }
}

/* Holds flag 0x100. */
void BtlCtrl_SetHidden(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_SetHeldFlag(chr, 0x100);
    }
}

/* Holds flag 0x103 + member: the fighter update re-reads that member's bonus levels from the setup. */
void BtlCtrl_ReloadMemberBonus(s32 player, u32 member) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL && member < 5) {
        BtlChar_SetHeldFlag(chr, member + 0x103);
    }
}

/* Holds flag 0x108 + member: the fighter update re-reads that member's ability words. */
void BtlCtrl_ReloadMemberAbilities(s32 player, u32 member) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL && member < 5) {
        BtlChar_SetHeldFlag(chr, member + 0x108);
    }
}

/* Holds flag 0x10D. */
void BtlCtrl_SetFlag10D(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_SetHeldFlag(chr, 0x10D);
    }
}

/* Drops flag 0x10D. */
void BtlCtrl_ClearFlag10D(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlChar_ClearFlag(chr, 0x10D);
    }
}

/* Adds percent % of a member's maximum to its health. The active member heals through BtlMember_AddHealth or is
   damaged through BtlMember_Damage (flags 0x42B: no defense, exact, no combo hit, no timer, cannot kill); a benched
   member is written directly. Int with no return statement, like the next two Add functions and SetPowerUp. */
s32 BtlCtrl_AddHp(s32 player, s32 member, f32 percent) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    BtlCapiBGauge *g;
    s32 amount;

    if (chr != NULL) {
        g = BtlMember_GetGauge(chr, member);
        amount = g->hpMax * percent * 0.01f;
        if (member == BtlMember_GetActiveIndex(chr)) {
            if (amount >= 0) {
                BtlMember_AddHealth(chr, amount);
            } else {
                BtlMember_Damage(chr, -amount, 0x42B);
            }
        } else {
            g->hp = BtlUtil_Clamp(g->hp + amount, 0, g->hpMax);
        }
    }
}

/* Raises a member's health to at least percent % of its maximum (at least 1 for a positive percentage). */
void BtlCtrl_RaiseHp(s32 player, s32 member, f32 percent) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    BtlCapiBGauge *g;
    s32 target;
    s32 max;

    if (chr != NULL) {
        g = BtlMember_GetGauge(chr, member);
        max = g->hpMax;
        target = max * percent * 0.01f;
        if (target <= 0 && 0.0f < percent) {
            target = 1;
        }
        g->hp = BtlUtil_Clamp(target, g->hp, max);
    }
}

/* Lowers a member's health to at most percent % of its maximum (at least 1 for a positive percentage). */
void BtlCtrl_LowerHp(s32 player, s32 member, f32 percent) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    BtlCapiBGauge *g;
    s32 target;

    if (chr != NULL) {
        g = BtlMember_GetGauge(chr, member);
        target = g->hpMax * percent * 0.01f;
        if (target <= 0 && 0.0f < percent) {
            target = 1;
        }
        g->hp = BtlUtil_Clamp(target, 0, g->hp);
    }
}

/* Adds bars * 20000 to a member's ki (the active member through BtlMember_AddKi / BtlMember_DrainKi). */
s32 BtlCtrl_AddKi(s32 player, s32 member, s32 bars) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    BtlCapiBGauge *g;
    s32 amount;

    if (chr != NULL) {
        g = BtlMember_GetGauge(chr, member);
        amount = bars * 20000;
        if (member == BtlMember_GetActiveIndex(chr)) {
            if (amount >= 0) {
                BtlMember_AddKi(chr, amount);
            } else {
                BtlMember_DrainKi(chr, -amount);
            }
        } else {
            g->ki = BtlUtil_Clamp(g->ki + amount, 0, g->kiMax);
        }
    }
}

/* Raises a member's ki to at least bars * 20000. */
void BtlCtrl_RaiseKi(s32 player, s32 member, s32 bars) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    BtlCapiBGauge *g;

    if (chr != NULL) {
        g = BtlMember_GetGauge(chr, member);
        g->ki = BtlUtil_Clamp(bars * 20000, g->ki, g->kiMax);
    }
}

/* Lowers a member's ki to at most bars * 20000. */
void BtlCtrl_LowerKi(s32 player, s32 member, s32 bars) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    BtlCapiBGauge *g;

    if (chr != NULL) {
        g = BtlMember_GetGauge(chr, member);
        g->ki = BtlUtil_Clamp(bars * 20000, 0, g->ki);
    }
}

/* Adds stocks * 100000 to a member's blast stock (the active member through BtlMember_AddBlast / SubBlast). */
s32 BtlCtrl_AddBlast(s32 player, s32 member, s32 stocks) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    BtlCapiBGauge *g;
    s32 amount;

    if (chr != NULL) {
        g = BtlMember_GetGauge(chr, member);
        amount = stocks * 100000;
        if (member == BtlMember_GetActiveIndex(chr)) {
            if (amount >= 0) {
                BtlMember_AddBlast(chr, amount);
            } else {
                BtlMember_SubBlast(chr, -amount);
            }
        } else {
            g->blast = BtlUtil_Clamp(g->blast + amount, 0, g->blastMax);
        }
    }
}

/* Raises a member's blast stock to at least stocks * 100000. */
void BtlCtrl_RaiseBlast(s32 player, s32 member, s32 stocks) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    BtlCapiBGauge *g;

    if (chr != NULL) {
        g = BtlMember_GetGauge(chr, member);
        g->blast = BtlUtil_Clamp(stocks * 100000, g->blast, g->blastMax);
    }
}

/* Lowers a member's blast stock to at most stocks * 100000. */
void BtlCtrl_LowerBlast(s32 player, s32 member, s32 stocks) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    BtlCapiBGauge *g;

    if (chr != NULL) {
        g = BtlMember_GetGauge(chr, member);
        g->blast = BtlUtil_Clamp(stocks * 100000, 0, g->blast);
    }
}

/* Turns the powered-up mode on (ki filled, its timer set to 30000, flag 6 held) or off (flag 6 dropped). */
s32 BtlCtrl_SetMaxPower(s32 player, s32 on) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        if (on) {
            BtlMember_AddKi(chr, 100000);
            BtlMember_AddMaxPower(chr, 30000);
            BtlChar_SetHeldFlag(chr, 6);
        } else {
            BtlChar_ClearFlag(chr, 6);
        }
    }
}

/* Makes the fighter transform into character `id`: finds it among its four transformation slots, checks
   BtlAct_CanTransform without the cost, raises the blast stock to the cost and holds flag 0x116 + slot. */
s32 BtlCtrl_Transform(s32 player, s32 id) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    s32 slot;
    s32 i;
    BtlCapiBGauge *g;

    if (chr != NULL) {
        slot = -1;
        for (i = 0; i < 4; i++) {
            if (BtlParam_GetSlotId(chr, i) == id) {
                slot = i;
                break;
            }
        }
        if (slot < 0) {
            return 0;
        }
        if (!BtlAct_CanTransform(chr, slot, 0, 0)) {
            return 0;
        }
        g = BtlMember_GetActiveGauge(chr);
        g->blast = BtlUtil_Clamp(BtlParam_GetSlotCost(chr, slot), g->blast, g->blastMax);
        BtlChar_SetHeldFlag(chr, slot + 0x116);
        BtlChar_ClearFlag(chr, 0xBE);
        chr->stunTimer = 0;
        return 1;
    }
    return 0;
}

/* Makes the fighter fuse into character `id`: the same over its three fusion slots (flag 0x11A + slot); it also
   refills the health of the fighter and of the fusion partner. */
s32 BtlCtrl_Fuse(s32 player, s32 id) {
    s32 partner;
    BtlCapiBChr *chr = BtlChar_Get(player);
    s32 slot;
    s32 i;
    BtlCapiBGauge *g;

    if (chr != NULL) {
        slot = -1;
        for (i = 0; i < 3; i++) {
            if (BtlParam_GetUnkB4(chr, i) == id) {
                slot = i;
                break;
            }
        }
        if (slot < 0) {
            return 0;
        }
        if (!BtlAct_CanFuse(chr, slot, 0, 0, &partner)) {
            return 0;
        }
        g = BtlMember_GetActiveGauge(chr);
        g->blast = BtlUtil_Clamp(BtlParam_GetCostAE(chr, slot), g->blast, g->blastMax);
        BtlChar_SetHeldFlag(chr, slot + 0x11A);
        BtlChar_ClearFlag(chr, 0xBE);
        chr->stunTimer = 0;
        g->hp = g->hpMax;
        BtlMember_GetGauge(chr, partner)->hp = BtlMember_GetGauge(chr, partner)->hpMax;
        return 1;
    }
    return 0;
}

/* Makes the side switch to team member `member`: fills the switch gauge and holds flag 0x11D, or for a fighter
   that is down holds flag 0x10E (which lets story mode replace it). */
s32 BtlCtrl_ChangeMember(s32 player, s32 member) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr == NULL) {
        return 0;
    }
    if (member == BtlMember_GetActiveIndex(chr)) {
        return 0;
    }
    if (!BtlChar_IsDead(chr)) {
        if (!BtlAct_CanSwitch(chr, 0, 0)) {
            return 0;
        }
    }
    if (!BtlMember_SetSwitchTarget(chr, member)) {
        return 0;
    }
    if (!BtlChar_IsDead(chr)) {
        chr->switchGauge = 100000;
        BtlChar_SetHeldFlag(chr, 0x11D);
        BtlChar_ClearFlag(chr, 0xBE);
        chr->stunTimer = 0;
    } else {
        BtlChar_SetHeldFlag(chr, 0x10E);
    }
    return 1;
}

/* Forces action 5 (kind 0, flag 0x110) or action 6 (kind 1, flag 0x111). */
s32 BtlCtrl_ForceFlag11x(s32 player, s32 kind) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    s32 flag;

    if (chr != NULL) {
        switch (kind) {
        case 0:
            flag = 0x110;
            break;
        case 1:
            flag = 0x111;
            break;
        default:
            flag = 0x110;
            break;
        }
        BtlChar_SetHeldFlag(chr, flag);
        BtlChar_ClearFlag(chr, 0xBE);
        chr->stunTimer = 0;
        return 1;
    }
    return 0;
}

/* Makes the fighter use a technique: kind 0 / 1 a move slot (blast stock raised to its cost; lock-on flag 5 if the
   move's flag bit 1 is set), kind 2..4 a skill slot (ki raised to its cost, flags 0x11F and 5; kind 4 also starts the
   powered-up mode with full ki). Holds the input flag 0x120 + kind and 0x11E (pad input off). */
s32 BtlCtrl_UseTechnique(s32 player, s32 kind) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    BtlCapiBGauge *g;

    if (chr == NULL) {
        return 0;
    }
    g = BtlMember_GetActiveGauge(chr);
    switch (kind) {
    case 0:
    case 1:
        g->blast = BtlUtil_Clamp(BtlSkill_GetBlastCost(chr, kind), g->blast, g->blastMax);
        if (BtlSkill_GetFlags(chr, kind) & 1) {
            BtlChar_SetHeldFlag(chr, 5);
        }
        break;
    case 2:
    case 3:
    case 4:
        g->ki = BtlUtil_Clamp(BtlSuper_GetKiCost(chr, kind), g->ki, g->kiMax);
        if (kind == 4) {
            BtlChar_SetHeldFlag(chr, 6);
            g->maxPower = 30000;
            g->ki = g->kiMax;
        }
        BtlChar_SetHeldFlag(chr, 0x11F);
        BtlChar_SetHeldFlag(chr, 5);
        break;
    default:
        return 0;
    }
    BtlChar_SetHeldFlag(chr, kind + 0x120);
    BtlChar_SetHeldFlag(chr, 0x11E);
    BtlChar_ClearFlag(chr, 0xBE);
    chr->stunTimer = 0;
    chr->unkE40 = 0;
    return 1;
}

/* Forces one of the waiting actions 7..10 (flag 0x112 + kind): the fighter stands and faces the opponent. */
s32 BtlCtrl_ForceReaction(s32 player, u32 kind) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr == NULL) {
        return 0;
    }
    if (kind >= 4) {
        return 0;
    }
    BtlChar_SetHeldFlag(chr, kind + 0x112);
    BtlChar_ClearFlag(chr, 0xBE);
    chr->stunTimer = 0;
    return 1;
}

/* Whether the fighter is in an action a script (or the object loader) may interrupt: not a throw / clash / move /
   technique / form change / member switch action. */
s32 BtlCtrl_IsInterruptible(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    s32 action;

    if (chr != NULL) {
        action = BtlAct_GetCurrent(chr);
        switch (action) {
        case 0xB7:
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBE:
        case 0xBF:
        case 0xFA:
        case 0xFB:
        case 0xFC:
        case 0x104:
        case 0x105:
        case 0x130:
        case 0x131:
        case 0x132:
            return 0;
        }
        if ((u32)(action - 0xFD) < 6) {
            return 0;
        }
        if ((u32)(action - 0x106) < 0x36) {
            return 0;
        }
        if ((u32)(action - 0xEC) < 0xE) {
            return 0;
        }
        return 1;
    }
    return 0;
}

/* Whether the fighter can take a forced action now: not frozen, not stunned, no reaction pending, flags 0xAF and
   0xB9 clear, and standing (action 0xB, 0xD, 0xE, 0x36) or, when down, lying (0xD8, 0xEB). */
s32 BtlCtrl_CanAct(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    s32 action;

    if (chr == NULL) {
        return 0;
    }
    if (BtlChar_IsFrozen(chr)) {
        return 0;
    }
    if (chr->stunTimer > 0) {
        return 0;
    }
    if (chr->reactId >= 3) {
        return 0;
    }
    if (BtlChar_TestFlag(chr, 0xAF)) {
        return 0;
    }
    if (BtlChar_TestFlag(chr, 0xB9)) {
        return 0;
    }
    action = BtlAct_GetCurrent(chr);
    switch (action) {
    case 0xB:
    case 0xD:
    case 0xE:
    case 0x36:
        return 1;
    default:
        if (BtlChar_IsDead(chr)) {
            switch (action) {
            case 0xD8:
            case 0xEB:
                return 1;
            }
        }
        break;
    }
    return 0;
}

/* Whether the character-change request being served is of type 0, for this player, and waits for the loader. */
s32 BtlChange_IsPendingType0(s32 player) {
    BtlCapiBChangeQueue *q;

    if (gBtlChars == NULL) {
        return 0;
    }
    q = &gBtlChars->change;
    if (q->cur == NULL) {
        return 0;
    }
    if (q->cur->type != 0) {
        return 0;
    }
    if (q->cur->player != player) {
        return 0;
    }
    return q->state == 1;
}

/* The same for a request of type 1. */
s32 BtlChange_IsPendingType1(s32 player) {
    BtlCapiBChangeQueue *q;
    s32 type;

    if (gBtlChars == NULL) {
        return 0;
    }
    q = &gBtlChars->change;
    if (q->cur == NULL) {
        return 0;
    }
    type = q->cur->type;
    if (type != 1) {
        return 0;
    }
    if (q->cur->player != player) {
        return 0;
    }
    return q->state == type;
}

/* Copies the seven parameter words of the request being served (any pointer may be NULL). */
void BtlChange_GetArgs(s32 *a, s32 *b, s32 *c, s32 *d, s32 *e, s32 *f, s32 *g) {
    BtlCapiBChangeQueue *q = &gBtlChars->change;

    if (q->cur == NULL) {
        return;
    }
    if (a != NULL) {
        *a = q->cur->arg[0];
    }
    if (b != NULL) {
        *b = q->cur->arg[1];
    }
    if (c != NULL) {
        *c = q->cur->arg[2];
    }
    if (d != NULL) {
        *d = q->cur->arg[3];
    }
    if (e != NULL) {
        *e = q->cur->arg[4];
    }
    if (f != NULL) {
        *f = q->cur->arg[5];
    }
    if (g != NULL) {
        *g = q->cur->arg[6];
    }
}

/* The loader has taken the request (BtlChange_SetTaken). */
void BtlChange_NotifyTaken(void) {
    BtlChange_SetTaken();
}

/* The request's files are loaded (BtlChange_SetLoaded). */
void BtlChange_NotifyLoaded(void) {
    BtlChange_SetLoaded();
}

/* Whether the request has reached state 4 and the battle is not paused. */
s32 BtlChange_IsReady(void) {
    BtlCapiBChangeQueue *q = &gBtlChars->change;

    if (q->cur == NULL) {
        return 0;
    }
    if (Battle_GetWork()->flags & 0x100) {
        return 0;
    }
    return q->state == 4;
}

/* Attaches a loaded partner object to the player's fighter (BtlPartner_Attach). */
void BtlCtrl_AttachPartner(s32 player, s32 slot, s32 objId) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        BtlPartner_Attach(chr, slot, objId);
    }
}

/* Icon of bit `bit` of the fighter's button word, for a button prompt: 0x10..0x13 for bits 4..7 (up, down, left,
   right), else by the pad button the fighter's key table gives that bit: 0x1000 (triangle) -> 0, 0x8000 (square)
   -> 1, 0x2000 (circle) -> 2, 0x4000 (cross) -> 3; -1 otherwise. The first argument is the fighter, not an id. */
s32 BtlCharApi_GetButtonIcon(BtlCapiBChr *chr, u32 bit) {
    u32 *mask;

    switch (bit) {
    case 4:
        return 0x10;
    case 5:
        return 0x11;
    case 6:
        return 0x12;
    case 7:
        return 0x13;
    }
    mask = chr->keyMask;
    mask += bit;
    switch (*mask) {
    case 0x1000:
        return 0;
    case 0x2000:
        return 2;
    case 0x4000:
        return 3;
    case 0x8000:
        return 1;
    }
    return -1;
}

/* Health of the side's active member. */
s32 BtlSide_GetHp(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->hp;
    }
    return 0;
}

/* Ki of the side's active member. */
s32 BtlSide_GetKi(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->ki;
    }
    return 0;
}

/* Blast stock of the side's active member. */
s32 BtlSide_GetBlast(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->blast;
    }
    return 0;
}

/* Its maximum. */
s32 BtlSide_GetBlastMax(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->blastMax;
    }
    return 0;
}

/* Gauge +0x1C of the side's active member (the powered-up mode's timer). */
s32 BtlSide_GetMaxPower(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        return BtlMember_GetActiveGauge(chr)->maxPower;
    }
    return 0;
}

/* The side's switch gauge. */
s32 BtlSide_GetSwitchGauge(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        return chr->switchGauge;
    }
    return 0;
}

/* Index of the side's member that is fighting, -1 for no fighter. */
s32 BtlSide_GetActiveMember(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr == NULL) {
        return -1;
    }
    return BtlMember_GetActiveIndex(chr);
}

/* Index of the member a switch would bring in, -1 with fewer than two members alive. */
s32 BtlSide_GetSwitchTarget(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr == NULL) {
        return -1;
    }
    if (BtlMember_CountAlive(chr) < 2) {
        return -1;
    }
    return BtlMember_GetSwitchTarget(chr);
}

/* Number of the side's members that are alive, -1 for no fighter. */
s32 BtlSide_CountAlive(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr == NULL) {
        return -1;
    }
    return BtlMember_CountAlive(chr);
}

/* Health of the member a switch would bring in, 0 with fewer than two alive. */
s32 BtlSide_GetSwitchTargetHp(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        if (BtlMember_CountAlive(chr) < 2) {
            return 0;
        }
        return BtlMember_GetGauge(chr, BtlMember_GetSwitchTarget(chr))->hp;
    }
    return 0;
}

/* Its maximum. */
s32 BtlSide_GetSwitchTargetHpMax(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        if (BtlMember_CountAlive(chr) < 2) {
            return 0;
        }
        return BtlMember_GetGauge(chr, BtlMember_GetSwitchTarget(chr))->hpMax;
    }
    return 0;
}

/* Word +0x2C of the side's character parameter block. */
s32 BtlSide_GetParamUnk2C(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        return BtlParam_GetGaugeB(chr);
    }
    return 0;
}

/* Fighter flag 6: the side is in the powered-up mode. */
s32 BtlSide_IsPoweredUp(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 6);
    }
    return 0;
}

/* Fighter flag 0xBE. */
s32 BtlSide_TestFlagBE(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 0xBE);
    }
    return 0;
}

/* Whether the player's active member has no health left. */
s32 BtlCtrl_IsActiveDead(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        return BtlChar_IsDead(chr);
    }
    return 0;
}

/* Whether every member of the player's team has no health left. */
s32 BtlCtrl_IsTeamDead(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);
    s32 dead = 1;
    s32 i;

    if (chr != NULL) {
        for (i = 0; i < chr->memberCount; i++) {
            if (BtlMember_GetGauge(chr, i)->hp > 0) {
                dead = 0;
                break;
            }
        }
        return dead;
    }
    return 0;
}

/* Fighter flag 7 (ring out: the end-of-battle check gives the other side the win). */
s32 BtlCtrl_TestFlag7(s32 player) {
    BtlCapiBChr *chr = BtlChar_Get(player);

    if (chr != NULL) {
        return BtlChar_TestFlag(chr, 7);
    }
    return 0;
}

/* Hits of the combo the side is dealing (counted on the opponent). */
s32 BtlSide_GetComboHits(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        return BtlChar_Get(BtlOpp_GetPlayer(chr))->comboHits;
    }
    return 0;
}

/* Damage of the combo the side is dealing, -1 while the opponent has flag 0xED. */
s32 BtlSide_GetComboDamage(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);
    BtlCapiBChr *opp;

    if (chr != NULL) {
        opp = BtlChar_Get(BtlOpp_GetPlayer(chr));
        if (BtlChar_TestFlag(opp, 0xED)) {
            return -1;
        }
        return opp->comboDamage;
    }
    return 0;
}

/* Whether the combo display timer of the opponent is running. */
s32 BtlSide_IsComboShown(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);

    if (chr != NULL) {
        return BtlChar_Get(BtlOpp_GetPlayer(chr))->comboTimer > 0;
    }
    return 0;
}

/* Whether a counted hit landed on the opponent this frame (0 while it is frozen or the battle is loading). */
s32 BtlSide_IsComboHitNew(s32 side) {
    BtlCapiBChr *chr = BtlChar_FindBySide(side);
    BtlCapiBChr *opp;

    if (chr != NULL) {
        opp = BtlChar_Get(BtlOpp_GetPlayer(chr));
        if (BtlChar_IsFrozen(opp)) {
            return 0;
        }
        if (Battle_GetWork()->flags & 0x2000) {
            return 0;
        }
        return opp->comboNewHit;
    }
    return 0;
}

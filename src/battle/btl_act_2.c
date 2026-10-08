#include "common.h"
#include "battle/btl_act_2.h"

/*
 * Fighter action handlers, third slice: 0x1EA5F8..0x1EE058 (a slice of the original handler file: its float
 * pool 0x2FD7C8..0x2FD988 and jump tables 0x2EF920..0x2EFA34 run on from the neighbours).
 * Handler protocol (phase 0 enter, 1 run, 2 decide, 3 leave): include/battle/btl_char_action.h. The handlers
 * are int functions without a return statement.
 *
 *   0xDA..0xDE  BtlAct_DownHandler          0x36  BtlAct_SearchHandler      0x3E  BtlAct_Action3E
 *   0xE1..0xE5  BtlAct_GetUpHandler         0x37  BtlAct_ChargeHandler      0x3F  BtlAct_Action3F
 *   0xE6        BtlAct_ActionE6             0x38  BtlAct_GuardHandler       0x40  BtlAct_Action40
 *   0xE7        BtlAct_ActionE7             0x39  BtlAct_Action39           0x41  BtlAct_Action41
 *   0xE8, 0xE9  BtlAct_ActionE8E9           0x3A  BtlAct_Action3A           0x42  BtlAct_Action42
 *   0xEA        BtlAct_ActionEA             0x3B  BtlAct_Action3B           0x43  BtlAct_Action43
 *   0xEB        BtlAct_ActionEB             0x3C  BtlAct_Action3C           0xAE  BtlAct_KiBlastHandler
 *   0xD3        BtlAct_StunHandler          0x3D  BtlAct_Action3D           0xAF  BtlAct_ChargedKiBlastHandler
 *   0xE0        BtlAct_AirStunHandler                                       0xB0  BtlAct_DashKiBlastHandler
 *                                                                           0xB1  BtlAct_DashChargedKiBlastHandler
 *
 * Flags used all over this file (meanings inferred from use): 0xE held = airborne, 0xF = standing on the
 * ground, 0x11 = no ground under the feet, 5 = locked on to the opponent. Most run phases end with
 * "flag 0x11 -> hold 0xE" and some with "flag 0xF -> clear 0xE".
 */

#define BTL_KMH(x) ((x) * 1000.0f / 3600.0f * (1.0f / 30.0f))
#define BTL_DEG(x) ((x) / 180.0f * 3.14159265f)

typedef BtlActCChr Chr;

extern void BtlAct_Request(Chr *chr, s32 id);
extern void BtlAct_SetQueue(Chr *chr, u32 slot, s32 id);
extern s32 BtlAct_HasQueued(Chr *chr);
extern s32 BtlAct_GetCurrent(Chr *chr);
extern s32 BtlAct_GetPrev(Chr *chr);
extern s32 BtlAct_GetQueued(Chr *chr);
extern f32 BtlAct_GetFacingRelCam(Chr *chr);
extern void BtlAct_SetPitchMotion(Chr *chr, s32 motionUp, s32 motionDown, s32 recalc);
extern s32 BtlAct_IsAirMotion(Chr *chr, s32 useSaved);
extern void BtlAct_PushDir(Chr *chr, Vec4 *dir, f32 speed, f32 arg);

extern s32 BtlAnim_Advance(Chr *chr, s32 flags);
extern void BtlAnim_AdvanceLoop(Chr *chr, s32 flags);
extern s32 BtlAnim_AdvanceThen(Chr *chr, s32 next, f32 blend, s32 flags);
extern u32 BtlAnim_GetFlags(u32 anim);
extern f32 BtlAnim_GetFrame(Chr *chr);
extern s32 BtlAnim_GetId(Chr *chr);
extern s32 BtlAnim_GetNextId(Chr *chr);
extern f32 BtlAnim_GetProgress(Chr *chr);
extern s32 BtlAnim_IsNew(Chr *chr);
extern s32 BtlAnim_PassedRatio(Chr *chr, f32 ratio);
extern void BtlAnim_Play(Chr *chr, s32 anim, f32 blend);
extern void BtlAnim_PlaySub(Chr *chr, s32 anim);
extern void BtlAnim_Request(Chr *chr, s32 anim, f32 blend);
extern void BtlAnim_SetDuration(Chr *chr, f32 seconds);

extern void BtlCharSnd_PlayBank8(Chr *chr, s32 id);
extern void BtlCharSnd_PlayCommon(Chr *chr, s32 id);
extern void BtlChar_ClearFlag(Chr *chr, s32 n);
extern s32 BtlChar_FrameMod(s32 n);
extern void *BtlChar_GetObj(Chr *chr);
extern BtlActCPose *BtlChar_GetPos(Chr *chr);
extern s32 BtlChar_IsFree(Chr *chr);
extern s32 BtlChar_IsStage4Or27(void);
extern void BtlChar_PlayVoice(Chr *chr, s32 kind);
extern void BtlChar_PlayVoiceSingle(Chr *chr, s32 kind);
extern void BtlChar_SetFlag(Chr *chr, s32 n);
extern void BtlChar_SetFxBit(Chr *chr, s32 n);
extern void BtlChar_SetHeldFlag(Chr *chr, s32 n);
extern void BtlChar_SetLookEnabled(Chr *chr, s32 enabled);
extern void BtlChar_SetVibration(Chr *chr, f32 power, f32 seconds);
extern s32 BtlChar_TestFlag(Chr *chr, s32 n);
extern s32 BtlChar_TestMemberUnk70(Chr *chr);
extern void BtlChar_Vibrate(Chr *chr, f32 power, f32 seconds);
extern s32 BtlInput_IsHeld(Chr *chr, u32 mask);
extern s32 BtlInput_IsPressed(Chr *chr, u32 mask);
extern s32 BtlInput_TestAction(Chr *chr, s32 id, s32 want);

extern s32 BtlMember_AddKi(Chr *chr, s32 amount);
extern void BtlMember_AddMaxPower(Chr *chr, s32 amount);
extern s32 BtlMember_Damage(Chr *chr, s32 amount, s32 flags);
extern BtlActCMember *BtlMember_GetActive(Chr *chr);
extern BtlActCGauge *BtlMember_GetActiveGauge(Chr *chr);
extern s32 BtlMember_HasAbility(Chr *chr, s32 n);
extern s32 BtlMember_HasBlast(Chr *chr, s32 amount);
extern s32 BtlMember_IsKiFull(Chr *chr);
extern s32 BtlMember_IsMaxPowerFull(Chr *chr);
extern s32 BtlMember_SpendKi(Chr *chr, s32 amount, s32 force);
extern void BtlMember_SubBlast(Chr *chr, s32 amount);

extern void BtlMove_Advance(Chr *chr, f32 speed, f32 accel);
extern void BtlMove_ApplyGravity(Chr *chr);
extern void BtlMove_BrakeVertical(Chr *chr);
extern void BtlMove_Fall(Chr *chr);
extern void BtlMove_MoveVertical(Chr *chr, f32 speed, f32 accel);
extern void BtlMove_RequestOrbit(Chr *chr, f32 near, f32 far);
extern void BtlMove_SetDirection(Chr *chr, s32 mode);
extern void BtlMove_SetLeanX(Chr *chr, f32 v);
extern void BtlMove_SteerAtOpponent(Chr *chr, f32 closeSpeed, f32 yawAccel, f32 pitchAccel, f32 yawMax, f32 pitchMax, f32 maxStep);
extern void BtlMove_Step(Chr *chr, s32 yawMode, s32 pitchMode, s32 dirMode, f32 speed, f32 accel);
extern void BtlMove_TurnModelYaw(Chr *chr, f32 maxStep, f32 rate);
extern void BtlMove_TurnPitch(Chr *chr, s32 mode, f32 maxStep);
extern void BtlMove_TurnYaw(Chr *chr, s32 mode, f32 maxStep);

extern f32 BtlOpp_GetDistanceXZ(Chr *chr);
extern f32 BtlOpp_GetGapXZ(Chr *chr);
extern s32 BtlOpp_GetParamWord0(Chr *chr);
extern s32 BtlOpp_GetSeenAction(Chr *chr);
extern f32 BtlOpp_GetYawFromFacing(Chr *chr);
extern void BtlStat_ClearPenalty(Chr *chr);
extern s32 BtlUtil_AngleToSector(f32 a);
extern f32 BtlUtil_MinF(f32 a, f32 b);
extern f32 BtlUtil_WrapAngle(f32 a);
extern void ChrCam_AddShake(Chr *chr, f32 strength, f32 time);
extern void ChrCam_RequestCut(Chr *chr, s32 kind, s32 cut);
extern f32 Mathf_Cos(f32 a);
extern f32 Mathf_Sin(f32 a);
extern f32 Vec3_Length(Vec4 *v);
extern void Vec4_Copy(Vec4 *dst, Vec4 *src);

extern void BtlActB_SetReactionFlags(Chr *chr);
extern s32 BtlActB_TickMemberChange(Chr *chr, s32 *work);
extern s32 BtlObjAnim_MaskToNode(s32 bits);
extern s32 BtlObjAnim_QueryEvent(void *obj, u64 a, s32 b, s32 c);
extern f32 BtlMoveParam_GetSpeed(Chr *chr, s32 n);
extern f32 BtlMoveParam_GetTurnRate(Chr *chr, s32 n);
extern s32 BtlChar_ClearFlagRet(Chr *chr, s32 n) __asm__("BtlChar_ClearFlag");
extern void BtlCharApi_GetNodePos(s32 objId, s32 node, Vec4 *out);
extern void BtlCharApi_CalcAimDir(s32 objId, s32 node, Vec4 *pos, Vec4 *out, f32 min, f32 max);
extern s32 BtlKiBlast_GetKiCost(Chr *chr);
extern s32 BtlKiBlast_GetHitsOf(Chr *chr, s32 n);
extern u32 BtlParam_GetFlags(Chr *chr);
extern s32 BtlParam_GetUnk0(Chr *chr);
extern f32 BtlParam_GetTypeValueA(Chr *chr);
extern s32 BtlParam_IsType2to4(Chr *chr);
extern s32 BtlParam_GetRateA(Chr *chr);
extern s32 BtlParam_GetRateB(Chr *chr);
extern s32 BtlParam_GetStepA(Chr *chr);
extern s32 BtlParam_GetChargeStartSound(Chr *chr);
extern s32 BtlParam_GetChargeLoopSound(Chr *chr);
extern s32 BtlParam_GetMaxPowerSound(Chr *chr);
extern s32 BtlParam_CanFly(Chr *chr);
extern f32 BtlParam_GetTypeValueB(Chr *chr);
extern s32 BtlDecide_Common(Chr *chr, s32 arg);
extern void BtlDecide_Main(Chr *chr, s32 mask);
extern s32 BtlDecide_Attack(Chr *chr, s32 arg);
extern void BtlDecide_QueueAttack(Chr *chr, s32 attack);
extern s32 BtlAct_CheckRecoveryInput(Chr *chr, s32 arg);
extern s32 BtlAct_GetDownAction(Chr *chr);
extern s32 BtlAct_GetEvasionAttack(Chr *chr);
extern s32 BtlAct_GetIdleFollowUp(Chr *chr);

/* BtlChar_SetFlag as the tail call of an int function (`return f();`). */
extern s32 BtlChar_SetFlagRet(Chr *chr, s32 n) __asm__("BtlChar_SetFlag");

/*
 * Actions 0xDA..0xDE: lying / hitting the ground after a knock-down (name from BtlAct_GetDownAction and the
 * recovery check; the animation meanings are inferred). 0xDA plays 0xB9 / 0xBA (second of each pair = the
 * "air" variant of BtlAct_IsAirMotion) and keeps the airborne flag; the others clear it and add a voice, a
 * sound and an effect bit. At the end of the animation 0xDC / 0xDD go on to 0xDB, the rest to the down action;
 * on stages 4 / 27 a free fighter with flag 0x17 goes to 0xE6 instead. While gauge +0x28 is set, any attack
 * button press (button bit 20) takes one off chr +0x1000. Decide: once free, with that counter used up (or
 * gauge +0x28 clear) and chr +0xD44 >= 10, the recovery input check queues the next action.
 */
s32 BtlAct_DownHandler(Chr *chr, s32 phase) {
    s32 air;
    s32 anim;
    s32 common;
    s32 bank8;
    s32 voice;
    s32 fx;
    s32 clear;

    if (phase == 0) {
        common = -1;
        air = BtlAct_IsAirMotion(chr, 0);
        anim = 0;
        bank8 = -1;
        voice = -1;
        fx = -1;
        clear = 1;
        switch (BtlAct_GetCurrent(chr)) {
            case 0xDA:
                anim = air ? 0xBA : 0xB9;
                clear = 0;
                break;
            case 0xDB:
                anim = air ? 0xCB : 0xCA;
                bank8 = 2;
                fx = 0x34;
                break;
            case 0xDC:
                anim = air ? 0xCD : 0xCC;
                bank8 = 3;
                voice = 0;
                fx = 0x34;
                break;
            case 0xDD:
                anim = air ? 0xCF : 0xCE;
                common = 0x25;
                voice = 1;
                fx = 0x35;
                break;
            case 0xDE:
                anim = air ? 0xCB : 0xCA;
                common = 0x25;
                voice = 1;
                fx = 0x35;
                break;
        }
        BtlAnim_Play(chr, anim, 0.0f);
        if (clear) {
            BtlChar_ClearFlag(chr, 0xE);
        }
        if (voice >= 0) {
            BtlChar_PlayVoice(chr, voice);
        }
        if (common >= 0) {
            BtlCharSnd_PlayCommon(chr, common);
        }
        if (bank8 >= 0) {
            BtlCharSnd_PlayBank8(chr, bank8);
        }
        if (fx >= 0) {
            BtlChar_SetFxBit(chr, fx);
        }
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            switch (BtlAct_GetCurrent(chr)) {
                case 0xDA:
                case 0xDB:
                case 0xDE:
                    BtlAct_Request(chr, BtlAct_GetDownAction(chr));
                    break;
                case 0xDC:
                case 0xDD:
                    BtlAct_Request(chr, 0xDB);
                    break;
            }
            if (BtlChar_IsStage4Or27() && BtlChar_IsFree(chr) && BtlChar_TestFlag(chr, 0x17)) {
                BtlAct_Request(chr, 0xE6);
            }
        }
        BtlMove_Step(chr, 6, 5, 6, 0.0f, BTL_KMH(50.0f) / 3.0f);
        BtlMove_ApplyGravity(chr);
        BtlActB_SetReactionFlags(chr);
        if (BtlMember_GetActiveGauge(chr)->unk28 != 0 && BtlChar_IsFree(chr)) {
            BtlChar_SetFlag(chr, 0xE7);
            if (BtlInput_IsPressed(chr, 0x100000)) {
                chr->unk1000--;
            }
        }
    }
    if (phase == 2) {
        if (BtlChar_IsFree(chr)) {
            if (BtlMember_GetActiveGauge(chr)->unk28 == 0 || chr->unk1000 <= 0) {
                if (chr->unkD44 >= 10) {
                    if (BtlAct_CheckRecoveryInput(chr, 0)) {
                        BtlAct_Request(chr, BtlAct_GetQueued(chr));
                    }
                }
            }
        }
    }
}

/*
 * Actions 0xE1..0xE5: getting up (inferred: they end in BtlAct_GetIdleFollowUp). 0xE1..0xE3 play 0xE2/0xE3,
 * 0xE4/0xE5, 0xE6/0xE7 (ground / air variant); 0xE4 and 0xE5 play 0xFE, 0xE5 after turning the facing by pi.
 * Random draw: in 0xE1, when gauge +0x20 is set and gauge +0x2C is not, BtlChar_FrameMod(2) picks voice 0x20
 * or 0x21, once (gauge +0x2C is then set).
 */
s32 BtlAct_GetUpHandler(Chr *chr, s32 phase) {
    s32 anim;
    s32 air;
    BtlActCPose *pose;

    if (phase == 0) {
        anim = 0;
        air = BtlAct_IsAirMotion(chr, 1);
        switch (BtlAct_GetCurrent(chr)) {
            case 0xE1:
                anim = air ? 0xE3 : 0xE2;
                break;
            case 0xE2:
                anim = air ? 0xE5 : 0xE4;
                break;
            case 0xE3:
                anim = air ? 0xE7 : 0xE6;
                break;
            case 0xE4:
                anim = 0xFE;
                break;
            case 0xE5:
                anim = 0xFE;
                pose = BtlChar_GetPos(chr);
                pose->facing = BtlUtil_WrapAngle(BtlChar_GetPos(chr)->facing + 3.14159265f);
                break;
        }
        BtlAnim_Play(chr, anim, 0.15f);
        if (BtlAct_GetCurrent(chr) != 0xE1) {
            BtlCharSnd_PlayCommon(chr, 0x1F);
        }
        if (BtlAct_GetCurrent(chr) == 0xE1) {
            if (BtlMember_GetActiveGauge(chr)->unk20 != 0 && BtlMember_GetActiveGauge(chr)->unk2C == 0) {
                BtlChar_PlayVoice(chr, BtlChar_FrameMod(2) ? 0x20 : 0x21);
                BtlMember_GetActiveGauge(chr)->unk2C = 1;
            }
        }
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, BtlAct_GetIdleFollowUp(chr));
        }
        BtlMove_Step(chr, 6, 5, 6, 0.0f, BTL_KMH(50.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0x41);
        BtlMove_RequestOrbit(chr, 20.0f, 50.0f);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
    }
}

/*
 * Action 0xE6: animation 0xEA / 0xEB, turning to the opponent under lock-on. Past 40% it runs the common
 * decide helpers (when nothing is queued) and takes the queued action; leaving raises flag 0x33.
 */
s32 BtlAct_ActionE6(Chr *chr, s32 phase) {
    f32 blend;
    s32 air;
    s32 anim;

    if (phase == 0) {
        blend = 0.15f;
        air = BtlAct_IsAirMotion(chr, 1);
        if (BtlAct_GetPrev(chr) == 0xD4) {
            blend = 0.0f;
        }
        anim = air ? 0xEB : 0xEA;
        BtlAnim_Play(chr, anim, blend);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlCharSnd_PlayCommon(chr, 0x21);
        if (BtlChar_TestFlag(chr, 5)) {
            if (BtlAnim_GetFlags(anim) & 0x8000) {
                BtlMove_TurnYaw(chr, 3, 6.2831853f);
            } else {
                BtlMove_TurnYaw(chr, 2, 6.2831853f);
            }
        }
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, BtlAct_GetIdleFollowUp(chr));
        }
        BtlMove_Step(chr, 6, 5, 7, 0.0f, BTL_KMH(50.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0x41);
        BtlChar_SetFlag(chr, 0x95);
    }
    if (phase == 2) {
        if (0.4f < BtlAnim_GetProgress(chr)) {
            if (!BtlAct_HasQueued(chr)) {
                BtlDecide_Main(chr, 0x81C68);
                BtlDecide_Attack(chr, 0xF);
                BtlDecide_Common(chr, 6);
            }
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
    }
    if (phase == 3) {
        return BtlChar_SetFlagRet(chr, 0x33);
    }
}

/* Action 0xE7: animation 0xC8 / 0xC9, voice 0; then 0xDB on the ground, the down action without ground, else 0xD9. */
s32 BtlAct_ActionE7(Chr *chr, s32 phase) {
    f32 zero;

    if (phase == 0) {
        zero = 0.0f;
        BtlAnim_Play(chr, BtlAct_IsAirMotion(chr, 0) ? 0xC9 : 0xC8, zero);
        BtlChar_ClearFlag(chr, 0xE);
        BtlChar_GetPos(chr)->unk98 = zero;
        BtlChar_PlayVoice(chr, 0);
        BtlCharSnd_PlayCommon(chr, 0x32);
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            if (BtlChar_TestFlag(chr, 0xF)) {
                BtlAct_Request(chr, 0xDB);
            } else if (BtlChar_TestFlag(chr, 0x11)) {
                BtlAct_Request(chr, BtlAct_GetDownAction(chr));
            } else {
                BtlAct_Request(chr, 0xD9);
            }
        }
        BtlMove_Step(chr, 6, 5, 6, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlActB_SetReactionFlags(chr);
    }
}

/* Actions 0xE8, 0xE9: animation 0xD0 / 0xD1 then action 0xB; 0xE8 starts with a 700 km/h move (yaw mode 7 / 8). */
s32 BtlAct_ActionE8E9(Chr *chr, s32 phase) {
    s32 air;
    f32 zero;

    if (phase == 0) {
        air = BtlAct_IsAirMotion(chr, 0);
        zero = 0.0f;
        BtlAnim_Play(chr, air ? 0xD1 : 0xD0, zero);
        switch (BtlAct_GetCurrent(chr)) {
            case 0xE8:
                BtlMove_TurnYaw(chr, air ? 8 : 7, 3.14159265f);
                BtlMove_SetDirection(chr, 2);
                BtlChar_GetPos(chr)->unk98 = BTL_KMH(700.0f);
                break;
            case 0xE9:
                BtlChar_GetPos(chr)->unk98 = zero;
                break;
        }
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlChar_PlayVoice(chr, 0);
        BtlCharSnd_PlayBank8(chr, 9);
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0xB);
        }
        BtlMove_Step(chr, 6, 5, 2, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlActB_SetReactionFlags(chr);
    }
}

/* Action 0xEA: loops animation 0xD4 while flag 0xBE is raised, then action 0xB. */
s32 BtlAct_ActionEA(Chr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, 0xD4, 0.15f);
    }
    if (phase == 1) {
        BtlAnim_AdvanceLoop(chr, 0);
        BtlMove_Step(chr, 6, 6, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlActB_SetReactionFlags(chr);
    }
    if (phase == 2) {
        if (!BtlChar_TestFlag(chr, 0xBE)) {
            BtlAct_Request(chr, 0xB);
        }
    }
}

/* Action 0xEB: stands in animation 0 (flag 0xB) until BtlActB_TickMemberChange says go, then action 0xF6. */
s32 BtlAct_ActionEB(Chr *chr, s32 phase) {
    s32 *work = &chr->work[2];

    if (phase == 0) {
        BtlAnim_Play(chr, 0, 0.0f);
    }
    if (phase == 1) {
        BtlAnim_AdvanceLoop(chr, 0);
        BtlMove_Step(chr, 6, 6, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_BrakeVertical(chr);
        BtlChar_SetFlag(chr, 0xB);
        if (BtlActB_TickMemberChange(chr, work)) {
            BtlAct_Request(chr, 0xF6);
        }
    }
}

/* Action 0xD3: loops animation 0xB7 until the countdown chr +0xFE0 runs out, then the idle follow-up. */
s32 BtlAct_StunHandler(Chr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, 0xB7, 0.0f);
    }
    if (phase == 1) {
        BtlAnim_AdvanceLoop(chr, 0);
        BtlMove_Step(chr, 6, 6, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlActB_SetReactionFlags(chr);
    }
    if (phase == 2) {
        if (chr->unkFE0 <= 0) {
            BtlAct_Request(chr, BtlAct_GetIdleFollowUp(chr));
        }
    }
}

/* Action 0xE0: the same in the air (0xD2 / 0xD3), sinking at 30 km/h without ground; ends in the down action. */
s32 BtlAct_AirStunHandler(Chr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, BtlAct_IsAirMotion(chr, 0) ? 0xD3 : 0xD2, 0.0f);
    }
    if (phase == 1) {
        f32 accel;

        BtlAnim_AdvanceLoop(chr, 0);
        accel = BTL_KMH(100.0f);
        BtlMove_Step(chr, 6, 6, 7, 0.0f, accel);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlMove_MoveVertical(chr, BTL_KMH(30.0f), accel);
        } else {
            BtlMove_Fall(chr);
        }
        BtlActB_SetReactionFlags(chr);
        BtlChar_SetFlag(chr, 0x2A);
    }
    if (phase == 2) {
        if (chr->unkFE0 <= 0) {
            BtlAct_Request(chr, BtlAct_GetDownAction(chr));
        }
    }
}

/*
 * Search (action 0x36): widens the search window, a half angle (chr +0xD58, up to pi) and a range (chr +0xD5C,
 * up to 1000000) by the fighter's two per-type values, 60% of them when bit 7 of the opponent's parameter
 * word is set. True on the frame the angle passes `limit`.
 */
s32 BtlAct_GrowSearchWindow(Chr *chr, f32 limit) {
    s32 slow;
    f32 dAngle;
    f32 dRange;
    f32 old;

    slow = (BtlOpp_GetParamWord0(chr) >> 7) & 1;
    dAngle = BtlParam_GetTypeValueA(chr);
    dRange = BtlParam_GetTypeValueB(chr);
    if (slow) {
        dAngle *= 0.6f;
        dRange *= 0.6f;
    }
    old = chr->unkD58;
    chr->unkD58 = old + dAngle;
    if (3.14159265f < chr->unkD58) {
        chr->unkD58 = 3.14159265f;
    }
    chr->unkD5C += dRange;
    if (1000000.0f < chr->unkD5C) {
        chr->unkD5C = 1000000.0f;
    }
    if (old < limit && limit <= chr->unkD58) {
        return 1;
    }
    return 0;
}

/*
 * True when the opponent is inside the search window: within the half angle of the facing and nearer than
 * range * (0.15 + 0.85 * (1 - angle / pi)). Never with flag 0xB2; flag 0xBA hides the opponent unless the
 * fighter has ability 0x6C and is in animation 0x17E.
 */
s32 BtlAct_IsOppInSearchWindow(Chr *chr) {
    s32 ret = 0;
    s32 flag;
    f32 dist;
    f32 yaw;

    if (BtlChar_TestFlag(chr, 0xB2)) {
        return 0;
    }
    BtlOpp_GetParamWord0(chr);
    flag = BtlChar_TestFlag(chr, 0xBA);
    dist = BtlOpp_GetDistanceXZ(chr);
    yaw = __builtin_fabsf(BtlOpp_GetYawFromFacing(chr));
    if (yaw <= chr->unkD58) {
        if (dist < chr->unkD5C * ((1.0f - yaw / 3.14159265f) * 0.85f + 0.15f)) {
            ret = flag == 0;
            if (BtlMember_HasAbility(chr, 0x6C)) {
                if (BtlAnim_GetId(chr) == 0x17E) {
                    ret = 1;
                }
            }
        }
    }
    return ret;
}

/*
 * Action 0x36: searching for an opponent that was lost from sight (lock-on flag 5 clear). Animation 0x17F
 * (types 2..4) into the loop 0x17E; the window grows every frame (voice 0x26 when the angle passes 90 degrees)
 * and when the opponent is inside it the lock-on is restored (voice 0x27, sound 0x2C) and action 0xB follows.
 * Leaving resets the window to 0.5 rad / 500.
 */
s32 BtlAct_SearchHandler(Chr *chr, s32 phase) {
    s32 grow;

    if (phase == 0) {
        if (BtlParam_IsType2to4(chr)) {
            BtlAnim_Play(chr, 0x17F, 0.15f);
        } else {
            BtlAnim_Play(chr, 0x17E, 0.15f);
        }
    }
    if (phase == 1) {
        grow = 0;
        switch (BtlAnim_GetId(chr)) {
            case 0x17F:
                BtlAnim_AdvanceThen(chr, 0x17E, 0.15f, 0);
                if (0.5f < BtlAnim_GetProgress(chr)) {
                    grow = 1;
                }
                break;
            case 0x17E:
                BtlAnim_AdvanceLoop(chr, 0);
                grow = 1;
                break;
        }
        if (grow) {
            if (BtlAct_GrowSearchWindow(chr, 1.5707963f)) {
                BtlChar_PlayVoice(chr, 0x26);
            }
        }
        if (BtlAct_IsOppInSearchWindow(chr)) {
            BtlChar_SetHeldFlag(chr, 5);
            BtlMove_TurnYaw(chr, 2, 3.14159265f);
            BtlChar_PlayVoice(chr, 0x27);
            BtlCharSnd_PlayCommon(chr, 0x2C);
            if (!BtlChar_TestFlag(chr, 0xBA)) {
                BtlChar_SetFxBit(chr, 0x15);
            }
        }
        BtlMove_Step(chr, 6, 6, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xC9);
        BtlChar_SetFlag(chr, 0x1B);
        BtlChar_SetFlag(chr, 0x1C);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlChar_TestFlag(chr, 0xF)) {
            BtlChar_ClearFlag(chr, 0xE);
        }
        if (BtlChar_TestFlag(chr, 5)) {
            BtlAct_Request(chr, 0xB);
        }
    }
    if (phase == 2) {
        BtlDecide_Main(chr, 0x481FEB);
        BtlDecide_Attack(chr, 3);
        BtlDecide_Common(chr, 6);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
    if (phase == 3) {
        chr->unkD58 = 0.5f;
        chr->unkD5C = 500.0f;
    }
}

/*
 * Action 0x37: ki charge. 0x34 start -> 0x35 loop -> 0x36 when the +0x1C gauge fills. The loop adds ki every
 * frame (a different rate without ground), pushes the fighter down along its yaw unless parameter word 0 is
 * 0x80, and with full ki and a blast stock feeds the +0x1C gauge; full ki also clears the stat penalty. A full
 * +0x1C gauge spends one blast stock and holds flag 6 (the powered-up mode). Decide: outside 0x36, releasing
 * the charge input (action input 19) goes to action 0xB. A fighter that cannot fly sinks while airborne.
 */
s32 BtlAct_ChargeHandler(Chr *chr, s32 phase) {
    Vec4 dir;
    f32 yaw;
    f32 accel;
    s32 amount;

    if (phase == 0) {
        BtlAnim_Play(chr, 0x34, 0.15f);
    }
    if (phase == 1) {
        switch (BtlAnim_GetId(chr)) {
            case 0x34:
                if (BtlAnim_AdvanceThen(chr, 0x35, 0.15f, 1)) {
                    BtlCharSnd_PlayCommon(chr, BtlParam_GetChargeStartSound(chr));
                    BtlChar_PlayVoice(chr, 0xD);
                }
                break;
            case 0x35:
                BtlAnim_AdvanceLoop(chr, 0);
                BtlCharSnd_PlayCommon(chr, BtlParam_GetChargeLoopSound(chr));
                BtlChar_SetFxBit(chr, 7);
                BtlChar_SetFlag(chr, 0xC);
                if (BtlParam_GetUnk0(chr) != 0x80) {
                    yaw = BtlChar_GetPos(chr)->rot.y;
                    dir.x = -Mathf_Sin(yaw);
                    dir.y = -1.0f;
                    dir.z = -Mathf_Cos(yaw);
                    BtlAct_PushDir(chr, &dir, 0.3f, 3.0f);
                }
                BtlChar_SetVibration(chr, 0.8f, 0.1f);
                if (BtlChar_TestFlag(chr, 0xF)) {
                    BtlChar_SetFxBit(chr, 0x36);
                }
                if (BtlChar_TestFlag(chr, 0x11)) {
                    amount = BtlParam_GetRateB(chr);
                } else {
                    amount = BtlParam_GetRateA(chr);
                }
                BtlMember_AddKi(chr, amount);
                if (BtlMember_IsKiFull(chr)) {
                    if (BtlMember_HasBlast(chr, 100000)) {
                        BtlMember_AddMaxPower(chr, BtlParam_GetStepA(chr));
                        BtlChar_SetFlag(chr, 0xBB);
                    }
                    BtlStat_ClearPenalty(chr);
                }
                if (BtlMember_IsMaxPowerFull(chr)) {
                    BtlAnim_Request(chr, 0x36, 0.15f);
                    BtlChar_SetFxBit(chr, 8);
                    BtlChar_SetFlag(chr, 0xD);
                    BtlMember_SubBlast(chr, 100000);
                    BtlCharSnd_PlayCommon(chr, BtlParam_GetMaxPowerSound(chr));
                    BtlChar_PlayVoice(chr, 0x13);
                    BtlChar_SetHeldFlag(chr, 6);
                    ChrCam_AddShake(chr, 4.0f, 0.3f);
                    BtlChar_Vibrate(chr, 0.8f, 0.4f);
                }
                break;
            case 0x36:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, BtlAct_GetIdleFollowUp(chr));
                }
                if (!(BtlParam_GetFlags(chr) & 0x10)) {
                    if (BtlAnim_PassedRatio(chr, 0.25f)) {
                        BtlChar_SetFlag(chr, 0x4A);
                    }
                }
                BtlChar_SetFlag(chr, 0xBB);
                BtlChar_SetFlag(chr, 0x59);
                break;
        }
        accel = BTL_KMH(100.0f);
        BtlMove_Step(chr, 6, 6, 7, 0.0f, accel);
        if (!BtlParam_CanFly(chr) && BtlChar_TestFlag(chr, 0xE) && !BtlChar_TestFlag(chr, 0x11)) {
            BtlMove_MoveVertical(chr, accel, accel);
        } else {
            BtlMove_ApplyGravity(chr);
        }
        BtlChar_SetFlag(chr, 0xC9);
        BtlChar_SetFlag(chr, 0x1C);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlChar_TestFlag(chr, 0xF)) {
            BtlChar_ClearFlag(chr, 0xE);
        }
    }
    if (phase == 2) {
        if (BtlAnim_GetId(chr) != 0x36 && BtlAnim_GetNextId(chr) != 0x36) {
            if (BtlInput_TestAction(chr, 0x13, 0)) {
                BtlAct_Request(chr, 0xB);
            }
            BtlDecide_Main(chr, 0xD01C00);
            BtlDecide_Common(chr, 6);
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
    }
    if (phase == 3) {
        BtlChar_PlayVoiceSingle(chr, 0xD);
    }
}

/*
 * Action 0x38: guard (forced by flags 0x68 / 0x69 / 0x6A when a hit is guarded: recoil 0xF2 / 0xF4 / 0xF0).
 * Poses 0xEF (middle), 0xF1 (action input 36) and 0xF3 (action input 37), each with a recoil animation at +1;
 * 0xF5 after flag 0x67. A pose change needs three frames of the new input (work[2]); releasing the guard
 * (action input 34) ends the action.
 *
 * The recoil case tests `phase` (always 1 there) around its second BtlChar_SetFlag with the same call in both
 * arms: the compiler merges the arms only after its second scheduling pass, and the block boundary that exists
 * until then is what gives the original order of the two argument loads in front of BtlAnim_AdvanceThen
 * (`addiu a1,v0,-1 / jal / move a2,zero`; with a plain second call the two come out exchanged). The original
 * source had some such boundary behind SetFlag(0x95); what it looked like is not known.
 */
s32 BtlAct_GuardHandler(Chr *chr, s32 phase) {
    s32 *work = &chr->work[2];
    f32 blend;
    f32 accel;
    s32 anim;
    s32 want;

    if (phase == 0) {
        blend = 0.0f;
        if (BtlChar_TestFlag(chr, 0x68)) {
            anim = 0xF2;
        } else if (BtlChar_TestFlag(chr, 0x69)) {
            anim = 0xF4;
        } else if (BtlChar_TestFlag(chr, 0x6A)) {
            anim = 0xF0;
        } else {
            blend = 0.15f;
            if (BtlInput_TestAction(chr, 0x24, 1)) {
                anim = 0xF1;
            } else {
                anim = BtlInput_TestAction(chr, 0x25, 1) ? 0xF3 : 0xEF;
            }
        }
        BtlAnim_Play(chr, anim, blend);
        BtlMove_TurnYaw(chr, 2, 3.14159265f);
    }
    if (phase == 1) {
        switch (BtlAnim_GetId(chr)) {
            case 0xEF:
            case 0xF1:
            case 0xF3:
                BtlAnim_AdvanceLoop(chr, 0);
                break;
            case 0xF0:
            case 0xF2:
            case 0xF4:
                BtlAnim_AdvanceThen(chr, BtlAnim_GetId(chr) - 1, 0.0f, 0);
                BtlChar_SetFlag(chr, 0x95);
                if (phase) {
                    BtlChar_SetFlag(chr, 0x41);
                } else {
                    BtlChar_SetFlag(chr, 0x41);
                }
                break;
            case 0xF5:
                BtlAnim_AdvanceThen(chr, 0xEF, 0.0f, 0);
                BtlChar_SetFlag(chr, 0x95);
                BtlChar_SetFlag(chr, 0x41);
                if (BtlAnim_IsNew(chr)) {
                    BtlChar_PlayVoice(chr, 0x1B);
                }
                break;
        }
        accel = BTL_KMH(100.0f);
        BtlMove_Step(chr, 6, 5, 2, 0.0f, accel);
        if (!BtlParam_CanFly(chr) && BtlChar_TestFlag(chr, 0xE) && !BtlChar_TestFlag(chr, 0x11)) {
            BtlMove_MoveVertical(chr, accel, accel);
        } else {
            BtlMove_ApplyGravity(chr);
        }
        BtlChar_SetFlag(chr, 0xC9);
        BtlChar_SetFlag(chr, 0xD5);
        BtlChar_SetFlag(chr, 0x1C);
        BtlChar_SetLookEnabled(chr, 1);
        if (BtlChar_TestFlag(chr, 0xF)) {
            if (BTL_KMH(200.0f) < Vec3_Length(&BtlChar_GetPos(chr)->move)) {
                BtlChar_SetFxBit(chr, 0x32);
            }
        }
        if (BtlChar_TestFlag(chr, 0x11) && !BtlChar_TestFlag(chr, 0xF)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlChar_TestFlag(chr, 0xF)) {
            BtlChar_ClearFlag(chr, 0xE);
        }
    }
    if (phase == 2) {
        if (BtlChar_TestFlag(chr, 0x66)) {
            switch (BtlAnim_GetId(chr)) {
                case 0xEF:
                case 0xF1:
                case 0xF3:
                    BtlAnim_Request(chr, BtlAnim_GetId(chr) + 1, 0.0f);
                    break;
                case 0xF0:
                case 0xF2:
                case 0xF4:
                    BtlAnim_Request(chr, BtlAnim_GetId(chr), 0.0f);
                    break;
                case 0xF5:
                    BtlAnim_Request(chr, 0xF0, 0.0f);
                    break;
            }
        } else if (BtlChar_TestFlag(chr, 0x67)) {
            BtlAnim_Request(chr, 0xF5, 0.0f);
        } else {
            switch (BtlAnim_GetId(chr)) {
                case 0xEF:
                case 0xF1:
                case 0xF3:
                    if (BtlInput_TestAction(chr, 0x22, 0)) {
                        BtlAct_Request(chr, 0xB);
                    }
                    want = 0xEF;
                    if (BtlInput_TestAction(chr, 0x24, 1)) {
                        want = 0xF1;
                    }
                    if (BtlInput_TestAction(chr, 0x25, 1)) {
                        want = 0xF3;
                    }
                    if (BtlAnim_GetId(chr) == want) {
                        *work = 0;
                    } else {
                        (*work)++;
                    }
                    if (*work >= 3) {
                        BtlAnim_Request(chr, want, 0.15f);
                        *work = 0;
                    }
                    break;
                case 0xF0:
                case 0xF2:
                case 0xF4:
                case 0xF5:
                    break;
            }
            BtlDecide_Main(chr, 0x1000000);
            BtlDecide_Attack(chr, 0x400);
            BtlDecide_Common(chr, 6);
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
    }
}

/* Action 0x39 (forced by flag 0x7B): animation 0xEC, then the evasion attack BtlAct_GetEvasionAttack picks. */
s32 BtlAct_Action39(Chr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, 0xEC, 0.0f);
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlDecide_QueueAttack(chr, BtlAct_GetEvasionAttack(chr));
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
        BtlMove_Step(chr, 6, 5, 2, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xC9);
        BtlChar_SetFlag(chr, 0xD5);
        if (BtlChar_TestFlag(chr, 0xF)) {
            if (BTL_KMH(200.0f) < Vec3_Length(&BtlChar_GetPos(chr)->move)) {
                BtlChar_SetFxBit(chr, 0x32);
            }
        }
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
    }
}

/* Action 0x3A: animation 0xEE (voice 0x1E under lock-on with flag 0x13), then action 0xB or the queue; leaving raises flag 0x33. */
s32 BtlAct_Action3A(Chr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, 0xEE, 0.15f);
        if (BtlChar_TestFlag(chr, 5) && BtlChar_TestFlag(chr, 0x13)) {
            BtlChar_PlayVoice(chr, 0x1E);
        }
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0xB);
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
        BtlMove_Step(chr, 3, 5, 2, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xD5);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
    }
    if (phase == 3) {
        return BtlChar_SetFlagRet(chr, 0x33);
    }
}

/*
 * Action 0x3B (forced by flag 0x6B): animation 0xF9, flags 0x50 / 0x51. A counter of 4 (work[2]) is refilled
 * by action input 43 and zeroed when the guard is released; each flag 0x31 takes one off and alternates
 * 0xF9 / 0xFA, and at 0 the action ends.
 */
s32 BtlAct_Action3B(Chr *chr, s32 phase) {
    s32 *work = &chr->work[2];

    if (phase == 0) {
        BtlAnim_Play(chr, 0xF9, 0.15f);
        *work = 4;
    }
    if (phase == 1) {
        BtlAnim_Advance(chr, 0);
        BtlMove_Step(chr, 6, 6, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0x1C);
        BtlChar_SetFlag(chr, 0x50);
        BtlChar_SetFlag(chr, 0x51);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlChar_TestFlag(chr, 0xF)) {
            BtlChar_ClearFlag(chr, 0xE);
        }
    }
    if (phase == 2) {
        if (BtlInput_TestAction(chr, 0x2B, 1)) {
            *work = 4;
        }
        if (BtlInput_TestAction(chr, 0x22, 0)) {
            *work = 0;
        }
        if (BtlChar_TestFlag(chr, 0x31)) {
            if (--*work <= 0) {
                BtlAct_Request(chr, 0xB);
            } else {
                BtlAnim_Request(chr, BtlAnim_GetId(chr) == 0xF9 ? 0xFA : 0xF9, 0.0f);
            }
        }
        BtlDecide_Common(chr, 6);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}

/* Action 0x3C: animation 0xFD with voice 5 and flag 0x52, then action 0xB. */
s32 BtlAct_Action3C(Chr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, 0xFD, 0.15f);
        BtlChar_PlayVoice(chr, 5);
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0xB);
        }
        BtlMove_Step(chr, 6, 6, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0x1C);
        BtlChar_SetFlag(chr, 0x52);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlChar_TestFlag(chr, 0xF)) {
            BtlChar_ClearFlag(chr, 0xE);
        }
    }
    if (phase == 2) {
        BtlDecide_Main(chr, 0x1C6A);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}

/* Action 0x3D (forced by flag 0x6B): 0xF9 -> loop 0xFA with flag 0x54 until the guard is released. */
s32 BtlAct_Action3D(Chr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, 0xF9, 0.15f);
        BtlCharSnd_PlayCommon(chr, 0x1F);
    }
    if (phase == 1) {
        switch (BtlAnim_GetId(chr)) {
            case 0xF9:
                BtlAnim_AdvanceThen(chr, 0xFA, 0.0f, 0);
                break;
            case 0xFA:
                BtlAnim_AdvanceLoop(chr, 0);
                break;
        }
        BtlMove_Step(chr, 6, 6, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0x1C);
        BtlChar_SetFlag(chr, 0x54);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlChar_TestFlag(chr, 0xF)) {
            BtlChar_ClearFlag(chr, 0xE);
        }
    }
    if (phase == 2) {
        if (BtlInput_TestAction(chr, 0x22, 0)) {
            BtlAct_Request(chr, 0xB);
        }
        BtlDecide_Common(chr, 6);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}

/*
 * Action 0x3E: an airborne step (animation 0xFB, voice 5) moving at the fighter's speed 4 / 5 around the
 * opponent, then action 0xF. Action input 44 past 40% queues another; 70% takes the queue; action input 64
 * goes to 0x58.
 */
s32 BtlAct_Action3E(Chr *chr, s32 phase) {
    f32 speed;
    f32 c;

    if (phase == 0) {
        BtlAnim_Play(chr, 0xFB, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlChar_PlayVoice(chr, 5);
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0xF);
        }
        speed = BtlMoveParam_GetSpeed(chr, BtlChar_TestFlag(chr, 0x11) + 4);
        BtlMove_TurnYaw(chr, 0, BtlMoveParam_GetTurnRate(chr, 0));
        BtlMove_TurnPitch(chr, 2, BTL_DEG(9.0f));
        BtlMove_TurnModelYaw(chr, BTL_DEG(36.0f), 0.3f);
        BtlMove_SetDirection(chr, 3);
        BtlMove_Advance(chr, speed, 10000.0f);
        BtlMove_ApplyGravity(chr);
        if (BtlChar_TestFlag(chr, 5) && !BtlInput_IsHeld(chr, 0xF0)) {
            BtlChar_SetFlag(chr, 0x1A);
        }
        c = Mathf_Cos(BtlOpp_GetYawFromFacing(chr));
        if (0.0f < c) {
            BtlMove_SetLeanX(chr, BtlChar_GetPos(chr)->speed * c);
        }
        BtlChar_SetFlag(chr, 0xCA);
        BtlMove_RequestOrbit(chr, 50.0f, 100.0f);
        BtlChar_SetFlag(chr, 0x50);
        BtlChar_SetFlag(chr, 0x51);
        BtlChar_SetFxBit(chr, 0xB);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
    }
    if (phase == 2) {
        if (0.4f < BtlAnim_GetProgress(chr) && BtlInput_TestAction(chr, 0x2C, 1)) {
            BtlAct_SetQueue(chr, 0, 0x3E);
        }
        if (BtlAnim_PassedRatio(chr, 0.7f)) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
        if (BtlInput_TestAction(chr, 0x40, 1)) {
            BtlAct_Request(chr, 0x58);
        }
        if (BtlDecide_Attack(chr, 0xC000)) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
        if (BtlDecide_Common(chr, 6)) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
    }
}

/* Action 0x3F: the ground version (animation 0xFC, then 0x11; input 64 goes to 0x59; landing ends it). */
s32 BtlAct_Action3F(Chr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, 0xFC, 0.15f);
        BtlChar_ClearFlag(chr, 0xE);
        BtlChar_PlayVoice(chr, 5);
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0x11);
        }
        BtlMove_Step(chr, 2, 5, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xD5);
        BtlChar_SetFlag(chr, 0x50);
        BtlChar_SetFlag(chr, 0x51);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
    }
    if (phase == 2) {
        if (0.4f < BtlAnim_GetProgress(chr) && BtlInput_TestAction(chr, 0x2C, 1)) {
            BtlAct_SetQueue(chr, 0, 0x3F);
        }
        if (BtlAnim_PassedRatio(chr, 0.7f)) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
        if (BtlInput_TestAction(chr, 0x40, 1)) {
            BtlAct_Request(chr, 0x59);
        }
        if (BtlDecide_Attack(chr, 0x60000)) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
        if (BtlChar_TestFlag(chr, 0xF)) {
            BtlAct_Request(chr, 0xB);
        }
    }
}

/* Action 0x40: animation 0xDE with flag 0x42, then action 0xB; past 60% a queued action takes over. */
s32 BtlAct_Action40(Chr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, 0xDE, 0.15f);
        BtlCharSnd_PlayCommon(chr, 0x1F);
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0xB);
        }
        BtlMove_Step(chr, 6, 5, 3, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xD5);
        BtlChar_SetFlag(chr, 0x42);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
    }
    if (phase == 2) {
        if (0.6f < BtlAnim_GetProgress(chr) && BtlAct_HasQueued(chr)) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
    }
}

/*
 * Action 0x41 (forced by flag 0x7D): plays 0x193, 0x194, 0x195 in turn (chr +0xD8C, reset unless the previous
 * action was 0x41) with camera cut 0x11 / 0x12 by the sign of the fighter camera's `side`, then idles (0 / 0x26)
 * until flag 0xB5 or the opponent's seen action is no longer 0x45, and requests 0x6A.
 */
s32 BtlAct_Action41(Chr *chr, s32 phase) {
    s32 idle;

    if (phase == 0) {
        if (BtlAct_GetPrev(chr) != 0x41) {
            chr->unkD8C = 0;
        }
        BtlAnim_Play(chr, chr->unkD8C + 0x193, 0.15f);
        chr->unkD8C++;
        chr->unkD8C %= 3;
        BtlCharSnd_PlayCommon(chr, 0x1F);
        ChrCam_RequestCut(chr, 1, chr->camSide < 0.0f ? 0x11 : 0x12);
    }
    if (phase == 1) {
        idle = BtlChar_TestFlag(chr, 0xE) ? 0x26 : 0;
        switch (BtlAnim_GetId(chr)) {
            case 0x193:
            case 0x194:
            case 0x195:
                BtlAnim_AdvanceThen(chr, idle, 0.15f, 0);
                break;
            case 0:
            case 0x26:
                BtlAnim_AdvanceLoop(chr, 0);
                break;
        }
        BtlMove_Step(chr, 2, 5, 3, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xD5);
        BtlChar_SetFlag(chr, 0x4E);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
    }
    if (phase == 2) {
        switch (BtlAnim_GetId(chr)) {
            case 0:
            case 0x26:
                if (BtlChar_TestFlag(chr, 0xB5) || BtlOpp_GetSeenAction(chr) != 0x45) {
                    BtlAct_Request(chr, 0x6A);
                }
                break;
            case 0x193:
            case 0x194:
            case 0x195:
                break;
        }
    }
}

/* Action 0x42: animation 0x192 over 25 frames; costs 5000 health (flags 0x42B), holds flag 0x4B for the first half. */
s32 BtlAct_Action42(Chr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, 0x192, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlChar_GetPos(chr)->unk98 = 0.0f;
        BtlChar_GetPos(chr)->unk9C = 0.0f;
        BtlMember_Damage(chr, 5000, 0x42B);
        BtlChar_SetHeldFlag(chr, 0x4B);
        BtlCharSnd_PlayCommon(chr, 0x24);
        BtlChar_PlayVoice(chr, 6);
        BtlChar_SetFxBit(chr, 0x2B);
        BtlChar_Vibrate(chr, 0.8f, 0.4f);
    }
    if (phase == 1) {
        BtlAnim_SetDuration(chr, 25.0f / 30.0f);
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0xB);
        }
        if (BtlAnim_PassedRatio(chr, 0.5f)) {
            BtlChar_ClearFlag(chr, 0x4B);
        }
        BtlMove_Step(chr, 6, 6, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xC9);
        BtlChar_SetFlag(chr, 0xD5);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
    }
    if (phase == 3) {
        return BtlChar_ClearFlagRet(chr, 0x4B);
    }
}

/* Action 0x43: animation 0x185 with voice 0x23; counts in the active member's +0xA0. */
s32 BtlAct_Action43(Chr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, 0x185, 0.15f);
        BtlChar_PlayVoice(chr, 0x23);
        BtlMember_GetActive(chr)->unkA0++;
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0xB);
        }
        BtlMove_Step(chr, 6, 5, 2, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
    }
}

/* Starts sub-animation `anim` and returns the model node of its event 4 (the hand the blast leaves from). */
s32 BtlAct_PlaySubAnim(Chr *chr, s32 anim) {
    void *obj = BtlChar_GetObj(chr);

    BtlAnim_PlaySub(chr, anim);
    return BtlObjAnim_MaskToNode(BtlObjAnim_QueryEvent(obj, 4, 1, 6));
}

/*
 * Action 0xAE: ki blast. Alternates animations 0x71 / 0x72 (chr +0xDE8; always 0x71 unless the member word or
 * parameter flag 0x100 allows the second hand), spends the ki cost, voice 0xC, counts the shots (chr +0xDE0)
 * and restarts a 30 frame timer (chr +0xDE4). The aim direction (chr +0xDD0) is the facing, or toward the
 * opponent from node 0x11 within 36 degrees under lock-on.
 */
s32 BtlAct_KiBlastHandler(Chr *chr, s32 phase) {
    Vec4 pos;
    s32 idle;
    f32 zero;

    if (phase == 0) {
        if (BtlAct_GetPrev(chr) != 0xAE) {
            chr->unkDE8 = 0;
        }
        if (chr->unkDE8 == 0) {
            BtlAnim_Play(chr, 0x71, 0.1f);
            chr->unkDE8 = 1;
        } else {
            BtlAnim_Play(chr, 0x72, 0.1f);
            if (!BtlChar_TestMemberUnk70(chr) && !(BtlParam_GetFlags(chr) & 0x100)) {
                chr->unkDE8 = 0;
            }
        }
        BtlMember_SpendKi(chr, BtlKiBlast_GetKiCost(chr), 0);
        BtlChar_PlayVoice(chr, 0xC);
        chr->unkDE0 += BtlKiBlast_GetHitsOf(chr, 0);
        chr->unkDE4 = 0x1E;
    }
    if (phase == 1) {
        idle = BtlChar_TestFlag(chr, 0xE) ? 0x26 : 0;
        switch (BtlAnim_GetId(chr)) {
            case 0x71:
            case 0x72:
                BtlAnim_AdvanceThen(chr, idle, 0.2f, 0);
                BtlAct_SetPitchMotion(chr, BtlAnim_GetId(chr) + 2, BtlAnim_GetId(chr) + 4, 1);
                break;
            case 0:
            case 0x26:
                BtlAnim_Advance(chr, 1);
                if (!BtlChar_TestFlag(chr, 0x34)) {
                    BtlAct_Request(chr, 0xB);
                }
                BtlChar_SetFlag(chr, 0x87);
                break;
        }
        zero = 0.0f;
        BtlMove_Step(chr, 2, 5, 7, zero, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xD5);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlChar_TestFlag(chr, 5)) {
            BtlCharApi_GetNodePos(chr->objId, 0x11, &pos);
            BtlCharApi_CalcAimDir(chr->objId, 0x11, &pos, &chr->unkDD0, -BTL_DEG(36.0f), BTL_DEG(36.0f));
        } else {
            chr->unkDD0.x = Mathf_Sin(BtlChar_GetPos(chr)->facing);
            chr->unkDD0.y = zero;
            chr->unkDD0.z = Mathf_Cos(BtlChar_GetPos(chr)->facing);
            chr->unkDD0.w = zero;
        }
    }
    if (phase == 2) {
        BtlDecide_Attack(chr, 0xC);
        switch (BtlAnim_GetId(chr)) {
            case 0x71:
            case 0x72:
                if (BtlChar_TestFlag(chr, 0x31)) {
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                }
                break;
            case 0:
            case 0x26:
                BtlDecide_Main(chr, 0x81CEA);
                BtlAct_Request(chr, BtlAct_GetQueued(chr));
                break;
        }
    }
}

/* Action 0xAF: charged ki blast: 0x7B wind-up -> 0x7C hold (until action input 91) -> 0x7D release (ki cost, voice 0x25). */
s32 BtlAct_ChargedKiBlastHandler(Chr *chr, s32 phase) {
    Vec4 pos;
    f32 frame;
    f32 zero;

    if (phase == 0) {
        BtlAnim_Play(chr, 0x7B, 0.15f);
        chr->unkDF0 = BtlAct_PlaySubAnim(chr, 0x7D);
        BtlChar_SetFxBit(chr, 0x17);
        BtlCharSnd_PlayCommon(chr, 0x2E);
        chr->unkDE0++;
        chr->unkDE4 = 0x1E;
    }
    if (phase == 1) {
        switch (BtlAnim_GetId(chr)) {
            case 0x7B:
                BtlAnim_AdvanceThen(chr, 0x7C, 0.0f, 0);
                BtlChar_SetFlag(chr, 0x8E);
                break;
            case 0x7C:
                BtlAnim_AdvanceThen(chr, 0x7D, 0.0f, 0);
                BtlChar_SetFlag(chr, 0x8E);
                chr->unkDEC = BtlAnim_GetProgress(chr);
                break;
            case 0x7D:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xB);
                }
                frame = BtlAnim_GetFrame(chr);
                if (frame < (f32)BtlObjAnim_QueryEvent(BtlChar_GetObj(chr), 4, 0, 0)) {
                    BtlChar_SetFlag(chr, 0x8E);
                }
                if (BtlAnim_IsNew(chr)) {
                    BtlMember_SpendKi(chr, BtlKiBlast_GetKiCost(chr), 0);
                    BtlChar_PlayVoice(chr, 0x25);
                }
                break;
        }
        zero = 0.0f;
        BtlMove_Step(chr, 2, 5, 7, zero, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xD5);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        BtlAct_SetPitchMotion(chr, BtlAnim_GetId(chr) + 3, BtlAnim_GetId(chr) + 6, 1);
        if (BtlChar_TestFlag(chr, 5)) {
            BtlCharApi_GetNodePos(chr->objId, 0x11, &pos);
            BtlCharApi_CalcAimDir(chr->objId, chr->unkDF0, &pos, &chr->unkDD0, -BTL_DEG(36.0f), BTL_DEG(36.0f));
        } else {
            chr->unkDD0.x = Mathf_Sin(BtlChar_GetPos(chr)->facing);
            chr->unkDD0.y = zero;
            chr->unkDD0.z = Mathf_Cos(BtlChar_GetPos(chr)->facing);
            chr->unkDD0.w = zero;
        }
    }
    if (phase == 2) {
        if (BtlAnim_GetId(chr) == 0x7C) {
            if (BtlInput_TestAction(chr, 0x5B, 1)) {
                BtlAnim_Request(chr, 0x7D, 0.0f);
            }
        }
    }
}

/*
 * Action 0xB0: ki blast while dashing. The animation is picked from the facing relative to the camera yaw
 * (0x77 forward, 0x78 / 0x79 sideways, firing 90 degrees off the direction of travel); forward under lock-on
 * the dash steers at the opponent. Then action 0xF.
 */
s32 BtlAct_DashKiBlastHandler(Chr *chr, s32 phase) {
    s32 anim;
    Vec4 *dir;
    f32 speed;
    f32 yawAccel;
    f32 max;
    f32 tmp;
    f32 tmp2;

    if (phase == 0) {
        anim = 0;
        switch (BtlUtil_AngleToSector(BtlAct_GetFacingRelCam(chr))) {
            case 0:
                anim = 0x77;
                if (BtlChar_TestFlag(chr, 5)) {
                    chr->work[0] |= 1;
                    BtlChar_SetHeldFlag(chr, 0xE);
                }
                break;
            case 2:
                anim = 0x78;
                break;
            case 1:
                anim = 0x77;
                break;
            case 3:
                anim = 0x79;
                break;
        }
        BtlAnim_Play(chr, anim, 0.15f);
        BtlMember_SpendKi(chr, BtlKiBlast_GetKiCost(chr), 0);
        BtlChar_PlayVoice(chr, 0xC);
        chr->unkDE0 += BtlObjAnim_QueryEvent(BtlChar_GetObj(chr), 4, 0, 3);
        chr->unkDE4 = 0x1E;
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0xF);
        }
        dir = &chr->unkDD0;
        Vec4_Copy(dir, &BtlChar_GetPos(chr)->vel);
        switch (BtlAnim_GetId(chr)) {
            case 0x77:
                break;
            case 0x78:
                tmp = -dir->x;
                tmp2 = chr->unkDD0.z;
                chr->unkDD0.z = tmp;
                dir->x = tmp2;
                break;
            case 0x79:
                tmp = -chr->unkDD0.z;
                tmp2 = dir->x;
                dir->x = tmp;
                chr->unkDD0.z = tmp2;
                break;
        }
        if ((chr->work[0] & 1) && BtlChar_TestFlag(chr, 5)) {
            yawAccel = BTL_DEG(9.0f);
            speed = BtlMoveParam_GetSpeed(chr, BtlChar_TestFlag(chr, 0x11) + 4);
            max = BTL_DEG(54.0f);
            if (chr->actionFrame == 0) {
                yawAccel = 3.14159265f;
            }
            BtlMove_SteerAtOpponent(chr, speed, yawAccel, 0.0f, BtlUtil_MinF(BtlOpp_GetGapXZ(chr) * 0.01f, 1.0f) * max, 0.0f, max);
            BtlMove_TurnModelYaw(chr, BTL_DEG(36.0f), 0.5f);
            BtlMove_SetDirection(chr, 3);
            BtlMove_Advance(chr, speed, BTL_KMH(200.0f));
        } else {
            speed = BtlMoveParam_GetSpeed(chr, BtlChar_TestFlag(chr, 0x11) + 4);
            BtlMove_TurnYaw(chr, 0, BtlMoveParam_GetTurnRate(chr, 0));
            BtlMove_TurnPitch(chr, 5, BTL_DEG(9.0f));
            BtlMove_TurnModelYaw(chr, BTL_DEG(36.0f), 0.3f);
            BtlMove_SetDirection(chr, 3);
            BtlMove_Advance(chr, speed, 10000.0f);
        }
        BtlMove_ApplyGravity(chr);
        BtlMove_RequestOrbit(chr, 20.0f, 50.0f);
        BtlChar_SetFlag(chr, 0xCA);
        BtlChar_SetFxBit(chr, 0xB);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlChar_TestFlag(chr, 5) && (chr->work[0] & 1)) {
            BtlMove_SetLeanX(chr, BtlChar_GetPos(chr)->speed);
        }
    }
    if (phase == 2) {
        if (0.5f < BtlAnim_GetProgress(chr)) {
            BtlDecide_Attack(chr, 0xC000);
            if (BtlInput_TestAction(chr, 0x2C, 1)) {
                BtlAct_SetQueue(chr, 0, 0x3E);
            }
            if (BtlChar_TestFlag(chr, 0x31)) {
                BtlAct_Request(chr, BtlAct_GetQueued(chr));
            }
        }
    }
}

/* Action 0xB1: charged ki blast while dashing (0x84 / 0x87 / 0x8A wind-up, +1 hold, +2 release). */
s32 BtlAct_DashChargedKiBlastHandler(Chr *chr, s32 phase) {
    s32 anim;
    Vec4 *dir;
    f32 speed;
    f32 yawAccel;
    f32 max;
    f32 tmp;
    f32 tmp2;
    f32 frame;

    if (phase == 0) {
        anim = 0;
        switch (BtlUtil_AngleToSector(BtlAct_GetFacingRelCam(chr))) {
            case 0:
                anim = 0x84;
                if (BtlChar_TestFlag(chr, 5)) {
                    chr->work[0] |= 1;
                    BtlChar_SetHeldFlag(chr, 0xE);
                }
                break;
            case 2:
                anim = 0x87;
                break;
            case 1:
                anim = 0x84;
                break;
            case 3:
                anim = 0x8A;
                break;
        }
        BtlAnim_Play(chr, anim, 0.15f);
        chr->unkDF0 = BtlAct_PlaySubAnim(chr, anim + 2);
        BtlChar_SetFxBit(chr, 0x17);
        BtlCharSnd_PlayCommon(chr, 0x2E);
        chr->unkDE0++;
        chr->unkDE4 = 0x1E;
    }
    if (phase == 1) {
        switch (BtlAnim_GetId(chr)) {
            case 0x84:
            case 0x87:
            case 0x8A:
                BtlAnim_AdvanceThen(chr, BtlAnim_GetId(chr) + 1, 0.0f, 0);
                BtlChar_SetFlag(chr, 0x8E);
                break;
            case 0x85:
            case 0x88:
            case 0x8B:
                BtlAnim_AdvanceThen(chr, BtlAnim_GetId(chr) + 1, 0.0f, 0);
                BtlChar_SetFlag(chr, 0x8E);
                chr->unkDEC = BtlAnim_GetProgress(chr);
                break;
            case 0x86:
            case 0x89:
            case 0x8C:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xF);
                }
                if (BtlAnim_IsNew(chr)) {
                    BtlChar_PlayVoice(chr, 0x25);
                    BtlMember_SpendKi(chr, BtlKiBlast_GetKiCost(chr), 0);
                }
                frame = BtlAnim_GetFrame(chr);
                if (frame < (f32)BtlObjAnim_QueryEvent(BtlChar_GetObj(chr), 4, 0, 0)) {
                    BtlChar_SetFlag(chr, 0x8E);
                }
                break;
        }
        dir = &chr->unkDD0;
        Vec4_Copy(dir, &BtlChar_GetPos(chr)->vel);
        switch (BtlAnim_GetId(chr)) {
            case 0x89:
                tmp = -dir->x;
                tmp2 = chr->unkDD0.z;
                chr->unkDD0.z = tmp;
                dir->x = tmp2;
                break;
            case 0x8C:
                tmp = -chr->unkDD0.z;
                tmp2 = dir->x;
                dir->x = tmp;
                chr->unkDD0.z = tmp2;
                break;
        }
        if ((chr->work[0] & 1) && BtlChar_TestFlag(chr, 5)) {
            yawAccel = BTL_DEG(9.0f);
            speed = BtlMoveParam_GetSpeed(chr, BtlChar_TestFlag(chr, 0x11) + 4);
            max = BTL_DEG(54.0f);
            if (chr->actionFrame == 0) {
                yawAccel = 3.14159265f;
            }
            BtlMove_SteerAtOpponent(chr, speed, yawAccel, 0.0f, BtlUtil_MinF(BtlOpp_GetGapXZ(chr) * 0.01f, 1.0f) * max, 0.0f, max);
            BtlMove_TurnModelYaw(chr, BTL_DEG(36.0f), 0.5f);
            BtlMove_SetDirection(chr, 3);
            BtlMove_Advance(chr, speed, BTL_KMH(200.0f));
        } else {
            speed = BtlMoveParam_GetSpeed(chr, BtlChar_TestFlag(chr, 0x11) + 4);
            BtlMove_TurnYaw(chr, 0, BtlMoveParam_GetTurnRate(chr, 0));
            BtlMove_TurnPitch(chr, 5, BTL_DEG(9.0f));
            BtlMove_TurnModelYaw(chr, BTL_DEG(36.0f), 0.3f);
            BtlMove_SetDirection(chr, 3);
            BtlMove_Advance(chr, speed, 10000.0f);
        }
        BtlMove_ApplyGravity(chr);
        BtlMove_RequestOrbit(chr, 20.0f, 50.0f);
        BtlChar_SetFlag(chr, 0xCA);
        BtlChar_SetFxBit(chr, 0xB);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlChar_TestFlag(chr, 5) && (chr->work[0] & 1)) {
            BtlMove_SetLeanX(chr, BtlChar_GetPos(chr)->speed);
        }
    }
    if (phase == 2) {
        switch (BtlAnim_GetId(chr)) {
            case 0x84:
            case 0x87:
            case 0x8A:
                break;
            case 0x85:
            case 0x88:
            case 0x8B:
                if (BtlInput_TestAction(chr, 0x5B, 1)) {
                    BtlAnim_Request(chr, BtlAnim_GetId(chr) + 1, 0.0f);
                }
                break;
            case 0x86:
            case 0x89:
            case 0x8C:
                break;
        }
    }
}


/* ======== merged from src/battle/btl_act_d.c ======== */


/*
 * Fighter action handlers: 0x1EE058..0x1F1930. A slice of the original handler source file (its float pool
 * runs on from the previous slice and into the next).
 *
 *   0xB2, 0xB3   ki blast fired at an aim direction, plain and chargeable
 *   0x95         ki volley attack (motions 0x71 / 0x72, up to ten shots)
 *   0x0B         the neutral state
 *   0x0C         the idle motion played once
 *   0x0D, 0x0E   moving with the stick: free, and close to the opponent (flag 0x13)
 *   0x0F         dash
 *   0x10..0x13   jump: take-off (0x10 plain, 0x12 from a dash) and airborne part (0x11, 0x13)
 *   0x14..0x16   ascend, descend, and the hop of a fighter that cannot fly
 *   0x17, 0x18   fast ascend / descend (ki drain)
 *   0x19         homing dash at the opponent (ki drain)
 *
 * Handler protocol (btl_char_action.h): s32 handler(chr, phase), phase 0 enter, 1 run, 2 decide, 3 leave; the
 * value is ignored and the handlers fall off their end.
 *
 * Flags used here (meanings inferred from these handlers): 0xE in flight mode (held), 0xF standing on the
 * ground, 0x11 in water, 0x12 another medium (dash sound 0x23), 0x13 close to the opponent, 5 locked on,
 * 0x16 / 0x17 force flight (0x17 only on stages 4 and 27), 0x15 / 0x16 stop a fast ascend / descend.
 */

#define BTL_KMH(x) ((x) * 1000.0f / 3600.0f * (1.0f / 30.0f))
#define BTL_DEG(x) ((x) / 180.0f * 3.14159265f)

#define PHASE_ENTER  0
#define PHASE_RUN    1
#define PHASE_DECIDE 2
#define PHASE_LEAVE  3

extern f32 Mathf_Sin(f32 a);
extern f32 Mathf_Cos(f32 a);
extern f32 Vec3_Length(Vec4 *v);
extern s32 Battle_GetMode(void);

#define BtlChar_GetObj ((void *(*)(BtlActDChr *chr))BtlChar_GetObj)
#define BtlChar_GetPos ((BtlActDPose *(*)(BtlActDChr *chr))BtlChar_GetPos)
#define BtlChar_TestMemberUnk70 ((s32 (*)(BtlActDChr *chr))BtlChar_TestMemberUnk70)
extern s32 BtlChar_IsStage4Or27(void);
#define BtlChar_PlayVoice ((void (*)(BtlActDChr *chr, s32 kind))BtlChar_PlayVoice)
#define BtlChar_SetVibration ((void (*)(BtlActDChr *chr, f32 power, f32 seconds))BtlChar_SetVibration)
#define BtlChar_Vibrate ((void (*)(BtlActDChr *chr, f32 power, f32 seconds))BtlChar_Vibrate)
#define BtlChar_TestFlag ((s32 (*)(BtlActDChr *chr, s32 n))BtlChar_TestFlag)
extern s32 BtlChar_IsFlagRaised(BtlActDChr *chr, s32 n);
#define BtlChar_SetFlag ((void (*)(BtlActDChr *chr, s32 n))BtlChar_SetFlag)
#define BtlChar_SetHeldFlag ((void (*)(BtlActDChr *chr, s32 n))BtlChar_SetHeldFlag)
#define BtlChar_ClearFlag ((void (*)(BtlActDChr *chr, s32 n))BtlChar_ClearFlag)
#define BtlChar_SetFxBit ((void (*)(BtlActDChr *chr, s32 n))BtlChar_SetFxBit)
#define BtlChar_SetLookEnabled ((void (*)(BtlActDChr *chr, s32 enabled))BtlChar_SetLookEnabled)
#define BtlCharSnd_PlayCommon ((void (*)(BtlActDChr *chr, s32 id))BtlCharSnd_PlayCommon)
extern void BtlCharApi_RumbleNear(Vec4 *pos, f32 near, f32 far, f32 power, f32 time);
extern void BtlCharApi_ShakeCamsNear(Vec4 *pos, f32 near, f32 far, f32 arg3, f32 arg4);
#define ChrCam_RequestCut ((void (*)(BtlActDChr *chr, s32 table, s32 index))ChrCam_RequestCut)
extern void ChrCam_EndCut(BtlActDChr *chr);

#define BtlInput_TestAction ((s32 (*)(BtlActDChr *chr, s32 id, s32 want))BtlInput_TestAction)
#define BtlInput_IsHeld ((s32 (*)(BtlActDChr *chr, u32 mask))BtlInput_IsHeld)
extern f32 BtlInput_GetStickY(BtlActDChr *chr);
extern f32 BtlInput_GetStickLength(BtlActDChr *chr);

#define BtlMember_GetActiveGauge ((BtlActDGauge *(*)(BtlActDChr *chr))BtlMember_GetActiveGauge)
#define BtlMember_GetActive ((void *(*)(BtlActDChr *chr))BtlMember_GetActive)
#define BtlMember_SpendKi ((s32 (*)(BtlActDChr *chr, s32 amount, s32 force))BtlMember_SpendKi)

#define BtlAct_Request ((void (*)(BtlActDChr *chr, s32 id))BtlAct_Request)
#define BtlAct_HasQueued ((s32 (*)(BtlActDChr *chr))BtlAct_HasQueued)
#define BtlAct_GetCurrent ((s32 (*)(BtlActDChr *chr))BtlAct_GetCurrent)
extern s32 BtlAct_GetRequested(BtlActDChr *chr);
#define BtlAct_GetPrev ((s32 (*)(BtlActDChr *chr))BtlAct_GetPrev)
#define BtlAct_GetQueued ((s32 (*)(BtlActDChr *chr))BtlAct_GetQueued)
extern s32 BtlAct_IsAttackId(s32 id);
extern void BtlAct_LatchAttack(BtlActDChr *chr);
extern f32 BtlAct_GetGroundY(BtlActDChr *chr);
extern f32 BtlAct_GetHeight(BtlActDChr *chr);
extern f32 BtlAct_GetFramesToGround(BtlActDChr *chr);
extern f32 BtlAct_GetRotYRelCam(BtlActDChr *chr);
extern f32 BtlAct_GetVelDirRelCam(BtlActDChr *chr);
extern f32 BtlAct_ScaleSpeedByApproach(BtlActDChr *chr, f32 speed, f32 range);
#define BtlAct_SetPitchMotion ((void (*)(BtlActDChr *chr, s32 motionUp, s32 motionDown, s32 recalc))BtlAct_SetPitchMotion)

#define BtlAnim_Play ((void (*)(BtlActDChr *chr, s32 anim, f32 blend))BtlAnim_Play)
#define BtlAnim_Request ((void (*)(BtlActDChr *chr, s32 anim, f32 blend))BtlAnim_Request)
extern void BtlAnim_RequestKeep(BtlActDChr *chr, s32 anim, f32 blend);
#define BtlAnim_PlaySub ((void (*)(BtlActDChr *chr, s32 anim))BtlAnim_PlaySub)
extern void BtlAnim_SetUnkC8C(BtlActDChr *chr, f32 v);
#define BtlAnim_SetDuration ((void (*)(BtlActDChr *chr, f32 seconds))BtlAnim_SetDuration)
#define BtlAnim_GetId ((s32 (*)(BtlActDChr *chr))BtlAnim_GetId)
#define BtlAnim_GetFrame ((f32 (*)(BtlActDChr *chr))BtlAnim_GetFrame)
#define BtlAnim_GetProgress ((f32 (*)(BtlActDChr *chr))BtlAnim_GetProgress)
#define BtlAnim_Advance ((s32 (*)(BtlActDChr *chr, s32 flags))BtlAnim_Advance)
#define BtlAnim_AdvanceThen ((s32 (*)(BtlActDChr *chr, s32 next, f32 blend, s32 flags))BtlAnim_AdvanceThen)
#define BtlAnim_AdvanceLoop ((void (*)(BtlActDChr *chr, s32 flags))BtlAnim_AdvanceLoop)
#define BtlAnim_PassedRatio ((s32 (*)(BtlActDChr *chr, f32 ratio))BtlAnim_PassedRatio)
#define BtlAnim_IsNew ((s32 (*)(BtlActDChr *chr))BtlAnim_IsNew)

#define BtlMove_TurnYaw ((void (*)(BtlActDChr *chr, s32 mode, f32 maxStep))BtlMove_TurnYaw)
#define BtlMove_TurnPitch ((void (*)(BtlActDChr *chr, s32 mode, f32 maxStep))BtlMove_TurnPitch)
#define BtlMove_SteerAtOpponent ((void (*)(BtlActDChr *chr, f32 closeSpeed, f32 yawAccel, f32 pitchAccel, f32 yawMax, f32 pitchMax, f32 maxStep))BtlMove_SteerAtOpponent)
#define BtlMove_TurnModelYaw ((void (*)(BtlActDChr *chr, f32 maxStep, f32 rate))BtlMove_TurnModelYaw)
#define BtlMove_SetDirection ((void (*)(BtlActDChr *chr, s32 mode))BtlMove_SetDirection)
#define BtlMove_Advance ((void (*)(BtlActDChr *chr, f32 speed, f32 accel))BtlMove_Advance)
#define BtlMove_Step ((void (*)(BtlActDChr *chr, s32 yawMode, s32 pitchMode, s32 dirMode, f32 speed, f32 accel))BtlMove_Step)
#define BtlMove_MoveVertical ((void (*)(BtlActDChr *chr, f32 speed, f32 accel))BtlMove_MoveVertical)
#define BtlMove_BrakeVertical ((void (*)(BtlActDChr *chr))BtlMove_BrakeVertical)
#define BtlMove_ApplyGravity ((void (*)(BtlActDChr *chr))BtlMove_ApplyGravity)
#define BtlMove_SetLeanX ((void (*)(BtlActDChr *chr, f32 v))BtlMove_SetLeanX)
extern s32 BtlMove_IsBlockedByOpponent(BtlActDChr *chr);
#define BtlMove_RequestOrbit ((void (*)(BtlActDChr *chr, f32 near, f32 far))BtlMove_RequestOrbit)
extern f32 BtlMove_CalcJumpSpeed(f32 height);

#define BtlOpp_GetGapXZ ((f32 (*)(BtlActDChr *chr))BtlOpp_GetGapXZ)
#define BtlOpp_GetYawFromFacing ((f32 (*)(BtlActDChr *chr))BtlOpp_GetYawFromFacing)
#define BtlOpp_GetSeenAction ((s32 (*)(BtlActDChr *chr))BtlOpp_GetSeenAction)
extern f32 BtlUtil_ApproachF(f32 cur, f32 target, f32 step);
extern f32 BtlUtil_MinF(f32 a, f32 b);

#define BtlParam_GetFlags ((s32 (*)(BtlActDChr *chr))BtlParam_GetFlags)
extern s32 BtlParam_GetUnk2(BtlActDChr *chr);
#define BtlParam_CanFly ((s32 (*)(BtlActDChr *chr))BtlParam_CanFly)   /* can fly: parameter flag 0x1000 clear, or ability 0x36 */
#define BtlKiBlast_GetKiCost ((s32 (*)(BtlActDChr *chr))BtlKiBlast_GetKiCost)   /* ki cost of the current attack */
extern void BtlCharApi_GetNodePos(s32 objId, s32 node, Vec4 *out);
extern void BtlCharApi_CalcAimDir(s32 objId, s32 node, Vec4 *pos, Vec4 *out, f32 a, f32 b);
extern s32 BtlObjAnim_QueryEvent(void *obj, u64 mask, s32 layer, s32 what); /* motion event query: what 0 = first frame, 3 = count */

/*
 * Action 0xB2: a ki blast (motion 0x7A) paid on entry, aimed 18..45 degrees below the horizon at the opponent when
 * locked on, else 45 degrees down along the facing. Ends in action 0xB in flight, else 0x11 (fall).
 */
s32 BtlAct_KiBlastB2(BtlActDChr *chr, s32 phase) {
    Vec4 pos;
    f32 r;

    if (phase == PHASE_ENTER) {
        BtlAnim_Play(chr, 0x7A, 0.15f);
        BtlMember_SpendKi(chr, BtlKiBlast_GetKiCost(chr), 0);
        BtlChar_PlayVoice(chr, 0xC);
        BtlChar_GetPos(chr)->speed = 0.0f;
        BtlChar_GetPos(chr)->fallSpeed = 0.0f;
        chr->unkDE0 += BtlObjAnim_QueryEvent(BtlChar_GetObj(chr), 4, 0, 3);
        chr->unkDE4 = 0x1E;
    }
    if (phase == PHASE_RUN) {
        if (BtlAnim_Advance(chr, 0)) {
            if (BtlChar_TestFlag(chr, 0xE)) {
                BtlAct_Request(chr, 0xB);
            } else {
                BtlAct_Request(chr, 0x11);
            }
        }
        BtlMove_Step(chr, 2, 5, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_BrakeVertical(chr);
        BtlChar_SetFlag(chr, 0xD5);
        if (BtlChar_TestFlag(chr, 5)) {
            BtlCharApi_GetNodePos(chr->objId, 0x11, &pos);
            BtlCharApi_CalcAimDir(chr->objId, 0x11, &pos, &chr->aimDir, BTL_DEG(18.0f), BTL_DEG(45.0f));
        } else {
            r = 0.7071f;
            chr->aimDir.x = Mathf_Sin(BtlChar_GetPos(chr)->yaw) * r;
            chr->aimDir.y = r;
            chr->aimDir.z = Mathf_Cos(BtlChar_GetPos(chr)->yaw) * r;
            chr->aimDir.w = 0.0f;
        }
    }
}

/*
 * Action 0xB3: the chargeable version: motions 0x8D wind-up, 0x8E charge (left when input 0x5B, BLAST released, is
 * seen; its progress is kept in +0xDEC), 0x8F release (paid when it starts). Flag 0x8E is raised while charging and
 * until the release motion reaches its first event of kind 4.
 */
s32 BtlAct_KiBlastChargeB3(BtlActDChr *chr, s32 phase) {
    Vec4 pos;
    f32 r;
    f32 frame;

    if (phase == PHASE_ENTER) {
        BtlAnim_Play(chr, 0x8D, 0.15f);
        chr->unkDF0 = BtlAct_PlaySubAnim(chr, 0x8F);
        BtlChar_SetFxBit(chr, 0x17);
        BtlCharSnd_PlayCommon(chr, 0x2E);
        chr->unkDE0++;
        chr->unkDE4 = 0x1E;
    }
    if (phase == PHASE_RUN) {
        switch (BtlAnim_GetId(chr)) {
            case 0x8D:
                BtlAnim_AdvanceThen(chr, BtlAnim_GetId(chr) + 1, 0.0f, 0);
                BtlChar_SetFlag(chr, 0x8E);
                break;
            case 0x8E:
                BtlAnim_AdvanceThen(chr, BtlAnim_GetId(chr) + 1, 0.0f, 0);
                BtlChar_SetFlag(chr, 0x8E);
                chr->unkDEC = BtlAnim_GetProgress(chr);
                break;
            case 0x8F:
                if (BtlAnim_Advance(chr, 0)) {
                    if (BtlChar_TestFlag(chr, 0xE)) {
                        BtlAct_Request(chr, 0xB);
                    } else {
                        BtlAct_Request(chr, 0x11);
                    }
                }
                if (BtlAnim_IsNew(chr)) {
                    BtlChar_PlayVoice(chr, 0x25);
                    BtlMember_SpendKi(chr, BtlKiBlast_GetKiCost(chr), 0);
                }
                frame = BtlAnim_GetFrame(chr);
                if (frame < BtlObjAnim_QueryEvent(BtlChar_GetObj(chr), 4, 0, 0)) {
                    BtlChar_SetFlag(chr, 0x8E);
                }
                break;
        }
        BtlMove_Step(chr, 2, 5, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_BrakeVertical(chr);
        BtlChar_SetFlag(chr, 0xD5);
        if (BtlChar_TestFlag(chr, 5)) {
            BtlCharApi_GetNodePos(chr->objId, 0x11, &pos);
            BtlCharApi_CalcAimDir(chr->objId, 0x11, &pos, &chr->aimDir, BTL_DEG(18.0f), BTL_DEG(45.0f));
        } else {
            r = 0.7071f;
            chr->aimDir.x = Mathf_Sin(BtlChar_GetPos(chr)->yaw) * r;
            chr->aimDir.y = r;
            chr->aimDir.z = Mathf_Cos(BtlChar_GetPos(chr)->yaw) * r;
            chr->aimDir.w = 0.0f;
        }
    }
    if (phase == PHASE_DECIDE) {
        if (BtlAnim_GetId(chr) == 0x8E) {
            if (BtlInput_TestAction(chr, 0x5B, 1)) {
                BtlAnim_Request(chr, BtlAnim_GetId(chr) + 1, 0.0f);
            }
        }
    }
}

/*
 * Action 0x95: attack record 0x25. Motions 0x71 / 0x72 of 0.1 s, each paid when it starts, alternating (always 0x72
 * with parameter flag 0x100 or the member's +0x70 word set), ten in all; the pitch variants are +2 / +4.
 */
s32 BtlAct_KiVolley95(BtlActDChr *chr, s32 phase) {
    Vec4 pos;
    s32 *count = &chr->work[2];
    BtlActDAttack *atk = &chr->attack;
    s32 next;

    if (phase == PHASE_ENTER) {
        BtlAct_LatchAttack(chr);
        BtlAnim_Play(chr, 0x71, 0.1f);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlChar_PlayVoice(chr, 0x14);
        if (atk->flags & 0x80) {
            BtlChar_SetFlag(chr, 0xCD);
        }
        if (atk->cut >= 0) {
            ChrCam_RequestCut(chr, 1, atk->cut);
        }
        if (atk->unk9 < 4) {
            if (atk->unk9 > 0) {
                BtlChar_SetFxBit(chr, 0xD);
            }
        }
    }
    if (phase == PHASE_RUN) {
        switch (BtlAnim_GetId(chr)) {
            case 0x71:
            case 0x72:
                if (BtlAnim_IsNew(chr)) {
                    (*count)++;
                    BtlMember_SpendKi(chr, BtlKiBlast_GetKiCost(chr), 0);
                }
                BtlAnim_SetDuration(chr, 0.1f);
                if (BtlAnim_Advance(chr, 0)) {
                    if (*count >= 10) {
                        BtlAct_Request(chr, 0xB);
                    } else {
                        if (BtlParam_GetFlags(chr) & 0x100) {
                            next = 0x72;
                        } else if (BtlChar_TestMemberUnk70(chr)) {
                            next = 0x72;
                        } else if (BtlAnim_GetId(chr) == 0x71) {
                            next = 0x72;
                        } else {
                            next = 0x71;
                        }
                        BtlAnim_Request(chr, next, 0.0f);
                    }
                }
                BtlAct_SetPitchMotion(chr, BtlAnim_GetId(chr) + 2, BtlAnim_GetId(chr) + 4, 1);
                break;
        }
        BtlMove_Step(chr, 2, 5, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        if (BtlChar_TestFlag(chr, 5)) {
            BtlCharApi_GetNodePos(chr->objId, 0x11, &pos);
            BtlCharApi_CalcAimDir(chr->objId, 0x11, &pos, &chr->aimDir, -BTL_DEG(36.0f), BTL_DEG(36.0f));
        } else {
            chr->aimDir.x = Mathf_Sin(BtlChar_GetPos(chr)->yaw);
            chr->aimDir.y = 0.0f;
            chr->aimDir.z = Mathf_Cos(BtlChar_GetPos(chr)->yaw);
            chr->aimDir.w = 0.0f;
        }
        if (atk->flags & 0x40) {
            if (BtlChar_IsFlagRaised(chr, 0x5B)) {
                if (atk->cut >= 0) {
                    ChrCam_EndCut(chr);
                }
            }
        }
    }
}

/*
 * Landing feedback. Parameter +2 == 4: rumble and camera shake around the fighter, sound 0x25, effect 0x35.
 * Otherwise effect 0x32 and sound 0xB (parameter flag 0x8000), 0x33 (soft) or 0x30 (hard).
 */
void BtlAct_PlayLandFx(BtlActDChr *chr, s32 hard) {
    s32 snd;

    if (BtlParam_GetUnk2(chr) == 4) {
        BtlCharApi_RumbleNear(&BtlChar_GetPos(chr)->pos, 100.0f, 1000.0f, 0.8f, 0.1f);
        BtlCharApi_ShakeCamsNear(&BtlChar_GetPos(chr)->pos, 100.0f, 1000.0f, 3.0f, 0.3f);
        BtlCharSnd_PlayCommon(chr, 0x25);
        BtlChar_SetFxBit(chr, 0x35);
        return;
    }
    if (BtlParam_GetFlags(chr) & 0x8000) {
        snd = 0xB;
    } else {
        snd = (hard != 0) ? 0x30 : 0x33;
    }
    BtlCharSnd_PlayCommon(chr, snd);
    BtlChar_SetFxBit(chr, 0x32);
}

/*
 * The idle motion: on the ground 0 or 1; in flight 0x26 or 0x27, or 0x18B for a fighter that cannot fly and is not
 * in water. The second of each pair is used while the gauge block's +0x28 word is set.
 */
s32 BtlAct_GetIdleMotion(BtlActDChr *chr) {
    s32 motion;

    if (BtlChar_TestFlag(chr, 0xE)) {
        if (!BtlParam_CanFly(chr)) {
            if (!BtlChar_TestFlag(chr, 0x11)) {
                motion = 0x18B;
            } else {
                motion = (BtlMember_GetActiveGauge(chr)->unk28 != 0) ? 0x27 : 0x26;
            }
        } else {
            motion = (BtlMember_GetActiveGauge(chr)->unk28 != 0) ? 0x27 : 0x26;
        }
    } else {
        motion = BtlMember_GetActiveGauge(chr)->unk28 != 0;
    }
    return motion;
}

/* Decision functions: fill the action queue from the input; mask bits enable groups; non-zero when queued. */
#define BtlDecide_Common ((s32 (*)(BtlActDChr *chr, s32 mask))BtlDecide_Common) /* forced transitions: stun, flag 0xBE, lost footing, death */
#define BtlDecide_Main ((s32 (*)(BtlActDChr *chr, u32 mask))BtlDecide_Main)   /* movement, guard, techniques, changes */
#define BtlDecide_Attack ((s32 (*)(BtlActDChr *chr, u32 mask))BtlDecide_Attack)    /* attacks: rush, ki blasts, throws */

/*
 * Action 0xB: the neutral state. Every other action returns here.
 * enter: leaves flight mode when lower than BTL_KMH(50) (0.46) above the ground, plays the idle motion.
 * run: flight mode follows the ground (0xF clears it; water sets it); faces the opponent (step modes 6, 6, 7) with
 *   no speed; flags 0xC9, 0x1C, 0x25; with gauge +0x28 set also 0x96, and 0x137 for parameter flag 0x10000.
 * decide: outside mode 1, a fighter with flag 3 whose member +0xA0 is <= 0 and without flag 0xB1 requests action
 *   0x43 after more than 90 frames; then BtlDecide_Main(0x22FCBFEB), BtlDecide_Attack(0x0800054F),
 *   BtlDecide_Common(0xF), and queue[0] is requested (the later calls overwrite slot 0 of the earlier ones).
 */
s32 BtlAct_NeutralHandler(BtlActDChr *chr, s32 phase) {
    s32 *timer = &chr->work[2];
    s32 look;
    s32 motion;
    f32 speed;

    if (phase == PHASE_ENTER) {
        if (BtlAct_GetHeight(chr) < BTL_KMH(50.0f)) {
            BtlChar_ClearFlag(chr, 0xE);
        }
        BtlAnim_Play(chr, BtlAct_GetIdleMotion(chr), 0.15f);
    }
    if (phase == PHASE_RUN) {
        look = 1;
        if (BtlChar_TestFlag(chr, 0xE)) {
            if (BtlChar_TestFlag(chr, 0xF)) {
                BtlChar_ClearFlag(chr, 0xE);
            }
        } else if (!BtlChar_TestFlag(chr, 0xF)) {
            if (BtlChar_TestFlag(chr, 0x11)) {
                BtlChar_SetHeldFlag(chr, 0xE);
            }
        }
        BtlAnim_AdvanceLoop(chr, 0);
        if (BtlMove_IsBlockedByOpponent(chr)) {
            BtlChar_GetPos(chr)->speed = 0.0f;
        }
        speed = BTL_KMH(100.0f);
        BtlMove_Step(chr, 6, 6, 7, 0.0f, speed);
        if (BtlAnim_GetId(chr) == 0x18B) {
            BtlMove_MoveVertical(chr, speed, speed);
        } else {
            BtlMove_ApplyGravity(chr);
        }
        BtlChar_SetFlag(chr, 0xC9);
        BtlChar_SetFlag(chr, 0x1C);
        BtlChar_SetFlag(chr, 0x25);
        if (BtlMember_GetActiveGauge(chr)->unk28 != 0) {
            BtlChar_SetFlag(chr, 0x96);
            if (BtlParam_GetFlags(chr) & 0x10000) {
                BtlChar_SetFlag(chr, 0x137);
                look = 0;
            }
        }
        motion = BtlAct_GetIdleMotion(chr);
        if (BtlAnim_GetId(chr) != motion) {
            BtlAnim_Request(chr, motion, 0.15f);
        }
        if (look) {
            BtlChar_SetLookEnabled(chr, 1);
        }
    }
    if (phase == PHASE_DECIDE) {
        if (Battle_GetMode() != 1) {
            if (BtlChar_TestFlag(chr, 3)) {
                if (((BtlActDMember *)BtlMember_GetActive(chr))->unkA0 <= 0) {
                    if (!BtlChar_TestFlag(chr, 0xB1)) {
                        if (++(*timer) > 90) {
                            BtlAct_Request(chr, 0x43);
                        }
                    }
                }
            }
        }
        BtlDecide_Main(chr, 0x22FCBFEB);
        BtlDecide_Attack(chr, 0x0800054F);
        BtlDecide_Common(chr, 0xF);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}

/* Action 0xC: the idle motion played once over one second without any input check, then action 0xB. */
s32 BtlAct_IdleOnceHandler(BtlActDChr *chr, s32 phase) {
    if (phase == PHASE_ENTER) {
        BtlAnim_Play(chr, BtlAct_GetIdleMotion(chr), 0.15f);
    }
    if (phase == PHASE_RUN) {
        BtlAnim_SetDuration(chr, 1.0f);
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0xB);
        }
        BtlMove_Step(chr, 6, 6, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0x1C);
        if (BtlMember_GetActiveGauge(chr)->unk28 != 0) {
            BtlChar_SetFlag(chr, 0x96);
            if (BtlParam_GetFlags(chr) & 0x10000) {
                BtlChar_SetFlag(chr, 0x137);
            }
        }
    }
}

#define BtlMoveParam_GetSpeed ((f32 (*)(BtlActDChr *chr, s32 kind))BtlMoveParam_GetSpeed) /* speed of the character table (obj +0x928)[kind], scaled; kinds 4, 5 also by pose +0xAC */
extern s32 BtlMove_TurnYawRet(BtlActDChr *chr, s32 mode, f32 maxStep) __asm__("BtlMove_TurnYaw");

/*
 * Action 0xD: free movement (motions 2 -> 3 with side layers 4..7 weighted by the sine of the heading relative to
 * the camera; 0x18B for a non-flyer in the air). Speed kind 0, or 1 in water. Ends when input 1 (a direction held)
 * is no longer true.
 */
s32 BtlAct_MoveHandler(BtlActDChr *chr, s32 phase) {
    s32 motion;
    s32 subNeg;
    s32 subPos;
    s32 pitchMode;
    f32 speed;
    f32 side;

    if (phase == PHASE_ENTER) {
        motion = 2;
        if (!BtlParam_CanFly(chr)) {
            if (BtlChar_TestFlag(chr, 0xE)) {
                if (!BtlChar_TestFlag(chr, 0x11)) {
                    motion = 0x18B;
                }
            }
        }
        BtlAnim_Play(chr, motion, 0.15f);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlCharSnd_PlayCommon(chr, 0x38);
        }
    }
    if (phase == PHASE_RUN) {
        speed = BtlMoveParam_GetSpeed(chr, BtlChar_TestFlag(chr, 0x11));
        subNeg = 0;
        subPos = 0;
        switch (BtlAnim_GetId(chr)) {
            case 2:
                BtlAnim_AdvanceThen(chr, 3, 0.0f, 0);
                subNeg = 4;
                subPos = 6;
                break;
            case 3:
                BtlAnim_AdvanceLoop(chr, 0);
                subNeg = 5;
                subPos = 7;
                break;
            case 0x18B:
                BtlAnim_AdvanceLoop(chr, 0);
                subNeg = -1;
                subPos = -1;
                break;
        }
        if (subNeg >= 0 && subPos >= 0) {
            side = Mathf_Sin(BtlAct_GetRotYRelCam(chr));
            if (0.0f < side) {
                BtlAnim_SetUnkC8C(chr, side);
                BtlAnim_PlaySub(chr, subPos);
            } else {
                BtlAnim_SetUnkC8C(chr, -side);
                BtlAnim_PlaySub(chr, subNeg);
            }
        }
        pitchMode = 5;
        if (BtlChar_TestFlag(chr, 0xE)) {
            if (BtlChar_TestFlag(chr, 5)) {
                pitchMode = 2;
            }
        }
        BtlMove_Step(chr, 0, pitchMode, 3, speed, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xC9);
        BtlChar_SetFlag(chr, 0x1C);
        BtlChar_SetFlag(chr, 0x25);
        BtlMove_RequestOrbit(chr, 20.0f, 50.0f);
        BtlChar_SetLookEnabled(chr, 1);
        if (BtlChar_TestFlag(chr, 0xE)) {
            if (BtlChar_TestFlag(chr, 0xF)) {
                BtlChar_ClearFlag(chr, 0xE);
            }
        }
        if (BtlMember_GetActiveGauge(chr)->unk28 != 0) {
            BtlChar_SetFlag(chr, 0x96);
        }
        if (BtlAnim_GetId(chr) == 0x18B) {
            if (BtlChar_TestFlag(chr, 0x11)) {
                BtlAnim_Request(chr, 3, 0.15f);
            }
        } else if (!BtlParam_CanFly(chr)) {
            if (!BtlChar_TestFlag(chr, 0x11)) {
                if (!BtlChar_TestFlag(chr, 0xF)) {
                    BtlAnim_Request(chr, 0x18B, 0.15f);
                }
            }
        }
    }
    if (phase == PHASE_DECIDE) {
        if (BtlInput_TestAction(chr, 1, 0)) {
            BtlAct_Request(chr, 0xB);
        }
        BtlDecide_Main(chr, 0x22981FEB);
        BtlDecide_Attack(chr, 0x0800040F);
        BtlDecide_Common(chr, 0xF);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}

/*
 * Action 0xE: movement close to the opponent (flag 0x13), always facing them: motions 8 (toward) / 9 (away) by the
 * direction of travel, side layers 0xA / 0xB. Speed kind 2, or 3 in water, scaled down on approach within 30.
 * Ends when input 2 (no direction for 3 frames) is true. Leaving to action 0x11 snaps the yaw.
 */
s32 BtlAct_CloseMoveHandler(BtlActDChr *chr, s32 phase) {
    f32 *lean = (f32 *)&chr->work[6];
    s32 motion;
    f32 dir;
    f32 speed;

    if (phase == PHASE_ENTER) {
        motion = 8;
        if (!(BtlInput_GetStickY(chr) < 0.0f)) {
            motion = 9;
        }
        if (!BtlParam_CanFly(chr)) {
            if (BtlChar_TestFlag(chr, 0xE)) {
                if (!BtlChar_TestFlag(chr, 0x11)) {
                    motion = 0x18B;
                }
            }
        }
        BtlAnim_Play(chr, motion, 0.15f);
        BtlMove_SetDirection(chr, 0);
        *lean = Mathf_Sin(BtlAct_GetVelDirRelCam(chr));
    }
    if (phase == PHASE_RUN) {
        if (BtlChar_TestFlag(chr, 0xE)) {
            if (BtlChar_TestFlag(chr, 0xF)) {
                BtlChar_ClearFlag(chr, 0xE);
            }
        }
        if (BtlAnim_GetId(chr) != 0x18B) {
            dir = BtlAct_GetVelDirRelCam(chr);
            if (-BTL_DEG(90.0f) < dir && dir < BTL_DEG(90.0f)) {
                if (BtlAnim_GetId(chr) == 9) {
                    BtlAnim_RequestKeep(chr, 8, 0.15f);
                }
            } else {
                if (BtlAnim_GetId(chr) == 8) {
                    BtlAnim_RequestKeep(chr, 9, 0.15f);
                }
            }
            *lean = BtlUtil_ApproachF(*lean, Mathf_Sin(dir), 8.0f / 30.0f);
            if (0.0f < *lean) {
                BtlAnim_SetUnkC8C(chr, *lean);
                BtlAnim_PlaySub(chr, 0xB);
            } else {
                BtlAnim_SetUnkC8C(chr, -*lean);
                BtlAnim_PlaySub(chr, 0xA);
            }
        }
        BtlAnim_AdvanceLoop(chr, 0);
        speed = BtlAct_ScaleSpeedByApproach(chr, BtlMoveParam_GetSpeed(chr, BtlChar_TestFlag(chr, 0x11) + 2), 30.0f);
        BtlMove_Step(chr, 2, 2, BtlChar_TestFlag(chr, 0xE) != 0, speed, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0x1C);
        BtlChar_SetLookEnabled(chr, 1);
        BtlMove_RequestOrbit(chr, 10.0f, 20.0f);
        if (BtlMember_GetActiveGauge(chr)->unk28 != 0) {
            BtlChar_SetFlag(chr, 0x96);
        }
        if (BtlAnim_GetId(chr) == 0x18B) {
            if (BtlChar_TestFlag(chr, 0x11)) {
                BtlAnim_Request(chr, (BtlInput_GetStickY(chr) < 0.0f) ? 8 : 9, 0.15f);
            }
        } else if (!BtlParam_CanFly(chr)) {
            if (!BtlChar_TestFlag(chr, 0x11)) {
                if (!BtlChar_TestFlag(chr, 0xF)) {
                    BtlAnim_Request(chr, 0x18B, 0.15f);
                }
            }
        }
    }
    if (phase == PHASE_DECIDE) {
        if (BtlInput_TestAction(chr, 2, 1)) {
            BtlAct_Request(chr, 0xB);
        }
        BtlDecide_Main(chr, 0x229CBFE3);
        BtlDecide_Attack(chr, 0x0800054F);
        BtlDecide_Common(chr, 0xF);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
    if (phase == PHASE_LEAVE) {
        if (BtlAct_GetRequested(chr) == 0x11) {
            return BtlMove_TurnYawRet(chr, 0, 3.14159265f);
        }
    }
}

#define BtlMoveParam_GetTurnRate ((f32 (*)(BtlActDChr *chr, s32 kind))BtlMoveParam_GetTurnRate) /* turn rate, table +0x74[kind] in degrees; kind 0 scaled by pose +0xAC */

/*
 * Action 0xF: dash (motions 0xC start, 0xD loop, 0xE stop, 0x15 turn-around; side layers 0xF..0x14, 0x16, 0x17).
 * Speed kind 4, or 5 in water. work[0] bit 0: a turn was asked for during the start; work[1]: finished.
 * Entered from actions 0x3E, 0xB0, 0xB1 it resumes in the loop.
 */
s32 BtlAct_DashMoveHandler(BtlActDChr *chr, s32 phase) {
    s32 subNeg;
    s32 subPos;
    s32 moving;
    s32 pitchMode;
    s32 resume;
    s32 queued;
    f32 ratio;
    f32 speed;
    f32 side;
    f32 toward;

    if (phase == PHASE_ENTER) {
        resume = 0;
        switch (BtlAct_GetPrev(chr)) {
            case 0x3E:
            case 0xB0:
            case 0xB1:
                resume = 1;
                break;
        }
        if (resume) {
            BtlAnim_Play(chr, 0xD, 0.25f);
        } else {
            BtlAnim_Play(chr, 0xC, 0.15f);
            BtlChar_SetFlag(chr, 0xD0);
            BtlMove_TurnYaw(chr, 0, 3.14159265f);
            if (BtlChar_TestFlag(chr, 0x12)) {
                BtlCharSnd_PlayCommon(chr, 0x23);
            } else if (BtlChar_TestFlag(chr, 0x11)) {
                BtlCharSnd_PlayCommon(chr, 0x39);
            } else {
                BtlCharSnd_PlayCommon(chr, 0x1E);
            }
        }
    }
    if (phase == PHASE_RUN) {
        subNeg = 0;
        subPos = 0;
        ratio = 0.0f;
        moving = 1;
        switch (BtlAnim_GetId(chr)) {
            case 0xC:
                BtlAnim_AdvanceThen(chr, 0xD, 0.15f, 0);
                subNeg = 0xF;
                ratio = BtlAnim_GetProgress(chr);
                subPos = 0x12;
                break;
            case 0xD:
                BtlAnim_AdvanceLoop(chr, 0);
                subNeg = 0x10;
                ratio = 1.0f;
                subPos = 0x13;
                break;
            case 0xE:
                if (BtlAnim_Advance(chr, 0)) {
                    chr->work[1] = 1;
                }
                subNeg = 0x11;
                ratio = 1.0f - BtlAnim_GetProgress(chr);
                subPos = 0x14;
                moving = 0;
                break;
            case 0x15:
                if (BtlAnim_IsNew(chr)) {
                    if (BtlChar_TestFlag(chr, 0xF)) {
                        BtlChar_SetFxBit(chr, 0x31);
                    }
                }
                if (BtlAnim_Advance(chr, 0)) {
                    chr->work[1] = 1;
                }
                subNeg = 0x16;
                ratio = 1.0f - BtlAnim_GetProgress(chr);
                subPos = 0x17;
                moving = 0;
                chr->work[0] &= ~1;
                break;
        }
        if (!(BtlParam_GetFlags(chr) & 0x20000)) {
            side = Mathf_Sin(BtlAct_GetRotYRelCam(chr));
            if (0.0f < side) {
                BtlAnim_SetUnkC8C(chr, side);
                BtlAnim_PlaySub(chr, subPos);
            } else {
                BtlAnim_SetUnkC8C(chr, -side);
                BtlAnim_PlaySub(chr, subNeg);
            }
        }
        if (BtlChar_TestFlag(chr, 5)) {
            if (moving) {
                if (!BtlChar_TestFlag(chr, 0xE) || !BtlInput_IsHeld(chr, 0xF0)) {
                    BtlChar_SetFlag(chr, 0x1A);
                }
            }
        }
        pitchMode = 5;
        if (BtlChar_TestFlag(chr, 0xE)) {
            if (BtlChar_TestFlag(chr, 5)) {
                pitchMode = 2;
            }
        }
        if (moving) {
            speed = BtlMoveParam_GetSpeed(chr, BtlChar_TestFlag(chr, 0x11) + 4);
            BtlMove_TurnYaw(chr, 0, BtlMoveParam_GetTurnRate(chr, 0));
            BtlMove_TurnPitch(chr, pitchMode, BTL_DEG(9.0f));
            BtlMove_TurnModelYaw(chr, BTL_DEG(36.0f), 0.3f);
            BtlMove_SetDirection(chr, 3);
            BtlMove_Advance(chr, speed, 10000.0f);
            BtlMove_ApplyGravity(chr);
            BtlChar_SetFxBit(chr, 0xB);
            if (BtlChar_TestFlag(chr, 0xF)) {
                BtlChar_SetFxBit(chr, 0x30);
            }
        } else {
            BtlMove_Step(chr, 6, 5, 3, 0.0f, BTL_KMH(100.0f));
            BtlMove_ApplyGravity(chr);
        }
        toward = Mathf_Cos(BtlOpp_GetYawFromFacing(chr));
        if (0.0f < toward) {
            BtlMove_SetLeanX(chr, BtlChar_GetPos(chr)->pitch * toward * ratio);
        }
        BtlChar_SetFlag(chr, 0xCA);
        BtlChar_SetFlag(chr, 0x25);
        BtlMove_RequestOrbit(chr, 50.0f, 100.0f);
        BtlChar_SetFxBit(chr, 0x3A);
        if (BtlMember_GetActiveGauge(chr)->unk28 != 0) {
            BtlChar_SetFlag(chr, 0x96);
        }
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlChar_IsStage4Or27()) {
            if (BtlChar_TestFlag(chr, 0x17)) {
                BtlChar_SetHeldFlag(chr, 0xE);
            }
        }
    }
    if (phase == PHASE_DECIDE) {
        if (chr->work[1] != 0) {
            BtlAct_Request(chr, 0xB);
            if (!BtlAct_HasQueued(chr)) {
                BtlDecide_Main(chr, 1);
                BtlDecide_Common(chr, 1);
            }
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
        switch (BtlAnim_GetId(chr)) {
            case 0xC:
                if (BtlInput_TestAction(chr, 4, 1)) {
                    chr->work[0] |= 1;
                }
                break;
            case 0xD:
                if (BtlInput_TestAction(chr, 6, 1)) {
                    BtlAnim_Request(chr, 0xE, 0.15f);
                }
                if (BtlMove_IsBlockedByOpponent(chr)) {
                    BtlAnim_Request(chr, 0xE, 0.15f);
                    BtlChar_GetPos(chr)->speed = 0.0f;
                }
                if (BtlInput_TestAction(chr, 4, 1) || (chr->work[0] & 1)) {
                    BtlAnim_Request(chr, 0x15, 0.15f);
                }
                break;
            case 0xE:
                if (BtlMove_IsBlockedByOpponent(chr)) {
                    BtlChar_GetPos(chr)->speed = 0.0f;
                }
                BtlDecide_Main(chr, 0x300);
                break;
            case 0x15:
                if (BtlAnim_PassedRatio(chr, 0.5f)) {
                    if (BtlInput_TestAction(chr, 5, 1)) {
                        BtlAct_Request(chr, 0xF);
                        BtlChar_GetPos(chr)->unkAC = BtlUtil_MinF(BtlChar_GetPos(chr)->unkAC + 0.1f, 1.5f);
                    }
                }
                break;
        }
        queued = BtlDecide_Main(chr, 0x20001C64);
        if (BtlAnim_GetId(chr) != 0xE) {
            if (BtlInput_TestAction(chr, 0x40, 1)) {
                BtlAct_Request(chr, 0x58);
            }
            queued |= BtlDecide_Attack(chr, 0xC000);
            if (BtlInput_TestAction(chr, 0x2C, 1)) {
                BtlAct_Request(chr, 0x3E);
            }
        }
        queued |= BtlDecide_Common(chr, 0xE);
        if (queued) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
    }
}

extern f32 BtlMoveParam_GetHeight(BtlActDChr *chr, s32 kind); /* jump height, table +0x5C[kind] * 10 */
extern s32 BtlAct_GetAction14or16(BtlActDChr *chr);
extern s32 BtlAct_GetAction11or15(BtlActDChr *chr);

/* Actions 0x10 and 0x12: jump take-off (motion 0x1F), then action 0x11 or 0x13. */
s32 BtlAct_JumpStartHandler(BtlActDChr *chr, s32 phase) {
    f32 height;
    f32 speed;
    BtlActDPose *pose;

    if (phase == PHASE_ENTER) {
        BtlAnim_Play(chr, 0x1F, 0.1f);
        BtlChar_ClearFlag(chr, 0xE);
        BtlCharSnd_PlayCommon(chr, 0x31);
        chr->unkFF0 = 0;
        if (BtlAct_GetCurrent(chr) == 0x12) {
            chr->work[0] |= 1;
        }
    }
    if (phase == PHASE_RUN) {
        if (BtlAnim_Advance(chr, 1)) {
            BtlAct_Request(chr, (chr->work[0] & 1) ? 0x13 : 0x11);
        }
        if (BtlAnim_PassedRatio(chr, 0.3f)) {
            height = BtlMoveParam_GetHeight(chr, chr->work[0] & 1);
            pose = BtlChar_GetPos(chr);
            pose->fallSpeed = BtlMove_CalcJumpSpeed(height);
        }
        speed = BtlMoveParam_GetSpeed(chr, (chr->work[0] & 1) ? 4 : 0xD);
        BtlMove_Step(chr, 0, 5, 3, speed * BtlInput_GetStickLength(chr), BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xC9);
        BtlMove_RequestOrbit(chr, 20.0f, 50.0f);
        BtlChar_SetFlag(chr, 0x1A);
        BtlChar_SetFxBit(chr, 0x3A);
        if (BtlMember_GetActiveGauge(chr)->unk28 != 0) {
            BtlChar_SetFlag(chr, 0x96);
        }
    }
    if (phase == PHASE_DECIDE) {
        if (BtlInput_TestAction(chr, 9, 1)) {
            BtlAct_Request(chr, BtlAct_GetAction14or16(chr));
        }
        if (BtlParam_CanFly(chr)) {
            if (BtlInput_TestAction(chr, 0xC, 1)) {
                BtlAct_Request(chr, BtlAct_GetAction11or15(chr));
            }
        }
        if (BtlInput_TestAction(chr, 7, 1)) {
            BtlAct_Request(chr, 0xF);
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        BtlDecide_Main(chr, 0x1C00);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
        if (BtlInput_TestAction(chr, 0x2C, 1)) {
            BtlAct_Request(chr, 0x3F);
        }
        if (BtlDecide_Attack(chr, 0x70000)) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
    }
}

/* Actions 0x11 and 0x13: jump in the air (motions 0x20 rising, 0x21 falling, 0x22 landing), then action 0xB. */
s32 BtlAct_JumpAirHandler(BtlActDChr *chr, s32 phase) {
    f32 turn;
    f32 speed;
    f32 accel;

    if (phase == PHASE_ENTER) {
        if (0.0f <= BtlChar_GetPos(chr)->fallSpeed) {
            BtlAnim_Play(chr, 0x21, 0.15f);
        } else {
            BtlAnim_Play(chr, 0x20, 0.15f);
        }
        BtlChar_ClearFlag(chr, 0xE);
    }
    if (phase == PHASE_RUN) {
        speed = 0.0f;
        accel = BTL_KMH(50.0f);
        turn = BtlMoveParam_GetTurnRate(chr, 1);
        switch (BtlAct_GetCurrent(chr)) {
            case 0x11:
                speed = BtlMoveParam_GetSpeed(chr, 0xD);
                break;
            case 0x13:
                speed = BtlMoveParam_GetSpeed(chr, 4);
                break;
        }
        switch (BtlAnim_GetId(chr)) {
            case 0x20:
                BtlAnim_AdvanceLoop(chr, 0);
                if (0.0f <= BtlChar_GetPos(chr)->fallSpeed) {
                    BtlAnim_Request(chr, 0x21, 0.15f);
                }
                break;
            case 0x21:
                BtlAnim_AdvanceLoop(chr, 0);
                if (BtlAct_GetFramesToGround(chr) < 3.0f) {
                    if (!BtlChar_IsStage4Or27() || BtlChar_TestFlag(chr, 7) || BtlAct_GetGroundY(chr) < -10.0f) {
                        BtlAnim_Request(chr, 0x22, 0.1f);
                    }
                }
                break;
            case 0x22:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xB);
                }
                speed = 0.0f;
                accel = BTL_KMH(100.0f);
                if (BtlAnim_PassedRatio(chr, 0.1f)) {
                    BtlAct_PlayLandFx(chr, 0);
                }
                break;
        }
        speed *= BtlInput_GetStickLength(chr);
        BtlMove_TurnYaw(chr, 0, turn);
        BtlMove_TurnPitch(chr, 5, 3.14159265f);
        BtlMove_TurnModelYaw(chr, BTL_DEG(36.0f), 0.3f);
        BtlMove_SetDirection(chr, 3);
        BtlMove_Advance(chr, speed, accel);
        BtlMove_ApplyGravity(chr);
        BtlMove_RequestOrbit(chr, 20.0f, 50.0f);
        BtlChar_SetFlag(chr, 0x1A);
        BtlChar_SetFxBit(chr, 0x3A);
        if (BtlMember_GetActiveGauge(chr)->unk28 != 0) {
            BtlChar_SetFlag(chr, 0x96);
        }
    }
    if (phase == PHASE_DECIDE) {
        if (BtlChar_TestFlag(chr, 0x16)) {
            BtlChar_SetHeldFlag(chr, 0xE);
            BtlAct_Request(chr, 0xB);
        }
        if (BtlChar_IsStage4Or27()) {
            if (BtlChar_TestFlag(chr, 0x17)) {
                BtlChar_SetHeldFlag(chr, 0xE);
                BtlAct_Request(chr, 0xB);
            }
        }
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
            BtlAct_Request(chr, 0xB);
        }
        if (BtlInput_TestAction(chr, 0xA, 1)) {
            BtlAct_Request(chr, BtlAct_GetAction14or16(chr));
        }
        if (BtlParam_CanFly(chr)) {
            if (BtlInput_TestAction(chr, 0xC, 1)) {
                BtlAct_Request(chr, BtlAct_GetAction11or15(chr));
            }
        }
        BtlDecide_Main(chr, (BtlAnim_GetId(chr) != 0x22) ? 0x1C00 : 0x1C02);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
        if (BtlInput_TestAction(chr, 7, 1)) {
            BtlAct_Request(chr, 0xF);
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlAnim_GetId(chr) != 0x22) {
            if (BtlInput_TestAction(chr, 0x2C, 1)) {
                BtlAct_Request(chr, 0x3F);
            }
            if (BtlDecide_Attack(chr, 0x70000)) {
                BtlAct_Request(chr, BtlAct_GetQueued(chr));
            }
        }
    }
}

extern f32 BtlMoveParam_GetAccel(BtlActDChr *chr, s32 kind); /* vertical acceleration, table +0x50[kind] */

/* Action 0x14: ascend (motion 0x2B, entered through 0x2E after a jump). */
s32 BtlAct_AscendHandler(BtlActDChr *chr, s32 phase) {
    s32 motion;
    f32 speed;
    f32 vspeed;
    f32 vaccel;

    if (phase == PHASE_ENTER) {
        motion = 0x2B;
        if (chr->unkFF0 > 0) {
            motion = 0x2E;
            BtlCharSnd_PlayCommon(chr, 0x1F);
        }
        BtlAnim_Play(chr, motion, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
    }
    if (phase == PHASE_RUN) {
        switch (BtlAnim_GetId(chr)) {
            case 0x2E:
                BtlAnim_SetDuration(chr, 0.35f);
                BtlAnim_AdvanceThen(chr, 0x2B, 0.15f, 0);
                BtlMove_Step(chr, 6, 5, 3, 0.0f, BTL_KMH(50.0f));
                BtlMove_ApplyGravity(chr);
                break;
            case 0x2B:
                if (BtlAnim_IsNew(chr)) {
                    BtlCharSnd_PlayCommon(chr, 0x18);
                }
                BtlAnim_AdvanceLoop(chr, 0);
                speed = BtlMoveParam_GetSpeed(chr, BtlChar_TestFlag(chr, 0x11)) * BtlInput_GetStickLength(chr);
                vspeed = -BtlMoveParam_GetSpeed(chr, 0x10);
                vaccel = BtlMoveParam_GetAccel(chr, 0);
                BtlMove_Step(chr, 0, 5, 3, speed, BTL_KMH(50.0f));
                BtlMove_MoveVertical(chr, vspeed, vaccel);
                break;
        }
        BtlChar_SetFlag(chr, 0xC9);
        BtlMove_RequestOrbit(chr, 20.0f, 50.0f);
        if (BtlMember_GetActiveGauge(chr)->unk28 != 0) {
            BtlChar_SetFlag(chr, 0x96);
        }
        BtlChar_SetFxBit(chr, 0x3A);
    }
    if (phase == PHASE_DECIDE) {
        if (BtlInput_TestAction(chr, 0xB, 1)) {
            BtlAct_Request(chr, 0xB);
            BtlDecide_Main(chr, 0x41);
        }
        BtlDecide_Main(chr, 0x1800);
        BtlDecide_Attack(chr, 0x70000);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}

/* Action 0x15: descend (motion 0x2C, entered through 0x1F after a jump), landing with motion 0x2D. */
s32 BtlAct_DescendHandler(BtlActDChr *chr, s32 phase) {
    s32 motion;
    f32 speed;
    f32 vspeed;
    f32 vaccel;

    if (phase == PHASE_ENTER) {
        motion = 0x2C;
        if (chr->unkFF0 > 0) {
            motion = 0x1F;
            BtlCharSnd_PlayCommon(chr, 0x1F);
        }
        BtlAnim_Play(chr, motion, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
    }
    if (phase == PHASE_RUN) {
        speed = BtlMoveParam_GetSpeed(chr, BtlChar_TestFlag(chr, 0x11)) * BtlInput_GetStickLength(chr);
        vspeed = BtlMoveParam_GetSpeed(chr, 0x11);
        vaccel = BtlMoveParam_GetAccel(chr, 1);
        switch (BtlAnim_GetId(chr)) {
            case 0x1F:
                BtlAnim_SetDuration(chr, 0.35f);
                BtlAnim_AdvanceThen(chr, 0x2C, 0.15f, 0);
                break;
            case 0x2C:
                if (BtlAnim_IsNew(chr)) {
                    BtlCharSnd_PlayCommon(chr, 0x1A);
                }
                BtlAnim_AdvanceLoop(chr, 0);
                if (BtlAct_GetFramesToGround(chr) < 1.0f) {
                    if (!BtlChar_IsStage4Or27() || BtlChar_TestFlag(chr, 7) || BtlAct_GetGroundY(chr) < -10.0f) {
                        if (!(BtlChar_GetPos(chr)->unkD0 & 0x40)) {
                            BtlAnim_Request(chr, 0x2D, 0.0f);
                            BtlAct_PlayLandFx(chr, 1);
                        }
                    }
                }
                BtlMove_Step(chr, 0, 5, 3, speed, BTL_KMH(50.0f));
                BtlMove_MoveVertical(chr, vspeed, vaccel);
                BtlChar_SetFlag(chr, 0xC9);
                break;
            case 0x2D:
                BtlChar_ClearFlag(chr, 0xE);
                if (BtlAnim_Advance(chr, 0)) {
                    chr->work[1] = 1;
                }
                BtlMove_Step(chr, 0, 5, 3, 0.0f, BTL_KMH(100.0f));
                BtlMove_ApplyGravity(chr);
                break;
        }
        BtlMove_RequestOrbit(chr, 20.0f, 50.0f);
        if (BtlMember_GetActiveGauge(chr)->unk28 != 0) {
            BtlChar_SetFlag(chr, 0x96);
        }
        BtlChar_SetFxBit(chr, 0x3A);
    }
    if (phase == PHASE_DECIDE) {
        switch (BtlAnim_GetId(chr)) {
            case 0x1F:
            case 0x2C:
                if (BtlInput_TestAction(chr, 0xD, 1)) {
                    BtlAct_Request(chr, 0xB);
                    BtlDecide_Main(chr, 0x21);
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                }
                break;
            case 0x2D:
                if (chr->work[1] != 0) {
                    if (BtlInput_TestAction(chr, 1, 1)) {
                        BtlAct_Request(chr, 0xD);
                    } else {
                        BtlAct_Request(chr, 0xB);
                    }
                }
                break;
        }
        BtlDecide_Main(chr, 0x1800);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}

extern s32 BtlParam_GetDashSound(BtlActDChr *chr);            /* burst sound: 0x1D with parameter flag 0x10, else 0x1C */
extern s32 BtlMoveParam_GetKiCost(BtlActDChr *chr, s32 kind);  /* ki drain per frame, table +0x64[kind] / 30 */

/* Action 0x16: a short rise that fades out over motion 0x18C, then action 0xB. */
s32 BtlAct_HopHandler(BtlActDChr *chr, s32 phase) {
    f32 fade;
    f32 speed;
    f32 vspeed;
    f32 vaccel;

    if (phase == PHASE_ENTER) {
        BtlAnim_Play(chr, 0x18C, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlCharSnd_PlayCommon(chr, 0x1F);
        BtlChar_GetPos(chr)->fallSpeed = 0.0f;
    }
    if (phase == PHASE_RUN) {
        fade = 1.0f;
        speed = BtlMoveParam_GetSpeed(chr, BtlChar_TestFlag(chr, 0x11));
        vspeed = BtlMoveParam_GetSpeed(chr, 0x10);
        vaccel = BtlMoveParam_GetAccel(chr, 0);
        fade -= BtlAnim_GetProgress(chr);
        speed *= BtlInput_GetStickLength(chr) * fade;
        vspeed *= fade;
        vaccel *= 0.5f;
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0xB);
        }
        BtlMove_Step(chr, 0, 5, 2, speed, BTL_KMH(50.0f));
        BtlMove_MoveVertical(chr, -vspeed, vaccel);
        BtlChar_SetFlag(chr, 0xC9);
        BtlMove_RequestOrbit(chr, 20.0f, 50.0f);
        BtlChar_SetFxBit(chr, 0x3A);
    }
    if (phase == PHASE_DECIDE) {
        BtlDecide_Main(chr, 0x1800);
        BtlDecide_Attack(chr, 0x70000);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}

/* Action 0x17: fast ascend (motions 0x2E start, 0x2F loop, 0x30 stop); drains ki while it lasts. */
s32 BtlAct_FastAscendHandler(BtlActDChr *chr, s32 phase) {
    f32 vspeed;

    if (phase == PHASE_ENTER) {
        BtlAnim_Play(chr, 0x2E, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
        if (chr->unkFF0 > 0) {
            chr->work[0] |= 1;
        }
    }
    if (phase == PHASE_RUN) {
        vspeed = -BtlMoveParam_GetSpeed(chr, 9);
        switch (BtlAnim_GetId(chr)) {
            case 0x2E:
                if (chr->work[0] & 1) {
                    BtlAnim_SetDuration(chr, 0.5f);
                }
                if (BtlAnim_AdvanceThen(chr, 0x2F, 0.15f, 0)) {
                    BtlCharSnd_PlayCommon(chr, BtlParam_GetDashSound(chr));
                    BtlChar_Vibrate(chr, 0.8f, 0.3f);
                }
                BtlMove_MoveVertical(chr, 0.0f, BTL_KMH(100.0f));
                break;
            case 0x2F:
                BtlAnim_AdvanceLoop(chr, 0);
                BtlMove_MoveVertical(chr, vspeed, 10000.0f);
                BtlChar_SetFxBit(chr, 4);
                BtlChar_SetFlag(chr, 9);
                BtlChar_SetVibration(chr, 0.7f, 0.1f);
                break;
            case 0x30:
                if (BtlAnim_Advance(chr, 0)) {
                    chr->work[1] = 1;
                }
                BtlMove_MoveVertical(chr, 0.0f, BTL_KMH(100.0f));
                break;
        }
        BtlMove_Step(chr, 6, 5, 3, 0.0f, BTL_KMH(100.0f));
        BtlChar_SetFxBit(chr, 0x3A);
    }
    if (phase == PHASE_DECIDE) {
        switch (BtlAnim_GetId(chr)) {
            case 0x2E:
                break;
            case 0x2F: {
                s32 stop = 0;

                if (BtlInput_TestAction(chr, 0x10, 1)) {
                    stop = 1;
                }
                if (BtlMember_SpendKi(chr, BtlMoveParam_GetKiCost(chr, 2), 0)) {
                    stop = 1;
                }
                if (BtlChar_TestFlag(chr, 0x15)) {
                    stop = 1;
                }
                if (stop) {
                    BtlAnim_Request(chr, 0x30, 0.15f);
                }
                break;
            }
            case 0x30:
                if (chr->work[1] != 0) {
                    BtlAct_Request(chr, 0xB);
                    BtlDecide_Main(chr, 1);
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                }
                break;
        }
        BtlDecide_Main(chr, 0x1400);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}

/* Action 0x18: fast descend (motions 0x31 start, 0x32 loop, 0x33 stop); drains ki while it lasts. */
s32 BtlAct_FastDescendHandler(BtlActDChr *chr, s32 phase) {
    f32 vspeed;

    if (phase == PHASE_ENTER) {
        BtlAnim_Play(chr, 0x31, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
        if (chr->unkFF0 > 0) {
            chr->work[0] |= 1;
        }
    }
    if (phase == PHASE_RUN) {
        vspeed = BtlMoveParam_GetSpeed(chr, 9);
        switch (BtlAnim_GetId(chr)) {
            case 0x31:
                if (chr->work[0] & 1) {
                    BtlAnim_SetDuration(chr, 0.5f);
                }
                if (BtlAnim_AdvanceThen(chr, 0x32, 0.15f, 0)) {
                    BtlCharSnd_PlayCommon(chr, BtlParam_GetDashSound(chr));
                    BtlChar_Vibrate(chr, 0.8f, 0.3f);
                }
                BtlMove_MoveVertical(chr, 0.0f, BTL_KMH(100.0f));
                break;
            case 0x32:
                BtlAnim_AdvanceLoop(chr, 0);
                BtlMove_MoveVertical(chr, vspeed, 10000.0f);
                BtlChar_SetFxBit(chr, 4);
                BtlChar_SetFlag(chr, 9);
                BtlChar_SetVibration(chr, 0.7f, 0.1f);
                break;
            case 0x33:
                if (BtlAnim_Advance(chr, 0)) {
                    chr->work[1] = 1;
                }
                BtlMove_MoveVertical(chr, 0.0f, BTL_KMH(100.0f));
                break;
        }
        BtlMove_Step(chr, 6, 5, 3, 0.0f, BTL_KMH(100.0f));
        BtlChar_SetFxBit(chr, 0x3A);
    }
    if (phase == PHASE_DECIDE) {
        switch (BtlAnim_GetId(chr)) {
            case 0x31:
                break;
            case 0x32: {
                s32 stop = 0;

                if (BtlInput_TestAction(chr, 0x12, 1)) {
                    stop = 1;
                }
                if (!(BtlChar_GetPos(chr)->unkD0 & 0x40000000) || (BtlChar_GetPos(chr)->unkD0 & 0x01000000)) {
                    if (BtlAct_GetFramesToGround(chr) < 0.3f * 30.0f) {
                        stop = 1;
                    }
                }
                if (BtlChar_TestFlag(chr, 0x16)) {
                    stop = 1;
                }
                if (BtlMember_SpendKi(chr, BtlMoveParam_GetKiCost(chr, 2), 0)) {
                    stop = 1;
                }
                if (stop) {
                    BtlAnim_Request(chr, 0x33, 0.15f);
                }
                break;
            }
            case 0x33:
                if (chr->work[1] != 0) {
                    BtlAct_Request(chr, 0xB);
                    BtlDecide_Main(chr, 1);
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                }
                break;
        }
        BtlDecide_Main(chr, 0xC00);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}

extern f32 BtlMoveParam_GetTurnAccel(BtlActDChr *chr, s32 axis); /* steering acceleration, table +0x80[axis] in degrees */
extern f32 BtlMoveParam_GetTurnMax(BtlActDChr *chr, s32 axis); /* steering speed limit */

/* Action 0x19: ki-draining dash that homes on the opponent (motions 0x18 / 0x2D start, 0x19 loop, 0x1A / 0x15 stop). */
s32 BtlAct_HomingDashHandler(BtlActDChr *chr, s32 phase) {
    s32 *slowFrames = &chr->work[2];
    s32 prev;
    s32 motion;
    s32 voice;
    s32 moving;
    s32 faceOpp;
    s32 first;
    s32 yawMode;
    f32 lean;
    f32 speed;
    f32 yawAccel;
    f32 pitchAccel;
    f32 yawMax;
    f32 pitchMax;
    f32 modelTurn;
    f32 near;
    BtlActDPose *pose;

    if (phase == PHASE_ENTER) {
        voice = -1;
        prev = BtlAct_GetPrev(chr);
        if (BtlAct_IsAttackId(prev)) {
            motion = 0x19;
            chr->unkD68++;
        } else {
            switch (prev) {
                case 0x47 ... 0x52:
                    voice = 0x1C;
                case 0x53 ... 0x57:
                case 0x5A ... 0x5D:
                case 0x6B ... 0x6F:
                    chr->unkD68++;
                case 0x1A:
                case 0x28:
                    motion = 0x19;
                    break;
                case 0x2A:
                case 0xD7:
                    motion = 0x19;
                    break;
                case 0x17:
                case 0x18:
                    motion = 0x2D;
                    break;
                default:
                    motion = 0x18;
                    voice = 0x24;
                    break;
            }
        }
        BtlAnim_Play(chr, motion, 0.15f);
        if (voice >= 0) {
            BtlChar_PlayVoice(chr, voice);
        }
        BtlChar_SetHeldFlag(chr, 0xE);
    }
    if (phase == PHASE_RUN) {
        lean = 0.0f;
        moving = 1;
        faceOpp = 1;
        first = 0;
        switch (BtlAnim_GetId(chr)) {
            case 0x18:
            case 0x2D:
                BtlAnim_AdvanceThen(chr, 0x19, 0.2f, 0);
                lean = BtlAnim_GetProgress(chr);
                moving = 0;
                if (0.5f < BtlAnim_GetProgress(chr)) {
                    BtlChar_SetFlag(chr, 0x47);
                }
                break;
            case 0x19:
                BtlAnim_AdvanceLoop(chr, 0);
                if (BtlAnim_IsNew(chr)) {
                    first = 1;
                    BtlCharSnd_PlayCommon(chr, BtlParam_GetDashSound(chr));
                    BtlChar_SetFlag(chr, 0xD1);
                    BtlChar_Vibrate(chr, 0.8f, 0.3f);
                    if (BtlAct_GetHeight(chr) < 20.0f) {
                        BtlChar_SetFxBit(chr, 0x35);
                    }
                }
                BtlChar_SetFlag(chr, 0x47);
                lean = 1.0f;
                moving = 1;
                BtlChar_SetVibration(chr, 0.7f, 0.1f);
                break;
            case 0x15:
            case 0x1A:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xB);
                    BtlChar_GetPos(chr)->speed = 0.0f;
                }
                moving = 0;
                faceOpp = 0;
                lean = 1.0f - BtlAnim_GetProgress(chr);
                break;
        }
        if (moving) {
            speed = BtlMoveParam_GetSpeed(chr, 6);
            yawAccel = BtlMoveParam_GetTurnAccel(chr, 0);
            pitchAccel = BtlMoveParam_GetTurnAccel(chr, 1);
            yawMax = BtlMoveParam_GetTurnMax(chr, 0);
            pitchMax = BtlMoveParam_GetTurnMax(chr, 1);
            modelTurn = BTL_DEG(36.0f);
            if (first) {
                pitchAccel = 3.14159265f;
                yawAccel = pitchAccel;
                modelTurn = pitchAccel;
            }
            near = BtlUtil_MinF(BtlOpp_GetGapXZ(chr) * 0.01f, 1.0f);
            BtlMove_SteerAtOpponent(chr, speed, yawAccel, pitchAccel, yawMax * near, pitchMax * near, 3.14159265f);
            BtlMove_TurnModelYaw(chr, modelTurn, 0.5f);
            BtlMove_SetDirection(chr, 3);
            BtlMove_Advance(chr, speed, 10000.0f);
            BtlMove_ApplyGravity(chr);
            BtlChar_SetFlag(chr, 9);
            BtlChar_SetFlag(chr, 0x50);
            BtlChar_SetFlag(chr, 0x51);
            BtlChar_SetFxBit(chr, 4);
            BtlChar_SetFxBit(chr, 9);
        } else {
            yawMode = faceOpp ? 2 : 6;
            if (BtlMove_IsBlockedByOpponent(chr)) {
                BtlChar_GetPos(chr)->speed = 0.0f;
            }
            BtlMove_Step(chr, yawMode, 6, 7, 0.0f, BTL_KMH(100.0f));
            BtlMove_ApplyGravity(chr);
        }
        BtlChar_SetFlag(chr, 0x89);
        BtlMove_SetLeanX(chr, BtlChar_GetPos(chr)->pitch * lean);
        BtlMove_RequestOrbit(chr, 50.0f, 100.0f);
        BtlChar_SetFxBit(chr, 0x3A);
    }
    if (phase == PHASE_DECIDE) {
        switch (BtlAnim_GetId(chr)) {
            case 0x18:
            case 0x2D:
                break;
            case 0x19: {
                s32 stop = 0;

                if (BtlMove_IsBlockedByOpponent(chr)) {
                    if (BtlOpp_GetSeenAction(chr) != 0xD5) {
                        stop = 1;
                    }
                }
                if (Vec3_Length(&BtlChar_GetPos(chr)->unk40) < BTL_KMH(100.0f)) {
                    if (++(*slowFrames) >= 16) {
                        stop = 1;
                    }
                } else {
                    *slowFrames = 0;
                }
                if (!BtlChar_TestFlag(chr, 5)) {
                    stop = 1;
                }
                if (BtlMember_SpendKi(chr, BtlMoveParam_GetKiCost(chr, 0), 0)) {
                    stop = 1;
                }
                if (BtlInput_TestAction(chr, 0x16, 1)) {
                    BtlAnim_Request(chr, 0x15, 0.0f);
                }
                if (BtlInput_TestAction(chr, 0x15, 1)) {
                    BtlAnim_Request(chr, 0x15, 0.0f);
                    chr->work[0] |= 1;
                }
                if (BtlDecide_Main(chr, 0x08000000)) {
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                }
                if (stop) {
                    BtlAnim_Request(chr, 0x1A, 0.0f);
                    pose = BtlChar_GetPos(chr);
                    pose->speed *= 0.5f;
                }
                break;
            }
            case 0x1A:
                if (BtlMove_IsBlockedByOpponent(chr)) {
                    BtlChar_GetPos(chr)->speed = 0.0f;
                }
                break;
            case 0x15:
                if (BtlInput_TestAction(chr, 0x18, 1)) {
                    chr->work[0] |= 1;
                }
                if (0.5f < BtlAnim_GetProgress(chr)) {
                    if (chr->work[0] & 1) {
                        BtlAct_Request(chr, 0x1A);
                    }
                }
                if (BtlDecide_Main(chr, 0x08000000)) {
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                }
                break;
        }
        BtlDecide_Main(chr, 0x1800);
        switch (BtlAnim_GetId(chr)) {
            case 0x19:
                BtlDecide_Attack(chr, 0x2000);
                break;
            case 0x1A:
                BtlDecide_Attack(chr, 3);
                break;
        }
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}


/* ======== merged from src/battle/btl_act_e.c ======== */


/*
 * Fighter action handlers, 0x1F1930..0x1F5460: actions 0x1A..0x35 and 0xFA..0xFC.
 * Handler protocol (phase 0 enter, 1 run, 2 decide, 3 leave): include/battle/btl_char_action.h.
 *
 * Return types: most handlers are int functions without a return statement, as elsewhere. Those whose
 * leave phase ends in a tail call in the original (0x25..0x2E, 0x31, 0x32, 0x34, 0xFA) only match as void.
 *
 * "Speed kind n" below is BtlMoveParam_GetSpeed(chr, n), "ki drain kind n" is BtlMoveParam_GetKiCost(chr, n): per-character
 * parameters owned by a module that is not decompiled yet.
 */

#define BTL_KMH(x) ((x) * 1000.0f / 3600.0f * (1.0f / 30.0f))
#define BTL_DEG(x) ((x) / 180.0f * 3.14159265f)
/* A second degrees macro: the two angles of BtlAct_SlideToAttackHandler only come out right in this order. */
#define BTL_DEG2RAD(x) ((x) * 3.14159265f / 180.0f)

extern f32 Mathf_Sin(f32 a);
extern f32 Mathf_Cos(f32 a);
extern f32 Vec3_Length(Vec4 *v);
extern f32 BtlUtil_WrapAngle(f32 a);
extern f32 sqrtf(f32 x);
extern void Vec3_Normalize(Vec4 *out, Vec4 *in);
extern void Vec4_Add(Vec4 *out, Vec4 *a, Vec4 *b);
extern void Vec4_SetZero(Vec4 *v); /* zeroes a vector */
extern void BtlEvent_Raise(s32 side, s32 event);
extern void BtlChar_AddStageTimer(f32 seconds);
extern void BtlChar_SetSmallVibration(BtlActEChr *chr, f32 seconds);
extern void BtlChar_SetFrameBits(BtlActEChr *chr, u64 bits);
extern void BtlOpp_GetTargetRot(BtlActEChr *chr, Vec4 *out);
extern void BtlOpp_GetDelta(BtlActEChr *chr, Vec4 *out);
extern void BtlOpp_GetVelocity(BtlActEChr *chr, Vec4 *out);
extern void BtlMove_WarpBehindOpponent(BtlActEChr *chr, f32 gap);
extern void BtlMove_SnapToOpponent(BtlActEChr *chr);
extern void BtlMove_WarpAheadOfOpponent(BtlActEChr *chr, f32 lead);
extern void BtlMove_CalcApproachPoint(BtlActEChr *chr, Vec4 *out, Vec4 *outTarget, f32 dist, f32 angle, f32 distScale, f32 lead);
extern void BtlMove_TurnToPoint(BtlActEChr *chr, Vec4 *target, f32 yawStep, f32 pitchStep);
extern void Vec4_Sub(Vec4 *out, Vec4 *a, Vec4 *b);
extern void Vec4_Scale(Vec4 *out, Vec4 *in, f32 scale);
extern void Vec4_Copy(Vec4 *out, Vec4 *in);
extern f32 floorf(f32 x);
#define BtlAnim_PlaySub ((void (*)(BtlActEChr *chr, s32 anim))BtlAnim_PlaySub)
#define BtlAct_IsAirMotion ((s32 (*)(BtlActEChr *chr, s32 useSaved))BtlAct_IsAirMotion)
extern s32 BtlParam_GetRecoverKiCost(BtlActEChr *chr);               /* character parameter: a ki cost */
#define BtlObjAnim_QueryEvent ((s32 (*)(BtlActEObj *obj, u64 a, s32 b, s32 c))BtlObjAnim_QueryEvent)

#define BtlChar_GetPos ((BtlActEPose *(*)(BtlActEChr *chr))BtlChar_GetPos)
#define BtlChar_GetObj ((BtlActEObj *(*)(BtlActEChr *chr))BtlChar_GetObj)
#define BtlChar_TestFlag ((s32 (*)(BtlActEChr *chr, s32 n))BtlChar_TestFlag)
#define BtlChar_SetFlag ((void (*)(BtlActEChr *chr, s32 n))BtlChar_SetFlag)
#define BtlChar_SetHeldFlag ((void (*)(BtlActEChr *chr, s32 n))BtlChar_SetHeldFlag)
#define BtlChar_ClearFlag ((void (*)(BtlActEChr *chr, s32 n))BtlChar_ClearFlag)
#define BtlChar_PlayVoice ((void (*)(BtlActEChr *chr, s32 kind))BtlChar_PlayVoice)
#define BtlChar_Vibrate ((void (*)(BtlActEChr *chr, f32 power, f32 seconds))BtlChar_Vibrate)
#define BtlChar_SetVibration ((void (*)(BtlActEChr *chr, f32 power, f32 seconds))BtlChar_SetVibration)
#define BtlChar_SetFxBit ((void (*)(BtlActEChr *chr, s32 n))BtlChar_SetFxBit)
#define BtlCharSnd_PlayCommon ((void (*)(BtlActEChr *chr, s32 id))BtlCharSnd_PlayCommon)
#define BtlCharSnd_PlayBank8 ((void (*)(BtlActEChr *chr, s32 id))BtlCharSnd_PlayBank8)
#define BtlInput_IsHeld ((s32 (*)(BtlActEChr *chr, u32 mask))BtlInput_IsHeld)
#define BtlInput_TestAction ((s32 (*)(BtlActEChr *chr, s32 id, s32 want))BtlInput_TestAction)
#define BtlMember_GetActiveGauge ((BtlActEGauge *(*)(BtlActEChr *chr))BtlMember_GetActiveGauge)
#define BtlMember_SpendKi ((s32 (*)(BtlActEChr *chr, s32 amount, s32 force))BtlMember_SpendKi)
extern f32 BtlOpp_GetYaw(BtlActEChr *chr);
extern f32 BtlOpp_GetClosingTime(BtlActEChr *chr);
#define BtlInput_IsPressed ((s32 (*)(BtlActEChr *chr, u32 mask))BtlInput_IsPressed)
extern void BtlCharSnd_PlayCommonFar(BtlActEChr *chr, s32 id);
extern void BtlChar_RequestPlaceOnPath(BtlActEChr *chr);
extern void BtlAnim_EnableHandle(BtlActEChr *chr);
#define BtlAct_GetRequested ((s32 (*)(BtlActEChr *chr))BtlAct_GetRequested)

extern BtlActERoster *gBtlChars;
extern s32 BtlOpp_GetClashCountB(BtlActEChr *chr);
extern void BtlChar_ClearFlagRange(BtlActEChr *chr, u32 a, u32 b);
#define BtlMember_HasAbility ((s32 (*)(BtlActEChr *chr, s32 n))BtlMember_HasAbility)
#define BtlMember_AddKi ((s32 (*)(BtlActEChr *chr, s32 amount))BtlMember_AddKi)
#define BtlMember_AddMaxPower ((void (*)(BtlActEChr *chr, s32 amount))BtlMember_AddMaxPower)
#define BtlMember_Damage ((s32 (*)(BtlActEChr *chr, s32 amount, s32 flags))BtlMember_Damage)
#define BtlAnim_PassedRatio ((s32 (*)(BtlActEChr *chr, f32 ratio))BtlAnim_PassedRatio)
#define BtlMove_SteerAtOpponent ((void (*)(BtlActEChr *chr, f32 closeSpeed, f32 yawAccel, f32 pitchAccel, f32 yawMax, f32 pitchMax, f32 maxStep))BtlMove_SteerAtOpponent)
extern s32 BtlAct_IsTechniqueId(s32 id);
extern s32 BtlAct_GetPrevClass(BtlActEChr *chr);
extern void BtlAct_CountAndMarkOpponent(BtlActEChr *chr);
extern s32 BtlSuper_GetKiCost(BtlActEChr *chr, s32 cls);
#define BtlOpp_GetYawFromFacing ((f32 (*)(BtlActEChr *chr))BtlOpp_GetYawFromFacing)
#define BtlParam_GetFlags ((s32 (*)(BtlActEChr *chr))BtlParam_GetFlags)               /* character parameter flags */
extern f32 BtlOpp_GetYawFromItsFacing(BtlActEChr *chr);
#define BtlOpp_GetSeenAction ((s32 (*)(BtlActEChr *chr))BtlOpp_GetSeenAction)
#define BtlAnim_SetDuration ((void (*)(BtlActEChr *chr, f32 seconds))BtlAnim_SetDuration)
extern s32 BtlMove_CircleOpponent(BtlActEChr *chr, s32 side, s32 lead, f32 baseYaw, f32 speed);
#define BtlMove_SetLeanX ((void (*)(BtlActEChr *chr, f32 v))BtlMove_SetLeanX)
#define ChrCam_RequestCut ((void (*)(BtlActEChr *chr, s32 arg1, s32 arg2))ChrCam_RequestCut)

#define BtlAnim_Play ((void (*)(BtlActEChr *chr, s32 anim, f32 blend))BtlAnim_Play)
#define BtlAnim_Request ((void (*)(BtlActEChr *chr, s32 anim, f32 blend))BtlAnim_Request)
#define BtlAnim_GetId ((s32 (*)(BtlActEChr *chr))BtlAnim_GetId)
#define BtlAnim_GetProgress ((f32 (*)(BtlActEChr *chr))BtlAnim_GetProgress)
extern f32 BtlAnim_GetLength(BtlActEChr *chr);
extern f32 BtlAnim_GetStep(BtlActEChr *chr);
#define BtlAnim_Advance ((s32 (*)(BtlActEChr *chr, s32 flags))BtlAnim_Advance)
#define BtlAnim_AdvanceThen ((s32 (*)(BtlActEChr *chr, s32 next, f32 blend, s32 flags))BtlAnim_AdvanceThen)
#define BtlAnim_AdvanceLoop ((void (*)(BtlActEChr *chr, s32 flags))BtlAnim_AdvanceLoop)
#define BtlAnim_IsNew ((s32 (*)(BtlActEChr *chr))BtlAnim_IsNew)

#define BtlMove_TurnYaw ((void (*)(BtlActEChr *chr, s32 mode, f32 maxStep))BtlMove_TurnYaw)
#define BtlMove_TurnPitch ((void (*)(BtlActEChr *chr, s32 mode, f32 maxStep))BtlMove_TurnPitch)
#define BtlMove_TurnModelYaw ((void (*)(BtlActEChr *chr, f32 maxStep, f32 rate))BtlMove_TurnModelYaw)
#define BtlMove_SetDirection ((void (*)(BtlActEChr *chr, s32 mode))BtlMove_SetDirection)
#define BtlMove_Advance ((void (*)(BtlActEChr *chr, f32 speed, f32 accel))BtlMove_Advance)
#define BtlMove_Step ((void (*)(BtlActEChr *chr, s32 yawMode, s32 pitchMode, s32 dirMode, f32 speed, f32 accel))BtlMove_Step)
#define BtlMove_MoveVertical ((void (*)(BtlActEChr *chr, f32 speed, f32 accel))BtlMove_MoveVertical)
#define BtlMove_ApplyGravity ((void (*)(BtlActEChr *chr))BtlMove_ApplyGravity)
#define BtlMove_IsBlockedByOpponent ((s32 (*)(BtlActEChr *chr))BtlMove_IsBlockedByOpponent)
#define BtlMove_RequestOrbit ((void (*)(BtlActEChr *chr, f32 near, f32 far))BtlMove_RequestOrbit)

#define BtlAct_Request ((void (*)(BtlActEChr *chr, s32 id))BtlAct_Request)
#define BtlAct_HasQueued ((s32 (*)(BtlActEChr *chr))BtlAct_HasQueued)
#define BtlAct_GetCurrent ((s32 (*)(BtlActEChr *chr))BtlAct_GetCurrent)
#define BtlAct_GetPrev ((s32 (*)(BtlActEChr *chr))BtlAct_GetPrev)
#define BtlAct_GetQueued ((s32 (*)(BtlActEChr *chr))BtlAct_GetQueued)
#define BtlAct_GetHeight ((f32 (*)(BtlActEChr *chr))BtlAct_GetHeight)
extern f32 BtlAct_ScaleSpeedByDist(BtlActEChr *chr, f32 speed, f32 range, f32 frames);

/* Not named yet (other modules). Purposes read from how they are used here. */
#define BtlDecide_Main ((void (*)(BtlActEChr *chr, s32 mask))BtlDecide_Main)    /* fill the follow-up queue from the input, by class mask */
#define BtlDecide_Attack ((s32 (*)(BtlActEChr *chr, s32 mask))BtlDecide_Attack)     /* the same for attack inputs; 1 = queued */
#define BtlDecide_QueueAttack ((s32 (*)(BtlActEChr *chr, s32 attack))BtlDecide_QueueAttack)   /* queue an attack; 1 = queued */
extern s32 BtlParam_GetAmountA(BtlActEChr *chr);               /* character parameter: a ki cost */
extern s32 BtlParam_GetAmountB(BtlActEChr *chr);               /* character parameter: a ki cost */
extern f32 BtlParam_GetUnk68(BtlActEChr *chr);               /* character parameter: progress ratio */
extern f32 BtlParam_GetUnk6C(BtlActEChr *chr);
#define BtlParam_GetDashSound ((s32 (*)(BtlActEChr *chr))BtlParam_GetDashSound)               /* character parameter: a common sound id */
#define BtlMoveParam_GetSpeed ((f32 (*)(BtlActEChr *chr, s32 n))BtlMoveParam_GetSpeed)        /* character speed parameter n */
#define BtlMoveParam_GetKiCost ((s32 (*)(BtlActEChr *chr, s32 n))BtlMoveParam_GetKiCost)        /* character ki cost n */
#define BtlMoveParam_GetTurnRate ((f32 (*)(BtlActEChr *chr, s32 n))BtlMoveParam_GetTurnRate)        /* character turn-rate parameter n */

/*
 * Action 0x1A: dash in the direction held. Animations 0x18 / 0x2D (start) -> 0x19 (loop) -> 0x1A or 0x15 (stop).
 * Speed kind 7, ASCEND / DESCEND (0x800 / 0x1000) move up / down at speed kind 8, ki drain kind 1 per frame. Stops when
 * blocked by the opponent, slower than 100 km/h for 16 frames, or out of ki. Inputs 0x14 / 0x15 (through
 * animation 0x15) chain into action 0x19 (with flag 5) or 0x1A again.
 */
s32 BtlAct_DragonDashHandler(BtlActEChr *chr, s32 phase) {
    s32 *counter = &chr->work[2];
    s32 flag;
    f32 speed;
    f32 turn;
    f32 vspeed;

    if (phase == 0) {
        s32 anim;

        switch (BtlAct_GetPrev(chr)) {
            case 0x19:
            case 0x1A:
                anim = 0x19;
                break;
            default:
                anim = 0x2D;
                break;
        }
        if (chr->unkFF0 > 0) {
            anim = 0x18;
        }
        BtlAnim_Play(chr, anim, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlMove_TurnYaw(chr, 0, 3.14159265f);
    }
    if (phase == 1) {
        flag = 1;
        switch (BtlAnim_GetId(chr)) {
            case 0x18:
            case 0x2D:
                BtlAnim_AdvanceThen(chr, 0x19, 0.2f, 0);
                flag = 0;
                break;
            case 0x19:
                BtlAnim_AdvanceLoop(chr, 0);
                if (BtlAnim_IsNew(chr)) {
                    BtlCharSnd_PlayCommon(chr, BtlParam_GetDashSound(chr));
                    BtlChar_Vibrate(chr, 0.8f, 0.3f);
                } else {
                    BtlChar_SetVibration(chr, 0.7f, 0.1f);
                }
                flag = 1;
                break;
            case 0x15:
            case 0x1A:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xB);
                    BtlChar_GetPos(chr)->unk98 = 0.0f;
                }
                flag = 0;
                break;
        }
        if (flag) {
            speed = BtlMoveParam_GetSpeed(chr, 7);
            turn = BtlMoveParam_GetTurnRate(chr, 2);
            vspeed = BtlMoveParam_GetSpeed(chr, 8);
            BtlMove_TurnYaw(chr, 0, turn);
            BtlMove_TurnPitch(chr, 5, 3.14159265f);
            BtlMove_TurnModelYaw(chr, BTL_DEG(36.0f), 0.3f);
            BtlMove_SetDirection(chr, 3);
            BtlMove_Advance(chr, speed, 10000.0f);
            if (BtlInput_IsHeld(chr, 0x800)) {
                BtlMove_MoveVertical(chr, -vspeed, BTL_KMH(100.0f));
            } else if (BtlInput_IsHeld(chr, 0x1000)) {
                BtlMove_MoveVertical(chr, vspeed, BTL_KMH(100.0f));
            } else {
                BtlMove_ApplyGravity(chr);
            }
            BtlChar_SetFlag(chr, 9);
            BtlChar_SetFlag(chr, 0x50);
            BtlChar_SetFlag(chr, 0x51);
            BtlChar_SetFxBit(chr, 4);
        } else {
            BtlMove_Step(chr, 6, 5, 3, 0.0f, BTL_KMH(100.0f));
            BtlMove_ApplyGravity(chr);
        }
        BtlMove_RequestOrbit(chr, 50.0f, 100.0f);
        BtlChar_SetFlag(chr, 0x1A);
        BtlChar_SetFxBit(chr, 0x3A);
    }
    if (phase == 2) {
        switch (BtlAnim_GetId(chr)) {
            case 0x18:
            case 0x2D:
                break;
            case 0x19:
                flag = BtlMove_IsBlockedByOpponent(chr) != 0;
                if (Vec3_Length(&BtlChar_GetPos(chr)->unk40) < BTL_KMH(100.0f)) {
                    if (++*counter >= 0x10) {
                        flag = 1;
                    }
                } else {
                    *counter = 0;
                }
                if (BtlMember_SpendKi(chr, BtlMoveParam_GetKiCost(chr, 1), 0)) {
                    flag = 1;
                }
                if (BtlInput_TestAction(chr, 0x16, 1)) {
                    BtlAnim_Request(chr, 0x15, 0.0f);
                }
                if (BtlInput_TestAction(chr, 0x14, 1)) {
                    BtlAnim_Request(chr, 0x15, 0.0f);
                    chr->work[0] |= 1;
                }
                if (BtlInput_TestAction(chr, 0x15, 1)) {
                    BtlAnim_Request(chr, 0x15, 0.0f);
                    chr->work[0] |= 2;
                }
                if (flag) {
                    BtlAnim_Request(chr, 0x1A, 0.0f);
                    BtlChar_GetPos(chr)->unk98 *= 0.5f;
                }
                break;
            case 0x1A:
                if (BtlMove_IsBlockedByOpponent(chr)) {
                    BtlChar_GetPos(chr)->unk98 = 0.0f;
                }
                break;
            case 0x15:
                if (BtlInput_TestAction(chr, 0x17, 1)) {
                    chr->work[0] |= 1;
                }
                if (BtlInput_TestAction(chr, 0x18, 1)) {
                    chr->work[0] |= 2;
                }
                if (BtlAnim_GetProgress(chr) > 0.5f) {
                    if (chr->work[0] & 1) {
                        if (BtlChar_TestFlag(chr, 5)) {
                            BtlAct_Request(chr, 0x19);
                        }
                    }
                    if (chr->work[0] & 2) {
                        BtlAct_Request(chr, 0x1A);
                    }
                }
                break;
        }
        BtlDecide_Main(chr, 0x1800);
        BtlAct_Request(chr, BtlAct_GetQueued(chr));
    }
}

/*
 * Actions 0x1B (back), 0x1C, 0x1D (left / right of the line to the opponent): a step. Animations 0x1C..0x1E on
 * the ground, 0x28..0x2A in the air (flag 0xE). Moves between 10% and 60% of the animation.
 */
s32 BtlAct_StepHandler(BtlActEChr *chr, s32 phase) {
    s32 voice;
    s32 anim;
    f32 yaw;
    f32 speed;

    if (phase == 0) {
        anim = 0;
        voice = -1;
        if (BtlAct_GetHeight(chr) < 5.0f) {
            BtlChar_ClearFlag(chr, 0xE);
        }
        yaw = BtlOpp_GetYaw(chr);
        switch (BtlAct_GetCurrent(chr)) {
            case 0x1B:
                anim = BtlChar_TestFlag(chr, 0xE) ? 0x28 : 0x1C;
                voice = 9;
                yaw = BtlUtil_WrapAngle(yaw + 3.14159265f);
                break;
            case 0x1C:
                anim = BtlChar_TestFlag(chr, 0xE) ? 0x29 : 0x1D;
                voice = 8;
                yaw = BtlUtil_WrapAngle(yaw - 3.14159265f / 2.0f);
                break;
            case 0x1D:
                anim = BtlChar_TestFlag(chr, 0xE) ? 0x2A : 0x1E;
                voice = 8;
                yaw = BtlUtil_WrapAngle(yaw + 3.14159265f / 2.0f);
                break;
        }
        BtlAnim_Play(chr, anim, 0.15f);
        BtlChar_GetPos(chr)->vel.x = Mathf_Sin(yaw);
        BtlChar_GetPos(chr)->vel.y = 0.0f;
        BtlChar_GetPos(chr)->vel.z = Mathf_Cos(yaw);
        if (BtlChar_TestFlag(chr, 0xE)) {
            BtlCharSnd_PlayCommon(chr, 0x1F);
        } else if (BtlAct_GetCurrent(chr) == 0x1B) {
            BtlCharSnd_PlayCommon(chr, 0x1F);
        } else {
            BtlCharSnd_PlayBank8(chr, 1);
        }
        if (voice >= 0) {
            BtlChar_PlayVoice(chr, voice);
        }
    }
    if (phase == 1) {
        speed = 0.0f;
        yaw = BtlAnim_GetProgress(chr);

        if (BtlAnim_Advance(chr, 0)) {
            chr->work[1] = 1;
        }
        if (BtlAnim_GetProgress(chr) < 0.2f) {
            BtlChar_SetFlag(chr, 0x42);
        }
        switch (BtlAnim_GetId(chr)) {
            case 0x1C:
            case 0x28:
                if (0.1f < yaw && yaw < 0.6f) {
                    speed = BtlMoveParam_GetSpeed(chr, 0xB);
                } else {
                    speed = 0.0f;
                }
                break;
            case 0x1D:
            case 0x1E:
            case 0x29:
            case 0x2A:
                if (0.1f < yaw && yaw < 0.6f) {
                    yaw = BtlMoveParam_GetSpeed(chr, 0xC);
                    speed = BtlAnim_GetLength(chr);
                    speed /= BtlAnim_GetStep(chr);
                    speed = BtlAct_ScaleSpeedByDist(chr, yaw, 3.14159265f / 2.0f, speed * (0.6f - 0.1f));
                } else {
                    speed = 0.0f;
                }
                break;
        }
        BtlMove_Step(chr, 2, 5, 6, speed, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0x88);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlMember_GetActiveGauge(chr)->unk28) {
            BtlChar_SetFlag(chr, 0x96);
        }
        BtlMove_RequestOrbit(chr, 5.0f, 10.0f);
    }
    if (phase == 2) {
        if (BtlAnim_GetId(chr) == 0x1C) {
            yaw = BtlParam_GetUnk6C(chr);
        } else {
            yaw = BtlParam_GetUnk68(chr);
        }
        if (chr->work[1]) {
            BtlAct_Request(chr, 0xB);
            anim = 1;
            if (BtlInput_IsHeld(chr, 0x20)) {
                anim = 0x11;
            }
            BtlDecide_Main(chr, anim);
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
        if (!BtlAct_HasQueued(chr) && BtlAnim_GetProgress(chr) > 0.4f) {
            BtlDecide_Main(chr, 0x14000);
            BtlDecide_Attack(chr, 0x83);
        }
        if (yaw < BtlAnim_GetProgress(chr)) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
    }
}

/* Actions 0x1E, 0x1F: short forward dash (animation 0x1B). 0x1E takes attack inputs; 0x1F queues attacks 0x8F, 0x8D, 0x8E. */
s32 BtlAct_StepInHandler(BtlActEChr *chr, s32 phase) {
    f32 speed;
    f32 accel;

    if (phase == 0) {
        BtlAnim_Play(chr, 0x1B, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlCharSnd_PlayCommon(chr, 0x1F);
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlAct_Request(chr, 0xB);
        }
        if (BtlAnim_GetProgress(chr) < 0.3f) {
            BtlChar_SetFlag(chr, 0x42);
        }
        if (BtlAnim_GetProgress(chr) < 0.7f) {
            speed = BtlMoveParam_GetSpeed(chr, 0xA);
            accel = 10000.0f;
        } else {
            speed = 0.0f;
            accel = BTL_KMH(100.0f);
        }
        BtlMove_Step(chr, 2, 1, 3, speed, accel);
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0x88);
        BtlChar_SetFlag(chr, 0xD5);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
        if (BtlMember_GetActiveGauge(chr)->unk28) {
            BtlChar_SetFlag(chr, 0x96);
        }
    }
    if (phase == 2) {
        switch (BtlAct_GetCurrent(chr)) {
            case 0x1E:
                BtlDecide_Attack(chr, 0x1C00803);
                if (BtlAnim_GetProgress(chr) > 0.5f) {
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                }
                if (BtlDecide_Attack(chr, 0x40)) {
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                }
                break;
            case 0x1F:
                BtlDecide_Attack(chr, 0x2000030);
                BtlDecide_QueueAttack(chr, 0x8F);
                BtlDecide_QueueAttack(chr, 0x8D);
                BtlDecide_QueueAttack(chr, 0x8E);
                if (BtlAnim_GetProgress(chr) > 0.3f) {
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                }
                break;
        }
    }
}

/*
 * Actions 0x20..0x23: vanishing side-step (animation 0xF6): seven frames of fast movement back (0x20),
 * left (0x21), right (0x22) or up along the line to the opponent (0x23), invulnerable (flags 0x42..0x46).
 */
s32 BtlAct_VanishStepHandler(BtlActEChr *chr, s32 phase) {
    Vec4 dir;
    Vec4 delta;
    Vec4 oppVel;
    s32 *counter = &chr->work[2];
    BtlActEPose *pose = BtlChar_GetPos(chr);
    f32 yaw;
    f32 c;
    f32 sn;

    if (phase == 0) {
        BtlAnim_Play(chr, 0xF6, 0.15f);
        if (BtlAct_GetPrev(chr) == 0x44) {
            BtlChar_SetHeldFlag(chr, 0x83);
            BtlMember_SpendKi(chr, BtlParam_GetAmountA(chr), 0);
        }
        if (BtlChar_TestFlag(chr, 0x74) || BtlChar_TestFlag(chr, 0x75) || BtlChar_TestFlag(chr, 0x76)) {
            chr->work[0] |= 1;
            BtlChar_SetFlag(chr, 0x12C);
            BtlEvent_Raise(chr->player, 0x41);
            BtlChar_SetFrameBits(chr, 0x8000);
            BtlChar_AddStageTimer(2.0f);
        }
        *counter = 7;
        BtlChar_SetFxBit(chr, 0xC);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlCharSnd_PlayCommon(chr, 0x20);
        BtlChar_GetPos(chr)->unk98 = 0.0f;
        BtlChar_GetPos(chr)->unk9C = 0.0f;
        switch (BtlAct_GetCurrent(chr)) {
            case 0x21:
            case 0x22:
                if (BtlChar_TestFlag(chr, 0x13)) {
                    BtlOpp_GetTargetRot(chr, &dir);
                    if (__builtin_fabsf(BtlUtil_WrapAngle(pose->facing - dir.y)) > 3.14159265f / 2.0f) {
                        BtlChar_PlayVoice(chr, 0x1D);
                    }
                }
                break;
        }
        if (BtlChar_TestFlag(chr, 0x78)) {
            chr->work[0] |= 2;
        }
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            chr->work[0] |= 1;
        }
        if (!(chr->work[0] & 1)) {
            BtlMove_Step(chr, 2, 5, 7, 0.0f, BTL_KMH(100.0f));
            BtlMove_ApplyGravity(chr);
        } else {
            if (*counter == 7) {
                Vec4_SetZero(&dir);
                BtlOpp_GetDelta(chr, &delta);
                Vec3_Normalize(&delta, &delta);
                yaw = -BtlOpp_GetYaw(chr);
                switch (BtlAct_GetCurrent(chr)) {
                    case 0x21:
                        dir.x = -1.0f;
                        break;
                    case 0x22:
                        dir.x = 1.0f;
                        break;
                    case 0x23:
                        dir.y = delta.y;
                        dir.z = sqrtf(1.0f - delta.y * delta.y);
                        break;
                    case 0x20:
                        dir.z = -1.0f;
                        break;
                }
                Vec3_Normalize(&dir, &dir);
                c = Mathf_Cos(yaw);
                sn = Mathf_Sin(yaw);
                pose->vel.x = dir.x * c - dir.z * sn;
                pose->vel.y = dir.y;
                pose->vel.z = dir.x * sn + dir.z * c;
            }
            --*counter;
            if (*counter >= 2) {
                c = BtlAct_ScaleSpeedByDist(chr, BtlMoveParam_GetSpeed(chr, 0x12), 3.14159265f, 6.0f);
            } else if (*counter == 1) {
                c = 0.0f;
                BtlChar_SetFxBit(chr, 0xD);
            } else {
                c = 0.0f;
            }
            BtlMove_Step(chr, 2, 5, 7, c, 10000.0f);
            BtlMove_ApplyGravity(chr);
            if (!(chr->work[0] & 2)) {
                if (BtlChar_TestFlag(chr, 5)) {
                    BtlOpp_GetVelocity(chr, &oppVel);
                    Vec4_Add(&pose->pos, &pose->pos, &oppVel);
                }
            }
            BtlChar_SetFlag(chr, 0xB);
        }
        BtlMove_RequestOrbit(chr, 5.0f, 10.0f);
        BtlChar_SetFlag(chr, 0x88);
        BtlChar_SetFxBit(chr, 0x3A);
        BtlChar_SetFlag(chr, 0x42);
        BtlChar_SetFlag(chr, 0x43);
        BtlChar_SetFlag(chr, 0x44);
        BtlChar_SetFlag(chr, 0x45);
        BtlChar_SetFlag(chr, 0x46);
        if (BtlChar_TestFlag(chr, 0x83)) {
            BtlChar_SetFlag(chr, 0x85);
        }
    }
    if (phase == 2) {
        if (!BtlAct_HasQueued(chr)) {
            BtlDecide_Attack(chr, 3);
        }
        if (*counter <= 0) {
            BtlAct_Request(chr, 0xB);
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
    }
}

/* Action 0x24: vanish and reappear behind the opponent (animation 0xF6), costs ki. */
s32 BtlAct_VanishBehindHandler(BtlActEChr *chr, s32 phase) {
    s32 *counter = &chr->work[2];

    if (phase == 0) {
        BtlAnim_Play(chr, 0xF6, 0.15f);
        BtlMember_SpendKi(chr, BtlParam_GetAmountB(chr), 0);
        BtlChar_SetFxBit(chr, 0xC);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlCharSnd_PlayCommon(chr, 0x20);
        BtlChar_PlayVoice(chr, 0x1D);
        BtlChar_GetPos(chr)->unk98 = 0.0f;
        BtlChar_GetPos(chr)->unk9C = 0.0f;
        *counter = 7;
        BtlChar_SetSmallVibration(chr, 0.2f);
    }
    if (phase == 1) {
        if (!BtlAnim_Advance(chr, 0)) {
            BtlMove_Step(chr, 2, 5, 7, 0.0f, BTL_KMH(100.0f));
            BtlMove_ApplyGravity(chr);
        } else {
            --*counter;
            if (*counter >= 2) {
                BtlChar_GetPos(chr)->unk98 = 0.0f;
                BtlChar_GetPos(chr)->unk9C = 0.0f;
            } else if (*counter == 1) {
                BtlChar_SetFxBit(chr, 0xD);
                if (!BtlChar_TestFlag(chr, 0xBA)) {
                    BtlMove_WarpBehindOpponent(chr, 0.0f);
                    BtlMove_TurnYaw(chr, 2, 3.14159265f);
                    BtlMove_TurnPitch(chr, 1, 3.14159265f);
                    BtlMove_TurnModelYaw(chr, 3.14159265f, 1.0f);
                    BtlMove_SetDirection(chr, 3);
                    BtlChar_SetFlag(chr, 0x3F);
                    BtlChar_SetFlag(chr, 0xB);
                    BtlChar_SetFlag(chr, 0xCC);
                    BtlChar_SetFlag(chr, 0x24);
                }
            }
            BtlChar_SetFlag(chr, 0xB);
        }
        BtlChar_SetFlag(chr, 0x42);
        BtlChar_SetFlag(chr, 0x43);
        BtlChar_SetFlag(chr, 0x44);
        BtlChar_SetFlag(chr, 0x45);
        BtlChar_SetFlag(chr, 0x46);
    }
    if (phase == 2) {
        if (!BtlAct_HasQueued(chr)) {
            BtlDecide_Attack(chr, 3);
        }
        if (*counter <= 0) {
            BtlAct_Request(chr, 0xB);
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
    }
}

/*
 * Actions 0x25..0x2A: recover in the air (animations 0xFE..0x102), costs ki. 0x29 and 0x2A turn around;
 * 0x26 goes on to action 0xF, 0x28 and 0x2A to action 0x19, the others to 0xB.
 */
void BtlAct_RecoverHandler(BtlActEChr *chr, s32 phase) {
    if (phase == 0) {
        BtlActEPose *pose = BtlChar_GetPos(chr);
        s32 anim = 0xFE;

        if (BtlAct_IsAirMotion(chr, 0)) {
            pose->facing = BtlUtil_WrapAngle(pose->facing + 3.14159265f);
        }
        switch (BtlAct_GetCurrent(chr)) {
            case 0x25:
            case 0x26:
                anim = 0xFE;
                break;
            case 0x27:
                anim = 0xFF;
                break;
            case 0x29:
                anim = 0x100;
                pose->unk98 = 0.0f;
                pose->rot.y = pose->facing = BtlUtil_WrapAngle(pose->facing + 3.14159265f);
                break;
            case 0x28:
                anim = 0x101;
                break;
            case 0x2A:
                anim = 0x102;
                pose->unk98 = 0.0f;
                pose->rot.y = pose->facing = BtlUtil_WrapAngle(pose->facing + 3.14159265f);
                break;
        }
        BtlAnim_Play(chr, anim, 0.0f);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlMember_SpendKi(chr, BtlParam_GetRecoverKiCost(chr), 0);
        BtlCharSnd_PlayCommon(chr, 0x21);
    }
    if (phase == 1) {
        switch (BtlAct_GetCurrent(chr)) {
            case 0x26:
                BtlAnim_Advance(chr, 0);
                if (BtlAnim_GetProgress(chr) > 0.4f) {
                    BtlAct_Request(chr, 0xF);
                }
                break;
            case 0x25:
            case 0x27:
            case 0x29:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xB);
                }
                break;
            case 0x28:
            case 0x2A:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0x19);
                }
                break;
        }
        BtlMove_Step(chr, 6, 6, 7, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        if (BtlChar_TestFlag(chr, 0x11)) {
            BtlChar_SetHeldFlag(chr, 0xE);
        }
    }
    if (phase == 3) {
        BtlChar_SetFlag(chr, 0x33);
    }
}

/*
 * Plays part `frame` of a multi-part attack motion as the sub-animation and returns the updated
 * lead time: each part before the last adds the object's value at +0xBDC (in 60ths) / atk->unk10
 * frames, the last part adds half of two animation marker frames and rounds.
 */
f32 BtlAct_PlayAttackPart(BtlActEChr *chr, BtlActEAttack *atk, s32 frame, f32 time) {
    BtlActEObj *obj = BtlChar_GetObj(chr);

    if (frame < atk->parts - 1) {
        BtlAnim_PlaySub(chr, atk->motion[frame]);
        time += obj->unkBDC * 30.0f / 60.0f / atk->unk10;
    } else if (frame == atk->parts - 1) {
        s32 n;

        BtlAnim_PlaySub(chr, atk->motion[frame]);
        n = BtlObjAnim_QueryEvent(obj, 1, 1, 0);
        n += BtlObjAnim_QueryEvent(obj, 2, 1, 0);
        time += (f32)n * 0.5f * 30.0f / 60.0f;
        time = floorf(time + 0.5f);
    }
    return time;
}

/* Action 0x2B: vanish (animation 0x26), then snap to the opponent and start the queued attack. */
void BtlAct_VanishAttackHandler(BtlActEChr *chr, s32 phase) {
    s32 *timer = &chr->work[3];
    s32 *step = &chr->work[2];

    if (phase == 0) {
        BtlChar_SetFxBit(chr, 0xC);
        BtlCharSnd_PlayCommon(chr, 0x20);
        BtlChar_SetSmallVibration(chr, 0.2f);
        BtlChar_SetHeldFlag(chr, 0xE);
    }
    if (phase == 1) {
        BtlAnim_AdvanceThen(chr, 0x26, 0.15f, 0);
        chr->nextAttack.unk18 = BtlAct_PlayAttackPart(chr, &chr->nextAttack, chr->actionFrame, chr->nextAttack.unk18);
        switch (*step) {
            case 0:
                if (++*timer >= 4) {
                    BtlChar_SetFlag(chr, 0xB);
                }
                if (*timer >= 6) {
                    *timer = 0;
                    ++*step;
                }
                break;
            case 1:
                BtlMove_SnapToOpponent(chr);
                BtlChar_SetFlag(chr, 0xB);
                BtlChar_SetFlag(chr, 0xCC);
                if (BtlAct_HasQueued(chr)) {
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                } else {
                    BtlAct_Request(chr, 0xB);
                }
                break;
        }
    }
    if (phase == 3) {
        BtlChar_SetHeldFlag(chr, 0xDD);
    }
}

/* Action 0x2C: the same as 0x2B with a one-frame wait. */
void BtlAct_VanishAttackQuickHandler(BtlActEChr *chr, s32 phase) {
    s32 *step = &chr->work[2];

    if (phase == 0) {
        BtlChar_SetFxBit(chr, 0xC);
        BtlCharSnd_PlayCommon(chr, 0x20);
        BtlChar_SetSmallVibration(chr, 0.2f);
        BtlChar_SetHeldFlag(chr, 0xE);
    }
    if (phase == 1) {
        BtlAnim_AdvanceThen(chr, 0x26, 0.15f, 0);
        chr->nextAttack.unk18 = BtlAct_PlayAttackPart(chr, &chr->nextAttack, chr->actionFrame, chr->nextAttack.unk18);
        switch (*step) {
            case 0:
                BtlChar_SetFlag(chr, 0xB);
                ++*step;
                break;
            case 1:
                BtlMove_SnapToOpponent(chr);
                BtlChar_SetFlag(chr, 0xB);
                BtlChar_SetFlag(chr, 0xCC);
                if (BtlAct_HasQueued(chr)) {
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                } else {
                    BtlAct_Request(chr, 0xB);
                }
                break;
        }
    }
    if (phase == 3) {
        BtlChar_SetHeldFlag(chr, 0xDD);
    }
}

/* Action 0x2D: appear behind the opponent on the first frame and start the queued attack. */
void BtlAct_WarpBehindAttackHandler(BtlActEChr *chr, s32 phase) {
    if (phase == 1) {
        BtlChar_GetPos(chr)->unk98 = 0.0f;
        BtlChar_GetPos(chr)->unk9C = 0.0f;
        if (!BtlChar_TestFlag(chr, 0xBA)) {
            BtlMove_WarpBehindOpponent(chr, 0.0f);
            BtlMove_TurnYaw(chr, 2, 3.14159265f);
            BtlMove_TurnPitch(chr, 1, 3.14159265f);
            BtlMove_TurnModelYaw(chr, 3.14159265f, 1.0f);
            BtlMove_SetDirection(chr, 3);
            BtlChar_SetFlag(chr, 0x3F);
            BtlChar_SetFlag(chr, 0xB);
            BtlChar_SetFlag(chr, 0xCC);
            BtlChar_SetFlag(chr, 0x24);
        }
        if (BtlAct_HasQueued(chr)) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        } else {
            BtlAct_Request(chr, 0xB);
        }
    }
    if (phase == 3) {
        BtlChar_SetHeldFlag(chr, 0xDD);
    }
}

/* Action 0x2E: appear ahead of the (moving) opponent, led by the attack's lead time, and start the queued attack. */
void BtlAct_WarpAheadAttackHandler(BtlActEChr *chr, s32 phase) {
    if (phase == 1) {
        BtlChar_GetPos(chr)->unk98 = 0.0f;
        BtlChar_GetPos(chr)->unk9C = 0.0f;
        if (!BtlChar_TestFlag(chr, 0xBA)) {
            BtlMove_WarpAheadOfOpponent(chr, chr->nextAttack.unk18);
            BtlMove_TurnYaw(chr, 2, 3.14159265f);
            BtlMove_TurnPitch(chr, 1, 3.14159265f);
            BtlMove_TurnModelYaw(chr, 3.14159265f, 1.0f);
            BtlMove_SetDirection(chr, 3);
            BtlChar_SetFlag(chr, 0x3F);
            BtlChar_SetFlag(chr, 0xB);
            BtlChar_SetFlag(chr, 0xCC);
            BtlChar_SetFlag(chr, 0x24);
        }
        if (BtlAct_HasQueued(chr)) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        } else {
            BtlAct_Request(chr, 0xB);
        }
    }
    if (phase == 3) {
        BtlChar_SetHeldFlag(chr, 0xDD);
    }
}

/*
 * Actions 0x2F, 0x30: slide in four frames to a point beside the opponent's predicted position
 * (35 degrees off at distance 11, or 70 degrees off at distance 8), then face the predicted point.
 */
s32 BtlAct_SlideToAttackHandler(BtlActEChr *chr, s32 phase) {
    Vec4 point;
    Vec4 target;
    Vec4 *step = (Vec4 *)&chr->work[12];
    s32 *counter = &chr->work[2];
    Vec4 *face = (Vec4 *)&chr->work[16];
    BtlActEPose *pose = BtlChar_GetPos(chr);

    if (phase == 0) {
        f32 dist = 0.0f;
        f32 angle = 0.0f;
        f32 extra = 0.0f;
        s32 frames = 4;

        switch (BtlAct_GetCurrent(chr)) {
            case 0x2F:
                angle = BTL_DEG2RAD(-35.0f);
                dist = 11.0f;
                extra = 1.0f;
                frames = 4;
                break;
            case 0x30:
                angle = BTL_DEG2RAD(-70.0f);
                dist = 8.0f;
                break;
        }
        BtlMove_CalcApproachPoint(chr, &point, &target, chr->nextAttack.unk1C, angle, dist, dist + frames + extra);
        Vec4_Sub(step, &point, &pose->pos);
        Vec4_Scale(step, step, 0.25f);
        Vec4_Copy(face, &target);
    }
    if (phase == 1) {
        pose->unk98 = 0.0f;
        pose->unk9C = 0.0f;
        Vec4_Add(&pose->pos, &pose->pos, step);
        if (++*counter == 4) {
            if (BtlAct_HasQueued(chr)) {
                BtlAct_Request(chr, BtlAct_GetQueued(chr));
            } else {
                BtlAct_Request(chr, 0xB);
            }
        }
        BtlChar_SetFlag(chr, 0x3F);
        BtlChar_SetFlag(chr, 0xB);
        BtlChar_SetFlag(chr, 0xCC);
        BtlChar_SetFlag(chr, 0x24);
    }
    if (phase == 3) {
        BtlMove_TurnToPoint(chr, face, 3.14159265f, 3.14159265f);
        BtlChar_SetHeldFlag(chr, 0xDD);
    }
}

/*
 * Action 0x31: rush at the opponent (animation 0xD, the dash loop) at the attack's speed until the
 * closing time drops below the attack's lead time, then start the queued attack; 0xE is the stop animation.
 */
void BtlAct_RushToAttackHandler(BtlActEChr *chr, s32 phase) {
    f32 *lead = (f32 *)&chr->work[6];
    s32 *counter = &chr->work[2];
    s32 reach;
    s32 stuck;
    f32 speed;
    f32 accel;

    if (phase == 0) {
        BtlAnim_Play(chr, 0xD, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlChar_SetFlag(chr, 0xD1);
        BtlCharSnd_PlayCommon(chr, BtlParam_GetDashSound(chr));
        BtlChar_SetVibration(chr, 0.8f, 0.2f);
    }
    if (phase == 1) {
        speed = 0.0f;
        *lead = BtlAct_PlayAttackPart(chr, &chr->nextAttack, chr->actionFrame, *lead);
        accel = BTL_KMH(100.0f);
        switch (BtlAnim_GetId(chr)) {
            case 0xD:
                BtlAnim_AdvanceLoop(chr, 0);
                speed = chr->nextAttack.unk1C;
                accel = 10000.0f;
                BtlChar_SetFxBit(chr, 4);
                if (BtlChar_TestFlag(chr, 0xF)) {
                    BtlChar_SetFxBit(chr, 0x30);
                }
                BtlChar_SetFlag(chr, 9);
                BtlChar_SetFlag(chr, 0x50);
                BtlChar_SetFlag(chr, 0x51);
                BtlChar_SetFxBit(chr, 9);
                break;
            case 0xE:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xB);
                }
                break;
        }
        BtlMove_Step(chr, 2, 1, 3, speed, accel);
        BtlMove_ApplyGravity(chr);
    }
    if (phase == 2) {
        if (BtlAnim_GetId(chr) == 0xD) {
            reach = 0;
            stuck = 0;
            if (BtlOpp_GetClosingTime(chr) < *lead) {
                reach = 1;
            }
            if (BtlMove_IsBlockedByOpponent(chr)) {
                reach = 1;
            }
            if (Vec3_Length(&BtlChar_GetPos(chr)->unk40) < BTL_KMH(100.0f)) {
                if (++*counter >= 8) {
                    stuck = 1;
                }
            } else {
                *counter = 0;
            }
            if (!BtlChar_TestFlag(chr, 5)) {
                reach = 1;
            }
            if (reach || stuck) {
                BtlChar_GetPos(chr)->unk98 = 0.0f;
                if (BtlAct_HasQueued(chr) && !stuck) {
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                } else {
                    BtlAnim_Request(chr, 0xE, 0.0f);
                }
            }
        }
    }
    if (phase == 3) {
        BtlChar_SetHeldFlag(chr, 0xDD);
    }
}

/* Action 0x32: hop back (animation 0x28) for half the animation, then start the queued attack. */
void BtlAct_HopBackAttackHandler(BtlActEChr *chr, s32 phase) {
    if (phase == 0) {
        BtlAnim_Play(chr, 0x28, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlCharSnd_PlayCommon(chr, 0x1F);
    }
    if (phase == 1) {
        f32 speed = BtlMoveParam_GetSpeed(chr, 0xB);

        BtlAnim_Advance(chr, 0);
        if (BtlAnim_GetProgress(chr) > 0.5f) {
            if (BtlAct_HasQueued(chr)) {
                BtlAct_Request(chr, BtlAct_GetQueued(chr));
            } else {
                BtlAct_Request(chr, 0xB);
            }
        }
        BtlMove_Step(chr, 2, 5, 4, speed, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
    }
    if (phase == 3) {
        BtlChar_SetHeldFlag(chr, 0xDD);
    }
}

/*
 * Action 0x33: dash in a circle around the opponent to get behind it (animations 0x2D -> 0x19 loop ->
 * 0x1A stop), spending ki every frame. Once round far enough, attack 0x90 or an attack input follows.
 */
s32 BtlAct_CircleDashHandler(BtlActEChr *chr, s32 phase) {
    s32 *counter = &chr->work[2];
    f32 *baseYaw = (f32 *)&chr->work[6];
    s32 flag;
    f32 lean;
    f32 speed;

    if (phase == 0) {
        BtlActEPose *pose = BtlChar_GetPos(chr);

        BtlAnim_Play(chr, 0x2D, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlChar_PlayVoice(chr, 5);
        BtlChar_SetFxBit(chr, 6);
        pose->unk98 = 0.0f;
        pose->unk9C = 0.0f;
        if (0.0f < BtlOpp_GetYawFromItsFacing(chr)) {
            chr->work[0] |= 1;
        }
        *baseYaw = BtlUtil_WrapAngle(pose->rot.y + 3.14159265f);
    }
    if (phase == 1) {
        flag = 0;
        lean = 0.0f;
        switch (BtlAnim_GetId(chr)) {
            case 0x2D:
                BtlAnim_AdvanceThen(chr, 0x19, 0.15f, 0);
                lean = 1.0f;
                break;
            case 0x19:
                BtlAnim_AdvanceLoop(chr, 0);
                lean = 1.0f;
                flag = 1;
                if (BtlAnim_IsNew(chr)) {
                    BtlCharSnd_PlayCommon(chr, BtlParam_GetDashSound(chr));
                    BtlChar_Vibrate(chr, lean, 0.3f);
                    ChrCam_RequestCut(chr, 1, 4);
                }
                break;
            case 0x1A:
                BtlAnim_SetDuration(chr, 0.3f);
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xB);
                }
                lean = 1.0f - BtlAnim_GetProgress(chr);
                break;
        }
        if (flag) {
            speed = BtlMoveParam_GetSpeed(chr, 0x13);
            if (BtlMove_CircleOpponent(chr, chr->work[0] & 1, BtlOpp_GetSeenAction(chr) == 0xD5, *baseYaw, speed)) {
                chr->work[0] |= 2;
            }
            BtlChar_SetFlag(chr, 9);
            BtlChar_SetFlag(chr, 0x50);
            BtlChar_SetFlag(chr, 0x51);
            BtlChar_SetFxBit(chr, 4);
            BtlChar_SetFlag(chr, 0x47);
        } else {
            BtlMove_Step(chr, (chr->work[0] & 2) ? 2 : 6, 6, 7, 0.0f, BTL_KMH(100.0f));
            BtlMove_ApplyGravity(chr);
        }
        BtlMove_SetLeanX(chr, BtlChar_GetPos(chr)->speed * lean);
        BtlChar_SetFlag(chr, 0x89);
    }
    if (phase == 2) {
        flag = 0;
        switch (BtlAnim_GetId(chr)) {
            case 0x2D:
                if (BtlMove_IsBlockedByOpponent(chr)) {
                    flag = 1;
                    BtlChar_GetPos(chr)->unk98 = 0.0f;
                }
                break;
            case 0x19:
                if (BtlMove_IsBlockedByOpponent(chr)) {
                    flag = 1;
                    BtlChar_GetPos(chr)->unk98 = 0.0f;
                }
                if (chr->work[0] & 2) {
                    flag = 1;
                    BtlChar_GetPos(chr)->unk98 = 0.0f;
                }
                if (BtlMember_SpendKi(chr, BtlMoveParam_GetKiCost(chr, 3), 0)) {
                    flag = 1;
                    BtlChar_GetPos(chr)->unk98 *= 0.2f;
                }
                if (!BtlChar_TestFlag(chr, 5)) {
                    flag = 1;
                }
                if (Vec3_Length(&BtlChar_GetPos(chr)->unk40) < BTL_KMH(1000.0f)) {
                    if (++*counter >= 6) {
                        flag = 1;
                        BtlChar_GetPos(chr)->unk98 = 0.0f;
                    }
                } else {
                    *counter = 0;
                }
                break;
            case 0x1A:
                break;
        }
        if (flag) {
            BtlAnim_Request(chr, 0x1A, 0.15f);
        }
        if (BtlChar_TestFlag(chr, 0x13)) {
            speed = 3.14159265f / 4.0f;
            if (BtlAnim_GetId(chr) == 0x1A) {
                speed = 3.14159265f / 2.0f;
            }
            if (__builtin_fabsf(BtlUtil_WrapAngle(BtlOpp_GetYaw(chr) - *baseYaw)) < speed) {
                if (BtlDecide_QueueAttack(chr, 0x90)) {
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                }
                if (BtlDecide_Attack(chr, 0x4000000)) {
                    BtlAct_Request(chr, BtlAct_GetQueued(chr));
                }
            }
        }
    }
}

/*
 * Action 0x34: rush dash at the opponent (animations 0x23 -> 0x24 loop -> 0x25 stop) at 1800 km/h.
 * Characters with parameter flag bit 20 do it vanished and invulnerable at 2200 km/h.
 */
void BtlAct_RushDashHandler(BtlActEChr *chr, s32 phase) {
    s32 fast = (BtlParam_GetFlags(chr) >> 20) & 1;

    if (phase == 0) {
        BtlAnim_Play(chr, 0x23, 0.15f);
        BtlChar_SetHeldFlag(chr, 0xE);
        if (fast) {
            BtlChar_SetFxBit(chr, 0xE);
            BtlCharSnd_PlayCommon(chr, 0x20);
        } else {
            BtlCharSnd_PlayCommon(chr, 0x31);
        }
        BtlChar_SetSmallVibration(chr, 0.2f);
    }
    if (phase == 1) {
        s32 yawMode = 2;
        f32 accel = 10000.0f;
        s32 pitchMode = 1;
        f32 speed;

        if (fast) {
            speed = 0.0f;
            BtlChar_SetFlag(chr, 0xB);
            BtlChar_SetFlag(chr, 0x42);
            BtlChar_SetFlag(chr, 0x43);
            BtlChar_SetFlag(chr, 0x44);
            BtlChar_SetFlag(chr, 0x45);
            BtlChar_SetFlag(chr, 0x46);
            if (chr->actionFrame != 0) {
                speed = BTL_KMH(2200.0f);
            }
        } else {
            speed = BTL_KMH(1800.0f);
        }
        switch (BtlAnim_GetId(chr)) {
            case 0x23:
                BtlAnim_AdvanceThen(chr, 0x24, 0.15f, 0);
                break;
            case 0x24:
                BtlAnim_AdvanceLoop(chr, 0);
                break;
            case 0x25:
                BtlAnim_SetDuration(chr, 0.1f);
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xB);
                    BtlChar_GetPos(chr)->unk98 = 0.0f;
                    BtlChar_GetPos(chr)->unk9C = 0.0f;
                }
                speed = 0.0f;
                yawMode = 6;
                accel = BTL_KMH(100.0f);
                pitchMode = 6;
                break;
        }
        BtlMove_Step(chr, yawMode, pitchMode, 3, speed, accel);
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFxBit(chr, 9);
        BtlChar_SetFxBit(chr, 0x3A);
    }
    if (phase == 2) {
        if (BtlAnim_GetId(chr) == 0x24) {
            if (BtlMove_IsBlockedByOpponent(chr)) {
                BtlAnim_Request(chr, 0x25, 0.0f);
            }
        }
        if (BtlDecide_QueueAttack(chr, 0x9C)) {
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
        if (BtlInput_TestAction(chr, 0x34, 1)) {
            BtlAct_Request(chr, 0x46);
        }
        if (BtlInput_TestAction(chr, 0x5D, 1)) {
            BtlAct_Request(chr, 0xB5);
        }
    }
    if (phase == 3) {
        if (fast) {
            BtlChar_SetFxBit(chr, 0xD);
        }
    }
}

/*
 * Action 0x35: vanished, invulnerable dash at 2000 km/h in the direction held (animations 0xC -> 0xD
 * loop -> 0xE stop); stops after 13 frames or when it runs into the opponent head on.
 */
s32 BtlAct_VanishDashHandler(BtlActEChr *chr, s32 phase) {
    s32 flag;

    if (phase == 0) {
        BtlAnim_Play(chr, 0xC, 0.15f);
        BtlChar_SetFxBit(chr, 0xE);
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlCharSnd_PlayCommon(chr, 0x20);
        BtlMove_TurnYaw(chr, 0, 3.14159265f);
        BtlMove_TurnPitch(chr, 2, 3.14159265f);
        BtlMove_TurnModelYaw(chr, 3.14159265f, 1.0f);
    }
    if (phase == 1) {
        flag = 1;
        switch (BtlAnim_GetId(chr)) {
            case 0xC:
                BtlAnim_SetDuration(chr, 0.05f);
                BtlAnim_AdvanceThen(chr, 0xD, 0.15f, 0);
                if (BtlAnim_IsNew(chr)) {
                    flag = 0;
                }
                break;
            case 0xD:
                BtlAnim_AdvanceLoop(chr, 0);
                BtlChar_SetFlag(chr, 0xB);
                break;
            case 0xE:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xB);
                }
                if (BtlAnim_IsNew(chr)) {
                    BtlChar_SetFxBit(chr, 0xD);
                    BtlChar_GetPos(chr)->unk98 = BTL_KMH(1000.0f);
                }
                if (BtlMove_IsBlockedByOpponent(chr)) {
                    BtlChar_GetPos(chr)->unk98 = 0.0f;
                }
                flag = 0;
                break;
        }
        if (flag) {
            BtlMove_Step(chr, 6, 6, 3, BTL_KMH(2000.0f), 10000.0f);
            BtlChar_SetFlag(chr, 9);
            BtlChar_SetFlag(chr, 0x42);
            BtlChar_SetFlag(chr, 0x43);
            BtlChar_SetFlag(chr, 0x44);
            BtlChar_SetFlag(chr, 0x45);
            BtlChar_SetFlag(chr, 0x46);
            BtlChar_SetFlag(chr, 0x47);
        } else {
            BtlMove_Step(chr, 6, 6, 3, 0.0f, BTL_KMH(100.0f));
        }
        BtlMove_ApplyGravity(chr);
        BtlMove_RequestOrbit(chr, 50.0f, 100.0f);
        BtlChar_SetFlag(chr, 0xBC);
    }
    if (phase == 2) {
        switch (BtlAnim_GetId(chr)) {
            case 0xC:
            case 0xD:
                flag = 1; if (chr->actionFrame < 0xD) { flag = 0; }
                if (BtlMove_IsBlockedByOpponent(chr)) {
                    if (__builtin_fabsf(BtlOpp_GetYawFromFacing(chr)) < 3.14159265f / 4.0f) {
                        flag = 1;
                    }
                }
                if (flag) {
                    BtlAnim_Request(chr, 0xE, 0.15f);
                }
                break;
            case 0xE:
                BtlDecide_Attack(chr, 3);
                BtlAct_Request(chr, BtlAct_GetQueued(chr));
                break;
        }
        if (chr->actionFrame >= 7) {
            BtlDecide_Main(chr, 0x20000000);
            BtlAct_Request(chr, BtlAct_GetQueued(chr));
        }
    }
}

/*
 * Action 0xFA (forced by flag 0x61): clash B, the two fighters trading blows. Every accepted input
 * (input condition 0x33) adds to clashCountB; abilities and a technique in progress add more on a
 * timer. The clash sequence raises 0xBF on the winner (animation 0x175) and 0xC0 on the loser (0x176).
 */
void BtlAct_ClashBlowsHandler(BtlActEChr *chr, s32 phase) {
    s32 *bonus = &chr->work[2];

    if (phase == 0) {
        BtlAnim_Play(chr, chr->player == 0 ? 0x173 : 0x174, 0.15f);
        chr->clashCountB = 0;
        if (BtlAct_IsTechniqueId(BtlAct_GetPrev(chr))) {
            chr->work[0] |= 1;
            switch (BtlAct_GetPrevClass(chr)) {
                case 2:
                    *bonus = BtlSuper_GetKiCost(chr, 2) / 2;
                    break;
                case 3:
                    *bonus = BtlSuper_GetKiCost(chr, 3) / 2;
                    break;
                case 4:
                    *bonus = 100000;
                    break;
            }
        }
        BtlChar_ClearFlagRange(chr, 0xBF, 0xC3);
    }
    if (phase == 1) {
        switch (BtlAnim_GetId(chr)) {
            case 0x173:
            case 0x174:
                BtlAnim_AdvanceLoop(chr, 0);
                BtlChar_SetFlag(chr, 0xE6);
                BtlChar_SetFlag(chr, 0xED);
                if (BtlInput_TestAction(chr, 0x33, 1)) {
                    BtlAct_CountAndMarkOpponent(chr);
                } else if (BtlMember_HasAbility(chr, 0x6A)) {
                    if (chr->actionFrame % 28 == 0) {
                        BtlAct_CountAndMarkOpponent(chr);
                    }
                } else if (BtlMember_HasAbility(chr, 0x69)) {
                    if (chr->actionFrame % 8 == 0) {
                        BtlAct_CountAndMarkOpponent(chr);
                    }
                } else if (BtlMember_HasAbility(chr, 0x68)) {
                    if (chr->actionFrame % 5 == 0) {
                        BtlAct_CountAndMarkOpponent(chr);
                    }
                } else {
                    if (chr->actionFrame % 4 == 0) {
                        BtlAct_CountAndMarkOpponent(chr);
                    }
                }
                if (chr->work[0] & 1) {
                    if (chr->actionFrame % 15 == 0) {
                        BtlAct_CountAndMarkOpponent(chr);
                    }
                }
                if (BtlMember_HasAbility(chr, 0x14)) {
                    if (chr->actionFrame % 5 == 0) {
                        BtlAct_CountAndMarkOpponent(chr);
                    }
                } else if (BtlMember_HasAbility(chr, 0x13)) {
                    if (chr->actionFrame % 11 == 0) {
                        BtlAct_CountAndMarkOpponent(chr);
                    }
                } else if (BtlMember_HasAbility(chr, 0x12)) {
                    if (chr->actionFrame % 17 == 0) {
                        BtlAct_CountAndMarkOpponent(chr);
                    }
                }
                chr->unkD48 = 30;
                if (BtlChar_TestFlag(chr, 0xBF)) {
                    BtlAnim_Request(chr, 0x175, 0.15f);
                    BtlAct_CountAndMarkOpponent(chr);
                    chr->unkD48 = 0;
                    if (*bonus > 0) {
                        BtlMember_AddKi(chr, *bonus);
                        if (BtlChar_TestFlag(chr, 6)) {
                            BtlMember_AddMaxPower(chr, 30000);
                        }
                    }
                }
                if (BtlChar_TestFlag(chr, 0xC0)) {
                    BtlAnim_Request(chr, 0x176, 0.15f);
                }
                break;
            case 0x175:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xB);
                }
                break;
            case 0x176:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xD5);
                }
                if (BtlAnim_PassedRatio(chr, 0.5f)) {
                    BtlMember_Damage(chr, BtlOpp_GetClashCountB(chr) * 60, 8);
                    BtlChar_PlayVoice(chr, 0x1A);
                }
                BtlChar_SetFlag(chr, 0x95);
                BtlChar_SetFlag(chr, 0x96);
                break;
        }
        BtlMove_SteerAtOpponent(chr, BTL_KMH(100.0f), 0.0f, 0.0f, 0.0f, 0.0f, 3.14159265f / 4.0f);
        BtlMove_TurnModelYaw(chr, BTL_DEG(36.0f), 0.3f);
        BtlChar_GetPos(chr)->facing = BtlUtil_WrapAngle(BtlChar_GetPos(chr)->facing + 0.5f);
        BtlMove_SetDirection(chr, 3);
        BtlMove_Advance(chr, BTL_KMH(100.0f), 10000.0f);
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xBB);
        BtlChar_SetFlag(chr, 0x128);
        BtlChar_Vibrate(chr, 1.0f, 0.1f);
        BtlChar_SetFlag(chr, 0xD2);
        BtlChar_SetFlag(chr, 0x134);
        BtlChar_SetFlag(chr, 0xEE);
        BtlChar_SetFlag(chr, 0x13B);
        BtlChar_SetFlag(chr, 0x42);
        BtlChar_SetFlag(chr, 0x43);
        BtlChar_SetFlag(chr, 0x44);
        BtlChar_SetFlag(chr, 0x45);
        BtlChar_SetFlag(chr, 0x46);
    }
    if (phase == 3) {
        chr->clashCountB = 0;
        BtlChar_ClearFlagRange(chr, 0xBF, 0xC3);
    }
}

/* Action 0xFB (forced by flag 0x62): start of clash C. Vanish (animation 0xF6), then seven frames later action 0xFC. */
s32 BtlAct_ClashVanishHandler(BtlActEChr *chr, s32 phase) {
    s32 *counter = &chr->work[2];

    if (phase == 0) {
        BtlAnim_Play(chr, 0xF6, 0.1f);
        chr->clashCountB = 0;
        BtlChar_SetFxBit(chr, 0xC);
        BtlCharSnd_PlayCommon(chr, 0x20);
        BtlChar_GetPos(chr)->unk98 = 0.0f;
        BtlChar_GetPos(chr)->unk9C = 0.0f;
    }
    if (phase == 1) {
        if (BtlAnim_Advance(chr, 0)) {
            BtlChar_SetFlag(chr, 0xB);
            if (++*counter >= 7) {
                BtlAct_Request(chr, 0xFC);
            }
        }
        BtlMove_Step(chr, 2, 5, 2, 0.0f, BTL_KMH(100.0f));
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xBB);
        BtlChar_SetFlag(chr, 0x128);
        BtlChar_SetFlag(chr, 0x42);
        BtlChar_SetFlag(chr, 0x43);
        BtlChar_SetFlag(chr, 0x44);
        BtlChar_SetFlag(chr, 0x45);
        BtlChar_SetFlag(chr, 0x46);
    }
}

/*
 * Action 0xFC: one exchange of clash C (the pair vanishing and reappearing along a stage path).
 * After a wait set by the clash level the fighter strikes (animation 0x4C, 0x3B, 0x4F or 0x40 in turn);
 * the frame on which the button the sequence picked is pressed goes to chr +0xE58 (-2 = wrong button).
 * The sequence answers with flags 0xC5 (again), 0xC6 (over), 0xC7 (won) and 0xC8 (lost).
 */
s32 BtlAct_ClashExchangeHandler(BtlActEChr *chr, s32 phase) {
    BtlActEClashC *c = &gBtlChars->clashC;
    s32 *counter = &chr->work[2];
    s32 wait;
    f32 duration;
    f32 speed;

    if (phase == 0) {
        s32 anim = 0;

        if (BtlAct_GetPrev(chr) != 0xFC) {
            chr->unkE54 = 0;
        } else {
            chr->unkE54++;
        }
        switch (chr->unkE54 % 4) {
            case 0:
                anim = 0x4C;
                break;
            case 1:
                anim = 0x3B;
                break;
            case 2:
                anim = 0x4F;
                break;
            case 3:
                anim = 0x40;
                break;
        }
        BtlAnim_Play(chr, anim, 0.15f);
        BtlAnim_EnableHandle(chr);
        BtlChar_GetPos(chr)->unk98 = 0.0f;
        BtlChar_GetPos(chr)->unk9C = 0.0f;
        BtlChar_SetHeldFlag(chr, 0xE);
        BtlChar_RequestPlaceOnPath(chr);
        chr->unkE58 = -1;
    }
    if (phase == 1) {
        duration = 0.4f;
        speed = 0.0f;
        wait = 15;
        switch (c->level) {
            case 0:
                duration = 0.5f;
                wait = 18;
                break;
            case 1:
                break;
            case 2:
                wait = 12;
                duration = 0.3f;
                break;
        }
        switch (BtlAnim_GetId(chr)) {
            case 0x3B:
            case 0x40:
            case 0x4C:
            case 0x4F:
                if (*counter < wait) {
                    ++*counter;
                    BtlChar_SetFlag(chr, 0xB);
                } else {
                    BtlAnim_SetDuration(chr, duration);
                    BtlAnim_Advance(chr, 0);
                    if (BtlAnim_GetProgress(chr) < 0.1f) {
                        BtlChar_SetFlag(chr, 0xB);
                    } else {
                        speed = BTL_KMH(2000.0f);
                    }
                    if (BtlAnim_PassedRatio(chr, 0.1f)) {
                        BtlChar_SetFxBit(chr, 0xC);
                        BtlChar_Vibrate(chr, 1.0f, 0.3f);
                        if (chr->player == 0) {
                            BtlChar_SetFxBit(chr, 0x2D);
                            BtlCharSnd_PlayCommon(chr, 0x4F);
                            BtlCharSnd_PlayCommon(chr, 0x4B);
                        }
                    }
                    if (BtlAnim_GetProgress(chr) > 0.7f) {
                        BtlChar_SetFlag(chr, 0xB);
                        BtlChar_SetFlag(chr, 0xC4);
                    }
                }
                if (chr->actionFrame > 0) {
                    BtlChar_SetFlag(chr, c->pick + 0xE9);
                    if (chr->unkE58 == -1) {
                        if (BtlInput_IsPressed(chr, 0x78000000)) {
                            if (BtlInput_IsPressed(chr, 0x8000000 << c->pick)) {
                                chr->unkE58 = chr->actionFrame;
                                if (!chr->injected) {
                                    BtlCharSnd_PlayCommonFar(chr, 0x14);
                                }
                            } else if (c->prevPick < 0 || !BtlInput_IsPressed(chr, 0x8000000 << c->prevPick)) {
                                chr->unkE58 = -2;
                            }
                        }
                    }
                }
                BtlChar_SetFlag(chr, 0x18);
                chr->unkD48 = 30;
                break;
            case 0x175:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xB);
                }
                break;
            case 0x176:
                if (BtlAnim_Advance(chr, 0)) {
                    BtlAct_Request(chr, 0xD5);
                }
                if (BtlAnim_PassedRatio(chr, 0.5f)) {
                    BtlMember_Damage(chr, BtlOpp_GetClashCountB(chr) * 600, 8);
                    BtlChar_PlayVoice(chr, 0x1A);
                }
                BtlChar_SetFlag(chr, 0x95);
                BtlChar_SetFlag(chr, 0x96);
                break;
        }
        BtlMove_Step(chr, 2, 5, 2, speed, 10000.0f);
        BtlMove_ApplyGravity(chr);
        BtlChar_SetFlag(chr, 0xBB);
        BtlChar_SetFlag(chr, 0x128);
        BtlChar_SetFlag(chr, 0x134);
        BtlChar_SetFlag(chr, 0xEE);
        BtlChar_SetFlag(chr, 0x42);
        BtlChar_SetFlag(chr, 0x43);
        BtlChar_SetFlag(chr, 0x44);
        BtlChar_SetFlag(chr, 0x45);
        BtlChar_SetFlag(chr, 0x46);
        BtlChar_SetFlag(chr, 0x91);
    }
    if (phase == 2) {
        if (BtlChar_TestFlag(chr, 0xC5)) {
            BtlAct_Request(chr, 0xFC);
        }
        if (BtlChar_TestFlag(chr, 0xC6)) {
            BtlAct_Request(chr, 0xB);
        }
        if (BtlChar_TestFlag(chr, 0xC7)) {
            BtlAnim_Request(chr, 0x175, 0.0f);
            chr->unkD48 = 0;
        }
        if (BtlChar_TestFlag(chr, 0xC8)) {
            BtlAnim_Request(chr, 0x176, 0.0f);
        }
    }
    if (phase == 3) {
        if (BtlAct_GetRequested(chr) != 0xFC) {
            chr->clashCountB = 0;
        }
    }
}

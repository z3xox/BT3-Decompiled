#ifndef BATTLE_BTL_INPUT_H
#define BATTLE_BTL_INPUT_H

#include "types.h"
#include "battle/battle.h"
#include "battle/btl_fighter.h"

/*
 * Per-fighter battle input (src/battle/btl_input.c, 0x1D3B40..0x1D4F30).
 * The data flow is described at the top of btl_input.c. What matters to anything that wants to feed or
 * synchronise input (replay, netplay):
 *
 * - The whole controller state the simulation sees for one fighter and one frame is BtlInputRecord:
 *   {buttons, stickX, stickY} plus the command word, which is a pure function of buttons, the previous record's
 *   buttons, the recCount frame counters and three pieces of fighter state (BtlInput_BuildCommands,
 *   BtlInput_TestSwitch: chr->techClass, fighter flags 0xA2 / 0xA3, the entry of BtlSuper_GetPromptRow).
 *   The game's own replay feature stores only {buttons, stickX, stickY} per fighter per frame
 *   (BtlReplay_Record writes, BtlReplay_Play reads; gBattleReplay data at 0x301810: per player 9000 x u8[2] stick,
 *   9000 x u32 buttons, s32 count, s32 position = 0xD2F8 bytes, then a flag word whose bit 0 means "ran out")
 *   and rebuilds the commands, so that triple is sufficient as far as this file is concerned.
 * - The record is only recorded / replaced by the replay when the fighter takes input this frame (fighter flag
 *   2 or 3 set, flag 0x136 clear, BtlChars_IsTimeStopped() == 0); otherwise it is forced neutral and the replay cursor
 *   does not move.
 * - Key config (SaveData.key) and the pad status are applied BEFORE the record is made; the double-tap bits
 *   (BTLB_ASCEND_TAP2 / BTLB_DESCEND_TAP2) are part of the recorded button word and are not re-derived.
 * - The ring between record and consumer never holds more than the current frame: BtlInput_Fetch is the only
 *   caller of push and pop, the public BtlInput_Push / BtlInput_Pop / BtlInput_GetQueued have no callers, and
 *   nothing else writes ringWrite / ringRead. There is no input delay in the shipped game; the ring (capacity
 *   7 frames) is the natural place to add one.
 * - After the ring, BtlInput_Fetch can blank or mask the input from battle state (BTL_INPUT_FLAG_*, fighter
 *   flags 0x11E, 0x11F, 0xB1). This gating is not part of the record.
 * - CPU fighters: chr->injectOn is set once, by the fighter reset BtlChar_Reset, to BattleSide_IsCpu(side)
 *   (side control type == 2). The AI writes chr->injectButtons / injectStickX / injectStickY every frame through
 *   BtlCharApi_SetInjectedInput(objId, buttons, x, y) (only caller: BtlAi_SendInput, which passes the AI work's +0x268, +0x26C,
 *   +0x270). The pad is still sampled for such a fighter but not used. Injected input skips the key config and
 *   the double-tap bits, and goes through the same replay hook as pad input.
 */

/* Battle button word (BtlInputRecord.buttons, BtlCharInput.held/pressed/released, BtlChar.injectButtons).
   Bit n is produced from the raw pad word by BtlCharInput.mask[n] (BtlInput_BuildMaskTable), so "action k"
   means "the physical button whose SaveData.key[player][i] is k". Default assignment in brackets.
   The gameplay meaning in the names comes from the default layout and the command builder: guess. */
#define BTLB_GUARD        0x00000001 /* action 2 [Circle] */
#define BTLB_DASH         0x00000002 /* action 1 [Cross] */
#define BTLB_BLAST        0x00000004 /* action 3 [Triangle] */
#define BTLB_RUSH         0x00000008 /* action 0 [Square] */
#define BTLB_UP           0x00000010 /* d-pad up, or left stick up (BtlInput_MergeStick) */
#define BTLB_DOWN         0x00000020 /* d-pad down, or stick */
#define BTLB_LEFT         0x00000040 /* d-pad left, or stick */
#define BTLB_RIGHT        0x00000080 /* d-pad right, or stick */
#define BTLB_RS_LEFT      0x00000100 /* right stick left (PAD_RSTICK_LEFT) */
#define BTLB_CHARGE       0x00000200 /* action 5 [L2] */
#define BTLB_RS_RIGHT     0x00000400 /* right stick right */
#define BTLB_ASCEND       0x00000800 /* action 6 [R1] */
#define BTLB_DESCEND      0x00001000 /* action 7 [R2] */
#define BTLB_R3           0x00002000 /* R3 */
#define BTLB_LOCKON       0x00004000 /* action 4 [L1] */
#define BTLB_L3R3         0x00008000 /* L3 and R3 together (the only mask where every bit must be down) */
#define BTLB_RS_UP        0x00010000 /* right stick up */
#define BTLB_RS_DOWN      0x00020000 /* right stick down */
#define BTLB_RS_LEFT2     0x00040000 /* right stick left again */
#define BTLB_RS_RIGHT2    0x00080000 /* right stick right again */
#define BTLB_ANY_FACE     0x00100000 /* any of actions 0-3; for injected input: copy of GUARD */
#define BTLB_ASCEND_TAP2  0x00200000 /* ASCEND pressed again within 7 frames (BtlInput_AddDoubleTaps); mask is 0 */
#define BTLB_DESCEND_TAP2 0x00400000 /* DESCEND pressed again within 7 frames; mask is 0 */
#define BTLB_SELECT       0x00800000 /* SELECT */
#define BTLB_BLAST2       0x01000000 /* action 3 again; for injected input: copy of BLAST */
#define BTLB_RS_UP2       0x02000000 /* right stick up again; injected: copy of RS_UP */
#define BTLB_RS_DOWN2     0x04000000 /* right stick down again; injected: copy of RS_DOWN */
#define BTLB_CIRCLE       0x08000000 /* the physical buttons, not remapped */
#define BTLB_CROSS        0x10000000
#define BTLB_TRIANGLE     0x20000000
#define BTLB_SQUARE       0x40000000
#define BTLB_DIR_MASK     0x000000F0

/* BattleWork.flags bits tested by BtlInput_Fetch (battle/battle.h). */
#define BTL_INPUT_FLAG_BLOCK  BATTLE_FLAG_DEMO  /* every fighter's fetched input is neutral */
#define BTL_INPUT_FLAG_FILTER BATTLE_FLAG_READY /* fetched input is masked (below); neutral instead on stages 4 and 27 */

/* Buttons / commands that still get through under BTL_INPUT_FLAG_FILTER. */
#define BTLB_FILTER_MASK  0x008F18F3
#define BTLC_FILTER_MASK  0x3C010011

/* Command word (BtlInputRecord.commands, BtlCharInput.cmdHeld/...), built from the button word of this
   frame and of the previous frame by BtlInput_BuildCommands. H = held, P = pressed this frame, R = released
   this frame; "dir" = any of UP/DOWN/LEFT/RIGHT. The names say what sets the bit, not what consumes it. */
#define BTLC_DASH_HELD      0x00000001 /* H DASH, CHARGE not held */
#define BTLC_DASH_CHARGE_P  0x00000002 /* P DASH while H CHARGE */
#define BTLC_DASH_P         0x00000004 /* P DASH */
#define BTLC_DASH_CHARGE_H  0x00000008 /* H DASH while H CHARGE */
#define BTLC_GUARD_H        0x00000010 /* H GUARD */
#define BTLC_RUSH_TAP       0x00000020 /* R RUSH less than 7 frames after its press */
#define BTLC_RUSH_HOLD6     0x00000040 /* RUSH held for exactly 6 frames since its press */
#define BTLC_RUSH_H         0x00000080 /* H RUSH */
#define BTLC_RUSH_NOT_H     0x00000100 /* RUSH not held */
#define BTLC_CHARGE_BLAST   0x00000200 /* H CHARGE, P BLAST, neither DOWN nor UP held */
#define BTLC_CHARGE_BLAST_U 0x00000400 /* H CHARGE, P BLAST, H UP (DOWN not held) */
#define BTLC_CHARGE_BLAST_D 0x00000800 /* H CHARGE, P BLAST, H DOWN */
#define BTLC_SWITCH         0x00001000 /* BtlInput_TestSwitch() while chr->techClass >= 2 */
#define BTLC_UNUSED2000     0x00002000 /* never set */
#define BTLC_CHARGE_GUARD   0x00004000 /* H CHARGE, P GUARD, UP not held */
#define BTLC_CHARGE_GUARD_U 0x00008000 /* H CHARGE, P GUARD, H UP */
#define BTLC_DASH_P2        0x00010000 /* P DASH (set on every path together with one of the DIR bits below) */
#define BTLC_DASH_TAP2      0x00020000 /* P DASH, no dir held, previous DASH press less than 7 frames ago */
#define BTLC_DASH_P3        0x00040000 /* P DASH */
#define BTLC_GUARD_SIDE     0x00080000 /* H GUARD and P LEFT or P RIGHT */
#define BTLC_GUARD_H2       0x00100000 /* H GUARD */
#define BTLC_BLAST_P        0x00200000 /* P BLAST, CHARGE not held */
#define BTLC_LOCKON_P       0x00400000 /* P LOCKON */
#define BTLC_DASH_P4        0x00800000 /* P DASH */
#define BTLC_RUSH_P         0x01000000 /* P RUSH */
#define BTLC_DIR_P          0x02000000 /* P any dir */
/* Direction qualifier of the DASH press / held RUSH / BLAST press that set the bits above (see the function:
   DASH ors in every held direction, UP when none; RUSH takes the first of UP, DOWN, LEFT, RIGHT;
   BLAST takes the first of UP, LEFT, RIGHT and DOWN otherwise). */
#define BTLC_DIR_LEFT       0x04000000
#define BTLC_DIR_RIGHT      0x08000000
#define BTLC_DIR_UP         0x10000000
#define BTLC_DIR_DOWN       0x20000000

#define BTL_STICK_CENTER 0x7F /* quantised stick: 0 = -1 (left / up), 0x7F = 0, 0xFE = +1 (right / down) */
#define BTL_TAP_FRAMES 7      /* window of the double taps and of RUSH_TAP */
#define BTL_COUNT_MAX 100     /* the frame counters saturate here */


/* Partial view of a fighter (0x1600 bytes, BtlChar_Get): only what this file touches. */
typedef BtlFighter BtlInputChr;

/* Entry returned by BtlSuper_GetPromptRow (0x14 bytes; only the field read here). */
typedef struct BtlInputSwitchEntry {
    /* 0x00 */ u8 unk0[2];
    /* 0x02 */ s8 dir; /* 0 up, 1 down, 2 left, 3 right: direction to hold with RUSH for BTLC_SWITCH */
} BtlInputSwitchEntry;

u8 BtlInput_StickToByte(f32 v);
f32 BtlInput_ByteToStick(u8 b);
BtlCharInput *BtlInput_Get(BtlInputChr *chr);
u32 BtlInput_RemapButtons(u32 rawHeld, u32 *mask);
void BtlInput_TickCounter(s32 cond, s8 *count);
void BtlInput_TickCounters(BtlInputCounters *c, u32 held, u32 pressed, u32 released);
void BtlInput_RingPush(BtlInputChr *chr, u32 buttons, u32 commands, u8 *stick);
void BtlInput_RingPop(BtlInputChr *chr, u32 *buttons, u32 *commands, u8 *stick);
u32 BtlInput_AddDoubleTaps(BtlInputChr *chr, u32 buttons, u32 prev, s32 padStatus);
s32 BtlInput_TestSwitch(BtlInputChr *chr, s32 techClass, u32 held, u32 pressed);
u32 BtlInput_MergeStick(BtlInputChr *chr, u32 buttons, u8 *stick);
u32 BtlInput_BuildCommands(BtlInputChr *chr, u32 held, u32 prev);
void BtlInput_BuildRecord(BtlInputChr *chr);
void BtlInput_Fetch(BtlInputChr *chr, u32 *buttons, u32 *commands, u8 *stick);
u32 BtlInput_GetKeyMask(BtlInputChr *chr, s32 action);
void BtlInput_BuildMaskTable(BtlInputChr *chr);
void BtlInput_Init(BtlInputChr *chr);
void BtlInput_Sample(BtlInputChr *chr);
void BtlInput_Update(BtlInputChr *chr);
void BtlInput_Stub(void);
void BtlInput_Push(BtlInputChr *chr, u32 buttons, u32 commands, u8 *stick);
void BtlInput_Pop(BtlInputChr *chr, u32 *buttons, u32 *commands, u8 *stick);
s32 BtlInput_GetQueued(BtlInputChr *chr);
s32 BtlInput_IsPressed(BtlInputChr *chr, u32 mask);
s32 BtlInput_IsHeldNotPrev(BtlInputChr *chr, u32 mask);
s32 BtlInput_IsHeld(BtlInputChr *chr, u32 mask);
s32 BtlInput_IsReleased(BtlInputChr *chr, u32 mask);
s32 BtlInput_GetHeldFrames(BtlInputChr *chr, s32 bit);
s32 BtlInput_GetNotHeldFrames(BtlInputChr *chr, s32 bit);
s32 BtlInput_GetFramesSincePress(BtlInputChr *chr, s32 bit);
s32 BtlInput_GetFramesSinceRelease(BtlInputChr *chr, s32 bit);
s32 BtlInput_GetPrevFramesSincePress(BtlInputChr *chr, s32 bit);
s32 BtlInput_IsCmdPressed(BtlInputChr *chr, u32 mask);
s32 BtlInput_IsCmdHeld(BtlInputChr *chr, u32 mask);
s32 BtlInput_IsCmdReleased(BtlInputChr *chr, u32 mask);
s32 BtlInput_GetCmdHeldFrames(BtlInputChr *chr, s32 bit);
s32 BtlInput_GetCmdNotHeldFrames(BtlInputChr *chr, s32 bit);
s32 BtlInput_GetCmdPrevFramesSincePress(BtlInputChr *chr, s32 bit);
void BtlInput_SetFrameBit(BtlInputChr *chr, s32 bit, s32 set);
s32 BtlInput_TestFrameBit(BtlInputChr *chr, s32 bit, s32 set);
void BtlInput_ClearFrameBits(BtlInputChr *chr);

#endif

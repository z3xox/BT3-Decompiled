#ifndef MENU_SIM_EVENTS_H
#define MENU_SIM_EVENTS_H

/* overlay_common.h declares Snd_PlaySe as returning nothing; it returns s32 (see include/menu/char_reference.h). */
#define Snd_PlaySe Snd_PlaySe_menuA
#define Rand_Range Rand_Range_menuA
#include "menu/overlay_common.h"
#undef Snd_PlaySe
#undef Rand_Range
#include "sys/pad.h"
#include "sys/common.h"
extern s32 Snd_PlaySe(u32 mask, s32 id);
/* overlay_common.h has u32 Rand_Range(u32); the shell game (sim_event_shell.c) only matches with a signed result. */
extern s32 Rand_Range(s32 n);

/* The day screen's work area (mode 22): a local view of SimDay (include/menu/sim_day.h has the full layout). */
typedef struct USimDay {
    /* 0x000 */ u8 unk0[0x20];
    /* 0x020 */ MFlash flash[7];    /* [4] = the movie of training 2 and of the mini games */
    /* 0x154 */ u8 unk154[0x3FC];
    /* 0x550 */ s32 flags;
    /* 0x554 */ s32 loadState;
    /* 0x558 */ s32 cur[13];
    /* 0x58C */ s32 timer;
    /* 0x590 */ s32 state;
    /* 0x594 */ s32 prevState;
    /* 0x598 */ s32 talk[2];
    /* 0x5A0 */ s32 msgLine;
    /* 0x5A4 */ s32 menuText;
    /* 0x5A8 */ s32 bgm;
    /* 0x5AC */ s32 wait;
    /* 0x5B0 */ s32 day;
    /* 0x5B4 */ s32 preview[5];
    /* 0x5C8 */ u8 unk5C8[0x59C];
    /* 0xB64 */ s32 event;
    /* 0xB68 */ s32 seq;
    /* 0xB6C */ s32 seqTimer;
    /* 0xB70 */ s32 lastEvent;
    /* 0xB74 */ s32 repeat;
    /* 0xB78 */ s32 faceA;
    /* 0xB7C */ s32 faceB;
    /* 0xB80 */ s32 rank[3];
    /* 0xB8C */ s32 answer;         /* answer of the yes / no window (SimDay_Cmd(0x11)) */
} USimDay;

/* One rank of a training (SimTrain of include/menu/sim_day.h). */
typedef struct USimTrain {
    /* 0x00 */ s32 param;       /* rock game: rocks thrown; = hits needed to win */
    /* 0x04 */ s32 speed;       /* rock game: the rock moves speed * 3 + 3 a frame */
    /* 0x08 */ s32 points;
    /* 0x0C */ s32 gain;
    /* 0x10 */ s32 loss;
    /* 0x14 */ s32 turn;
} USimTrain; /* 0x18 */

extern const USimTrain gSimTrain1[5];   /* 0x3B9088 */
extern const USimTrain gSimTrain2[5];   /* 0x3B9100 */

extern void Flash_ClipSetOffset(MFlash *flash, MFlashRef *ref, s32 x, s32 y);
extern void StreamSe_Stop(s32 se);
extern void Bgm_Stop(void);
extern void SimDay_PlayBgm(void);

/* Work of the rock game (event 32). Uninitialised globals: they sit in the main executable's .bss. */
extern s32 gSimRockX;       /* 0x31EAFC */
extern s32 gSimRockSpeed;   /* 0x31EB00 */
extern s32 gSimRockLeft;    /* 0x31EB04 */
extern s32 gSimRockHits;    /* 0x31EB08 */
extern u8 gSimRockFlags;    /* 0x31EB0C: 1 = swinging, 2 = won */
extern s32 gSimRockRank;    /* 0x31EB10 */

/* Work of the shell game (event 34), in the main executable's .bss like the rock game's. */
extern s32 gSimPopoTarget;  /* 0x31EB14: the figure that holds the mark */
extern s32 gSimPopoState;   /* 0x31EB18: phase of a swap */
extern s32 gSimPopoDx;      /* 0x31EB1C */
extern s32 gSimPopoDy;      /* 0x31EB20 */
extern s32 gSimPopoSpeed;   /* 0x31EB24: index of the speed table */
extern s32 gSimPopoA;       /* 0x31EB28: the pair being swapped, A < B */
extern s32 gSimPopoB;       /* 0x31EB2C */
extern s32 gSimPopoCount;   /* 0x31EB30: swaps done */
extern s32 gSimPopoWon;     /* 0x31EB34 */
extern s32 gSimPopoRank;    /* 0x31EB38 */

extern s32 SimDay_CountItems2(void);
extern void SimDay_TakeItem(void);

s32 SimPopo_Swap(USimDay *day, s32 a, s32 b);
void SimPopo_CursorOn(USimDay *day);
void SimPopo_CursorOff(USimDay *day);

extern void SimDay_AddChange(s32 stat, s32 amount);
extern s32 SimDay_GetStat(s32 stat, s32 preview);
extern void SimDay_GiveItem(void);
extern void SimDay_Cmd(s32 cmd);

#endif

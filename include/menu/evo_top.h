#ifndef MENU_EVO_TOP_H
#define MENU_EVO_TOP_H

#include "menu/overlay_extra.h"

/*
 * Menu overlay DBZP.BIN, 0x39EFC0..0x3A3848 (placeholder stem "menu_x"). Three pieces:
 *
 *   (menu_x.c)  0x39EFC0..0x39FAA8  EvoTop      tail of the top menu of Evolution Z, mode 48: now merged into
 *                                               src/menu/evo_top.c (0x39EB08..0x39FAA8), which uses the
 *                                               EvoTop layout below
 *
 *   option_mode.c  0x39FAA8..0x39FBB8  OptMode_Main, the handler of mode 62 (the options)
 *   option.c  0x39FBB8..0x3A65E8  Option      the options screen object (menu_y.c, its tail, was merged in;
 *                                               include/menu/option.h has the prototypes that half added)
 */

/* ---- EvoTop (evo_top.c): the top menu of Evolution Z (mode 48), guide Krillin ---- */

/* gProgress + 0x7D0: the plate the Evolution Z top menu was left on (-1 when the mode is entered). */
#ifndef EVO_PROGRESS_CURSOR
#define EVO_PROGRESS_CURSOR (*(s32 *)((u8 *)gProgress + 0x7D0))
#endif

#define EVOTOP_FLASH_NUM 1

typedef struct EvoTop {
    /* 0x00 */ u32 *pack;           /* this screen's section of archive 7 (compressed) */
    /* 0x04 */ u32 *res;            /* the same unpacked */
    /* 0x08 */ MFlash flash[EVOTOP_FLASH_NUM];
    /* 0x34 */ void *bg;            /* background picture */
    /* 0x38 */ u8 *tex[13];
    /* 0x6C */ s32 blink;
    /* 0x70 */ s32 talk;
    /* 0x74 */ void *subtitles;
    /* 0x78 */ void *msgText;
    /* 0x7C */ u8 unk7C[0x3C];      /* handed to MsgWin_Init, which ignores it */
    /* 0xB8 */ s32 cursor;          /* plate 0..2 */
    /* 0xBC */ s32 voiceLine;       /* guide line being said / shown, -1 = none */
    /* 0xC0 */ s32 result;          /* 0 = back, 1 / 2 = the plate chosen */
    /* 0xC4 */ s32 timer;           /* idle frames; after the choice, frames until the screen is left */
    /* 0xC8 */ s32 flags;           /* EVOTOP_ */
    /* 0xCC */ f32 cloud;
} EvoTop; /* 0xD0 */

#define EVOTOP_CHOSEN 1
#define EVOTOP_LEAVING 2
#define EVOTOP_STARTED 4       /* "fl_in" was started */
#define EVOTOP_GREETED 8       /* the first voice line was started */
#define EVOTOP_EXPLAIN 0x20    /* plate 2: the guide's explanation is running */

#define EVOTOP_VOICE_BASE 0x86C9
#define EVOTOP_IDLE_FRAMES 0xE10 /* 3600 frames until the guide's idle line */
#define EVOTOP_LINE_EXPLAIN_FIRST 4
#define EVOTOP_LINE_EXPLAIN_LAST 0x10
#define EVOTOP_LINE_IDLE 0x11

/* src/menu/evo_top.c */
void EvoTop_PlayVoice(EvoTop *m, s32 line);          /* plays a guide line and shows it */
s32 EvoTop_Wrap(s32 value, s32 min, s32 max);        /* wraps value into min..max */
void EvoTop_SetPlate(EvoTop *m, s32 on);             /* lights / dims the cursor plate */
void EvoTop_SetPlateText(EvoTop *m);                 /* sets the plates' text rectangles */
void EvoTop_Advance(EvoTop *m);                      /* advances the movie */
void EvoTop_Init(EvoTop *m, s32 section);

void EvoTop_Term(EvoTop *m);
void EvoTop_Draw(EvoTop *m);
void EvoTop_Input(EvoTop *m);
s32 EvoTop_Leave(EvoTop *m);
s32 EvoTop_Run(s32 section);
s32 Option_Init(s32 section);
s32 Option_Run(s32 section);
s32 Option_Input(void);


#endif

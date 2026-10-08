#ifndef MENU_MENU_Y_H
#define MENU_MENU_Y_H

#include "menu/overlay_extra.h"

/* ---- Option (option.c): the options screen, mode 62 ---- */

#define OPTION_FLASH_NUM 1
#define OPTION_PAD_NUM 2
#define OPTION_KEY_NUM 8

typedef struct Option {
    /* 0x000 */ u32 *pack;          /* this screen's section of archive 10 (compressed) */
    /* 0x004 */ u32 *res;           /* the same unpacked: a pack of 39 sections */
    /* 0x008 */ MFlash flash[OPTION_FLASH_NUM];
    /* 0x034 */ void *bg;           /* section 30: background picture */
    /* 0x038 */ u8 *tex[51];
    /* 0x104 */ s32 cursor;         /* OPT_ITEM_: the row the cursor is on */
    /* 0x108 */ s32 state;          /* OPT_ST_ */
    /* 0x10C */ s32 page;           /* 0 top, 1 screen, 2 sound, 3 controller */
    /* 0x110 */ s32 bgmVolume;      /* the save's volumes when the volume picker was opened / last confirmed */
    /* 0x114 */ s32 seVolume;
    /* 0x118 */ s32 bgmCursor;      /* sound test: index into bgmIds */
    /* 0x11C */ s32 bgmTop;         /* first visible row */
    /* 0x120 */ s32 bgmBottom;      /* one past the last visible row (top + 6) */
    /* 0x124 */ s32 bgmExtra;       /* row that scrolls out */
    /* 0x128 */ s32 value;          /* column of a two-way picker, or volume row (0 music, 1 effects) */
    /* 0x12C */ s32 voiceLine;      /* guide line being said, also the subtitle shown */
    /* 0x130 */ s32 picker;         /* which captions the two-way picker shows; 7 and up = the volume window */
    /* 0x134 */ s32 unk134;
    /* 0x138 */ s32 pad;            /* controller page: the player being edited (0 / 1) */
    /* 0x13C */ s32 fromPage;       /* how the top page was reached again (which rows the page lists) */
    /* 0x140 */ s32 ctrlKind;       /* 0 vibration, 1 key assignment */
    /* 0x144 */ s32 screenX;        /* the save's screen position when the adjust page was opened */
    /* 0x148 */ s32 screenY;
    /* 0x14C */ s32 type;           /* controller type shown (0..2) */
    /* 0x150 */ s32 typePrev;       /* the type scrolling out */
    /* 0x154 */ s32 blink;          /* guide's eyes */
    /* 0x158 */ s32 talk;           /* guide's mouth */
    /* 0x15C */ s32 resetStep;      /* step of Option_UpdateReset */
    /* 0x160 */ s32 keyRow;         /* key page: index into the save's key table of the row under the cursor */
    /* 0x164 */ s32 markX;          /* where Option_SetKeyMark puts the cursor mark and the arrows */
    /* 0x168 */ s32 markY;
    /* 0x16C */ s32 arrowX;
    /* 0x170 */ s32 arrowY;
    /* 0x174 */ s32 keyOld;         /* value of that row when the button went down, -1 = none */
    /* 0x178 */ s32 valueOld;       /* the value when a picker was opened (to tell a change) */
    /* 0x17C */ s32 keyCursor;      /* key page: cursor 0..7 (two columns of four) */
    /* 0x180 */ s32 keyEdit;        /* the key page is open */
    /* 0x184 */ s32 keyHeld;        /* confirm is held on the key page this frame */
    /* 0x188 */ s32 mcBusy;         /* a memory card flow is running: Option_Run skips Option_Input */
    /* 0x18C */ s32 started;
    /* 0x190 */ s32 greeted;        /* the second greeting line was started */
    /* 0x194 */ s32 unk194;
    /* 0x198 */ s32 dirty;          /* a setting was changed: save on leaving */
    /* 0x19C */ s32 *bgmIds;        /* section 37 + 0x10: music ids of the sound test */
    /* 0x1A0 */ s32 bgmCount;
    /* 0x1A4 */ void *msgText;      /* section 33 */
    /* 0x1A8 */ void *dialogMsg;    /* section 38: message table of the "reset?" dialog */
    /* 0x1AC */ void *subtitles;    /* section 36 */
    /* 0x1B0 */ u8 unk1B0[0x3C];    /* handed to MsgWin_Init, which ignores it */
    /* 0x1EC */ s32 key[OPTION_PAD_NUM][OPTION_KEY_NUM]; /* the default key assignment */
} Option; /* 0x22C */

#define OPTION_VOICE_BASE 0x873A

/* Option.cursor / Option.state: one number per row of the four pages; a state of the same number means that
   row is open. */
#define OPT_ST_SAVED (-2)      /* the save flow started by leaving has finished */
#define OPT_ST_START (-1)
#define OPT_ST_TOP 0
#define OPT_ITEM_SAVE 1        /* top page: save / load (two-way picker) */
#define OPT_ITEM_SCREEN 2      /* -> screen page */
#define OPT_ITEM_SOUND 3       /* -> sound page */
#define OPT_ITEM_CTRL 4        /* -> controller page */
#define OPT_ITEM_EXIT 5
#define OPT_ST_FADE 6          /* fading out */
#define OPT_ITEM_TYPE 7        /* screen page: three-way type with a picture (save + 0x1694) */
#define OPT_ITEM_SCR2 8        /* screen page: two-way setting (save + 0x1698) */
#define OPT_ITEM_ADJUST 9      /* screen page: screen position */
#define OPT_ITEM_SCR_RESET 10  /* screen page: reset */
#define OPT_ST_ADJUST_RESET 11
#define OPT_ITEM_STEREO 12     /* sound page: stereo / mono */
#define OPT_ITEM_VOLUME 13     /* sound page: music / effect volume */
#define OPT_ITEM_BGM 14        /* sound page: sound test */
#define OPT_ITEM_VOICE 15      /* sound page: voice language */
#define OPT_ITEM_SND_RESET 16  /* sound page: reset */
#define OPT_ITEM_KEYS 17       /* controller page: key assignment */
#define OPT_ITEM_VIB 18        /* controller page: vibration */
#define OPT_ITEM_CTRL_RESET 19 /* controller page: reset */
#define OPT_ST_VIB_EDIT 20
#define OPT_ST_KEYS_PAD 21
#define OPT_ST_KEYS_EDIT 22

extern Option *gOption;

/* Option_PlateGoto: which clip */
#define OPT_CLIP_ROW 0         /* "mc_menu_plate_%d" of the top page's cursor */
#define OPT_CLIP_ROW_SCREEN 1  /* the same on the screen page (cursor - 6) */
#define OPT_CLIP_ROW_SOUND 2   /* the sound page (cursor - 11) */
#define OPT_CLIP_BOTTOM 3      /* "mc_bottom_plate_1" */
#define OPT_CLIP_PICK 4        /* "mc_select_plate_%d" of the two-way picker's column */
#define OPT_CLIP_VIB_OFF 5     /* the vibration picker's plate for the current setting */
#define OPT_CLIP_KEY_OFF 6
#define OPT_CLIP_BGM 7         /* "mc_bgm_plate_%d" of the sound test's cursor */
#define OPT_CLIP_VOL_BGM 8     /* "mc_vol_plate_bgm_%d" of the saved music volume */
#define OPT_CLIP_VOL_SE 9
#define OPT_CLIP_ROW_CTRL 10   /* the controller page (cursor - 16) */

/* second half of the object (was src/menu/menu_y.c) */
void Option_Draw(void);
void Option_PlateGoto(s32 unused, s32 clip, char *label);  /* sends a plate / cursor clip to a label */
s32 Option_SetKeyMark(s32 row);                            /* key page: moves the cursor mark */
void Option_UpdateReset(void);                             /* the reset states (10, 11, 16, 19) */
void Option_DimPickers(void);
void Option_OnSaved(void);                                 /* McFlow "done" callback */
void Option_Term(void);

/* ---- the second half of the object (formerly its own header) ---- */

#include "types.h"
#include "menu/overlay_common.h"
#include "sys/save.h"
#include "sys/pad.h"

/*
 * Menu overlay DBZP.BIN, 0x3A3848..0x3A7D98 (placeholder stem "menu_y"). Two pieces, cut at the object boundary:
 *
 *   (menu_y.c)  0x3A3848..0x3A65E8  Option   tail of the option screen object (mode 62, handler 0x39FAA8; work
 *                                            pointer 0x3BC364): now merged into src/menu/option.c
 *   dc_list.c  0x3A65E8..0x3A9850  DcList   the Data Center's custom character list (mode 55, handler 0x3A9850;
 *                                            menu_z.c, its tail, was merged in): declared in dc.h
 *
 * All names are guesses from what the code does. The structures are this chunk's own views.
 */

/* ---- Main executable, beyond what overlay_common.h declares ---- */

extern void Sprite_SetScissor(s32 x0, s32 x1, s32 y0, s32 y1);
extern void Flash_ClipSetCallbackA(MFlash *flash, MFlashRef *ref, void *fn, void *arg);
extern void Flash_ClipSetCallbackB(MFlash *flash, MFlashRef *ref, void *fn, void *arg);
extern void Flash_ClipSetOffset(MFlash *flash, MFlashRef *ref, s32 x, s32 y);
extern void Flash_ClipSetScale(MFlash *flash, MFlashRef *ref, f32 x, f32 y);
extern void TextBox_AttachLine(MFlash *flash, MFlashRef *ref, s32 x, s32 y, s32 line, MTextBox *box);
extern void IconWin_SetIcon(s32 icon);
extern void Dialog_Draw(s32 visible);
extern void Dialog_Start(s32 kind);
extern s32 Dialog_Input(s32 allowCancel);
extern void Dialog_SetChoices(s32 on);
extern void Dialog_SetMsgTable(void *table);
extern void Dialog_SetMsg(s32 idx);
extern void Dialog_SetCursor(s32 cursor);
extern s32 Dialog_IsClosed(void);
extern void SndOpt_Apply(void);
extern void Dialog_Init(void *file, void *msgTbl, s32 size);
extern void Dialog_Term(void);

/* Snd_PlaySe returns a value in the original (overlay_common.h declares it void). */
#define Snd_PlaySe ((s32 (*)(u32, s32))Snd_PlaySe)

/* Voice_GetStat result when nothing is playing. */
#define Y_VOICE_IDLE 5

/* ---- Option (option.c): the option screen of mode 62 ---- */

/* The work area (Option, gOption) is declared in include/menu/evo_top.h / option.h; src/menu/option.c, the whole object
   since menu_y.c was merged into it, includes that header first. */

/* Option.state values this chunk tests (evo_top.h / option.h: OPT_ITEM_ / OPT_ST_). */
#define YOPT_SCREEN 2          /* top page row: screen page */
#define YOPT_SOUND 3
#define YOPT_CTRL 4
#define YOPT_TYPE 7            /* screen page: controller-layout type */
#define YOPT_ADJUST 9
#define YOPT_SCR_RESET 10
#define YOPT_ADJUST_RESET 11
#define YOPT_STEREO 12
#define YOPT_VOLUME 13
#define YOPT_BGM 14
#define YOPT_SND_RESET 16
#define YOPT_KEYS 17
#define YOPT_CTRL_RESET 19
#define YOPT_KEYS_PAD 21
#define YOPT_KEYS_EDIT 22

/* Option_PlateGoto: which clip */
#define OPT_CLIP_ROW 0         /* "mc_menu_plate_%d" of the top page's cursor */
#define OPT_CLIP_ROW_SCREEN 1  /* the same on the screen page (cursor - 6) */
#define OPT_CLIP_ROW_SOUND 2   /* the sound page (cursor - 11) */
#define OPT_CLIP_BOTTOM 3      /* "mc_bottom_plate_1" */
#define OPT_CLIP_PICK 4        /* "mc_select_plate_%d" of the two-way picker's column */
#define OPT_CLIP_VIB_OFF 5     /* the vibration picker's plate for the current setting */
#define OPT_CLIP_KEY_OFF 6
#define OPT_CLIP_BGM 7         /* "mc_bgm_plate_%d" of the sound test's cursor */
#define OPT_CLIP_VOL_BGM 8     /* "mc_vol_plate_bgm_%d" of the saved music volume */
#define OPT_CLIP_VOL_SE 9
#define OPT_CLIP_ROW_CTRL 10   /* the controller page (cursor - 16) */

void Option_Draw(void);
void Option_PlateGoto(s32 unused, s32 clip, char *label);
s32 Option_SetKeyMark(s32 row);
void Option_UpdateReset(void);
void Option_SetBottomText(MFlash *flash, MFlashRef *ref, char *name, MFlashUv uv);
void Option_SetMenuText(MFlash *flash, MFlashRef *ref, char *name, MFlashUv uv);
void Option_DimPickers(void);
void Option_SetBgmScissor(void);
void Option_ResetScissor(void);
void Option_OnSaved(void);
void Option_Term(void);


/* DcList (dc_list.c, the whole object since menu_z.c was merged into it): see include/menu/dc.h. */

#endif

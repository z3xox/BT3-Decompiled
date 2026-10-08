#ifndef BATTLE_VIEW_A_H
#define BATTLE_VIEW_A_H

#include "types.h"
#include "sys/job.h"
#include "sys/list.h"

/*
 * Menu support code of the main executable, 0x25C2A8..0x2600B0 (src/ui). Six modules:
 *
 *   reward_window.c    0x25C2A8..0x25CFC0  GetWin    the "you got ..." reward window
 *   message_window.c  0x25CFC0..0x25D290  MsgWin    the left / right message window
 *   icon_window.c  0x25D290..0x25D468  IconWin   a small window with a changeable icon
 *   char_viewer.c  0x25D468..0x25DE68  ChrView   the character model viewer (own loop, orbit camera)
 *   menu_util_1.c  0x25DE68..0x25FE00  Progress_Init and the menu helpers (movie-clip animation, number
 *                                   drawing, cursor movement on the character / stage grids, unlock lists)
 *   menu_util_1.c  0x25FE00..0x2600B0  TextBox   the head of the text box module, which goes on to 0x260D20 in the
 *                                   same file (the rest of its declarations are in ui/menu_support.h)
 *
 * Almost every caller is in the menu overlay (DBZP.BIN).
 */

/* ---- The Flash-like movie player (src/sys/flash.c; the full structures are in sys/flash.h). Local view. ---- */

/* A playing movie. */
typedef struct Flash {
    /* 0x00 */ s32 unk0[3]; /* [2] (+0x08) is Flash.flags of sys/flash.h: bit 1 (FLASH_PAD) is set and cleared by the
                               movie itself (action "pad" "true" / "false"): the screen may take input */
    /* 0x0C */ s32 trig;   /* Flash.trig of sys/flash.h, not flags: bit n is set for one frame by the movie's action
                               "trig" "n". Bit 0 is what GetWin_IsAnimDone tests */
    /* 0x10 */ s32 unk10[7];
} Flash; /* size 0x2C */

/* What Flash_FindLabel fills in: a handle to one movie clip. */
typedef struct FlashRef {
    /* 0x00 */ s32 index;      /* negative: not found (`index` in sys/flash.h) */
    /* 0x04 */ s32 more;    /* `more` in sys/flash.h: how many more clips share the name */
} FlashRef; /* size 8 */

/* Texture rectangle of a movie clip (argument of Flash_ClipSetUv). */
typedef struct FlashUv {
    /* 0x00 */ s32 x0;
    /* 0x04 */ s32 y0;
    /* 0x08 */ s32 x1;
    /* 0x0C */ s32 y1;
    /* 0x10 */ s32 unk10;
} FlashUv; /* size 0x14 */

/* Properties set with Flash_ClipSetFlags. */
#define FLASH_PROP_VISIBLE 2
#define FLASH_PROP_UV 0x200

/* Header of an unpacked sprite resource after Res_RelocateOffsets: the textures are 0x40-byte entries. */
typedef struct FlashTexRes {
    /* 0x00 */ s32 unk0[4];
    /* 0x10 */ u8 *tex;
} FlashTexRes;

#define FLASH_TEX_SIZE 0x40

/* A section of a pack file: the header word is a byte offset, rounded down to a multiple of 4. */
#define PACK_AT(pack, n) ((void *)((u8 *)(pack) + ((((u32 *)(pack))[n] >> 2) << 2)))

/* ---- TextBox (menu_util_1.c; the flags and prototypes of the second half are in ui/menu_support.h) ---- */

#define TEXTBOX_FLAG_CLIP 1    /* `clip` is set (TextBox_SetClip): the font clip rectangle */
#define TEXTBOX_FLAG_COLOR 2   /* `color` is set (TextBox_SetColor) */
#define TEXTBOX_FLAG_SHADOW 4  /* `shadow` is set (TextBox_SetShadowColor): the colour of the text's shadow */

/* A text file with the style its lines are drawn in. The full layout is TextBoxFull in ui/menu_support.h. */
typedef struct TextBox {
    /* 0x00 */ s32 flags;      /* TEXTBOX_FLAG_* */
    /* 0x04 */ void *text;     /* text file: line n is at text + (((u32 *)text)[n + 1] & ~3) */
    /* 0x08 */ s32 unk8;
    /* 0x0C */ s32 x;          /* TextBox_SetOffset: added to every line's position (0 or 0x100 in the presets) */
    /* 0x10 */ s32 y;
    /* 0x14 */ u8 unk14[0x1C];
    /* 0x30 */ u8 color[4];    /* r, g, b, a */
    /* 0x34 */ u8 shadow[4];   /* TextBox_SetShadowColor: colour of the text's shadow */
    /* 0x38 */ s32 clip[4];    /* font clip x0, y0, x1, y1; TextBox_SetClip takes them as x0, x1, y0, y1 */
    /* 0x48 */ u8 unk48[8];
    /* 0x50 */ s32 align;      /* Font_SetAlign value: 0 or 2 in the presets, 1 in the reward window */
    /* 0x54 */ u8 unk54[0x2C];
    /* 0x80 */ s32 noFlush;    /* 1: the text is left in the font queue instead of being drawn at once */
    /* 0x84 */ u8 unk84[8];
} TextBox; /* size 0x8C */

void TextBox_Init(TextBox *box, void *text, u32 preset);
void TextBox_SetAlign(TextBox *box, s32 align);
void TextBox_SetNoFlush(TextBox *box, s32 noFlush);
void TextBox_SetOffset(TextBox *box, s32 x, s32 y);
void TextBox_SetClip(TextBox *box, s32 x0, s32 x1, s32 y0, s32 y1);
void TextBox_SetColor(TextBox *box, u32 rgba);
void TextBox_SetShadowColor(TextBox *box, u32 rgba);

/* ---- GetWin (reward_window.c) ---- */

#define GETWIN_TEXT_COUNT 5

/* What is announced (GetWin.kind). GetWin_Setup takes 0, 1, 2, 6, 7, 8, 9 and turns 2 into 3 / 4 / 5. */
#define GETWIN_KIND_0 0
#define GETWIN_KIND_1 1
#define GETWIN_KIND_ITEM 2     /* argument only: becomes 3 + the item's type byte */
#define GETWIN_KIND_ITEM_A 3
#define GETWIN_KIND_ITEM_B 4
#define GETWIN_KIND_ITEM_C 5
#define GETWIN_KIND_MONEY 6    /* the value is drawn as a 7-digit number */
#define GETWIN_KIND_7 7
#define GETWIN_KIND_8 8        /* two texts: a group heading and the entry */
#define GETWIN_KIND_9 9

/* Entry of the table GetWin_Setup(2, n) looks at. */
typedef struct GetWinItem {
    /* 0x00 */ u8 type;
    /* 0x01 */ u8 unk1[0x27];
} GetWinItem; /* size 0x28 */

typedef struct GetWin {
    /* 0x000 */ void *text[GETWIN_TEXT_COUNT];   /* text files of the pack */
    /* 0x014 */ TextBox box[GETWIN_TEXT_COUNT];  /* one per text file */
    /* 0x2D0 */ Flash flash[1];
    /* 0x2FC */ u8 *tex[13];       /* texture list of the movie; [3] and [4] stay NULL */
    /* 0x330 */ GetWinItem *items;
    /* 0x334 */ s32 kind;          /* GETWIN_KIND_* */
    /* 0x338 */ s32 box1;          /* text box of the first line */
    /* 0x33C */ s32 box2;          /* text box of the second line */
    /* 0x340 */ s32 line1;         /* -1 = none */
    /* 0x344 */ s32 line2;         /* line number, or the amount for GETWIN_KIND_MONEY */
    /* 0x348 */ s32 tall;          /* kind 8: the second line has more than one row */
    /* 0x34C */ s32 se;            /* sound effect of the open and "next" animations */
} GetWin; /* size 0x350 */

void GetWin_Init(u32 *pack, s32 lang);
void GetWin_Term(void);
void GetWin_Draw(void);
void GetWin_Open(void);
void GetWin_Close(void);
void GetWin_Next(void);
s32 GetWin_IsAnimDone(void);
void GetWin_Setup(s32 kind, s32 value);

/* ---- MsgWin (message_window.c) ---- */

typedef struct MsgWin {
    /* 0x00 */ void *text;     /* NULL: the window is drawn without text */
    /* 0x04 */ Flash flash[1];
    /* 0x30 */ u8 *tex[4];
    /* 0x40 */ s32 side;       /* 0 = slides in from the left, else from the right */
    /* 0x44 */ TextBox box;
} MsgWin; /* size 0xD0 */

void MsgWin_Init(u32 *pack, void *text, s32 side);
void MsgWin_Term(void);
void MsgWin_Draw(s32 unused0, s32 unused1, s32 line);
void MsgWin_Open(void);
void MsgWin_Close(void);
void MsgWin_SetText(void *text);
void MsgWin_SetSide(s32 side);
void MsgWin_SetSpacing(s32 spacingX, s32 spacingY);

/* ---- IconWin (icon_window.c) ---- */

typedef struct IconWin {
    /* 0x00 */ Flash flash;
    /* 0x2C */ FlashTexRes *icons; /* second argument of IconWin_Init */
    /* 0x30 */ u8 *tex[5];         /* [4] is the icon shown */
    /* 0x44 */ s32 unk44;
} IconWin; /* size 0x48 */

void IconWin_Init(u32 *pack, FlashTexRes *icons);
void IconWin_Term(void);
void IconWin_Draw(void);
void IconWin_Open(void);
void IconWin_Close(void);
void IconWin_SetIcon(s32 icon);

/* ---- ChrView (char_viewer.c) ---- */

#define CHRVIEW_SLOT_COUNT 2
#define CHRVIEW_MODEL_SIZE 0xCE000
#define CHRVIEW_FILE_FIRST 0x590   /* model of character c, costume n: 0x590 + c * 10 + n (+4 for the damaged model) */
#define CHRVIEW_FILE_STAGE 0x197   /* the viewer's backdrop, loaded once by ChrView_Init */
#define CHRVIEW_CHARA_COUNT 0xA1

/* One file of a model resource slot (BtlResFile in battle/btl_obj.h). */
typedef struct ChrViewFile {
    /* 0x00 */ void *buf;
    /* 0x04 */ s32 size;
    /* 0x08 */ s32 id;         /* file id, -1 = none */
    /* 0x0C */ void *orig;
} ChrViewFile; /* size 0x10 */

/* A model buffer of the viewer. `file` is handed to BtlRes_LoadSingle and BtlObj_Create as a BtlResSlot, of
 * which it is only the first 0x30 bytes (the three files; only file[0] is used). */
typedef struct ChrViewSlot {
    /* 0x00 */ SListNode node;
    /* 0x04 */ s32 obj;        /* BtlObj id */
    /* 0x08 */ ChrViewFile file[3]; /* [0].buf: CHRVIEW_MODEL_SIZE bytes */
} ChrViewSlot; /* size 0x38 */

/* The viewer's load job (the first 0x24 bytes of ChrView). */
typedef struct ChrViewJob {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 (*step)(struct ChrViewJob *job);
    /* 0x08 */ s32 state;
    /* 0x0C */ s32 count;      /* frame counter of the waiting states */
    /* 0x10 */ s32 chara;
    /* 0x14 */ s32 costume;
    /* 0x18 */ s32 damaged;
    /* 0x1C */ ChrViewSlot *slot;
    /* 0x20 */ u8 *obj;        /* BtlObj of the new model */
} ChrViewJob; /* size 0x24 */

typedef struct ChrView {
    /* 0x00 */ ChrViewJob job;
    /* 0x24 */ s32 loading;    /* 1 while the load job runs */
    /* 0x28 */ s32 anim;       /* model animation played on a new model: 7 */
    /* 0x2C */ s32 delay;      /* frames the job waits before it starts loading: 0 */
    /* 0x30 */ s32 stageOn;    /* 1 once a model is shown: the stage is updated and drawn */
    /* 0x34 */ s32 visible;    /* ChrView_IsVisible: the 3D scene is drawn */
    /* 0x38 */ s32 zoom;       /* non-zero: the camera distance may be changed between 3 and 100 */
    /* 0x3C */ ChrViewSlot *cur; /* the model on screen */
    /* 0x40 */ ChrViewSlot slot[CHRVIEW_SLOT_COUNT];
    /* 0xB0 */ SList free;
} ChrView; /* size 0xBC */

/* Entry of the character table of common file 4 (gCommonRes->data[2], section 1; see ChrView_GetCharaInfo). */
typedef struct ChrViewInfo {
    /* 0x00 */ u8 unk0[0x30];
    /* 0x30 */ f32 targetY;    /* height the camera looks at */
    /* 0x34 */ f32 dist;       /* camera distance */
    /* 0x38 */ u8 unk38[4];
} ChrViewInfo; /* size 0x3C */

s32 ChrView_GetModelFile(s32 chara, s32 costume, s32 damaged);
ChrViewInfo *ChrView_GetCharaInfo(u32 chara);
ChrViewSlot *ChrView_AllocSlot(void);
s32 ChrView_FreeSlot(ChrViewSlot *slot);
ChrView *ChrView_Get(void);
ChrViewSlot *ChrView_GetCur(void);
void ChrView_SetCur(ChrViewSlot *slot);
s32 ChrView_IsLoading(void);
void ChrView_SetLoading(s32 loading);
void ChrView_SetupCamera(s32 chara);
s32 ChrView_StepLoad(ChrViewJob *job);
s32 ChrView_StepLoadStage(ChrViewJob *job);
s32 ChrView_CancelLoad(void);
s32 ChrView_Hide(void);
s32 ChrView_Show(s32 chara, s32 costume, s32 damaged);
void ChrView_Update(void);
s32 ChrView_IsVisible(void);
void ChrView_Init(void);
void ChrView_Term(void);

/* ---- Progress and the menu helpers (menu_util_1.c) ---- */

#define PROGRESS_FLAG_40 0x40
#define PROGRESS_FLAG_FREEZE 0x100 /* the clip animations of this file do nothing while it is set */

/* 0x30 bytes; two arrays of five in the progress block. */
typedef struct ProgressEntry {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ s32 unk4[11];
} ProgressEntry;

/* gProgress: the 0x7FC-byte block of state that survives between the menu and the battle. Local view. */
typedef struct ViewProgress {
    /* 0x000 */ s32 language;
    /* 0x004 */ s32 baseFile;          /* 0x1C1 at start: first file id of the common files (`baseFile` in the menu headers) */
    /* 0x008 */ void *loadPack;    /* 0x3000 bytes */
    /* 0x00C */ void *loadRes;     /* 0x6800 bytes */
    /* 0x010 */ void *loadSprites; /* 0x380 bytes */
    /* 0x014 */ s32 flags;         /* PROGRESS_FLAG_* */
    /* 0x018 */ s32 mode;          /* the menu screen / game mode */
    /* 0x01C */ s32 unk1C[2];
    /* 0x024 */ s32 demoPick;         /* -1 at start: `demoPick` in ui/menu_support.h, the demo battle's last pairing */
    /* 0x028 */ s32 unk28[2];
    /* 0x030 */ u8 unk30[0x4C];    /* cleared per session */
    /* 0x07C */ u8 unk7C[0x3C4];   /* cleared per session */
    /* 0x440 */ ProgressEntry unk440[5]; /* cleared per session; language = 0 */
    /* 0x530 */ ProgressEntry unk530[5]; /* language = 1 */
    /* 0x620 */ s32 unk620;
    /* 0x624 */ s32 battleType;        /* `battleType` in the menu headers; 2 in mode 0x28: the grid is built without the
                                      random cell (CHRGRID_NO_RANDOM in ChrGrid_Build) */
    /* 0x628 */ s32 unk628[3];
    /* 0x634 */ u8 unk634[0x58];   /* cleared per session */
    /* 0x68C */ u8 unk68C[0x144];  /* cleared per session; +0x68C replayFlags, +0x69C the seven replay slot entries
                                      (McFlowSlotInfo in sys/memcard_flow.c, ProgressTeam in ui/menu_support.h) */
    /* 0x7D0 */ s32 unk7D0;        /* cleared per session */
    /* 0x7D4 */ u8 unk7D4[0x28];   /* cleared per session */
} ViewProgress; /* size 0x7FC */

/* Character grid: 7 columns. A cell is a character or a marker. */
#define CHRGRID_COLS 7
#define CHRGRID_ID_LOCKED 0xA2   /* a locked character: the cursor skips it */
#define CHRGRID_ID_CUSTOM 0xA3   /* the saved custom characters: selectable like a character */
#define CHRGRID_ID_EMPTY 0xA4    /* filler: the cursor skips it */
#define CHRGRID_ID_RANDOM 0xA1   /* the "random" cell (first marker id: ids >= 0xA1 are not characters) */
#define CHRGRID_FORM_MAX 7

/* A cell of the character grid. */
typedef struct ChrGridCell {
    /* 0x00 */ s32 id;                     /* character id (0..0xA0) or CHRGRID_ID_* */
    /* 0x04 */ s32 formCount;              /* number of forms listed, 0 = the cell is the one character */
    /* 0x08 */ s32 form[CHRGRID_FORM_MAX]; /* character ids of the forms */
} ChrGridCell; /* size 0x24 */

/* Stage grid: 6 columns of stage ids. */
#define STGGRID_COLS 6
#define STGGRID_ID_LOCKED 0x24
#define STGGRID_ID_EMPTY 0x25

#define BGMLIST_ID_RANDOM 0x18
#define BGMLIST_ID_LOCKED 0x19

/* How Num_DrawEx finds and draws its digit clips. */
#define NUM_FLAG_ZEROS 1       /* leading zeros are drawn */
#define NUM_FLAG_SIGN 2        /* the clip in front of the first digit shows cell 10 when the value is not 0 */
#define NUM_FLAG_CHILD_FMT 4   /* the format names a child of clip `a` */
#define NUM_FLAG_PARENT_FMT 8  /* the format names the parent of clip `b` */
#define NUM_FLAG_BLANK 0x10    /* every digit shows cell 10 */

typedef struct NumStyle {
    /* 0x00 */ s32 flags;      /* NUM_FLAG_* */
    /* 0x04 */ s32 unk4[2];
    /* 0x0C */ s32 w;          /* size of one digit cell; the sheet has 4 cells per row */
    /* 0x10 */ s32 h;
    /* 0x14 */ s32 first;      /* number of the clip of the last digit */
    /* 0x18 */ s32 count;      /* digits, 0 = as many as the value has */
} NumStyle;

void Progress_Init(void);
void Progress_ClearSession(void);
void FlashAnim_Blink(Flash *flash, FlashRef *ref, s32 *timer, s32 frame);
void FlashAnim_ShowNext(Flash *flash, FlashRef *ref, s32 frame);
void FlashAnim_Talk(Flash *flash, FlashRef *ref, s32 *timer, s32 frame);
void FlashAnim_ShowNext2(Flash *flash, FlashRef *ref, s32 frame);
void FlashAnim_Sheet(Flash *flash, FlashRef *ref, s32 *timer, s32 *frame, FlashUv *uv, s32 cols, s32 rows, s32 period);
void FlashAnim_Scroll(Flash *flash, FlashRef *ref, FlashUv *uv, f32 *x, f32 *y, f32 dx, f32 dy);
void Num_ToDigits(u8 *digits, s32 value, s32 count, s32 zeroPad);
void Num_DrawDigits(Flash *flash, FlashRef *refs, s32 x, s32 y, FlashUv *cells, u8 *digits);
s32 Num_Pow(s32 base, s32 exp);
void Num_Draw(Flash *flash, char *fmt, s32 first, s32 count, s32 value, s32 w, s32 h, s32 mode);
void Num_DrawChild(Flash *flash, char *parent, char *fmt, s32 first, s32 count, s32 value, s32 w, s32 h, s32 mode,
                   s32 parentFmt);
s32 Num_CountDigits(s32 value);
void Num_DrawEx(Flash *flash, NumStyle *style, char *a, char *b, s32 value);
s32 ChrGrid_IsSelectable(ChrGridCell *cells, s32 index);
s32 ChrGrid_MoveRight(ChrGridCell *cells, s32 *col, s32 row);
s32 ChrGrid_MoveLeft(ChrGridCell *cells, s32 *col, s32 row);
void ChrGrid_MoveDown(ChrGridCell *cells, s32 *col, s32 *row, s32 rows);
void ChrGrid_MoveUp(ChrGridCell *cells, s32 *col, s32 *row, s32 rows);
void ChrGrid_NextForm(s32 *forms, s32 *index);
void ChrGrid_PrevForm(s32 *forms, s32 *index);
s32 ChrGrid_FixCursor(ChrGridCell *cells, s32 *col, s32 *row, s32 rows);
s32 StgGrid_IsSelectable(s32 *ids, s32 index);
void StgGrid_MoveRight(s32 *ids, s32 *col, s32 row);
void StgGrid_MoveLeft(s32 *ids, s32 *col, s32 row);
s32 StgGrid_MoveRightWrap(s32 *ids, s32 *col, s32 row);
void StgGrid_MoveDown(s32 *ids, s32 *col, s32 *row, s32 rows);
void StgGrid_MoveUp(s32 *ids, s32 *col, s32 *row, s32 rows);
void ChrGrid_Build(s32 *outCount, ChrGridCell *out, s32 *inCount, ChrGridCell *in, s32 *customCount,
                   ChrGridCell *custom);
void StgGrid_ApplyUnlocks(s32 *count, s32 *ids);
void BgmList_ApplyUnlocks(s32 *count, s32 *ids);

#endif

#include "common.h"
#include "ui/reward_window.h"
#include "sys/heap.h"

/*
 * MsgWin, 0x25CFC0..0x25D290: a message window that slides in from the left or the right and shows one line
 * of a text file ("mc_message_text"). Used all over the menu overlay. See battle/view_a.h.
 */

extern void *memset(void *dst, s32 c, u32 n);

extern void Res_RelocateOffsets(void *out, void *base, void *hdr);
extern void Flash_Create(Flash *flash, void *data, void *tex);
extern void Flash_Destroy(Flash *flash);
extern void Flash_Advance(Flash *flash);
extern void Flash_Draw(Flash *flash);
extern void Flash_Play(Flash *flash, s32 arg);
extern void Flash_GotoLabel(Flash *flash, char *label, s32 arg);
extern void Flash_FindLabel(Flash *flash, char *parent, char *name, FlashRef *out);
extern void Font_FlushAll(void);

/* text box module, after 0x2600B0 (not decompiled) */
extern void TextBox_AttachLine(Flash *flash, FlashRef *ref, s32 a, s32 b, s32 line, TextBox *box);
extern void TextBox_SetSpacing(TextBox *box, s32 a, s32 b);

/* Defined here: this object's .sdata (0x2FF0D8). */
MsgWin *gMsgWin = NULL;

/* Allocates the message window: movie and textures from the pack, text box preset 5 on `text`. */
void MsgWin_Init(u32 *pack, void *text, s32 side) {
    FlashTexRes *res = NULL;

    gMsgWin = Heap_Alloc(sizeof(MsgWin), 0x20, 0, HEAP_ANY);
    memset(gMsgWin, 0, sizeof(MsgWin));

    res = PACK_AT(pack, 1);
    Res_RelocateOffsets(&res, res, res);
    gMsgWin->tex[0] = res->tex;
    gMsgWin->tex[1] = res->tex + FLASH_TEX_SIZE;

    Flash_Create(gMsgWin->flash, PACK_AT(pack, 2), gMsgWin->tex);
    Flash_Play(gMsgWin->flash, 1);
    TextBox_Init(&gMsgWin->box, text, 5);
    gMsgWin->text = text;
    gMsgWin->side = side;
}

/* Destroys the movie and frees the window. */
void MsgWin_Term(void) {
    Flash_Destroy(gMsgWin->flash);
    if (gMsgWin != NULL) {
        Heap_Free(gMsgWin);
        gMsgWin = NULL;
    }
}

/* Steps and draws the window with one line of its text file. The first two arguments are not used. */
void MsgWin_Draw(s32 unused0, s32 unused1, s32 line) {
    FlashRef ref;
    Flash *flash;
    s32 i;

    Flash_Advance(gMsgWin->flash);
    flash = gMsgWin->flash;
    if (gMsgWin->text != NULL) {
        Flash_FindLabel(flash, NULL, "mc_message_text", &ref);
        TextBox_AttachLine(flash, &ref, 0, 0, line, &gMsgWin->box);
    }
    for (i = 0; i < 1; i++) {
        Flash_Draw(&gMsgWin->flash[i]);
    }
    Font_FlushAll();
}

/* Starts the slide-in animation of the window's side. */
void MsgWin_Open(void) {
    if (gMsgWin->side == 0) {
        Flash_GotoLabel(gMsgWin->flash, "fl_l_in", 1);
    } else {
        Flash_GotoLabel(gMsgWin->flash, "fl_r_in", 1);
    }
}

/* Starts the slide-out animation of the window's side. */
void MsgWin_Close(void) {
    if (gMsgWin->side == 0) {
        Flash_GotoLabel(gMsgWin->flash, "fl_l_out", 1);
    } else {
        Flash_GotoLabel(gMsgWin->flash, "fl_r_out", 1);
    }
}

/* Changes the text file of the window (NULL: no text). */
void MsgWin_SetText(void *text) {
    gMsgWin->text = text;
    gMsgWin->box.text = text;
}

/* Chooses the side the window slides in from. */
void MsgWin_SetSide(s32 side) {
    gMsgWin->side = side;
}

/* Passes two values to TextBox_SetSpacing for the window's text box. */
void MsgWin_SetBoxParam(s32 a, s32 b) {
    TextBox_SetSpacing(&gMsgWin->box, a, b);
}

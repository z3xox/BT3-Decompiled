/*
 * The text printer, whole object: 0x239BB0..0x23C310 (formerly col_b_b.c + font.c, merged at integration).
 *
 * First part, 0x239BB0..0x239EA0: the GS packet helpers. Not collision code. Drawing only; nothing here touches
 * the simulation.
 *
 * A packet is written through a CbFontOut (a pointer into the display list). Screen pixel (x, y) becomes the GS
 * primitive coordinate ((x + 0x700) << 4, (y + 0x720) << 4): the window origin is (1792, 1824).
 */
#include "common.h"
#include "battle/col_primitives.h"

extern u64 *Dma_BeginDirect(void);
extern void Dma_EndDirect(u64 *end);
extern void Gfx_PutDefaultEnv(CbFontOut *out);

/* A rectangle in GS primitive coordinates lies inside the 0..4095 range. */
s32 Font_IsInGsRange(s32 x0, s32 y0, s32 x1, s32 y1) {
    return x0 > 0 && x1 < 0x1000 && y0 > 0 && y1 < 0x1000;
}

/* A rectangle in screen pixels overlaps the printer's clip rectangle. */
s32 Font_IsInClip(CbFontClip *clip, s32 x0, s32 y0, s32 x1, s32 y1) {
    return clip->x0 < x1 && x0 < clip->x1 && clip->y0 < y1 && y0 < clip->y1;
}

/* Opens a direct packet and writes the text state: the default environment, then CLAMP_1 = 0, TEST_1 = 0,
   TEXA = (0x80, AEM, 0x80), ZBUF_1 (masked) and PRIM = alpha-blended textured sprite with UV coordinates. */
void Font_BeginPacket(CbFontOut *out) {
    out->p = Dma_BeginDirect();
    Gfx_PutDefaultEnv(out);
    out->p[0] = 0x1000000000008005;
    out->p[1] = 0xE;
    out->p += 2;
    out->p[0] = 0;
    out->p[1] = 8;
    out->p += 2;
    out->p[0] = 0;
    out->p[1] = 0x47;
    out->p += 2;
    out->p[0] = 0x0000008000008080;
    out->p[1] = 0x3B;
    out->p += 2;
    out->p[0] = 0x00000001310000E0;
    out->p[1] = 0x4E;
    out->p += 2;
    out->p[0] = 0x156;
    out->p[1] = 0;
    out->p += 2;
}

/* Closes the direct packet. */
void Font_EndPacket(CbFontOut *out) {
    Dma_EndDirect(out->p);
}

/* Appends one sprite (TEX0, RGBAQ with q = 1.0, two UV / XYZ2 pairs, z = 0xFFFFFF0); dropped when it leaves the
   GS coordinate range. Texel coordinates get the half-texel offset (u * 16 + 8). */
void Font_PutSprite(CbFontOut *out, s32 x0, s32 y0, s32 x1, s32 y1, s32 u0, s32 v0, s32 u1, s32 v1, u32 rgba,
                    u64 tex0) {
    x0 += 0x700;
    x1 += 0x700;
    y0 += 0x720;
    y1 += 0x720;
    if (Font_IsInGsRange(x0, y0, x1, y1)) {
        out->p[0] = tex0;
        out->p[1] = (u64)rgba | ((u64)0x3F800000 << 32);
        out->p += 2;
        out->p[0] = (u64)(u0 * 16 + 8) | ((u64)(v0 * 16 + 8) << 16);
        out->p[1] = ((u64)x0 << 4) | ((u64)y0 << 20) | ((u64)0xFFFFFF0 << 32);
        out->p += 2;
        out->p[0] = (u64)(u1 * 16 + 8) | ((u64)(v1 * 16 + 8) << 16);
        out->p[1] = ((u64)x1 << 4) | ((u64)y1 << 20) | ((u64)0xFFFFFF0 << 32);
        out->p += 2;
    }
}

/* ------------------------------------------------------------------------------------------------------------
 * Second part, 0x239EA0..0x23C310 (the file as its agent wrote it), with its own header and view types. The four
 * packet helpers above that it declared with other parameter types (u64 ** for CbFontOut *, FontCmd * for
 * CbFontClip *) are reached through cast macros (the generated code is the same).
 * ------------------------------------------------------------------------------------------------------------ */
#include "sys/font.h"

/*
 * The text printer: 0x239EA0..0x23C310. Text is queued as commands (Font_Print* and friends) with a
 * snapshot of the current style; Font_Flush turns the queue into one GS packet. Drawing only: nothing
 * here is read by the simulation.
 *
 * The original source file starts at 0x239BB0: Font_Flush only matches with Font_BeginPacket and
 * Font_EndPacket defined above it in the same file, which is why the five packet helpers were merged in.
 */

typedef struct FontCommonRes {
    /* 0x00 */ u32 *boot; /* CommonRes.boot: header of byte offsets, +4 / +8 the two fonts, +0x20 the icons */
} FontCommonRes;

extern FontCommonRes *gCommonRes;

extern void *Heap_Alloc(s32 size, u32 align, s32 fromTail, s32 heap);
extern void Heap_Free(void *ptr);
extern void *memset(void *dst, s32 value, u32 size);
extern void Tex_Upload(FontTex *tex, u32 tbp, s32 cbp); /* uploads a texture and its CLUT */
extern void TexFile_UploadAll(void *res, void *tex, void *tex2);

/* 0x239BB0..0x239EA0, defined above with the first part's types. */
#define Font_IsInClip ((s32 (*)(FontCmd *cmd, s32 x0, s32 y0, s32 x1, s32 y1))Font_IsInClip) /* 1 when the box touches the command's clip box */
#define Font_BeginPacket ((void (*)(u64 **pkt))Font_BeginPacket)                                 /* Dma_BeginDirect + the 2D state */
#define Font_EndPacket ((void (*)(u64 **pkt))Font_EndPacket)                                   /* Dma_EndDirect */
#define Font_PutSprite ((void (*)(u64 **pkt, s32 x0, s32 y0, s32 x1, s32 y1, s32 u0, s32 v0, s32 u1, s32 v1, u32 color, u64 tex0))Font_PutSprite)                         /* one textured sprite */

extern FontCursor gFontCursor; /* 0x31C1C0 */
extern FontStyle gFontStyle;   /* 0x31C1D0 */
extern s32 gFontReady;         /* 1 between Font_InitEx and Font_Term */
extern s32 gFontEnabled;
extern s32 gFontGlyphGap;      /* added after every glyph; always 0 */
extern s32 gFontLineHeight;    /* line height of the font set last */
extern FontSlot *gFontSlots;
extern FontCursor *gFontCursorStack;
extern FontStyle *gFontStyleStack;
extern FontCmd *gFontCmds;
extern s32 gFontCursorDepth;
extern s32 gFontStyleDepth;
extern s32 gFontCmdCount;
extern s32 gFontCmdMax;

void Font_InitEx(s32 unused, s32 maxCmds);
void Font_Flush(s32 first);
s32 Font_MeasureWidthEx(void *str, s32 kind, f32 scale);
s32 Font_CountLinesEx(void *str, s32 kind);
s32 Font_GetGlyphHeight(void);
void Font_AddCmd(s32 arg, s32 digits, s32 kind);
FontCmd *Font_AddCmdAt(s32 x, s32 y, s32 arg, s32 digits, s32 kind);
void Font_Home(void);
void Font_DrawCmd(u64 **pkt, FontCmd *cmd);
s32 Font_DrawChar(u64 **pkt, FontCmd *cmd, s32 *x, s32 *y, u16 ch, void *next);
s32 Font_AlignLine(FontCmd *cmd, s32 x, void *str);
s32 Font_MeasureLine(FontCmd *cmd, void *str);
s32 Font_MeasureChar(FontCmd *cmd, s32 *width, u16 ch);
void Font_AdvanceCursor(FontCmd *cmd);
s32 Font_AdvanceChar(FontCmd *cmd, s32 *x, s32 *y, u16 ch);
s32 Font_CharSize(s32 kind);
u16 Font_ReadChar(s32 kind, void *p);
s32 Font_IsNewline(u16 ch);
s32 Font_CountDigits(s32 value);
s32 Font_Pow10(s32 digits);
FontGlyph *Font_FindGlyph(u16 ch, FontSlot **slotOut);
FontGlyph *Font_SearchGlyph(FontSlot *slot, u16 ch);
FontCmd *Font_AllocCmd(void);
void Font_SetCmdCount(s32 count);
FontCmd *Font_NewCmd(s32 arg, s32 digits, s32 x, s32 y, s32 homeX, s32 kind);
void Font_SetFont(s32 slot, FontData *data);
s32 Font_DrawGlyph(u64 **pkt, FontCmd *cmd, s32 x, s32 y, u16 ch);

/* Writes the command's clip box as a GS SCISSOR register. */
void Font_PutScissor(u64 **pkt, FontCmd *cmd) {
    s32 x0 = cmd->clipX0;
    s32 y0 = cmd->clipY0;
    s32 x1 = cmd->clipX1 - 1;
    s32 y1 = cmd->clipY1 - 1;

    (*pkt)[0] = 0x1000000000008001;
    (*pkt)[1] = 0xE;
    *pkt += 2;
    (*pkt)[0] = (u64)x0 | ((u64)x1 << 16) | ((u64)y0 << 32) | ((u64)y1 << 48);
    (*pkt)[1] = 0x40;
    *pkt += 2;
}

/* Opens a REGLIST tag of sprites (TEX0_1, RGBAQ, UV, XYZ2, UV, XYZ2 per sprite) and remembers where it is. */
void Font_OpenSpriteTag(u64 **pkt, u64 **tag) {
    *tag = *pkt;
    (*pkt)[0] = 0x6400000000008000;
    (*pkt)[1] = 0x535316;
    *pkt += 2;
}

/* Patches the tag opened by Font_OpenSpriteTag with the number of sprites written since. */
void Font_CloseSpriteTag(u64 **pkt, u64 **tag) {
    *(u16 *)*tag = ((*pkt - *tag) - 2) / 6;
}

/* GS TEX0 value of a font's texture with CLUT number clut (0..3). */
u64 Font_MakeTex0(FontSlot *slot, s32 clut) {
    u64 tex0 = slot->data->tex->tex0;

    tex0 |= (u64)(slot->cbp + clut) << 37;
    tex0 |= slot->tbp;
    return tex0 | 0x400000000;
}

/* Uploads a font's texture to its VRAM blocks. */
void Font_UploadTexture(FontSlot *slot) {
    Tex_Upload(slot->data->tex, slot->tbp, slot->cbp);
}

/* Boot-time init with room for 120 commands. */
void Font_Init(s32 unused) {
    Font_InitEx(unused, 0x78);
}

/* Resets cursor and style, allocates the slots, stacks and command queue, binds the two fonts of the boot file. */
void Font_InitEx(s32 unused, s32 maxCmds) {
    u32 *boot;

    gFontReady = 1;
    gFontEnabled = 1;
    gFontCursor.homeX = 16;
    gFontCursor.homeY = 16;
    gFontCursor.x = 16;
    gFontCursor.y = 16;
    gFontStyle.scaleX = 1.0f;
    gFontStyle.scaleY = 1.0f;
    gFontStyle.scale = 1.0f;
    gFontStyle.color = 0x80FFFFFF;
    gFontStyle.color2 = 0x80808080;
    gFontStyle.spacingX = 0;
    gFontStyle.spacingY = 0;
    gFontStyle.align = 0;
    gFontStyle.flags = 0;
    gFontStyle.shadowMode = 0;
    gFontStyle.shadowColor = 0x80808080;
    gFontStyle.shadowDx = 1;
    gFontStyle.shadowDy = 2;
    gFontStyle.clipX0 = 0;
    gFontStyle.clipY0 = 0;
    gFontStyle.clipX1 = 0x1FF;
    gFontStyle.clipY1 = 0x1BF;
    gFontStyle.userFlags = 0;
    gFontStyle.begin = NULL;
    gFontStyle.tag = NULL;
    gFontStyle.tagArg = NULL;
    gFontSlots = Heap_Alloc(FONT_SLOT_COUNT * sizeof(FontSlot), 0x20, 0, 2);
    gFontCursorStack = Heap_Alloc(FONT_STACK_DEPTH * sizeof(FontCursor), 0x20, 0, 2);
    gFontStyleStack = Heap_Alloc(FONT_STACK_DEPTH * sizeof(FontStyle), 0x20, 0, 2);
    gFontCmds = Heap_Alloc(maxCmds * sizeof(FontCmd), 0x20, 0, 2);
    gFontCursorDepth = 0;
    gFontCmdMax = maxCmds;
    gFontStyleDepth = 0;
    gFontCmdCount = 0;
    memset(gFontSlots, 0, FONT_SLOT_COUNT * sizeof(FontSlot));
    boot = gCommonRes->boot;
    Font_SetFont(0, (FontData *)(boot + (boot[1] >> 2)));
    Font_SetFont(1, (FontData *)(boot + (boot[2] >> 2)));
}

/* Frees everything Font_InitEx allocated. */
void Font_Term(void) {
    if (gFontSlots != NULL) {
        Heap_Free(gFontSlots);
        gFontSlots = NULL;
    }
    if (gFontCursorStack != NULL) {
        Heap_Free(gFontCursorStack);
        gFontCursorStack = NULL;
    }
    if (gFontStyleStack != NULL) {
        Heap_Free(gFontStyleStack);
        gFontStyleStack = NULL;
    }
    if (gFontCmds != NULL) {
        Heap_Free(gFontCmds);
        gFontCmds = NULL;
    }
    gFontReady = 0;
    gFontEnabled = 0;
}

/*
 * Draws one character into the packet at (*x, *y) unless it is 0 or a line break (used by the "<UB0>" tag).
 * The original returns nothing on the path that does not draw.
 */
s32 Font_DrawCharNow(u64 **pkt, FontCmd *cmd, s32 *x, s32 *y, u16 ch) {
    if (ch != 0 && !Font_IsNewline(ch)) {
        return Font_DrawChar(pkt, cmd, x, y, ch, NULL);
    }
}

/* Draws and empties the whole queue. */
void Font_FlushAll(void) {
    Font_Flush(0);
}

/* Matches only with Font_BeginPacket and Font_EndPacket DEFINED above it in the same file (otherwise the back
 * branch of the last loop comes out `bnezl` instead of `bnez`): evidence that the object starts at 0x239BB0. */
/* Draws the queued commands from index first on, then truncates the queue to first and homes the cursor. */
void Font_Flush(s32 first) {
    u64 *pkt;
    u64 *tag;
    s32 vram = 0x3838;
    FontSlot *slot;
    FontCmd *cmd;
    s32 i;

    if (gFontReady && gFontEnabled && first < gFontCmdCount) {
        slot = gFontSlots;
        for (i = 0; i < FONT_SLOT_COUNT; i++, slot++) {
            if (slot->data != NULL) {
                Font_UploadTexture(slot);
            }
        }
        cmd = &gFontCmds[first];
        for (i = first; i < gFontCmdCount; i++, cmd++) {
            if (cmd->begin != NULL) {
                cmd->vramBase = vram;
                vram += cmd->begin(vram);
            }
        }
        Font_BeginPacket(&pkt);
        cmd = &gFontCmds[first];
        for (i = first; i < gFontCmdCount; i++, cmd++) {
            Font_PutScissor(&pkt, cmd);
            Font_OpenSpriteTag(&pkt, &tag);
            Font_DrawCmd(&pkt, cmd);
            Font_CloseSpriteTag(&pkt, &tag);
        }
        Font_EndPacket(&pkt);
        Font_Home();
        Font_SetCmdCount(first);
    }
}

/* Number of queued commands (callers save it and flush from it). */
s32 Font_GetCmdCount(void) {
    return gFontCmdCount;
}

/* Turns drawing on or off; ignored before init. */
void Font_SetEnabled(s32 enabled) {
    if (gFontReady) {
        gFontEnabled = enabled;
    }
}

/* 1 when flushing draws. */
s32 Font_IsEnabled(void) {
    return gFontEnabled;
}

/* Line height of the font bound last. */
s32 Font_GetLineHeight(void) {
    return gFontLineHeight;
}

/* The same value again (glyph height). */
s32 Font_GetGlyphHeight(void) {
    return gFontLineHeight;
}

/* Width in pixels of the widest line of a string at a scale, with the current style. */
s32 Font_MeasureWidthEx(void *str, s32 kind, f32 scale) {
    FontStyle *style = &gFontStyle;
    s32 max = 0;
    s32 width = 0;
    s32 gap = gFontGlyphGap + style->spacingX;
    u16 ch;
    FontGlyph *glyph;

    while (1) {
        ch = Font_ReadChar(kind, str);
        str = (u8 *)str + Font_CharSize(kind);
        if (ch == 0) {
            break;
        }
        if (Font_IsNewline(ch)) {
            if (style->flags & 1) {
                break;
            }
            width = 0;
        } else {
            glyph = Font_FindGlyph(ch, NULL);
            if (glyph != NULL) {
                width += (s32)((f32)((glyph->u >> 10) & 0x1F) * style->scaleX * scale * style->scale) + gap;
                if (max < width) {
                    max = width;
                }
            }
        }
    }
    return max - gap;
}

/* Width of a 16-bit string at scale 1. */
s32 Font_GetWidth(u16 *str) {
    return Font_MeasureWidthEx(str, FONT_CMD_STR16, 1.0f);
}

/* Width of an 8-bit string at scale 1. */
s32 Font_GetWidthAscii(char *str) {
    return Font_MeasureWidthEx(str, FONT_CMD_STR8, 1.0f);
}

/* Largest scale, from 1 down in steps of 0.01, at which a 16-bit string is narrower than width. */
f32 Font_FitScale(u16 *str, s32 width) {
    f32 step = 0.01f;
    f32 scale = 1.0f;

    while (step < scale) {
        if (Font_MeasureWidthEx(str, FONT_CMD_STR16, scale) < width) {
            break;
        }
        scale -= step;
    }
    return scale;
}

/* Number of lines of a string. */
s32 Font_CountLinesEx(void *str, s32 kind) {
    s32 lines = 1;
    u16 ch;

    for (;;) {
        ch = Font_ReadChar(kind, str);
        str = (u8 *)str + Font_CharSize(kind);
        if (ch == 0) {
            break;
        }
        if (Font_IsNewline(ch)) {
            lines++;
        }
    }
    return lines;
}

/* Height in pixels of a 16-bit string with the current scale and line spacing. */
s32 Font_GetHeight(u16 *str) {
    FontStyle *style = &gFontStyle;
    s32 lines = Font_CountLinesEx(str, FONT_CMD_STR16);

    return lines * ((s32)((f32)Font_GetGlyphHeight() * style->scaleY * style->scale) + style->spacingY) -
           style->spacingY;
}

/* Unscaled height of an 8-bit string. */
s32 Font_GetHeightAscii(char *str) {
    s32 lines = Font_CountLinesEx(str, FONT_CMD_STR8);

    return lines * Font_GetGlyphHeight();
}

/* Number of lines of a 16-bit string. */
s32 Font_CountLines(u16 *str) {
    return Font_CountLinesEx(str, FONT_CMD_STR16);
}

/* Number of lines of an 8-bit string. */
s32 Font_CountLinesAscii(char *str) {
    return Font_CountLinesEx(str, FONT_CMD_STR8);
}

/* Queues a command at the cursor and moves the cursor past what it will print. */
void Font_AddCmd(s32 arg, s32 digits, s32 kind) {
    Font_AdvanceCursor(Font_NewCmd(arg, digits, gFontCursor.x, gFontCursor.y, gFontCursor.homeX, kind));
}

/* Queues a command at a position; the cursor is not touched. */
FontCmd *Font_AddCmdAt(s32 x, s32 y, s32 arg, s32 digits, s32 kind) {
    return Font_NewCmd(arg, digits, x, y, x, kind);
}

/* Prints a 16-bit string at the cursor. */
void Font_Print(u16 *str) {
    Font_AddCmd((s32)str, 0, FONT_CMD_STR16);
}

/* Prints a 16-bit string at a position. */
void Font_PrintAt(s32 x, s32 y, u16 *str) {
    Font_AddCmdAt(x, y, (s32)str, 0, FONT_CMD_STR16);
}

/* Prints one 16-bit character at the cursor. */
void Font_PutChar(u16 ch) {
    Font_AddCmd(ch, 0, FONT_CMD_CHAR16);
}

/* Prints one 16-bit character at a position. */
void Font_PutCharAt(s32 x, s32 y, u16 ch) {
    Font_AddCmdAt(x, y, ch, 0, FONT_CMD_CHAR16);
}

/* Prints an 8-bit string at the cursor. */
void Font_PrintAscii(char *str) {
    Font_AddCmd((s32)str, 0, FONT_CMD_STR8);
}

/* Prints an 8-bit string at a position. */
void Font_PrintAsciiAt(s32 x, s32 y, char *str) {
    Font_AddCmdAt(x, y, (s32)str, 0, FONT_CMD_STR8);
}

/* Prints one 8-bit character at the cursor. */
void Font_PutAsciiChar(s8 ch) {
    Font_AddCmd(ch, 0, FONT_CMD_CHAR8);
}

/* Prints one 8-bit character at a position. */
void Font_PutAsciiCharAt(s32 x, s32 y, s8 ch) {
    Font_AddCmdAt(x, y, ch, 0, FONT_CMD_CHAR8);
}

/* Prints a decimal number at the cursor; digits 0 means as many as the value needs. */
void Font_PrintNum(s32 value, s32 digits) {
    if (digits == 0) {
        digits = Font_CountDigits(value);
    }
    Font_AddCmd(value, digits, FONT_CMD_NUMBER);
}

/* Prints a decimal number at a position. */
void Font_PrintNumAt(s32 x, s32 y, s32 value, s32 digits) {
    if (digits == 0) {
        digits = Font_CountDigits(value);
    }
    Font_AddCmdAt(x, y, value, digits, FONT_CMD_NUMBER);
}

/* Replaces home and cursor. */
void Font_SetCursorState(FontCursor *cursor) {
    gFontCursor = *cursor;
}

/* Saves home and cursor on the five-deep stack (no bound check). */
void Font_PushCursor(void) {
    gFontCursorStack[gFontCursorDepth] = gFontCursor;
    gFontCursorDepth++;
}

/* Restores home and cursor from the stack. */
void Font_PopCursor(void) {
    gFontCursorDepth--;
    gFontCursor = gFontCursorStack[gFontCursorDepth];
}

/* Sets the home position and moves the cursor there. */
void Font_SetHome(s32 x, s32 y) {
    gFontCursor.homeX = x;
    gFontCursor.homeY = y;
    Font_Home();
}

/* Reads the home position. */
void Font_GetHome(s32 *x, s32 *y) {
    *x = gFontCursor.homeX;
    *y = gFontCursor.homeY;
}

/* Moves the cursor. */
void Font_SetCursor(s32 x, s32 y) {
    gFontCursor.x = x;
    gFontCursor.y = y;
}

/* Reads the cursor. */
void Font_GetCursor(s32 *x, s32 *y) {
    *x = gFontCursor.x;
    *y = gFontCursor.y;
}

/* Moves the cursor to the home position. */
void Font_Home(void) {
    gFontCursor.x = gFontCursor.homeX;
    gFontCursor.y = gFontCursor.homeY;
}

/* Moves the cursor to the start of the next line. */
void Font_NewLine(void) {
    gFontCursor.x = gFontCursor.homeX;
    gFontCursor.y += (s32)((f32)(gFontLineHeight + gFontStyle.spacingY) * (gFontStyle.scaleY * gFontStyle.scale));
}

/* Replaces the whole style. */
void Font_SetStyle(FontStyle *style) {
    gFontStyle = *style;
}

/* The current style. */
FontStyle *Font_GetStyle(void) {
    return &gFontStyle;
}

/* Saves the style on the five-deep stack (no bound check). */
void Font_PushStyle(void) {
    gFontStyleStack[gFontStyleDepth] = gFontStyle;
    gFontStyleDepth++;
}

/* Restores the style from the stack. */
void Font_PopStyle(void) {
    gFontStyleDepth--;
    gFontStyle = gFontStyleStack[gFontStyleDepth];
}

/* Text colour as one RGBA word. */
void Font_SetColor(u32 color) {
    gFontStyle.color = color;
}

u32 Font_GetColor(void) {
    return gFontStyle.color;
}

/* Common scale factor. */
void Font_SetScale(f32 scale) {
    gFontStyle.scale = scale;
}

f32 Font_GetScale(void) {
    return gFontStyle.scale;
}

/* Horizontal and vertical scale. */
void Font_SetScaleXY(f32 x, f32 y) {
    gFontStyle.scaleX = x;
    gFontStyle.scaleY = y;
}

void Font_GetScaleXY(f32 *x, f32 *y) {
    *x = gFontStyle.scaleX;
    *y = gFontStyle.scaleY;
}

/* Extra pixels between glyphs and between lines. */
void Font_SetSpacing(s32 x, s32 y) {
    gFontStyle.spacingX = x;
    gFontStyle.spacingY = y;
}

void Font_GetSpacing(s32 *x, s32 *y) {
    *x = gFontStyle.spacingX;
    *y = gFontStyle.spacingY;
}

/* Clip box in screen pixels (the scissor of the next commands). */
void Font_SetClip(s32 x0, s32 y0, s32 x1, s32 y1) {
    gFontStyle.clipX0 = x0;
    gFontStyle.clipY0 = y0;
    gFontStyle.clipX1 = x1;
    gFontStyle.clipY1 = y1;
}

void Font_GetClip(s32 *x0, s32 *y0, s32 *x1, s32 *y1) {
    *x0 = gFontStyle.clipX0;
    *y0 = gFontStyle.clipY0;
    *x1 = gFontStyle.clipX1;
    *y1 = gFontStyle.clipY1;
}

/* Line alignment: 0 left, 1 centred, 2 right. */
void Font_SetAlign(s32 align) {
    gFontStyle.align = align;
}

s32 Font_GetAlign(void) {
    return gFontStyle.align;
}

/* Style flags (bit 0: print only the first line). */
void Font_SetFlags(s32 flags) {
    gFontStyle.flags = flags;
}

s32 Font_GetFlags(void) {
    return gFontStyle.flags;
}

/* Shadow mode: 0 none, 1 drop shadow, 2 eight-way outline, 3 four-way outline. */
void Font_SetShadowMode(s32 mode) {
    gFontStyle.shadowMode = mode;
}

s32 Font_GetShadowMode(void) {
    return gFontStyle.shadowMode;
}

/* Shadow colour as one RGBA word. */
void Font_SetShadowColor(u32 color) {
    gFontStyle.shadowColor = color;
}

u32 Font_GetShadowColor(void) {
    return gFontStyle.shadowColor;
}

/* Shadow offset in pixels. */
void Font_SetShadowOffset(s32 dx, s32 dy) {
    gFontStyle.shadowDx = dx;
    gFontStyle.shadowDy = dy;
}

void Font_GetShadowOffset(s32 *dx, s32 *dy) {
    *dx = gFontStyle.shadowDx;
    *dy = gFontStyle.shadowDy;
}

/* Text colour from components. */
void Font_SetColorRGBA(u8 r, u8 g, u8 b, s32 a) {
    gFontStyle.color = (r | (a << 24)) | ((b << 16) | (g << 8));
}

/* Second colour from components (the colour of inline icons). */
void Font_SetColor2RGBA(u8 r, u8 g, u8 b, s32 a) {
    gFontStyle.color2 = (r | (a << 24)) | ((b << 16) | (g << 8));
}

/* Shadow colour from components. */
void Font_SetShadowColorRGBA(u8 r, u8 g, u8 b, s32 a) {
    gFontStyle.shadowColor = (r | (a << 24)) | ((b << 16) | (g << 8));
}

/* Callback run once per command at flush time, before the packet is built. */
void Font_SetBeginCallback(FontBeginFn fn) {
    gFontStyle.begin = fn;
}

/* Callback run in front of every character of a 16-bit string, with its argument. */
void Font_SetTagCallback(FontTagFn fn, void *arg) {
    gFontStyle.tag = fn;
    gFontStyle.tagArg = arg;
}

/* userFlags = (userFlags & mask) | set. */
void Font_ChangeUserFlags(s32 mask, s32 set) {
    gFontStyle.userFlags = (gFontStyle.userFlags & mask) | set;
}

s32 Font_GetUserFlags(void) {
    return gFontStyle.userFlags;
}

/* Draws one queued command into the packet. */
void Font_DrawCmd(u64 **pkt, FontCmd *cmd) {
    FontCmd saved;
    s32 x;
    s32 y;
    void *str;
    s32 kind;
    s32 vram;
    u16 ch;
    s32 div;
    s32 i;

    x = cmd->x;
    y = cmd->y;
    kind = cmd->kind;
    vram = cmd->vramBase;
    if (pkt != NULL) {
        saved = *cmd;
        switch (kind) {
        case FONT_CMD_STR16:
        case FONT_CMD_STR8:
            str = (void *)cmd->arg;
            x = Font_AlignLine(cmd, x, str);
            do {
                if (kind == FONT_CMD_STR16 && cmd->tag != NULL) {
                    cmd->tag(pkt, cmd, &saved, &x, &y, (u16 **)&str, vram, 1);
                }
                ch = Font_ReadChar(kind, str);
                str = (u8 *)str + Font_CharSize(kind);
            } while (!Font_DrawChar(pkt, cmd, &x, &y, ch, str));
            break;
        case FONT_CMD_CHAR16:
        case FONT_CMD_CHAR8:
            ch = cmd->arg;
            x = Font_AlignLine(cmd, x, NULL);
            Font_DrawChar(pkt, cmd, &x, &y, ch, NULL);
            break;
        case FONT_CMD_NUMBER:
            x = Font_AlignLine(cmd, x, NULL);
            div = Font_Pow10(cmd->digits);
            for (i = 0; i < cmd->digits; i++) {
                Font_DrawChar(pkt, cmd, &x, &y, (cmd->arg / div) % 10 + '0', NULL);
                div /= 10;
            }
            break;
        }
    }
}

/* Draws one character and advances (*x, *y); returns 1 at the end of the text. */
s32 Font_DrawChar(u64 **pkt, FontCmd *cmd, s32 *x, s32 *y, u16 ch, void *next) {
    s32 done = 0;

    if (ch == 0) {
        done = 1;
    } else if (Font_IsNewline(ch)) {
        *x = Font_AlignLine(cmd, cmd->homeX, next);
        *y += (s32)((f32)(gFontLineHeight + cmd->spacingY) * cmd->scaleY);
        if (cmd->flags & 1) {
            done = 1;
        }
    } else {
        *x += Font_DrawGlyph(pkt, cmd, *x, *y, ch);
    }
    return done;
}

/* Start x of a line for the command's alignment. */
s32 Font_AlignLine(FontCmd *cmd, s32 x, void *str) {
    switch (cmd->align) {
    case 1:
        x -= Font_MeasureLine(cmd, str) / 2;
        break;
    case 0:
        break;
    case 2:
        x -= Font_MeasureLine(cmd, str);
        break;
    }
    return x;
}

/* Width in pixels of the first line of what a command prints from str on. */
s32 Font_MeasureLine(FontCmd *cmd, void *str) {
    s32 width;
    s32 kind;
    u16 ch;
    s32 div;
    s32 i;

    kind = cmd->kind;
    width = 0;
    switch (kind) {
    case FONT_CMD_STR16:
    case FONT_CMD_STR8:
        do {
            ch = Font_ReadChar(kind, str);
            str = (u8 *)str + Font_CharSize(kind);
        } while (!Font_MeasureChar(cmd, &width, ch));
        break;
    case FONT_CMD_CHAR16:
    case FONT_CMD_CHAR8:
        Font_MeasureChar(cmd, &width, cmd->arg);
        break;
    case FONT_CMD_NUMBER:
        div = Font_Pow10(cmd->digits);
        for (i = 0; i < cmd->digits; i++) {
            Font_MeasureChar(cmd, &width, (cmd->arg / div) % 10 + '0');
            div /= 10;
        }
        break;
    }
    return width;
}

/* Adds one character's advance to *width; returns 1 at a line break or the end. */
s32 Font_MeasureChar(FontCmd *cmd, s32 *width, u16 ch) {
    s32 done = 0;
    FontGlyph *glyph;

    if (ch == 0) {
        done = 1;
    } else if (Font_IsNewline(ch)) {
        done = 1;
    } else {
        glyph = Font_FindGlyph(ch, NULL);
        if (glyph != NULL) {
            *width += (s32)((f32)((glyph->u >> 10) & 0x1F) * cmd->scaleX) + (cmd->spacingX + gFontGlyphGap);
        }
    }
    return done;
}

/* Moves the cursor past what a freshly queued command will print. */
void Font_AdvanceCursor(FontCmd *cmd) {
    s32 x;
    s32 y;
    s32 kind;
    void *str;
    u16 ch;
    s32 div;
    s32 i;

    kind = cmd->kind;
    x = gFontCursor.x;
    y = gFontCursor.y;
    switch (kind) {
    case FONT_CMD_STR16:
    case FONT_CMD_STR8:
        str = (void *)cmd->arg;
        do {
            ch = Font_ReadChar(kind, str);
            str = (u8 *)str + Font_CharSize(kind);
        } while (!Font_AdvanceChar(cmd, &x, &y, ch));
        break;
    case FONT_CMD_CHAR16:
    case FONT_CMD_CHAR8:
        Font_AdvanceChar(cmd, &x, &y, cmd->arg);
        break;
    case FONT_CMD_NUMBER:
        div = Font_Pow10(cmd->digits);
        for (i = 0; i < cmd->digits; i++) {
            Font_AdvanceChar(cmd, &x, &y, (cmd->arg / div) % 10 + '0');
            div /= 10;
        }
        break;
    }
    gFontCursor.x = x;
    gFontCursor.y = y;
}

/* Advances (*x, *y) by one character without drawing; returns 1 at the end of the text. */
s32 Font_AdvanceChar(FontCmd *cmd, s32 *x, s32 *y, u16 ch) {
    s32 done = 0;
    FontGlyph *glyph;

    if (ch == 0) {
        done = 1;
    } else if (Font_IsNewline(ch)) {
        *x = gFontCursor.homeX;
        *y += (s32)((f32)(gFontLineHeight + cmd->spacingY) * cmd->scaleY);
        if (cmd->flags & 1) {
            done = 1;
        }
    } else {
        glyph = Font_FindGlyph(ch, NULL);
        if (glyph != NULL) {
            *x += (s32)((f32)((glyph->u >> 10) & 0x1F) * cmd->scaleX) + (cmd->spacingX + gFontGlyphGap);
        }
    }
    return done;
}

/* Bytes per character of a string kind. */
s32 Font_CharSize(s32 kind) {
    s32 size;

    switch (kind) {
    case FONT_CMD_STR16:
        size = 2;
        break;
    case FONT_CMD_STR8:
        size = 1;
        break;
    default:
        size = 2;
        break;
    }
    return size;
}

/* Reads one character of a string kind (8-bit characters are sign-extended, then cut to 16 bits). */
u16 Font_ReadChar(s32 kind, void *p) {
    switch (kind) {
    case FONT_CMD_STR16:
        return *(u16 *)p;
    case FONT_CMD_STR8:
        return *(s8 *)p;
    default:
        return *(u16 *)p;
    }
}

/* 1 for line feed and carriage return. */
s32 Font_IsNewline(u16 ch) {
    s32 result = 0;

    if (ch == '\n' || ch == '\r') {
        result = 1;
    }
    return result;
}

/* Number of decimal digits of a value (1..10). */
s32 Font_CountDigits(s32 value) {
    s32 limit = 10;
    s32 n;

    for (n = 1; n < 10; n++) {
        if (value < limit) {
            break;
        }
        limit *= 10;
    }
    return n;
}

/* 10 to the power digits - 1. */
s32 Font_Pow10(s32 digits) {
    s32 result = 1;
    s32 i;

    for (i = 1; i < digits; i++) {
        result *= 10;
    }
    return result;
}

/* Looks a character up in every bound font, slot 0 first; a no-break space falls back to a space. */
FontGlyph *Font_FindGlyph(u16 ch, FontSlot **slotOut) {
    FontGlyph *glyph = NULL;
    FontSlot *slot = gFontSlots;
    s32 i;

    for (i = 0; i < FONT_SLOT_COUNT; i++, slot++) {
        if (slot->data != NULL) {
            glyph = Font_SearchGlyph(slot, ch);
            if (glyph != NULL) {
                if (slotOut != NULL) {
                    *slotOut = slot;
                }
                break;
            }
        }
    }
    if (glyph == NULL && ch == 0xA0) {
        glyph = Font_FindGlyph(' ', slotOut);
    }
    return glyph;
}

/* Binary search of a font's glyph table. */
FontGlyph *Font_SearchGlyph(FontSlot *slot, u16 ch) {
    s32 lo = 0;
    s32 hi = slot->data->glyphCount - 1;
    FontGlyph *glyphs = slot->data->glyphs;
    FontGlyph *glyph;
    s32 mid;

    while (lo <= hi) {
        mid = lo + (hi - lo) / 2;
        glyph = (FontGlyph *)(mid * sizeof(FontGlyph) + (u32)glyphs);
        if (glyph->code < ch) {
            lo = mid + 1;
        } else if (ch < glyph->code) {
            hi = mid - 1;
        } else if (glyph->code == ch) {
            return glyph;
        }
    }
    return NULL;
}

/* Takes the next free command (no bound check against gFontCmdMax). */
FontCmd *Font_AllocCmd(void) {
    return &gFontCmds[gFontCmdCount++];
}

/* Truncates the queue. */
void Font_SetCmdCount(s32 count) {
    gFontCmdCount = count;
}

/* Queues a command with a snapshot of the current style. */
FontCmd *Font_NewCmd(s32 arg, s32 digits, s32 x, s32 y, s32 homeX, s32 kind) {
    FontCmd *cmd = Font_AllocCmd();

    cmd->arg = arg;
    cmd->x = x;
    cmd->y = y;
    cmd->digits = digits;
    cmd->homeX = homeX;
    cmd->kind = kind;
    cmd->scaleX = gFontStyle.scaleX * gFontStyle.scale;
    cmd->scaleY = gFontStyle.scaleY * gFontStyle.scale;
    cmd->spacingX = gFontStyle.spacingX;
    cmd->spacingY = gFontStyle.spacingY;
    cmd->color = gFontStyle.color;
    cmd->color2 = gFontStyle.color2;
    cmd->align = gFontStyle.align;
    cmd->flags = gFontStyle.flags;
    cmd->clipX0 = gFontStyle.clipX0;
    cmd->clipY0 = gFontStyle.clipY0;
    cmd->clipX1 = gFontStyle.clipX1;
    cmd->clipY1 = gFontStyle.clipY1;
    cmd->userFlags = gFontStyle.userFlags;
    cmd->shadowMode = gFontStyle.shadowMode;
    cmd->shadowColor = gFontStyle.shadowColor;
    cmd->shadowDx = gFontStyle.shadowDx;
    cmd->shadowDy = gFontStyle.shadowDy;
    cmd->begin = gFontStyle.begin;
    cmd->tag = gFontStyle.tag;
    cmd->tagArg = gFontStyle.tagArg;
    cmd->vramBase = -1;
    return cmd;
}

/* Binds a font file to a slot (fixing its offsets up the first time) and makes its line height current. */
void Font_SetFont(s32 slot, FontData *data) {
    FontTex *tex;
    FontSlot *s;

    if ((u32)data->tex < (u32)data) {
        tex = (FontTex *)((u32)data->tex + (u32)data);
        data->tex = tex;
        data->glyphs = (FontGlyph *)((u32)data->glyphs + (u32)data);
        tex->pixelsPtr = tex->pixels + (s32)data;
        tex->clutPtr = tex->clut + (s32)data;
    }
    s = &gFontSlots[slot];
    s->cbp = slot * 0x208 + 0x2A00;
    s->tbp = slot * 0x208 + 0x2A08;
    s->data = data;
    s->unk0C = data->unk1C;
    s->lineHeight = data->lineHeight;
    gFontGlyphGap = 0;
    gFontLineHeight = data->lineHeight;
}

/* Draws one glyph with its shadow or outline; returns the advance in pixels (0 for an unknown character). */
s32 Font_DrawGlyph(u64 **pkt, FontCmd *cmd, s32 x, s32 y, u16 ch) {
    FontSlot *slot;
    u32 color;
    u32 shadowColor;
    FontGlyph *glyph;
    u64 tex0;
    s32 u0;
    s32 v0;
    s32 u1;
    s32 v1;
    s32 w;
    s32 h;
    s32 x1;
    s32 y1;
    s32 sx0;
    s32 sy0;
    s32 sx1;
    s32 sy1;
    s32 dx;
    s32 dy;
    s32 i;

    color = cmd->color;
    shadowColor = cmd->shadowColor;
    glyph = Font_FindGlyph(ch, &slot);
    if (glyph == NULL) {
        return 0;
    }
    sx0 = 0;
    sy0 = 0;
    w = (glyph->u >> 10) & 0x1F;
    h = (glyph->v >> 10) & 0x1F;
    u0 = glyph->u & 0x3FF;
    v0 = glyph->v & 0x3FF;
    x1 = x + (s32)((f32)w * cmd->scaleX);
    y1 = y + (s32)((f32)h * cmd->scaleY);
    u1 = u0 + w;
    v1 = v0 + h;
    sx1 = 0;
    sy1 = 0;
    tex0 = Font_MakeTex0(slot, ((glyph->v >> 15) << 1) + (glyph->u >> 15));
    dx = cmd->shadowDx;
    dy = cmd->shadowDy;
    switch (cmd->shadowMode) {
    case FONT_SHADOW_DROP:
        sx0 = x + dx;
        sy0 = y + dy;
        sx1 = x1 + dx;
        sy1 = y1 + dy;
        if (Font_IsInClip(cmd, sx0, sy0, sx1, sy1)) {
            Font_PutSprite(pkt, sx0, sy0, sx1, sy1, u0, v0, u1, v1, shadowColor, tex0);
        }
        break;
    case FONT_SHADOW_OUTLINE8:
        for (i = 0; i < 8; i++) {
            switch (i) {
            case 0:
                sx0 = x + dx;
                sy0 = y + dy;
                sx1 = x1 + dx;
                sy1 = y1 + dy;
                break;
            case 1:
                sx0 = x - dx;
                sy0 = y + dy;
                sx1 = x1 - dx;
                sy1 = y1 + dy;
                break;
            case 2:
                sx0 = x + dx;
                sy0 = y - dy;
                sx1 = x1 + dx;
                sy1 = y1 - dy;
                break;
            case 3:
                sx0 = x - dx;
                sy0 = y - dy;
                sx1 = x1 - dx;
                sy1 = y1 - dy;
                break;
            case 4:
                sx0 = x + dx;
                sy0 = y;
                sx1 = x1 + dx;
                sy1 = y1;
                break;
            case 5:
                sx0 = x - dx;
                sy0 = y;
                sx1 = x1 - dx;
                sy1 = y1;
                break;
            case 6:
                sx0 = x;
                sy0 = y + dy;
                sx1 = x1;
                sy1 = y1 + dy;
                break;
            case 7:
                sx0 = x;
                sy0 = y - dy;
                sx1 = x1;
                sy1 = y1 - dy;
                break;
            }
            if (Font_IsInClip(cmd, sx0, sy0, sx1, sy1)) {
                Font_PutSprite(pkt, sx0, sy0, sx1, sy1, u0, v0, u1, v1, shadowColor, tex0);
            }
        }
        break;
    case FONT_SHADOW_OUTLINE4:
        for (i = 0; i < 4; i++) {
            switch (i) {
            case 0:
                sx0 = x - dx;
                sy0 = y - dy;
                sx1 = x1 - dx;
                sy1 = y1 + dy;
                break;
            case 1:
                sx0 = x + dx;
                sy0 = y - dy;
                sx1 = x1 + dx;
                sy1 = y1 + dy;
                break;
            case 2:
                sx0 = x - dx;
                sy0 = y - dy;
                sx1 = x1 + dx;
                sy1 = y1 - dy;
                break;
            case 3:
                sx0 = x - dx;
                sy0 = y + dy;
                sx1 = x1 + dx;
                sy1 = y1 + dy;
                break;
            }
            if (Font_IsInClip(cmd, sx0, sy0, sx1, sy1)) {
                Font_PutSprite(pkt, sx0, sy0, sx1, sy1, u0, v0, u1, v1, shadowColor, tex0);
            }
        }
        break;
    }
    if (Font_IsInClip(cmd, x, y, x1, y1)) {
        Font_PutSprite(pkt, x, y, x1, y1, u0, v0, u1, v1, color, tex0);
    }
    return (s32)((f32)((glyph->u >> 10) & 0x1F) * cmd->scaleX) + gFontGlyphGap + cmd->spacingX;
}


/* ======== merged from src/battle/col_c_b.c ======== */


/*
 * Inline icon tags of the text printer: 0x23C310..0x23D1E8. A 16-bit string may contain
 * "<PAD=x>", "<PADS=a,b,c,d>", "<COL=RRGGBBAA>" / "<COL=DEF>" and "<UB0>"; the printer calls
 * FontTag_Handle in front of every character and the tag draws controller button icons or
 * changes the text colour.
 */

typedef struct FontIconCommonRes {
    /* 0x00 */ u32 *boot; /* CommonRes.boot: word 8 is the byte offset of the icon file */
} FontIconCommonRes;

#define gCommonRes (*(FontIconCommonRes* *)&gCommonRes)

extern void Res_RelocateOffsets(void *out, void *res, void *base);
#define TexFile_UploadAll ((void (*)(FontIconRes *res, s32 tbp, s32 cbp))TexFile_UploadAll) /* uploads every texture of a file */

extern FontIconRes *gFontIconRes;
extern s32 gFontIconUploaded;  /* 1 once the icon textures were uploaded for the current flush */
extern s32 gFontIconPadType;   /* row of gFontIconTables in use */
extern s32 gFontIconFrame;     /* animation clock */
extern s32 gFontIconTick;      /* counts calls at 60 Hz so the clock runs at 30 Hz */
extern FontIconEntry *gFontIconTables[FONT_ICON_PAD_TYPES];
extern FontTagDef gFontTags[4];
extern u16 D_002C6428[]; /* "DEF" */
extern FontIconDef D_002C6838[];

s32 FontIcon_IsOnScreen(s32 x0, s32 y0, s32 x1, s32 y1);
FontIconEntry *FontIcon_Find(u16 *str, s32 padType);
s32 FontTag_Match(u16 *str, u16 *name, s32 len);
void FontTag_Handle(u64 **pkt, FontCmd *cmd, FontCmd *saved, s32 *x, s32 *y, u16 **str, s32 vramBase,
                    s32 draw);

/* Stub: returns 1. No caller. */
s32 FontIcon_Stub23C310(void) {
    return 1;
}

/* Empty. No caller. */
void FontIcon_Stub23C318(void) {
}

/* Empty. No caller. */
void FontIcon_Stub23C320(void) {
}

/* Uploads the icon textures behind the fonts' VRAM blocks. */
void FontIcon_Upload(s32 vramBase) {
    TexFile_UploadAll(gFontIconRes, vramBase + 0x40, vramBase);
}

/* 1 when a box in GS coordinates lies inside the drawing area. */
s32 FontIcon_IsOnScreen(s32 x0, s32 y0, s32 x1, s32 y1) {
    s32 result = 0;

    if (x0 > 0 && x1 < 0x1000 && y0 > 0 && y1 < 0x1000) {
        result = 1;
    }
    return result;
}

/*
 * Writes one icon sprite (TEX0_1, RGBAQ, UV, XYZ2, UV, XYZ2) showing a frame of its texture.
 * Cells per row = texture width / cell size; drawn size = (s32)(cell size * scale); corners at
 * (x + 0x700, y + 0x720) and + drawn size; column = frame % cells, row = frame / cells;
 * UV = cell * size .. (cell + 1) * size; Z = 0xFFFFFFF0; Q = 1.0f.
 *
 * Matching notes: the column and row are written out at each of their four uses (the four divisions
 * leave four zero checks in the RTL, which is what pushes the function over the scheduler's 100
 * instruction limit for a region, so that nothing of the block behind the visibility test is moved
 * in front of it); the TEX0 word is the raw word with CBP merged in, then `| tbp | TCC` into a
 * second variable.
 */
void FontIcon_PutSprite(u64 **pkt, FontCmd *cmd, FontIconDef *icon, s32 frame, s32 x, s32 y, f32 scale,
                        u32 color, s32 vramBase) {
    FontTex *tex;
    s32 size;
    u64 tex0;
    u64 reg;
    s32 tbp;
    s32 cbp;
    s32 drawn;
    s32 perRow;
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;
    s32 tu0;
    s32 tv0;
    s32 tu1;
    s32 tv1;

    tex = &gFontIconRes->tex[icon->tex];
    size = icon->size;
    tex0 = tex->tex0;
    drawn = (s32)((f32)size * scale);
    perRow = (1 << ((tex0 >> 26) & 0xF)) / size;
    tbp = tex->tbp + vramBase + 0x40;
    cbp = tex->cbp + vramBase;
    tex0 |= (u64)cbp << 37;
    reg = tex0 | tbp | 0x400000000;
    x0 = x + 0x700;
    y0 = y + 0x720;
    x1 = x + drawn + 0x700;
    y1 = y + drawn + 0x720;
    if (FontIcon_IsOnScreen(x0, y0, x1, y1)) {
        tu0 = size * (frame % perRow);
        tv0 = size * (frame / perRow);
        tu1 = (frame % perRow + 1) * size;
        tv1 = (frame / perRow + 1) * size;
        (*pkt)[0] = reg;
        (*pkt)[1] = (u64)color | ((u64)0x3F800000 << 32);
        *pkt += 2;
        (*pkt)[0] = ((u64)tu0 << 4) | ((u64)tv0 << 20);
        (*pkt)[1] = ((u64)x0 << 4) | ((u64)y0 << 20) | ((u64)0xFFFFFFF0 << 32);
        *pkt += 2;
        (*pkt)[0] = ((u64)tu1 << 4) | ((u64)tv1 << 20);
        (*pkt)[1] = ((u64)x1 << 4) | ((u64)y1 << 20) | ((u64)0xFFFFFFF0 << 32);
        *pkt += 2;
    }
}

/* Ticks one icon's animation takes. */
s32 FontIcon_GetAnimLength(FontIconEntry *entry) {
    s32 total = 0;
    s32 i;

    for (i = 0; i < entry->icon->frameCount; i++) {
        total += entry->icon->frameTime[i];
    }
    return total;
}

/* Ticks the animations of all icons named in a tag argument take together. */
s32 FontIcon_GetTotalLength(u16 *str, s32 padType) {
    s32 total = 0;
    FontIconEntry *entry;

    do {
        if (*str == 0) {
            break;
        }
        entry = FontIcon_Find(str, padType);
        if (entry == NULL) {
            break;
        }
        total += FontIcon_GetAnimLength(entry);
        str += entry->name->len;
    } while (1);
    return total;
}

/* Largest cell size of the icons named in a tag argument. */
s32 FontIcon_GetMaxSize(u16 *str, s32 padType) {
    s32 max = 0;
    FontIconEntry *entry;

    while (*str != 0) {
        entry = FontIcon_Find(str, padType);
        if (entry != NULL) {
            if (max < entry->icon->size) {
                max = entry->icon->size;
            }
            str += entry->name->len;
        } else {
            break;
        }
    }
    return max;
}

/* Frame an icon shows now: the animation clock modulo length, minus start, walked through its frame times. */
s32 FontIcon_GetFrame(FontIconEntry *entry, FontIconDef *icon, s32 length, s32 start) {
    s32 frame = 0;
    s32 t;
    s32 sum;
    s32 i;
    s32 n;

    if (icon->frameCount >= 2) {
        t = gFontIconFrame % length - start;
        if (t > 0) {
            sum = 0;
            n = icon->frameCount;
            frame = n - 1;
            for (i = 0; i < n; i++) {
                if (t < sum + icon->frameTime[i]) {
                    frame = i;
                    break;
                }
                sum += icon->frameTime[i];
            }
        }
    }
    return frame;
}

/* The icon whose name starts a string, the longest name winning; NULL if none. */
FontIconEntry *FontIcon_Find(u16 *str, s32 padType) {
    FontIconEntry *entry = gFontIconTables[padType];
    FontIconEntry *best = NULL;
    FontIconName *name;

    while (entry->name != NULL) {
        name = entry->name;
        if (FontTag_Match(str, name->chars, name->len)) {
            if (best == NULL || best->name->len < name->len) {
                best = entry;
            }
        }
        entry++;
    }
    return best;
}

/* 1 when the first len characters of str equal name. */
s32 FontTag_Match(u16 *str, u16 *name, s32 len) {
    s32 result = 1;
    s32 i;

    for (i = 0; i < len; i++) {
        if (*str == 0 || *str++ != *name++) {
            result = 0;
            break;
        }
    }
    return result;
}

/* Copies characters up to a stop character (at most max), leaving *src on it; 1 when max was reached. */
s32 FontTag_CopyArg(u16 **src, u16 *dst, s32 max, u16 stop) {
    u16 *p = *src;
    s32 i;

    for (i = 0; i < max; i++) {
        if (*p == stop) {
            break;
        }
        *dst++ = *p++;
    }
    *dst = 0;
    *src = p;
    return i == max;
}

/* Copies the low bytes of up to max characters (for a stripped debug message). */
void FontTag_ToAscii(u16 *src, char *dst, s32 max) {
    s32 i;

    for (i = 0; i < max; i++) {
        if (*src == 0) {
            break;
        }
        *dst++ = *(u8 *)src;
        src++;
    }
}

/* Printer begin callback: uploads the icon textures once per flush. Uses no VRAM of its own. */
s32 FontIcon_Begin(s32 vramBase) {
    if (!gFontIconUploaded) {
        FontIcon_Upload(vramBase);
        gFontIconUploaded = 1;
    }
    return 0;
}

/* Printer tag callback: when *str is at a '<' that starts a known tag, runs it and moves *str past it. */
void FontTag_Handle(u64 **pkt, FontCmd *cmd, FontCmd *saved, s32 *x, s32 *y, u16 **str, s32 vramBase,
                    s32 draw) {
    char name[16];
    u16 *p = *str;
    FontTagDef *def;
    u32 i;
    u16 ch;

    gFontIconUploaded = 0;
    if (*p == '<') {
        def = gFontTags;
        for (i = 0; i < 4; i++, def++) {
            if (!FontTag_Match(p + 1, def->name, def->len)) {
                continue;
            }
            ch = p[def->len + 1];
            if (def->hasArg == 1) {
                if (ch != '=') {
                    FontTag_ToAscii(p, name, def->len);
                    break;
                }
            } else if (ch != '>') {
                FontTag_ToAscii(p, name, def->len);
                break;
            }
            def->handler(def, pkt, cmd, saved, x, y, str, vramBase, draw);
            ch = **str;
                if (ch != '>') {
                FontTag_ToAscii(p, name, def->len);
                break;
            }
            *str += 1;
            FontTag_Handle(pkt, cmd, saved, x, y, str, vramBase, draw);
            break;
        }
    }
}

/* "<PAD=names>": draws the named icons side by side, each on its own animation. */
void FontTag_Pad(FontTagDef *def, u64 **pkt, FontCmd *cmd, FontCmd *saved, s32 *x, s32 *y, u16 **str,
                 s32 vramBase, s32 draw) {
    u16 arg[32];
    u16 *p;
    s32 *padTypePtr;
    s32 padType;
    s32 max;
    u16 *s;
    FontIconEntry *entry;
    FontIconDef *icon;
    FontIconName *name;
    s32 frame;
    s32 dy;

    p = *str + def->len + 2;
    padTypePtr = cmd->tagArg;
    FontTag_CopyArg(&p, arg, 0x1F, '>');
    padType = *padTypePtr;
    FontIcon_GetTotalLength(arg, padType);
    max = FontIcon_GetMaxSize(arg, padType);
    for (s = arg; *s != 0; s += name->len) {
        entry = FontIcon_Find(s, padType);
        if (entry != NULL) {
            icon = entry->icon;
            name = entry->name;
            frame = FontIcon_GetFrame(entry, icon, FontIcon_GetAnimLength(entry), 0);
            switch (icon->size) {
            case 0x20:
                dy = (max - 0x20) / 2 - 6;
                break;
            case 0x40:
                dy = (max - 0x40) / 2;
                break;
            case 0x80:
                dy = (max - 0x80) / 2 - 6;
                break;
            default:
                dy = (max - icon->size) / 2 - 6;
                break;
            }
            if (draw) {
                FontIcon_PutSprite(pkt, cmd, icon, frame, *x, *y + dy, entry->scale, cmd->color2, vramBase);
            }
            *x += (s32)((f32)icon->advance * entry->scale) + cmd->spacingX;
        } else {
            break;
        }
    }
    *str = p;
}

/* "<PADS=a,b,c,d>": one list per controller type; the icons of the current type animate one after another. */
void FontTag_PadSequence(FontTagDef *def, u64 **pkt, FontCmd *cmd, FontCmd *saved, s32 *x, s32 *y,
                         u16 **str, s32 vramBase, s32 draw) {
    u16 arg[FONT_ICON_PAD_TYPES][32];
    u16 *p;
    s32 *padTypePtr;
    s32 padType;
    s32 total;
    s32 start;
    s32 max;
    u16 *s;
    FontIconEntry *entry;
    FontIconDef *icon;
    FontIconName *name;
    s32 frame;
    s32 dy;

    p = *str + def->len + 2;
    start = 0;
    padTypePtr = cmd->tagArg;
    FontTag_CopyArg(&p, arg[0], 0x1F, ',');
    p++;
    FontTag_CopyArg(&p, arg[1], 0x1F, ',');
    p++;
    FontTag_CopyArg(&p, arg[2], 0x1F, ',');
    p++;
    FontTag_CopyArg(&p, arg[3], 0x1F, '>');
    padType = *padTypePtr;
    total = FontIcon_GetTotalLength(arg[padType], padType);
    max = FontIcon_GetMaxSize(arg[padType], padType);
    for (s = arg[padType]; *s != 0; s += name->len) {
        entry = FontIcon_Find(s, padType);
        if (entry != NULL) {
            icon = entry->icon;
            name = entry->name;
            frame = FontIcon_GetFrame(entry, icon, total, start);
            start += FontIcon_GetAnimLength(entry);
            switch (icon->size) {
            case 0x20:
                dy = (max - 0x20) / 2 - 6;
                break;
            case 0x40:
                dy = (max - 0x40) / 2;
                break;
            case 0x80:
                dy = (max - 0x80) / 2 - 6;
                break;
            default:
                dy = (max - icon->size) / 2 - 6;
                break;
            }
            if (draw) {
                FontIcon_PutSprite(pkt, cmd, icon, frame, *x, *y + dy, entry->scale, cmd->color2, vramBase);
            }
            *x += (s32)((f32)icon->advance * entry->scale) + cmd->spacingX;
        } else {
            break;
        }
    }
    *str = p;
}

/* "<COL=RRGGBBAA>" sets the text colour of the rest of the command, "<COL=DEF>" restores it. */
void FontTag_Color(FontTagDef *def, u64 **pkt, FontCmd *cmd, FontCmd *saved, s32 *x, s32 *y, u16 **str,
                   s32 vramBase, s32 draw) {
    u16 *p = *str + def->len;
    u16 *s = p + 2;
    u32 color;
    u32 hex;
    s32 i;
    u16 ch;

    if (FontTag_Match(s, D_002C6428, 3)) {
        color = saved->color;
        s = p + 5;
    } else {
        hex = 0;
        for (i = 7; i >= 0; i--) {
            ch = *s++;
            if ((u16)(ch - '0') < 10) {
                hex |= (ch - '0') << (i * 4);
            } else if ((u16)(ch - 'A') < 6) {
                hex |= (ch - 'A' + 10) << (i * 4);
            } else {
                return;
            }
        }
        color = (((hex >> 16) & 0xFF) | ((hex & 0xFF) << 16)) | (((hex >> 24) << 24) | (hex & 0xFF00));
    }
    if (draw) {
        cmd->color = color;
    }
    *str = s;
}

/* "<UB0>": draws character 0xFF10 at once, in front of the text that follows. */
void FontTag_Ub0(FontTagDef *def, u64 **pkt, FontCmd *cmd, FontCmd *saved, s32 *x, s32 *y, u16 **str,
                 s32 vramBase, s32 draw) {
    u16 *p = *str + def->len + 1;

    if (draw) {
        Font_DrawCharNow(pkt, cmd, x, y, 0xFF10);
    }
    *str = p;
}

/* Boot-time init: binds the icon file and hooks the printer's callbacks. */
void FontIcon_Init(void) {
    u32 *boot = gCommonRes->boot;

    gFontIconRes = (FontIconRes *)(boot + (boot[8] >> 2));
    Res_RelocateOffsets(&gFontIconRes, gFontIconRes, gFontIconRes);
    Font_SetBeginCallback(FontIcon_Begin);
    Font_SetTagCallback(FontTag_Handle, &gFontIconPadType);
}

/* Selects the controller type whose icon table is used. */
void FontIcon_SetPadType(s32 padType) {
    gFontIconPadType = padType;
}

/* The icon file. */
FontIconRes *FontIcon_GetRes(void) {
    return gFontIconRes;
}

/* The icon definitions. */
FontIconDef *FontIcon_GetDefs(void) {
    return D_002C6838;
}

/* Restarts the icon animation clock. */
void FontIcon_ResetAnim(void) {
    gFontIconFrame = 0;
}

/* Advances the animation clock once per 30 Hz frame: every second call when vsyncs is 1. */
void FontIcon_Tick(s32 vsyncs) {
    if (vsyncs == 1) {
        if (!(gFontIconTick & 1)) {
            gFontIconFrame++;
        }
        gFontIconTick++;
    } else {
        gFontIconFrame++;
    }
}

/* Largest icon size named in a string that begins with "<PAD=". */
s32 FontIcon_GetPadTagSize(u16 *str) {
    u16 arg[32];
    u16 *p;

    p = str + 5;
    FontTag_CopyArg(&p, arg, 0x1F, '>');
    return FontIcon_GetMaxSize(arg, gFontIconPadType);
}

/* Empty; called after Font_FlushAll by the battle draw. */
void FontIcon_Stub23D1E0(void) {
}

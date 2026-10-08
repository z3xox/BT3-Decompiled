#ifndef SYS_VU1_PACKET_H
#define SYS_VU1_PACKET_H

#include "types.h"

/*
 * VU1 draw packets of the 3D renderer at 0x10AD58..0x116xxx. src/sys/vu1_packet.c = 0x123130..0x123F48.
 * See the comment at the top of vu1_packet.c.
 */

/* Source-chain DMA tag "call": the data follows the tag, then the chain at the tag's address runs until its "ret". */
#define VU1_DMA_CALL 0x50000000
#define VU1_DMA_CNT 0x10000000
#define VU1_DMA_END 0x70000000
#define VU1_ADDR_MASK 0x0FFFFFFF

/* VIF codes. */
#define VU1_VIF_NOP 0x00000000
#define VU1_VIF_OFFSET(n) (0x02000000 | (n))
#define VU1_VIF_BASE(n) (0x03000000 | (n))
#define VU1_VIF_FLUSHE 0x10000000
#define VU1_VIF_MSCALF(addr) (0x15000000 | (addr))
#define VU1_VIF_UNPACK_V4_32(qwc, addr) (0x6C000000 | ((qwc) << 16) | (addr))
/* The code Vu1Node_OrFlags looks for: three quadwords at TOPS + 0. */
#define VU1_VIF_NODE_HEADER 0x6C038000

/* One node of a model chunk list (variable size; the next one is `size` bytes further on). */
typedef struct Vu1Node {
    /* 0x00 */ s32 size;
    /* 0x04 */ s32 popCount;   /* matrices to drop from the VU0 matrix stack after this node */
    /* 0x08 */ s32 last;       /* non-zero on the last node */
    /* 0x0C */ s32 hasMesh;    /* non-zero when a VIF stream follows at +0xF0 */
    /* 0x10 */ f32 pos[4];
    /* 0x20 */ f32 rot[4];
    /* 0x30 */ f32 pivot[4];
    /* 0x40 */ f32 parentPivot[4];
    /* 0x50 */ f32 parent[16]; /* matrix on entry */
    /* 0x90 */ f32 world[16];  /* matrix after pos/rot */
    /* 0xD0 */ f32 pivotOut[4];   /* copies of pivot / parentPivot */
    /* 0xE0 */ f32 parentPivotOut[4];
    /* 0xF0 */ u32 vif[1];     /* ends with a 0x70000000 word */
} Vu1Node;

/* One key of a node's animation track, 0x20 bytes. */
typedef struct Vu1Key {
    /* 0x00 */ f32 pos[3];
    /* 0x0C */ u32 frame;
    /* 0x10 */ f32 rot[4];
} Vu1Key;

/* One node's animation track; the next node's follows the last key. */
typedef struct Vu1Track {
    /* 0x00 */ u8 unk00[0xA];
    /* 0x0A */ u16 keyCount;
    /* 0x0C */ u32 unk0C;
    /* 0x10 */ Vu1Key key[1];
} Vu1Track;

/* VU1 microprograms (in the .vutext/.vudata area right after .text). */
extern u8 gVu1Prog0[], gVu1Prog1[], gVu1Prog2a[], gVu1Prog2b[], gVu1Prog4[], gVu1Prog5[], gVu1Prog6[], gVu1Prog7[],
    gVu1Prog8[], gVu1ProgEnd[];

u32 *Vu1Pkt_CallProg7(u32 chain, f32 w);
u32 *Vu1Pkt_LoadProg7(void);
u32 *Vu1Pkt_CallProg0(u32 chain);
u32 *Vu1Pkt_LoadProg0(void);
u32 *Vu1Pkt_CallProg1(u32 obj);
u32 *Vu1Pkt_LoadProg1(void);
u32 *Vu1Pkt_CallProg2(u32 obj);
u32 *Vu1Pkt_LoadProg2(s32 alt);
void Vu1Pkt_CallProg4(u32 chain);
void Vu1Pkt_LoadProg4(void *mtx);
void Vu1Pkt_CallProg5(u32 chain, void *mtx);
void Vu1Pkt_LoadProg5(void *mtx);
void Vu1Node_OrFlags(Vu1Node *node, u32 bits0, u32 bits1);
void Vu1Node_Animate(Vu1Node *node, Vu1Track *track, void *mtx, f32 frame);
void Vu1Node_Draw(Vu1Node *node);
u32 *Vu1Pkt_CallProg8(Vu1Node *node);
u32 *Vu1Pkt_LoadProg8(void);
void Vu1Pkt_CallProg6(u32 chain);
void Vu1Pkt_LoadProg6(u8 *obj, void *mtx, f32 *vec);

#endif

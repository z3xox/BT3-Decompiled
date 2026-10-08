#ifndef BATTLE_EFT_M_H
#define BATTLE_EFT_M_H

#include "types.h"
#include "sys/math3d.h"

/*
 * Effect tasks, 0x15F728..0x1637A0 (src/battle/eft_aura.c):
 *   0x15F728..0x1609C8  speed lines: the second half of the module whose spawners are in eft_l (0x15EF18, 0x15F3D8)
 *   0x1609C8..0x1637A0  the aura's particles: the first half of the aura module, which continues in the next file at 0x1637A0
 * Both are drawing only: nothing here writes a fighter, a battle object or a hit record.
 * All structs are this file's views.
 */

/* A vector passed by value. The original vector type is 16-byte aligned (argument copies use ld / sd);
   Vec4 in sys/math3d.h is not, so this file casts between the two. */
typedef struct EftVec {
    /* 0x0 */ f32 x;
    /* 0x4 */ f32 y;
    /* 0x8 */ f32 z;
    /* 0xC */ f32 w;
} __attribute__((aligned(16))) EftVec;

/* ---- speed lines ---------------------------------------------------------------------------------------- */

/* Trail segment: a short quad that follows its fighter for two frames. */
typedef struct EftSpdTrail {
    /* 0x00 */ Vec4 pos;      /* leading end */
    /* 0x10 */ Vec4 tail;     /* trailing end */
    /* 0x20 */ Vec4 dir;      /* axis the quad is built around */
    /* 0x30 */ s32 objId;     /* owner (battle object id) */
    /* 0x34 */ f32 width;     /* half width */
    /* 0x38 */ f32 alpha;     /* 64 */
    /* 0x3C */ f32 life;      /* frames left, starts at 2 */
    /* 0x40 */ s32 flags;     /* 0 = free slot, bit 0 = alive */
    /* 0x44 */ u8 tex;        /* index into EftSpdLineWork.tex */
    /* 0x45 */ u8 blend;      /* second argument of EftGfx_DrawPolyAvgZFront */
    /* 0x46 */ u8 unk46[2];
    /* 0x48 */ struct EftSpdTrail *prev;
    /* 0x4C */ struct EftSpdTrail *next;
} EftSpdTrail; /* size 0x50 */

/* Streak segment: a line that flies along its own direction and fades out. */
typedef struct EftSpdStreak {
    /* 0x00 */ Vec4 pos;
    /* 0x10 */ Vec4 tail;
    /* 0x20 */ Vec4 dir;      /* unit direction of travel */
    /* 0x30 */ s32 objId;
    /* 0x34 */ f32 speed;     /* distance per frame */
    /* 0x38 */ f32 width;     /* full width */
    /* 0x3C */ f32 alpha;     /* starts at 160 */
    /* 0x40 */ f32 fade;      /* alpha lost per frame: 160 / (life * 30) */
    /* 0x44 */ s32 flags;     /* 0 = free slot, bit 0 = alive */
    /* 0x48 */ u8 tex;
    /* 0x49 */ u8 blend;
    /* 0x4A */ u8 unk4A[2];
    /* 0x4C */ struct EftSpdStreak *prev;
    /* 0x50 */ struct EftSpdStreak *next;
    /* 0x54 */ u8 unk54[0xC];
} EftSpdStreak; /* size 0x60 */

typedef struct EftSpdTex {
    /* 0x0 */ u64 tex;        /* passed to EftGfx_DrawPolyAvgZFront */
    /* 0x8 */ u64 unk8;
} EftSpdTex;

/* gEftSpdLine: 0x18D0 bytes from the effect pool. */
typedef struct EftSpdLineWork {
    /* 0x0000 */ EftSpdTrail trails[30];
    /* 0x0960 */ EftSpdStreak streaks[40];
    /* 0x1860 */ s32 trailCount;
    /* 0x1864 */ s32 streakCount;
    /* 0x1868 */ EftSpdTex tex[4];        /* filled by EftTexSet_Load4 from common entry 0x237 */
    /* 0x18A8 */ u8 unk18A8[8];
    /* 0x18B0 */ s32 texReady;            /* set by EftSpdLine_UpdateTexture; only the streak draw tests it */
    /* 0x18B4 */ EftSpdTrail *trailHead;
    /* 0x18B8 */ EftSpdTrail *trailTail;
    /* 0x18BC */ EftSpdStreak *streakHead;
    /* 0x18C0 */ EftSpdStreak *streakTail;
    /* 0x18C4 */ u8 unk18C4[0xC];
} EftSpdLineWork; /* size 0x18D0 */

extern EftSpdLineWork *gEftSpdLine;

s32 EftSpdLine_SpawnStreakRing(s32 objId, Vec4 *dir, s32 node, s32 count, f32 unused, f32 lifeMin, f32 lifeRange);
void EftSpdLine_Init(void);
void EftSpdLine_Term(void);
void EftSpdLine_Update(void);
void EftSpdLine_Stub(void);
void EftSpdLine_Draw(void);
EftSpdTrail *EftSpdLine_AllocTrail(void);
void EftSpdLine_AddTrail(s32 objId, EftVec pos, EftVec dir);
void EftSpdLine_UpdateTrails(void);
s32 EftSpdLine_StepTrail(EftSpdTrail *p);
void EftSpdLine_DrawTrails(void);
EftSpdStreak *EftSpdLine_AllocStreak(void);
void EftSpdLine_AddStreak(s32 objId, EftVec pos, EftVec tail, EftVec dir, f32 speed, f32 life, f32 width);
void EftSpdLine_UpdateStreaks(void);
s32 EftSpdLine_StepStreak(EftSpdStreak *p);
void EftSpdLine_DrawStreaks(void);
void EftSpdLine_BuildTrailQuad(Vec4 *out, EftSpdTrail *p);
void EftSpdLine_BuildStreakQuad(Vec4 *out, EftSpdStreak *p);
void EftSpdLine_DrawQuad(Vec4 *quad, Mtx44 *mtx, EftSpdTex *tex, f32 r, f32 g, f32 b, f32 a, u8 blend, s32 inView);
void EftSpdLine_UpdateTexture(s32 a, s32 b);

/* ---- aura particles ------------------------------------------------------------------------------------- */

#define EFT_AURA_PARTS 10   /* body parts that emit flames */
#define EFT_AURA_SPARKS 12  /* spark emitters */

/* One fighter's aura (allocated and filled by the aura task at 0x1637A0..; only the fields this file reads are named). */
typedef struct EftAura {
    /* 0x000 */ s32 flags;       /* 1 active, 2 faded out, 0x10 / 0x20 variant (set by EftAura_Start), 0x80 */
    /* 0x004 */ u32 altMask;     /* bit per body part, toggled at every flame spawn */
    /* 0x008 */ s32 fadeA;       /* 1 = unk50 falls to 0, else rises to 1 */
    /* 0x00C */ s32 fadeB;       /* the same for unk58 */
    /* 0x010 */ s32 type;        /* 9 and 10 select the long-lived variant */
    /* 0x014 */ s32 unk14;
    /* 0x018 */ s32 colorCount;  /* number of colour entries to pick from */
    /* 0x01C */ s32 unk1C;
    /* 0x020 */ s32 texCount;    /* flame flag 0x100: unk6 = rand() % texCount */
    /* 0x024 */ s32 partsStarted; /* body parts that have had their first flame, 0..10 */
    /* 0x028 */ s32 fade;        /* 0 steady, 1 fading in, 2 / 3 fading out (3 sets flag 2 at the end) */
    /* 0x02C */ s32 burst;       /* 0 none, 1 rising, 2 holding, 3 falling */
    /* 0x030 */ s32 unk30;
    /* 0x034 */ s32 flameCount;  /* live flames */
    /* 0x038 */ f32 alpha;       /* 0..1, driven by `fade` */
    /* 0x03C */ f32 burstSpeed;  /* cfg->burstSpeed * burstLevel, added to every flame's speed */
    /* 0x040 */ f32 burstTime;   /* frames spent in burst 2 */
    /* 0x044 */ f32 lifeScale;   /* 1, or 1.2 during a burst */
    /* 0x048 */ f32 level;       /* aura strength */
    /* 0x04C */ f32 burstLevel;  /* 0..1 */
    /* 0x050 */ f32 unk50;       /* 0..1, driven by fadeA; multiplies the spark alpha */
    /* 0x054 */ f32 unk54;       /* its rate */
    /* 0x058 */ f32 unk58;       /* 0..1, driven by fadeB */
    /* 0x05C */ f32 unk5C;       /* its rate */
    /* 0x060 */ f32 scale;       /* body scale */
    /* 0x064 */ u8 unk64[0xC];
    /* 0x070 */ Vec4 pos;        /* fighter root */
    /* 0x080 */ Vec4 prevPos;
    /* 0x090 */ Vec4 part[EFT_AURA_PARTS];      /* body part positions */
    /* 0x130 */ Vec4 partPrev[EFT_AURA_PARTS];  /* the frame before */
    /* 0x1D0 */ Vec4 partEnd[EFT_AURA_PARTS];   /* second node of a part (cfg->flame[i].node) */
    /* 0x270 */ u8 unk270[0xA0];
    /* 0x310 */ Vec4 unk310;     /* centre the flames are pushed away from (x, z) */
    /* 0x320 */ Vec4 axis;       /* aura axis (x, z) */
    /* 0x330 */ Vec4 sparkPos[EFT_AURA_SPARKS];
    /* 0x3F0 */ Vec4 colorStart; /* flame colour, w = alpha */
    /* 0x400 */ Vec4 colorEnd;
    /* 0x410 */ Vec4 colorAlt[4];   /* flame flag 0x800 */
    /* 0x450 */ Vec4 sparkColor[4];
    /* 0x490 */ Vec4 sparkColorAlt[4]; /* aura flag 0x20 */
} EftAura;

/* Flame particle. */
typedef struct EftAuraFlame {
    /* 0x00 */ s32 flags;   /* low bits: 1 fade-in done, 2 fading out, 4 successor spawned, 8 / 0x10 size phases 2 / 3
                               started; from EftAura_GetFlameFlags: 0x20 / 0x40 burst 1 / 2, 0x80, 0x100, 0x200 long life,
                               0x400 / 0x800 alternate kinds, 0x1000 invisible */
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 kind;     /* 0, 1 or 2: row of the parameter tables */
    /* 0x06 */ u8 tex;
    /* 0x07 */ u8 objId;
    /* 0x08 */ u8 unk8;     /* cfg->flame[part].unk0 */
    /* 0x09 */ u8 part;
    /* 0x0A */ u8 flip;     /* rand() % 2 */
    /* 0x0B */ u8 alt;      /* 1 with flag 0x800 */
    /* 0x0C */ f32 endTime; /* time left at which the last size phase starts */
    /* 0x10 */ f32 time;    /* frames left */
    /* 0x14 */ f32 life;
    /* 0x18 */ f32 scale;
    /* 0x1C */ f32 size;
    /* 0x20 */ f32 stretch;
    /* 0x24 */ f32 sizeRate;
    /* 0x28 */ f32 stretchRate;
    /* 0x2C */ f32 end;
    /* 0x30 */ f32 speed;
    /* 0x34 */ f32 follow;  /* 0..1 along the part */
    /* 0x38 */ f32 power;
    /* 0x3C */ f32 unk3C;
    /* 0x40 */ Vec4 offset;
    /* 0x50 */ Vec4 pos;
    /* 0x60 */ Vec4 color;
    /* 0x70 */ Vec4 colorRate;
    /* 0x80 */ Vec4 dir;
    /* 0x90 */ Vec4 dirStart;
    /* 0xA0 */ Vec4 dirTurn;
    /* 0xB0 */ Vec4 move;   /* accumulated displacement */
    /* 0xC0 */ struct EftAuraFlame *next;
    /* 0xC4 */ u8 unkC4[0xC];
} EftAuraFlame; /* size 0xD0 */

/* Spark particle. */
typedef struct EftAuraSpark {
    /* 0x00 */ s32 flags;   /* 1 fading out, 2, 4, 8 (aura flag 0x80), 0x10 (aura type 9 / 10) */
    /* 0x04 */ u8 objId;
    /* 0x05 */ u8 emitter;
    /* 0x06 */ u8 kind;
    /* 0x07 */ u8 unk7;     /* 1 */
    /* 0x08 */ f32 age;
    /* 0x0C */ f32 life;
    /* 0x10 */ f32 rot;     /* -pi..pi */
    /* 0x14 */ f32 rotSpeed;
    /* 0x18 */ f32 size;
    /* 0x1C */ f32 speed;
    /* 0x20 */ f32 follow;
    /* 0x24 */ u8 unk24[0xC];
    /* 0x30 */ Vec4 pos;
    /* 0x40 */ Vec4 offset;
    /* 0x50 */ Vec4 move;
    /* 0x60 */ Vec4 dir;
    /* 0x70 */ Vec4 color;
    /* 0x80 */ Vec4 colorRate;
    /* 0x90 */ Vec4 uv;
    /* 0xA0 */ struct EftAuraSpark *next;
    /* 0xA4 */ u8 unkA4[0xC];
} EftAuraSpark; /* size 0xB0 */

/* gEftAuraPool */
typedef struct EftAuraPool {
    /* 0x000 */ s32 unk0;
    /* 0x004 */ s32 flameMax;
    /* 0x008 */ s32 sparkMax;
    /* 0x00C */ EftAuraFlame *flames;
    /* 0x010 */ EftAuraFlame *flameFree;
    /* 0x014 */ EftAuraFlame *flameUsed;
    /* 0x018 */ EftAuraSpark *sparks;
    /* 0x01C */ EftAuraSpark *sparkFree;
    /* 0x020 */ EftAuraSpark *sparkUsed;
    /* 0x024 */ s32 flameNext;   /* slots handed out so far */
    /* 0x028 */ s32 sparkNext;
    /* 0x02C */ u8 unk2C[0x25C];
    /* 0x288 */ s32 sparkTex;    /* passed by address to EftSpr_DrawRot */
} EftAuraPool;

typedef struct EftAuraCfgFlame {
    /* 0x0 */ u8 unk0;
    /* 0x4 */ s32 node;  /* second model node of the part, or negative */
    /* 0x8 */ s32 unk8;
} EftAuraCfgFlame;

typedef struct EftAuraCfgSpark {
    /* 0x0 */ s32 kind;
    /* 0x4 */ s32 node;  /* model node the spark is pulled towards, or negative */
} EftAuraCfgSpark;

/* gEftAuraCfg */
typedef struct EftAuraCfg {
    /* 0x000 */ u8 unk0[0x194];
    /* 0x194 */ f32 fadeInTime;
    /* 0x198 */ f32 fadeOutTime;
    /* 0x19C */ f32 burstHold;
    /* 0x1A0 */ f32 unk1A0;
    /* 0x1A4 */ f32 burstSpeed;
    /* 0x1A8 */ f32 unk1A8;
    /* 0x1AC */ f32 unk1AC;
    /* 0x1B0 */ f32 unk1B0;
    /* 0x1B4 */ f32 unk1B4;
    /* 0x1B8 */ f32 sparkFadeIn;
    /* 0x1BC */ f32 sparkFadeOut;
    /* 0x1C0 */ f32 sparkLife;
    /* 0x1C4 */ f32 sparkLifeRange;
    /* 0x1C8 */ f32 sparkLifeLong;
    /* 0x1CC */ f32 sparkLifeLongRange;
    /* 0x1D0 */ f32 sparkStep;
    /* 0x1D4 */ f32 sparkRotSpeed;
    /* 0x1D8 */ f32 sparkAccel;
    /* 0x1DC */ f32 sparkDamp;
    /* 0x1E0 */ f32 sparkSize[4][2];  /* {size, speed}: normal, flag 8, alt, alt + flag 8 */
    /* 0x200 */ f32 sparkMul[2][2];   /* {rotSpeed, speed} factors for flags 4 and 2 */
    /* 0x210 */ u8 unk210[0x200];
    /* 0x410 */ EftAuraCfgFlame flame[EFT_AURA_SPARKS];
    /* 0x4A0 */ s32 unk4A0;
    /* 0x4A4 */ EftAuraCfgSpark spark[EFT_AURA_SPARKS];
} EftAuraCfg;

typedef struct EftAuraPrmKind {
    /* 0x00 */ f32 side[2];   /* base, range */
    /* 0x08 */ f32 away[2];
    /* 0x10 */ f32 out[2];
    /* 0x18 */ f32 turn;
    /* 0x1C */ f32 accel;
} EftAuraPrmKind;

/* gEftAuraPrm */
typedef struct EftAuraPrm {
    /* 0x000 */ f32 endRatio;
    /* 0x004 */ f32 growRatio;
    /* 0x008 */ f32 startDist;
    /* 0x00C */ f32 sparkRise;
    /* 0x010 */ f32 flameStep;
    /* 0x014 */ f32 lean;
    /* 0x018 */ f32 unk18;
    /* 0x01C */ f32 life[3];       /* base, range, long */
    /* 0x028 */ f32 unk28;
    /* 0x02C */ f32 fadeIn;
    /* 0x030 */ f32 fadeOut;
    /* 0x034 */ f32 speed[6];
    /* 0x04C */ f32 speedRand[3][2];
    /* 0x064 */ f32 stretch[4];    /* by phase */
    /* 0x074 */ f32 size[4];
    /* 0x084 */ f32 stretchMul[3];
    /* 0x090 */ f32 sizeMul[3];
    /* 0x09C */ f32 stretchKind[4];
    /* 0x0AC */ f32 sizeKind[3];
    /* 0x0B8 */ f32 unkB8;
    /* 0x0BC */ f32 stretchRand[3][3];
    /* 0x0E0 */ f32 end[3][3];
    /* 0x104 */ EftAuraPrmKind kind[3];
    /* 0x164 */ f32 alpha[2];
    /* 0x16C */ f32 alphaMul[4];
    /* 0x17C */ f32 sparkVel;
    /* 0x180 */ f32 sparkVelAlt;
    /* 0x184 */ f32 sideScale;
    /* 0x188 */ f32 drag;
    /* 0x18C */ f32 speedRef;
    /* 0x190 */ f32 outRise;
} EftAuraPrm;

extern EftAuraPool *gEftAuraPool;
extern EftAuraCfg *gEftAuraCfg;
extern EftAuraPrm *gEftAuraPrm;

void EftAura_GetSparkDir(Vec4 *out, s32 objId);
void EftAura_GetSparkDirAlt(Vec4 *out, s32 objId);
void EftAura_GetFlameSideDir(EftAura *aura, Vec4 *out, s32 objId, f32 power, f32 away, f32 side);
void EftAura_GetFlameDragDir(EftAura *aura, Vec4 *pos, Vec4 *out, s32 objId, s32 part);
void EftAura_GetFlameOutDir(EftAura *aura, Vec4 *out, s32 objId, s32 part, f32 power, f32 amount);
void EftAura_TurnFlame(EftAuraFlame *f, Vec4 *pos, f32 t);
void EftAura_AccelFlame(EftAuraFlame *f, f32 t);
void EftAura_GetPartOffset(EftAura *aura, Vec4 *out, s32 objId, s32 part, f32 radial, f32 vertical);
EftAuraSpark *EftAura_AllocSpark(void);
void EftAura_FreeSparks(s32 objId);
s32 EftAura_SpawnSpark(EftAura *aura, s32 objId, s32 emitter, s32 kind, Vec4 *offset, f32 follow);
void EftAura_SpawnSparks(EftAura *aura, s32 objId, s32 emitter, s32 once);
void EftAura_StepSparks(EftAura *aura, s32 objId);
void EftAura_UpdateSparks(EftAura *aura, s32 objId, s32 *state);
void EftAura_DrawSparks(EftAura *aura, s32 objId, f32 alpha);
EftAuraFlame *EftAura_AllocFlame(void);
s32 EftAura_GetFlameFlags(EftAura *aura, s32 objId, s32 second, s32 alt);
f32 EftAura_GetFlameLife(s32 flags, f32 burst);
f32 EftAura_GetFlameSpeed(s32 flags, s32 objId, f32 level, f32 burst);
f32 EftAura_GetFlameAlpha(EftAura *aura, s32 flags, f32 alpha);
f32 EftAura_GetFlameStretch(EftAuraFlame *f, s32 objId, s32 phase);
f32 EftAura_GetFlameSize(EftAuraFlame *f, s32 objId, s32 phase);
f32 EftAura_GetFlameEnd(EftAuraFlame *f, s32 objId);
s32 EftAura_SpawnFlame(EftAura *aura, s32 objId, s32 part, Vec4 *pos, Vec4 *offset, f32 follow, s32 alt, s32 second);
s32 EftAura_SpawnFlames(EftAura *aura, s32 objId, s32 part, s32 once);
void EftAura_Start(EftAura *aura, s32 *arg, s32 reset);
void EftAura_FreeFlames(s32 objId);
void EftAura_StepFlames(EftAura *aura, s32 objId);
void EftAura_UpdateState(EftAura *aura, s32 objId);
void EftAura_BuildFlameMtx(Mtx44 *out, Vec4 *dir, Vec4 *pos);


/* ======== formerly eft_aura_part2.h ======== */


#include "types.h"
#include "sys/math3d.h"

/*
 * Effect tasks, 0x1637A0..0x167E68 (src/battle/eft_n.c):
 *   0x1637A0..0x165758  the aura, second half of the module that starts in eft_aura.c at 0x1609C8: flame drawing,
 *                       the per-fighter aura task, the aura manager task and the entry points the fighter effect
 *                       layer calls
 *   0x165758..0x167E68  body lightning, first half of the module that continues at 0x167E68: bolts made of
 *                       chained segments between two model nodes, and flash sprites
 * Both are drawing only: nothing here writes a fighter, a battle object or a hit record.
 * 41 of the 47 functions are C; see the top of the C file for the six left in assembly.
 *
 * The structs below are this file's views. EftAuraWork, EftAuraMgr and EftAuraData are the full layouts of what
 * eft_aura.h calls EftAura, EftAuraPool and EftAuraCfg / EftAuraPrm (same field names where both name a field).
 */

/* ---- aura ----------------------------------------------------------------------------------------------- */

/* Argument block of the aura task (copied into the work at +0x64). */
typedef struct EftAuraArg {
    /* 0x0 */ s32 objId;
    /* 0x4 */ s32 variant;   /* 0 = normal (aura flag 0x10), not 0 = flag 0x20, which is never drawn */
} EftAuraArg;

/* Work of one fighter's aura task (0x510 bytes; `EftAura` in eft_aura.h). */
typedef struct EftAuraWork {
    /* 0x000 */ s32 flags;       /* 1 active, 2 finished (the task kills itself), 4 hidden, 8 first frame (no previous
                                    positions yet), 0x10 / 0x20 variant, 0x40 model changed, 0x80 types 8..10 */
    /* 0x004 */ u32 altMask;
    /* 0x008 */ s32 fadeA;
    /* 0x00C */ s32 fadeB;
    /* 0x010 */ s32 type;        /* aura type of the character (BtlCharApi_GetAuraType) */
    /* 0x014 */ s32 unk14;       /* the type again */
    /* 0x018 */ s32 colorCount;  /* 1, or 2 with flag 0x80 */
    /* 0x01C */ s32 texFirst;    /* flag 0x80: first texture of the pair, 8 / 10 / 12 */
    /* 0x020 */ s32 texCount;    /* flag 0x80: 2 */
    /* 0x024 */ s32 partsStarted;
    /* 0x028 */ s32 fade;        /* 0 steady, 1 fading in, 2 fading out, 3 fading out and then finished */
    /* 0x02C */ s32 burst;       /* 0 none, 1 rising, 2 peak, 4 ending (set here; stepped by EftAura_UpdateState) */
    /* 0x030 */ s32 frame;       /* unpaused frames since the task started */
    /* 0x034 */ s32 flameCount;
    /* 0x038 */ f32 alpha;
    /* 0x03C */ f32 burstSpeed;
    /* 0x040 */ f32 burstTime;
    /* 0x044 */ f32 lifeScale;
    /* 0x048 */ f32 level;
    /* 0x04C */ f32 burstLevel;
    /* 0x050 */ f32 unk50;
    /* 0x054 */ f32 unk54;
    /* 0x058 */ f32 unk58;
    /* 0x05C */ f32 unk5C;
    /* 0x060 */ f32 scale;       /* fighter height / 19.35, at least 0.7 */
    /* 0x064 */ EftAuraArg arg;
    /* 0x06C */ s32 unk6C;
    /* 0x070 */ Vec4 pos;        /* node 3 */
    /* 0x080 */ Vec4 prevPos;
    /* 0x090 */ Vec4 part[EFT_AURA_PARTS];
    /* 0x130 */ Vec4 partPrev[EFT_AURA_PARTS];
    /* 0x1D0 */ Vec4 partEnd[EFT_AURA_PARTS];
    /* 0x270 */ Vec4 partRef[EFT_AURA_PARTS];  /* third node of a part: flames lean away from it */
    /* 0x310 */ Vec4 unk310;     /* node 0x30 */
    /* 0x320 */ Vec4 axis;       /* node 0x11 */
    /* 0x330 */ Vec4 sparkPos[EFT_AURA_SPARKS];
    /* 0x3F0 */ Vec4 colorStart;
    /* 0x400 */ Vec4 colorEnd;
    /* 0x410 */ Vec4 colorAlt[4];
    /* 0x450 */ Vec4 sparkColor[4];
    /* 0x490 */ Vec4 sparkColorAlt[4];
    /* 0x4D0 */ u64 tex[7];      /* GS TEX0 of the flame sheet: [0] normal, [1 + n] with flag 0x80 */
    /* 0x508 */ s32 sparkState;  /* handed to EftAura_UpdateSparks */
    /* 0x50C */ s32 unk50C;
} EftAuraWork; /* size 0x510 */

/* One GS TEX0 value of a texture set (include/battle/eft_shot.h has the same layout). */
typedef struct EftNTexEntry {
    /* 0x00 */ u64 tex0;
    /* 0x08 */ u64 unk8;
} EftNTexEntry; /* 0x10 */

typedef struct EftNTexSet {
    /* 0x000 */ EftNTexEntry entry[32];
    /* 0x200 */ s32 count;
    /* 0x204 */ s32 unk204;
} EftNTexSet; /* 0x208 */

/* A texture set of the aura manager with its pack data. */
typedef struct EftAuraTexGroup {
    /* 0x000 */ s32 ready;       /* cleared by the manager every frame; group 1: TEX0 values are up to date */
    /* 0x004 */ u8 unk4[0x1C];
    /* 0x020 */ s32 *data;       /* common entry 3 (flame sheets) / 2 (spark sheets) */
    /* 0x024 */ s32 unk24;
    /* 0x028 */ EftNTexSet set;
} EftAuraTexGroup; /* size 0x230 */

/* gEftAuraPool (0x4A0 bytes; `EftAuraPool` in eft_aura.h). */
typedef struct EftAuraMgr {
    /* 0x000 */ s32 count;       /* characters */
    /* 0x004 */ s32 flameMax;    /* 50 per character */
    /* 0x008 */ s32 sparkMax;    /* 18 per character */
    /* 0x00C */ EftAuraFlame *flames;
    /* 0x010 */ EftAuraFlame *flameFree;
    /* 0x014 */ EftAuraFlame *flameUsed;
    /* 0x018 */ EftAuraSpark *sparks;
    /* 0x01C */ EftAuraSpark *sparkFree;
    /* 0x020 */ EftAuraSpark *sparkUsed;
    /* 0x024 */ s32 flameNext;
    /* 0x028 */ s32 sparkNext;
    /* 0x02C */ s32 unk2C;
    /* 0x030 */ EftAuraTexGroup grp[2];  /* [0] flames (set at +0x58), [1] sparks (set at +0x288) */
    /* 0x490 */ struct EftNTask **tasks; /* the aura task of each character, or NULL */
    /* 0x494 */ s32 started;     /* the per-character lightning check has run since the stage became ready */
    /* 0x498 */ s32 waitStage;   /* the stage was seen not ready */
    /* 0x49C */ void *data;      /* common entry 1: gEftAuraCfg and gEftAuraPrm both point at it */
} EftAuraMgr; /* size 0x4A0 */

/* Colours of one aura type, 0..255 per component. */
typedef struct EftAuraColors {
    /* 0x00 */ Vec4 color;       /* flame colour (start and end; the end alpha is forced to 0) */
    /* 0x10 */ Vec4 alt[4];
    /* 0x50 */ Vec4 spark[4];
    /* 0x90 */ Vec4 sparkAlt[4];
} EftAuraColors; /* size 0xD0 */

typedef struct EftAuraDataPart {
    /* 0x0 */ s32 node;      /* model node of the part */
    /* 0x4 */ s32 nodeEnd;   /* its second node, or negative */
    /* 0x8 */ s32 nodeRef;   /* node the flame leans away from, or negative */
} EftAuraDataPart;

/* A spark emitter. (eft_aura.h places these 4 bytes later with the fields {kind, node}: its `kind` is this `node`
   and its `node` is the next entry's `unk0`.) */
typedef struct EftAuraDataSpark {
    /* 0x0 */ s32 unk0;
    /* 0x4 */ s32 node;      /* model node the emitter sits on */
} EftAuraDataSpark;

/* The aura parameter file (common entry 1). Its first 0x194 bytes are `EftAuraPrm`, the rest `EftAuraCfg`. */
typedef struct EftAuraData {
    /* 0x000 */ EftAuraPrm prm;
    /* 0x194 */ u8 unk194[0x7C];
    /* 0x210 */ Vec4 corner[2][4];     /* flame quad corners (x across, y depth, z along) by flame->unk4 */
    /* 0x290 */ Vec4 cornerB[2][4];    /* the same for auras with flag 0x80 */
    /* 0x310 */ Vec4 uv[4][4];         /* texture coordinates by unk4 * 2 + flip */
    /* 0x410 */ EftAuraDataPart part[EFT_AURA_PARTS];
    /* 0x488 */ s32 fadeNode[6];       /* model nodes a flame fades out near; the draw reads seven, so the last
                                          one is spark[0].unk0 */
    /* 0x4A0 */ EftAuraDataSpark spark[EFT_AURA_SPARKS];
    /* 0x500 */ u8 unk500[0x10];
    /* 0x510 */ Vec4 color[9][13];     /* by aura type; each row is an EftAuraColors */
    /* 0xC60 */ Vec4 colorB[5][13];    /* flag 0x80: row 0, 2 or 3 */
} EftAuraData;

struct EftNTask;

f32 EftAura_GetNodeFade(s32 objId, Vec4 *pos, Vec4 *nodes, f32 scale);
void EftAura_DrawFlames(EftAuraWork *aura, s32 objId, f32 alpha);
void EftAura_UpdateTextures(EftAuraWork *aura);
void EftAura_UpdateLightning(s32 objId);
void EftAura_SetType(EftAuraWork *aura, s32 objId, s32 type, s32 paramFlags);
void EftAura_Setup(EftAuraWork *aura, s32 objId);
void EftAuraTask_Init(struct EftNTask *task, EftAuraArg *arg);
void EftAuraTask_Term(struct EftNTask *task);
void EftAuraTask_Update(struct EftNTask *task);
void EftAuraTask_PostUpdate(struct EftNTask *task);
void EftAuraTask_Reset(struct EftNTask *task);
void EftAuraTask_Draw(struct EftNTask *task);
void EftAuraMgr_Init(struct EftNTask *task);
void EftAuraMgr_Term(void);
void EftAuraMgr_Update(void);
void EftAuraMgr_Stub(void);
s32 EftAura_Request(s32 objId, s32 variant);
s32 EftAura_Stop(s32 objId, s32 variant);
s32 EftAura_Kill(s32 objId);
void EftAura_SetLevel(s32 objId, f32 level);
s32 EftAura_IsVisible(s32 objId);
s32 EftAura_IsBurstRising(s32 objId);
s32 EftAura_StartBurst(s32 objId);
s32 EftAura_EndBurst(s32 objId);
s32 EftAura_PeakBurst(s32 objId);
s32 EftAura_FadeIn(s32 objId);
s32 EftAura_FadeOut(s32 objId);
s32 EftAura_Command(s32 objId, s32 cmd);
void EftAura_Show(s32 objId);
void EftAura_Hide(s32 objId);
void EftAura_MarkModelNew(s32 objId);
s32 EftAura_ChangeType(s32 objId, s32 type, s32 paramFlags);
EftAuraColors *EftAura_GetColors(s32 type, s32 paramFlags);
s32 EftAura_Finish(s32 objId);
s32 EftAura_CommandAlt(s32 objId, s32 cmd);

/* ---- body lightning ------------------------------------------------------------------------------------- */

#define EFT_BOLT_COUNT 10
#define EFT_BOLT_FLASHES 20

/* One joint of a bolt. A bolt is drawn as a strip of quads between consecutive joints. */
typedef struct EftBoltSeg {
    /* 0x00 */ s32 flags;    /* 0 = free; 1 used, 2 shown, 4 first flash frame done, 8 flash over, 0x10 released (it
                                drifts, shrinks and fades), 0x20 thick (not the first or last joint) */
    /* 0x04 */ f32 widthA;   /* half width on one side */
    /* 0x08 */ f32 widthB;   /* and on the other */
    /* 0x0C */ f32 alphaMax; /* 150 / 255 */
    /* 0x10 */ Vec4 pos;     /* relative to the bolt's origin */
    /* 0x20 */ f32 r;        /* 50 / 255 */
    /* 0x24 */ f32 g;        /* 30 / 255 */
    /* 0x28 */ f32 b;        /* 150 / 255 */
    /* 0x2C */ f32 alpha;
    /* 0x30 */ struct EftBoltSeg *next;
    /* 0x34 */ u8 unk34[0xC];
} EftBoltSeg; /* size 0x40 */

/* A bolt: a chain of joints from one model node to another. */
typedef struct EftBolt {
    /* 0x00 */ EftBoltSeg *head;
    /* 0x04 */ s32 shown;    /* joints shown; jumps to `count` on the first update, then falls by 2 or 3 a frame, and
                                the first `count - shown` joints are released */
    /* 0x08 */ s32 count;    /* joints in the chain */
    /* 0x0C */ s32 flags;    /* 0 = free; 1 active, 2 fully shown (joints are being released), 4 finished */
    /* 0x10 */ s32 kind;     /* row of the node pair table */
    /* 0x14 */ s32 nodeA;
    /* 0x18 */ s32 nodeB;
    /* 0x1C */ f32 life;     /* frames left, 30 at the start */
    /* 0x20 */ f32 follow;   /* how far the origin moves from node A towards node B, 0..1 */
    /* 0x24 */ u8 unk24[0xC];
    /* 0x30 */ Vec4 pos;     /* origin */
    /* 0x40 */ Vec4 unk40;
    /* 0x50 */ Vec4 start;   /* start point, relative to the origin */
    /* 0x60 */ Vec4 end;     /* end point */
    /* 0x70 */ Vec4 dir;     /* unit vector from start to end */
    /* 0x80 */ Vec4 normal;  /* unit vector the bolt bulges along */
} EftBolt; /* size 0x90 */

/* A flash sprite that follows two model nodes. */
typedef struct EftBoltFlash {
    /* 0x00 */ s32 flags;    /* 0 = free; 1 alive */
    /* 0x04 */ s32 nodeA;
    /* 0x08 */ s32 nodeB;
    /* 0x0C */ s32 frame;    /* 0..3: cell of the 2 x 2 sheet */
    /* 0x10 */ s32 kind;     /* texture of the flash set */
    /* 0x14 */ f32 life;     /* frames left: 6 (kind 0) or 3 */
    /* 0x18 */ f32 size;
    /* 0x1C */ f32 sizeRate;
    /* 0x20 */ f32 alphaRate;
    /* 0x24 */ f32 follow;
    /* 0x28 */ f32 rot;      /* -pi..pi */
    /* 0x2C */ f32 unk2C;
    /* 0x30 */ Vec4 pos;
    /* 0x40 */ Vec4 vel;
    /* 0x50 */ f32 r;
    /* 0x54 */ f32 g;
    /* 0x58 */ f32 b;
    /* 0x5C */ f32 a;
    /* 0x60 */ Vec4 uv;
} EftBoltFlash; /* size 0x70 */

/* Work of one fighter's lightning task (0xEB0 bytes; the task itself is at 0x167FD8.., in the next file). */
typedef struct EftBoltWork {
    /* 0x000 */ s32 flags;
    /* 0x004 */ s32 objId;
    /* 0x008 */ u8 unk8[8];
    /* 0x010 */ EftBolt bolt[EFT_BOLT_COUNT];
    /* 0x5B0 */ EftBoltFlash flash[EFT_BOLT_FLASHES];
    /* 0xE70 */ s32 paramFlags;  /* BtlCharApi_ObjGetParamFlags0; 8 = bolts more often */
    /* 0xE74 */ s32 side;        /* alternates 0 / 1 per spawn */
    /* 0xE78 */ s32 boltCount;   /* live bolts */
    /* 0xE7C */ s32 orderPos;    /* 0..3: position in `order` */
    /* 0xE80 */ s32 order[4];    /* a random permutation of 0..3 */
    /* 0xE90 */ f32 angle[2];    /* running angle of each side */
    /* 0xE98 */ u64 tex0;        /* GS TEX0 of the bolt texture */
    /* 0xEA0 */ f32 scale;       /* fighter height / 19.35 */
    /* 0xEA4 */ u8 unkEA4[0xC];
} EftBoltWork; /* size 0xEB0 */

/* A pair of model nodes a bolt can run between. */
typedef struct EftBoltPair {
    /* 0x00 */ s32 nodeA;
    /* 0x04 */ s32 nodeB;
    /* 0x08 */ f32 length;
    /* 0x0C */ f32 follow;       /* base */
    /* 0x10 */ f32 followRange;
} EftBoltPair; /* size 0x14 */

/* gEftBoltPool (0x478 bytes, allocated at 0x1682F0 in the next file). */
typedef struct EftBoltPool {
    /* 0x000 */ s32 unk0;
    /* 0x004 */ s32 segMax;
    /* 0x008 */ EftBoltSeg *segs;
    /* 0x00C */ s32 segNext;     /* where the search for a free joint starts */
    /* 0x010 */ u8 unk10[0x258];
    /* 0x268 */ EftNTexSet flashTex;
    /* 0x470 */ struct EftNTask **tasks;
    /* 0x474 */ EftBoltPair *pairs;
} EftBoltPool; /* size 0x478 */

extern EftBoltPool *gEftBoltPool;

s32 EftBolt_AllocSegs(EftBolt *bolt, s32 count);
void EftBolt_FreeSegs(EftBolt *bolt);
void EftBolt_Shape(EftBolt *bolt, EftVec start, EftVec end, f32 scale);
s32 EftBolt_Step(EftBolt *bolt, f32 scale);
void EftBolt_Draw(EftBoltWork *work, EftBolt *bolt, f32 alpha);
s32 EftBolt_Start(EftBoltWork *work, EftBolt *bolt, s32 objId, s32 kind, s32 nodeA, s32 nodeB, EftVec dirA,
                  EftVec dirB, f32 length, f32 follow);
void EftBolt_UpdateAll(EftBoltWork *work, s32 objId);
void EftBolt_DrawAll(EftBoltWork *work, f32 alpha);
s32 EftBolt_AddFlash(EftBoltWork *work, s32 kind, s32 nodeA, s32 nodeB, EftVec vel, f32 size, f32 follow);
void EftBolt_UpdateFlashes(EftBoltWork *work, s32 objId);
void EftBolt_DrawFlashes(EftBoltWork *work, f32 alpha);
void EftBolt_Spawn(EftBoltWork *work, s32 objId);


#endif

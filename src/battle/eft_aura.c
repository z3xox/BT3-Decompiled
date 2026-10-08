#include "common.h"
#include "battle/eft_aura.h"
#include "sys/gfx_ot.h"

/*
 * Effect tasks, 0x15F728..0x1637A0. See include/battle/eft_aura.h.
 */

#define RAND_MAX_F 2147483647.0f
#define RANDF() ((f32)rand() / RAND_MAX_F)

typedef struct EftMBattleWork {
    /* 0x0000 */ u8 unk0[0x19F0];
    /* 0x19F0 */ u64 flags; /* 0x100 = paused */
} EftMBattleWork;

/* The view being drawn (include/battle/btl_cam.h). */
typedef struct EftMView {
    /* 0x000 */ Mtx44 world2view;
    /* 0x040 */ Mtx44 world2view2;
    /* 0x080 */ u8 unk80[0xC0];
    /* 0x140 */ Mtx44 world2screen;
} EftMView;

extern EftMView *gBtlCamView;

#define V(p) ((Vec4 *)(p))

extern s32 rand(void);
extern void *memset(void *dst, s32 c, u32 n);
extern f32 sqrtf(f32 x);
extern f32 Mathf_Sin(f32 angle);
extern f32 Mathf_Cos(f32 angle);
extern void Vec4_Set(Vec4 *dst, f32 x, f32 y, f32 z, f32 w);
extern void Vec3_Set(Vec4 *dst, f32 x, f32 y, f32 z);  /* sets x, y, z */
extern void Vec4_Copy(Vec4 *dst, Vec4 *src);
extern void Vec3_Copy(Vec4 *dst, Vec4 *src);            /* copies x, y, z */
extern void Vec4_Add(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void Vec3_Add(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void Vec4_Sub(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void Vec3_Sub(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void Vec4_Scale(Vec4 *dst, Vec4 *src, f32 s);
extern void Vec3_Scale(Vec4 *dst, Vec4 *src, f32 s);
extern f32 Vec3_Dot(Vec4 *a, Vec4 *b);
extern void Vec3_Cross(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void Vec3_Normalize(Vec4 *dst, Vec4 *src);
extern void Vec4_Clamp(Vec4 *dst, Vec4 *src, f32 lo, f32 hi); /* clamps each component */
extern void Mtx_StoreIdentity(Mtx44 *m);
extern void Mtx_Copy(Mtx44 *dst, Mtx44 *src);          /* copy */
extern void Mtx_InverseRT(Mtx44 *dst, Mtx44 *src);          /* inverse of a rotation + translation matrix */
extern void Vu0Cur_Push(void);                            /* VU0 matrix stack push */
extern void Vu0Cur_LoadMtx(Mtx44 *m);                        /* load the matrix */
extern void Vu0Cur_Pop(void);                            /* pop */
extern void ClipVtx_Set(void *vtx, Vec4 *pos, Vec4 *uv, Vec4 *col);
extern void EftGfx_DrawPolyAvgZFront(void *prim, s32 blend, s32 a2, s32 a3, s32 inView, s32 flip, u64 tex, s32 zOfs);
extern void EftTexSet_Load4(EftSpdTex *tex, s32 *entry);
extern void EftTexSet_Keep4(EftSpdTex *tex, s32 tcc, s32 tfx);
extern EftMBattleWork *Battle_GetWork(void);
extern s32 BtlPool_GetCurrent(void);
extern void *BtlPool_Alloc(s32 slot, s32 size);
extern void BtlPool_Free(s32 slot, void *ptr);
extern s32 *BtlScene_GetCommonEntry(s32 idx);
extern s32 BtlScene_IsCharInView(s32 objId);
extern f32 BtlCharApi_GetHeight(s32 objId);
extern f32 BtlCharApi_GetNodeBoundSize(s32 objId, s32 node);
extern void BtlCharApi_GetNodePos(s32 objId, s32 node, Vec4 *out);
extern void BtlCharApi_GetFrameMove(s32 objId, Vec4 *out);
extern void BtlCharApi_GetVelocity(s32 objId, Vec4 *out);
extern void BtlCharApi_GetDir(s32 objId, Vec4 *out);
extern f32 BtlCharApi_GetSpeed(s32 objId);
extern f32 BtlCharApi_GetFallSpeed(s32 objId);
extern s32 BtlCharApi_GetActionFxKind(s32 objId);

/* Spawns `count` streaks in a ring around a model node of the fighter, pointing against `dir`. */
s32 EftSpdLine_SpawnStreakRing(s32 objId, Vec4 *dir, s32 node, s32 count, f32 unused, f32 lifeMin, f32 lifeRange) {
    EftVec pos;
    EftVec tail;
    EftVec back;
    EftVec tmp;
    EftVec up = { 0.0f, -1.0f, 0.0f, 1.0f };
    EftVec side2;
    EftVec side;
    f32 size;
    f32 minSize;
    f32 ang;
    f32 radius;
    f32 dist;
    f32 speed;
    s32 i;

    if (gEftSpdLine == NULL) {
        return 0;
    }
    minSize = 0.8f;
    size = BtlCharApi_GetNodeBoundSize(objId, node) * 0.4f;
    if (size < minSize) {
        size = BtlCharApi_GetHeight(objId) * 0.05f * minSize;
    }
    Vec4_Scale(V(&back), dir, -1.0f);
    Vec3_Normalize(V(&back), V(&back));
    Vec3_Cross(V(&side), V(&back), V(&up));
    Vec3_Normalize(V(&side), V(&side));
    Vec3_Cross(V(&side2), V(&back), V(&side));
    Vec3_Normalize(V(&side2), V(&side2));
    for (i = 0; i < count; i++) {
        BtlCharApi_GetNodePos(objId, node, V(&pos));
        ang = RANDF() * 6.2831853f;
        radius = (RANDF() * 1.8f + 0.1f) * size;
        dist = (RANDF() * 2.0f + 1.5f) * size;
        Vec4_Scale(V(&tmp), V(&side), Mathf_Cos(ang) * radius);
        Vec4_Add(V(&pos), V(&pos), V(&tmp));
        Vec4_Scale(V(&tmp), V(&side2), Mathf_Sin(ang) * radius);
        Vec4_Add(V(&pos), V(&pos), V(&tmp));
        Vec4_Scale(V(&tmp), V(&back), dist);
        Vec4_Sub(V(&pos), V(&pos), V(&tmp));
        Vec4_Scale(V(&tmp), V(&back), (RANDF() * 3.0f + 2.0f) * size);
        Vec4_Add(V(&tail), V(&pos), V(&tmp));
        speed = (RANDF() * 2.0f + 0.5f) * (BtlCharApi_GetHeight(objId) * 0.05f);
        EftSpdLine_AddStreak(objId, pos, tail, back, speed, RANDF() * lifeRange + lifeMin, 0.075f);
    }
    return 1;
}

/* Allocates the work once and loads the textures (common entry 0x237). */
void EftSpdLine_Init(void) {
    if (gEftSpdLine == NULL) {
        gEftSpdLine = BtlPool_Alloc(BtlPool_GetCurrent(), sizeof(EftSpdLineWork));
        memset(gEftSpdLine, 0, sizeof(EftSpdLineWork));
    }
    EftTexSet_Load4(gEftSpdLine->tex, BtlScene_GetCommonEntry(0x237));
}

/* Frees the work. */
void EftSpdLine_Term(void) {
    if (gEftSpdLine != NULL) {
        BtlPool_Free(BtlPool_GetCurrent(), gEftSpdLine);
        gEftSpdLine = NULL;
    }
}

/* Task update: steps both lists, then the texture. */
void EftSpdLine_Update(void) {
    EftSpdLine_UpdateTrails();
    EftSpdLine_UpdateStreaks();
    EftSpdLine_UpdateTexture(1, 0);
}

/* Empty task slot. */
void EftSpdLine_Stub(void) {
}

/* Task draw: both lists under the current view's world-to-screen matrix. */
void EftSpdLine_Draw(void) {
    Vu0Cur_Push();
    Vu0Cur_LoadMtx(&gBtlCamView->world2screen);
    EftSpdLine_DrawTrails();
    EftSpdLine_DrawStreaks();
    Vu0Cur_Pop();
}

/* Returns a zeroed free trail slot, or NULL when all 30 are in use. */
EftSpdTrail *EftSpdLine_AllocTrail(void) {
    EftSpdTrail *p = gEftSpdLine->trails;
    s32 i;

    for (i = 0; i < 30; i++, p++) {
        if (p->flags == 0) {
            memset(p, 0, sizeof(EftSpdTrail));
            return p;
        }
    }
    return NULL;
}

/* Adds a trail at the list tail: random width scaled by the fighter's height, alpha 64, two frames of life. */
void EftSpdLine_AddTrail(s32 objId, EftVec pos, EftVec dir) {
    EftSpdTrail *p = EftSpdLine_AllocTrail();
    s32 r;

    if (p != NULL) {
        if (gEftSpdLine->trailHead == NULL) {
            gEftSpdLine->trailHead = p;
            p->prev = NULL;
            p->next = NULL;
        } else {
            EftSpdTrail *last = gEftSpdLine->trailTail;

            p->next = NULL;
            p->prev = last;
        }
        if (gEftSpdLine->trailTail != NULL) {
            gEftSpdLine->trailTail->next = p;
        }
        gEftSpdLine->trailTail = p;
        p->objId = objId;
        Vec4_Copy(&p->pos, V(&pos));
        Vec4_Copy(&p->tail, V(&pos));
        Vec4_Copy(&p->dir, V(&dir));
        r = rand();
        p->width = ((f32)r / RAND_MAX_F * 0.1f + 0.05f) * (BtlCharApi_GetHeight(objId) * 0.05f);
        p->alpha = 64.0f;
        p->life = 2.0f;
        p->flags = 1;
        p->tex = 0;
        p->blend = 0;
        gEftSpdLine->trailCount++;
    }
}

/* Steps every trail (not while paused) and unlinks the finished ones. */
void EftSpdLine_UpdateTrails(void) {
    EftSpdTrail *p;
    EftSpdTrail *cur;
    s32 done;

    if (!(Battle_GetWork()->flags & 0x100)) {
        p = gEftSpdLine->trailHead;
        while (p != NULL) {
            cur = p;
            done = EftSpdLine_StepTrail(p);
            p = p->next;
            if (done) {
                cur->flags = 0;
                if (cur->prev == NULL) {
                    gEftSpdLine->trailHead = cur->next;
                } else {
                    cur->prev->next = cur->next;
                }
                if (cur->next == NULL) {
                    gEftSpdLine->trailTail = cur->prev;
                } else {
                    cur->next->prev = cur->prev;
                }
                cur->prev = NULL;
                cur->next = NULL;
                gEftSpdLine->trailCount--;
            }
        }
    }
}

/* Moves a trail with its fighter; returns 1 when its life ran out. */
s32 EftSpdLine_StepTrail(EftSpdTrail *p) {
    Vec4 tmp;
    Vec4 move;
    f32 unit;
    s32 r;

    if (p->flags & 1) {
        unit = 0.05f;
        BtlCharApi_GetFrameMove(p->objId, &move);
        Vec4_Scale(&tmp, &move, BtlCharApi_GetHeight(p->objId) * unit * 0.2f);
        Vec4_Add(&p->pos, &p->pos, &tmp);
        r = rand();
        Vec4_Scale(&tmp, &move, -(((f32)r / RAND_MAX_F * 0.8f + 0.8f) * (BtlCharApi_GetHeight(p->objId) * unit)));
        Vec4_Add(&p->tail, &p->pos, &tmp);
        p->life -= 1.0f;
    }
    if (p->life <= 0.0f) {
        p->life = 0.0f;
        return 1;
    }
    return 0;
}

/* Draws every live trail, white with its alpha. */
void EftSpdLine_DrawTrails(void) {
    Mtx44 mtx;
    Vec4 quad[4];
    EftSpdTrail *p;
    s32 inView;

    Mtx_Copy(&mtx, &gBtlCamView->world2screen);
    for (p = gEftSpdLine->trailHead; p != NULL; p = p->next) {
        if (p->flags & 1) {
            inView = BtlScene_IsCharInView(p->objId);
            EftSpdLine_BuildTrailQuad(quad, p);
            EftSpdLine_DrawQuad(quad, &mtx, &gEftSpdLine->tex[p->tex], 255.0f, 255.0f, 255.0f, p->alpha, p->blend,
                                inView);
        }
    }
}

/* Returns a zeroed free streak slot, or NULL when all 40 are in use. */
EftSpdStreak *EftSpdLine_AllocStreak(void) {
    EftSpdStreak *p = gEftSpdLine->streaks;
    s32 i;

    for (i = 0; i < 40; i++, p++) {
        if (p->flags == 0) {
            memset(p, 0, sizeof(EftSpdStreak));
            return p;
        }
    }
    return NULL;
}

/* Adds a streak at the list tail: alpha 160, fading to 0 in `life` seconds. */
void EftSpdLine_AddStreak(s32 objId, EftVec pos, EftVec tail, EftVec dir, f32 speed, f32 life, f32 width) {
    EftSpdStreak *p = EftSpdLine_AllocStreak();

    if (p != NULL) {
        if (gEftSpdLine->streakHead == NULL) {
            gEftSpdLine->streakHead = p;
            p->prev = NULL;
            p->next = NULL;
        } else {
            EftSpdStreak *last = gEftSpdLine->streakTail;

            p->next = NULL;
            p->prev = last;
        }
        if (gEftSpdLine->streakTail != NULL) {
            gEftSpdLine->streakTail->next = p;
        }
        gEftSpdLine->streakTail = p;
        p->objId = objId;
        Vec4_Copy(&p->pos, V(&pos));
        Vec4_Copy(&p->tail, V(&tail));
        Vec4_Copy(&p->dir, V(&dir));
        p->speed = speed;
        p->width = width;
        p->alpha = 160.0f;
        p->flags = 1;
        p->tex = 0;
        p->fade = 160.0f / (life * 30.0f);
        p->blend = 0;
        gEftSpdLine->streakCount++;
    }
}

/* Steps every streak (not while paused) and unlinks the finished ones. */
void EftSpdLine_UpdateStreaks(void) {
    EftSpdStreak *p;
    EftSpdStreak *cur;
    s32 done;

    if (!(Battle_GetWork()->flags & 0x100)) {
        p = gEftSpdLine->streakHead;
        while (p != NULL) {
            cur = p;
            done = EftSpdLine_StepStreak(p);
            p = p->next;
            if (done) {
                cur->flags = 0;
                if (cur->prev == NULL) {
                    gEftSpdLine->streakHead = cur->next;
                } else {
                    cur->prev->next = cur->next;
                }
                if (cur->next == NULL) {
                    gEftSpdLine->streakTail = cur->prev;
                } else {
                    cur->next->prev = cur->prev;
                }
                cur->prev = NULL;
                cur->next = NULL;
                gEftSpdLine->streakCount--;
            }
        }
    }
}

/* Moves a streak along its direction and fades it; returns 1 when it is fully transparent. */
s32 EftSpdLine_StepStreak(EftSpdStreak *p) {
    Vec4 move;

    if (p->flags & 1) {
        Vec4_Scale(&move, &p->dir, p->speed);
        Vec4_Add(&p->pos, &p->pos, &move);
        Vec4_Add(&p->tail, &p->tail, &move);
        p->alpha -= p->fade;
    }
    if (p->alpha <= 0.0f) {
        p->alpha = 0.0f;
        return 1;
    }
    return 0;
}

/* Draws every live streak, black with its alpha, once the texture is ready. */
void EftSpdLine_DrawStreaks(void) {
    Mtx44 mtx;
    Vec4 quad[4];
    EftSpdStreak *p;
    s32 inView;

    if (gEftSpdLine->texReady == 0) {
        return;
    }
    Mtx_Copy(&mtx, &gBtlCamView->world2screen);
    for (p = gEftSpdLine->streakHead; p != NULL; p = p->next) {
        if (p->flags & 1) {
            inView = BtlScene_IsCharInView(p->objId);
            EftSpdLine_BuildStreakQuad(quad, p);
            EftSpdLine_DrawQuad(quad, &mtx, &gEftSpdLine->tex[p->tex], 0.0f, 0.0f, 0.0f, p->alpha, p->blend, inView);
        }
    }
}

/* Corners of a trail: both ends widened across the line of sight from the camera. */
void EftSpdLine_BuildTrailQuad(Vec4 *out, EftSpdTrail *p) {
    Vec4 eye;
    Vec4 toEye;
    Vec4 side;
    Mtx44 inv;
    Vec4 *first = &p->tail;
    Vec4 *out1 = &out[1];
    Vec4 *dir = &p->dir;
    Vec4 *out3 = &out[3];

    Mtx_InverseRT(&inv, &gBtlCamView->world2view2);
    Vec4_Copy(&eye, (Vec4 *)inv.m[3]);
    Vec4_Sub(&toEye, &eye, first);
    Vec3_Normalize(&toEye, &toEye);
    Vec3_Cross(&side, dir, &toEye);
    Vec3_Normalize(&side, &side);
    Vec4_Scale(&side, &side, p->width);
    Vec4_Add(out, first, &side);
    Vec4_Sub(out1, first, &side);
    out1->w = 1.0f;
    out->w = 1.0f;
    Vec4_Sub(&toEye, &eye, &p->pos);
    Vec3_Normalize(&toEye, &toEye);
    Vec3_Cross(&side, dir, &toEye);
    Vec3_Normalize(&side, &side);
    Vec4_Scale(&side, &side, p->width);
    Vec4_Add(&out[2], &p->pos, &side);
    Vec4_Sub(out3, &p->pos, &side);
    out3->w = 1.0f;
    out[2].w = 1.0f;
}

/* Corners of a streak: as for a trail, with half the width on each side. */
void EftSpdLine_BuildStreakQuad(Vec4 *out, EftSpdStreak *p) {
    Vec4 eye;
    Vec4 toEye;
    Vec4 side;
    Mtx44 inv;
    Vec4 *second = &p->tail;
    Vec4 *out1 = &out[1];
    Vec4 *out3 = &out[3];
    Vec4 *dir = &p->dir;

    Mtx_InverseRT(&inv, &gBtlCamView->world2view2);
    Vec4_Copy(&eye, (Vec4 *)inv.m[3]);
    Vec4_Sub(&toEye, &eye, &p->pos);
    Vec3_Normalize(&toEye, &toEye);
    Vec3_Cross(&side, dir, &toEye);
    Vec3_Normalize(&side, &side);
    Vec4_Scale(&side, &side, p->width * 0.5f);
    Vec4_Add(out, &p->pos, &side);
    Vec4_Sub(out1, &p->pos, &side);
    out1->w = 1.0f;
    out->w = 1.0f;
    Vec4_Sub(&toEye, &eye, second);
    Vec3_Normalize(&toEye, &toEye);
    Vec3_Cross(&side, dir, &toEye);
    Vec3_Normalize(&side, &side);
    Vec4_Scale(&side, &side, p->width * 0.5f);
    Vec4_Add(&out[2], second, &side);
    Vec4_Sub(out3, second, &side);
    out3->w = 1.0f;
    out[2].w = 1.0f;
}

/* Sends the four corners as two triangles with one colour; the matrix argument is not used. */
void EftSpdLine_DrawQuad(Vec4 *quad, Mtx44 *mtx, EftSpdTex *tex, f32 r, f32 g, f32 b, f32 a, u8 blend, s32 inView) {
    u8 prim[9][0x30];
    Vec4 pos[4];
    Vec4 uv[4];
    Vec4 col[4];
    s32 i;

    Vec4_Copy(&pos[0], &quad[0]);
    Vec4_Copy(&pos[1], &quad[1]);
    Vec4_Copy(&pos[2], &quad[2]);
    Vec4_Copy(&pos[3], &quad[3]);
    Vec4_Set(&uv[0], 0.0f, 0.0f, 1.0f, 0.0f);
    Vec4_Set(&uv[1], 1.0f, 0.0f, 1.0f, 0.0f);
    Vec4_Set(&uv[2], 0.0f, 1.0f, 1.0f, 0.0f);
    Vec4_Set(&uv[3], 1.0f, 1.0f, 1.0f, 0.0f);
    Vec4_Set(&col[0], r, g, b, a);
    Vec4_Set(&col[1], r, g, b, a);
    Vec4_Set(&col[2], r, g, b, a);
    Vec4_Set(&col[3], r, g, b, a);
    memset(prim, 0, sizeof(prim));
    for (i = 0; i < 2; i++) {
        ClipVtx_Set(prim[0], &pos[i], &uv[i], &col[i]);
        ClipVtx_Set(prim[1], &pos[i + 1], &uv[i + 1], &col[i + 1]);
        ClipVtx_Set(prim[2], &pos[i + 2], &uv[i + 2], &col[i + 2]);
        EftGfx_DrawPolyAvgZFront(prim, blend, 0, 0, inView, 0, tex->tex, 0);
    }
}

/* Keeps the textures referenced while any segment is alive. */
void EftSpdLine_UpdateTexture(s32 tcc, s32 tfx) {
    if (gEftSpdLine->trailCount + gEftSpdLine->streakCount != 0) {
        EftTexSet_Keep4(gEftSpdLine->tex, tcc, tfx);
        gEftSpdLine->texReady = 1;
    } else {
        gEftSpdLine->texReady = 0;
    }
}

/* ---- aura particles ------------------------------------------------------------------------------------- */

extern void EftSpr_DrawRot(u8 r, u8 g, u8 b, u8 a, f32 x, f32 y, f32 z, f32 u0, f32 v0, f32 u1, f32 v1, f32 rot,
                          s32 ofsX, s32 ofsY, s32 w, s32 h, s32 unused, u32 size, s32 ctx, s32 layer, void *tex);

/* Drift direction of a spark: against the fighter's velocity, with a random vertical part. */
void EftAura_GetSparkDir(Vec4 *out, s32 objId) {
    Vec4 vel;

    BtlCharApi_GetVelocity(objId, &vel);
    Vec4_Scale(&vel, &vel, gEftAuraPrm->sparkVel);
    Vec4_Set(out, -vel.x, gEftAuraPrm->sparkRise * RANDF(), -vel.z, 1.0f);
    Vec3_Normalize(out, out);
}

/* The same for the alternate aura: straight up / down while the action effect kind is 3 / 4. */
void EftAura_GetSparkDirAlt(Vec4 *out, s32 objId) {
    Vec4 vel;
    s32 kind;

    BtlCharApi_GetVelocity(objId, &vel);
    Vec4_Scale(&vel, &vel, gEftAuraPrm->sparkVelAlt);
    kind = BtlCharApi_GetActionFxKind(objId);
    if (kind == 3) {
        Vec3_Set(out, 0.0f, 1.0f - BtlCharApi_GetFallSpeed(objId), 0.0f);
    } else if (kind == 4) {
        Vec3_Set(out, 0.0f, -1.0f - BtlCharApi_GetFallSpeed(objId), 0.0f);
    } else {
        Vec3_Set(out, -vel.x, gEftAuraPrm->sparkRise * RANDF(), -vel.z);
    }
    out->w = 1.0f;
    Vec3_Normalize(out, out);
}

/* Flame direction term: away from the aura centre, plus a random left / right push across the facing. */
void EftAura_GetFlameSideDir(EftAura *aura, Vec4 *out, s32 objId, f32 power, f32 away, f32 side) {
    Vec4 dir;
    Vec4 centre;
    f32 one = 1.0f;

    side += gEftAuraCfg->unk1A0 * power * gEftAuraPrm->sideScale;
    BtlCharApi_GetDir(objId, &dir);
    dir.y = 0.0f;
    dir.w = one;
    Vec3_Normalize(&dir, &dir);
    if (rand() & 1) {
        side = -side;
    }
    Vec4_Scale(&dir, &dir, side);
    Vec4_Set(&centre, aura->pushCentre.x, 0.0f, aura->pushCentre.z, one);
    Vec3_Sub(out, &centre, &aura->pos);
    out->w = one;
    Vec3_Normalize(out, out);
    Vec3_Scale(out, out, away);
    out->x += dir.x;
    out->z += dir.z;
}

/* Flame direction term: against the motion of the body part relative to the fighter's root. */
void EftAura_GetFlameDragDir(EftAura *aura, Vec4 *pos, Vec4 *out, s32 objId, s32 part) {
    Vec4 d;
    Vec4 now;
    Vec4 before;
    f32 one;

    Vec3_Sub(&now, pos, &aura->pos);
    Vec3_Sub(&before, &aura->partPrev[part], &aura->prevPos);
    Vec3_Sub(&d, &now, &before);
    one = 1.0f;
    d.w = one;
    Vec3_Normalize(&d, &d);
    Vec4_Scale(&d, &d, sqrtf(Vec3_Dot(&d, &d)) * gEftAuraPrm->drag);
    Vec3_Scale(out, &d, -1.0f);
    out->w = one;
}

/* Flame direction term: outward from a random point between the aura axis and the root, scaled down by speed. */
void EftAura_GetFlameOutDir(EftAura *aura, Vec4 *out, s32 objId, s32 part, f32 power, f32 amount) {
    Vec4 from;
    Vec4 span;
    Vec4 d;
    f32 one;
    f32 ratio;
    f32 t;

    ratio = BtlCharApi_GetSpeed(objId) * gEftAuraPrm->speedRef;
    one = 1.0f;
    t = RANDF();
    amount += gEftAuraCfg->unk1A0 * power;
    if (one < ratio) {
        ratio = one;
    }
    amount *= one - ratio;
    span.x = aura->pos.x - aura->axis.x;
    span.z = aura->pos.z - aura->axis.z;
    from.w = 0.0f;
    from.y = aura->pos.y;
    from.x = aura->axis.x + span.x * t;
    from.z = aura->axis.z + span.z * t;
    Vec3_Copy(&d, &aura->part[part]);
    d.w = 0.0f;
    Vec4_Sub(&d, &d, &from);
    Vec3_Normalize(&d, &d);
    Vec4_Set(out, d.x * amount, d.y * amount * gEftAuraPrm->outRise, d.z * amount, one);
}

/* Bends a flame's direction: early towards its start direction and turn axis, late towards `pos`. */
void EftAura_TurnFlame(EftAuraFlame *f, Vec4 *pos, f32 t) {
    Vec4 d;
    Vec4 tmp;
    f32 t3 = t * t * t;
    f32 one = 1.0f;
    Vec4 *dir = &f->dir;
    f32 turn = gEftAuraPrm->kind[f->kind].turn;

    Vec3_Scale(&tmp, &f->dirStart, (one - t3) * 0.1f * turn);
    Vec3_Add(dir, dir, &tmp);
    Vec3_Scale(&tmp, &f->dirTurn, (one - t) * 0.08f * turn);
    Vec3_Add(dir, dir, &tmp);
    Vec3_Sub(&d, pos, &f->pos);
    d.w = one;
    Vec3_Normalize(&d, &d);
    Vec3_Scale(&tmp, &d, t3 * 0.15f * turn);
    Vec3_Add(dir, dir, &tmp);
    f->dir.w = one;
    Vec3_Normalize(dir, dir);
}

/* Accelerates a flame early in its life and damps it later. */
void EftAura_AccelFlame(EftAuraFlame *f, f32 t) {
    f->speed = (f->speed + gEftAuraPrm->speed[1] * (1.0f - t) * gEftAuraPrm->kind[f->kind].accel) * (t * 0.2f + 0.8f);
}

/* Offset from the aura axis (at the root's height) towards a body part. */
void EftAura_GetPartOffset(EftAura *aura, Vec4 *out, s32 objId, s32 part, f32 radial, f32 vertical) {
    Vec4 from;
    Vec4 d;

    from.x = aura->axis.x;
    from.y = aura->pos.y;
    from.z = aura->axis.z;
    from.w = 0.0f;
    Vec3_Copy(&d, &aura->part[part]);
    Vec3_Sub(&d, &d, &from);
    d.w = 1.0f;
    Vec3_Normalize(&d, &d);
    out->w = 0.0f;
    out->x = d.x * radial;
    out->y = d.y * vertical;
    out->z = d.z * radial;
}

/* Takes a spark from the free list, else the next unused slot; links it on the used list. */
EftAuraSpark *EftAura_AllocSpark(void) {
    EftAuraSpark *p;

    if (gEftAuraPool->sparkFree != NULL) {
        p = gEftAuraPool->sparkFree;
        gEftAuraPool->sparkFree = p->next;
    } else if (gEftAuraPool->sparkNext <= gEftAuraPool->sparkMax - 1) {
        p = &gEftAuraPool->sparks[gEftAuraPool->sparkNext++];
    } else {
        return NULL;
    }
    p->next = gEftAuraPool->sparkUsed;
    gEftAuraPool->sparkUsed = p;
    p->flags = 0;
    return p;
}

/* Returns every spark of a fighter to the free list. */
void EftAura_FreeSparks(s32 objId) {
    EftAuraSpark **link = &gEftAuraPool->sparkUsed;
    EftAuraSpark *p;

    while (*link != NULL) {
        p = *link;
        if (p->objId != objId) {
            link = &p->next;
        } else {
            *link = p->next;
            p->next = gEftAuraPool->sparkFree;
            gEftAuraPool->sparkFree = p;
        }
    }
}

/* Creates one spark of an emitter: random life and colour, direction from the fighter's motion. */
s32 EftAura_SpawnSpark(EftAura *aura, s32 objId, s32 emitter, s32 emitNode, Vec4 *offset, f32 follow) {
    Vec4 dir;
    EftAuraSpark *s = EftAura_AllocSpark();
    s32 color = rand() % aura->colorCount;
    f32 alpha;

    if (s == NULL) {
        return 0;
    }
    s->objId = objId;
    s->flags = 0;
    if (aura->flags & 0x80) {
        s->flags = 8;
    }
    if ((u32)(aura->type - 9) < 2) {
        s->flags |= 0x10;
    }
    if (aura->burst == 1) {
        s->flags |= 2;
    }
    s->emitter = emitter;
    s->kind = emitNode;
    s->age = 0.0f;
    if (s->flags & 0x10) {
        s->life = gEftAuraCfg->sparkLifeLong + gEftAuraCfg->sparkLifeLongRange * RANDF();
    } else {
        s->life = gEftAuraCfg->sparkLife + gEftAuraCfg->sparkLifeRange * RANDF();
    }
    s->follow = follow;
    Vec4_Copy(&s->offset, offset);
    Vec4_Set(&s->move, 0.0f, 0.0f, 0.0f, 0.0f);
    s->unk7 = 1;
    if (!(aura->flags & 0x20)) {
        Vec3_Copy(&s->color, &aura->sparkColor[color]);
        s->color.w = 0.0f;
        alpha = aura->sparkColor[color].w * aura->level;
        EftAura_GetSparkDir(&dir, objId);
        if (s->flags & 8) {
            s->size = gEftAuraCfg->sparkSize[1][0];
            s->speed = gEftAuraCfg->sparkSize[1][1];
        } else {
            s->size = gEftAuraCfg->sparkSize[0][0];
            s->speed = gEftAuraCfg->sparkSize[0][1];
        }
    } else {
        Vec3_Copy(&s->color, &aura->sparkColorAlt[color]);
        s->color.w = 0.0f;
        alpha = aura->sparkColorAlt[color].w;
        EftAura_GetSparkDirAlt(&dir, objId);
        if (s->flags & 8) {
            s->size = gEftAuraCfg->sparkSize[3][0];
            s->speed = gEftAuraCfg->sparkSize[3][1];
        } else {
            s->size = gEftAuraCfg->sparkSize[2][0];
            s->speed = gEftAuraCfg->sparkSize[2][1];
        }
    }
    Vec4_Copy(&s->dir, &dir);
    s->dir.w = 1.0f;
    Vec3_Set(&s->colorRate, 0.0f, 0.0f, 0.0f);
    s->colorRate.w = alpha / gEftAuraCfg->sparkFadeIn;
    Vec4_Set(&s->uv, 0.0f, 0.0f, 1.0f, 1.0f);
    s->rot = 0.0f;
    s->rotSpeed = gEftAuraCfg->sparkRotSpeed * 3.14159265f;
    if (rand() & 1) {
        s->rotSpeed = -s->rotSpeed;
    }
    if (s->flags & 4) {
        s->rotSpeed *= gEftAuraCfg->sparkMul[0][0];
        s->speed *= gEftAuraCfg->sparkMul[0][1];
    } else if (s->flags & 2) {
        s->rotSpeed *= gEftAuraCfg->sparkMul[1][0];
        s->speed *= gEftAuraCfg->sparkMul[1][1];
    }
    return 1;
}

/* Spawns the sparks of one emitter: one per `sparkStep` of the distance to its node (one only with `once`). */
void EftAura_SpawnSparks(EftAura *aura, s32 objId, s32 emitter, s32 once) {
    Vec4 pos;
    Vec4 nodePos;
    Vec4 delta;
    Vec4 d;
    Vec4 offset;
    s32 count = 1;
    f32 step;
    f32 len;
    s32 node;
    s32 emitNode;
    s32 i;

    step = gEftAuraCfg->sparkStep * aura->scale;
    Vec4_Set(&offset, 0.0f, 0.0f, 0.0f, 1.0f);
    emitNode = gEftAuraCfg->spark[emitter].kind;
    node = gEftAuraCfg->spark[emitter].node;
    Vec4_Copy(&pos, &aura->sparkPos[emitter]);
    Vec4_Set(&delta, 0.0f, 0.0f, 0.0f, 1.0f);
    if (node >= 0) {
        BtlCharApi_GetNodePos(objId, node, &nodePos);
        Vec4_Sub(&d, &nodePos, &pos);
        len = sqrtf(Vec3_Dot(&d, &d));
        Vec4_Copy(&delta, &d);
        if (step < len) {
            count = len / step + 1.0f;
        }
    }
    for (i = 0; i < count; i++) {
        EftAura_SpawnSpark(aura, objId, emitter, emitNode, &offset, RANDF());
        if (once) {
            break;
        }
    }
}

/* Moves, spins and fades a fighter's sparks; a spark at the end of its life is replaced by a new one. */
void EftAura_StepSparks(EftAura *aura, s32 objId) {
    Vec4 tmp;
    Vec4 nodePos;
    Vec4 move;
    EftAuraSpark **link;
    EftAuraSpark *s;
    Vec4 *pos;
    Vec4 *drift;
    f32 one = 1.0f;
    f32 t;
    f32 fadeOut;
    s32 node;

    Vec4_Set(&tmp, 0.0f, 0.0f, 0.0f, one);
    link = &gEftAuraPool->sparkUsed;
    while (*link != NULL) {
        s = *link;
        if (s->objId == objId) {
        if (!(s->age < s->life)) {
            EftAura_SpawnSparks(aura, s->objId, s->emitter, 1);
            *link = s->next;
            s->next = gEftAuraPool->sparkFree;
            gEftAuraPool->sparkFree = s;
        } else {
            pos = &s->pos;
            Vec4_Copy(pos, &aura->sparkPos[s->emitter]);
            node = gEftAuraCfg->spark[s->emitter].node;
            if (node >= 0) {
                BtlCharApi_GetNodePos(s->objId, node, &nodePos);
                Vec4_Sub(&tmp, &nodePos, pos);
                Vec3_Scale(&move, &tmp, s->follow);
                Vec4_Add(pos, pos, &move);
            }
            s->pos.w = one;
            Vec3_Scale(&move, &s->dir, s->speed);
            drift = &s->move;
            Vec4_Add(drift, drift, &move);
            t = s->age / s->life;
            s->speed += gEftAuraCfg->sparkAccel * t;
            s->speed *= gEftAuraCfg->sparkDamp * t;
            Vec4_Add(pos, pos, drift);
            s->pos.w = one;
            s->rot += s->rotSpeed;
            if (s->rot >= 3.14159265f) {
                s->rot -= 6.2831853f;
            } else if (s->rot <= -3.14159265f) {
                s->rot += 6.2831853f;
            }
            fadeOut = gEftAuraCfg->sparkFadeOut;
            if (s->life - s->age <= fadeOut) {
                if (!(s->flags & 1)) {
                    s->flags |= 1;
                    s->colorRate.w = -s->color.w / fadeOut;
                }
            } else if (gEftAuraCfg->sparkFadeIn < s->age) {
                s->colorRate.w = 0.0f;
            }
            s->color.w += s->colorRate.w;
            if (s->color.w < 0.0f) {
                s->color.w = 0.0f;
            }
            s->age += one;
            link = &s->next;
        }
        } else {
            link = &s->next;
        }
    }
}

/* Task entry: on the first call spawns the sparks of all twelve emitters, then steps them. */
void EftAura_UpdateSparks(EftAura *aura, s32 objId, s32 *state) {
    s32 i;

    if (!(*state & 1)) {
        for (i = 0; i < EFT_AURA_SPARKS; i++) {
            EftAura_SpawnSparks(aura, objId, i, 0);
        }
        *state |= 1;
    }
    EftAura_StepSparks(aura, objId);
}

/* Draws a fighter's sparks as rotated sprites. */
void EftAura_DrawSparks(EftAura *aura, s32 objId, f32 alpha) {
    EftAuraSpark **link = &gEftAuraPool->sparkUsed;
    s32 *tex = &gEftAuraPool->sparkTex;
    EftAuraSpark *s;
    f32 a;

    if (aura->flags & 0x20) {
        a = alpha;
    } else {
        a = aura->alpha * aura->alphaFade * alpha;
    }
    while (*link != NULL) {
        s = *link;
        if (s->objId == objId) {
            EftSpr_DrawRot((u32)(s->color.x * 255.0f), (u32)(s->color.y * 255.0f), (u32)(s->color.z * 255.0f),
                          (u32)(s->color.w * 255.0f * a), s->pos.x, s->pos.y, s->pos.z, s->uv.x, s->uv.y, s->uv.z,
                          s->uv.w, s->rot, 0, 0, 0x40, 0x40, 0, (u32)(s->size * 4096.0f), 0, s->unk7, tex);
        }
        link = &s->next;
    }
}

/* Takes a flame from the free list, else the next unused slot; links it on the used list. */
EftAuraFlame *EftAura_AllocFlame(void) {
    EftAuraFlame *p;

    if (gEftAuraPool->flameFree != NULL) {
        p = gEftAuraPool->flameFree;
        gEftAuraPool->flameFree = p->next;
    } else if (gEftAuraPool->flameNext <= gEftAuraPool->flameMax - 1) {
        p = &gEftAuraPool->flames[gEftAuraPool->flameNext++];
    } else {
        return NULL;
    }
    p->next = gEftAuraPool->flameUsed;
    gEftAuraPool->flameUsed = p;
    p->flags = 0;
    return p;
}

/* Flag word of a new flame from the aura's state. */
s32 EftAura_GetFlameFlags(EftAura *aura, s32 objId, s32 second, s32 alt) {
    s32 flags = 0x800;

    if (second == 0) {
        if (alt == 0) {
            flags = 0;
        } else {
            flags = 0x400;
        }
    }
    if (aura->flags & 0x80) {
        flags |= 0x100;
    }
    if ((u32)(aura->type - 9) < 2) {
        flags |= 0x200;
    }
    if (aura->burst == 1) {
        flags |= 0x20;
    } else if (aura->burst == 2) {
        flags |= 0x40;
    }
    if (aura->flags & 0x20) {
        flags |= 0x1000;
    }
    return flags;
}

/* Life of a new flame in frames. */
f32 EftAura_GetFlameLife(s32 flags, f32 burst) {
    f32 *p = gEftAuraPrm->life;
    f32 life;

    if (flags & 0x200) {
        life = p[2] + p[2] * RANDF();
    } else {
        life = p[0] + p[1] * RANDF();
    }
    if (flags & 0x20) {
        life *= 1.0f - burst * 0.15f;
    } else if (flags & 0x80) {
        life *= 0.85f;
    }
    return life;
}

/* Speed of a new flame. */
f32 EftAura_GetFlameSpeed(s32 flags, s32 objId, f32 level, f32 burst) {
    f32 *p = gEftAuraPrm->speed;
    f32 speed;

    speed = (BtlCharApi_GetSpeed(objId) * p[2] + p[0]) * level;
    if (flags & 0x100) {
        if (flags & 0x800) {
            speed += p[3];
        }
    }
    if (flags & 0x80) {
        speed += p[4];
    } else if (flags & 0x20) {
        speed += p[5] * burst;
    }
    if (!(flags & 0x800)) {
        if (flags & 0x400) {
            p = gEftAuraPrm->speedRand[1];
        } else {
            p = gEftAuraPrm->speedRand[0];
        }
    } else {
        p = gEftAuraPrm->speedRand[2];
    }
    return speed * (p[0] + p[1] * RANDF());
}

/* Peak alpha of a new flame. */
f32 EftAura_GetFlameAlpha(EftAura *aura, s32 flags, f32 alpha) {
    f32 *base = gEftAuraPrm->alpha;
    f32 *mul = gEftAuraPrm->alphaMul;

    if (!(flags & 0x800)) {
        alpha *= base[0];
    } else {
        alpha *= base[1];
    }
    if (flags & 0x80) {
        alpha *= mul[2];
    } else if (flags & 0x20) {
        alpha *= mul[3];
    } else if (!(flags & 0x800)) {
        alpha *= mul[0];
    } else {
        alpha *= mul[1];
    }
    return alpha;
}

/* Stretch of a flame at a phase (0 start, 1 grown, 2 late, 3 end); 0 for an invisible flame. */
f32 EftAura_GetFlameStretch(EftAuraFlame *f, s32 objId, s32 phase) {
    f32 *mul = gEftAuraPrm->stretchMul;
    f32 *kind = gEftAuraPrm->stretchKind;
    f32 *rnd = gEftAuraPrm->stretchRand[f->kind];
    f32 *st = &gEftAuraPrm->stretch[phase];
    f32 v = *st;
    f32 k;

    if (f->flags & 0x1000) {
        return 0.0f;
    }
    if (f->flags & 0x100) {
        if (!(f->flags & 0x800)) {
            v *= kind[2];
        } else {
            v *= kind[3];
        }
    } else if (!(f->flags & 0x800)) {
        v *= kind[0];
    } else {
        v *= kind[1];
    }
    if (f->flags & 0x80) {
        v *= mul[0];
    } else if (f->flags & 0x20) {
        v *= mul[1];
    } else if (f->flags & 0x40) {
        v *= mul[2];
    }
    k = rnd[0] + rnd[1] * RANDF();
    k += BtlCharApi_GetSpeed(objId) * rnd[2];
    if (1.0f < k) {
        k = 1.0f;
    }
    return v * k;
}

/* Size of a flame at a phase; 0 for an invisible flame. */
f32 EftAura_GetFlameSize(EftAuraFlame *f, s32 objId, s32 phase) {
    f32 v = gEftAuraPrm->size[phase];

    if (f->flags & 0x1000) {
        return 0.0f;
    }
    if (f->flags & 0x100) {
        if (!(f->flags & 0x800)) {
            v *= gEftAuraPrm->sizeKind[2];
        } else {
            v *= gEftAuraPrm->sizeKind[2];
        }
    } else {
        v *= gEftAuraPrm->sizeKind[0];
    }
    if (f->flags & 0x80) {
        v *= gEftAuraPrm->sizeMul[0];
    } else if (f->flags & 0x20) {
        v *= gEftAuraPrm->sizeMul[1];
    } else if (f->flags & 0x40) {
        v *= gEftAuraPrm->sizeMul[2];
    }
    return v;
}

/* End ratio of a flame: random, lower at speed, at least 0.65 (0.9 for the alternate kind). */
f32 EftAura_GetFlameEnd(EftAuraFlame *f, s32 objId) {
    f32 *p = gEftAuraPrm->end[f->kind];
    f32 v;

    v = p[0] + p[1] * RANDF();
    v -= BtlCharApi_GetSpeed(objId) * p[2];
    if (!(f->flags & 0x800)) {
        if (v < 0.65f) {
            v = 0.65f;
        }
    } else {
        if (v < 0.9f) {
            v = 0.9f;
        }
    }
    return v;
}

/* Creates one flame at a body part. `second` makes the extra flame of kind 2 (flag 0x800). */
s32 EftAura_SpawnFlame(EftAura *aura, s32 objId, s32 part, Vec4 *pos, Vec4 *offset, f32 follow, s32 alt, s32 second) {
    Vec4 dir;
    Vec4 side;
    Vec4 drag;
    Vec4 out;
    Vec4 d;
    EftAuraFlame *f = EftAura_AllocFlame();
    s32 color = rand() % aura->colorCount;
    f32 lifeScale;
    f32 level;
    f32 burst;
    f32 grow;
    f32 r1;
    f32 r2;
    f32 r3;
    EftAuraPrmKind *kind;

    Vec4_Set(&side, 0.0f, 0.0f, 0.0f, 1.0f);
    Vec4_Set(&drag, 0.0f, 0.0f, 0.0f, 1.0f);
    Vec4_Set(&out, 0.0f, 0.0f, 0.0f, 1.0f);
    if (f == NULL) {
        return 0;
    }
    lifeScale = aura->lifeScale;
    level = aura->level;
    burst = aura->burstLevel;
    f->objId = objId;
    f->part = part;
    f->partNode = gEftAuraCfg->flame[part].partNode;
    f->flags = EftAura_GetFlameFlags(aura, objId, second, alt);
    if (second == 0) {
        f->shape = 0;
        if (f->flags & 0x400) {
            f->kind = 1;
        } else {
            f->kind = 0;
        }
    } else {
        f->shape = 1;
        f->kind = 2;
    }
    f->life = f->time = EftAura_GetFlameLife(f->flags, burst);
    Vec4_Copy(&f->offset, offset);
    f->follow = follow;
    f->speed = EftAura_GetFlameSpeed(f->flags, objId, level, burst);
    f->power = lifeScale * burst;
    f->scale = aura->scale * level;
    f->size = EftAura_GetFlameSize(f, objId, 0);
    f->stretch = EftAura_GetFlameStretch(f, objId, 0);
    grow = f->life * gEftAuraPrm->growRatio;
    f->sizeRate = (EftAura_GetFlameSize(f, objId, 1) - f->size) / grow;
    f->stretchRate = (EftAura_GetFlameStretch(f, objId, 1) - f->stretch) / grow;
    f->end = EftAura_GetFlameEnd(f, objId);
    EftAura_GetSparkDir(&dir, objId);
    EftAura_GetFlameDragDir(aura, pos, &drag, objId, part);
    if (f->flags & 0x100) {
        f->tex = rand() % aura->texCount;
    }
    if (!(f->flags & 0x800)) {
        f->alt = 0;
        Vec3_Scale(&f->color, &aura->colorStart, RANDF() * 0.2f + 1.0f);
        f->color.w = 0.0f;
        Vec4_Sub(&d, &aura->colorEnd, &f->color);
        Vec3_Scale(&f->colorRate, &d, 1.0f / f->life);
        f->colorRate.w = EftAura_GetFlameAlpha(aura, f->flags, aura->colorStart.w) * level / gEftAuraPrm->fadeIn;
    } else {
        f->alt = 1;
        Vec3_Scale(&f->color, &aura->colorAlt[color], 1.0f);
        f->color.w = 0.0f;
        Vec4_Sub(&d, &aura->colorAlt[color], &f->color);
        Vec3_Scale(&f->colorRate, &d, 1.0f / f->life);
        f->colorRate.w = EftAura_GetFlameAlpha(aura, f->flags, aura->colorAlt[color].w) * level / gEftAuraPrm->fadeIn;
    }
    kind = &gEftAuraPrm->kind[f->kind];
    r1 = kind->side[0] + kind->side[1] * RANDF();
    r2 = kind->away[0] + kind->away[1] * RANDF();
    r3 = kind->out[0] + kind->out[1] * RANDF();
    EftAura_GetFlameSideDir(aura, &side, objId, f->power, r1, r2);
    EftAura_GetFlameOutDir(aura, &out, objId, f->part, f->power, r3);
    Vec4_Add(&f->dirTurn, &dir, &drag);
    Vec3_Normalize(&f->dirTurn, &f->dirTurn);
    Vec4_Add(&dir, &dir, &side);
    Vec4_Add(&dir, &dir, &drag);
    Vec3_Normalize(&dir, &dir);
    Vec4_Add(&f->dir, &out, &dir);
    Vec3_Normalize(&f->dir, &f->dir);
    Vec4_Copy(&f->dirStart, &f->dir);
    Vec4_Sub(&f->dirTurn, &f->dir, &f->dirTurn);
    Vec3_Normalize(&f->dirTurn, &f->dirTurn);
    Vec3_Scale(&f->move, &f->dir, gEftAuraPrm->startDist);
    Vec4_Add(&f->pos, pos, offset);
    Vec4_Add(&f->pos, &f->pos, &f->move);
    f->pos.w = 1.0f;
    f->endTime = f->life * gEftAuraPrm->endRatio;
    f->flip = rand() % 2;
    return 1;
}

/* Spawns the flames of one body part: one per `flameStep` of its length (one only with `once`), each with a
   possible second flame while fewer than 45 are alive. Returns the last primary spawn's result. */
s32 EftAura_SpawnFlames(EftAura *aura, s32 objId, s32 part, s32 once) {
    Vec4 pos;
    Vec4 nodePos;
    Vec4 delta;
    Vec4 d;
    Vec4 offset;
    Vec4 along;
    Vec4 radial;
    s32 ret = 0;
    s32 count = 1;
    f32 one = 1.0f;
    f32 scale = aura->scale;
    f32 step = gEftAuraPrm->flameStep * scale;
    f32 len;
    f32 t;
    f32 r1;
    s32 node;
    s32 alt;
    s32 i;

    Vec4_Copy(&pos, &aura->part[part]);
    Vec4_Set(&delta, 0.0f, 0.0f, 0.0f, one);
    node = gEftAuraCfg->flame[part].node;
    if (node >= 0) {
        BtlCharApi_GetNodePos(objId, node, &nodePos);
        Vec4_Sub(&d, &nodePos, &pos);
        len = sqrtf(Vec3_Dot(&d, &d));
        Vec4_Copy(&delta, &d);
        if (step < len) {
            count = len / step + one;
        }
    }
    for (i = 0; i < count; i++) {
        if (part < 2) {
            t = RANDF() * 0.4f + 0.3f;
        } else {
            t = RANDF();
        }
        Vec3_Scale(&along, &delta, t);
        r1 = RANDF() * 0.05f + 0.3f;
        EftAura_GetPartOffset(aura, &radial, objId, part, r1, RANDF() * 0.05f + 0.3f);
        Vec4_Scale(&radial, &radial, scale);
        Vec3_Copy(&offset, &radial);
        if (aura->altMask & (1U << part)) {
            aura->altMask &= ~(1U << part);
            alt = 1;
        } else {
            aura->altMask |= 1U << part;
            alt = 0;
        }
        if ((u32)(aura->burst - 1) < 2) {
            if (part < 6) {
                alt = 0;
            } else {
                alt = 1;
            }
        }
        ret = EftAura_SpawnFlame(aura, objId, part, &pos, &offset, t, alt, 0);
        if (ret != 0) {
            aura->flameCount++;
            if (aura->flameCount < 45) {
                if (part < 2 || !(rand() & 1)) {
                    if (EftAura_SpawnFlame(aura, objId, part, &pos, &offset, t, 0, 1)) {
                        aura->flameCount++;
                    }
                }
            }
        }
        if (once) {
            break;
        }
    }
    return ret;
}

/* Marks the aura active, picks the variant from the argument, and with `reset` restarts its fade-in. */
void EftAura_Start(EftAura *aura, s32 *arg, s32 reset) {
    aura->flags |= 1;
    if (arg[1] != 0) {
        aura->flags |= 0x20;
    } else {
        aura->flags |= 0x10;
    }
    if (reset) {
        aura->sizeFadeRate = 0.0f;
        aura->burst = 0;
        aura->burstLevel = 0.0f;
        aura->fadeA = 0;
        aura->alphaFadeRate = 0.0f;
        aura->fadeB = 0;
        aura->lifeScale = 1.0f;
        aura->alphaFade = 1.0f;
        aura->sizeFade = 1.0f;
        aura->fade = 1;
    }
}

/* Returns every flame of a fighter to the free list. */
void EftAura_FreeFlames(s32 objId) {
    EftAuraFlame **link = &gEftAuraPool->flameUsed;
    EftAuraFlame *p;

    while (*link != NULL) {
        p = *link;
        if (p->objId != objId) {
            link = &p->next;
        } else {
            *link = p->next;
            p->next = gEftAuraPool->flameFree;
            gEftAuraPool->flameFree = p;
        }
    }
}

/* Moves, grows, colours and fades a fighter's flames; a flame near its end spawns its successor, a finished
   one is freed. */
void EftAura_StepFlames(EftAura *aura, s32 objId) {
    Vec4 pos;
    Vec4 base;
    Vec4 end;
    Vec4 d;
    Vec4 tmp;
    EftAuraFlame **link;
    EftAuraFlame *f;
    Vec4 *move;
    Vec4 *color;
    f32 t;

    link = &gEftAuraPool->flameUsed;
    while (*link != NULL) {
        f = *link;
        if (f->objId == objId) {
            if (0.0f < f->time) {
                Vec4_Copy(&base, &aura->part[f->part]);
                Vec4_Copy(&pos, &base);
                if (gEftAuraCfg->flame[f->part].node >= 0) {
                    Vec4_Copy(&end, &aura->partEnd[f->part]);
                    Vec4_Sub(&d, &end, &base);
                    Vec3_Scale(&tmp, &d, f->follow);
                    Vec4_Add(&pos, &pos, &tmp);
                }
                Vec4_Add(&pos, &pos, &f->offset);
                t = f->time / f->life;
                EftAura_TurnFlame(f, &pos, t);
                EftAura_AccelFlame(f, t);
                Vec3_Scale(&tmp, &f->dir, f->speed + aura->burstSpeed);
                move = &f->move;
                Vec3_Add(move, move, &tmp);
                Vec4_Add(&f->pos, &pos, move);
                f->pos.w = 1.0f;
                f->size += f->sizeRate;
                f->stretch += f->stretchRate;
                if (f->size < 0.0f) {
                    f->size = 0.0f;
                }
                if (f->stretch < 0.0f) {
                    f->stretch = 0.0f;
                }
                if (!(f->flags & 8)) {
                    if (gEftAuraPrm->growRatio < 1.0f - f->time / f->life) {
                        f->sizeRate = (EftAura_GetFlameSize(f, f->objId, 2) - f->size) / f->time;
                        f->stretchRate = (EftAura_GetFlameStretch(f, f->objId, 2) - f->stretch) / f->time;
                        f->flags |= 8;
                    }
                }
                if (!(f->flags & 1)) {
                    if (gEftAuraPrm->fadeIn <= f->life - f->time) {
                        f->colorRate.w = 0.0f;
                        f->flags |= 1;
                    }
                }
                if (!(f->flags & 2)) {
                    if (f->time <= gEftAuraPrm->fadeOut) {
                        f->colorRate.w = -((f->color.w - aura->colorEnd.w) * (1.0f / (f->time + 1.0f)));
                        f->flags |= 2;
                    }
                }
                color = &f->color;
                Vec4_Add(color, color, &f->colorRate);
                Vec4_Clamp(color, color, 0.0f, 1.0f);
                f->time -= 1.0f;
                if (f->time <= f->endTime) {
                    if (!(f->flags & 0x10)) {
                        f->sizeRate = (EftAura_GetFlameSize(f, f->objId, 3) - f->size) / f->time;
                        f->stretchRate = (EftAura_GetFlameStretch(f, f->objId, 3) - f->stretch) / f->time;
                        f->flags |= 0x10;
                    }
                    if (!(f->flags & 4)) {
                        if (f->flags & 0x800) {
                            f->flags |= 4;
                        } else if (EftAura_SpawnFlames(aura, f->objId, f->part, 1)) {
                            f->flags |= 4;
                        }
                    }
                }
                link = &f->next;
            } else if (!(f->flags & 4)) {
                if (EftAura_SpawnFlames(aura, f->objId, f->part, 1)) {
                    f->flags |= 4;
                }
            } else {
                *link = f->next;
                f->next = gEftAuraPool->flameFree;
                gEftAuraPool->flameFree = f;
                aura->flameCount--;
            }
        } else {
            link = &f->next;
        }
    }
}

/* Per-frame aura state: first flames of the ten body parts, the burst and the three fades, then the flames. */
void EftAura_UpdateState(EftAura *aura, s32 objId) {
    while (aura->partsStarted < EFT_AURA_PARTS) {
        EftAura_SpawnFlames(aura, objId, aura->partsStarted, 0);
        aura->partsStarted++;
    }
    if (aura->burst != 0) {
        if (aura->burst == 1) {
            aura->burstLevel += 0.15f;
            aura->lifeScale = 1.2f;
            if (1.0f <= aura->burstLevel) {
                aura->burstLevel = 1.0f;
            }
        } else if (aura->burst == 2) {
            aura->burstLevel = 1.0f;
            aura->burstTime += 1.0f;
            if (gEftAuraCfg->burstHold <= aura->burstTime) {
                aura->burst = 3;
                aura->burstTime = 0.0f;
                aura->fadeA = 1;
                aura->fadeB = 1;
            }
        } else {
            aura->burstLevel -= 0.1f;
            if (aura->burstLevel <= 0.0f) {
                aura->burstLevel = 0.0f;
                aura->lifeScale -= 0.2f;
                if (aura->lifeScale <= 1.0f) {
                    aura->lifeScale = 1.0f;
                }
            }
            if (aura->lifeScale <= 1.0f && aura->burstLevel <= 0.0f) {
                aura->burst = 0;
            }
        }
    }
    aura->burstSpeed = gEftAuraCfg->burstSpeed * aura->burstLevel;
    if (aura->fadeA == 1) {
        if (0.0f < aura->alphaFade) {
            aura->alphaFade -= 1.0f / gEftAuraCfg->alphaFadeOutTime;
            if (aura->alphaFade <= 0.0f) {
                aura->alphaFade = 0.0f;
                aura->alphaFadeRate = 0.0f;
                aura->fadeA = 0;
            }
        }
    } else if (aura->alphaFade < 1.0f) {
        aura->alphaFadeRate += 1.0f / gEftAuraCfg->alphaFadeInTime;
        aura->alphaFade += aura->alphaFadeRate;
        if (1.0f <= aura->alphaFade) {
            aura->alphaFade = 1.0f;
            aura->alphaFadeRate = 0.0f;
        }
    }
    if (aura->fadeB == 1) {
        if (0.0f < aura->sizeFade) {
            aura->sizeFade -= 1.0f / gEftAuraCfg->sizeFadeOutTime;
            if (aura->sizeFade <= 0.0f) {
                aura->sizeFade = 0.0f;
                aura->sizeFadeRate = 0.0f;
                aura->fadeB = 0;
            }
        }
    } else if (aura->sizeFade < 1.0f) {
        aura->sizeFadeRate += 1.0f / gEftAuraCfg->sizeFadeInTime;
        aura->sizeFade += aura->sizeFadeRate;
        if (1.0f <= aura->sizeFade) {
            aura->sizeFade = 1.0f;
            aura->sizeFadeRate = 0.0f;
        }
    }
    if (aura->fade != 0) {
        f32 step;

        if (aura->fade == 1) {
            step = 1.0f / gEftAuraCfg->fadeInTime;
            aura->alpha += step;
            if (1.0f <= aura->alpha) {
                aura->alpha = 1.0f;
                aura->fade = 0;
            }
        } else {
            step = 1.0f / gEftAuraCfg->fadeOutTime;
            aura->alpha -= step;
            if (aura->alpha <= 0.0f) {
                if (aura->fade == 3) {
                    aura->flags |= 2;
                }
                aura->alpha = 0.0f;
                aura->fade = 0;
            }
        }
    }
    EftAura_StepFlames(aura, objId);
}

/* Matrix for a flame quad: third axis along `dir` leaned by the camera's view axis, translation `pos`. */
void EftAura_BuildFlameMtx(Mtx44 *out, Vec4 *dir, Vec4 *pos) {
    Vec4 v;
    Vec4 x;
    Vec4 y;
    Vec4 z;
    Vec4 view;
    f32 one = 1.0f;
    f32 zero = 0.0f;
    f32 d;
    f32 lean;

    view.x = gBtlCamView->world2view2.m[0][2];
    view.y = gBtlCamView->world2view2.m[1][2];
    view.z = gBtlCamView->world2view2.m[2][2];
    view.w = one;
    d = Vec3_Dot(&view, dir);
    if (d < zero) {
        lean = -d * gEftAuraPrm->lean;
        Vec3_Scale(&v, &view, -1.0f);
        v.w = one;
    } else {
        lean = d * gEftAuraPrm->lean;
        Vec3_Copy(&v, &view);
        v.w = one;
    }
    Vec4_Sub(&v, dir, &v);
    Vec3_Normalize(&v, &v);
    Vec3_Scale(&v, &v, lean);
    z.x = dir->x + v.x;
    z.y = dir->y + v.y;
    z.z = dir->z + v.z;
    z.w = 1.0f;
    Vec3_Normalize(&z, &z);
    Vec3_Cross(&x, &view, &z);
    Vec3_Normalize(&x, &x);
    Vec3_Cross(&y, &x, &z);
    Vec3_Normalize(&y, &y);
    Mtx_StoreIdentity(out);
    out->m[0][0] = x.x;
    out->m[1][0] = y.x;
    out->m[2][0] = z.x;
    out->m[0][1] = x.y;
    out->m[1][1] = y.y;
    out->m[2][1] = z.y;
    out->m[0][2] = x.z;
    out->m[1][2] = y.z;
    out->m[2][2] = z.z;
    out->m[3][0] = pos->x;
    out->m[3][1] = pos->y;
    out->m[3][2] = pos->z;
}


/* ======== merged from src/battle/eft_n.c ======== */


/*
 * Effect tasks, 0x1637A0..0x167E68. See include/battle/eft_aura_part2.h.
 *
 * All 47 functions are C (EftAura_DrawFlames since cleanup W1, EftBolt_Shape and EftBolt_Draw since 2026-10-08).
 * The file's .lit4 is the whole of 0x2FC97C..0x2FCAAC.
 *
 * Nothing here is simulation: no hit record, fighter, battle object or battle event is written. The fighter is
 * only read through BtlCharApi_* getters. libc rand() is drawn by the lightning (EftBolt_Shape, EftBolt_Step,
 * EftBolt_UpdateAll, EftBolt_AddFlash, EftBolt_Spawn), at a rate that depends on the aura state.
 */

#define RAND_MAX_F 2147483647.0f
#define RANDF() ((f32)rand() / RAND_MAX_F)
#define EFT_DEG(x) ((x) / 180.0f * 3.14159265f)

typedef struct EftNBattleWork {
    /* 0x0000 */ u8 unk0[0x19F0];
    /* 0x19F0 */ u64 flags; /* 0x100 = paused */
} EftNBattleWork;

/* The view being drawn (include/battle/btl_cam.h). */
typedef struct EftNView {
    /* 0x000 */ Mtx44 world2view;
    /* 0x040 */ Mtx44 world2view2;
    /* 0x080 */ u8 unk80[0xC0];
    /* 0x140 */ Mtx44 world2screen;
    /* 0x180 */ u8 unk180[0xA0];
    /* 0x220 */ Vec4 pos;
} EftNView;

/* Effect task (include/battle/eft_shot.h). */
typedef struct EftNTask {
    /* 0x00 */ u8 flags;
    /* 0x01 */ u8 unk1[0x37];
    /* 0x38 */ void *work;
} EftNTask;

/* A GS screen position as Vu0Cur_ProjectPoint writes it. */
typedef struct EftNScr {
    /* 0x0 */ u32 x;
    /* 0x4 */ u32 y;
    /* 0x8 */ s32 z;
    /* 0xC */ s32 w;
} EftNScr;

/* One vertex of the quad packets built here: colour, texture coordinates, position. */
typedef struct EftNVtx {
    /* 0x00 */ u8 r;
    /* 0x01 */ u8 g;
    /* 0x02 */ u8 b;
    /* 0x03 */ u8 a;
    /* 0x04 */ f32 q;
    /* 0x08 */ f32 s;
    /* 0x0C */ f32 t;
    /* 0x10 */ u64 x : 16;
    /* 0x12 */ u64 y : 16;
    /* 0x14 */ u64 z : 24;
    /* 0x17 */ u64 f : 8;
} EftNVtx; /* 0x18 */

/* A 4-vertex textured, gouraud-shaded triangle strip (0x90 bytes at gOtCur). */
typedef struct EftNQuadPkt {
    /* 0x00 */ u32 tag;
    /* 0x04 */ OtPrim *next;
    /* 0x08 */ u32 vif0;
    /* 0x0C */ u32 vif1;
    /* 0x10 */ u64 gif0;
    /* 0x18 */ u64 gif1;
    /* 0x20 */ u64 prim;
    /* 0x28 */ u64 tex0;
    /* 0x30 */ EftNVtx v[4];
} EftNQuadPkt; /* 0x90 */

#define gBtlCamView (*(EftNView* *)&gBtlCamView)
extern u8 gBattleProf[];
extern void *gEftAuraTaskList;
extern void *gEftAuraTaskClass[];

#define V(p) ((Vec4 *)(p))
#define gPool ((EftAuraMgr *)gEftAuraPool)
#define gData ((EftAuraData *)gEftAuraCfg)

extern s32 rand(void);
extern void *memset(void *dst, s32 c, u32 n);
extern f32 sqrtf(f32 x);
extern f32 asinf(f32 x);
extern f32 atan2f(f32 y, f32 x);
extern void Vec4_Copy(Vec4 *dst, Vec4 *src);
extern void Vec4_Add(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void Vec4_Sub(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void Vec4_Scale(Vec4 *dst, Vec4 *src, f32 s);
extern void Vec3_Scale(Vec4 *dst, Vec4 *src, f32 s);
extern f32 Vec3_Dot(Vec4 *a, Vec4 *b);
extern void Vec3_Cross(Vec4 *dst, Vec4 *a, Vec4 *b);
extern void Vec3_Normalize(Vec4 *dst, Vec4 *src);
extern f32 Vec3_LengthSq(Vec4 *v);                          /* squared length */
extern void Mtx_StoreIdentity(Mtx44 *m);
extern void Mtx_MulVec4(Vec4 *dst, Mtx44 *m, Vec4 *src);
extern void Mtx_RotateZ(Mtx44 *dst, Mtx44 *src, f32 angle); /* rotate about Z */
extern void Mtx_RotateX(Mtx44 *dst, Mtx44 *src, f32 angle); /* rotate about X */
extern void Mtx_RotateY(Mtx44 *dst, Mtx44 *src, f32 angle); /* rotate about Y */
extern void Vu0Cur_Push(void);                            /* VU0 matrix stack push */
extern void Vu0Cur_LoadMtx(Mtx44 *m);                        /* load the matrix */
extern void Vu0Cur_Pop(void);                            /* pop */
extern s32 Vu0Cur_ProjectPoint(EftNScr *out, Vec4 *pos);          /* project to GS screen coordinates; returns a value */
extern void IVec4_Set(s32 *out, s32 x, s32 y, s32 z, s32 w);
extern void EftSpr_DrawRot(u8 r, u8 g, u8 b, u8 a, f32 x, f32 y, f32 z, f32 u0, f32 v0, f32 u1, f32 v1, f32 rot,
                          s32 ofsX, s32 ofsY, s32 w, s32 h, s32 unused, u32 size, s32 ctx, s32 layer, void *tex);
extern void BtlTask_SetDead(EftNTask *task);                  /* kills the task */
extern u64 EftVram_AddTex(EftNTexEntry *tex, s32 a, s32 b);  /* TEX0 of an entry */
extern u64 EftVram_AddImage(EftNTexEntry *tex, s32 a, s32 b);  /* TEX0 without the palette */
extern u64 EftVram_AddClut(EftNTexEntry *tex);                /* palette base of an entry */
extern void EftTexSet_Load32(EftNTexSet *set, s32 *data);
extern void EftBolt_Enable(s32 objId);                       /* creates the fighter's lightning task (eft_o) */
extern void EftBolt_Disable(s32 objId);                       /* asks it to stop */
extern s32 EftGlow_IsActive(s32 objId);
extern void Dbg_ProfMark(void *prof);
extern void Dbg_ProfColor(void *prof, u32 color);
#define Battle_GetWork ((EftNBattleWork *(*)(void))Battle_GetWork)
extern s32 BtlPool_GetCurrent(void);
extern void *BtlPool_Alloc(s32 slot, s32 size);
extern void BtlPool_Free(s32 slot, void *ptr);
extern s32 *BtlScene_GetCommonEntry(s32 idx);
extern s32 BtlScene_GetCharCount(void);
extern s32 BtlScene_IsStageFlagOn(void);
extern s32 BtlStage_IsReady(void);
extern void *BtlTask_CreateChildList(EftNTask *task, s32 count, s32 workSize);
extern EftNTask *BtlTaskList_AddTail(void *list, void *cls, void *arg);
extern s32 BtlCharApi_ObjGetParamFlags0(s32 objId);
extern s32 BtlCharApi_GetAuraType(s32 objId);
extern f32 BtlCharApi_GetHeight(s32 objId);
extern void BtlCharApi_GetNodePos(s32 objId, s32 node, Vec4 *out);
extern s32 BtlCharApi_IsModelNew(s32 objId);
extern s32 BtlCharApi_IsHidden(s32 objId);
extern s32 BtlCharApi_IsCamShown(s32 objId);
extern s32 BtlCharApi_ObjTestFlagBit21(s32 objId);
extern s32 BtlCharApi_IsInTechnique(s32 objId);
extern s32 BtlCharApi_IsInRushSequence(s32 objId);

/* Alpha factor (0.5..1) of a flame corner: it falls off when the corner is close to the body, measured by the
   distance to the two nearest of seven model nodes. */
f32 EftAura_GetNodeFade(s32 objId, Vec4 *pos, Vec4 *nodes, f32 scale) {
    Vec4 d;
    f32 best[2] = { -1.0f, -1.0f };
    f32 dist;
    f32 ratio;
    f32 limit;
    s32 i;

    dist = 0.0f;
    ratio = 1.0f;
    for (i = 0; i < 7; i++) {
        Vec4_Sub(&d, pos, &nodes[i]);
        if (i == 0) {
            dist = Vec3_LengthSq(&d);
        } else {
            f32 t = Vec3_LengthSq(&d);
            if (t < dist) {
                dist = t;
            }
        }
        if (best[0] < 0.0f) {
            best[0] = dist;
        } else if (best[1] < 0.0f) {
            if (dist < best[0]) {
                best[1] = best[0];
                best[0] = dist;
            } else {
                best[1] = dist;
            }
        } else if (dist < best[0]) {
            best[1] = best[0];
            best[0] = dist;
        } else if (dist < best[1]) {
            best[1] = dist;
        }
    }
    best[0] = sqrtf(best[0]);
    best[1] = sqrtf(best[1]);
    dist = best[0] * gEftAuraPrm->unk18 + best[1] * (1.0f - gEftAuraPrm->unk18);
    limit = scale * 4.5f;
    if (dist < limit) {
        ratio = dist / limit;
        limit = ratio * ratio;
        limit *= ratio;
        ratio = limit * limit;
    }
    return ratio * 0.5f + 0.5f;
}

/* Draws the fighter's flames: each is a quad that stands along the flame's direction, leaning with the camera
   (EftAura_BuildFlameMtx), its two far corners stretched by the flame's end ratio. Corner alpha falls off near the
   body (EftAura_GetNodeFade), and the two near corners get a tenth of it. Flames with flag 0x800 are skipped.
   The quad goes into the order table slot of its average depth, in the layer given by the flame's `alt`.
   Matched in cleanup W1. What it took: Vu0Cur_ProjectPoint returns a value (declared `void`, the three off-screen
   tests came out with x / limit / reload register permuted); the order-table insert is the usual inline helper
   called with layer 1 or `f->alt` (it reads gOtZ in every arm); the loop is
   `for (link = &head; *link != NULL; link = &f->next) { f = *link; ...` (the test loads through `link`, the body
   assigns `f` from it again: gcse's PRE makes that a register copy, which gives `lw v0 / bnez v0 / move s1,v0`;
   with `f = f->next` or `(f = *link) != NULL` cse folds load and assignment into one `lw s1`); the depth is the plain sum of the
   four z; header stores in the order prim, tag, vif0, vif1, gif0, gif1, next; `sparkTex` is assigned after the
   camera vector; and the three flag tests inside the loop go through inline predicates written
   `if (flags & bit) return 1; return 0;`: their extra RTL instructions make the flame loop 449 instructions long
   in the second loop pass, one more than the 448 at which the 0.5f of the lean would be hoisted (it must stay in
   the loop: f4), and that form keeps the texture test's branch prediction. */
static inline s32 EftAura_FlameFlagInl(EftAuraFlame *f, u32 bit) {
    if (f->flags & bit) {
        return 1;
    }
    return 0;
}

static inline s32 EftAura_WorkFlagInl(EftAuraWork *aura, u32 bit) {
    if (aura->flags & bit) {
        return 1;
    }
    return 0;
}

/* Links a packet into the chain of a depth slot (clamped to 0..0xFFF); the same helper as EftOt_Add (eft_shot.c). */
static inline void EftAura_OtAdd(OtPrim *p, s32 z, s32 layer) {
    OtEntry *e;

    if (layer >= 2) {
        layer -= 2;
    }
    if (z < 0) {
        e = &gOtZ[0].layer[layer];
    } else if (z >= 0x1000) {
        e = &gOtZ[0xFFF].layer[layer];
    } else {
        e = &gOtZ[z].layer[layer];
    }
    e->tail->next = p;
    e->tail = p;
}

void EftAura_DrawFlames(EftAuraWork *aura, s32 objId, f32 alpha) {
    Mtx44 mtx;
    Vec4 *corner[2];
    Vec4 *uv[4];
    Vec4 st[4];
    Vec4 dir;
    Vec4 ndir;
    Vec4 p;
    Vec4 c;
    Vec4 camFwd;
    Vec4 away;
    u8 rgb[3];
    EftNScr scr[4];
    f32 fade[4];
    u8 al[4];
    Vec4 node[7];
    EftNTexSet *sparkTex;
    s32 clipped;
    EftAuraFlame *f;
    EftAuraFlame **link;
    EftNQuadPkt *pkt;
    s32 i;
    s32 z;
    f32 a;
    f32 w;
    f32 h;
    f32 d;

    camFwd.x = gBtlCamView->world2view2.m[0][2];
    camFwd.y = gBtlCamView->world2view2.m[1][2];
    camFwd.z = gBtlCamView->world2view2.m[2][2];
    camFwd.w = 1.0f;
    sparkTex = &gPool->grp[1].set;
    if (aura->flags & 0x80) {
        corner[0] = gData->cornerB[0];
        corner[1] = gData->cornerB[1];
    } else {
        corner[0] = gData->corner[0];
        corner[1] = gData->corner[1];
    }
    uv[0] = gData->uv[0];
    uv[1] = gData->uv[1];
    uv[2] = gData->uv[2];
    uv[3] = gData->uv[3];
    for (i = 0; i < 7; i++) {
        BtlCharApi_GetNodePos(objId, gData->fadeNode[i], &node[i]);
    }
    a = aura->alpha * aura->alphaFade * alpha;
    Dbg_ProfMark(gBattleProf);
    for (link = &gPool->flameUsed; *link != NULL; link = &f->next) {
        f = *link;
        if (f->objId != objId) {
            continue;
        }
        dir.x = f->dir.x;
        clipped = 0;
        dir.y = f->dir.y;
        dir.z = f->dir.z;
        dir.w = 1.0f;
        if (gData->part[f->part].nodeRef >= 0) {
            d = __builtin_fabsf(Vec3_Dot(&camFwd, &dir));
            Vec4_Sub(&away, &aura->partRef[f->part], &aura->pos);
            away.y = 0.0f;
            Vec3_Normalize(&away, &away);
            dir.x += away.x * d * 0.5f;
            dir.z += away.z * d * 0.5f;
        }
        Vec3_Normalize(&ndir, &dir);
        EftAura_BuildFlameMtx(&mtx, &ndir, &f->pos);
        w = f->stretch * f->scale * aura->sizeFade;
        h = f->size * f->scale * aura->sizeFade;
        for (i = 0; i < 4; i++) {
            c.x = corner[f->shape][i].x * w;
            c.y = corner[f->shape][i].y * f->scale;
            c.z = corner[f->shape][i].z * h;
            c.w = 1.0f;
            if (i >= 2) {
                c.z *= f->end;
            }
            Mtx_MulVec4(&p, &mtx, &c);
            Vu0Cur_ProjectPoint(&scr[i], &p);
            if (scr[i].x > 0xFFF0) {
                clipped = 1;
                break;
            }
            if (scr[i].y > 0xFFF0) {
                clipped = 1;
                break;
            }
            if (scr[i].z < 0) {
                clipped = 1;
                break;
            }
            Vec4_Scale(&st[i], &uv[f->shape * 2 + f->flip][i], 1.0f / (f32)scr[i].w);
            fade[i] = EftAura_GetNodeFade(f->objId, &p, node, aura->scale);
            if (i < 2) {
                fade[i] *= 0.1f;
            }
        }
        if (clipped) {
            continue;
        }
        z = (scr[0].z + scr[1].z + scr[2].z + scr[3].z) >> 10;
        rgb[0] = (u32)(f->color.x * 255.0f);
        rgb[1] = (u32)(f->color.y * 255.0f);
        rgb[2] = (u32)(f->color.z * 255.0f);
        for (i = 0; i < 4; i++) {
            al[i] = (u32)(fade[i] * (f->color.w * 255.0f * a));
        }
        if (EftAura_FlameFlagInl(f, 0x800)) {
            continue;
        }
        pkt = (EftNQuadPkt *)gOtCur;
        gOtCur = (u32 *)(pkt + 1);
        if (pkt == NULL) {
            return;
        }
        pkt->prim = 0x5C;
        pkt->tag = 0x20000008;
        pkt->vif0 = 0x10000000;
        pkt->vif1 = 0x50000008;
        pkt->gif0 = 0xE400000000008001;
        pkt->gif1 = 0x42142142142160;
        pkt->next = NULL;
        pkt->v[0].r = rgb[0];
        pkt->v[0].g = rgb[1];
        pkt->v[0].b = rgb[2];
        pkt->v[0].a = al[0];
        pkt->v[0].q = st[0].z;
        pkt->v[1].r = rgb[0];
        pkt->v[1].g = rgb[1];
        pkt->v[1].b = rgb[2];
        pkt->v[1].a = al[1];
        pkt->v[1].q = st[1].z;
        pkt->v[2].r = rgb[0];
        pkt->v[2].g = rgb[1];
        pkt->v[2].b = rgb[2];
        pkt->v[2].a = al[2];
        pkt->v[2].q = st[2].z;
        pkt->v[3].r = rgb[0];
        pkt->v[3].g = rgb[1];
        pkt->v[3].b = rgb[2];
        pkt->v[3].a = al[3];
        pkt->v[3].q = st[3].z;
        pkt->v[0].s = st[0].x;
        pkt->v[0].t = st[0].y;
        pkt->v[1].s = st[1].x;
        pkt->v[1].t = st[1].y;
        pkt->v[2].s = st[2].x;
        pkt->v[2].t = st[2].y;
        pkt->v[3].s = st[3].x;
        pkt->v[3].t = st[3].y;
        pkt->v[0].x = scr[0].x;
        pkt->v[0].y = scr[0].y;
        pkt->v[0].z = scr[0].z;
        pkt->v[0].f = 0xFF;
        pkt->v[1].x = scr[1].x;
        pkt->v[1].y = scr[1].y;
        pkt->v[1].z = scr[1].z;
        pkt->v[1].f = 0xFF;
        pkt->v[2].x = scr[2].x;
        pkt->v[2].y = scr[2].y;
        pkt->v[2].z = scr[2].z;
        pkt->v[2].f = 0xFF;
        pkt->v[3].x = scr[3].x;
        pkt->v[3].y = scr[3].y;
        pkt->v[3].z = scr[3].z;
        pkt->v[3].f = 0xFF;
        if (EftAura_FlameFlagInl(f, 0x800)) {
            pkt->tex0 = sparkTex->entry[0].tex0;
        } else if (EftAura_WorkFlagInl(aura, 0x80)) {
            pkt->tex0 = aura->tex[f->tex + 1];
        } else {
            pkt->tex0 = aura->tex[0];
        }
        if (BtlScene_IsStageFlagOn()) {
            EftAura_OtAdd((OtPrim *)pkt, z, 1);
        } else {
            EftAura_OtAdd((OtPrim *)pkt, z, f->alt);
        }
    }
    Dbg_ProfColor(gBattleProf, 0x8000FFFF);
}

/* GS TEX0 values for this frame: the aura's flame sheet(s), and once per frame every entry of the spark set. */
void EftAura_UpdateTextures(EftAuraWork *aura) {
    EftNTexSet *set = &gPool->grp[0].set;
    s32 *ready;
    u64 base;
    s32 i;

    if (aura->flags & 0x80) {
        base = EftVram_AddImage(&set->entry[8], 1, 0);
        for (i = aura->texFirst; i < aura->texFirst + aura->texCount; i++) {
            ((EftAuraWork *)((u8 *)aura + 8))->tex[i - aura->texFirst] = base | ((u64)EftVram_AddClut(&set->entry[i]) << 37);
        }
    } else {
        aura->tex[0] = EftVram_AddImage(&set->entry[0], 1, 0);
        aura->tex[0] |= (u64)EftVram_AddClut(&set->entry[aura->type]) << 37;
    }
    ready = &gPool->grp[1].ready;
    if (!*ready) {
        set = &gPool->grp[1].set;
        for (i = 0; i < set->count; i++) {
            set->entry[i].tex0 = EftVram_AddTex(&set->entry[i], 1, 0);
        }
        *ready = 1;
    }
}

/* Starts the fighter's body lightning when its character parameter flags have bit 4 or 8, else stops it. */
void EftAura_UpdateLightning(s32 objId) {
    if (BtlCharApi_ObjGetParamFlags0(objId) & 0xC) {
        EftBolt_Enable(objId);
    } else {
        EftBolt_Disable(objId);
    }
}

/* Selects the texture pair and the colours for an aura type. Types 8..10 use two textures and the second colour
   table.
   Notes for the match: `colorEnd.w = 0.0f` stands in both arms of the helper (the compiler merges the two
   tails, and with them the second Vec4_Scale call); the colour count is decided by testing the flag word
   itself and `multi` is taken from it again afterwards (the original tests the same register twice in a row);
   the three plain stores are in this order. */
/* Converts a 0..255 colour to 0..1. */
#define EftAura_ScaleColor(dst, src) Vec4_Scale(dst, src, 1.0f / 255.0f)

/* Copies the colours of the aura's type from the parameter file. */
static inline void EftAura_LoadColors(EftAuraWork *aura, s32 alt, s32 multi) {
    s32 i;
    s32 j;

    if (multi) {
        EftAura_ScaleColor(&aura->colorStart, &gData->colorB[alt][0]);
        Vec4_Scale(&aura->colorEnd, &gData->colorB[alt][0], 1.0f / 255.0f);
        aura->colorEnd.w = 0.0f;
    } else {
        EftAura_ScaleColor(&aura->colorStart, &gData->color[aura->type][0]);
        Vec4_Scale(&aura->colorEnd, &gData->color[aura->type][0], 1.0f / 255.0f);
        aura->colorEnd.w = 0.0f;
    }
    for (i = 0; i < 4; i++) {
        if (aura->flags & 0x80) {
            EftAura_ScaleColor(&aura->colorAlt[i], &gData->colorB[alt][1 + i]);
            EftAura_ScaleColor(&aura->sparkColor[i], &gData->colorB[alt][5 + i]);
            EftAura_ScaleColor(&aura->sparkColorAlt[i], &gData->colorB[alt][9 + i]);
        } else {
            if (aura->type < 8) {
                j = 0;
            } else {
                j = i;
            }
            EftAura_ScaleColor(&aura->colorAlt[i], &gData->color[aura->type][1 + j]);
            EftAura_ScaleColor(&aura->sparkColor[i], &gData->color[aura->type][5 + j]);
            EftAura_ScaleColor(&aura->sparkColorAlt[i], &gData->color[aura->type][9 + j]);
        }
    }
    alt = 0;
}

void EftAura_SetType(EftAuraWork *aura, s32 objId, s32 type, s32 paramFlags) {
    s32 alt = 0;
    s32 multi;

    aura->flags &= ~0x80;
    if ((u32)(type - 8) < 3) {
        aura->flags |= 0x80;
        aura->texFirst = 8;
        aura->texCount = 2;
    }
    switch (type) {
    case 8:
        aura->texFirst = 8;
        aura->texCount = 2;
        break;
    case 9:
        aura->texFirst = 10;
        aura->texCount = 2;
        alt = 2;
        break;
    case 10:
        aura->texFirst = 12;
        aura->texCount = 2;
        alt = 3;
        break;
    }
    aura->type = type;
    aura->unk14 = type;
    aura->colorCount = 1;
    if (aura->flags & 0x80) {
        aura->colorCount = 2;
    }
    multi = aura->flags & 0x80;
    EftAura_LoadColors(aura, alt, multi);
}

/* Reads the fighter's aura type and height: called at creation and again every frame. */
void EftAura_Setup(EftAuraWork *aura, s32 objId) {
    s32 flags = BtlCharApi_ObjGetParamFlags0(objId);

    EftAura_SetType(aura, objId, BtlCharApi_GetAuraType(objId), flags);
    aura->scale = BtlCharApi_GetHeight(objId) / 19.35f;
    if (aura->scale < 0.7f) {
        aura->scale = 0.7f;
    }
}

/* Task init: clears the work, keeps the argument block and starts the aura with a fade-in. */
void EftAuraTask_Init(EftNTask *task, EftAuraArg *arg) {
    EftAuraWork *aura = task->work;

    memset(aura, 0, sizeof(EftAuraWork));
    aura->arg = *arg;
    aura->level = 1.0f;
    aura->alpha = 0.0f;
    aura->flags |= 8;
    aura->partsStarted = 0;
    aura->flameCount = 0;
    EftAura_Setup(aura, arg->objId);
    EftAura_Start((EftAura *)aura, (s32 *)arg, 1);
}

/* Task term: returns the fighter's flames and sparks to the pools and clears its entry of the task table. */
void EftAuraTask_Term(EftNTask *task) {
    EftAuraWork *aura = task->work;
    s32 *objId;

    aura->flags = 0;
    objId = &aura->arg.objId;
    EftAura_FreeFlames(*objId);
    EftAura_FreeSparks(*objId);
    gPool->tasks[*objId] = NULL;
}

/* Task update: samples the fighter's model nodes, steps the state machines and particles (not while paused), and
   kills the task once the aura has finished. */
void EftAuraTask_Update(EftNTask *task) {
    EftAuraWork *aura = task->work;
    s32 *objId = &aura->arg.objId;
    s32 i;

    if (BtlCharApi_IsModelNew(*objId)) {
        aura->flags |= 0x40;
    }
    EftAura_Setup(aura, *objId);
    aura->flags &= ~0x40;
    if (!(Battle_GetWork()->flags & 0x100)) {
        for (i = 0; i < EFT_AURA_SPARKS; i++) {
            BtlCharApi_GetNodePos(*objId, gData->spark[i].node, &aura->sparkPos[i]);
        }
        BtlCharApi_GetNodePos(*objId, 3, &aura->pos);
        BtlCharApi_GetNodePos(*objId, 0x30, &aura->pushCentre);
        BtlCharApi_GetNodePos(*objId, 0x11, &aura->axis);
        for (i = 0; i < EFT_AURA_PARTS; i++) {
            BtlCharApi_GetNodePos(*objId, gData->part[i].node, &aura->part[i]);
        }
        for (i = 0; i < EFT_AURA_PARTS; i++) {
            BtlCharApi_GetNodePos(*objId, gData->part[i].nodeEnd, &aura->partEnd[i]);
        }
        for (i = 0; i < EFT_AURA_PARTS; i++) {
            BtlCharApi_GetNodePos(*objId, gData->part[i].nodeRef, &aura->partRef[i]);
        }
        if (aura->flags & 8) {
            Vec4_Copy(&aura->prevPos, &aura->pos);
            for (i = 0; i < EFT_AURA_PARTS; i++) {
                Vec4_Copy(&aura->partPrev[i], &aura->part[i]);
            }
            aura->flags &= ~8;
        }
        EftAura_UpdateState((EftAura *)aura, *objId);
        EftAura_UpdateSparks((EftAura *)aura, *objId, &aura->sparkState);
        Vec4_Copy(&aura->prevPos, &aura->pos);
        for (i = 0; i < EFT_AURA_PARTS; i++) {
            Vec4_Copy(&aura->partPrev[i], &aura->part[i]);
        }
        aura->frame++;
    }
    if (aura->flags & 2) {
        BtlTask_SetDead(task);
    } else {
        EftAura_UpdateTextures(aura);
    }
}

/* Task post-update: nothing (it only fetches the battle work). */
void EftAuraTask_PostUpdate(EftNTask *task) {
    if (Battle_GetWork()->flags & 0x100) {
    }
}

/* Task reset: kills the task. */
void EftAuraTask_Reset(EftNTask *task) {
    BtlTask_SetDead(task);
}

/* Task draw: the flames, unless the fighter is hidden, in a technique or rush sequence, or the aura is hidden or of
   the undrawn variant. At 30% alpha when the fighter is the transparent one in front of the camera. */
void EftAuraTask_Draw(EftNTask *task) {
    f32 alpha = 1.0f;
    EftAuraWork *aura = task->work;
    s32 *objId = &aura->arg.objId;

    if (BtlCharApi_IsHidden(*objId)) {
        return;
    }
    if (aura->flags & 4) {
        return;
    }
    if (BtlCharApi_IsCamShown(*objId) && BtlCharApi_ObjTestFlagBit21(*objId)) {
        alpha = 0.3f;
    }
    if (BtlCharApi_IsInTechnique(*objId)) {
        return;
    }
    if (BtlCharApi_IsInRushSequence(*objId)) {
        return;
    }
    if (EftGlow_IsActive(*objId)) {
        return;
    }
    if (aura->flags & 0x20) {
        return;
    }
    Vu0Cur_Push();
    Vu0Cur_LoadMtx(&gBtlCamView->world2screen);
    EftAura_DrawFlames(aura, *objId, alpha);
    Vu0Cur_Pop();
}

/* Manager init: the pool header, 50 flames and 18 sparks per character, the task table, the two texture sets
   (common entries 3 and 2), the parameter file (common entry 1) and the list the aura tasks live in. */
void EftAuraMgr_Init(EftNTask *task) {
    s32 i;

    gEftAuraPool = BtlPool_Alloc(BtlPool_GetCurrent(), sizeof(EftAuraMgr));
    memset(gEftAuraPool, 0, sizeof(EftAuraMgr));
    gPool->count = BtlScene_GetCharCount();
    gPool->flameMax = gPool->count * 50;
    gPool->sparkMax = gPool->count * 18;
    gPool->flameFree = NULL;
    gPool->flameUsed = NULL;
    gPool->flameNext = 0;
    gPool->flames = BtlPool_Alloc(BtlPool_GetCurrent(), gPool->flameMax * sizeof(EftAuraFlame));
    memset(gPool->flames, 0, gPool->flameMax * sizeof(EftAuraFlame));
    gPool->sparkFree = NULL;
    gPool->sparkUsed = NULL;
    gPool->sparkNext = 0;
    gPool->sparks = BtlPool_Alloc(BtlPool_GetCurrent(), gPool->sparkMax * sizeof(EftAuraSpark));
    memset(gPool->sparks, 0, gPool->sparkMax * sizeof(EftAuraSpark));
    gPool->tasks = BtlPool_Alloc(BtlPool_GetCurrent(), gPool->count * 4);
    memset(gPool->tasks, 0, gPool->count * 4);
    gPool->grp[0].data = BtlScene_GetCommonEntry(3);
    gPool->grp[1].data = BtlScene_GetCommonEntry(2);
    for (i = 0; i < 2; i++) {
        EftTexSet_Load32(&gPool->grp[i].set, gPool->grp[i].data);
    }
    gPool->data = BtlScene_GetCommonEntry(1);
    gEftAuraPrm = (EftAuraPrm *)(gEftAuraCfg = gPool->data); /* 0x2FEA08 is stored first */
    for (i = 0; i < gPool->count; i++) {
        gPool->tasks[i] = NULL;
    }
    gEftAuraTaskList = BtlTask_CreateChildList(task, gPool->count, sizeof(EftAuraWork));
}

/* Manager term: frees everything the init allocated. */
void EftAuraMgr_Term(void) {
    BtlPool_Free(BtlPool_GetCurrent(), gPool->tasks);
    BtlPool_Free(BtlPool_GetCurrent(), gPool->flames);
    BtlPool_Free(BtlPool_GetCurrent(), gPool->sparks);
    BtlPool_Free(BtlPool_GetCurrent(), gEftAuraPool);
    gEftAuraPool = NULL;
}

/* Manager update: marks the texture sets stale, and starts or stops each character's body lightning on the first
   frame, on the first frame after the stage became ready again, and whenever a character's model changed. */
void EftAuraMgr_Update(void) {
    s32 i;

    for (i = 0; i < 2; i++) {
        gPool->grp[i].ready = 0;
    }
    if (!BtlStage_IsReady()) {
        gPool->waitStage = 1;
    }
    if (gPool->waitStage) {
        if (BtlStage_IsReady()) {
            gPool->started = 0;
            gPool->waitStage = 0;
        }
    }
    for (i = 0; i < gPool->count; i++) {
        if (!gPool->started || BtlCharApi_IsModelNew(i)) {
            EftAura_UpdateLightning(i);
        }
    }
    gPool->started = 1;
}

/* Empty task callback. */
void EftAuraMgr_Stub(void) {
}

/* Creates the fighter's aura task, or restarts the aura of the existing one with the new variant (without a new
   fade-in). Returns 0 only when no task could be created. */
s32 EftAura_Request(s32 objId, s32 variant) {
    EftAuraArg arg;
    EftNTask *task;
    EftAuraWork *aura;

    if (gPool->tasks[objId] == NULL) {
        arg.objId = objId;
        arg.variant = variant;
        task = BtlTaskList_AddTail(gEftAuraTaskList, gEftAuraTaskClass, &arg);
        if (task != NULL) {
            gPool->tasks[objId] = task;
        } else {
            return 0;
        }
    } else {
        aura = gPool->tasks[objId]->work;
        aura->arg.variant = variant;
        EftAura_Start((EftAura *)aura, (s32 *)&aura->arg, 0);
    }
    return 1;
}

/* Variant 0: starts the final fade-out (the task ends with it) unless the aura is of the other variant. Other
   variants only clear flag 0x20. */
s32 EftAura_Stop(s32 objId, s32 variant) {
    EftNTask *task = gPool->tasks[objId];
    EftAuraWork *aura;

    if (task != NULL) {
        aura = task->work;
        if (!(aura->flags & 1)) {
            return 0;
        }
        if (aura->fade == 3) {
            return 0;
        }
        if (variant == 0) {
            if (!(aura->flags & 0x20)) {
                aura->fade = 3;
                aura->burst = 0;
                aura->burstTime = 0.0f;
            }
        } else {
            if (aura->flags & 0x20) {
                aura->flags &= ~0x20;
            }
        }
    }
    return 1;
}

/* Ends the aura at once: its task dies on its next update. */
s32 EftAura_Kill(s32 objId) {
    EftNTask *task = gPool->tasks[objId];
    EftAuraWork *aura;

    if (task != NULL) {
        aura = task->work;
        if (!(aura->flags & 1)) {
            return 0;
        }
        aura->flags = 2;
    }
    return 1;
}

/* Sets the aura strength (the fighter's ki ratio). */
void EftAura_SetLevel(s32 objId, f32 level) {
    EftNTask *task;

    if (gEftAuraPool != NULL) {
        task = gPool->tasks[objId];
        if (task != NULL) {
            ((EftAuraWork *)task->work)->level = level;
        }
    }
}

/* 1 when the fighter has an active aura that is not fully faded out. */
s32 EftAura_IsVisible(s32 objId) {
    s32 ret = 0;
    EftNTask *task;
    EftAuraWork *aura;

    if (gEftAuraPool == NULL) {
        return 0;
    }
    task = gPool->tasks[objId];
    if (task != NULL) {
        aura = task->work;
        if (aura->flags & 1) {
            if (!(aura->alpha <= 0.0f)) {
                ret = 1;
            }
        }
    }
    return ret;
}

/* 1 when the fighter's visible aura is in the rising phase of a burst. */
s32 EftAura_IsBurstRising(s32 objId) {
    s32 ret = 0;
    EftNTask *task;
    EftAuraWork *aura;

    if (gEftAuraPool == NULL) {
        return 0;
    }
    task = gPool->tasks[objId];
    if (task != NULL) {
        aura = task->work;
        if (aura->flags & 1) {
            if (!(aura->alpha <= 0.0f)) {
                ret = aura->burst == 1;
            }
        }
    }
    return ret;
}

/* Starts a burst (burst state 1). */
s32 EftAura_StartBurst(s32 objId) {
    EftNTask *task = gPool->tasks[objId];
    EftAuraWork *aura;

    if (task != NULL) {
        aura = task->work;
        if (!(aura->flags & 1)) {
            return 0;
        }
        aura->burstTime = 0.0f;
        aura->burst = 1;
    }
    return 1;
}

/* Ends a burst (burst state 4). */
s32 EftAura_EndBurst(s32 objId) {
    EftNTask *task = gPool->tasks[objId];
    EftAuraWork *aura;

    if (task != NULL) {
        aura = task->work;
        if (!(aura->flags & 1)) {
            return 0;
        }
        aura->burstTime = 0.0f;
        aura->burst = 4;
    }
    return 1;
}

/* Jumps to the peak of a burst (state 2, full burst level, flame life x 1.2). */
s32 EftAura_PeakBurst(s32 objId) {
    EftNTask *task = gPool->tasks[objId];
    EftAuraWork *aura;

    if (task != NULL) {
        aura = task->work;
        if (!(aura->flags & 1)) {
            return 0;
        }
        aura->lifeScale = 1.2f;
        aura->burstLevel = 1.0f;
        aura->burstTime = 0.0f;
        aura->burst = 2;
    }
    return 1;
}

/* Starts a fade-in, unless the final fade-out is running. */
s32 EftAura_FadeIn(s32 objId) {
    EftNTask *task = gPool->tasks[objId];
    EftAuraWork *aura;

    if (task != NULL) {
        aura = task->work;
        if (!(aura->flags & 1)) {
            return 0;
        }
        if (aura->fade == 3) {
            return 0;
        }
        aura->fade = 1;
    }
    return 1;
}

/* Starts a fade-out that keeps the task, unless the final fade-out is running. */
s32 EftAura_FadeOut(s32 objId) {
    EftNTask *task = gPool->tasks[objId];
    EftAuraWork *aura;

    if (task != NULL) {
        aura = task->work;
        if (!(aura->flags & 1)) {
            return 0;
        }
        if (aura->fade == 3) {
            return 0;
        }
        aura->fade = 2;
    }
    return 1;
}

/* What the fighter effect layer calls: 0 start, 1 stop, 2 burst start, 3 burst end, 4 burst peak, 5 fade in,
   6 fade out, 7 kill. */
s32 EftAura_Command(s32 objId, s32 cmd) {
    if (gEftAuraPool == NULL) {
        return 0;
    }
    switch (cmd) {
    case 0:
        EftAura_Request(objId, 0);
        break;
    case 1:
        EftAura_Stop(objId, 0);
        break;
    case 2:
        EftAura_StartBurst(objId);
        break;
    case 3:
        EftAura_EndBurst(objId);
        break;
    case 4:
        EftAura_PeakBurst(objId);
        break;
    case 5:
        EftAura_FadeIn(objId);
        break;
    case 6:
        EftAura_FadeOut(objId);
        break;
    case 7:
        EftAura_Kill(objId);
        break;
    }
    return 1;
}

/* Clears the hidden flag of the aura of fighter 0 or 1. */
void EftAura_Show(s32 objId) {
    EftNTask *task;
    EftAuraWork *aura;

    if (gEftAuraPool != NULL && objId < 2) {
        task = gPool->tasks[objId];
        if (task != NULL) {
            aura = task->work;
            if (aura->flags & 1) {
                aura->flags &= ~4;
            }
        }
    }
}

/* Sets the hidden flag: the flames keep updating but are not drawn. */
void EftAura_Hide(s32 objId) {
    EftNTask *task;
    EftAuraWork *aura;

    if (gEftAuraPool != NULL && objId < 2) {
        task = gPool->tasks[objId];
        if (task != NULL) {
            aura = task->work;
            if (aura->flags & 1) {
                aura->flags |= 4;
            }
        }
    }
}

/* Sets flag 0x40 (the update sets the same flag when the fighter's model changed, and clears it again). */
void EftAura_MarkModelNew(s32 objId) {
    EftNTask *task;
    EftAuraWork *aura;

    if (gEftAuraPool != NULL && objId < 2) {
        task = gPool->tasks[objId];
        if (task != NULL) {
            aura = task->work;
            if (aura->flags & 1) {
                aura->flags |= 0x40;
            }
        }
    }
}

/* Changes the type of the aura of fighter 0 or 1. Not void in the original (the call is not a tail call).
   Matches only while EftAura_SetType is defined in C above it (with only a prototype, three branches come out
   as branch-likely): that is what the ASM_STUB_BEGIN / ASM_STUB_END around its attempt is for. */
s32 EftAura_ChangeType(s32 objId, s32 type, s32 paramFlags) {
    EftNTask *task;

    if (objId < 2) {
        if (gEftAuraPool != NULL && type >= 0) {
            task = gPool->tasks[objId];
            if (task != NULL) {
                EftAura_SetType(task->work, objId, type, paramFlags);
            }
        }
    }
}

/* Colour table entry of an aura type: the second table when the parameter flags have any of 0x5E. */
EftAuraColors *EftAura_GetColors(s32 type, s32 paramFlags) {
    if (gEftAuraPool == NULL) {
        return NULL;
    }
    if (type < 0) {
        return NULL;
    }
    if (!(paramFlags & 0x5E)) {
        if (type >= 9) {
            return NULL;
        }
        return (EftAuraColors *)gData->color[type];
    }
    if (type >= 5) {
        return NULL;
    }
    return (EftAuraColors *)gData->colorB[type];
}

/* Sets the finished flag: the task dies on its next update. */
s32 EftAura_Finish(s32 objId) {
    EftNTask *task = gPool->tasks[objId];

    if (task != NULL) {
        ((EftAuraWork *)task->work)->flags |= 2;
    }
    return 1;
}

/* A second dispatcher, for the variant-1 aura: 0 request, 1 stop, 2 finish. */
s32 EftAura_CommandAlt(s32 objId, s32 cmd) {
    switch (cmd) {
    case 0:
        EftAura_Request(objId, 1);
        break;
    case 1:
        EftAura_Stop(objId, cmd);
        break;
    case 2:
        EftAura_Finish(objId);
        break;
    }
    return 1;
}

/* Takes up to `count` free joints from the pool (searching once around the ring from the last position) and chains
   them into the bolt. */
s32 EftBolt_AllocSegs(EftBolt *bolt, s32 count) {
    EftBoltSeg *prev = NULL;
    s32 first = 1;
    s32 tries;
    s32 got = 0;
    s32 idx;
    EftBoltSeg *seg;

    idx = gEftBoltPool->segNext;
    for (tries = 0; tries < gEftBoltPool->segMax; tries++) {
        seg = (EftBoltSeg *)(idx * sizeof(EftBoltSeg) + (u32)gEftBoltPool->segs);
        if (seg->flags == 0) {
            seg->next = NULL;
            if (first) {
                bolt->head = seg;
                first = 0;
            } else {
                prev->next = seg;
            }
            prev = seg;
            gEftBoltPool->segNext = idx + 1;
            if (!(gEftBoltPool->segNext < gEftBoltPool->segMax)) {
                gEftBoltPool->segNext = 0;
            }
            got++;
            if (got >= count) {
                break;
            }
        }
        idx++;
        if (idx >= gEftBoltPool->segMax) {
            idx = 0;
        }
    }
    bolt->count = got;
    bolt->shown = 0;
    return 1;
}

/* Returns the bolt's joints to the pool. */
void EftBolt_FreeSegs(EftBolt *bolt) {
    EftBoltSeg *seg;
    EftBoltSeg *next;

    if (bolt->head != NULL) {
        seg = bolt->head;
        do {
            next = seg->next;
            seg->flags = 0;
            seg = next;
        } while (next != NULL);
    }
    bolt->head = NULL;
}

/* Lays the bolt's joints out between `start` and `end` (relative to the bolt's origin): a random zigzag across the
   bolt's normal that bulges along it, and the colour, width and alpha of every joint.
   Matching notes (matched 2026-10-08):
   - r1 / r2 are used only in front of the loop; inside it `t` is the factor that lives across a rand() call and `r`
     the one that does not (one variable for both puts every factor in a saved register: twelve instead of eleven).
   - The if / else with identical arms is deliberate and holds ONLY the second random factor: the original has a
     `c.lt.s amp, 0` whose branch is gone, the rand() divisor of the first two factors is one register that is not
     hoisted (set in one block, used in the arms) and the third factor loads its own.
   - `bright = 1.0f` stands behind the calls, next to `amp = scale` (cse then keeps amp's 1.0 as the first
     register and makes bright the copy).
   - The joint widths: `n == 0 || seg->next == NULL` first (the 0.02 arm lies in front of the other one).
   - POSSIBLY A FAKE: `base = &p0;` between Vec4_Sub and Vec3_Normalize (found by the permuter). Without it the
     addresses of `dir` and `side` swap s0 / s1: a local-alloc priority race (6 references over 40 instructions
     against 3 over 10) that one more instruction between those calls in the first scheduling pass decides. The
     natural source form of that instruction is unknown. */
void EftBolt_Shape(EftBolt *bolt, EftVec start, EftVec end, f32 scale) {
    Vec4 p0;
    Vec4 p1;
    Vec4 off;
    Vec4 step;
    Vec4 side;
    Vec4 up;
    Vec4 dir;
    f32 amp = 1.0f;
    f32 bright;
    f32 r1;
    f32 r2;
    f32 t;
    f32 r;
    s32 count;
    s32 thick = 1;
    s32 n = 0;
    EftBoltSeg *seg;
    EftBoltSeg **link;
    Vec4 *base;

    r1 = (RANDF() + amp) * scale;
    r2 = (RANDF() + amp) * scale;
    count = bolt->count;
    Vec4_Sub(&dir, V(&end), V(&start));
    base = &p0; /* see the matching notes */
    Vec3_Normalize(&dir, &dir);
    Vec3_Cross(&side, &dir, &bolt->normal);
    Vec3_Cross(&up, &side, &dir);
    Vec4_Copy(base, V(&start));
    Vec4_Copy(&p1, V(&end));
    bright = 1.0f;
    amp = scale;
    p1.x += up.x * r1;
    p1.y += up.y * r1;
    p1.z += up.z * r1;
    off.x = up.x * r2;
    off.y = up.y * r2;
    off.z = up.z * r2;
    off.w = bright;
    link = &bolt->head;
    while (*link != NULL) {
        seg = *link;
        seg->flags |= 1;
        if (n == 0) {
            r = (RANDF() - RANDF()) * 0.2f * scale;
            off.x += side.x * r;
            off.y += side.y * r;
            off.z += side.z * r;
        } else {
            count--;
            r = (RANDF() * 0.5f + 0.3f) * amp;
            amp = -amp;
            if (count <= 0) {
                count = 1;
            }
            off.x += side.x * r;
            off.y += side.y * r;
            off.z += side.z * r;
            Vec4_Add(&step, base, &off);
            Vec4_Sub(&step, &p1, &step);
            step.x /= count;
            step.y /= count;
            step.z /= count;
            t = (1.0f - (f32)((count - 1) / bolt->count)) * 0.6f + 0.4f;
            t *= RANDF() * 0.2f + 0.85f;
            off.x += step.x * t;
            off.y += step.y * t;
            off.z += step.z * t;
        }
        seg->pos.x = off.x;
        seg->pos.y = off.y;
        seg->pos.z = off.z;
        seg->pos.w = 1.0f;
        t = (f32)(count / bolt->count);
        r = (RANDF() * 0.5f + 0.3f) * t * amp;
        off.x += side.x * r;
        off.y += side.y * r;
        off.z += side.z * r;
        if (amp < 0.0f) {
            r = (RANDF() * 0.4f + 0.1f) * t * amp;
        } else {
            r = (RANDF() * 0.4f + 0.1f) * t * amp;
        }
        off.x += up.x * r;
        off.y += up.y * r;
        off.z += up.z * r;
        r = (RANDF() * 0.4f + 0.6f) * scale;
        off.x += up.x * r;
        off.y += up.y * r;
        off.z += up.z * r;
        seg->r = bright * 50.0f / 255.0f;
        seg->g = bright * 30.0f / 255.0f;
        seg->b = bright * 150.0f / 255.0f;
        seg->alphaMax = 150.0f / 255.0f;
        seg->alpha = 0.0f;
        if (n == 0 || seg->next == NULL) {
            seg->widthB = seg->widthA = 0.02f;
        } else {
            thick++;
            seg->flags |= 0x20;
            if (thick == 1) {
                seg->widthA = RANDF() * 0.03f + 0.05f;
                seg->widthB = RANDF() * 0.03f + 0.05f;
            } else {
                seg->widthA = RANDF() * 0.05f + 0.11f;
                seg->widthB = RANDF() * 0.05f + 0.11f;
            }
            if (thick >= 3) {
                thick = 0;
            }
        }
        n++;
        seg->widthA *= scale * 3.0f;
        seg->widthB *= scale * 3.0f;
        link = &seg->next;
    }
}

/* One frame of a bolt's joints: reveals them (from the start, or towards the end once the bolt is retracting),
   gives each a two-frame flash, then lets it drift and fade. Returns 1 when the last joint has faded. */
s32 EftBolt_Step(EftBolt *bolt, f32 scale) {
    Vec4 d;
    EftBoltSeg *seg;
    EftBoltSeg *next;
    EftBoltSeg **link;
    s32 n = 0;
    s32 done = 0;
    s32 flags;
    f32 t;
    f32 limit;

    memset(&d, 0, sizeof(Vec4));
    link = &bolt->head;
    while (*link != NULL) {
        seg = *link;
        next = seg->next;
        if (bolt->flags & 2) {
            seg->flags |= 2;
            if (!(seg->flags & 0x10)) {
                if (!(bolt->count - bolt->shown < n)) {
                    seg->flags |= 0x10;
                }
            }
        } else {
            if (!(bolt->shown < n)) {
                seg->flags |= 2;
            }
        }
        flags = seg->flags;
        if (!(flags & 8)) {
            if (flags & 2) {
                if (!(flags & 4)) {
                    seg->alpha = seg->alphaMax * 0.35f;
                    if (flags & 0x20) {
                        if (bolt->kind < 4) {
                            seg->widthA *= 5.5f;
                            seg->widthB *= 5.5f;
                        } else {
                            seg->widthA *= 2.75f;
                            seg->widthB *= 2.75f;
                        }
                    }
                    seg->flags = flags | 4;
                } else {
                    seg->alpha = seg->alphaMax;
                    if (flags & 0x20) {
                        if (bolt->kind < 4) {
                            seg->widthA *= 0.3f;
                            seg->widthB *= 0.3f;
                        } else {
                            seg->widthA *= 0.61f;
                            seg->widthB *= 0.61f;
                        }
                    }
                    seg->flags = flags | 8;
                }
            }
        } else if (flags & 0x10) {
            seg->widthA *= 0.8f;
            seg->widthB *= 0.8f;
            if (next != NULL) {
                Vec4_Sub(&d, &next->pos, &seg->pos);
            } else {
                t = RANDF() * 0.45f;
                d.x = bolt->dir.x * t;
                d.y = bolt->dir.y * t;
                d.z = bolt->dir.z * t;
                t = RANDF() * 0.35f - RANDF() * 0.35f - 0.3f;
                d.x += bolt->normal.x * t;
                d.y += bolt->normal.y * t;
                d.z += bolt->normal.z * t;
                Vec3_Normalize(&d, &d);
                Vec4_Scale(&d, &d, scale * 1.1f);
            }
            seg->pos.x += d.x;
            seg->pos.y += d.y;
            seg->pos.z += d.z;
            seg->alpha -= seg->alphaMax * 0.25f;
            limit = seg->alphaMax * 0.1f;
            if (seg->alpha <= limit) {
                seg->alpha = limit;
                if (!(n < bolt->count - 1)) {
                    done = 1;
                }
            }
        }
        link = &seg->next;
        n++;
    }
    return done;
}

/* Draws a bolt: for each shown joint that has a successor, a camera-facing quad from this joint's width to the
   next one's, coloured per joint, sharing its far edge with the next quad.
   Matching notes (matched 2026-10-08; the rest is taken from the matched EftAura_DrawFlames: Vu0Cur_ProjectPoint
   returns a value, the order-table insert is the inline helper with layer 1, the loop is `for (link = &head;
   *link != NULL; link = &seg->next) { seg = *link; ...` with `continue`):
   - The screen positions are `s32 scr[4][4]`, not an array of EftNScr. With the 2-D array the member offset joins
     the index (`scr + (i * 16 + 8)`), all four addresses hang on one base register that gcse hoists, and the loop
     pass reduces two pointers as the original has them: &scr[i] (x at 0, w at 12, the call argument) and
     &scr[i][2] (y at -4, z at 0). With the struct the offsets join the base (`(scr + 12) + i * 16`) and only one
     pointer is reduced.
   - The packet header is in the usual order (prim, tag, vif0, vif1, gif0, gif1, next). Any order with prim behind
     the tags puts the address of col0 into a0 before the constants are stored (first scheduling pass), and the
     four 64-bit constants move from a0..a3 to a1..t0.
   - The depth is the plain sum `scr[0].z + scr[1].z + scr[2].z + scr[2].w` (the last term is the original's typo
     for scr[3].z); the uv stores are in plain field order. */
void EftBolt_Draw(EftBoltWork *work, EftBolt *bolt, f32 alpha) {
    Vec4 quad[4];
    Vec4 prev[2];
    Vec4 st[4];
    Vec4 uv[4];
    Vec4 org;
    Vec4 p;
    Vec4 cam;
    Vec4 side;
    Vec4 toCam;
    Vec4 tmp;
    s32 scr[4][4]; /* x, y, z, w of each corner as Vu0Cur_ProjectPoint writes them (EftNScr); see the notes */
    s32 col0[4];
    s32 col1[4];
    s32 clipped;
    s32 first;
    s32 n;
    s32 n1;
    EftBoltSeg *seg;
    EftBoltSeg *next;
    EftBoltSeg **link;
    EftNQuadPkt *pkt;
    s32 i;
    s32 z;

    Vec4_Copy(&cam, &gBtlCamView->pos);
    Vec4_Add(&org, &bolt->pos, &bolt->start);
    first = 1;
    n = 0;
    for (link = &bolt->head; *link != NULL; link = &seg->next) {
        seg = *link;
        next = seg->next;
        if (next == NULL) {
            continue;
        }
        if (!(seg->flags & 2)) {
            continue;
        }
        Vec4_Sub(&side, &next->pos, &seg->pos);
        clipped = 0;
        Vec4_Add(&p, &org, &seg->pos);
        p.w = 1.0f;
        Vec4_Sub(&toCam, &p, &cam);
        Vec3_Cross(&side, &side, &toCam);
        Vec3_Normalize(&side, &side);
        if (first) {
            Vec3_Scale(&tmp, &side, seg->widthB);
            Vec4_Add(&quad[0], &p, &tmp);
            quad[0].w = 1.0f;
            Vec3_Scale(&tmp, &side, -seg->widthA);
            Vec4_Add(&quad[1], &p, &tmp);
            quad[1].w = 1.0f;
        } else {
            Vec4_Copy(&quad[0], &prev[0]);
            Vec4_Copy(&quad[1], &prev[1]);
        }
        n1 = n + 1;
        Vec3_Scale(&tmp, &side, next->widthB);
        Vec4_Add(&quad[2], &p, &tmp);
        quad[2].w = 1.0f;
        first = 0;
        Vec3_Scale(&tmp, &side, -next->widthA);
        Vec4_Add(&quad[3], &p, &tmp);
        quad[3].w = 1.0f;
        Vec4_Copy(&prev[0], &quad[2]);
        Vec4_Copy(&prev[1], &quad[3]);
        uv[0].x = n % 2;
        uv[0].y = n1 % 2;
        uv[0].z = 1.0f;
        uv[0].w = 0.0f;
        uv[1].x = n % 2;
        uv[1].y = n % 2;
        uv[1].z = 1.0f;
        uv[1].w = 0.0f;
        uv[2].x = n1 % 2;
        uv[2].y = n1 % 2;
        uv[2].z = 1.0f;
        uv[2].w = 0.0f;
        uv[3].x = n1 % 2;
        uv[3].y = n % 2;
        uv[3].z = 1.0f;
        uv[3].w = 0.0f;
        for (i = 0; i < 4; i++) {
            Vu0Cur_ProjectPoint((EftNScr *)scr[i], &quad[i]);
            Vec4_Scale(&st[i], &uv[i], 1.0f / (f32)scr[i][3]);
            if ((u32)scr[i][0] > 0xFFF0) {
                clipped = 1;
                break;
            }
            if ((u32)scr[i][1] > 0xFFF0) {
                clipped = 1;
                break;
            }
            if (scr[i][2] < 0) {
                clipped = 1;
                break;
            }
        }
        if (!clipped) {
            pkt = (EftNQuadPkt *)gOtCur;
            gOtCur = (u32 *)(pkt + 1);
            if (pkt == NULL) {
                return;
            }
            pkt->prim = 0x5C;
            pkt->tag = 0x20000008;
            pkt->vif0 = 0x10000000;
            pkt->vif1 = 0x50000008;
            pkt->gif0 = 0xE400000000008001;
            pkt->gif1 = 0x42142142142160;
            pkt->next = NULL;
            IVec4_Set(col0, seg->r * 255.0f, seg->g * 255.0f, seg->b * 255.0f, seg->alpha * 255.0f * alpha);
            IVec4_Set(col1, next->r * 255.0f, next->g * 255.0f, next->b * 255.0f,
                          next->alpha * 255.0f * alpha);
            pkt->v[0].r = col0[0];
            pkt->v[0].g = col0[1];
            pkt->v[0].b = col0[2];
            pkt->v[0].a = col0[3];
            pkt->v[0].q = st[0].z;
            pkt->v[1].r = col0[0];
            pkt->v[1].g = col0[1];
            pkt->v[1].b = col0[2];
            pkt->v[1].a = col0[3];
            pkt->v[1].q = st[1].z;
            pkt->v[2].r = col1[0];
            pkt->v[2].g = col1[1];
            pkt->v[2].b = col1[2];
            pkt->v[2].a = col1[3];
            pkt->v[2].q = st[2].z;
            pkt->v[3].r = col1[0];
            pkt->v[3].g = col1[1];
            pkt->v[3].b = col1[2];
            pkt->v[3].a = col1[3];
            pkt->v[3].q = st[3].z;
            pkt->v[0].s = st[0].x;
            pkt->v[0].t = st[0].y;
            pkt->v[1].s = st[1].x;
            pkt->v[1].t = st[1].y;
            pkt->v[2].s = st[2].x;
            pkt->v[2].t = st[2].y;
            pkt->v[3].s = st[3].x;
            pkt->v[3].t = st[3].y;
            pkt->v[0].x = scr[0][0];
            pkt->v[0].y = scr[0][1];
            pkt->v[0].z = scr[0][2];
            pkt->v[0].f = 0xFF;
            pkt->v[1].x = scr[1][0];
            pkt->v[1].y = scr[1][1];
            pkt->v[1].z = scr[1][2];
            pkt->v[1].f = 0xFF;
            pkt->v[2].x = scr[2][0];
            pkt->v[2].y = scr[2][1];
            pkt->v[2].z = scr[2][2];
            pkt->v[2].f = 0xFF;
            pkt->v[3].x = scr[3][0];
            pkt->v[3].y = scr[3][1];
            pkt->v[3].z = scr[3][2];
            pkt->v[3].f = 0xFF;
            pkt->tex0 = work->tex0;
            /* the fourth term is scr[2][3], not scr[3][2]: a typo in the original */
            z = (scr[0][2] + scr[1][2] + scr[2][2] + scr[2][3]) >> 10;
            EftAura_OtAdd((OtPrim *)pkt, z, 1);
        }
        n = n1;
    }
}

/* Sets a bolt up between two model nodes: its origin, end points along the two given directions, axis and normal,
   and a chain of joints sized to its length (at most 10). Returns 0 when fewer than 3 joints were free. */
s32 EftBolt_Start(EftBoltWork *work, EftBolt *bolt, s32 objId, s32 kind, s32 nodeA, s32 nodeB, EftVec dirA,
                  EftVec dirB, f32 length, f32 follow) {
    Vec4 p0;
    Vec4 p1;
    Vec4 tmp;
    Vec4 d;
    f32 one = 1.0f;
    f32 t;
    f32 x;
    f32 y;
    f32 z;
    s32 n;

    bolt->kind = kind;
    bolt->nodeB = nodeB;
    bolt->flags |= 1;
    bolt->nodeA = nodeA;
    bolt->life = 30.0f;
    BtlCharApi_GetNodePos(objId, nodeA, &bolt->pos);
    bolt->start.x = dirA.x * length;
    bolt->start.y = dirA.y * length;
    bolt->start.z = one;
    bolt->end.x = dirB.x * length;
    bolt->end.y = dirB.y * length;
    bolt->end.z = one;
    bolt->follow = follow;
    BtlCharApi_GetNodePos(objId, bolt->nodeB, &tmp);
    Vec4_Sub(&tmp, &tmp, &bolt->pos);
    x = bolt->pos.x + tmp.x * bolt->follow;
    y = bolt->pos.y + tmp.y * bolt->follow;
    z = bolt->pos.z + tmp.z * bolt->follow;
    p0.x = x + bolt->start.x;
    p0.y = y + bolt->start.y;
    p0.z = z + bolt->start.z;
    p0.w = one;
    p1.x = x + bolt->end.x;
    p1.y = y + bolt->end.y;
    p1.z = z + bolt->end.z;
    p1.w = one;
    Vec4_Sub(&d, &p1, &p0);
    Vec3_Normalize(&bolt->dir, &d);
    bolt->normal.x = (dirA.x + dirB.x) * 0.5f;
    bolt->normal.y = (dirA.y + dirB.y) * 0.5f;
    bolt->normal.z = (dirA.z + dirB.z) * 0.5f;
    bolt->normal.w = one;
    Vec3_Normalize(&bolt->normal, &bolt->normal);
    t = work->scale * 0.6f;
    bolt->start.x -= bolt->normal.x * t;
    bolt->start.y -= bolt->normal.y * t;
    bolt->start.z -= bolt->normal.z * t;
    bolt->end.x -= bolt->normal.x * t;
    bolt->end.y -= bolt->normal.y * t;
    bolt->end.z -= bolt->normal.z * t;
    n = sqrtf(Vec3_Dot(&d, &d)) / work->scale + 3.0f;
    if (n > 10) {
        n = 10;
    }
    EftBolt_AllocSegs(bolt, n);
    if (bolt->count >= 3) {
        EftBolt_Shape(bolt, *(EftVec *)&p0, *(EftVec *)&p1, work->scale);
    } else {
        EftBolt_FreeSegs(bolt);
        return 0;
    }
    work->boltCount++;
    return 1;
}

/* Steps every live bolt of the fighter: follows its nodes, steps the joints, advances the reveal / retract counter
   and the 30-frame life, and frees the bolts that have finished. */
void EftBolt_UpdateAll(EftBoltWork *work, s32 objId) {
    Vec4 tmp;
    EftBolt *bolt;
    s32 i;

    for (i = 0; i < EFT_BOLT_COUNT; i++) {
        bolt = &work->bolt[i];
        if (bolt->flags & 1) {
            if (!(bolt->flags & 4)) {
                BtlCharApi_GetNodePos(objId, bolt->nodeA, &bolt->pos);
                BtlCharApi_GetNodePos(objId, bolt->nodeB, &tmp);
                Vec4_Sub(&tmp, &tmp, &bolt->pos);
                bolt->pos.x += tmp.x * bolt->follow;
                bolt->pos.y += tmp.y * bolt->follow;
                bolt->pos.z += tmp.z * bolt->follow;
                if (EftBolt_Step(bolt, work->scale)) {
                    bolt->flags |= 4;
                }
                if (!(bolt->flags & 2)) {
                    if (bolt->shown < bolt->count) {
                        bolt->shown += bolt->count;
                        if (!(bolt->shown < bolt->count)) {
                            bolt->shown = bolt->count;
                            bolt->flags |= 2;
                        }
                    }
                }
                if (bolt->flags & 2) {
                    if (bolt->shown > 0) {
                        bolt->shown = bolt->shown - rand() % 2 - 2;
                        if (bolt->shown < 0) {
                            bolt->shown = 0;
                        }
                    }
                }
                bolt->life -= 1.0f;
                if (bolt->life <= 0.0f) {
                    bolt->flags |= 4;
                }
            } else {
                EftBolt_FreeSegs(bolt);
                bolt->flags = 0;
                work->boltCount--;
            }
        }
    }
}

/* Draws every live bolt of the fighter. */
void EftBolt_DrawAll(EftBoltWork *work, f32 alpha) {
    EftBolt *bolt;
    s32 i;

    Vu0Cur_Push();
    Vu0Cur_LoadMtx(&gBtlCamView->world2screen);
    bolt = work->bolt;
    for (i = 0; i < EFT_BOLT_COUNT; i++, bolt++) {
        if (bolt->flags & 1) {
            EftBolt_Draw(work, bolt, alpha);
        }
    }
    Vu0Cur_Pop();
}

/* Starts a flash sprite between two model nodes. Kind 0 is the violet flash at a bolt's root (6 frames); other
   kinds are grey and last 3 frames. Returns 0 when all 20 are in use. */
s32 EftBolt_AddFlash(EftBoltWork *work, s32 kind, s32 nodeA, s32 nodeB, EftVec vel, f32 size, f32 follow) {
    EftBoltFlash *f = NULL;
    s32 i;

    for (i = 0; i < EFT_BOLT_FLASHES; i++) {
        if (work->flash[i].flags == 0) {
            f = &work->flash[i];
            break;
        }
    }
    if (f == NULL) {
        return 0;
    }
    f->flags |= 1;
    f->kind = kind;
    if (kind == 0) {
        f->life = 6.0f;
        f->r = 50.0f / 255.0f;
        f->g = 30.0f / 255.0f;
        f->a = f->b = 150.0f / 255.0f;
    } else {
        f->life = 3.0f;
        f->b = f->g = f->r = 128.0f / 255.0f;
        f->a = 96.0f / 255.0f;
    }
    f->nodeA = nodeA;
    f->nodeB = nodeB;
    Vec4_Copy(&f->vel, V(&vel));
    f->follow = follow;
    f->size = size;
    f->sizeRate = -(size * 0.2f) / f->life;
    f->alphaRate = -(f->a / (f->life + 1.0f));
    f->frame = rand() % 4;
    f->rot = RANDF() * 6.2831853f;
    if (f->rot >= 3.14159265f) {
        f->rot -= 6.2831853f;
    }
    return 1;
}

/* Steps the fighter's flash sprites: they follow their nodes, drift, shrink, fade and cycle through the four
   cells of their sheet. */
void EftBolt_UpdateFlashes(EftBoltWork *work, s32 objId) {
    Vec4 tmp;
    EftBoltFlash *f;
    s32 i;

    for (i = 0; i < EFT_BOLT_FLASHES; i++) {
        f = &work->flash[i];
        if (f->flags & 1) {
            BtlCharApi_GetNodePos(objId, f->nodeA, &f->pos);
            BtlCharApi_GetNodePos(objId, f->nodeB, &tmp);
            Vec4_Sub(&tmp, &tmp, &f->pos);
            f->pos.x += tmp.x * f->follow;
            f->pos.y += tmp.y * f->follow;
            f->pos.z += tmp.z * f->follow;
            Vec4_Add(&f->pos, &f->pos, &f->vel);
            f->a += f->alphaRate;
            f->size += f->sizeRate;
            if (f->a < 0.0f) {
                f->a = 0.0f;
            }
            f->uv.x = (f32)(f->frame % 2) * 0.5f;
            f->uv.y = (f32)(f->frame / 2) * 0.5f;
            f->uv.z = f->uv.x + 0.5f;
            f->uv.w = f->uv.y + 0.5f;
            f->frame++;
            if (f->frame >= 4) {
                f->frame = 0;
            }
            f->life -= 1.0f;
            if (f->life < 0.0f) {
                f->flags = 0;
            }
        }
    }
}

/* Draws the fighter's flash sprites as camera-facing quads. */
void EftBolt_DrawFlashes(EftBoltWork *work, f32 alpha) {
    EftNTexSet *tex = &gEftBoltPool->flashTex;
    EftBoltFlash *f;
    s32 i;

    for (i = 0; i < EFT_BOLT_FLASHES; i++) {
        f = &work->flash[i];
        if (f->flags & 1) {
            EftSpr_DrawRot((u32)(f->r * 255.0f), (u32)(f->g * 255.0f), (u32)(f->b * 255.0f),
                          (u32)(f->a * 255.0f * alpha), f->pos.x, f->pos.y, f->pos.z, f->uv.x, f->uv.y, f->uv.z,
                          f->uv.w, f->rot, 0, 0, 0x40, 0x40, 0, (u32)(f->size * 4096.0f), 0, 1,
                          &tex->entry[f->kind]);
        }
    }
}

/* Decides whether new bolts appear this frame and starts up to three, alternating between the two sides of the
   body, each with a flash at its root. How often depends on the aura: every frame while the aura is in the rising
   part of a burst or no bolt is alive, otherwise 1 frame in 12 with a visible aura, 1 in 6 while the body glow of the powered-up look is active (EftGlow_IsActive) and
   1 in 18 without; parameter flag 8 adds another 1 in 20. */
/* The node pairs as an array inside a structure: the original adds a member's offset to the table pointer
   before the index (`(pairs + 0x10) + kind * 0x14`, visible as one address computation per member read), which
   this compiler does for an array member reached through a structure, not for pointer arithmetic. */
typedef struct EftBoltPairTbl {
    /* 0x00 */ EftBoltPair p[8];
} EftBoltPairTbl;
#define BOLTPAIRS (((EftBoltPairTbl *)gEftBoltPool->pairs)->p)

/* The work block's bolts as 16-byte aligned records (EftBolt holds vectors; the Vec4 of this file's header is
   not aligned). The free-slot search reads `flags` as 12(&work->bolt[0]) in the original, the form the compiler
   uses when the record's alignment is larger than the member's. */
typedef struct EftBoltA {
    /* 0x00 */ s32 unk0[3];
    /* 0x0C */ s32 flags;
    /* 0x10 */ u8 unk10[0x80];
} __attribute__((aligned(16))) EftBoltA;
typedef struct EftBoltWorkA {
    /* 0x000 */ u8 pad[0x10];
    /* 0x010 */ EftBoltA bolt[EFT_BOLT_COUNT];
} EftBoltWorkA;
#define BOLTS_A (((EftBoltWorkA *)work)->bolt)
void EftBolt_Spawn(EftBoltWork *work, s32 objId) {
    s32 kinds[3];
    s32 pick[4];
    Mtx44 m;
    Vec4 a;
    Vec4 b;
    Vec4 dirA;
    Vec4 dirB;
    Vec4 posA;
    Vec4 posB;
    Vec4 d;
    Vec4 rot;
    Vec4 vel;
    s32 n = 0;
    s32 count = 0;
    s32 side = 0;
    s32 spawned = 0;
    EftBolt *bolt;
    EftBoltPair *pair;
    s32 *node;
    f32 t;
    s32 kind;
    s32 nodeA;
    s32 nodeB;
    s32 i;
    s32 left;
    s32 *out;
    f32 ang;
    f32 dAng;
    f32 turn;
    f32 follow;
    f32 length;
    f32 size;

    if (EftAura_IsVisible(objId)) {
        if (EftAura_IsBurstRising(objId) || work->boltCount <= 0 || rand() % 12 == 0) {
            count = 3;
        }
    } else if (EftGlow_IsActive(objId)) {
        if (work->boltCount <= 0 || rand() % 6 == 0) {
            count = 3;
        }
    } else {
        if (work->boltCount <= 0 || rand() % 18 == 0) {
            count = 3;
        }
    }
    if (work->paramFlags & 8) {
        if (rand() % 20 == 0) {
            count = 3;
        }
    }
    if (count > 0) {
        spawned = 1;
        side = work->side;
        work->side = side ^ 1;
        if (work->orderPos == 0) {
            for (i = 0; i < 4; i++) {
                pick[i] = i;
            }
            for (left = 4, out = work->order; left != 0; left--) {
                n = rand() % left;
                *out = pick[n];
                for (i = n; i < left - 1; i++) {
                    pick[i] = pick[i + 1];
                }
                out++;
            }
        }
        kinds[0] = work->order[work->orderPos];
        if (side != 0) {
            kinds[1] = 5;
            kinds[2] = 6;
        } else {
            kinds[1] = 4;
            kinds[2] = 7;
        }
        n = 0;
    }
    while (count != 0) {
        bolt = NULL;
        for (i = 0; i < EFT_BOLT_COUNT; i++) {
            if (BOLTS_A[i].flags == 0) {
                bolt = &work->bolt[i];
                break;
            }
        }
        if (bolt == NULL) {
            break;
        }
        kind = kinds[n % 3];
        n++;
        if (side == 0) {
            ang = work->angle[0];
            dAng = -(RANDF() * 0.35f + 0.05f) * 3.14159265f;
            turn = -(RANDF() * EFT_DEG(36.0f) + EFT_DEG(90.0f));
        } else {
            ang = work->angle[1];
            dAng = (RANDF() * 0.35f + 0.05f) * 3.14159265f;
            turn = RANDF() * EFT_DEG(36.0f) + EFT_DEG(90.0f);
        }
        nodeA = BOLTPAIRS[kind].nodeA;
        nodeB = BOLTPAIRS[kind].nodeB;
        follow = BOLTPAIRS[kind].follow + BOLTPAIRS[kind].followRange * RANDF();
        if (EftGlow_IsActive(objId)) {
            length = BOLTPAIRS[kind].length * 1.4f * work->scale;
        } else {
            length = BOLTPAIRS[kind].length * work->scale;
        }
        a.x = 0.0f;
        a.y = 1.0f;
        if (kind < 4) {
            a.z = RANDF() * 0.3f + -0.9f;
        } else {
            a.z = RANDF() * 0.1f + -0.6f;
        }
        a.w = 1.0f;
        ang += dAng;
        Vec3_Normalize(&a, &a);
        Mtx_StoreIdentity(&m);
        if (ang >= 3.14159265f) {
            ang -= 6.2831853f;
        } else if (ang <= -3.14159265f) {
            ang += 6.2831853f;
        }
        Mtx_RotateZ(&m, &m, ang);
        Mtx_MulVec4(&dirA, &m, &a);
        b.x = 0.0f;
        b.y = 1.0f;
        if (kind < 4) {
            b.z = 0.4f - RANDF() * 0.25f;
        } else {
            b.z = 0.5f - RANDF() * 0.2f;
        }
        b.w = 1.0f;
        ang += turn;
        Vec3_Normalize(&b, &b);
        Mtx_StoreIdentity(&m);
        if (ang >= 3.14159265f) {
            ang -= 6.2831853f;
        } else if (ang <= -3.14159265f) {
            ang += 6.2831853f;
        }
        Mtx_RotateZ(&m, &m, ang);
        Mtx_MulVec4(&dirB, &m, &b);
        BtlCharApi_GetNodePos(objId, nodeA, &posA);
        BtlCharApi_GetNodePos(objId, nodeB, &posB);
        Vec4_Sub(&d, &posA, &posB);
        Vec3_Normalize(&d, &d);
        Mtx_StoreIdentity(&m);
        rot.x = asinf(d.y);
        Mtx_RotateX(&m, &m, rot.x);
        rot.y = atan2f(d.x, d.z) + 3.14159265f;
        if (rot.y >= 3.14159265f) {
            rot.y -= 6.2831853f;
        }
        Mtx_RotateY(&m, &m, rot.y);
        Mtx_MulVec4(&dirA, &m, &dirA);
        Vec3_Normalize(&dirA, &dirA);
        Mtx_MulVec4(&dirB, &m, &dirB);
        Vec3_Normalize(&dirB, &dirB);
        if (!EftBolt_Start(work, bolt, objId, kind, nodeA, nodeB, *(EftVec *)&dirA, *(EftVec *)&dirB, length,
                           follow)) {
            break;
        }
        Vec4_Scale(&vel, &dirA, length * 0.8f);
        if (EftGlow_IsActive(objId)) {
            size = (RANDF() * 0.25f + 0.7f) * 0.42f * work->scale * 1.4f;
        } else {
            size = (RANDF() * 0.25f + 0.7f) * 0.42f * work->scale;
        }
        EftBolt_AddFlash(work, 0, nodeA, nodeB, *(EftVec *)&vel, size, follow);
        if (side == 0) {
            work->angle[0] = ang;
        } else {
            work->angle[1] = ang;
        }
        count--;
        side ^= 1;
    }
    if (spawned) {
        work->orderPos++;
        if (work->orderPos >= 4) {
            work->orderPos = 0;
        }
    }
}

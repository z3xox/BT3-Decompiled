/*
 * Collision primitives: 0x230B38..0x236190 (include/battle/col_box.h has the layouts and the contracts).
 *
 * The object really starts one function earlier: StgAabb_SetEmpty (0x230B10, at the end of stg_d.c) is the first
 * of the box helpers and owns the constant in front of this file's pool (0x2FE3F0, 1e10).
 *
 * In address order:
 *   0x230B38  axis-aligned box helpers (ColBox_*)
 *   0x230E88  collision mesh: tree node tests, polygon and vertex access, base pointer
 *   0x230F70  oriented box set-up (ColObb_Init / ColObb_Update)
 *   0x231148  box against box, ray against box, segment against box
 *   0x231590  the two walks of a mesh's box tree (collect indices / callback per leaf)
 *   0x2317C0  Col_NearEq, Col_LineLineParams
 *   0x231930  oriented box against oriented box (ColObb_Overlaps)
 *   0x231F38  ColObb_Contact (dead code: nothing calls it)
 *
 * Everything here is compiled C on the FPU; no function contains VU0 code. The vector helpers it CALLS are
 * hand-written VU0 routines (Vec4_Copy, Vec4_Sub, Vec3_Dot, Mtx_MulVec4, Vec3_Copy, Vec3_LengthSq,
 * Vec3_Normalize): see the header for what that means for a port.
 *
 * SIMULATION: all of it is pure geometry called by the stage collision (stg_collision.c), the hit detection
 * (eft_detect.c), the camera and the battle objects. No random draw, no pad or screen input, no time.
 * The only state is the mesh base pointer and the walk variables (gColWalk*), which make the two mesh walks
 * non re-entrant: a callback must not start another walk.
 */
#include "common.h"
#include "battle/col_box.h"

extern void Vec4_Copy(ColVec *dst, ColVec *src);
extern void Vec4_Sub(ColVec *dst, ColVec *a, ColVec *b);
extern void Mtx_MulVec4(ColVec *dst, void *mtx, ColVec *v);
extern f32 Vec3_Dot(ColVec *a, ColVec *b);
extern void Mtx_Copy(void *dst, void *src);      /* copies a 4x4 matrix (128-bit moves) */
extern void Vec3_Copy(ColVec *dst, ColVec *src);  /* copies x, y, z and leaves w (VU0) */
extern f32 Vec3_LengthSq(ColVec *v);                  /* squared length of x, y, z (VU0) */
extern void ColSeg_GetMidpoint(ColSeg *seg, ColVec *out);  /* middle of a segment, w = 1 */

/* Sets a box from its centre and half extents. */
void ColBox_SetCenterHalf(ColBox *box, ColVec *center, ColVec *half) {
    box->min[0] = center->x - half->x;
    box->min[1] = center->y - half->y;
    box->min[2] = center->z - half->z;
    box->max[0] = center->x + half->x;
    box->max[1] = center->y + half->y;
    box->max[2] = center->z + half->z;
}

/* Grows a box to contain a point; also sets the point's w to 1. */
void ColBox_AddPoint(ColBox *box, ColVec *pt) {
    if (pt->x < box->min[0]) {
        box->min[0] = pt->x;
    }
    if (box->max[0] < pt->x) {
        box->max[0] = pt->x;
    }
    if (pt->y < box->min[1]) {
        box->min[1] = pt->y;
    }
    if (box->max[1] < pt->y) {
        box->max[1] = pt->y;
    }
    if (pt->z < box->min[2]) {
        box->min[2] = pt->z;
    }
    if (box->max[2] < pt->z) {
        box->max[2] = pt->z;
    }
    pt->w = 1.0f;
}

/* Grows a box to contain a point, leaving the point alone. */
void ColBox_AddPointKeepW(ColBox *box, ColVec *pt) {
    if (pt->x < box->min[0]) {
        box->min[0] = pt->x;
    }
    if (box->max[0] < pt->x) {
        box->max[0] = pt->x;
    }
    if (pt->y < box->min[1]) {
        box->min[1] = pt->y;
    }
    if (box->max[1] < pt->y) {
        box->max[1] = pt->y;
    }
    if (pt->z < box->min[2]) {
        box->min[2] = pt->z;
    }
    if (box->max[2] < pt->z) {
        box->max[2] = pt->z;
    }
}

/* Centre of a box: (max + min) * 0.5, w = 1. */
void ColBox_GetCenter(ColBox *box, ColVec *out) {
    out->x = (box->max[0] + box->min[0]) * 0.5f;
    out->y = (box->max[1] + box->min[1]) * 0.5f;
    out->z = (box->max[2] + box->min[2]) * 0.5f;
    out->w = 1.0f;
}

/* Half extents of a box: (max - min) * 0.5, w = 1. */
void ColBox_GetHalf(ColBox *box, ColVec *out) {
    out->x = (box->max[0] - box->min[0]) * 0.5f;
    out->y = (box->max[1] - box->min[1]) * 0.5f;
    out->z = (box->max[2] - box->min[2]) * 0.5f;
    out->w = 1.0f;
}

/* Size of a box: max - min, w = 1. No caller. */
void ColBox_GetSize(ColBox *box, ColVec *out) {
    out->x = box->max[0] - box->min[0];
    out->y = box->max[1] - box->min[1];
    out->z = box->max[2] - box->min[2];
    out->w = 1.0f;
}

/* 1 when the box is at least 1e-6 thick on every axis (max >= min + 1e-6); an empty box gives 0. No caller. */
s32 ColBox_HasVolume(ColBox *box) {
    if (box->max[0] < box->min[0] + 0.000001f || box->max[1] < box->min[1] + 0.000001f ||
        box->max[2] < box->min[2] + 0.000001f) {
        return 0;
    }
    return 1;
}

/* 1 when the point is inside the box, faces included. */
s32 ColBox_ContainsPoint(ColBox *box, ColVec *pt) {
    s32 ret = 0;

    if (!(box->max[0] < pt->x) && !(pt->x < box->min[0]) && !(box->max[1] < pt->y) && !(pt->y < box->min[1]) &&
        !(box->max[2] < pt->z) && !(pt->z < box->min[2])) {
        ret = 1;
    }
    return ret;
}

/* 1 when the tree node is a leaf (no second child). */
s32 ColNode_IsLeaf(ColNode *node) {
    return node->right == -1;
}

/* 1 when the tree node is a leaf without a polygon. No caller. */
s32 ColNode_IsEmptyLeaf(ColNode *node) {
    s32 ret = 0;

    if (node->right == -1) {
        ret = node->left == -1;
    }
    return ret;
}

/* Copies the three corners of a polygon out of the mesh's vertex array. */
void ColMesh_GetPolyVerts(ColMesh *mesh, ColPoly *poly, ColVec *v0, ColVec *v1, ColVec *v2) {
    ColVec *vtx = (ColVec *)(gColMeshBase + mesh->vtxOfs);

    Vec4_Copy(v0, &vtx[poly->vtx[0]]);
    Vec4_Copy(v1, &vtx[poly->vtx[1]]);
    Vec4_Copy(v2, &vtx[poly->vtx[2]]);
}

/* Polygon `idx` of a mesh. */
ColPoly *ColMesh_GetPoly(ColMesh *mesh, s32 idx) {
    ColPoly *polys;

    polys = (ColPoly *)(gColMeshBase + mesh->polyOfs);
    return polys + idx;
}

/* Sets the base the mesh offsets are counted from (the stage's collision block). */
void ColMesh_SetBase(s32 *base) {
    gColMeshBase = base;
}

/* Sets up an oriented box from a centre and three half extents: the eight local corners. */
void ColObb_Init(ColObb *obb, ColVec *center, f32 hx, f32 hy, f32 hz) {
    obb->center.x = center->x;
    obb->center.y = center->y;
    obb->center.z = center->z;
    obb->center.w = 1.0f;
    obb->half.x = hx;
    obb->half.y = hy;
    obb->half.z = hz;
    obb->half.w = 1.0f;
    obb->local[0].x = hx;
    obb->local[0].y = hy;
    obb->local[0].z = hz;
    obb->local[0].w = 1.0f;
    obb->local[1].x = -hx;
    obb->local[1].y = hy;
    obb->local[1].z = hz;
    obb->local[1].w = 1.0f;
    obb->local[2].x = hx;
    obb->local[2].y = -hy;
    obb->local[2].z = hz;
    obb->local[2].w = 1.0f;
    obb->local[3].x = -hx;
    obb->local[3].y = -hy;
    obb->local[3].z = hz;
    obb->local[3].w = 1.0f;
    obb->local[4].x = hx;
    obb->local[4].y = hy;
    obb->local[4].z = -hz;
    obb->local[4].w = 1.0f;
    obb->local[5].x = -hx;
    obb->local[5].y = hy;
    obb->local[5].z = -hz;
    obb->local[5].w = 1.0f;
    obb->local[6].x = hx;
    obb->local[6].y = -hy;
    obb->local[6].z = -hz;
    obb->local[6].w = 1.0f;
    obb->local[7].x = -hx;
    obb->local[7].y = -hy;
    obb->local[7].z = -hz;
    obb->local[7].w = 1.0f;
}

/* Places an oriented box: copies the matrix, then takes the axes, the centre and the world corners from it. */
void ColObb_Update(ColObb *obb, void *mtx) {
    Mtx_Copy(obb->mtx, mtx);
    obb->axis[0].x = obb->mtx[0][0];
    obb->axis[0].y = obb->mtx[0][1];
    obb->axis[0].z = obb->mtx[0][2];
    obb->axis[1].x = obb->mtx[1][0];
    obb->axis[1].y = obb->mtx[1][1];
    obb->axis[1].z = obb->mtx[1][2];
    obb->axis[2].x = obb->mtx[2][0];
    obb->axis[2].y = obb->mtx[2][1];
    obb->axis[2].z = obb->mtx[2][2];
    obb->center.x = obb->mtx[3][0];
    obb->center.y = obb->mtx[3][1];
    obb->center.z = obb->mtx[3][2];
    Mtx_MulVec4(&obb->world[0], obb->mtx, &obb->local[0]);
    Mtx_MulVec4(&obb->world[1], obb->mtx, &obb->local[1]);
    Mtx_MulVec4(&obb->world[2], obb->mtx, &obb->local[2]);
    Mtx_MulVec4(&obb->world[3], obb->mtx, &obb->local[3]);
    Mtx_MulVec4(&obb->world[4], obb->mtx, &obb->local[4]);
    Mtx_MulVec4(&obb->world[5], obb->mtx, &obb->local[5]);
    Mtx_MulVec4(&obb->world[6], obb->mtx, &obb->local[6]);
    Mtx_MulVec4(&obb->world[7], obb->mtx, &obb->local[7]);
}

/* 1 when two boxes overlap or touch. */
s32 ColBox_Overlaps(ColBox *a, ColBox *b) {
    s32 ret = 0;

    if (!(a->min[0] > b->max[0]) && !(a->max[0] < b->min[0]) && !(a->min[1] > b->max[1]) &&
        !(a->max[1] < b->min[1]) && !(a->min[2] > b->max[2]) && !(a->max[2] < b->min[2])) {
        ret = 1;
    }
    return ret;
}

/*
 * Where a ray enters a box (Woo's method, "Fast Ray-Box Intersection"). Returns 1 and the entry point, or 1 and
 * the origin itself (w = 1) when the origin is inside or on the box. The ray is unbounded: `dir` is not
 * normalised and a hit beyond origin + dir still counts. The entry point may miss the face by 0.001 per axis.
 * A zero direction component on an axis where the origin is outside gives t = -1 for that axis (never chosen
 * over a real candidate); a ray that starts outside and points away gives 0. `out->w` is only written for the
 * inside case. Ties between candidate planes go to the lower axis (x, then y, then z).
 */
s32 ColBox_ClipRay(ColBox *box, ColRay *ray, ColVecA *out) {
    s8 side[3];
    f32 t[3];
    f32 plane[3];
    s32 inside = 1;
    s32 i;
    s32 best;

    for (i = 0; i < 3; i++) {
        if (ray->origin.v[i] < box->min[i]) {
            side[i] = 1;
            plane[i] = box->min[i];
            inside = 0;
        } else if (box->max[i] < ray->origin.v[i]) {
            side[i] = 0;
            plane[i] = box->max[i];
            inside = 0;
        } else {
            side[i] = 2;
        }
    }
    if (inside) {
        out->v[0] = ray->origin.v[0];
        out->v[1] = ray->origin.v[1];
        out->v[2] = ray->origin.v[2];
        out->v[3] = 1.0f;
        return 1;
    }
    for (i = 0; i < 3; i++) {
        if (side[i] != 2 && ray->dir.v[i] != 0.0f) {
            t[i] = (plane[i] - ray->origin.v[i]) / ray->dir.v[i];
        } else {
            t[i] = -1.0f;
        }
    }
    best = 0;
    for (i = 1; i < 3; i++) {
        if (t[best] < t[i]) {
            best = i;
        }
    }
    if (t[best] < 0.0f) {
        return 0;
    }
    for (i = 0; i < 3; i++) {
        if (best != i) {
            out->v[i] = ray->origin.v[i] + t[best] * ray->dir.v[i];
            if (out->v[i] < box->min[i] && __builtin_fabsf(box->min[i] - out->v[i]) > 0.001f) {
                return 0;
            }
            if (out->v[i] > box->max[i] && __builtin_fabsf(out->v[i] - box->max[i]) > 0.001f) {
                return 0;
            }
        } else {
            out->v[i] = plane[i];
        }
    }
    return 1;
}

/*
 * 1 when a segment touches a box: separating axes (the box's three, then the three cross products with the
 * segment), touching counts as a hit. The cross tests add 100 * FLT_MIN (0x03C80000) to the absolute direction,
 * which is nothing in practice, so a zero-length segment is simply a point-in-box test. No caller.
 */
s32 ColBox_TestSegment(ColBox *box, ColSeg *seg) {
    ColVec center;
    ColVec half;
    ColVec mid;
    ColVec dir;
    f32 ax, ay, az;

    ColBox_GetCenter(box, &center);
    ColBox_GetHalf(box, &half);
    ColSeg_GetMidpoint(seg, &mid);
    Vec4_Sub(&dir, &seg->b, &mid);
    Vec4_Sub(&mid, &mid, &center);
    ax = __builtin_fabsf(dir.x);
    if (__builtin_fabsf(mid.x) > half.x + ax) {
        return 0;
    }
    ay = __builtin_fabsf(dir.y);
    if (__builtin_fabsf(mid.y) > half.y + ay) {
        return 0;
    }
    az = __builtin_fabsf(dir.z);
    if (__builtin_fabsf(mid.z) > half.z + az) {
        return 0;
    }
    ax += 1.175494351e-36f;
    ay += 1.175494351e-36f;
    az += 1.175494351e-36f;
    if (__builtin_fabsf(mid.y * dir.z - mid.z * dir.y) > half.y * az + half.z * ay) {
        return 0;
    }
    if (__builtin_fabsf(mid.z * dir.x - mid.x * dir.z) > half.x * az + half.z * ax) {
        return 0;
    }
    if (__builtin_fabsf(mid.x * dir.y - mid.y * dir.x) > half.x * ay + half.y * ax) {
        return 0;
    }
    return 1;
}

/* 1 when a tree node's box overlaps a box. */
s32 ColNode_TestBox(ColNode *node, ColBox *box) {
    return ColBox_Overlaps(box, &node->box) != 0;
}

/* Tree walk of ColMesh_CollectBox: appends the polygon index of every leaf whose box overlaps `box`. */
void ColMesh_CollectNode(ColNode *nodes, ColBox *box, s32 idx) {
    ColNode *node = &nodes[idx];

    if (ColNode_TestBox(node, box)) {
        if (ColNode_IsLeaf(node)) {
            if (gColWalkBuf != NULL) {
                if (gColWalkCount >= gColWalkMax) {
                    return;
                }
                gColWalkBuf[gColWalkCount] = node->left;
            }
            gColWalkCount++;
        } else {
            ColMesh_CollectNode(nodes, box, node->left);
            ColMesh_CollectNode(nodes, box, node->right);
        }
    }
}

/* Collects the polygon indices of a mesh whose leaf boxes overlap `box` (at most `max`); returns the count. */
s32 ColMesh_CollectBox(s32 max, u16 *buf, ColMesh *mesh, ColBox *box) {
    ColNode *nodes;

    if (mesh->nodeCount <= 0) {
        return 0;
    }
    nodes = (ColNode *)(gColMeshBase + mesh->nodeOfs);
    gColWalkMax = max;
    gColWalkBuf = buf;
    gColWalkCount = 0;
    ColMesh_CollectNode(nodes, box, 0);
    gColWalkBuf = NULL;
    return gColWalkCount;
}

/* Tree walk of ColMesh_WalkBox: calls the callback for every leaf whose box overlaps `box`. */
void ColMesh_WalkNode(ColNode *nodes, ColBox *box, s32 idx) {
    ColNode *node = &nodes[idx];

    if (ColNode_TestBox(node, box)) {
        if (ColNode_IsLeaf(node)) {
            s32 ret = gColWalkCb(node, gColWalkCtx);

            gColWalkCount |= ret;
            return;
        }
        ColMesh_WalkNode(nodes, box, node->left);
        ColMesh_WalkNode(nodes, box, node->right);
    }
}

/* Calls `cb(leaf, ctx)` for every leaf of a mesh whose box overlaps `box`; returns the OR of the results. */
s32 ColMesh_WalkBox(ColMesh *mesh, ColBox *box, void *ctx, ColMeshCb cb) {
    ColNode *nodes;

    if (mesh->nodeCount <= 0) {
        return 0;
    }
    if (cb == NULL) {
        return 0;
    }
    nodes = (ColNode *)(gColMeshBase + mesh->nodeOfs);
    gColWalkBox = box;
    gColWalkCb = cb;
    gColWalkCtx = ctx;
    gColWalkCount = 0;
    ColMesh_WalkNode(nodes, box, 0);
    return gColWalkCount;
}

/* 1 when |a - b| <= eps (0 when either is not a number). */
s32 Col_NearEq(f32 a, f32 b, f32 eps) {
    if (__builtin_fabsf(a - b) <= eps) {
        return 1;
    }
    return 0;
}

/*
 * Parameters of the closest points of two lines p1 + s d1 and p2 + t d2 with unit directions.
 * As built the two guards are inverted: a direction whose squared length IS within 1e-6 of 1 gives s = t = 0,
 * so the formula only runs for directions that are not unit length. Parallel lines (d1.d2 squared within 1e-6 of
 * 1, or larger) give 0, 0 too. Only called by ColObb_Contact.
 */
void Col_LineLineParams(f32 *s, f32 *t, ColVec *p1, ColVec *d1, ColVec *p2, ColVec *d2) {
    ColVec diff;
    f32 a;
    f32 b;
    f32 c;
    f32 den;
    f32 inv;

    if (Col_NearEq(Vec3_LengthSq(d1), 1.0f, 0.000001f)) {
        *s = 0.0f;
        *t = 0.0f;
        return;
    }
    if (Col_NearEq(Vec3_LengthSq(d2), 1.0f, 0.000001f)) {
        *s = 0.0f;
        *t = 0.0f;
        return;
    }
    Vec4_Sub(&diff, p2, p1);
    a = Vec3_Dot(&diff, d1);
    b = Vec3_Dot(&diff, d2);
    c = Vec3_Dot(d1, d2);
    den = c * c - 1.0f;
    if (den < -0.000001f) {
        inv = 1.0f / den;
        *s = (c * b - a) * inv;
        *t = (b - c * a) * inv;
    } else {
        *s = 0.0f;
        *t = 0.0f;
    }
}

/* 1 when two oriented boxes overlap or touch (the fifteen separating axes, no tolerance). */
s32 ColObb_Overlaps(ColObb *a, ColObb *b) {
    ColVec ha;
    ColVec hb;
    ColVec diff;
    ColMat3 R;
    ColMat3 absR;
    ColVec d;
    f32 t;
    f32 rb;
    f32 sum;
    f32 ra;

    Vec3_Copy(&ha, &a->half);
    Vec3_Copy(&hb, &b->half);
    Vec4_Sub(&diff, &a->center, &b->center);
    R.m[0][0] = Vec3_Dot(&a->axis[0], &b->axis[0]);
    R.m[0][1] = Vec3_Dot(&a->axis[0], &b->axis[1]);
    R.m[0][2] = Vec3_Dot(&a->axis[0], &b->axis[2]);
    d.x = Vec3_Dot(&a->axis[0], &diff);
    absR.m[0][0] = __builtin_fabsf(R.m[0][0]);
    absR.m[0][1] = __builtin_fabsf(R.m[0][1]);
    absR.m[0][2] = __builtin_fabsf(R.m[0][2]);
    t = __builtin_fabsf(d.x);
    rb = hb.x * absR.m[0][0] + hb.y * absR.m[0][1] + hb.z * absR.m[0][2];
    sum = ha.x + rb;
    if (t > sum) {
        return 0;
    }
    R.m[1][0] = Vec3_Dot(&a->axis[1], &b->axis[0]);
    R.m[1][1] = Vec3_Dot(&a->axis[1], &b->axis[1]);
    R.m[1][2] = Vec3_Dot(&a->axis[1], &b->axis[2]);
    d.y = Vec3_Dot(&a->axis[1], &diff);
    absR.m[1][0] = __builtin_fabsf(R.m[1][0]);
    absR.m[1][1] = __builtin_fabsf(R.m[1][1]);
    absR.m[1][2] = __builtin_fabsf(R.m[1][2]);
    t = __builtin_fabsf(d.y);
    rb = hb.x * absR.m[1][0] + hb.y * absR.m[1][1] + hb.z * absR.m[1][2];
    sum = ha.y + rb;
    if (t > sum) {
        return 0;
    }
    R.m[2][0] = Vec3_Dot(&a->axis[2], &b->axis[0]);
    R.m[2][1] = Vec3_Dot(&a->axis[2], &b->axis[1]);
    R.m[2][2] = Vec3_Dot(&a->axis[2], &b->axis[2]);
    d.z = Vec3_Dot(&a->axis[2], &diff);
    absR.m[2][0] = __builtin_fabsf(R.m[2][0]);
    absR.m[2][1] = __builtin_fabsf(R.m[2][1]);
    absR.m[2][2] = __builtin_fabsf(R.m[2][2]);
    t = __builtin_fabsf(d.z);
    rb = hb.x * absR.m[2][0] + hb.y * absR.m[2][1] + hb.z * absR.m[2][2];
    sum = ha.z + rb;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(Vec3_Dot(&b->axis[0], &diff));
    ra = ha.x * absR.m[0][0] + ha.y * absR.m[1][0] + ha.z * absR.m[2][0];
    sum = ra + hb.x;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(Vec3_Dot(&b->axis[1], &diff));
    ra = ha.x * absR.m[0][1] + ha.y * absR.m[1][1] + ha.z * absR.m[2][1];
    sum = ra + hb.y;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(Vec3_Dot(&b->axis[2], &diff));
    ra = ha.x * absR.m[0][2] + ha.y * absR.m[1][2] + ha.z * absR.m[2][2];
    sum = ra + hb.z;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(d.z * R.m[1][0] - d.y * R.m[2][0]);
    ra = ha.y * absR.m[2][0] + ha.z * absR.m[1][0];
    rb = hb.y * absR.m[0][2] + hb.z * absR.m[0][1];
    sum = ra + rb;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(d.z * R.m[1][1] - d.y * R.m[2][1]);
    ra = ha.y * absR.m[2][1] + ha.z * absR.m[1][1];
    rb = hb.x * absR.m[0][2] + hb.z * absR.m[0][0];
    sum = ra + rb;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(d.z * R.m[1][2] - d.y * R.m[2][2]);
    ra = ha.y * absR.m[2][2] + ha.z * absR.m[1][2];
    rb = hb.x * absR.m[0][1] + hb.y * absR.m[0][0];
    sum = ra + rb;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(d.x * R.m[2][0] - d.z * R.m[0][0]);
    ra = ha.x * absR.m[2][0] + ha.z * absR.m[0][0];
    rb = hb.y * absR.m[1][2] + hb.z * absR.m[1][1];
    sum = ra + rb;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(d.x * R.m[2][1] - d.z * R.m[0][1]);
    ra = ha.x * absR.m[2][1] + ha.z * absR.m[0][1];
    rb = hb.x * absR.m[1][2] + hb.z * absR.m[1][0];
    sum = ra + rb;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(d.x * R.m[2][2] - d.z * R.m[0][2]);
    ra = ha.x * absR.m[2][2] + ha.z * absR.m[0][2];
    rb = hb.x * absR.m[1][1] + hb.y * absR.m[1][0];
    sum = ra + rb;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(d.y * R.m[0][0] - d.x * R.m[1][0]);
    ra = ha.x * absR.m[1][0] + ha.y * absR.m[0][0];
    rb = hb.y * absR.m[2][2] + hb.z * absR.m[2][1];
    sum = ra + rb;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(d.y * R.m[0][1] - d.x * R.m[1][1]);
    ra = ha.x * absR.m[1][1] + ha.y * absR.m[0][1];
    rb = hb.x * absR.m[2][2] + hb.z * absR.m[2][0];
    sum = ra + rb;
    if (t > sum) {
        return 0;
    }
    t = __builtin_fabsf(d.y * R.m[0][2] - d.x * R.m[1][2]);
    ra = ha.x * absR.m[1][2] + ha.y * absR.m[0][2];
    rb = hb.x * absR.m[2][1] + hb.y * absR.m[2][0];
    sum = ra + rb;
    if (t > sum) {
        return 0;
    }
    return 1;
}

/*
 * Contact between two oriented boxes: least-penetration axis (code 0..2 a face of a, 3..5 a face of b, 6 + 3 i + j
 * the cross product of a.axis[i] and b.axis[j]), then a contact normal and point. 0 when the boxes are apart; the
 * smallest overlap is left in *depth. NO CALLER anywhere in the game (dead code), and it does not do what it was
 * meant to. Everything below is as built (verified by the match):
 *   - the nine edge-edge axes are only considered when two ways of computing the same quantity DISAGREE by more
 *     than 1e-6 (an inverted check, like the one in Col_LineLineParams), and the first of them (code 6) never is:
 *     its length is taken from row 0 of R instead of column 0 and then compared with itself;
 *   - Col_LineLineParams returns 0, 0 for unit axes, so an edge contact is just the chosen corner of box a;
 *   - edge case 4 (code 10) tests its helper vector for box a before filling it, so the test sees the zeros of
 *     the memset and always takes the first branch (0 > -1e-5); edge case 6 (code 12) leaves one component of
 *     that vector unscaled; edge case 0 alone does not clear the vector first;
 *   - if no axis is ever kept (only possible with NaNs), code stays -1 and the face-of-a branch indexes R and dA
 *     with -1.
 * The fifteen axes and the nine edge cases are written out one by one, as in the original (the compiler output
 * has a block per axis and a nine-way jump table at 0x2F2170 whose cases differ in exactly these details).
 * What the match needs: Col_NearEq known to be `const` (see the header); the axes indexed as component arrays in
 * the face cases (ColObbV); one function-level 12-byte scratch (`tmp`) that holds the three corner signs of the
 * face cases and the helper vector of the edge cases at the same stack slot; `w` local to each half of a case.
 */
extern f32 D_002FEBA8[];     /* 0x2FEBA8: FLT_MAX (initialised data, reached with lui / lwc1, so not a small extern) */
extern f32 sqrtf(f32 x);
extern void *memset(void *dst, s32 c, u32 n);
extern void Vec3_Normalize(ColVec *dst, ColVec *src);

/* The same box with its vectors as component arrays: the face cases index the axes through this view. */
typedef struct ColObbV {
    /* 0x00 */ ColVecA center;
    /* 0x10 */ ColVecA half;
    /* 0x20 */ ColVecA axis[3];
} ColObbV;

s32 ColObb_Contact(ColObb *a, ColObb *b, ColVec *normal, ColVec *point, f32 *depth) {
    ColVec diff;
    ColMat3 R;
    ColMat3 absR;
    ColVecA dA;
    ColVecA dB;
    ColVecA pA;
    ColVecA pB;
    ColVec n;
    s32 sA[2];
    s32 sB[2];
    union {
        s32 i[3];
        f32 f[3];
    } tmp;
    f32 s;
    f32 u;
    f32 t;
    f32 ra;
    f32 rb;
    f32 sum;
    f32 d;
    f32 len;
    s32 code = -1;
    s32 eq;

    *depth = D_002FEBA8[0];
    Vec4_Sub(&diff, &b->center, &a->center);

    R.m[0][0] = Vec3_Dot(&a->axis[0], &b->axis[0]);
    R.m[0][1] = Vec3_Dot(&a->axis[0], &b->axis[1]);
    R.m[0][2] = Vec3_Dot(&a->axis[0], &b->axis[2]);
    dA.v[0] = Vec3_Dot(&a->axis[0], &diff);
    absR.m[0][0] = __builtin_fabsf(R.m[0][0]);
    absR.m[0][1] = __builtin_fabsf(R.m[0][1]);
    absR.m[0][2] = __builtin_fabsf(R.m[0][2]);
    t = __builtin_fabsf(dA.v[0]);
    rb = b->half.x * absR.m[0][0] + b->half.y * absR.m[0][1] + b->half.z * absR.m[0][2];
    sum = a->half.x + rb;
    d = sum - t;
    if (d < 0.0f) {
        return 0;
    }
    if (d < *depth) {
        *depth = d;
        code = 0;
    }
    R.m[1][0] = Vec3_Dot(&a->axis[1], &b->axis[0]);
    R.m[1][1] = Vec3_Dot(&a->axis[1], &b->axis[1]);
    R.m[1][2] = Vec3_Dot(&a->axis[1], &b->axis[2]);
    dA.v[1] = Vec3_Dot(&a->axis[1], &diff);
    absR.m[1][0] = __builtin_fabsf(R.m[1][0]);
    absR.m[1][1] = __builtin_fabsf(R.m[1][1]);
    absR.m[1][2] = __builtin_fabsf(R.m[1][2]);
    t = __builtin_fabsf(dA.v[1]);
    rb = b->half.x * absR.m[1][0] + b->half.y * absR.m[1][1] + b->half.z * absR.m[1][2];
    sum = a->half.y + rb;
    d = sum - t;
    if (d < 0.0f) {
        return 0;
    }
    if (d < *depth) {
        *depth = d;
        code = 1;
    }
    R.m[2][0] = Vec3_Dot(&a->axis[2], &b->axis[0]);
    R.m[2][1] = Vec3_Dot(&a->axis[2], &b->axis[1]);
    R.m[2][2] = Vec3_Dot(&a->axis[2], &b->axis[2]);
    dA.v[2] = Vec3_Dot(&a->axis[2], &diff);
    absR.m[2][0] = __builtin_fabsf(R.m[2][0]);
    absR.m[2][1] = __builtin_fabsf(R.m[2][1]);
    absR.m[2][2] = __builtin_fabsf(R.m[2][2]);
    t = __builtin_fabsf(dA.v[2]);
    rb = b->half.x * absR.m[2][0] + b->half.y * absR.m[2][1] + b->half.z * absR.m[2][2];
    sum = a->half.z + rb;
    d = sum - t;
    if (d < 0.0f) {
        return 0;
    }
    if (d < *depth) {
        *depth = d;
        code = 2;
    }
    dB.v[0] = Vec3_Dot(&b->axis[0], &diff);
    t = __builtin_fabsf(dB.v[0]);
    ra = a->half.x * absR.m[0][0] + a->half.y * absR.m[1][0] + a->half.z * absR.m[2][0];
    sum = ra + b->half.x;
    d = sum - t;
    if (d < 0.0f) {
        return 0;
    }
    if (d < *depth) {
        *depth = d;
        code = 3;
    }
    dB.v[1] = Vec3_Dot(&b->axis[1], &diff);
    t = __builtin_fabsf(dB.v[1]);
    ra = a->half.x * absR.m[0][1] + a->half.y * absR.m[1][1] + a->half.z * absR.m[2][1];
    sum = ra + b->half.y;
    d = sum - t;
    if (d < 0.0f) {
        return 0;
    }
    if (d < *depth) {
        *depth = d;
        code = 4;
    }
    dB.v[2] = Vec3_Dot(&b->axis[2], &diff);
    t = __builtin_fabsf(dB.v[2]);
    ra = a->half.x * absR.m[0][2] + a->half.y * absR.m[1][2] + a->half.z * absR.m[2][2];
    sum = ra + b->half.z;
    d = sum - t;
    if (d < 0.0f) {
        return 0;
    }
    if (d < *depth) {
        *depth = d;
        code = 5;
    }
    /* code 6: a.axis[0] x b.axis[0] */
    t = __builtin_fabsf(dA.v[2] * R.m[1][0] - dA.v[1] * R.m[2][0]);
    len = sqrtf(R.m[0][2] * R.m[0][2] + R.m[0][1] * R.m[0][1]);
    eq = Col_NearEq(__builtin_fabsf(dB.v[1] * R.m[0][2] - dB.v[2] * R.m[0][1]), t, 0.000001f);
    eq |= Col_NearEq(len, len, 0.000001f);
    if (eq == 0) {
        if (len == 0.0f) {
            len = 0.000001f;
        }
        ra = a->half.y * absR.m[2][0] + a->half.z * absR.m[1][0];
        rb = b->half.y * absR.m[0][2] + b->half.z * absR.m[0][1];
        sum = ra + rb;
        d = (sum - t) / len;
        if (d != 0.0f) {
            if (d < 0.0f) {
                return 0;
            }
            if (d < *depth) {
                *depth = d;
                code = 6;
            }
        }
    }
    /* code 7: a.axis[0] x b.axis[1] */
    t = __builtin_fabsf(dA.v[2] * R.m[1][1] - dA.v[1] * R.m[2][1]);
    len = sqrtf(R.m[2][1] * R.m[2][1] + R.m[1][1] * R.m[1][1]);
    eq = Col_NearEq(__builtin_fabsf(dB.v[0] * R.m[0][2] - dB.v[2] * R.m[0][0]), t, 0.000001f);
    eq |= Col_NearEq(len, sqrtf(R.m[0][2] * R.m[0][2] + R.m[0][0] * R.m[0][0]), 0.000001f);
    if (eq == 0) {
        ra = a->half.y * absR.m[2][1] + a->half.z * absR.m[1][1];
        rb = b->half.x * absR.m[0][2] + b->half.z * absR.m[0][0];
        sum = ra + rb;
        if (len == 0.0f) {
            len = 0.000001f;
        }
        d = (sum - t) / len;
        if (d != 0.0f) {
            if (d < 0.0f) {
                return 0;
            }
            if (d < *depth) {
                *depth = d;
                code = 7;
            }
        }
    }
    /* code 8: a.axis[0] x b.axis[2] */
    t = __builtin_fabsf(dA.v[2] * R.m[1][2] - dA.v[1] * R.m[2][2]);
    len = sqrtf(R.m[2][2] * R.m[2][2] + R.m[1][2] * R.m[1][2]);
    eq = Col_NearEq(__builtin_fabsf(dB.v[0] * R.m[0][1] - dB.v[1] * R.m[0][0]), t, 0.000001f);
    eq |= Col_NearEq(len, sqrtf(R.m[0][1] * R.m[0][1] + R.m[0][0] * R.m[0][0]), 0.000001f);
    if (eq == 0) {
        ra = a->half.y * absR.m[2][2] + a->half.z * absR.m[1][2];
        rb = b->half.x * absR.m[0][1] + b->half.y * absR.m[0][0];
        sum = ra + rb;
        if (len == 0.0f) {
            len = 0.000001f;
        }
        d = (sum - t) / len;
        if (d != 0.0f) {
            if (d < 0.0f) {
                return 0;
            }
            if (d < *depth) {
                *depth = d;
                code = 8;
            }
        }
    }
    /* code 9: a.axis[1] x b.axis[0] */
    t = __builtin_fabsf(dA.v[0] * R.m[2][0] - dA.v[2] * R.m[0][0]);
    len = sqrtf(R.m[2][0] * R.m[2][0] + R.m[0][0] * R.m[0][0]);
    eq = Col_NearEq(__builtin_fabsf(dB.v[1] * R.m[1][2] - dB.v[2] * R.m[1][1]), t, 0.000001f);
    eq |= Col_NearEq(len, sqrtf(R.m[1][2] * R.m[1][2] + R.m[1][1] * R.m[1][1]), 0.000001f);
    if (eq == 0) {
        ra = a->half.x * absR.m[2][0] + a->half.z * absR.m[0][0];
        rb = b->half.y * absR.m[1][2] + b->half.z * absR.m[1][1];
        sum = ra + rb;
        if (len == 0.0f) {
            len = 0.000001f;
        }
        d = (sum - t) / len;
        if (d != 0.0f) {
            if (d < 0.0f) {
                return 0;
            }
            if (d < *depth) {
                *depth = d;
                code = 9;
            }
        }
    }
    /* code 10: a.axis[1] x b.axis[1] */
    t = __builtin_fabsf(dA.v[0] * R.m[2][1] - dA.v[2] * R.m[0][1]);
    len = sqrtf(R.m[2][1] * R.m[2][1] + R.m[0][1] * R.m[0][1]);
    eq = Col_NearEq(__builtin_fabsf(dB.v[0] * R.m[1][2] - dB.v[2] * R.m[1][0]), t, 0.000001f);
    eq |= Col_NearEq(len, sqrtf(R.m[1][2] * R.m[1][2] + R.m[1][0] * R.m[1][0]), 0.000001f);
    if (eq == 0) {
        ra = a->half.x * absR.m[2][1] + a->half.z * absR.m[0][1];
        rb = b->half.x * absR.m[1][2] + b->half.z * absR.m[1][0];
        sum = ra + rb;
        if (len == 0.0f) {
            len = 0.000001f;
        }
        d = (sum - t) / len;
        if (d != 0.0f) {
            if (d < 0.0f) {
                return 0;
            }
            if (d < *depth) {
                *depth = d;
                code = 10;
            }
        }
    }
    /* code 11: a.axis[1] x b.axis[2] */
    t = __builtin_fabsf(dA.v[0] * R.m[2][2] - dA.v[2] * R.m[0][2]);
    len = sqrtf(R.m[2][2] * R.m[2][2] + R.m[0][2] * R.m[0][2]);
    eq = Col_NearEq(__builtin_fabsf(dB.v[0] * R.m[1][1] - dB.v[1] * R.m[1][0]), t, 0.000001f);
    eq |= Col_NearEq(len, sqrtf(R.m[1][1] * R.m[1][1] + R.m[1][0] * R.m[1][0]), 0.000001f);
    if (eq == 0) {
        ra = a->half.x * absR.m[2][2] + a->half.z * absR.m[0][2];
        rb = b->half.x * absR.m[1][1] + b->half.y * absR.m[1][0];
        sum = ra + rb;
        if (len == 0.0f) {
            len = 0.000001f;
        }
        d = (sum - t) / len;
        if (d != 0.0f) {
            if (d < 0.0f) {
                return 0;
            }
            if (d < *depth) {
                *depth = d;
                code = 11;
            }
        }
    }
    /* code 12: a.axis[2] x b.axis[0] */
    t = __builtin_fabsf(dA.v[1] * R.m[0][0] - dA.v[0] * R.m[1][0]);
    len = sqrtf(R.m[1][0] * R.m[1][0] + R.m[0][0] * R.m[0][0]);
    eq = Col_NearEq(__builtin_fabsf(dB.v[1] * R.m[2][2] - dB.v[2] * R.m[2][1]), t, 0.000001f);
    eq |= Col_NearEq(len, sqrtf(R.m[2][2] * R.m[2][2] + R.m[2][1] * R.m[2][1]), 0.000001f);
    if (eq == 0) {
        ra = a->half.x * absR.m[1][0] + a->half.y * absR.m[0][0];
        rb = b->half.y * absR.m[2][2] + b->half.z * absR.m[2][1];
        sum = ra + rb;
        if (len == 0.0f) {
            len = 0.000001f;
        }
        d = (sum - t) / len;
        if (d != 0.0f) {
            if (d < 0.0f) {
                return 0;
            }
            if (d < *depth) {
                *depth = d;
                code = 12;
            }
        }
    }
    /* code 13: a.axis[2] x b.axis[1] */
    t = __builtin_fabsf(dA.v[1] * R.m[0][1] - dA.v[0] * R.m[1][1]);
    len = sqrtf(R.m[1][1] * R.m[1][1] + R.m[0][1] * R.m[0][1]);
    eq = Col_NearEq(__builtin_fabsf(dB.v[0] * R.m[2][2] - dB.v[2] * R.m[2][0]), t, 0.000001f);
    eq |= Col_NearEq(len, sqrtf(R.m[2][2] * R.m[2][2] + R.m[2][0] * R.m[2][0]), 0.000001f);
    if (eq == 0) {
        ra = a->half.x * absR.m[1][1] + a->half.y * absR.m[0][1];
        rb = b->half.x * absR.m[2][2] + b->half.z * absR.m[2][0];
        sum = ra + rb;
        if (len == 0.0f) {
            len = 0.000001f;
        }
        d = (sum - t) / len;
        if (d != 0.0f) {
            if (d < 0.0f) {
                return 0;
            }
            if (d < *depth) {
                *depth = d;
                code = 13;
            }
        }
    }
    /* code 14: a.axis[2] x b.axis[2] */
    t = __builtin_fabsf(dA.v[1] * R.m[0][2] - dA.v[0] * R.m[1][2]);
    len = sqrtf(R.m[1][2] * R.m[1][2] + R.m[0][2] * R.m[0][2]);
    eq = Col_NearEq(__builtin_fabsf(dB.v[0] * R.m[2][1] - dB.v[1] * R.m[2][0]), t, 0.000001f);
    eq |= Col_NearEq(len, sqrtf(R.m[2][1] * R.m[2][1] + R.m[2][0] * R.m[2][0]), 0.000001f);
    if (eq == 0) {
        ra = a->half.x * absR.m[1][2] + a->half.y * absR.m[0][2];
        rb = b->half.x * absR.m[2][1] + b->half.y * absR.m[2][0];
        sum = ra + rb;
        if (len == 0.0f) {
            len = 0.000001f;
        }
        d = (sum - t) / len;
        if (d != 0.0f) {
            if (d < 0.0f) {
                return 0;
            }
            if (d < *depth) {
                *depth = d;
                code = 14;
            }
        }
    }

    if (code < 3) {
        tmp.i[0] = (R.m[code][0] > -0.00001f) ? 1 : -1;
        tmp.i[1] = (R.m[code][1] > -0.00001f) ? 1 : -1;
        tmp.i[2] = (R.m[code][2] > -0.00001f) ? 1 : -1;
        if (!(dA.v[code] > 0.0f)) {
            normal->x = ((ColObbV *)a)->axis[code].v[0];
            normal->y = ((ColObbV *)a)->axis[code].v[1];
            normal->z = ((ColObbV *)a)->axis[code].v[2];
            point->x = b->center.x + tmp.i[0] * b->axis[0].x * b->half.x + tmp.i[1] * b->axis[1].x * b->half.y +
                       tmp.i[2] * b->axis[2].x * b->half.z;
                point->y = b->center.y + tmp.i[0] * b->axis[0].y * b->half.x + tmp.i[1] * b->axis[1].y * b->half.y +
                       tmp.i[2] * b->axis[2].y * b->half.z;
                point->z = b->center.z + tmp.i[0] * b->axis[0].z * b->half.x + tmp.i[1] * b->axis[1].z * b->half.y +
                       tmp.i[2] * b->axis[2].z * b->half.z;
        } else {
            normal->x = -((ColObbV *)a)->axis[code].v[0];
            normal->y = -((ColObbV *)a)->axis[code].v[1];
            normal->z = -((ColObbV *)a)->axis[code].v[2];
            point->x = b->center.x - tmp.i[0] * b->axis[0].x * b->half.x - tmp.i[1] * b->axis[1].x * b->half.y -
                       tmp.i[2] * b->axis[2].x * b->half.z;
                point->y = b->center.y - tmp.i[0] * b->axis[0].y * b->half.x - tmp.i[1] * b->axis[1].y * b->half.y -
                       tmp.i[2] * b->axis[2].y * b->half.z;
                point->z = b->center.z - tmp.i[0] * b->axis[0].z * b->half.x - tmp.i[1] * b->axis[1].z * b->half.y -
                       tmp.i[2] * b->axis[2].z * b->half.z;
        }
    } else if (code < 6) {
        tmp.i[0] = (R.m[0][code - 3] > -0.00001f) ? 1 : -1;
        tmp.i[1] = (R.m[1][code - 3] > -0.00001f) ? 1 : -1;
        tmp.i[2] = (R.m[2][code - 3] > -0.00001f) ? 1 : -1;
        if (!(Vec3_Dot(&b->axis[code - 3], &diff) > 0.0f)) {
            normal->x = ((ColObbV *)b)->axis[code - 3].v[0];
            normal->y = ((ColObbV *)b)->axis[code - 3].v[1];
            normal->z = ((ColObbV *)b)->axis[code - 3].v[2];
            point->x = a->center.x - tmp.i[0] * a->axis[0].x * a->half.x - tmp.i[1] * a->axis[1].x * a->half.y -
                       tmp.i[2] * a->axis[2].x * a->half.z;
                point->y = a->center.y - tmp.i[0] * a->axis[0].y * a->half.x - tmp.i[1] * a->axis[1].y * a->half.y -
                       tmp.i[2] * a->axis[2].y * a->half.z;
                point->z = a->center.z - tmp.i[0] * a->axis[0].z * a->half.x - tmp.i[1] * a->axis[1].z * a->half.y -
                       tmp.i[2] * a->axis[2].z * a->half.z;
        } else {
            normal->x = -((ColObbV *)b)->axis[code - 3].v[0];
            normal->y = -((ColObbV *)b)->axis[code - 3].v[1];
            normal->z = -((ColObbV *)b)->axis[code - 3].v[2];
            point->x = a->center.x + tmp.i[0] * a->axis[0].x * a->half.x + tmp.i[1] * a->axis[1].x * a->half.y +
                       tmp.i[2] * a->axis[2].x * a->half.z;
                point->y = a->center.y + tmp.i[0] * a->axis[0].y * a->half.x + tmp.i[1] * a->axis[1].y * a->half.y +
                       tmp.i[2] * a->axis[2].y * a->half.z;
                point->z = a->center.z + tmp.i[0] * a->axis[0].z * a->half.x + tmp.i[1] * a->axis[1].z * a->half.y +
                       tmp.i[2] * a->axis[2].z * a->half.z;
        }
    } else {
        switch (code - 6) {
        case 0: /* a.axis[0] x b.axis[0] */ {
            sA[0] = (R.m[2][0] < 0.0f) ? 1 : -1;
            sA[1] = (0.0f < R.m[1][0]) ? 1 : -1;
            sB[0] = (R.m[0][2] < 0.0f) ? 1 : -1;
            sB[1] = (0.0f < R.m[0][1]) ? 1 : -1;
            {
                f32 w = -R.m[2][0] * dA.v[1] + R.m[1][0] * dA.v[2];

                tmp.f[0] = 0.0f;
                tmp.f[1] = -R.m[2][0] * w;
                tmp.f[2] = R.m[1][0] * w;
                if (a->half.y * sA[0] * tmp.f[1] + a->half.z * sA[1] * tmp.f[2] > -0.00001f) {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x + a->axis[1].x * a->half.y * sA[0] +
                              a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x + a->axis[1].y * a->half.y * sA[0] +
                              a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x + a->axis[1].z * a->half.y * sA[0] +
                              a->axis[2].z * a->half.z * sA[1];
                } else {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x + -a->axis[1].x * a->half.y * sA[0] +
                              -a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x + -a->axis[1].y * a->half.y * sA[0] +
                              -a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x + -a->axis[1].z * a->half.y * sA[0] +
                              -a->axis[2].z * a->half.z * sA[1];
                }
            }
            {
                f32 w = -R.m[0][2] * dB.v[1] + R.m[0][1] * dB.v[2];

                tmp.f[0] = 0.0f;
                tmp.f[1] = -R.m[0][2] * w;
                tmp.f[2] = R.m[0][1] * w;
                if (b->half.y * sB[0] * tmp.f[1] + b->half.z * sB[1] * tmp.f[2] > -0.00001f) {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x + -b->axis[1].x * b->half.y * sB[0] +
                              -b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x + -b->axis[1].y * b->half.y * sB[0] +
                              -b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x + -b->axis[1].z * b->half.y * sB[0] +
                              -b->axis[2].z * b->half.z * sB[1];
                } else {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x + b->axis[1].x * b->half.y * sB[0] +
                              b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x + b->axis[1].y * b->half.y * sB[0] +
                              b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x + b->axis[1].z * b->half.y * sB[0] +
                              b->axis[2].z * b->half.z * sB[1];
                }
            }
            Col_LineLineParams(&s, &u, (ColVec *)&pA, &a->axis[0], (ColVec *)&pB, &b->axis[0]);
            point->x = pA.v[0] + a->axis[0].x * s;
            point->y = pA.v[1] + a->axis[0].y * s;
            point->z = pA.v[2] + a->axis[0].z * s;
            n.x = pB.v[0] + b->axis[0].x * u - point->x;
            n.y = pB.v[1] + b->axis[0].y * u - point->y;
            n.z = pB.v[2] + b->axis[0].z * u - point->z;
            Vec3_Normalize(normal, &n);
            break;
        }
        case 1: /* a.axis[0] x b.axis[1] */ {
            sA[0] = (R.m[2][1] < 0.0f) ? 1 : -1;
            sA[1] = (0.0f < R.m[1][1]) ? 1 : -1;
            sB[0] = (0.0f < R.m[0][2]) ? 1 : -1;
            sB[1] = (R.m[0][0] < 0.0f) ? 1 : -1;
            {
                f32 w = -R.m[2][1] * dA.v[1] + R.m[1][1] * dA.v[2];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = 0.0f;
                tmp.f[1] = -R.m[2][1] * w;
                tmp.f[2] = R.m[1][1] * w;
                if (a->half.y * sA[0] * tmp.f[1] + a->half.z * sA[1] * tmp.f[2] > -0.00001f) {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x + a->axis[1].x * a->half.y * sA[0] +
                              a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x + a->axis[1].y * a->half.y * sA[0] +
                              a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x + a->axis[1].z * a->half.y * sA[0] +
                              a->axis[2].z * a->half.z * sA[1];
                } else {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x + -a->axis[1].x * a->half.y * sA[0] +
                              -a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x + -a->axis[1].y * a->half.y * sA[0] +
                              -a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x + -a->axis[1].z * a->half.y * sA[0] +
                              -a->axis[2].z * a->half.z * sA[1];
                }
            }
            {
                f32 w = R.m[0][2] * dB.v[0] - R.m[0][0] * dB.v[2];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = R.m[0][2] * w;
                tmp.f[1] = 0.0f;
                tmp.f[2] = -R.m[0][0] * w;
                if (b->half.x * sB[0] * tmp.f[0] + b->half.z * sB[1] * tmp.f[2] > -0.00001f) {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x * sB[0] + -b->axis[1].x * b->half.y +
                              -b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x * sB[0] + -b->axis[1].y * b->half.y +
                              -b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x * sB[0] + -b->axis[1].z * b->half.y +
                              -b->axis[2].z * b->half.z * sB[1];
                } else {
                    pB.v[0] = b->center.x + b->axis[0].x * b->half.x * sB[0] + -b->axis[1].x * b->half.y +
                              b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + b->axis[0].y * b->half.x * sB[0] + -b->axis[1].y * b->half.y +
                              b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + b->axis[0].z * b->half.x * sB[0] + -b->axis[1].z * b->half.y +
                              b->axis[2].z * b->half.z * sB[1];
                }
            }
            Col_LineLineParams(&s, &u, (ColVec *)&pA, &a->axis[0], (ColVec *)&pB, &b->axis[1]);
            point->x = pA.v[0] + a->axis[0].x * s;
            point->y = pA.v[1] + a->axis[0].y * s;
            point->z = pA.v[2] + a->axis[0].z * s;
            n.x = pB.v[0] + b->axis[1].x * u - point->x;
            n.y = pB.v[1] + b->axis[1].y * u - point->y;
            n.z = pB.v[2] + b->axis[1].z * u - point->z;
            Vec3_Normalize(normal, &n);
            break;
        }
        case 2: /* a.axis[0] x b.axis[2] */ {
            sA[0] = (R.m[2][2] < 0.0f) ? 1 : -1;
            sA[1] = (0.0f < R.m[1][2]) ? 1 : -1;
            sB[0] = (R.m[0][1] < 0.0f) ? 1 : -1;
            sB[1] = (0.0f < R.m[0][0]) ? 1 : -1;
            {
                f32 w = -R.m[2][2] * dA.v[1] + R.m[1][2] * dA.v[2];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = 0.0f;
                tmp.f[1] = -R.m[2][2] * w;
                tmp.f[2] = R.m[1][2] * w;
                if (a->half.y * sA[0] * tmp.f[1] + a->half.z * sA[1] * tmp.f[2] > -0.00001f) {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x + a->axis[1].x * a->half.y * sA[0] +
                              a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x + a->axis[1].y * a->half.y * sA[0] +
                              a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x + a->axis[1].z * a->half.y * sA[0] +
                              a->axis[2].z * a->half.z * sA[1];
                } else {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x + -a->axis[1].x * a->half.y * sA[0] +
                              -a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x + -a->axis[1].y * a->half.y * sA[0] +
                              -a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x + -a->axis[1].z * a->half.y * sA[0] +
                              -a->axis[2].z * a->half.z * sA[1];
                }
            }
            {
                f32 w = -R.m[0][1] * dB.v[0] + R.m[0][0] * dB.v[1];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = -R.m[0][1] * w;
                tmp.f[1] = R.m[0][0] * w;
                tmp.f[2] = 0.0f;
                if (b->half.x * sB[0] * tmp.f[0] + b->half.y * sB[1] * tmp.f[1] > -0.00001f) {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x * sB[0] +
                              -b->axis[1].x * b->half.y * sB[1] + -b->axis[2].x * b->half.z;
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x * sB[0] +
                              -b->axis[1].y * b->half.y * sB[1] + -b->axis[2].y * b->half.z;
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x * sB[0] +
                              -b->axis[1].z * b->half.y * sB[1] + -b->axis[2].z * b->half.z;
                } else {
                    pB.v[0] = b->center.x + b->axis[0].x * b->half.x * sB[0] +
                              b->axis[1].x * b->half.y * sB[1] + -b->axis[2].x * b->half.z;
                    pB.v[1] = b->center.y + b->axis[0].y * b->half.x * sB[0] +
                              b->axis[1].y * b->half.y * sB[1] + -b->axis[2].y * b->half.z;
                    pB.v[2] = b->center.z + b->axis[0].z * b->half.x * sB[0] +
                              b->axis[1].z * b->half.y * sB[1] + -b->axis[2].z * b->half.z;
                }
            }
            Col_LineLineParams(&s, &u, (ColVec *)&pA, &a->axis[0], (ColVec *)&pB, &b->axis[2]);
            point->x = pA.v[0] + a->axis[0].x * s;
            point->y = pA.v[1] + a->axis[0].y * s;
            point->z = pA.v[2] + a->axis[0].z * s;
            n.x = pB.v[0] + b->axis[2].x * u - point->x;
            n.y = pB.v[1] + b->axis[2].y * u - point->y;
            n.z = pB.v[2] + b->axis[2].z * u - point->z;
            Vec3_Normalize(normal, &n);
            break;
        }
        case 3: /* a.axis[1] x b.axis[0] */ {
            sA[0] = (0.0f < R.m[2][0]) ? 1 : -1;
            sA[1] = (R.m[0][0] < 0.0f) ? 1 : -1;
            sB[0] = (R.m[1][2] < 0.0f) ? 1 : -1;
            sB[1] = (0.0f < R.m[1][1]) ? 1 : -1;
            {
                f32 w = R.m[2][0] * dA.v[0] - R.m[0][0] * dA.v[2];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = R.m[2][0] * w;
                tmp.f[1] = 0.0f;
                tmp.f[2] = -R.m[0][0] * w;
                if (a->half.x * sA[0] * tmp.f[0] + a->half.z * sA[1] * tmp.f[2] > -0.00001f) {
                    pA.v[0] = a->center.x + a->axis[0].x * a->half.x * sA[0] + -a->axis[1].x * a->half.y +
                              a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + a->axis[0].y * a->half.x * sA[0] + -a->axis[1].y * a->half.y +
                              a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + a->axis[0].z * a->half.x * sA[0] + -a->axis[1].z * a->half.y +
                              a->axis[2].z * a->half.z * sA[1];
                } else {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x * sA[0] + -a->axis[1].x * a->half.y +
                              -a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x * sA[0] + -a->axis[1].y * a->half.y +
                              -a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x * sA[0] + -a->axis[1].z * a->half.y +
                              -a->axis[2].z * a->half.z * sA[1];
                }
            }
            {
                f32 w = -R.m[1][2] * dB.v[1] + R.m[1][1] * dB.v[2];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = 0.0f;
                tmp.f[1] = -R.m[1][2] * w;
                tmp.f[2] = R.m[1][1] * w;
                if (b->half.y * sB[0] * tmp.f[1] + b->half.z * sB[1] * tmp.f[2] > -0.00001f) {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x + -b->axis[1].x * b->half.y * sB[0] +
                              -b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x + -b->axis[1].y * b->half.y * sB[0] +
                              -b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x + -b->axis[1].z * b->half.y * sB[0] +
                              -b->axis[2].z * b->half.z * sB[1];
                } else {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x + b->axis[1].x * b->half.y * sB[0] +
                              b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x + b->axis[1].y * b->half.y * sB[0] +
                              b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x + b->axis[1].z * b->half.y * sB[0] +
                              b->axis[2].z * b->half.z * sB[1];
                }
            }
            Col_LineLineParams(&s, &u, (ColVec *)&pA, &a->axis[1], (ColVec *)&pB, &b->axis[0]);
            point->x = pA.v[0] + a->axis[1].x * s;
            point->y = pA.v[1] + a->axis[1].y * s;
            point->z = pA.v[2] + a->axis[1].z * s;
            n.x = pB.v[0] + b->axis[0].x * u - point->x;
            n.y = pB.v[1] + b->axis[0].y * u - point->y;
            n.z = pB.v[2] + b->axis[0].z * u - point->z;
            Vec3_Normalize(normal, &n);
            break;
        }
        case 4: /* a.axis[1] x b.axis[1] */ {
            sA[0] = (0.0f < R.m[2][1]) ? 1 : -1;
            sA[1] = (R.m[0][1] < 0.0f) ? 1 : -1;
            sB[0] = (0.0f < R.m[1][2]) ? 1 : -1;
            sB[1] = (R.m[1][0] < 0.0f) ? 1 : -1;
            {
                f32 w = R.m[2][1] * dA.v[0] - R.m[0][1] * dA.v[2];
                f32 dp;

                memset(&tmp, 0, sizeof(tmp));
                /* as built: the test reads the vector before it is filled, so it sees the zeros of the memset */
                dp = a->half.x * sA[0] * tmp.f[0] + a->half.z * sA[1] * tmp.f[2];
                tmp.f[0] = R.m[2][1] * w;
                tmp.f[1] = 0.0f;
                tmp.f[2] = -R.m[0][1] * w;
                if (dp > -0.00001f) {
                    pA.v[0] = a->center.x + a->axis[0].x * a->half.x * sA[0] + -a->axis[1].x * a->half.y +
                              a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + a->axis[0].y * a->half.x * sA[0] + -a->axis[1].y * a->half.y +
                              a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + a->axis[0].z * a->half.x * sA[0] + -a->axis[1].z * a->half.y +
                              a->axis[2].z * a->half.z * sA[1];
                } else {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x * sA[0] + -a->axis[1].x * a->half.y +
                              -a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x * sA[0] + -a->axis[1].y * a->half.y +
                              -a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x * sA[0] + -a->axis[1].z * a->half.y +
                              -a->axis[2].z * a->half.z * sA[1];
                }
            }
            {
                f32 w = R.m[1][2] * dB.v[0] - R.m[1][0] * dB.v[2];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = R.m[1][2] * w;
                tmp.f[1] = 0.0f;
                tmp.f[2] = -R.m[1][0] * w;
                if (b->half.x * sB[0] * tmp.f[0] + b->half.z * sB[1] * tmp.f[2] > -0.00001f) {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x * sB[0] + -b->axis[1].x * b->half.y +
                              -b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x * sB[0] + -b->axis[1].y * b->half.y +
                              -b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x * sB[0] + -b->axis[1].z * b->half.y +
                              -b->axis[2].z * b->half.z * sB[1];
                } else {
                    pB.v[0] = b->center.x + b->axis[0].x * b->half.x * sB[0] + -b->axis[1].x * b->half.y +
                              b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + b->axis[0].y * b->half.x * sB[0] + -b->axis[1].y * b->half.y +
                              b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + b->axis[0].z * b->half.x * sB[0] + -b->axis[1].z * b->half.y +
                              b->axis[2].z * b->half.z * sB[1];
                }
            }
            Col_LineLineParams(&s, &u, (ColVec *)&pA, &a->axis[1], (ColVec *)&pB, &b->axis[1]);
            point->x = pA.v[0] + a->axis[1].x * s;
            point->y = pA.v[1] + a->axis[1].y * s;
            point->z = pA.v[2] + a->axis[1].z * s;
            n.x = pB.v[0] + b->axis[1].x * u - point->x;
            n.y = pB.v[1] + b->axis[1].y * u - point->y;
            n.z = pB.v[2] + b->axis[1].z * u - point->z;
            Vec3_Normalize(normal, &n);
            break;
        }
        case 5: /* a.axis[1] x b.axis[2] */ {
            sA[0] = (0.0f < R.m[2][2]) ? 1 : -1;
            sA[1] = (R.m[0][2] < 0.0f) ? 1 : -1;
            sB[0] = (R.m[1][1] < 0.0f) ? 1 : -1;
            sB[1] = (0.0f < R.m[1][0]) ? 1 : -1;
            {
                f32 w = R.m[2][2] * dA.v[0] - R.m[0][2] * dA.v[2];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = R.m[2][2] * w;
                tmp.f[1] = 0.0f;
                tmp.f[2] = -R.m[0][2] * w;
                if (a->half.x * sA[0] * tmp.f[0] + a->half.z * sA[1] * tmp.f[2] > -0.00001f) {
                    pA.v[0] = a->center.x + a->axis[0].x * a->half.x * sA[0] + -a->axis[1].x * a->half.y +
                              a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + a->axis[0].y * a->half.x * sA[0] + -a->axis[1].y * a->half.y +
                              a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + a->axis[0].z * a->half.x * sA[0] + -a->axis[1].z * a->half.y +
                              a->axis[2].z * a->half.z * sA[1];
                } else {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x * sA[0] + -a->axis[1].x * a->half.y +
                              -a->axis[2].x * a->half.z * sA[1];
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x * sA[0] + -a->axis[1].y * a->half.y +
                              -a->axis[2].y * a->half.z * sA[1];
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x * sA[0] + -a->axis[1].z * a->half.y +
                              -a->axis[2].z * a->half.z * sA[1];
                }
            }
            {
                f32 w = -R.m[1][1] * dB.v[0] + R.m[1][0] * dB.v[1];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = -R.m[1][1] * w;
                tmp.f[1] = R.m[1][0] * w;
                tmp.f[2] = 0.0f;
                if (b->half.x * sB[0] * tmp.f[0] + b->half.y * sB[1] * tmp.f[1] > -0.00001f) {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x * sB[0] +
                              -b->axis[1].x * b->half.y * sB[1] + -b->axis[2].x * b->half.z;
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x * sB[0] +
                              -b->axis[1].y * b->half.y * sB[1] + -b->axis[2].y * b->half.z;
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x * sB[0] +
                              -b->axis[1].z * b->half.y * sB[1] + -b->axis[2].z * b->half.z;
                } else {
                    pB.v[0] = b->center.x + b->axis[0].x * b->half.x * sB[0] +
                              b->axis[1].x * b->half.y * sB[1] + -b->axis[2].x * b->half.z;
                    pB.v[1] = b->center.y + b->axis[0].y * b->half.x * sB[0] +
                              b->axis[1].y * b->half.y * sB[1] + -b->axis[2].y * b->half.z;
                    pB.v[2] = b->center.z + b->axis[0].z * b->half.x * sB[0] +
                              b->axis[1].z * b->half.y * sB[1] + -b->axis[2].z * b->half.z;
                }
            }
            Col_LineLineParams(&s, &u, (ColVec *)&pA, &a->axis[1], (ColVec *)&pB, &b->axis[2]);
            point->x = pA.v[0] + a->axis[1].x * s;
            point->y = pA.v[1] + a->axis[1].y * s;
            point->z = pA.v[2] + a->axis[1].z * s;
            n.x = pB.v[0] + b->axis[2].x * u - point->x;
            n.y = pB.v[1] + b->axis[2].y * u - point->y;
            n.z = pB.v[2] + b->axis[2].z * u - point->z;
            Vec3_Normalize(normal, &n);
            break;
        }
        case 6: /* a.axis[2] x b.axis[0] */ {
            sA[0] = (R.m[1][0] < 0.0f) ? 1 : -1;
            sA[1] = (0.0f < R.m[0][0]) ? 1 : -1;
            sB[0] = (R.m[2][2] < 0.0f) ? 1 : -1;
            sB[1] = (0.0f < R.m[2][1]) ? 1 : -1;
            {
                f32 w = -R.m[1][0] * dA.v[0] + R.m[0][0] * dA.v[1];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = -R.m[1][0] * w;
                tmp.f[1] = R.m[0][0];   /* as built: not scaled by w */
                tmp.f[2] = 0.0f;
                if (a->half.x * sA[0] * tmp.f[0] + a->half.y * sA[1] * tmp.f[1] > -0.00001f) {
                    pA.v[0] = a->center.x + a->axis[0].x * a->half.x * sA[0] +
                              a->axis[1].x * a->half.y * sA[1] + -a->axis[2].x * a->half.z;
                    pA.v[1] = a->center.y + a->axis[0].y * a->half.x * sA[0] +
                              a->axis[1].y * a->half.y * sA[1] + -a->axis[2].y * a->half.z;
                    pA.v[2] = a->center.z + a->axis[0].z * a->half.x * sA[0] +
                              a->axis[1].z * a->half.y * sA[1] + -a->axis[2].z * a->half.z;
                } else {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x * sA[0] +
                              -a->axis[1].x * a->half.y * sA[1] + -a->axis[2].x * a->half.z;
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x * sA[0] +
                              -a->axis[1].y * a->half.y * sA[1] + -a->axis[2].y * a->half.z;
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x * sA[0] +
                              -a->axis[1].z * a->half.y * sA[1] + -a->axis[2].z * a->half.z;
                }
            }
            {
                f32 w = -R.m[2][2] * dB.v[1] + R.m[2][1] * dB.v[2];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = 0.0f;
                tmp.f[1] = -R.m[2][2] * w;
                tmp.f[2] = R.m[2][1] * w;
                if (b->half.y * sB[0] * tmp.f[1] + b->half.z * sB[1] * tmp.f[2] > -0.00001f) {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x + -b->axis[1].x * b->half.y * sB[0] +
                              -b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x + -b->axis[1].y * b->half.y * sB[0] +
                              -b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x + -b->axis[1].z * b->half.y * sB[0] +
                              -b->axis[2].z * b->half.z * sB[1];
                } else {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x + b->axis[1].x * b->half.y * sB[0] +
                              b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x + b->axis[1].y * b->half.y * sB[0] +
                              b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x + b->axis[1].z * b->half.y * sB[0] +
                              b->axis[2].z * b->half.z * sB[1];
                }
            }
            Col_LineLineParams(&s, &u, (ColVec *)&pA, &a->axis[2], (ColVec *)&pB, &b->axis[0]);
            point->x = pA.v[0] + a->axis[2].x * s;
            point->y = pA.v[1] + a->axis[2].y * s;
            point->z = pA.v[2] + a->axis[2].z * s;
            n.x = pB.v[0] + b->axis[0].x * u - point->x;
            n.y = pB.v[1] + b->axis[0].y * u - point->y;
            n.z = pB.v[2] + b->axis[0].z * u - point->z;
            Vec3_Normalize(normal, &n);
            break;
        }
        case 7: /* a.axis[2] x b.axis[1] */ {
            sA[0] = (R.m[1][1] < 0.0f) ? 1 : -1;
            sA[1] = (0.0f < R.m[0][1]) ? 1 : -1;
            sB[0] = (0.0f < R.m[2][2]) ? 1 : -1;
            sB[1] = (R.m[2][0] < 0.0f) ? 1 : -1;
            {
                f32 w = -R.m[1][1] * dA.v[0] + R.m[0][1] * dA.v[1];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = -R.m[1][1] * w;
                tmp.f[1] = R.m[0][1] * w;
                tmp.f[2] = 0.0f;
                if (a->half.x * sA[0] * tmp.f[0] + a->half.y * sA[1] * tmp.f[1] > -0.00001f) {
                    pA.v[0] = a->center.x + a->axis[0].x * a->half.x * sA[0] +
                              a->axis[1].x * a->half.y * sA[1] + -a->axis[2].x * a->half.z;
                    pA.v[1] = a->center.y + a->axis[0].y * a->half.x * sA[0] +
                              a->axis[1].y * a->half.y * sA[1] + -a->axis[2].y * a->half.z;
                    pA.v[2] = a->center.z + a->axis[0].z * a->half.x * sA[0] +
                              a->axis[1].z * a->half.y * sA[1] + -a->axis[2].z * a->half.z;
                } else {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x * sA[0] +
                              -a->axis[1].x * a->half.y * sA[1] + -a->axis[2].x * a->half.z;
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x * sA[0] +
                              -a->axis[1].y * a->half.y * sA[1] + -a->axis[2].y * a->half.z;
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x * sA[0] +
                              -a->axis[1].z * a->half.y * sA[1] + -a->axis[2].z * a->half.z;
                }
            }
            {
                f32 w = R.m[2][2] * dB.v[0] - R.m[2][0] * dB.v[2];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = R.m[2][2] * w;
                tmp.f[1] = 0.0f;
                tmp.f[2] = -R.m[2][0] * w;
                if (b->half.x * sB[0] * tmp.f[0] + b->half.z * sB[1] * tmp.f[2] > -0.00001f) {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x * sB[0] + -b->axis[1].x * b->half.y +
                              -b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x * sB[0] + -b->axis[1].y * b->half.y +
                              -b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x * sB[0] + -b->axis[1].z * b->half.y +
                              -b->axis[2].z * b->half.z * sB[1];
                } else {
                    pB.v[0] = b->center.x + b->axis[0].x * b->half.x * sB[0] + -b->axis[1].x * b->half.y +
                              b->axis[2].x * b->half.z * sB[1];
                    pB.v[1] = b->center.y + b->axis[0].y * b->half.x * sB[0] + -b->axis[1].y * b->half.y +
                              b->axis[2].y * b->half.z * sB[1];
                    pB.v[2] = b->center.z + b->axis[0].z * b->half.x * sB[0] + -b->axis[1].z * b->half.y +
                              b->axis[2].z * b->half.z * sB[1];
                }
            }
            Col_LineLineParams(&s, &u, (ColVec *)&pA, &a->axis[2], (ColVec *)&pB, &b->axis[1]);
            point->x = pA.v[0] + a->axis[2].x * s;
            point->y = pA.v[1] + a->axis[2].y * s;
            point->z = pA.v[2] + a->axis[2].z * s;
            n.x = pB.v[0] + b->axis[1].x * u - point->x;
            n.y = pB.v[1] + b->axis[1].y * u - point->y;
            n.z = pB.v[2] + b->axis[1].z * u - point->z;
            Vec3_Normalize(normal, &n);
            break;
        }
        case 8: /* a.axis[2] x b.axis[2] */ {
            sA[0] = (R.m[1][2] < 0.0f) ? 1 : -1;
            sA[1] = (0.0f < R.m[0][2]) ? 1 : -1;
            sB[0] = (R.m[2][1] < 0.0f) ? 1 : -1;
            sB[1] = (0.0f < R.m[2][0]) ? 1 : -1;
            {
                f32 w = -R.m[1][2] * dA.v[0] + R.m[0][2] * dA.v[1];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = -R.m[1][2] * w;
                tmp.f[1] = R.m[0][2] * w;
                tmp.f[2] = 0.0f;
                if (a->half.x * sA[0] * tmp.f[0] + a->half.y * sA[1] * tmp.f[1] > -0.00001f) {
                    pA.v[0] = a->center.x + a->axis[0].x * a->half.x * sA[0] +
                              a->axis[1].x * a->half.y * sA[1] + -a->axis[2].x * a->half.z;
                    pA.v[1] = a->center.y + a->axis[0].y * a->half.x * sA[0] +
                              a->axis[1].y * a->half.y * sA[1] + -a->axis[2].y * a->half.z;
                    pA.v[2] = a->center.z + a->axis[0].z * a->half.x * sA[0] +
                              a->axis[1].z * a->half.y * sA[1] + -a->axis[2].z * a->half.z;
                } else {
                    pA.v[0] = a->center.x + -a->axis[0].x * a->half.x * sA[0] +
                              -a->axis[1].x * a->half.y * sA[1] + -a->axis[2].x * a->half.z;
                    pA.v[1] = a->center.y + -a->axis[0].y * a->half.x * sA[0] +
                              -a->axis[1].y * a->half.y * sA[1] + -a->axis[2].y * a->half.z;
                    pA.v[2] = a->center.z + -a->axis[0].z * a->half.x * sA[0] +
                              -a->axis[1].z * a->half.y * sA[1] + -a->axis[2].z * a->half.z;
                }
            }
            {
                f32 w = -R.m[2][1] * dB.v[0] + R.m[2][0] * dB.v[1];

                memset(&tmp, 0, sizeof(tmp));
                tmp.f[0] = -R.m[2][1] * w;
                tmp.f[1] = R.m[2][0] * w;
                tmp.f[2] = 0.0f;
                if (b->half.x * sB[0] * tmp.f[0] + b->half.y * sB[1] * tmp.f[1] > -0.00001f) {
                    pB.v[0] = b->center.x + -b->axis[0].x * b->half.x * sB[0] +
                              -b->axis[1].x * b->half.y * sB[1] + -b->axis[2].x * b->half.z;
                    pB.v[1] = b->center.y + -b->axis[0].y * b->half.x * sB[0] +
                              -b->axis[1].y * b->half.y * sB[1] + -b->axis[2].y * b->half.z;
                    pB.v[2] = b->center.z + -b->axis[0].z * b->half.x * sB[0] +
                              -b->axis[1].z * b->half.y * sB[1] + -b->axis[2].z * b->half.z;
                } else {
                    pB.v[0] = b->center.x + b->axis[0].x * b->half.x * sB[0] +
                              b->axis[1].x * b->half.y * sB[1] + -b->axis[2].x * b->half.z;
                    pB.v[1] = b->center.y + b->axis[0].y * b->half.x * sB[0] +
                              b->axis[1].y * b->half.y * sB[1] + -b->axis[2].y * b->half.z;
                    pB.v[2] = b->center.z + b->axis[0].z * b->half.x * sB[0] +
                              b->axis[1].z * b->half.y * sB[1] + -b->axis[2].z * b->half.z;
                }
            }
            Col_LineLineParams(&s, &u, (ColVec *)&pA, &a->axis[2], (ColVec *)&pB, &b->axis[2]);
            point->x = pA.v[0] + a->axis[2].x * s;
            point->y = pA.v[1] + a->axis[2].y * s;
            point->z = pA.v[2] + a->axis[2].z * s;
            n.x = pB.v[0] + b->axis[2].x * u - point->x;
            n.y = pB.v[1] + b->axis[2].y * u - point->y;
            n.z = pB.v[2] + b->axis[2].z * u - point->z;
            Vec3_Normalize(normal, &n);
            break;
        }
        }
    }
    return 1;
}

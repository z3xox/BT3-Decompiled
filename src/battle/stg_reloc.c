#include "common.h"
#include "battle/obj_gs_env.h"

/*
 * Relocation of the stage file, 0x114B18..0x115170. See include/battle/obj_gs_env.h.
 *
 * BtlStage_Init (stg.c) points gBtlStage at the stage's header and calls BtlStage_Relocate with the base the
 * file's offsets are counted from. Every offset is in WORDS (pointer = base + offset * 4). The code that reads
 * the tables is the stage model code that follows (stg_model_anim.c onwards) and stg.c / stg_ambient.c.
 */

extern StgRelocHdr *gBtlStage;

#define STG_RELOC(type, field) ((field) = (type)(base + (s32)(field)))

/* Turns the offsets of an octree node into pointers, then does the same for its eight children. */
void StgOctree_Relocate(StgOctNode *node, s32 *base) {
    s32 i;
    s32 j;
    s32 k;

    if (node->unk00 != NULL) {
        STG_RELOC(s32 *, node->unk00);
    }
    if (node->list != NULL) {
        STG_RELOC(s32 **, node->list);
    }
    for (i = 0; i < node->count; i++) {
        if (node->list[i] != NULL && (s32)node->list[i] != -1) {
            STG_RELOC(s32 *, node->list[i]);
        } else {
            node->list[i] = NULL;
        }
    }
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 2; j++) {
            for (k = 0; k < 2; k++) {
                if (node->child[i][j][k] != NULL) {
                    STG_RELOC(StgOctNode *, node->child[i][j][k]);
                    StgOctree_Relocate(node->child[i][j][k], base);
                }
            }
        }
    }
}

/* Turns every offset of the stage header and of its tables into a pointer, then relocates the stage's
   texture file. The header pointer is used before it is tested (original bug: the test can never help). */
void BtlStage_Relocate(s32 *base) {
    u32 i;
    s32 j;
    s32 k;

    gBtlStage->base = base;
    if (gBtlStage != NULL) {
        STG_RELOC(StgRelocTimer *, gBtlStage->timers);
        STG_RELOC(StgRelocA *, gBtlStage->mdl);
        STG_RELOC(StgRelocB *, gBtlStage->always);
        STG_RELOC(void *, gBtlStage->vis);
        STG_RELOC(StgRelocC *, gBtlStage->nodes);
        STG_RELOC(StgRelocD *, gBtlStage->zones);
        STG_RELOC(void *, gBtlStage->bounds);
        STG_RELOC(StgRelocE *, gBtlStage->objs);
        STG_RELOC(StgOctNode *, gBtlStage->tree);
        for (i = 0; i < gBtlStage->timerCount; i++) {
            StgRelocTimer *t = &gBtlStage->timers[i];

            STG_RELOC(void *, t->ptr);
        }
        for (i = 0; i < gBtlStage->groupCount; i++) {
            StgRelocA *a = &gBtlStage->mdl[i];

            STG_RELOC(StgRelocA1 *, a->items);
            for (j = 0; j < a->count; j++) {
                StgRelocA1 *a1 = &a->items[j];

                if (a1->opt != NULL) {
                    STG_RELOC(void *, a1->opt);
                }
                STG_RELOC(StgRelocA2 *, a1->items);
                for (k = 0; k < a1->count; k++) {
                    StgRelocA2 *a2 = &a1->items[k];
                    s32 chainOfs = (s32)a2->chain;
                    s32 mtxOfs = (s32)a2->mtx;

                    a2->chain = base + chainOfs;
                    a2->mtx = base + mtxOfs;
                }
            }
        }
        for (i = 0; i < gBtlStage->alwaysCount; i++) {
            StgRelocB *b = &gBtlStage->always[i];

            STG_RELOC(StgRelocB1 *, b->items);
            for (j = 0; (u32)j < b->count; j++) {
                StgRelocB1 *b1 = &b->items[j];

                if (b1->opt != NULL) {
                    STG_RELOC(void *, b1->opt);
                }
                STG_RELOC(void **, b1->list);
                for (k = 0; k < b1->count; k++) {
                    STG_RELOC(void *, b1->list[k]);
                }
            }
        }
        for (i = 0; i < gBtlStage->nodeCount; i++) {
            StgRelocC *c = &gBtlStage->nodes[i];

            STG_RELOC(void *, c->keys);
        }
        for (i = 0; i < gBtlStage->zoneCount; i++) {
            StgRelocD *d = &gBtlStage->zones[i];

            {
                s32 o04 = (s32)d->mesh;
                s32 o0C = (s32)d->near;
                s32 o10 = (s32)d->ptr10;
                s32 o18 = (s32)d->items;

                d->mesh = base + o04;
                d->near = base + o0C;
                d->ptr10 = base + o10;
                d->items = (StgRelocD1 *)(base + o18);
            }
            for (j = 0; j < d->count; j++) {
                StgRelocD1 *d1 = &d->items[j];

                STG_RELOC(void *, d1->mesh);
            }
        }
        for (i = 0; i < gBtlStage->objCount; i++) {
            StgRelocE *e = &gBtlStage->objs[i];

            STG_RELOC(void **, e->list);
            for (j = 0; (u32)j < e->count; j++) {
                STG_RELOC(void *, e->list[j]);
            }
            for (j = 0; j < 2; j++) {
                STG_RELOC(void *, e->ptr0C[j]);
                STG_RELOC(void *, e->ptr14[j]);
                STG_RELOC(void *, e->ptr1C[j]);
            }
        }
        StgOctree_Relocate(gBtlStage->tree, base);
        Res_RelocateOffsets(&gBtlStage->tex, (u8 *)base, (TexFile *)(gBtlStage->base + gBtlStage->texOfs));
    }
}

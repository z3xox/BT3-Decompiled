#include "common.h"
#include "sys/save.h"
#include "sys/heap.h"
#include "sys/rand.h"
#include "sys/common.h"

extern void *memset(void *dst, s32 value, u32 size);

extern void Progress_ClearSession(void); /* clears the per-session parts of gProgress */

/* Debug (no callers): unlocks every character, stage, list entry and item, marks about half the items new, and gives 4,850,000 money. */
void Save_UnlockAll(SaveData *save) {
    SaveSlot *slot = save->slot;
    s32 i;
    s32 *val;

    for (i = 0; i < SAVE_CHARA_WORDS; i++) {
        save->charaBits[i] = 0;
    }
    for (i = 0; i < SAVE_CHARA_COUNT; i++) {
        SAVE_CHARA_WORD(gSaveData, i) |= SAVE_CHARA_MASK(i);
    }
    gSaveData->stageBits = 0;
    for (i = 0; i < SAVE_STAGE_COUNT; i++) {
        gSaveData->stageBits |= 1LL << i;
    }
    for (i = 0; i < SAVE_BGM_LIST_COUNT; i++) {
        if (i >= SAVE_BGM_COUNT) {
            break;
        }
        gSaveData->bgmBits |= 1 << i;
    }
    for (i = 0; i < SAVE_FLAG8_COUNT; i++) {
        gSaveData->unlockFlags |= 1 << i;
    }
    for (i = 0; i < SAVE_ITEM_COUNT; i++) {
        save->item[i] |= SAVE_ITEM_OWNED;
        if (Rand_Range(100) & 1) {
            save->item[i] |= SAVE_ITEM_NEW;
        }
    }
    save->money = SAVE_MONEY_UNLOCK_ALL;
    for (i = 0; i < SAVE_REC_COUNT; i++) {
        save->rec[i].chara = -1;
    }
    save->missionPages = 20;
    save->ubFlags |= 1;
    for (i = 0; i < SAVE_SLOT_COUNT; i++) {
        slot[i].flags |= 3;
        val = save->slot[i].val;
        val[0] = 0xFFFF;
        val[1] = 0xFFFF;
        val[2] = 0xFFFF;
    }
}

/* Clears the block and writes a new save: starting characters/stages/items, default key assignment, volumes and flags. */
void Save_SetDefaults(SaveData *save) {
    s32 i;
    ItemInfo *info;

    info = (ItemInfo *)((u8 *)gCommonRes->data[2] + ((ItemFile *)gCommonRes->data[2])->itemOffset / 4 * 4);
    Progress_ClearSession();
    memset(save, 0, SAVE_SIZE);
    save->slot[0].flags |= 3;
    save->slot[0].val[0] |= 1;
    save->slot[0].val[2] |= 1;
    save->level = 1;
    Save_ResetRules();
    for (i = 0; i < SAVE_CHARA_COUNT; i++) {
        switch (i) {
        case 7:
        case 8:
        case 9:
        case 10:
        case 0x18:
        case 0x26:
        case 0x36:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x4B:
        case 0x4C:
        case 0x61:
        case 0x6E:
        case 0x78:
        case 0x91:
        case 0x95:
        case 0x96:
        case 0x97:
        case 0x98:
        case 0x99:
        case 0x9A:
        case 0x9B:
        case 0x9C:
        case 0x9D:
        case 0x9E:
        case 0x9F:
        case 0xA0:
            break;
        default:
            SAVE_CHARA_WORD(gSaveData, i) |= SAVE_CHARA_MASK(i);
            break;
        }
    }
    for (i = 0; i < SAVE_STAGE_COUNT; i++) {
        switch (i) {
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x1F:
        case 0x20:
            break;
        default:
            gSaveData->stageBits |= 1LL << i;
            break;
        }
    }
    for (i = 0; i < SAVE_BGM_LIST_COUNT; i++) {
        if (i >= SAVE_BGM_COUNT) {
            break;
        }
        gSaveData->bgmBits |= 1 << i;
    }
    for (i = 0; i < SAVE_ITEM_COUNT; i++) {
        if (info[i].flags & ITEM_INFO_INITIAL) {
            Save_AddItem(i);
        }
    }
    for (i = 0; i < SAVE_REC_COUNT; i++) {
        save->rec[i].chara = -1;
    }
    save->missionPages = 5;
    save->rank = 99;
    for (i = 0; i < SAVE_PAD_COUNT; i++) {
        save->key[i][0] = 2;
        save->key[i][1] = 1;
        save->key[i][2] = 0;
        save->key[i][3] = 3;
        save->key[i][4] = 4;
        save->key[i][5] = 5;
        save->key[i][6] = 6;
        save->key[i][7] = 7;
        save->keyEdit[i][0] = 2;
        save->keyEdit[i][1] = 1;
        save->keyEdit[i][2] = 0;
        save->keyEdit[i][3] = 3;
        save->keyEdit[i][4] = 4;
        save->keyEdit[i][5] = 5;
        save->keyEdit[i][6] = 6;
        save->keyEdit[i][7] = 7;
    }
    save->flags |= SAVE_FLAG_DEFAULT;
    save->camDistMode = 0;
    save->bgmVolume = SAVE_VOLUME_DEFAULT;
    save->seVolume = SAVE_VOLUME_DEFAULT;
}

/* Empty; called with the block right after the defaults are written. */
void Save_Stub266600(SaveData *save) {
}

/* Allocates the 0x4000-byte save block and fills it with a new save. */
void Save_Init(void) {
    gSaveData = Heap_Alloc(SAVE_SIZE, 0x20, 0, 2);
    memset(gSaveData, 0, SAVE_SIZE);
    Save_SetDefaults(gSaveData);
    Save_Stub266600(gSaveData);
}

/* Gives item idx (0-based) if it is not owned yet, marking it new. */
void Save_AddItem(s32 idx) {
    if (!(gSaveData->item[idx] & SAVE_ITEM_OWNED)) {
        gSaveData->item[idx] |= SAVE_ITEM_OWNED;
        gSaveData->item[idx] |= SAVE_ITEM_NEW;
    }
}

/* Adds amount (may be negative) to the money, clamped to 0..9,999,999. */
void Save_AddMoney(s32 amount) {
    gSaveData->money += amount;
    if (gSaveData->money > SAVE_MONEY_MAX) {
        gSaveData->money = SAVE_MONEY_MAX;
    } else if (gSaveData->money < 0) {
        gSaveData->money = 0;
    }
}

/* Restores the six rule settings at 0xC34 to 3, 2, 2, 0, 0, 0. */
void Save_ResetRules(void) {
    gSaveData->rule[0] = 3;
    gSaveData->rule[1] = 2;
    gSaveData->rule[2] = 2;
    gSaveData->rule[3] = 0;
    gSaveData->rule[4] = 0;
    gSaveData->rule[5] = 0;
}

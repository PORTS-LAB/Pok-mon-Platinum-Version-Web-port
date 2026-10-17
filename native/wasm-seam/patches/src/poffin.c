#include "poffin.h"

#include <nitro.h>
#include <string.h>

#include "constants/flavor.h"

#include "heap.h"
#include "math_util.h"
#include "poffin_types.h"
#include "savedata.h"

/* wasm-seam: upstream nests an anonymous struct inside an anonymous union
 * (mwcc extension). clang exposes the union members directly, not through
 * the inner tag. Aliasing to a named view of the same bytes: layout is
 * identical (first member of the union, same offsets), so every access is
 * semantically equal to the original. POFFIN_FIELDS takes a Poffin*,
 * POFFIN_OBJ takes a Poffin lvalue. */
typedef struct PoffinInnerView {
    u8 type;
    u8 spiciness;
    u8 dryness;
    u8 sweetness;
    u8 bitterness;
    u8 sourness;
    u8 smoothness;
    u8 dummy;
} PoffinInnerView;
#define POFFIN_FIELDS(p) ((PoffinInnerView *)(void *)(p))
#define POFFIN_OBJ(o) ((PoffinInnerView *)(void *)&(o))

#define TYPE_NONE 30

int Poffin_SizeOf(void)
{
    return sizeof(Poffin);
}

BOOL Poffin_HasValidFlavor(Poffin *poffin)
{
    if (POFFIN_FIELDS(poffin)->type == TYPE_NONE) {
        return FALSE;
    }

    return TRUE;
}

void Poffin_Clear(Poffin *poffin)
{
    POFFIN_FIELDS(poffin)->type = TYPE_NONE;
    POFFIN_FIELDS(poffin)->spiciness = 0;
    POFFIN_FIELDS(poffin)->dryness = 0;
    POFFIN_FIELDS(poffin)->sweetness = 0;
    POFFIN_FIELDS(poffin)->bitterness = 0;
    POFFIN_FIELDS(poffin)->sourness = 0;
    POFFIN_FIELDS(poffin)->smoothness = 0;
    POFFIN_FIELDS(poffin)->dummy = 0;
}

Poffin *Poffin_New(enum HeapID heapID)
{
    Poffin *poffin = Heap_AllocAtEnd(heapID, sizeof(Poffin));
    Poffin_Clear(poffin);

    return poffin;
}

void Poffin_Copy(Poffin *src, Poffin *dest)
{
    POFFIN_FIELDS(dest)->type = POFFIN_FIELDS(src)->type;
    POFFIN_FIELDS(dest)->spiciness = POFFIN_FIELDS(src)->spiciness;
    POFFIN_FIELDS(dest)->dryness = POFFIN_FIELDS(src)->dryness;
    POFFIN_FIELDS(dest)->sweetness = POFFIN_FIELDS(src)->sweetness;
    POFFIN_FIELDS(dest)->bitterness = POFFIN_FIELDS(src)->bitterness;
    POFFIN_FIELDS(dest)->sourness = POFFIN_FIELDS(src)->sourness;
    POFFIN_FIELDS(dest)->smoothness = POFFIN_FIELDS(src)->smoothness;
    POFFIN_FIELDS(dest)->dummy = POFFIN_FIELDS(src)->dummy;
}

u8 Poffin_GetAttribute(Poffin *poffin, enum PoffinAttributeID attributeID)
{
    switch (attributeID) {
    case POFFIN_ATTRIBUTEID_TYPE:
        return POFFIN_FIELDS(poffin)->type;
    case POFFIN_ATTRIBUTEID_SPICINESS:
        return POFFIN_FIELDS(poffin)->spiciness;
    case POFFIN_ATTRIBUTEID_DRYNESS:
        return POFFIN_FIELDS(poffin)->dryness;
    case POFFIN_ATTRIBUTEID_SWEETNESS:
        return POFFIN_FIELDS(poffin)->sweetness;
    case POFFIN_ATTRIBUTEID_BITTERNESS:
        return POFFIN_FIELDS(poffin)->bitterness;
    case POFFIN_ATTRIBUTEID_SOURNESS:
        return POFFIN_FIELDS(poffin)->sourness;
    case POFFIN_ATTRIBUTEID_SMOOTHNESS:
    default:
        return POFFIN_FIELDS(poffin)->smoothness;
    }
}

static void Poffin_MakeFoul(Poffin *poffin, u8 param1)
{
    int v0;
    u8 v1;

    v0 = 0;

    do {
        v1 = LCRNG_Next() % 5;

        if (poffin->attributes[v1 + 1] != 0) {
            continue;
        }

        poffin->attributes[v1 + 1] = 2;
        ++v0;
    } while (v0 < 3);

    POFFIN_FIELDS(poffin)->type = POFFIN_TYPE_FOUL;
    POFFIN_FIELDS(poffin)->smoothness = param1;
}

int Poffin_MakePoffin(Poffin *poffin, u8 *flavors, u8 smoothness, BOOL isFoul)
{
    int i, flavorCount = 0;
    u8 poffinFlavors[FLAVOR_MAX];
    u8 isMild = FALSE, type = 0;

    type = POFFIN_TYPE_FOUL;

    if (isFoul) {
        Poffin_MakeFoul(poffin, smoothness);
        return type;
    }

    for (i = 0; i < FLAVOR_MAX; i++) {
        if (flavors[i]) {
            if (flavors[i] >= 50) {
                isMild = TRUE;
            }

            poffinFlavors[flavorCount++] = i;
        }
    }

    switch (flavorCount) {
    case 0:
        Poffin_MakeFoul(poffin, smoothness);
        return type;
    case 1:
        type = poffinFlavors[0] * FLAVOR_MAX + poffinFlavors[0];
        break;
    case 2:
        if (flavors[poffinFlavors[0]] >= flavors[poffinFlavors[1]]) {
            type = poffinFlavors[0] * FLAVOR_MAX + poffinFlavors[1];
        } else {
            type = poffinFlavors[1] * FLAVOR_MAX + poffinFlavors[0];
        }
        break;
    case 3:
        type = POFFIN_TYPE_RICH;
        break;
    case 4:
    case 5:
        type = POFFIN_TYPE_OVERRIPE;
        break;
    }

    if (isMild) {
        type = POFFIN_TYPE_MILD;
    }

    for (i = 0; i < FLAVOR_MAX; i++) {
        poffin->attributes[i + 1] = flavors[i];
    }

    POFFIN_FIELDS(poffin)->type = type;
    POFFIN_FIELDS(poffin)->smoothness = smoothness;

    return type;
}

void Poffin_StoreAttributesToArray(Poffin *poffin, u8 *dest)
{
    dest[0] = POFFIN_FIELDS(poffin)->type;
    dest[1] = POFFIN_FIELDS(poffin)->spiciness;
    dest[2] = POFFIN_FIELDS(poffin)->dryness;
    dest[3] = POFFIN_FIELDS(poffin)->sweetness;
    dest[4] = POFFIN_FIELDS(poffin)->bitterness;
    dest[5] = POFFIN_FIELDS(poffin)->sourness;
    dest[6] = POFFIN_FIELDS(poffin)->smoothness;
}

u8 Poffin_CalcLevel(Poffin *poffin)
{
    u8 v0 = 0;
    u8 level = 0;

    v0 = POFFIN_FIELDS(poffin)->type / 5;

    switch (v0) {
    case 0:
        level = POFFIN_FIELDS(poffin)->spiciness;
        break;
    case 1:
        level = POFFIN_FIELDS(poffin)->dryness;
        break;
    case 2:
        level = POFFIN_FIELDS(poffin)->sweetness;
        break;
    case 3:
        level = POFFIN_FIELDS(poffin)->bitterness;
        break;
    case 4:
        level = POFFIN_FIELDS(poffin)->sourness;
        break;
    case 5:
    default:
        level = POFFIN_FIELDS(poffin)->spiciness;

        if (level < POFFIN_FIELDS(poffin)->dryness) {
            level = POFFIN_FIELDS(poffin)->dryness;
        }

        if (level < POFFIN_FIELDS(poffin)->sweetness) {
            level = POFFIN_FIELDS(poffin)->sweetness;
        }

        if (level < POFFIN_FIELDS(poffin)->bitterness) {
            level = POFFIN_FIELDS(poffin)->bitterness;
        }

        if (level < POFFIN_FIELDS(poffin)->sourness) {
            level = POFFIN_FIELDS(poffin)->sourness;
        }
        break;
    }

    if (level > 99) {
        return 99;
    }

    return level;
}

PoffinCase *SaveData_GetPoffinCase(SaveData *saveData)
{
    return SaveData_SaveTable(saveData, SAVE_TABLE_ENTRY_POFFINS);
}

int PoffinCase_SaveSize(void)
{
    return sizeof(PoffinCase);
}

void PoffinCase_Init(PoffinCase *poffinCase)
{
    int i;

    for (i = 0; i < MAX_POFFINS; i++) {
        Poffin_Clear(&poffinCase->slot[i]);
    }
}

u16 PoffinCase_GetEmptySlot(PoffinCase *poffinCase)
{
    u16 i;

    for (i = 0; i < MAX_POFFINS; i++) {
        if (POFFIN_OBJ(poffinCase->slot[i])->type == TYPE_NONE) {
            return i;
        }
    }

    return POFFIN_NONE;
}

u16 PoffinCase_AddPoffin(PoffinCase *poffinCase, Poffin *poffin)
{
    u16 slotId = PoffinCase_GetEmptySlot(poffinCase);

    if (slotId == POFFIN_NONE) {
        return slotId;
    }

    Poffin_Copy(poffin, &poffinCase->slot[slotId]);
    return slotId;
}

BOOL PoffinCase_ClearSlot(PoffinCase *poffinCase, u16 slot)
{
    if (slot >= MAX_POFFINS) {
        return FALSE;
    }

    Poffin_Clear(&poffinCase->slot[slot]);
    return TRUE;
}

static u16 PoffinCase_GetFirstValidPoffin(PoffinCase *poffinCase, u16 startingSlot)
{
    u16 i;

    for (i = startingSlot; i < MAX_POFFINS; i++) {
        if (POFFIN_OBJ(poffinCase->slot[i])->type != TYPE_NONE) {
            return i;
        }
    }

    return POFFIN_NONE;
}

void PoffinCase_Compact(PoffinCase *poffinCase)
{
    u16 i, unused;
    u16 nextValidSlotNum, emptySlotNum, targetSlotNum;
    u16 remainingSlots = MAX_POFFINS;

    for (i = 0; i < remainingSlots; i++) {
        // skip over occupied slots
        if (POFFIN_OBJ(poffinCase->slot[i])->type != TYPE_NONE) {
            continue;
        }

        // found an empty slot, proceed
        emptySlotNum = i;
        nextValidSlotNum = PoffinCase_GetFirstValidPoffin(poffinCase, emptySlotNum); // find the next valid poffin

        if (nextValidSlotNum == POFFIN_NONE) {
            break;
        }

        targetSlotNum = nextValidSlotNum;
        nextValidSlotNum = targetSlotNum - emptySlotNum;

        for (; targetSlotNum < remainingSlots; emptySlotNum++, targetSlotNum++) {
            Poffin_Copy(&poffinCase->slot[targetSlotNum], &poffinCase->slot[emptySlotNum]);
            Poffin_Clear(&poffinCase->slot[targetSlotNum]);
        }

        remainingSlots -= nextValidSlotNum;
    }
}

void PoffinCase_CopyPoffinFromSlot(PoffinCase *poffinCase, u16 srcSlot, Poffin *poffin)
{
    if (srcSlot >= MAX_POFFINS) {
        Poffin_Clear(poffin);
        return;
    }

    Poffin_Copy(&poffinCase->slot[srcSlot], poffin);
    return;
}

Poffin *PoffinCase_AllocateForSlot(PoffinCase *poffinCase, u16 destSlot, enum HeapID heapID)
{
    Poffin *poffin = Poffin_New(heapID);

    if (destSlot >= MAX_POFFINS) {
        Poffin_Clear(poffin);
        return NULL;
    }

    Poffin_Copy(&poffinCase->slot[destSlot], poffin);
    return poffin;
}

u16 PoffinCase_CountFilledSlots(PoffinCase *poffinCase)
{
    u16 j = 0, i;

    for (i = 0; i < MAX_POFFINS; i++) {
        if (Poffin_HasValidFlavor(&poffinCase->slot[i])) {
            ++j;
        }
    }

    return j;
}

u16 PoffinCase_CountEmptySlots(PoffinCase *poffinCase)
{
    u16 i, j = 0;

    for (i = 0; i < MAX_POFFINS; i++) {
        if (POFFIN_OBJ(poffinCase->slot[i])->type == TYPE_NONE) {
            j++;
        }
    }

    return j;
}

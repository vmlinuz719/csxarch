#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "pagemap.h"

void init_page_map(em3_page_map_t *pm) {
    srand(time(NULL));

    pm->num_present = 0;
    pm->num_sticky = 0;
    pm->next_victim = 0;

    for (int i = 0; i < PM_MAP_SIZE; i++) {
        em3_page_map_entry_t *ent = &(pm->map[i]);

        ent->va = 0;
        ent->pa = 0;
        ent->asid = 0;
        ent->flags = 0;
    }

     for (int i = 0; i < PM_CACHE_SIZE; i++) {
         pm->cache[i] = (em3_page_cache_entry_t){-1, 0};
     }
}

void choose_victim(em3_page_map_t *pm) {
    if (pm->num_present != pm->num_sticky)
        pm->next_victim = rand() % (
            pm->num_present - pm->num_sticky
        )
        + pm->num_sticky;
}

int pin(em3_page_map_t *pm, int index) {
    if (
        index < pm->num_sticky || index >= pm->num_present
        || pm->num_sticky >= PM_MAX_PINNED
    ) return PM_ERROR_MAX_PINNED;

    int pin_index = pm->num_sticky++;

    if (index != pin_index) {
        em3_page_map_entry_t temp = pm->map[pin_index];
        pm->map[pin_index] = pm->map[index];
        pm->map[index] = temp;
    }

    choose_victim(pm);
    return pin_index;
}

int unpin(em3_page_map_t *pm, int index) {
    if (index >= pm->num_sticky) return PM_ERROR_MAX_PINNED;
    if (index >= pm->num_present) return PM_ERROR_INDEX;

    int pin_index = --pm->num_sticky;

    if (index != pin_index) {
        em3_page_map_entry_t temp = pm->map[pin_index];
        pm->map[pin_index] = pm->map[index];
        pm->map[index] = temp;
    }

    choose_victim(pm);
    return pin_index;
}

int lookup(em3_page_map_t *pm, uint64_t va, uint16_t asid) {
    uint64_t cache_tag = va % PM_CACHE_SIZE;
    int cached_index = pm->cache[cache_tag].index;
    if (cached_index != -1
        && (pm->map[cached_index].flags & PM_VALID)
        && pm->map[cached_index].va == va
        && (
            pm->cache[cache_tag].asid == asid
            || pm->cache[cache_tag].asid == 0
        )
    ) {
        return cached_index;
    }
    else {
        pm->cache[cache_tag].index = -1;
    }

    for (int i = 0; i < pm->num_present; i++) {
        em3_page_map_entry_t *ent = &(pm->map[i]);

        if ((ent->flags & PM_VALID) &&
            va == ent->va &&
            (
                asid == ent->asid
                || ent->asid == 0
            )
        ) {
            pm->cache[cache_tag].index = i;
            pm->cache[cache_tag].asid = asid;
            return i;
        }
    }

    return -1;
}

int lookup_no_global(em3_page_map_t *pm, uint64_t va, uint16_t asid) {
    uint64_t cache_tag = va % PM_CACHE_SIZE;
    int cached_index = pm->cache[cache_tag].index;
    if (cached_index != -1
        && (pm->map[cached_index].flags & PM_VALID)
        && pm->map[cached_index].va == va
        && (
            pm->map[cached_index].asid == asid
        )
    ) {
        return cached_index;
    }

    for (int i = 0; i < pm->num_present; i++) {
        em3_page_map_entry_t *ent = &(pm->map[i]);

        if ((ent->flags & PM_VALID) &&
            va == ent->va &&
            (
                asid == ent->asid
                // || ent->asid == 0
            )
        ) {
            // pm->cache[cache_tag].index = i;
            // pm->cache[cache_tag].asid = asid;
            return i;
        }
    }

    return -1;
}

int lookup_any_local(em3_page_map_t *pm, uint64_t va) {
    uint64_t cache_tag = va % PM_CACHE_SIZE;
    int cached_index = pm->cache[cache_tag].index;
    if (cached_index != -1
        && (pm->map[cached_index].flags & PM_VALID)
        && pm->map[cached_index].va == va
        && (
            pm->cache[cache_tag].asid != 0
        )
    ) {
        return cached_index;
    }
    else {
        pm->cache[cache_tag].index = -1;
    }

    for (int i = 0; i < pm->num_present; i++) {
        em3_page_map_entry_t *ent = &(pm->map[i]);

        if ((ent->flags & PM_VALID) &&
            va == ent->va &&
            (
                ent->asid != 0
            )
        ) {
            // pm->cache[cache_tag].index = i;
            // pm->cache[cache_tag].asid = asid;
            return i;
        }
    }

    return -1;
}

int delete(em3_page_map_t *pm, int index) {
    if (index >= pm->num_present) return PM_ERROR_INDEX;

    int index_unpin = unpin(pm, index);
    if (index_unpin != -1) index = index_unpin;

    int top = pm->num_present - 1;

    if (index < top) {
        pm->map[index] = pm->map[top];
    }
    pm->map[top].va = 0;
    pm->map[top].flags = 0;

    pm->num_present--;
    return 0;
}

int upsert(em3_page_map_t *pm, em3_page_map_entry_t ent) {
    // case 0: invalidate a page
    if (!(ent.flags & (1 << PM_BIT_VALID))) {
        int victim = lookup(pm, ent.va, ent.asid);
        return delete(pm, victim);
    }

    // case 1: page already mapped
    // clobbers global mapping
    // error if mapping global page that already has local mappings
    if (ent.asid == 0 && lookup_any_local(pm, ent.va) >= 0) {
        return PM_ERROR_GLOBAL_OVERWRITE;
    }
    
    int existing = lookup(pm, ent.va, ent.asid);
    if (existing >= 0) {
        // are we pinning this?
        if (
            (ent.flags & (1 << PM_BIT_STICKY))
            && !(pm->map[existing].flags & (1 << PM_BIT_STICKY))
        ) {
            existing = pin(pm, existing);
            if (existing < 0) return existing;
        }

        // are we unpinning this?
        else if (
            !(ent.flags & (1 << PM_BIT_STICKY))
            && (pm->map[existing].flags & (1 << PM_BIT_STICKY))
        ) {
            existing = unpin(pm, existing);
            if (existing < 0) return existing;
        }

        pm->map[existing] = ent;
        return 1;
    }

    // case 2: page not mapped, map not full
    if (pm->num_present < PM_MAP_SIZE) {
        // are we asking to pin more than PM_MAX_PINNED?
        if (
            (ent.flags & (1 << PM_BIT_STICKY))
            && pm->num_sticky >= PM_MAX_PINNED
        ) return PM_ERROR_MAX_PINNED;

        int index = pm->num_present++;
        pm->map[index] = ent;

        if (ent.flags & (1 << PM_BIT_STICKY)) pin(pm, index);
        else if (pm->num_present == PM_MAP_SIZE) choose_victim(pm);

        return 2;
    }

    // case 3: page not mapped, map full
    else {
        // are we asking to pin more than PM_MAX_PINNED?
        if (
            (ent.flags & (1 << PM_BIT_STICKY))
            && pm->num_sticky >= PM_MAX_PINNED
        ) return PM_ERROR_MAX_PINNED;

        pm->map[pm->next_victim] = ent;
        if (ent.flags & (1 << PM_BIT_STICKY)) pin(pm, pm->next_victim);
        else choose_victim(pm);
        return 3;
    }
}

void print_flags(uint16_t flags) {
    const char flag_names[] = "XWRxwBCDSPV";
    for (int i = 0; i < 10; i++)
        printf("%c", ((flags >> (10 - i)) & 1) ? flag_names[i] : ' ');
    printf("\n");
}

void print_table(em3_page_map_t *pm) {
    printf("SGR0003I  #  AS            VA            PA TAG\n");
    //printf("==========================================\n");
    for (int i = 0; i < PM_MAP_SIZE; i++) {
        if (pm->map[i].flags & (1 << PM_BIT_VALID)) {
            printf("        %3d %3X %13lX %13lX ",
                i, pm->map[i].asid, pm->map[i].va, pm->map[i].pa
            );
            print_flags(pm->map[i].flags);
        }
    }
}

#ifdef _DEBUG_PAGE_MAP_
int main(int argc, char *argv[]) {
    em3_page_map_t pm;
    init_page_map(&pm);

    printf("Filling page map\n");
    for (int i = 0; i < 16; i++) {
        upsert(&pm, (em3_page_map_entry_t){rand(), rand(), rand() & 0xFFF, 0x1});
    }
    print_table(&pm);

    printf("\nPinning some pages\n");
    choose_victim(&pm);
    for (int i = 0; i < 4; i++) {
        em3_page_map_entry_t temp = pm.map[pm.next_victim];
        upsert(
            &pm,
            (em3_page_map_entry_t)
            {temp.va, temp.pa, temp.asid, temp.flags | (1 << PM_BIT_STICKY)}
        );
    }
    print_table(&pm);

    printf("\nUnpinning first page\n");
    {
        em3_page_map_entry_t temp = pm.map[0];
        upsert(
            &pm,
            (em3_page_map_entry_t)
            {temp.va, temp.pa, temp.asid, 1}
        );
    }
    print_table(&pm);

    printf("\nUnpinning last page\n");
    {
        em3_page_map_entry_t temp = pm.map[2];
        upsert(
            &pm,
            (em3_page_map_entry_t)
            {temp.va, temp.pa, temp.asid, 1}
        );
    }
    print_table(&pm);

    return 0;
}
#endif


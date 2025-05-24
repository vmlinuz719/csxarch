#include <stdint.h>

#include "uthash.h"
#include "tlb.h"

int tlb_set(struct tlb *tlb, int i, uint64_t k) {
    struct tlb_entry *e = &(tlb->entries[i]);

    if (k == e->key) return 0;

    struct tlb_entry *o = NULL;
    HASH_FIND_INT64(tlb->hash, &k, o);
    if (o != NULL) return -1;

    o = NULL;
    HASH_FIND_INT64(tlb->hash, &(e->key), o);
    if (o != NULL) {
        HASH_DEL(tlb->hash, o);
    }

    e->key = k;
    if (PGID(k)) {
        HASH_ADD_INT64(tlb->hash, key, e);
    }

    return 0;
}

int tlb_lookup(struct tlb *tlb, uint64_t k) {
    struct tlb_entry *o = NULL;
    HASH_FIND_INT64(tlb->hash, &k, o);
    if (o == NULL) return -1;
    return o - tlb->entries;
}

void tlb_invpgid(struct tlb *tlb, uint64_t id) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (PGID(tlb->entries[i].key) == id) {
            tlb_set(tlb, i, 0);
        }
    }
}

void tlb_destroy(struct tlb *tlb) {
    struct tlb_entry *current, *tmp;
    HASH_ITER(hh, tlb->hash, current, tmp) {
        HASH_DEL(tlb->hash, current);
    }
}
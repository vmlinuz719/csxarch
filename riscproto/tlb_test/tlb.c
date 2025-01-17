#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

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

int main(int argc, char *argv[]) {
    struct tlb tlb;
    memset(&tlb, 0, sizeof(tlb));

    tlb_set(&tlb, 0, 0xDEADBEEF);
    tlb_set(&tlb, 5, 0x51C1B1D1);
    tlb_set(&tlb, 7, 0x51C1B1D2);

    printf("Lookup 0xDEADBEEF: %d\n", tlb_lookup(&tlb, 0xDEADBEEF));
    printf("Lookup 0x51C1B1D1: %d\n", tlb_lookup(&tlb, 0x51C1B1D1));
    printf("Lookup 0x51C1B1D2: %d\n", tlb_lookup(&tlb, 0x51C1B1D2));
    printf("Lookup 0x11111111: %d\n", tlb_lookup(&tlb, 0x11111111));

    tlb_set(&tlb, 0, 0xFEEDF00D);
    tlb_set(&tlb, 5, 0);

    printf("====================\n");

    printf("Lookup 0xDEADBEEF: %d\n", tlb_lookup(&tlb, 0xDEADBEEF));
    printf("Lookup 0xFEEDF00D: %d\n", tlb_lookup(&tlb, 0xFEEDF00D));
    printf("Lookup 0x51C1B1D1: %d\n", tlb_lookup(&tlb, 0x51C1B1D1));
    printf("Lookup 0x51C1B1D2: %d\n", tlb_lookup(&tlb, 0x51C1B1D2));

    printf("====================\n");

    tlb_set(&tlb, 10, 0x1234000D);
    tlb_set(&tlb, 11, 0x5678000E);
    tlb_set(&tlb, 12, 0x9ABC000D);
    tlb_set(&tlb, 13, 0xDEF0000E);

    tlb_set(&tlb, 14, 0x1234000E);
    tlb_set(&tlb, 15, 0x5678000D);
    tlb_set(&tlb, 16, 0x9ABC000E);
    tlb_set(&tlb, 17, 0xDEF0000D);

    printf("Doing 500M lookups (1 miss, 4 hit)\n");

    int k = 0;

    for (int i = 0; i < 99999999; i++) {
        k += tlb_lookup(&tlb, 0xDEADBEEF);
        k += tlb_lookup(&tlb, 0x1234000D);
        k += tlb_lookup(&tlb, 0x5678000D);
        k += tlb_lookup(&tlb, 0x9ABC000D);
        k += tlb_lookup(&tlb, 0xDEF0000D);
    }

    printf("%d\n", k);

    printf("====================\n");

    tlb_invpgid(&tlb, 0xD);
    printf("Lookup 0x1234000D: %d\n", tlb_lookup(&tlb, 0x1234000D));
    printf("Lookup 0x1234000E: %d\n", tlb_lookup(&tlb, 0x1234000E));

    return 0;
}
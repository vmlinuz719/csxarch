#ifndef _TLB_
#define _TLB_

#include <stdint.h>
#include "uthash.h"

#define TLB_SIZE 128

#define PGID(x) ((x) & 0x3FF) // 10-bit Page Group Identifiers
#define PTAG(x) ((x) & 0xFFFFFFFFFFFFFC00) // 54-bit Page Tag
#define POFF(x) (PGID(x))

#define PFLAG_P(x) ((x) & 1)
#define PFLAG_W(x) ((x) & 2)
#define PFLAG_NC(x) ((x) & 4)

struct tlb_entry {
    uint64_t key;
    uint64_t value;
    UT_hash_handle hh;
};

struct tlb {
    struct tlb_entry entries[TLB_SIZE];
    struct tlb_entry *hash;
};

int tlb_set(struct tlb *tlb, int i, uint64_t k);
int tlb_lookup(struct tlb *tlb, uint64_t k);
void tlb_invpgid(struct tlb *tlb, uint64_t id);
void tlb_destroy(struct tlb *tlb);

#endif


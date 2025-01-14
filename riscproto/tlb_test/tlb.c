#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "uthash.h"

#define TLB_SIZE 128

#define PGID(x) ((x) & 0x3FF) // 10-bit Page Group Identifiers

struct tlb_entry {
    uint64_t key;
    uint64_t value;
    UT_hash_handle hh;
};

struct tlb {
    struct tlb_entry entries[TLB_SIZE];
    struct tlb_entry *hash;
};



int main(int argc, char *argv[]) {
    return 0;
}
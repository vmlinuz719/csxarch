#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "uthash.h"

#define TLB_SIZE 128

struct cam_index {
    uint64_t key;
    int index;
    UT_hash_handle hh;
};

struct cam_entry {
    uint64_t key;
    uint64_t value;
    int valid;
};

struct cam {
    struct cam_entry entries[TLB_SIZE];
    struct cam_index *table;
};

int main(int argc, char *argv[]) {
    return 0;
}
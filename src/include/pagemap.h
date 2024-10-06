#ifndef _PAGE_MAP_
#define _PAGE_MAP_

#include <stdint.h>

#define PM_MAP_SIZE 512
#define PM_CACHE_SIZE 8192
#define PM_MAX_PINNED 256

#define PM_BIT_VALID 0
#define PM_BIT_PRESENT 1
#define PM_BIT_STICKY 2
#define PM_BIT_DIRTY 3
#define PM_BIT_NOCACHE 4
#define PM_BIT_WRITEBACK 5
#define PM_BIT_WRITE 6
#define PM_BIT_EXEC 7
#define PM_BIT_READ 8
#define PM_BIT_PRIVWRITE 9
#define PM_BIT_PRIVEXEC 10

#define PM_VALID (1 << PM_BIT_VALID)
#define PM_PRESENT (1 << PM_BIT_PRESENT)
#define PM_STICKY (1 << PM_BIT_STICKY)
#define PM_DIRTY (1 << PM_BIT_DIRTY)
#define PM_NOCACHE (1 << PM_BIT_NOCACHE)
#define PM_WRITEBACK (1 << PM_BIT_WRITEBACK)
#define PM_WRITE (1 << PM_BIT_WRITE)
#define PM_EXEC (1 << PM_BIT_EXEC)
#define PM_READ (1 << PM_BIT_READ)
#define PM_PRIVWRITE (1 << PM_BIT_PRIVWRITE)
#define PM_PRIVEXEC (1 << PM_BIT_PRIVEXEC)

#define PM_ERROR_MAX_PINNED -1
#define PM_ERROR_GLOBAL_OVERWRITE -2
#define PM_ERROR_INDEX -3

typedef struct {
    uint64_t va;
    uint64_t pa;
    uint16_t asid;
    uint16_t flags;
} em3_page_map_entry_t;

typedef struct {
    int index;
    uint16_t asid;
} em3_page_cache_entry_t;

typedef struct {
    int num_present;
    int num_sticky;
    int next_victim;
    em3_page_map_entry_t map[PM_MAP_SIZE];
    em3_page_cache_entry_t cache[PM_CACHE_SIZE];
} em3_page_map_t;

void init_page_map(em3_page_map_t *pm);
int lookup(em3_page_map_t *pm, uint64_t va, uint16_t asid);
int lookup_no_global(em3_page_map_t *pm, uint64_t va, uint16_t asid);
int upsert(em3_page_map_t *pm, em3_page_map_entry_t ent);
void print_table(em3_page_map_t *pm);
void print_flags(uint16_t flags);

#endif


#ifndef _MMIO_
#define _MMIO_

#include <stdint.h>

#include "error.h"

#define MMIO_BASE 0xFFFF000000000000
#define MMIO_UNIT(n) (((n) >> 32) & 0xFFFF)
#define MMIO_REGISTER(n) ((n) & 0xFFFFFFFF)

typedef struct {
    void *ctx;
    uint64_t (*read)
        (void *, int /* size */, uint32_t /* addr */, em3_access_error_t *);
    void (*write)
        (void *, int /* size */, uint32_t /* addr */, uint64_t /* data */,
            em3_access_error_t *);
    uint64_t (*sense)
        (void *, int /* reg */);
    uint64_t (*command)
        (void *, uint64_t /* command */, em3_access_error_t *);
    void (*destroy) (void *);
} mmio_unit_t;

#endif


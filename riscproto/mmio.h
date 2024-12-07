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
        (void *, int /* size */, uint32_t /* addr */, lcca_error_t *);
    void (*write)
        (void *, int /* size */, uint32_t /* addr */, uint64_t /* data */,
            lcca_error_t *);
    uint64_t (*sense)
        (void *, int /* reg */);
    uint64_t (*command)
        (void *, int /* reg */, uint64_t /* command */, lcca_error_t *);
    void (*destroy) (void *);
} mmio_unit_t;

typedef struct lcca_bus_t {
    pthread_mutex_t *cas_lock;

    mmio_unit_t *mmio;
    uint64_t num_units;

    uint8_t *memory;
    uint64_t mem_limit;
} lcca_bus_t;

uint64_t read_u1b(lcca_bus_t *r, uint64_t a, lcca_error_t *e);
uint64_t read_u2b(lcca_bus_t *r, uint64_t a, lcca_error_t *e);
uint64_t read_u4b(lcca_bus_t *r, uint64_t a, lcca_error_t *e);
uint64_t read_8b(lcca_bus_t *r, uint64_t a, lcca_error_t *e);
void write_1b(lcca_bus_t *r, uint64_t a, uint64_t v, lcca_error_t *e);
void write_2b(lcca_bus_t *r, uint64_t a, uint64_t v, lcca_error_t *e);
void write_4b(lcca_bus_t *r, uint64_t a, uint64_t v, lcca_error_t *e);
void write_8b(lcca_bus_t *r, uint64_t a, uint64_t v, lcca_error_t *e);

#endif


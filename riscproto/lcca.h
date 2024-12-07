#ifndef _LCCA_
#define _LCCA_

#include <stdint.h>
#include <pthread.h>

#include "error.h"

typedef struct lcca_t {
    lcca_bus_t *bus;
    pthread_t *run;

    uint64_t regs[64];
    uint64_t pc;

    void (*operations[16]) (struct lcca_t *, uint32_t);
} lcca_t;

#endif


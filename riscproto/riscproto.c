#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

#include "byteswap.h"
#include "error.h"
#include "mmio.h"

typedef struct lcca_t {
    lcca_bus_t *bus;

    uint64_t regs[64];
    uint64_t pc;

    void (*operations[16]) (struct lcca_t *, uint32_t);
} lcca_t;


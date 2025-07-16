#ifndef _CONSOLE_
#define _CONSOLE_

#include <stdint.h>

#define CONSOLE_IRQ 4

#define CON_READ_READY (1 << 7)
#define CON_WRITE_READY (1 << 6)
#define CON_INPUT_FULL (1 << 1)

#define ROM_BASE 0x10000
#define ROM_SIZE 0x2000
#define BUF_SIZE 128
#define PRT_BUF_SIZE 32

#include "mmio.h"
#include "error.h"
#include "lcca.h"

void init_console(mmio_unit_t *u, lcca_t *cpu);

#endif


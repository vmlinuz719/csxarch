#ifndef _BLINK_
#define _BLINK_

#include <stdint.h>

#include "mmio.h"
#include "error.h"
#include "lcca.h"

void init_blink(mmio_unit_t *u, lcca_t *cpu);

#endif


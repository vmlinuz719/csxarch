#ifndef _DISK_
#define _DISK_

#include <stdint.h>

#include "mmio.h"
#include "error.h"
#include "csx.h"

#define DISK_RC_SUCCESS         1
#define DISK_RC_SUCCESS_EXTRA   2
#define DISK_RC_UNSUPPORTED     4
#define DISK_RC_MEDIUM_CHECK    8
#define DISK_RC_MACHINE_CHECK   16
#define DISK_RC_NOT_READY       32
#define DISK_RC_WRITE_PROTECT   64
#define DISK_RC_SECTOR_OVERRUN  128
#define DISK_RC_RESET           256
#define DISK_RC_BUS_ERROR       512

#define DISK_NUM_LUN            64

typedef struct {
    FILE *image;
    size_t sector_size, num_sectors;
    int write_protect;
} disk_lun_t;

void init_disk(mmio_unit_t *u, em3_regs_t *cpu);
int disk_init_lun(mmio_unit_t *u, int lun, char *fname, size_t sector_size, int ro);

#endif


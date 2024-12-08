#ifndef _LCCA_
#define _LCCA_

#include <stdint.h>
#include <pthread.h>

#include "error.h"

#define EXT4(x)             ((x) & (1L << 3)  ? (x) | 0xFFFFFFFFFFFFFFF0 : (x))
#define EXT8(x)             ((x) & (1L << 7)  ? (x) | 0xFFFFFFFFFFFFFF00 : (x))
#define EXT10(x)            ((x) & (1L << 9)  ? (x) | 0xFFFFFFFFFFFFFC00 : (x))
#define EXT12(x)            ((x) & (1L << 11) ? (x) | 0xFFFFFFFFFFFFF000 : (x))
#define EXT15(x)            ((x) & (1L << 14) ? (x) | 0xFFFFFFFFFFFF8000 : (x))
#define EXT16(x)            ((x) & (1L << 15) ? (x) | 0xFFFFFFFFFFFF0000 : (x))
#define EXT20(x)            ((x) & (1L << 19) ? (x) | 0xFFFFFFFFFFF00000 : (x))
#define EXT32(x)            ((x) & (1L << 31) ? (x) | 0xFFFFFFFF00000000 : (x))
#define EXT48(x)            ((x) & (1L << 47) ? (x) | 0xFFFF000000000000 : (x))

#define OPCODE(x)           ((x) >> 28)
#define RA(x)               (((x) >> 23) & 0x1F)
#define RB(x)               (((x) >> 5) & 0x1F)
#define RC(x)               ((x) & 0x1F)
#define FN(x)               (((x) >> 20) & 0x7)

#define RR_IMM(x)           (((x) >> 10) & 0x3FF)

#define BR_DISP(x)          ((x) & 0xFFFFF)

#define LS_DISP(x)          (((x) >> 5) & 0x7FFF)

typedef struct lcca_t {
    lcca_bus_t *bus;
    pthread_t *run;

    uint64_t regs[32];
    uint64_t c_regs[32];
    uint64_t pc;

    void (*operations[16]) (struct lcca_t *, uint32_t);
} lcca_t;

uint64_t get_reg_l(lcca_t *, int);
void set_reg_l(lcca_t *, int, uint64_t);

uint64_t get_reg_q(lcca_t *, int);
void set_reg_q(lcca_t *, int, uint64_t);

void lcca32_rr_0(lcca_t *, uint32_t);
void lcca32_br_1(lcca_t *, uint32_t);
void lcca32_ls_2(lcca_t *, uint32_t);

#endif


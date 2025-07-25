#ifndef _LCCA_
#define _LCCA_

#include <stdint.h>
#include <pthread.h>

#include "error.h"
#include "mmio.h"
#include "tlb.h"

#define EXT4(x)             ((x) & (1L << 3)  ? (x) | 0xFFFFFFFFFFFFFFF0 : (x))
#define EXT8(x)             ((x) & (1L << 7)  ? (x) | 0xFFFFFFFFFFFFFF00 : (x))
#define EXT10(x)            ((x) & (1L << 9)  ? (x) | 0xFFFFFFFFFFFFFC00 : (x))
#define EXT12(x)            ((x) & (1L << 11) ? (x) | 0xFFFFFFFFFFFFF000 : (x))
#define EXT15(x)            ((x) & (1L << 14) ? (x) | 0xFFFFFFFFFFFF8000 : (x))
#define EXT16(x)            ((x) & (1L << 15) ? (x) | 0xFFFFFFFFFFFF0000 : (x))
#define EXT20(x)            ((x) & (1L << 19) ? (x) | 0xFFFFFFFFFFF00000 : (x))
#define EXT23(x)            ((x) & (1L << 22) ? (x) | 0xFFFFFFFFFF800000 : (x))
#define EXT32(x)            ((x) & (1L << 31) ? (x) | 0xFFFFFFFF00000000 : (x))
#define EXT48(x)            ((x) & (1L << 47) ? (x) | 0xFFFF000000000000 : (x))

#define OPCODE(x)           ((x) >> 28)
#define RA(x)               (((x) >> 23) & 0x1F)
#define RB(x)               (((x) >> 5) & 0x1F)
#define RC(x)               ((x) & 0x1F)
#define FN(x)               (((x) >> 20) & 0x7)
#define EX_FN(x)            (((x) >> 10) & 0x1FFF)

#define RR_IMM(x)           (((x) >> 10) & 0x3FF)

#define IM_IMM(x)           ((x) & 0x7FFFFF)

#define BR_DISP(x)          ((x) & 0xFFFFF)

#define LS_DISP(x)          (((x) >> 5) & 0x7FFF)

#define LSS_DISP(x)         (((x) >> 5) & 0xFFF)
#define LSS_SEL(x)          (((x) >> 17) & 0x7)

#define LGISL2_IMM(x)       (((x) & 0x7FFFFF) | (((x) & 0xF0000000) >> 5))

#define CR_MAX              48

#define CR_PSQ              0
#define CR_PSQ_EI           (1)
#define CR_PSQ_PL           (1 << 1)
#define CR_PSQ_WS           (1 << 2)
#define CR_PSQ_LG           (1 << 3)
#define CR_PSQ_OC           (1 << 4)
#define CR_PSQ_AE           (1 << 5)
#define CR_PSQ_PGID         (0x3FF << 6)
#define GET_PGID(x)         (((x) >> 6) & 0x3FF)
#define CR_PSQ_INTR_ENTRY_MASK 0xFFFFFFFFFFFFFFE0

#define CR_APC              1
#define CR_APSQ             2
#define CR_ASP              3
#define CR_IA               4
#define IA_HANDLER_INSTS    16
#define CR_EIM              5
#define CR_EIP              6
#define EIP_EXTERNAL_INTRS  16
#define CR_FI               7
#define CR_FA               8
#define CR_TIMER            9

#define CR_OB0              16
#define CR_OD0              32
#define CR_OD_P             (1)
#define CR_OD_E             (1 << 1)
#define CR_OD_D             (1 << 2)
#define CR_OD_C             (1 << 3)
#define CR_OD_B             (1 << 4)
#define CR_OD_w             (1 << 5)
#define CR_OD_x             (1 << 6)
#define CR_OD_R             (1 << 7)
#define CR_OD_W             (1 << 8)
#define CR_OD_X             (1 << 9)

#define R_ABI_X8            28
#define R_ABI_X9            29
#define R_ABI_SP            30
#define R_ABI_LR            31

typedef struct lcca_t {
    lcca_bus_t *bus;
    pthread_t *run;
    pthread_mutex_t intr_mutex;
    pthread_cond_t wake;

    pthread_t timer;

    uint64_t regs[32];
    uint64_t c_regs[CR_MAX];
    uint64_t pc;
    uint64_t rng[2];

    uint64_t intr_pending, intr_msg[EIP_EXTERNAL_INTRS];

    uint32_t inst;
    int running, throttle, timer_active, timer_msg;
    pthread_t timer_thread;

    void (*operations[16]) (struct lcca_t *, uint32_t);

    struct tlb tlb;
} lcca_t;

extern const char *psq_bits;
void print_bits(uint64_t n, const char *bits);

uint64_t get_reg_q(lcca_t *, int);
void set_reg_q(lcca_t *, int, uint64_t);

uint64_t shl(uint64_t, uint64_t);
uint64_t shr(uint64_t, uint64_t);
uint64_t sar(uint64_t, uint64_t);
uint64_t sh(uint64_t, int);
uint64_t sha(uint64_t, int);

void error(lcca_t *cpu, lcca_error_t e, uint64_t fi, uint64_t fa);

void intr_internal(lcca_t *cpu, int which, uint64_t fi, uint64_t fa);
void intr_restore(lcca_t *cpu);
void intr_restore_disp(lcca_t *cpu, uint64_t d);

void lcca64_rr_0(lcca_t *, uint32_t);
void lcca64_br_1(lcca_t *, uint32_t);
void lcca64_ls_2(lcca_t *, uint32_t);
void lcca64_ls_3(lcca_t *, uint32_t);
void lcca64_im_4(lcca_t *, uint32_t);
void lcca64_im_5(lcca_t *, uint32_t);
void lcca64_im_6(lcca_t *, uint32_t);
void lcca64_xmu_7(lcca_t *, uint32_t);
void lcca64_ls_a(lcca_t *, uint32_t);
void lcca64_ls_b(lcca_t *, uint32_t);
void lcca64_rr_c(lcca_t *, uint32_t);
void lcca64_ls_e(lcca_t *, uint32_t);

typedef enum {
    READ = CR_OD_R | (1 << 10),
    WRITE = CR_OD_W | CR_OD_w,
    FETCH = CR_OD_X | CR_OD_x
} lcca_access_t;

typedef enum {
    CHAR = 0,
    WORD,
    LONG,
    QUAD
} lcca_size_t;

uint64_t translate(lcca_t *cpu, uint64_t addr, lcca_size_t size, lcca_access_t access_type, lcca_error_t *e);

void *lcca_run(lcca_t *);
void lcca_wake(lcca_t *);
void lcca_intr(void *dev, int intr, uint64_t msg);

void lcca64_print(lcca_t *);

#endif


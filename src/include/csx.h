#ifndef _CSX_
#define _CSX_

#include <stdint.h>
#include <pthread.h>

#include "pagemap.h"
#include "mmio.h"

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
    defined(__LITTLE_ENDIAN__) || \
    defined(__ARMEL__) || \
    defined(__THUMBEL__) || \
    defined(__AARCH64EL__) || \
    defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
#define __CSX_LITTLE_ENDIAN__
#endif

#define NUM_INT 18
#define NMI 16
#define SOFT 17
#define NUM_VEC 64

#define MAJOR_OPCODE(x)     (((x) >> 56) & 0xFF)

#define RR_RD(x)            (((x) >> 52) & 0xF)
#define RR_RS(x)            (((x) >> 48) & 0xF)

#define MINOR_OPCODE_0(x)   RR_RS(x)

#define RR_I8(x)            (((x) >> 48) & 0xFF)

#define EXT05_MINOR(x)      (((x) >> 48) & 0xFF)
#define EXT05_RR_RD(x)      (((x) >> 44) & 0xF)
#define EXT05_RR_RS(x)      (((x) >> 40) & 0xF)
#define EXT05_RR_I8(x)      (((x) >> 32) & 0xFF)
#define EXT05_RC_RB(x)      (((x) >> 40) & 0xF)
#define EXT05_RC_RX(x)      (((x) >> 36) & 0xF)
#define EXT05_RC_RC(x)      (((x) >> 32) & 0xF)

#define RIw_RD(x)           RR_RD(x)
#define RIw_I16(x)          (((x) >> 32) & 0xFFFF)

#define RM_RD(x)            RR_RD(x)
#define RM_RB(x)            RR_RS(x)
#define RM_RX(x)            (((x) >> 44) & 0xF)
#define RM_I12(x)           (((x) >> 32) & 0xFFF)

#define BCASR_RS(x)            (((x) >> 28) & 0xF)
#define BCASR_I12(x)           (((x) >> 16) & 0xFFF)

#define RMl_I16(x)          RIw_I16(x)

#define RB_I8(x)           (((x) >> 32) & 0xFF)

#define B_CC(x)             RR_RD(x)
#define BR_CC(x)            (((x) >> 56) & 0xF)
#define B_RS1(x)            RR_RS(x)
#define B_RS2(x)            RM_RX(x)
#define BR_RS1(x)           RR_RD(x)
#define BR_RS2(x)           RM_RB(x)
#define B_RD(x)             (((x) >> 40) & 0xF)
#define B_RB(x)             (((x) >> 36) & 0xF)
#define B_RX(x)             (((x) >> 32) & 0xF)
#define B_I8(x)             (((x) >> 44) & 0xFF)

#define BL_I16(x)           (((x) >> 16) & 0xFFFF)
#define BL_I8(x)            (((x) >> 44) & 0xFF)
#define BL_I24(x)           (BL_I16(x) | (BL_I8(x) << 16))

#define BIC_I20(x)          (BL_I16(x) | (BR_RS1(x) << 16))

#define RIh_RD(x)           RR_RD(x)
#define RIh_I32(x)          (((x) >> 16) & 0xFFFFFFFF)

#define RIl_RD(x)           RR_RD(x)
#define RIl_I48(x)          ((x) & 0xFFFFFFFFFFFF)

#define EXT4(x)             ((x) & (1L << 3)  ? (x) | 0xFFFFFFFFFFFFFFF0 : (x))
#define EXT8(x)             ((x) & (1L << 7)  ? (x) | 0xFFFFFFFFFFFFFF00 : (x))
#define EXT12(x)            ((x) & (1L << 11) ? (x) | 0xFFFFFFFFFFFFF000 : (x))
#define EXT16(x)            ((x) & (1L << 15) ? (x) | 0xFFFFFFFFFFFF0000 : (x))
#define EXT20(x)            ((x) & (1L << 19) ? (x) | 0xFFFFFFFFFFF00000 : (x))
#define EXT32(x)            ((x) & (1L << 31) ? (x) | 0xFFFFFFFF00000000 : (x))
#define EXT48(x)            ((x) & (1L << 47) ? (x) | 0xFFFF000000000000 : (x))

#define MAX_UNPRIV_READ 15
#define MAX_UNPRIV_WRITE 15

#define CR_PSW 16 // Processor Status Word

#define INST_LEN(r, n) set_reg((r), CR_PSW, \
	(get_reg((r), CR_PSW) & 0xFFFFFFFFFFFFFF00) | (n))

#define GET_PSW_CTR(r) ((get_reg((r), CR_PSW) & 0xFFFF0000) >> 16)
#define SET_PSW_CTR(r, n) set_reg((r), CR_PSW, \
	(get_reg((r), CR_PSW) & 0xFFFFFFFF0000FFFF) | ((n) << 16))
    
#define GET_PSW_TMP(r) ((get_reg((r), CR_PSW) & 0xFF00000000) >> 32)
#define SET_PSW_TMP(r, n) set_reg((r), CR_PSW, \
	(get_reg((r), CR_PSW) & 0xFFFFFF00FFFFFFFF) | ((n) << 32))

#define GET_PSW_PRI(r) ((get_reg((r), CR_PSW) & 0xF000000000000000) >> 60)
#define SET_PSW_PRI(r, n) set_reg((r), CR_PSW, \
	(get_reg((r), CR_PSW) & 0x0FFFFFFFFFFFFFFF) | ((n) << 60))
    
#define GET_PSW_WST(r) ((get_reg((r), CR_PSW) & 0x0001000000000000) >> 48)
#define SET_PSW_WST(r, n) set_reg((r), CR_PSW, \
	(get_reg((r), CR_PSW) & 0xFFFEFFFFFFFFFFFF) | ((n) << 48))

/* ---------------------------------------------------------------------------- */

#define NEW_CR_SR 17 // Supervisor Return
#define NEW_CR_SRPSW 18 // Supervisor Return PSW
#define NEW_CR_EADDR 19 // Exception Address
#define NEW_CR_ETYPE 20 // Exception Type
#define NEW_CR_TIMER 21 // Interrupt Timer

#define GET_NEW_PSW_PRI(r) (((get_reg((r), CR_PSW) & 0x4000000000000000) >> 62) ? 15 : 0)
#define SET_NEW_PSW_PRI(r, n) set_reg((r), CR_PSW, \
	(get_reg((r), CR_PSW) & 0xBFFFFFFFFFFFFFFF) | ((uint64_t)(n) << 62))

#define GET_NEW_PSW_WST(r) ((get_reg((r), CR_PSW) & 0x2000000000000000) >> 61)
#define SET_NEW_PSW_WST(r, n) set_reg((r), CR_PSW, \
	(get_reg((r), CR_PSW) & 0xDFFFFFFFFFFFFFFF) | ((uint64_t)(n) << 61))

#define NEW_CR_PSW_PL (1L << 60)
#define NEW_CR_PSW_ASID 0x0FFF000000000000

typedef struct em3_regs_t {
    em3_page_map_t map;
    
    mmio_unit_t *mmio;
    uint64_t num_units;
    
    uint64_t regs[32];
    
    uint64_t instruction_buffer;

    uint8_t *memory;
    uint64_t mem_limit;
    
    uint64_t pc;
    uint64_t count;
    uint64_t fault_addr;
    int increment;
    uint16_t fault_sense;

    pthread_cond_t wake;

    pthread_mutex_t intr_lock;
    int any_pending;
    int intr[NUM_INT];
    void (*intr_ack[NUM_INT])(struct em3_regs_t *, int);
    int timer_active;
    
    pthread_mutex_t cas_lock;

    pthread_t monitor_thread, timer_thread;
} em3_regs_t;

int intr(em3_regs_t *r, int which, int vector);
void default_iack(em3_regs_t *r, int which);

uint64_t get_reg(em3_regs_t *r, size_t n);
void set_reg(em3_regs_t *r, size_t n, uint64_t v);

uint64_t read_u1b(em3_regs_t *r, uint64_t a, em3_access_error_t *e);
uint64_t read_u2b(em3_regs_t *r, uint64_t a, em3_access_error_t *e);
uint64_t read_u4b(em3_regs_t *r, uint64_t a, em3_access_error_t *e);
uint64_t read_8b(em3_regs_t *r, uint64_t a, em3_access_error_t *e);
void write_1b(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e);
void write_2b(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e);
void write_4b(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e);
void write_8b(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e);

uint64_t translate_read(em3_regs_t *r, uint64_t va, em3_access_error_t *e);
uint64_t translate_exec(em3_regs_t *r, uint64_t va, em3_access_error_t *e);
uint64_t translate_write(em3_regs_t *r, uint64_t va, em3_access_error_t *e);

void em_exit(em3_regs_t *r);
int intr_save(em3_regs_t *r);
int intr_restore(em3_regs_t *r);
void error(em3_regs_t *r, uint64_t e);

uint64_t shl(uint64_t x, uint64_t shamt);
uint64_t shr(uint64_t x, uint64_t shamt);
uint64_t sar(uint64_t x, uint64_t shamt);

uint64_t vread_c(em3_regs_t *r, uint64_t a, em3_access_error_t *e);
uint64_t vread_w(em3_regs_t *r, uint64_t a, em3_access_error_t *e);
uint64_t vread_w_exec(em3_regs_t *r, uint64_t a, em3_access_error_t *e);
uint64_t vread_h(em3_regs_t *r, uint64_t a, em3_access_error_t *e);
uint64_t vread_l(em3_regs_t *r, uint64_t a, em3_access_error_t *e);

void vwrite_c(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e);
void vwrite_w(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e);
void vwrite_h(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e);
void vwrite_l(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e);
#endif
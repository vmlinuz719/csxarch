#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <byteswap.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>

#include "pagemap.h"
#include "mmio.h"
#include "error.h"
#include "console.h"
#include "disk.h"
#include "bcd.h"
#include "csx.h"
#include "csximpl.h"

/* ABANDON HOPE ALL YE WHO ENTER 
   I NEED TO CLEAN THIS CODE UP 
   YOU HAVE BEEN WARNED          */

int intr(em3_regs_t *r, int which, int vector) {
    pthread_mutex_lock(&(r->intr_lock));
    if ((!(r->intr[which])) && vector < NUM_VEC) {
        r->intr[which] = vector;
        r->any_pending = 1;
        pthread_cond_signal(&(r->wake));
        pthread_mutex_unlock(&(r->intr_lock));
        return 1;
    }
    pthread_mutex_unlock(&(r->intr_lock));
    return 0;
}

void default_iack(em3_regs_t *r, int which) {
    // YOU MUST HOLD intr_lock
    // printf("INT ACK %d\n", which);
    r->intr[which] = 0;
    int flush_pending = 1;
    for (int i = 0; i < NUM_INT; i++) {
        if (r->intr[i]) {
            flush_pending = 0;
            break;
        }
    }
    if (flush_pending) r->any_pending = 0;
}

uint64_t get_reg(em3_regs_t *r, size_t n) {
    // obey hardwire-zero registers
    if (n == 0 || n >= 32) return 0;
    return r->regs[n];
}

void set_reg(em3_regs_t *r, size_t n, uint64_t v) {
    if (n != 0 && n < 32) r->regs[n] = v;
    // if (n == 2 || n == 9) printf("set reg %d %ld\n", n, v);
}

uint64_t read_u1b(em3_regs_t *r, uint64_t a, em3_access_error_t *e) {
    uint8_t *m = r->memory;
    
    
    
    // no alignment required for bytes
    if (a < MMIO_BASE) {
        if (a >= r->mem_limit - 1) {
            *e = BUS_ERROR;
            r->fault_sense = 1;
            r->fault_addr = a;
            return 0;
        }
        uint64_t result = (uint64_t) m[a];
        return result;
    } else {
        // TODO: MMIO
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].read == NULL
        ) {
            *e = BUS_ERROR;
            r->fault_sense = 1;
            r->fault_addr = a;
            return 0;
        }
        
        uint64_t result = r->mmio[unit].read(
            r->mmio[unit].ctx,
            1,
            MMIO_REGISTER(a),
            e
        );
        if (*e == BUS_ERROR || *e == ERR_ALIGN) {
            r->fault_sense = 1;
            r->fault_addr = a;
        }
        if (*e) return 0;
        else return result;
    }
}

uint64_t read_u2b(em3_regs_t *r, uint64_t a, em3_access_error_t *e) {
    uint8_t *m = r->memory;
    
    
    
    // require 2-byte alignment
    if (a % 2) {
        if (e != NULL) *e = ERR_ALIGN;
        r->fault_sense = 1;
        r->fault_addr = a;
        return 0;
    }

    if (a < MMIO_BASE) {
        if (a >= r->mem_limit - 2) {
            *e = BUS_ERROR;
            r->fault_sense = 1;
            r->fault_addr = a;
            return 0;
        }
        uint64_t result = (uint64_t) bswap_16(*((uint16_t *) (m + a)));
        return result;
    } else {
        // TODO: MMIO
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].read == NULL
        ) {
            *e = BUS_ERROR;
            r->fault_sense = 1;
            r->fault_addr = a;
            return 0;
        }
        
        uint64_t result = r->mmio[unit].read(
            r->mmio[unit].ctx,
            2,
            MMIO_REGISTER(a),
            e
        );
        if (*e == BUS_ERROR || *e == ERR_ALIGN) {
            r->fault_sense = 1;
            r->fault_addr = a;
        }
        if (*e) return 0;
        else return result;
    }
}

uint64_t read_u4b(em3_regs_t *r, uint64_t a, em3_access_error_t *e) {
    uint8_t *m = r->memory;
    
    
    
    
    // require 4-byte alignment
    if (a % 4) {
        if (e != NULL) *e = ERR_ALIGN;
        r->fault_sense = 1;
        r->fault_addr = a;
        return 0;
    }

    if (a < MMIO_BASE) {
        if (a >= r->mem_limit - 4) {
            *e = BUS_ERROR;
            r->fault_sense = 1;
            r->fault_addr = a;
            return 0;
        }
        uint64_t result = (uint64_t) bswap_32(*((uint32_t *) (m + a)));
        return result;
    } else {
        // TODO: MMIO
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].read == NULL
        ) {
            *e = BUS_ERROR;
            r->fault_sense = 1;
            r->fault_addr = a;
            return 0;
        }
        
        uint64_t result = r->mmio[unit].read(
            r->mmio[unit].ctx,
            4,
            MMIO_REGISTER(a),
            e
        );
        if (*e == BUS_ERROR || *e == ERR_ALIGN) {
            r->fault_sense = 1;
            r->fault_addr = a;
        }
        if (*e) return 0;
        else return result;
    }
}

uint64_t read_8b(em3_regs_t *r, uint64_t a, em3_access_error_t *e) {
    uint8_t *m = r->memory;
    
    
    
    // require 8-byte alignment
    if (a % 8) {
        if (e != NULL) *e = ERR_ALIGN;
        r->fault_sense = 1;
        r->fault_addr = a;
        return 0;
    }

    if (a < MMIO_BASE) {
        if (a >= r->mem_limit - 8) {
            *e = BUS_ERROR;
            r->fault_sense = 1;
            r->fault_addr = a;
            return 0;
        }
        uint64_t result = bswap_64(*((uint64_t *) (m + a)));
        return result;
    } else {
        // TODO: MMIO
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].read == NULL
        ) {
            *e = BUS_ERROR;
            r->fault_sense = 1;
            r->fault_addr = a;
            return 0;
        }
        
        uint64_t result = r->mmio[unit].read(
            r->mmio[unit].ctx,
            8,
            MMIO_REGISTER(a),
            e
        );
        if (*e == BUS_ERROR || *e == ERR_ALIGN) {
            r->fault_sense = 1;
            r->fault_addr = a;
        }
        if (*e) return 0;
        else return result;
    }
}

void write_1b(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e) {
    uint8_t *m = r->memory;
    // no alignment required for bytes
    
    if (a < MMIO_BASE) {
        if (a >= r->mem_limit - 1) {
            *e = BUS_ERROR;
            r->fault_sense = 2;
            r->fault_addr = a;
            return;
        }
        
        m[a] = (uint8_t) v;
    } else {
        // TODO: MMIO
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].write == NULL
        ) {
            *e = BUS_ERROR;
            r->fault_sense = 2;
            r->fault_addr = a;
            return;
        }
        
        r->mmio[unit].write(
            r->mmio[unit].ctx,
            1,
            MMIO_REGISTER(a),
            v,
            e
        );
        if (*e == BUS_ERROR || *e == ERR_ALIGN) {
            r->fault_sense = 2;
            r->fault_addr = a;
        }
    }
}

void write_2b(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e) {
    uint8_t *m = r->memory;
    
    
    
    // require 2-byte alignment
    if (a % 2) {
        if (e != NULL) *e = ERR_ALIGN;
        r->fault_sense = 2;
        r->fault_addr = a;
        return;
    }

    if (a < MMIO_BASE) {
        if (a >= r->mem_limit - 2) {
            *e = BUS_ERROR;
            r->fault_sense = 2;
            r->fault_addr = a;
            return;
        }
        *((uint16_t *) (m + a)) = bswap_16((uint16_t) v);
    } else {
        // TODO: MMIO
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].write == NULL
        ) {
            *e = BUS_ERROR;
            r->fault_sense = 2;
            r->fault_addr = a;
            return;
        }
        
        r->mmio[unit].write(
            r->mmio[unit].ctx,
            2,
            MMIO_REGISTER(a),
            v,
            e
        );
        if (*e == BUS_ERROR || *e == ERR_ALIGN) {
            r->fault_sense = 2;
            r->fault_addr = a;
        }
    }
}

void write_4b(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e) {
    uint8_t *m = r->memory;
    
    
    
    // require 4-byte alignment
    if (a % 4) {
        if (e != NULL) *e = ERR_ALIGN;
        r->fault_sense = 2;
        r->fault_addr = a;
        return;
    }

    if (a < MMIO_BASE) {
        if (a >= r->mem_limit - 4) {
            *e = BUS_ERROR;
            r->fault_sense = 2;
            r->fault_addr = a;
            return;
        }
        *((uint32_t *) (m + a)) = bswap_32((uint32_t) v);
    } else {
        // TODO: MMIO
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].write == NULL
        ) {
            *e = BUS_ERROR;
            r->fault_sense = 2;
            r->fault_addr = a;
            return;
        }
        
        r->mmio[unit].write(
            r->mmio[unit].ctx,
            4,
            MMIO_REGISTER(a),
            v,
            e
        );
        if (*e == BUS_ERROR || *e == ERR_ALIGN) {
            r->fault_sense = 2;
            r->fault_addr = a;
        }
    }
    
}

void write_8b(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e) {
    uint8_t *m = r->memory;
    
    
    
    // require 8-byte alignment
    if (a % 8) {
        if (e != NULL) *e = ERR_ALIGN;
        r->fault_sense = 2;
        r->fault_addr = a;
        return;
    }
    
    if (a < MMIO_BASE) {
        if (a >= r->mem_limit - 8) {
            *e = BUS_ERROR;
            r->fault_sense = 2;
            r->fault_addr = a;
            return;
        }
        *((uint64_t *) (m + a)) = bswap_64(v);
    } else {
        // TODO: MMIO
        int unit = MMIO_UNIT(a);
        if (
            unit >= r->num_units
            || r->mmio[unit].ctx == NULL
            || r->mmio[unit].write == NULL
        ) {
            *e = BUS_ERROR;
            r->fault_sense = 2;
            r->fault_addr = a;
            return;
        }
        
        r->mmio[unit].write(
            r->mmio[unit].ctx,
            8,
            MMIO_REGISTER(a),
            v,
            e
        );
        if (*e == BUS_ERROR || *e == ERR_ALIGN) {
            r->fault_sense = 2;
            r->fault_addr = a;
        }
    }
}



uint64_t translate_read(em3_regs_t *r, uint64_t va, em3_access_error_t *e) {
    uint64_t psw = get_reg(r, CR_PSW);

    if ((psw & NEW_CR_PSW_ASID) == 0) {
        return va; // real mode
    }

    else {
        int map_index = lookup(&r->map, va >> 12, (psw & NEW_CR_PSW_ASID) >> 48);

        if (map_index < 0) {
            *e = PAGE_FAULT;
            r->fault_sense = PM_VALID;
            r->fault_addr = va;
            return 0;
        }
        
        uint16_t flags = r->map.map[map_index].flags;
        
        if (!(flags & PM_PRESENT)) { // page not present
            *e = PAGE_FAULT;
            r->fault_sense = PM_PRESENT;
            r->fault_addr = va;
            return 0;
        }
        
        if ((psw & NEW_CR_PSW_PL) && !(flags & PM_READ)) { // page not readable
            *e = PAGE_FAULT;
            r->fault_sense = PM_READ;
            r->fault_addr = va;
            return 0;
        }

        uint64_t pa = (r->map.map[map_index].pa << 12) | (va & 0xFFF);
        return pa;
    }
}

uint64_t translate_exec(em3_regs_t *r, uint64_t va, em3_access_error_t *e) {
    uint64_t psw = get_reg(r, CR_PSW);

    if ((psw & NEW_CR_PSW_ASID) == 0) {
        return va; // real mode
    }

    else {
        int map_index = lookup(&r->map, va >> 12, (psw & NEW_CR_PSW_ASID) >> 48);

        if (map_index < 0) {
            *e = PAGE_FAULT;
            r->fault_sense = PM_VALID;
            r->fault_addr = va;
            return 0;
        }
        
        uint16_t flags = r->map.map[map_index].flags;
        
        if (!(flags & PM_PRESENT)) { // page not present
			INST_LEN(r, 0);
            *e = PAGE_FAULT;
            r->fault_sense = PM_PRESENT;
            r->fault_addr = va;
            return 0;
        }
        
        if (!(psw & NEW_CR_PSW_PL) && !(flags & PM_PRIVEXEC)) { // no privexec
			INST_LEN(r, 0);
            *e = PAGE_FAULT;
            r->fault_sense = PM_PRIVEXEC;
            r->fault_addr = va;
            return 0;
        }
        
        if ((psw & NEW_CR_PSW_PL) && !(flags & PM_READ)) { // page not readable
			INST_LEN(r, 0);
            *e = PAGE_FAULT;
            r->fault_sense = PM_READ;
            r->fault_addr = va;
            return 0;
        }
        
        if ((psw & NEW_CR_PSW_PL) && !(flags & PM_EXEC)) { // page not executable
			INST_LEN(r, 0);
            *e = PAGE_FAULT;
            r->fault_sense = PM_EXEC;
            r->fault_addr = va;
            return 0;
        }

        uint64_t pa = (r->map.map[map_index].pa << 12) | (va & 0xFFF);
        return pa;
    }
}

uint64_t translate_write(em3_regs_t *r, uint64_t va, em3_access_error_t *e) {
    uint64_t psw = get_reg(r, CR_PSW);

    if ((psw & NEW_CR_PSW_ASID) == 0) {
        return va; // real mode
    }

    else {
        int map_index = lookup(&r->map, va >> 12, (psw & NEW_CR_PSW_ASID) >> 48);

        if (map_index < 0) {
            *e = PAGE_FAULT;
            r->fault_sense = PM_VALID;
            r->fault_addr = va;
            return 0;
        }
        
        uint16_t flags = r->map.map[map_index].flags;
        
        if (!(flags & PM_PRESENT)) { // page not present
            *e = PAGE_FAULT;
            r->fault_sense = PM_PRESENT;
            r->fault_addr = va;
            return 0;
        }
        
        if (!(psw & NEW_CR_PSW_PL) && !(flags & PM_PRIVWRITE)) { // no privwrite
            *e = PAGE_FAULT;
            r->fault_sense = PM_PRIVWRITE;
            r->fault_addr = va;
            return 0;
        }
        
        if ((psw & NEW_CR_PSW_PL) && !(flags & PM_READ)) { // page not readable
            *e = PAGE_FAULT;
            r->fault_sense = PM_READ;
            r->fault_addr = va;
            return 0;
        }
        
        if ((psw & NEW_CR_PSW_PL) && !(flags & PM_WRITE)) { // page not writable
            *e = PAGE_FAULT;
            r->fault_sense = PM_WRITE;
            r->fault_addr = va;
            return 0;
        }

		if (!(psw & NEW_CR_PSW_PL) && !(flags & PM_DIRTY)) { // mark page as dirty
            r->map.map[map_index].flags = flags | PM_DIRTY;
        }

        uint64_t pa = (r->map.map[map_index].pa << 12) | (va & 0xFFF);
        return pa;
    }
}

void em_exit(em3_regs_t *r) {
    free(r->memory);
    for (int i = 0; i < r->num_units; i++) {
        if (r->mmio[i].ctx != NULL && r->mmio[i].destroy != NULL)
            r->mmio[i].destroy(r->mmio[i].ctx);
    }
    pthread_mutex_destroy(&(r->intr_lock));
    pthread_cond_destroy(&(r->wake));
    printf("\n================================================================================\n");
    printf("Emulator exited after %ld instructions\n", r->count);
    exit(EXIT_FAILURE);
}

int intr_save(em3_regs_t *r) {
    uint64_t psw = get_reg(r, CR_PSW);
    set_reg(r, CR_PSW, psw & ~(NEW_CR_PSW_PL));
    uint64_t return_addr = r->pc;

    set_reg(r, NEW_CR_SRPSW, psw);
    set_reg(r, NEW_CR_SR, return_addr);

    if (r->fault_sense) {
        set_reg(r, NEW_CR_EADDR, r->fault_addr);
        set_reg(r, NEW_CR_ETYPE, r->fault_sense);
        // printf("EADDR %016lX\n", get_reg(r, NEW_CR_EADDR));
        // printf("ETYPE %04lX\n", get_reg(r, NEW_CR_ETYPE));
        // printf("PC    %016lX\n", r->pc);
        r->fault_addr = 0;
        r->fault_sense = 0;
    }

    return 0;
}

int intr_restore(em3_regs_t *r) {
    r->pc = get_reg(r, NEW_CR_SR);
    set_reg(r, CR_PSW, get_reg(r, NEW_CR_SRPSW));

    return 0;
}

void error(em3_regs_t *r, uint64_t e) {
    // printf("DEBUG: error %ld\n", e);
    intr(r, SOFT, e + 1);
}

uint64_t shl(uint64_t x, uint64_t shamt) {
    if (shamt == 64) return 0;
    else if (shamt < 64) return x << shamt;
    else return x;
}

uint64_t shr(uint64_t x, uint64_t shamt) {
    if (shamt == 64) return 0;
    else if (shamt < 64) return x >> shamt;
    else return x;
}

uint64_t sar(uint64_t x, uint64_t shamt) {
    if (shamt == 64) return 0;
    else if (shamt < 64) return (uint64_t) (((int64_t) x) >> shamt);
    else return x;
}

// TODO: use translate_write for write functions
uint64_t vread_c(em3_regs_t *r, uint64_t a, em3_access_error_t *e) {
    a = translate_read(r, a, e);
    if (*e) return 0;
    else return read_u1b(r, a, e);
}

uint64_t vread_w(em3_regs_t *r, uint64_t a, em3_access_error_t *e) {
    a = translate_read(r, a, e);
    if (*e) return 0;
    else return read_u2b(r, a, e);
}

uint64_t vread_w_exec(em3_regs_t *r, uint64_t a, em3_access_error_t *e) {
    a = translate_exec(r, a, e);
    if (*e) return 0;
    else return read_u2b(r, a, e);
}

uint64_t vread_h(em3_regs_t *r, uint64_t a, em3_access_error_t *e) {
    a = translate_read(r, a, e);
    if (*e) return 0;
    else return read_u4b(r, a, e);
}

uint64_t vread_l(em3_regs_t *r, uint64_t a, em3_access_error_t *e) {
    a = translate_read(r, a, e);
    if (*e) return 0;
    else return read_8b(r, a, e);
}

void vwrite_c(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e) {
    a = translate_write(r, a, e);
    if (*e) return;
    else write_1b(r, a, v, e);
}

void vwrite_w(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e) {
    a = translate_write(r, a, e);
    if (*e) return;
    else write_2b(r, a, v, e);
}

void vwrite_h(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e) {
    a = translate_write(r, a, e);
    if (*e) return;
    else write_4b(r, a, v, e);
}

void vwrite_l(em3_regs_t *r, uint64_t a, uint64_t v, em3_access_error_t *e) {
    a = translate_write(r, a, e);
    if (*e) return;
    else write_8b(r, a, v, e);
}

static const uint8_t jeff[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 0x
    0, 3, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1x
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, // 2x
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 1, 1, 1, 1, // 3x - revisit worst case?
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 4x
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, // 5x
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, // 6x
    0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 7x
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 8x
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9x
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Ax
    1, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Bx
    1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Cx
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // Dx
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // Ex
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // Fx
};

uint64_t vfetch(em3_regs_t *r, em3_access_error_t *e) {
    // require 2-byte alignment
    if (r->pc % 2) {
        if (e != NULL) *e = ERR_ALIGN;
        r->fault_sense = 3;
        r->fault_addr = r->pc;
        return 0;
    }

    em3_access_error_t our_e = OK;    

    r->instruction_buffer = 0;
    // printf("PC %lX\n", r->pc - 0x46);

    uint64_t syllables[4] = {0, 0, 0, 0};

    syllables[0] = vread_w_exec(r, r->pc, &our_e);

    if (our_e) {
        if (e != NULL) *e = our_e;
        if (our_e != PAGE_FAULT && r->fault_sense) r->fault_sense = 3;
        return 0;
    }

    uint64_t opcode = syllables[0] >> 8;

    for (int i = 1; i <= jeff[opcode]; i++) {
        
        syllables[i] = vread_w_exec(r, r->pc + 2 * i, &our_e);
        if (our_e) {
            if (e != NULL) *e = our_e;
            if (r->fault_sense) r->fault_sense = 3;
            return 0;
        }
    }

    r->instruction_buffer = (
        syllables[0] << 48
        | syllables[1] << 32
        | syllables[2] << 16
        | syllables[3]
    );

    return r->instruction_buffer;
}

// TODO: move Control+C NMI logic to console.c
static volatile int nmi = 0;
em3_regs_t *nmi_cpu;
uint64_t last_instr = 0;
struct timeval timer;

void nmi_handler(int dummy) {
    (void) dummy;
    // nmi = 1;
    struct timeval new_time;
    gettimeofday(&new_time, 0);

    long seconds = new_time.tv_sec - timer.tv_sec;
    long microseconds = new_time.tv_usec - timer.tv_usec;

    double elapsed = seconds + microseconds * 1e-6;

    uint64_t instrs = nmi_cpu->count - last_instr;
    double mips = (instrs / elapsed) / (double) 1000000;

    for (int i = 0; i < 24; i += 4) {
        for (int j = 0; j < 4; j++) {
            uint64_t rv = get_reg(nmi_cpu, i + j);
            printf("%016lX ", j == 0 && i == 0 ? nmi_cpu->pc : rv);
        }
        printf("\n");
    }

    printf("\n*** %ld / %.4f = %.4f MIPS ***\n", instrs, elapsed, mips);

    timer = new_time;
    last_instr = nmi_cpu->count;
}

void cpu_run(em3_regs_t *r) {
    nmi_cpu = r;
    signal(SIGINT, nmi_handler);
    
    em3_access_error_t fetch_error = OK;

    gettimeofday(&timer, 0);

    while (1) {
        if (nmi) {
            intr(r, NMI, 1);
            nmi = 0;
        }
        
        if (!(GET_NEW_PSW_WST(r))) {
            fetch_error = OK;
            uint64_t inst = vfetch(r, &fetch_error);
            
            if (fetch_error) error(r, fetch_error);

            // else {
            if (r->increment) set_reg(r, CR_PSW, get_reg(r, CR_PSW) & 0x7FFFFF000000F000);
            r->increment = 0;

            if (r->any_pending) {
                pthread_mutex_lock(&(r->intr_lock));

                // printf("PENDING\n");

                int pending = -1;
                int vector = 0;

                for (int i = NUM_INT - 1; i >= (int) GET_NEW_PSW_PRI(r); i--) {
                    // printf("SCAN %d\n", i);
                    if (r->intr[i]) {
                        // printf("CAUGHT\n");
                        pending = i;
                        vector = r->intr[i];
                        break;
                    }
                }

                if (pending > -1) {
                    r->intr_ack[pending](r, pending);
                    if (!intr_save(r)) {
                        // printf("VECTOR %d @ %16lX\n", vector, r->pc);
                        r->pc = read_8b(r, 8 * vector, NULL);
                        SET_NEW_PSW_PRI(r, 0);
                        set_reg(r, CR_PSW, get_reg(r, CR_PSW) & ~(NEW_CR_PSW_PL));
                        
                        fetch_error = OK;
                        inst = vfetch(r, &fetch_error);
                        if (fetch_error) {
                            // printf("whar??\n");
                            error(r, fetch_error);
                        }
                    }
                } else {
                    // printf("SPURIOUS!\n");
                    r->any_pending = 0;
                }

                pthread_mutex_unlock(&(r->intr_lock));
            }

            if ((!(GET_NEW_PSW_WST(r))) && (!(fetch_error))) {
                inst_ex(r, inst);
                r->pc += r->increment;
                r->count++;
            }
            // }
        } else if (GET_NEW_PSW_PRI(r) == 0xF) {
            uint64_t psw_hh = (get_reg(r, CR_PSW) >> 48) & 0xFFFF;
            uint64_t psw_hl = (get_reg(r, CR_PSW) >> 32) & 0xFFFF;
            uint64_t psw_lh = (get_reg(r, CR_PSW) >> 16) & 0xFFFF;
            uint64_t psw_ll = (get_reg(r, CR_PSW) >> 0)  & 0xFFFF;
            
            sleep(1);
            
            printf("\n X %04lX %04lX %04lX %04lX\n",
                psw_hh, psw_hl, psw_lh, psw_ll);
            
            break;
        }
        else {
            pthread_mutex_lock(&(r->intr_lock));
            pthread_cond_wait(
                &(r->wake),
                &(r->intr_lock)
            );
            pthread_mutex_unlock(&(r->intr_lock));
            if (r->any_pending) {
                SET_NEW_PSW_WST(r, 0L);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    em3_regs_t r; r.pc = 0xFFFF000000010000;
    pthread_mutex_init(&(r.intr_lock), NULL);
    pthread_mutex_init(&(r.cas_lock), NULL);
    pthread_cond_init(&(r.wake), NULL);
    for (int i = 0; i < 32; i++) r.regs[i] = 0;
    for (int i = 0; i < NUM_INT; i++) {
        r.intr[i] = 0;
        r.intr_ack[i] = default_iack;
    }
    r.any_pending = 0;
    r.count = 0;
    r.fault_addr = 0;
    r.fault_sense = 0;

    init_page_map(&r.map);

    uint8_t *memory = malloc(1048576);
    r.memory = memory;
    r.mem_limit = 1048576;
    
    mmio_unit_t units[2];
    r.mmio = units;
    init_console(&units[0], &r);
    init_disk(&units[1], &r);
    disk_init_lun(&units[1], 0, "ipl.img", 512, 0);
    r.num_units = 2;

    cpu_run(&r);
    
    em_exit(&r);

    return 0;
}

#include <stdint.h>
#include "csx.h"
#include "csximpl.h"

void inst_BAS(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 0);
	
    uint64_t i1 = RM_I12(i);
    
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + ((EXT12(i1)) << 1);

    set_reg(r, RM_RD(i), r->pc + 4);
    r->pc = addr;
}

void inst_BASR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 0);
	
    uint64_t i1 = RMl_I16(i);
    
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + ((EXT16(i1)) << 1);

    set_reg(r, RM_RD(i), r->pc + 4);
    r->pc = addr + (r->pc);
}

void inst_BASQ(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 0);
	
    uint64_t addr = get_reg(r, RR_RS(i));

    // printf("\n==> %lX %lX\n", r->regs[1], r->regs[2]);

    set_reg(r, RR_RD(i), r->pc + 2);
    r->pc = addr;
}

void inst_BCASR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 0);

    pthread_mutex_lock(&(r->cas_lock));
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    uint64_t result = vread_l(r, addr, &e);

    if (e)
        error(r, e);
    else {
        if (result == get_reg(r, BCASR_RS(i))) {
            vwrite_l(r, addr, get_reg(r, RM_RD(i)), &e);
            if (e)
                error(r, e);
            else {
                uint64_t tgt = BCASR_I12(i);
                r->pc += (EXT12(tgt) << 1);
            }
        } else {
            r->increment = 6;
        }
    }

    pthread_mutex_unlock(&(r->cas_lock));
}

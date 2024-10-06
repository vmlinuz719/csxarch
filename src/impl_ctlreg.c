#include <stdint.h>
#include "csx.h"
#include "csximpl.h"

void inst_STCR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    int rd = RR_RD(i) + 16;
    if ((get_reg(r, CR_PSW) & NEW_CR_PSW_PL) && rd > MAX_UNPRIV_READ) {
        error(r, PRIVILEGE_VIOLATION);
        return;
    }

    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    vwrite_l(r, addr, get_reg(r, rd), &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
    }
}

void inst_LCR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    int rd = RR_RD(i) + 16;
    if ((get_reg(r, CR_PSW) & NEW_CR_PSW_PL) && rd > MAX_UNPRIV_WRITE) {
        error(r, PRIVILEGE_VIOLATION);
        return;
    }

    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    uint64_t result = vread_l(r, addr, &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
        set_reg(r, RM_RD(i), result);
    }
}

void inst_MFCR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    int rd = RR_RD(i) + 16;
    int rs = RR_RS(i);

    if ((get_reg(r, CR_PSW) & NEW_CR_PSW_PL) && rd > MAX_UNPRIV_READ) {
        // printf("DEBUG: MFCR FAIL\n");
        error(r, PRIVILEGE_VIOLATION);
    }
    else {
        set_reg(r, rs, get_reg(r, rd));
        r->increment = 2;
    }
}

void inst_MTCR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    int rd = RR_RD(i) + 16;
    int rs = RR_RS(i);

    if ((get_reg(r, CR_PSW) & NEW_CR_PSW_PL) && rd > MAX_UNPRIV_WRITE) {
        // printf("DEBUG: MTCR FAIL\n");
        error(r, PRIVILEGE_VIOLATION);
    }
    else {
        // printf("Wrote %016lX to register %d\n", get_reg(r, rs), rd);
        set_reg(r, rd, get_reg(r, rs));
        r->increment = 2;
    }
}

void inst_REX(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
    
    uint8_t svcid = RR_I8(i);

    if (!svcid) {
        if ((get_reg(r, CR_PSW) & NEW_CR_PSW_PL)) {
            error(r, PRIVILEGE_VIOLATION);
        }
        else {
            // printf("Wrote %016lX to register %d\n", get_reg(r, rs), rd);
            intr_restore(r);
            // r->increment = 2;
        }
    } else {
        r->pc += 2;

        if (!intr_save(r)) {
            set_reg(r, CR_PSW, get_reg(r, CR_PSW) & ~(NEW_CR_PSW_PL));
            r->pc = read_8b(r, 0, NULL);
            set_reg(r, NEW_CR_ETYPE, svcid);
        }
    }
}

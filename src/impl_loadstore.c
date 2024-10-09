#include <stdint.h>
#include "csx.h"
#include "csximpl.h"

void inst_LC(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    uint64_t result = vread_c(r, addr, &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
        set_reg(r, RM_RD(i), result);
    }
}

void inst_LCS(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    uint64_t result = vread_c(r, addr, &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
        set_reg(r, RM_RD(i), EXT8(result));
    }
}

void inst_LW(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    uint64_t result = vread_w(r, addr, &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
        set_reg(r, RM_RD(i), result);
    }
}

void inst_LWS(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    uint64_t result = vread_w(r, addr, &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
        set_reg(r, RM_RD(i), EXT16(result));
    }
}

void inst_LH(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    uint64_t result = vread_h(r, addr, &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
        set_reg(r, RM_RD(i), result);
    }
}

void inst_LHS(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    uint64_t result = vread_h(r, addr, &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
        set_reg(r, RM_RD(i), EXT32(result));
    }
}

void inst_L(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
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
        set_reg(r, RR_RD(i), result);
    }
}

void inst_STC(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    // printf("STC %02hhX %lX\n", get_reg(r, RM_RD(i)), addr);
    vwrite_c(r, addr, get_reg(r, RM_RD(i)), &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
    }
}

void inst_STW(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    vwrite_w(r, addr, get_reg(r, RM_RD(i)), &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
    }
}

void inst_STH(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    vwrite_h(r, addr, get_reg(r, RM_RD(i)), &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
    }
}

void inst_ST(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    // printf("Store %lX @ %lX\n", get_reg(r, RM_RD(i)), addr);

    em3_access_error_t e = OK;

    vwrite_l(r, addr, get_reg(r, RM_RD(i)), &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
    }
}

void inst_LM(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t ctr = GET_PSW_CTR(r);
    
    uint64_t dst = RM_RD(i);
    uint64_t limit = RM_RX(i);
    
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + RM_I12(i);
    
    em3_access_error_t e = OK;
    
    uint64_t read_val = vread_l(r, addr + 8 * ctr, &e);
    if (e) {
        error(r, e);
        return;
    }
    
    set_reg(r, (dst + ctr) % 16, read_val);
    
    if ((dst + ctr) % 16 == limit) {
        // done
        r->increment = 4;
        return;
    } else {
        SET_PSW_CTR(r, ctr + 1);
        return;
    }
}

void inst_STM(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t ctr = GET_PSW_CTR(r);
    
    uint64_t dst = RM_RD(i);
    uint64_t limit = RM_RX(i);
    
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + RM_I12(i);
    
    em3_access_error_t e = OK;
    
    uint64_t write_val = get_reg(r, (dst + ctr) % 16);
    vwrite_l(r, addr + 8 * ctr, write_val, &e);
    if (e) {
        error(r, e);
        return;
    }
    
    if ((dst + ctr) % 16 == limit) {
        // done
        
        r->increment = 4;
        return;
    } else {
        SET_PSW_CTR(r, ctr + 1);
        return;
    }
}
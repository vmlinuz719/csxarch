#include <stdint.h>
#include "csx.h"
#include "csximpl.h"

void inst_NC(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) & result);
    }
}

void inst_OC(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) | result);
    }
}

void inst_XC(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) ^ result);
    }
}

void inst_NW(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) & result);
    }
}

void inst_OW(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) | result);
    }
}

void inst_XW(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) ^ result);
    }
}

void inst_AH(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) + result);
    }
}

void inst_AHS(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) + EXT32(result));
    }
}

void inst_SH(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) - result);
    }
}

void inst_SHS(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) - EXT32(result));
    }
}

void inst_NH(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) & result);
    }
}

void inst_OH(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) | result);
    }
}

void inst_XH(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) ^ result);
    }
}

void inst_A(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) + result);
    }
}

void inst_S(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) - result);
    }
}

void inst_N(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) & result);
    }
}

void inst_O(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) | result);
    }
}

void inst_X(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), get_reg(r, RM_RD(i)) ^ result);
    }
}

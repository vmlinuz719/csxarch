#include <stdint.h>
#include "csx.h"
#include "csximpl.h"
#include "bcd.h"

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

void inst_AD(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RB_I8(i);

    em3_access_error_t e = OK;

    uint64_t x = csx2valid(vread_l(r, addr, &e));
    uint64_t y = csx2valid(get_reg(r, RM_RD(i)));
    uint64_t c = get_reg(r, B_RD(i));

    if (e) {
        r->increment = 0;
        error(r, e);
    } else if (!(bcd_valid(x) && bcd_valid(y))) {
        r->increment = 0;
        error(r, DECIMAL_FORMAT);
        return;
    } else {
        r->increment = 4;

        x = bcd_add(x, y, &c);
    
        set_reg(r, RM_RD(i), tc2csx(x));
        set_reg(r, B_RD(i), c);
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

void inst_SD(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RB_I8(i);

    em3_access_error_t e = OK;

    uint64_t x = csx2valid(vread_l(r, addr, &e));
    uint64_t y = csx2valid(get_reg(r, RM_RD(i)));
    uint64_t c = get_reg(r, B_RD(i));

    if (e) {
        r->increment = 0;
        error(r, e);
    } else if (!(bcd_valid(x) && bcd_valid(y))) {
        r->increment = 0;
        error(r, DECIMAL_FORMAT);
        return;
    } else {
        r->increment = 4;

        x = bcd_sub(x, y, &c);
    
        set_reg(r, RM_RD(i), tc2csx(x));
        set_reg(r, B_RD(i), c);
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

#include <stdint.h>
#include "csx.h"
#include "csximpl.h"

void inst_LCIN(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) + 1);
    }
}

void inst_LWIN(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) + 2);
    }
}

void inst_LHIN(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) + 4);
    }
}

void inst_LIN(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) + 8);
    }
}

void inst_LSK(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    uint64_t addr =
        get_reg(r, RM_RB(i));

    em3_access_error_t e = OK;

    uint64_t result = vread_l(r, addr, &e);

    if (e)
        error(r, e);
    else {
        r->increment = 2;
        set_reg(r, RR_RD(i), result);
        set_reg(r, RM_RB(i), get_reg(r, RM_RB(i)) + 8);
    }
}

void inst_STCIN(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) + 1);
    }
}

void inst_STWIN(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) + 2);
    }
}

void inst_STHIN(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) + 4);
    }
}

void inst_STIN(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    vwrite_l(r, addr, get_reg(r, RM_RD(i)), &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
        set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) + 8);
    }
}

void inst_DESTC(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) - 1);

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

void inst_DESTW(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) - 2);

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

void inst_DESTH(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);

    set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) - 4);
	
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

void inst_DEST(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);

    set_reg(r, RM_RX(i), get_reg(r, RM_RX(i)) - 8);
	
    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    em3_access_error_t e = OK;

    vwrite_l(r, addr, get_reg(r, RM_RD(i)), &e);

    if (e)
        error(r, e);
    else {
        r->increment = 4;
    }
}

void inst_STSK(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);

    set_reg(r, RM_RB(i), get_reg(r, RM_RB(i)) - 8);
	
    uint64_t addr =
        get_reg(r, RM_RB(i));

    em3_access_error_t e = OK;

    vwrite_l(r, addr, get_reg(r, RM_RD(i)), &e);

    if (e)
        error(r, e);
    else {
        r->increment = 2;
    }
}

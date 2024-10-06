#include <stdint.h>
#include "csx.h"
#include "bcd.h"
#include "csximpl.h"

void inst_LDHS(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), bcd_ext7(result));
    }
}

void inst_ADH(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), bcd_add(get_reg(r, RM_RD(i)), result));
    }
}

void inst_ADHS(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), bcd_add(get_reg(r, RM_RD(i)), bcd_ext7(result)));
    }
}

void inst_SDH(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), bcd_sub(get_reg(r, RM_RD(i)), result));
    }
}

void inst_SDHS(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), bcd_sub(get_reg(r, RM_RD(i)), bcd_ext7(result)));
    }
}

void inst_AD(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), bcd_add(get_reg(r, RM_RD(i)), result));
    }
}

void inst_SD(em3_regs_t *r, uint64_t i) {
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
        set_reg(r, RM_RD(i), bcd_sub(get_reg(r, RM_RD(i)), result));
    }
}

void inst_CD(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), bcd_bin2dec(get_reg(r, RR_RS(i))));
}

void inst_CB(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), bcd_dec2bin(get_reg(r, RR_RS(i))));
}

void inst_CDS(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), bcd_ext7(get_reg(r, RR_RS(i))));
}

void inst_CLDHS(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), bcd_trunc7(get_reg(r, RR_RS(i))));
}

void inst_ADR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), bcd_add(get_reg(r, RR_RD(i)), get_reg(r, RR_RS(i))));
}

void inst_SDR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), bcd_sub(get_reg(r, RR_RD(i)), get_reg(r, RR_RS(i))));
}

void inst_NED(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), bcd_neg(get_reg(r, RR_RS(i))));
}

void inst_EDC(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;

    uint64_t a = get_reg(r, RR_RD(i));
    uint64_t b = get_reg(r, RR_RS(i));

    if ((b & 0xF000000000000000) == 0x1000000000000000) {
        b &= 0x0FFFFFFFFFFFFFFF;
        a = bcd_add(a, 1);
    }

    else if ((b & 0xF000000000000000) == 0xF000000000000000) {
        b = bcd_add(0x1000000000000000, b);
        a = bcd_sub(a, 1);
    }

    set_reg(r, RR_RD(i), a);
    set_reg(r, RR_RS(i), b);
}

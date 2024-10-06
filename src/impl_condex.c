#include <stdint.h>
#include "csx.h"
#include "csximpl.h"

int cond(uint64_t cc, uint64_t a, uint64_t b) {
    switch (cc) {
        case 0:
            return 1;
        case 1:
            return 0;
        case 2:
            return ((int64_t) a) < ((int64_t) b);
        case 3:
            // printf("test %llX >= %llX\n", a, b);
            return ((int64_t) a) >= ((int64_t) b);
        case 4:
            return ((int64_t) a) <= ((int64_t) b);
        case 5:
            return ((int64_t) a) > ((int64_t) b);
        case 6:
            return a < b;
        case 7:
            return a >= b;
        case 8:
            return a <= b;
        case 9:
            return a > b;
        case 10:
            // if (a == b) printf("\n%lX == %lX\n", a, b);
            // else printf("\n%lX != %lX\n", a, b);
            return a == b;
        case 11:
            return a != b;
        case 12:
            return (b & (1 << a)) != 0;
        case 13:
            return (b & (1 << a)) == 0;
        case 14:
            return (b & (1 << a)) != 0;
        case 15:
            return (b & (1 << a)) == 0;
        default:
            // how???
            return 0;
    }
}

void inst_BRcc(em3_regs_t *r, uint64_t i) {
    INST_LEN(r, 0);

    uint64_t cc = BR_CC(i);
    
    uint64_t i1 = RMl_I16(i);

    uint64_t a = (cc == 0xE || cc == 0xF) ?
        BR_RS2(i) :
        get_reg(r, BR_RS2(i));
    uint64_t b = get_reg(r, BR_RS1(i));
    if (cond(cc, a, b)) {
        r->pc += (EXT16(i1)) << 1;
    } else {
        r->increment = 4;
    }
}

void inst_BICcc(em3_regs_t *r, uint64_t i) {
    INST_LEN(r, 0);

    uint64_t cc = BR_CC(i);
    
    uint64_t i1 = RMl_I16(i);

    uint64_t b = BIC_I20(i);
    b = EXT20(b);
    uint64_t a = get_reg(r, BR_RS2(i));
    // printf("%ld ~ %ld\n", a, b);
    if (cond(cc, a, b)) {
        r->pc += (EXT16(i1)) << 1;
    } else {
        r->increment = 6;
    }
}

void inst_MVRcc_ARcc(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	r->increment = 4;

    uint64_t cc = B_CC(i);
    uint64_t a = (cc == 0xE || cc == 0xF) ?
        B_RS2(i) :
        get_reg(r, B_RS2(i));
    uint64_t b = get_reg(r, B_RS1(i));

    if (cond(cc, a, b)) {
        uint64_t result = get_reg(r, B_RB(i)) + get_reg(r, B_RX(i));
        set_reg(r, B_RD(i), result);
    }
}

void inst_SELcc(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	r->increment = 4;

    uint64_t cc = B_CC(i);
    uint64_t a = (cc == 0xE || cc == 0xF) ?
        B_RS2(i) :
        get_reg(r, B_RS2(i));
    uint64_t b = get_reg(r, B_RS1(i));

    if (cond(cc, a, b)) {
        set_reg(r, B_RD(i), get_reg(r, B_RB(i)));
    } else {
        set_reg(r, B_RD(i), get_reg(r, B_RX(i)));
    }
}

void inst_ECRcc(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	r->increment = 4;

    uint64_t cc = B_CC(i);
    uint64_t a = (cc == 0xE || cc == 0xF) ?
        B_RS2(i) :
        get_reg(r, B_RS2(i));
    uint64_t b = get_reg(r, B_RS1(i));

    if (cond(cc, a, b)) {
        uint64_t temp = get_reg(r, B_RB(i));
        set_reg(r, B_RB(i), get_reg(r, B_RD(i)));
        set_reg(r, B_RD(i), temp);
    } else {
        uint64_t temp = get_reg(r, B_RX(i));
        set_reg(r, B_RX(i), get_reg(r, B_RD(i)));
        set_reg(r, B_RD(i), temp);
    }
}

void inst_SRcc(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	r->increment = 4;

    uint64_t cc = B_CC(i);
    uint64_t a = (cc == 0xE || cc == 0xF) ?
        B_RS2(i) :
        get_reg(r, B_RS2(i));
    uint64_t b = get_reg(r, B_RS1(i));

    if (cond(cc, a, b)) {
        uint64_t result = get_reg(r, B_RB(i)) - get_reg(r, B_RX(i));
        set_reg(r, B_RD(i), result);
    }
}

void inst_NRcc(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	r->increment = 4;

    uint64_t cc = B_CC(i);
    uint64_t a = (cc == 0xE || cc == 0xF) ?
        B_RS2(i) :
        get_reg(r, B_RS2(i));
    uint64_t b = get_reg(r, B_RS1(i));

    if (cond(cc, a, b)) {
        uint64_t result = get_reg(r, B_RB(i)) & get_reg(r, B_RX(i));
        set_reg(r, B_RD(i), result);
    }
}

void inst_ORcc(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	r->increment = 4;

    uint64_t cc = B_CC(i);
    uint64_t a = (cc == 0xE || cc == 0xF) ?
        B_RS2(i) :
        get_reg(r, B_RS2(i));
    uint64_t b = get_reg(r, B_RS1(i));

    if (cond(cc, a, b)) {
        uint64_t result = get_reg(r, B_RB(i)) | get_reg(r, B_RX(i));
        set_reg(r, B_RD(i), result);
    }
}

void inst_XRcc(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	r->increment = 4;

    uint64_t cc = B_CC(i);
    uint64_t a = (cc == 0xE || cc == 0xF) ?
        B_RS2(i) :
        get_reg(r, B_RS2(i));
    uint64_t b = get_reg(r, B_RS1(i));

    if (cond(cc, a, b)) {
        uint64_t result = get_reg(r, B_RB(i)) ^ get_reg(r, B_RX(i));
        set_reg(r, B_RD(i), result);
    }
}


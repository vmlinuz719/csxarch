#include <stdint.h>
#include "csx.h"
#include "csximpl.h"

void inst_AR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), get_reg(r, RR_RD(i)) + get_reg(r, RR_RS(i)));
}

void inst_ARCK(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    r->increment = 4;
    
    uint64_t x = get_reg(r, RM_RD(i));
    uint64_t y = get_reg(r, RM_RB(i)) + RM_I12(i);

    uint64_t result = 0;
    if (x > 0xFFFFFFFFFFFFFFFF - y) result |= 1; // carry
    if (
        ((x & (1L << 63)) && (y & (1L << 63)) && !((x + y) & (1L << 63)))
        | (!(x & (1L << 63)) && !(y & (1L << 63)) && ((x + y) & (1L << 63)))
    ) result |= 2; // overflow
    
    set_reg(r, RM_RD(i), x + y);
    set_reg(r, RM_RX(i), result);
}

void inst_ECR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    
    uint64_t temp = get_reg(r, RR_RS(i));
    set_reg(r, RR_RS(i), get_reg(r, RR_RD(i)));
    set_reg(r, RR_RD(i), temp);
}

void inst_SR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), get_reg(r, RR_RD(i)) - get_reg(r, RR_RS(i)));
}

void inst_SRCK(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    r->increment = 4;
    
    uint64_t x = get_reg(r, RM_RD(i));
    uint64_t y = get_reg(r, RM_RB(i)) + RM_I12(i);

    uint64_t result = 0;
    if (x < y) result |= 1; // carry
    if (
        ((int64_t) x < (int64_t) y && !((x - y) & (1L << 63)))
        | ((int64_t) x >= (int64_t) y && ((x + y) & (1L << 63)))
    ) result |= 2; // overflow
    
    set_reg(r, RM_RD(i), x - y);
    set_reg(r, RM_RX(i), result);
}

void inst_NR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), get_reg(r, RR_RD(i)) & get_reg(r, RR_RS(i)));
}

void inst_OR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), get_reg(r, RR_RD(i)) | get_reg(r, RR_RS(i)));
}

void inst_XR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;
    set_reg(r, RR_RD(i), get_reg(r, RR_RD(i)) ^ get_reg(r, RR_RS(i)));
}

void inst_MASR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    r->increment = 4;
    
    uint64_t x = get_reg(r, RM_RD(i));
    uint64_t y = get_reg(r, RM_RB(i));
    uint64_t z = shl(get_reg(r, RM_RX(i)), RM_I12(i));
    
    set_reg(r, RM_RD(i), x * y + z);
}

void inst_DMR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
    
    uint64_t x = get_reg(r, RM_RD(i));
    uint64_t y = get_reg(r, RM_RB(i)) + RM_I12(i);
    
    if (y == 0) {
        r->increment = 0;
        error(r, DIVIDE_BY_ZERO);
        return;
    }
    
    r->increment = 4;
    set_reg(r, RM_RD(i), x / y);
    set_reg(r, RM_RX(i), x % y);
}

void inst_SLR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;

    uint64_t shamt = get_reg(r, RR_RS(i));

    set_reg(r, RR_RD(i), shl(get_reg(r, RR_RD(i)), shamt));
}

void inst_SRR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;

    uint64_t shamt = get_reg(r, RR_RS(i));

    set_reg(r, RR_RD(i), shr(get_reg(r, RR_RD(i)), shamt));
}

void inst_SRAR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    r->increment = 2;

    uint64_t shamt = get_reg(r, RR_RS(i));

    set_reg(r, RR_RD(i), sar(get_reg(r, RR_RD(i)), shamt));
}

void inst_imm_wide_block(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    r->increment = 4;

    uint64_t i16 = RIw_I16(i);
    int rd = RIw_RD(i);

    switch (MINOR_OPCODE_0(i)) {

        case 1: set_reg(r, rd, get_reg(r, rd) - i16); break;
        case 2: set_reg(r, rd, get_reg(r, rd) & i16); break;
        case 3: set_reg(r, rd, get_reg(r, rd) | i16); break;
        case 4: set_reg(r, rd, get_reg(r, rd) | (i16 << 48)); break;
        case 5: set_reg(r, rd, get_reg(r, rd) ^ i16); break;

        default:
            r->increment = 0;
            error(r, ILLEGAL_INSTRUCTION);
    }
}

void inst_imm_half_block(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 6);
	
    r->increment = 6;

    uint64_t i32 = RIh_I32(i);
    int rd = RIh_RD(i);

    switch (MINOR_OPCODE_0(i)) {
        case 0: set_reg(r, rd, get_reg(r, rd) + i32); break;
        case 1: set_reg(r, rd, get_reg(r, rd) - i32); break;
        case 2: set_reg(r, rd, get_reg(r, rd) & i32); break;
        case 3: set_reg(r, rd, get_reg(r, rd) | i32); break;
        case 4: set_reg(r, rd, get_reg(r, rd) | (i32 << 32)); break;
        case 5: set_reg(r, rd, get_reg(r, rd) ^ i32); break;

        default:
            r->increment = 0;
            error(r, ILLEGAL_INSTRUCTION);
    }
}

void inst_imm_long_block(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 8);
	
    r->increment = 8;

    uint64_t i48 = RIl_I48(i);
    int rd = RIl_RD(i);

    switch (MINOR_OPCODE_0(i)) {
        case 0: set_reg(r, rd, get_reg(r, rd) + i48); break;
        case 1: set_reg(r, rd, get_reg(r, rd) - i48); break;
        case 2: set_reg(r, rd, get_reg(r, rd) & i48); break;
        case 3: set_reg(r, rd, get_reg(r, rd) | i48); break;
        case 4: set_reg(r, rd, get_reg(r, rd) ^ i48); break;

        default:
            r->increment = 0;
            error(r, ILLEGAL_INSTRUCTION);
    }
}
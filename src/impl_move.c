#include <stdint.h>
#include "csx.h"
#include "csximpl.h"

void inst_MVR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
    r->increment = 2;

    int rd = RR_RD(i);
    int rs = RR_RS(i);

    set_reg(r, rd, get_reg(r, rs));
}

void inst_LWI(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    r->increment = 4;

    uint64_t i16 = RIw_I16(i);
    int rd = RIw_RD(i);

    set_reg(r, rd, i16);
}

void inst_LWIS(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    r->increment = 4;

    uint64_t i16 = RIw_I16(i);
    int rd = RIw_RD(i);

    set_reg(r, rd, EXT16(i16));
}

void inst_LHI(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 6);
	
    r->increment = 6;

    uint64_t i32 = RIh_I32(i);
    int rd = RIh_RD(i);

    set_reg(r, rd, i32);
}

void inst_LHIS(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 6);
	
    r->increment = 6;

    uint64_t i32 = RIh_I32(i);
    int rd = RIh_RD(i);

    set_reg(r, rd, EXT32(i32));
}

void inst_LLI(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 8);	
	
    r->increment = 8;

    uint64_t i48 = RIl_I48(i);
    int rd = RIl_RD(i);

    set_reg(r, rd, i48);
}

void inst_LLIS(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 8);
	
    r->increment = 8;

    uint64_t i48 = RIl_I48(i);
    int rd = RIl_RD(i);

    set_reg(r, rd, EXT48(i48));
}

void inst_LA(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    r->increment = 4;

    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + RM_I12(i);

    set_reg(r, RM_RD(i), addr);
}

void inst_LAL(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    r->increment = 4;

    uint64_t addr =
        get_reg(r, RM_RB(i))
        + RMl_I16(i);

    set_reg(r, RM_RD(i), addr);
}

void inst_LAPR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    r->increment = 4;

    uint64_t i1 = RM_I12(i);

    uint64_t addr =
        get_reg(r, RM_RB(i))
        + get_reg(r, RM_RX(i))
        + EXT12(i1)
        + (r->pc);

    set_reg(r, RM_RD(i), addr);
}

void inst_LAPRL(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    r->increment = 4;

    uint64_t i1 = RMl_I16(i);

    uint64_t addr =
        get_reg(r, RM_RB(i))
        + EXT16(i1)
        + (r->pc);

    set_reg(r, RM_RD(i), addr);
}

void inst_load_imm_block(em3_regs_t *r, uint64_t i) {
    switch(MINOR_OPCODE_0(i)) {
        case 0:     inst_LWI    (r, i); break;
        case 1:     inst_LWIS   (r, i); break;
        case 2:     inst_LHI    (r, i); break;
        case 3:     inst_LHIS   (r, i); break;
        case 4:     inst_LLI    (r, i); break;
        case 5:     inst_LLIS   (r, i); break;
        default:    error(r, ILLEGAL_INSTRUCTION);
    }
}
#include <stdint.h>
#include "byteswap.h"
#include "csx.h"
#include "csximpl.h"
#include "bcd.h"

void inst_LSI(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
    r->increment = 2;

    int rd = RR_RD(i);
    int rs = RR_RS(i);

    set_reg(r, rd, rs);
}

void inst_LSIS(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
    r->increment = 2;

    int rd = RR_RD(i);
    int rs = RR_RS(i);

    set_reg(r, rd, EXT4(rs));
}

void inst_bswpw(em3_regs_t *r, uint64_t i) {
    if (EXT05_RR_I8(i) != 0) {
        r->increment = 0;
        error(r, ILLEGAL_INSTRUCTION);
    } else {
        r->increment = 4;
        
        uint64_t original = get_reg(r, EXT05_RR_RS(i));
        uint64_t result = (original & 0xFFFFFFFFFFFF0000)
                        | (uint64_t) bswap_16(original & 0xFFFF);
        set_reg(r, EXT05_RR_RD(i), result);
    }
}

void inst_bswph(em3_regs_t *r, uint64_t i) {
    if (EXT05_RR_I8(i) != 0) {
        r->increment = 0;
        error(r, ILLEGAL_INSTRUCTION);
    } else {
        r->increment = 4;
        
        uint64_t original = get_reg(r, EXT05_RR_RS(i));
        uint64_t result = (original & 0xFFFFFFFF00000000)
                        | (uint64_t) bswap_32(original & 0xFFFFFFFF);
        set_reg(r, EXT05_RR_RD(i), result);
    }
}

void inst_bswp(em3_regs_t *r, uint64_t i) {
    if (EXT05_RR_I8(i) != 0) {
        r->increment = 0;
        error(r, ILLEGAL_INSTRUCTION);
    } else {
        r->increment = 4;
        
        uint64_t original = get_reg(r, EXT05_RR_RS(i));
        uint64_t result = bswap_64(original);
        set_reg(r, EXT05_RR_RD(i), result);
    }
}

void inst_conv_clr_block(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    uint64_t reg = get_reg(r, RR_RS(i));
    r->increment = 2;

    switch(MAJOR_OPCODE(i)) {
        case 0:
            set_reg(r, RR_RD(i), EXT8(reg));
            break;
        case 1:
            set_reg(r, RR_RD(i), EXT16(reg));
            break;
        case 2:
            set_reg(r, RR_RD(i), EXT32(reg));
            break;

        case 4:
            set_reg(r, RR_RD(i), reg & 0xFF);
            break;
        case 5:
            set_reg(r, RR_RD(i), reg & 0xFFFF);
            break;
        case 6:
            set_reg(r, RR_RD(i), reg & 0xFFFFFFFF);
            break;

        case 8:
            set_reg(r, RR_RD(i), ~reg);
            break;
        case 9:
            set_reg(r, RR_RD(i), -reg);
            break;

        case 0xA:
            // printf("%lX -> %lX\n", get_reg(r, RR_RD(i)), reg + 1);
            set_reg(r, RR_RD(i), reg + 1);
            break;
        case 0xB:
            set_reg(r, RR_RD(i), reg - 1);
            break;
        case 0xC:
            set_reg(r, RR_RD(i), shl(reg, 1));
            break;
        case 0xD:
            set_reg(r, RR_RD(i), shr(reg, 1));
            break;
        case 0xE:
            set_reg(r, RR_RD(i), sar(reg, 1));
            break;

        default:
            r->increment = 0;
            error(r, ILLEGAL_INSTRUCTION);
    }
}

void inst_ext05_block_0f(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
    

    switch(EXT05_MINOR(i)) {
        case 0:
            inst_bswpw(r, i);
            break;
        case 1:
            inst_bswph(r, i);
            break;
        case 2:
            inst_bswp(r, i);
            break;
            
        case 3: {
            r->increment = 4;
            uint64_t original = get_reg(r, EXT05_RR_RS(i));
            int64_t shamt = EXT8(EXT05_RR_I8(i));
            if (shamt >= 0) {
                set_reg(r, EXT05_RR_RD(i), shl(original, shamt));
            } else {
                set_reg(r, EXT05_RR_RD(i), shr(original, -shamt));
            }
        } break;
        case 4: {
            r->increment = 4;
            uint64_t original = get_reg(r, EXT05_RR_RS(i));
            int64_t shamt = EXT8(EXT05_RR_I8(i));
            if (shamt >= 0) {
                set_reg(r, EXT05_RR_RD(i), shl(original, shamt));
            } else {
                set_reg(r, EXT05_RR_RD(i), sar(original, -shamt));
            }
        } break;

        case 5: {
            r->increment = 4;
            
            uint64_t original = csx2valid(get_reg(r, EXT05_RR_RS(i)));
            
            if (!(bcd_valid(original))) {
                r->increment = 0;
                error(r, DECIMAL_FORMAT);
                return;
            }
            
            int64_t shamt = EXT8(EXT05_RR_I8(i));
            
            uint64_t result;
            if (shamt >= 0) {
                result = shl(original, shamt * 4);
            } else {
                result = shr(original, shamt * -4);
            }
            
            set_reg(r, EXT05_RR_RD(i), tc2csx(result));
        } break;

        default:
            r->increment = 0;
            error(r, ILLEGAL_INSTRUCTION);
    }
}

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>

#include "byteswap.h"
#include "error.h"
#include "mmio.h"
#include "lcca.h"

uint64_t get_reg_l(lcca_t *cpu, int reg) {
    if (reg == 0) {
        return 0;
    }

    else {
        uint64_t result = cpu->regs[reg] & 0xFFFFFFFF;
        return EXT32(result);
    }
}

void set_reg_l(lcca_t *cpu, int reg, uint64_t value) {
    if (reg != 0) {
        uint64_t result = value & 0xFFFFFFFF;
        cpu->regs[reg] = EXT32(result);
    }
}

uint64_t get_reg_q(lcca_t *cpu, int reg) {
    if (reg == 0) {
        return 0;
    }

    else {
        return cpu->regs[reg];
    }
}

void set_reg_q(lcca_t *cpu, int reg, uint64_t value) {
    if (reg != 0) {
        cpu->regs[reg] = value;
    }
}

uint64_t shl(uint64_t x, uint64_t shamt) {
    if (shamt == 64) return 0;
    else if (shamt < 64) return x << shamt;
    else return x;
}

uint64_t shr(uint64_t x, uint64_t shamt) {
    if (shamt == 64) return 0;
    else if (shamt < 64) return x >> shamt;
    else return x;
}

uint64_t sar(uint64_t x, uint64_t shamt) {
    if (shamt == 64) return 0;
    else if (shamt < 64) return (uint64_t) (((int64_t) x) >> shamt);
    else return x;
}

uint64_t sh(uint64_t x, int shamt) {
    if (shamt < 0) return shr(x, -shamt);
    else return shl(x, shamt);
}

uint64_t sha(uint64_t x, int shamt) {
    if (shamt < 0) return sar(x, -shamt);
    else return shl(x, shamt);
}

void lcca32_rr_0(lcca_t *cpu, uint32_t inst) {
    uint64_t b = get_reg_l(cpu, RB(inst));
    uint64_t c = get_reg_l(cpu, RC(inst));
    uint64_t d = RR_IMM(inst);
    d = EXT10(d);

    switch (FN(inst)) {
        case 0: set_reg_l(cpu, RA(inst), b + c + d); break;
        case 1: set_reg_l(cpu, RA(inst), b - (c + d)); break;
        case 2: set_reg_l(cpu, RA(inst), b & (c | d)); break;
        case 3: set_reg_l(cpu, RA(inst), b | (c & ~d)); break;
        case 4: set_reg_l(cpu, RA(inst), b ^ (c ^ d)); break;
        case 5: set_reg_l(cpu, RA(inst), sh(b, c + d)); break;
        case 6: set_reg_l(cpu, RA(inst), sha(b, c + d)); break;
    }
}

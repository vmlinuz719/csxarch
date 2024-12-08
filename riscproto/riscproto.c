#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
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
        case 5: set_reg_l(cpu, RA(inst), sh(b & 0xFFFFFFFF, c + d)); break;
        case 6: set_reg_l(cpu, RA(inst), sha(b, c + d)); break;
    }
}

void lcca32_br_1(lcca_t *cpu, uint32_t inst) {
    uint64_t a = get_reg_l(cpu, RA(inst));
    uint64_t d = BR_DISP(inst);
    d = EXT20(d);

    switch (FN(inst)) {
        case 0: {
            cpu->pc += a + (d << 2);
            cpu->pc &= 0xFFFFFFFF;
        } break;

        case 1: {
            set_reg_l(cpu, 31, cpu->pc);
            cpu->pc += a + (d << 2);
            cpu->pc &= 0xFFFFFFFF;
        } break;

        case 2: {
            cpu->pc += get_reg_l(cpu, 31) + (a + (d << 2));
            cpu->pc &= 0xFFFFFFFF;
        } break;

        case 3: {
            set_reg_l(cpu, RA(inst), d);
        } break;

        case 4: {
            if (a == 0) {
                cpu->pc += (d) << 2;
                cpu->pc &= 0xFFFFFFFF;
            }
        } break;

        case 5: {
            if (a != 0) {
                cpu->pc += (d) << 2;
                cpu->pc &= 0xFFFFFFFF;
            }
        } break;

        case 6: {
            if ((int64_t) a > 0) {
                cpu->pc += (d) << 2;
                cpu->pc &= 0xFFFFFFFF;
            }
        } break;

        case 7: {
            if ((int64_t) a <= 0) {
                cpu->pc += (d) << 2;
                cpu->pc &= 0xFFFFFFFF;
            }
        } break;
    }
}

void error(lcca_t *cpu, uint64_t e) {
    // TODO: Handle errors
    cpu->running = 0;
}

void lcca32_ls_2(lcca_t *cpu, uint32_t inst) {
    uint64_t c = get_reg_l(cpu, RC(inst));
    uint64_t d = LS_DISP(inst);
    d = EXT15(d);

    lcca_error_t e = 0;
    uint64_t result;
    int writeback = 1;

    // TODO: Check storage key

    switch (FN(inst)) {
        case 0: {
            result = read_u1b(cpu->bus, (c + d) & 0xFFFFFFFF, &e);
            result = EXT8(result);
        } break;

        case 1: {
            result = read_u1b(cpu->bus, (c + d) & 0xFFFFFFFF, &e);
        } break;

        case 2: {
            result = read_u2b(cpu->bus, (c + (d << 1)) & 0xFFFFFFFF, &e);
            result = EXT16(result);
        } break;

        case 3: {
            result = read_u2b(cpu->bus, (c + (d << 1)) & 0xFFFFFFFF, &e);
        } break;

        case 4: {
            result = read_u4b(cpu->bus, (c + (d << 2)) & 0xFFFFFFFF, &e);
        } break;

        case 5: {
            writeback = 0;
            write_1b(cpu->bus, (c + d) & 0xFFFFFFFF, get_reg_l(cpu, RA(inst)), &e);
        } break;

        case 6: {
            writeback = 0;
            write_2b(cpu->bus, (c + (d << 1)) & 0xFFFFFFFF, get_reg_l(cpu, RA(inst)), &e);
        } break;

        case 7: {
            writeback = 0;
            write_4b(cpu->bus, (c + (d << 2)) & 0xFFFFFFFF, get_reg_l(cpu, RA(inst)), &e);
        } break;
    }

    if (e) {
        error(cpu, e);
    }

    else if (writeback) {
        set_reg_l(cpu, RA(inst), result);
    }
}

void *lcca_run(lcca_t *cpu) {
    // TODO: Interrupts

    lcca_bus_t *bus = cpu->bus;
    lcca_error_t fetch_error = 0;

    // TODO: This is for quick and dirty benchmarking; remove it
    int i = 0;
    int target = 400000000;

    while(cpu->running && i++ < target) {
        uint32_t inst = read_u4b(bus, cpu->pc, &fetch_error);
        if (fetch_error) error(cpu, fetch_error);
        else {
            void (*operation) (struct lcca_t *, uint32_t) = cpu->operations[OPCODE(inst)];
            if (operation == NULL) error(cpu, ILLEGAL_INSTRUCTION);
            else {
                cpu->pc += 4;
                operation(cpu, inst);
            }
        }
    }

    return NULL;
}

void lcca_print(lcca_t *cpu) {
    printf("%%PC: %08X \n", (uint32_t) (cpu->pc & 0xFFFFFFFF));
    for (int i = 0; i < 32; i += 4) {
        printf(
            "%%%02d: %08X %%%02d: %08X %%%02d: %08X %%%02d: %08X \n",
            i, (uint32_t) (get_reg_l(cpu, i) & 0xFFFFFFFF),
            i + 1, (uint32_t) (get_reg_l(cpu, i + 1) & 0xFFFFFFFF),
            i + 2, (uint32_t) (get_reg_l(cpu, i + 2) & 0xFFFFFFFF),
            i + 3, (uint32_t) (get_reg_l(cpu, i + 3) & 0xFFFFFFFF)
        );
    }
}

int main(int argc, char *argv[]) {
    lcca_bus_t bus;

    uint8_t *mem = malloc(65536);
    bus.memory = mem;
    bus.mem_limit = 65536;

    mem[0] = 0x00;
    mem[1] = 0x80;
    mem[2] = 0x04;
    mem[3] = 0x01;

    mem[4] = 0x10;
    mem[5] = 0x0F;
    mem[6] = 0xFF;
    mem[7] = 0xFE;

    lcca_t cpu;
    memset(&cpu, 0, sizeof(cpu));
    cpu.bus = &bus;
    cpu.operations[0] = lcca64_rr_0;
    cpu.operations[1] = lcca64_br_1;
    cpu.operations[2] = lcca64_ls_2;
    cpu.operations[3] = lcca64_ls_ap_3;

    cpu.running = 1;
    lcca_run(&cpu);
    lcca64_print(&cpu);

    free(mem);

    return 0;
}

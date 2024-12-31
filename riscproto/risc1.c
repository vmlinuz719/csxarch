#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "byteswap.h"
#include "error.h"
#include "mmio.h"
#include "lcca.h"

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

void intr_internal(lcca_t *cpu, int which, uint64_t fi, uint64_t fa) {
    cpu->c_regs[CR_APC] = cpu->pc;
    cpu->c_regs[CR_APSQ] = cpu->c_regs[CR_PSQ];

    if (cpu->c_regs[CR_PSQ] & CR_PSQ_AE) {
        cpu->regs[R_ABI_X8] = cpu->regs[R_ABI_SP];
        cpu->regs[R_ABI_X9] = cpu->regs[R_ABI_LR];
        cpu->regs[R_ABI_SP] = cpu->c_regs[CR_ASP];
    }

    cpu->c_regs[CR_PSQ] &= CR_PSQ_INTR_ENTRY_MASK;
    cpu->pc = cpu->c_regs[CR_IA] + (4 * IA_HANDLER_INSTS) * which;
    cpu->c_regs[CR_FI] = fi;
    cpu->c_regs[CR_FA] = fa;
}

void intr_restore(lcca_t *cpu) {
    cpu->pc = cpu->c_regs[CR_APC];
    cpu->c_regs[CR_PSQ] = cpu->c_regs[CR_APSQ];

    if (cpu->c_regs[CR_PSQ] & CR_PSQ_AE) {
        cpu->c_regs[CR_ASP] = cpu->regs[R_ABI_SP];
        cpu->regs[R_ABI_SP] = cpu->regs[R_ABI_X8];
        cpu->regs[R_ABI_LR] = cpu->regs[R_ABI_X9];
    }
}

void error(lcca_t *cpu, lcca_error_t e, uint64_t fi, uint64_t fa) {
    switch (e) {
        case CPRC: {
            intr_internal(cpu, e, 0, fa);
        } break;

        case PBRK:
        case EMLT:
        case IPLT:
        case SVCT: {
            intr_internal(cpu, e, fi, 0);
        } break;

        case BERR:
        case RSGV:
        case WSGV:
        case DALT: {
            intr_internal(cpu, e, fi, fa);
        } break;

        default: {
            intr_internal(cpu, e, 0, 0);
        }
    }
}

void *lcca_run(lcca_t *cpu) {
    // TODO: External interrupts

    lcca_bus_t *bus = cpu->bus;
    lcca_error_t fetch_error;

    while(cpu->running) {
        fetch_error = 0;
        uint32_t inst;
        uint64_t addr = translate(cpu, cpu->pc, LONG, FETCH, &fetch_error);
        if (!fetch_error) {
            inst = fetch_u4b(bus, addr, &fetch_error);
        }
        if (fetch_error) error(cpu, fetch_error, 0, addr);
        else if (cpu->c_regs[CR_PSQ] & CR_PSQ_LG) {
            cpu->pc += 4;
            uint64_t a = get_reg_q(cpu, RA(inst));
            set_reg_q(cpu, RA(inst), a | ((uint64_t) LGISL2_IMM(inst) << 14));
            cpu->c_regs[CR_PSQ] ^= CR_PSQ_LG;
        }
        else {
            void (*operation) (struct lcca_t *, uint32_t) = cpu->operations[OPCODE(inst)];
            if (operation == NULL) error(cpu, EMLT, inst, cpu->pc);
            else {
                cpu->pc += 4;
                operation(cpu, inst);
            }
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    FILE *rom = NULL;
    if (argc == 1) {
        printf("Warning: running without IPL\n");
    } else if (argc == 2) {
        rom = fopen(argv[1], "rb");
        if (rom == NULL) {
            printf("Error: IPL image failed to open\n");
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "usage: %s [in_file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    lcca_bus_t bus;
    pthread_mutex_t cas_lock;

    uint8_t *mem = malloc(65536 * 32);
    bus.memory = mem;
    
    if (rom != NULL) {
        fread(mem + 0x10000, sizeof(char), 0x10000, rom);
        fclose(rom);
    }
    
    bus.mem_limit = 65536 * 32;
    pthread_mutex_init(&cas_lock, NULL);
    bus.cas_lock = &cas_lock;

    lcca_t cpu;
    memset(&cpu, 0, sizeof(cpu));
    cpu.bus = &bus;
    cpu.operations[0] = lcca64_rr_0;
    cpu.operations[1] = lcca64_br_1;
    cpu.operations[2] = lcca64_ls_2;
    cpu.operations[3] = lcca64_ls_3;
    cpu.operations[4] = lcca64_im_4;
    cpu.operations[5] = lcca64_im_5;
    cpu.operations[6] = lcca64_im_6;
    cpu.operations[0xE] = lcca64_ls_e;
    cpu.pc = 0x10000;
    pthread_mutex_init(&(cpu.intr_mutex), NULL);

    cpu.running = 1;
    lcca_run(&cpu);

    free(mem);
    pthread_mutex_destroy(&(cpu.intr_mutex));
    pthread_mutex_destroy(&cas_lock);

    return 0;
}

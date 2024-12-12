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

void intr_internal(lcca_t *cpu, int which) {
    cpu->c_regs[CR_APC] = cpu->pc;
    cpu->c_regs[CR_APSQ] = cpu->c_regs[CR_PSQ];

    if (cpu->c_regs[CR_PSQ] & CR_PSQ_AE) {
        cpu->regs[R_ABI_X8] = cpu->regs[R_ABI_SP];
        cpu->regs[R_ABI_X9] = cpu->regs[R_ABI_LR];
        cpu->regs[R_ABI_SP] = cpu->c_regs[CR_ASP];
    }

    cpu->c_regs[CR_PSQ] &= CR_PSQ_INTR_ENTRY_MASK;
    cpu->pc = cpu->c_regs[CR_IA] + (4 * IA_HANDLER_INSTS) * which;
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

void error(lcca_t *cpu, uint64_t e) {
    // TODO: Handle errors
    cpu->running = 0;
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
    cpu.operations[3] = lcca64_im_3;
    cpu.operations[4] = lcca64_im_4;
    cpu.operations[5] = lcca64_ls_ap_5;
    pthread_mutex_init(&(cpu.intr_mutex), NULL);

    cpu.running = 1;
    lcca_run(&cpu);
    lcca64_print(&cpu);

    free(mem);
    pthread_mutex_destroy(&(cpu.intr_mutex));

    return 0;
}

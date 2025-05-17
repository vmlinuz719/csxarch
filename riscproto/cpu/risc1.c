#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "byteswap.h"
#include "error.h"
#include "mmio.h"
#include "lcca.h"
#include "console.h"
#include "blinkenlights.h"

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

void intr_restore_disp(lcca_t *cpu, uint64_t d) {
    cpu->pc = cpu->c_regs[CR_APC] + d;
    cpu->c_regs[CR_PSQ] = cpu->c_regs[CR_APSQ];

    if (cpu->c_regs[CR_PSQ] & CR_PSQ_AE) {
        cpu->c_regs[CR_ASP] = cpu->regs[R_ABI_SP];
        cpu->regs[R_ABI_SP] = cpu->regs[R_ABI_X8];
        cpu->regs[R_ABI_LR] = cpu->regs[R_ABI_X9];
    }
}

void intr_restore(lcca_t *cpu) {
    intr_restore_disp(cpu, 0);
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
        case DALT: 
        case PGNP:
        case PGNM:
        case PGNW:
        case NXMU: {
            intr_internal(cpu, e, fi, fa);
        } break;

        default: {
            intr_internal(cpu, e, 0, 0);
        }
    }
}

void lcca_wake(lcca_t *cpu) {
    pthread_cond_signal(&(cpu->wake));
}

void lcca_intr(void *dev, int intr, uint64_t msg) {
    lcca_t *cpu = dev;

    pthread_mutex_lock(&(cpu->intr_mutex));
    cpu->intr_msg[intr] = msg;
    cpu->intr_pending |= 1L << intr;
    pthread_mutex_unlock(&(cpu->intr_mutex));
    lcca_wake(cpu);
}

void *lcca_run(lcca_t *cpu) {
    lcca_bus_t *bus = cpu->bus;
    lcca_error_t fetch_error;

    uint64_t now_pending = 0;

    while(cpu->running) {
        if ((cpu->c_regs[CR_PSQ] & CR_PSQ_EI) && (now_pending = cpu->intr_pending & cpu->c_regs[CR_EIM])) {
            for (int i = 0; i < EIP_EXTERNAL_INTRS; i++) {
                if (now_pending & (1L << i)) {
                    pthread_mutex_lock(&(cpu->intr_mutex));
                    cpu->intr_pending ^= 1L << i;
                    cpu->c_regs[CR_EIP] = cpu->intr_msg[i];
                    pthread_mutex_unlock(&(cpu->intr_mutex));
                    intr_internal(cpu, EXTN, 0, 0);
                }
            }
        }

        if (!(cpu->c_regs[CR_PSQ] & CR_PSQ_WS)) {
            fetch_error = 0;
            uint32_t inst = 0;
            uint64_t addr = translate(cpu, cpu->pc, LONG, FETCH, &fetch_error);
            if (!fetch_error) {
                inst = fetch_u4b(bus, addr, &fetch_error);
            }
            if (!fetch_error) cpu->inst = inst;
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
        } else {
            pthread_mutex_lock(&(cpu->intr_mutex));
            if (!(cpu->c_regs[CR_PSQ] & CR_PSQ_EI)) {
                struct timespec millisecond;
                millisecond.tv_nsec = 100000000;
                millisecond.tv_sec = 0;
                nanosleep(&millisecond, NULL);
                fprintf(stderr, "\nCPU disabled, exiting emulator at:\n");
                // TODO: Deduplicate PSQ printing
                fprintf(stderr, " PC: %16lX PSQ: %016lX (%03lX", cpu->pc, cpu->c_regs[CR_PSQ], (cpu->c_regs[CR_PSQ] & CR_PSQ_PGID) >> 6);
                print_bits(cpu->c_regs[CR_PSQ], psq_bits);
                fprintf(stderr, ")\n");
                return NULL;
            }
            pthread_cond_wait(
                &(cpu->wake),
                &(cpu->intr_mutex)
            );
            pthread_mutex_unlock(&(cpu->intr_mutex));
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    FILE *rom = NULL;
    if (argc == 1) {
        fprintf(stderr, "Warning: running without IPL\n");
    } else if (argc == 2) {
        rom = fopen(argv[1], "rb");
        if (rom == NULL) {
            fprintf(stderr, "Error: IPL image failed to open\n");
            return EXIT_FAILURE;
        }
    } else {
        fprintf(stderr, "usage: %s [in_file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    lcca_bus_t bus;
    pthread_mutex_t cas_lock;

    mmio_unit_t mmio[4096];
    memset(mmio, 0, sizeof(mmio_unit_t) * 4096);

    uint8_t *mem = malloc(65536 * 32);
    bus.memory = mem;
    
    if (rom != NULL) {
        int x = fread(mem + 0x10000, sizeof(char), 0x10000, rom);
        fclose(rom);
    }
    
    bus.mem_limit = 65536 * 32;
    pthread_mutex_init(&cas_lock, NULL);
    bus.cas_lock = &cas_lock;
    bus.mmio = mmio;
    bus.num_units = 4096;

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
    cpu.operations[7] = lcca64_xmu_7;
    cpu.operations[10] = lcca64_ls_a;
    cpu.operations[11] = lcca64_ls_b;
    cpu.operations[12] = lcca64_rr_c;
    cpu.operations[14] = lcca64_ls_e;
    cpu.pc = 0x10000;
    pthread_mutex_init(&(cpu.intr_mutex), NULL);
    pthread_cond_init(&(cpu.wake), NULL);

    init_blink(&(mmio[0x0]), &cpu);
    init_console(&(mmio[0x101]), &cpu);

    cpu.running = 1;
    lcca_run(&cpu);

    mmio[0x0].destroy(mmio[0x0].ctx);
    mmio[0x101].destroy(mmio[0x101].ctx);

    free(mem);
    tlb_destroy(&(cpu.tlb));
    pthread_cond_destroy(&(cpu.wake));
    pthread_mutex_destroy(&(cpu.intr_mutex));
    pthread_mutex_destroy(&cas_lock);

    return 0;
}

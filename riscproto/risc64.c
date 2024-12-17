#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "byteswap.h"
#include "error.h"
#include "mmio.h"
#include "lcca.h"

void lcca64_rr_0(lcca_t *cpu, uint32_t inst) {
    uint64_t b = get_reg_q(cpu, RB(inst));
    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = RR_IMM(inst);
    d = EXT10(d);

    switch (FN(inst)) {
        case 0: set_reg_q(cpu, RA(inst), b + c + d); break;
        case 1: set_reg_q(cpu, RA(inst), b - (c + d)); break;
        case 2: set_reg_q(cpu, RA(inst), b & (c | d)); break;
        case 3: set_reg_q(cpu, RA(inst), b | (c & ~d)); break;
        case 4: set_reg_q(cpu, RA(inst), b ^ (c ^ d)); break;
        case 5: set_reg_q(cpu, RA(inst), sh(b, c + d)); break;
        case 6: set_reg_q(cpu, RA(inst), sha(b, c + d)); break;
        case 7: set_reg_q(cpu, RA(inst), (b > c + d) ? 1 : 0); break;
    }
}

void lcca64_br_1(lcca_t *cpu, uint32_t inst) {
    uint64_t a = get_reg_q(cpu, RA(inst));
    uint64_t d = BR_DISP(inst);
    d = EXT20(d);

    switch (FN(inst)) {
        case 0: {
            cpu->pc += a + (d << 2);
        } break;

        case 1: {
            set_reg_q(cpu, R_ABI_LR, cpu->pc);
            cpu->pc += a + (d << 2);
        } break;

        case 2: {
            cpu->pc += get_reg_q(cpu, 31) + (a + (d << 2));
        } break;

        case 3: {
            cpu->pc += a + (d << 2);
        } break;

        case 4: {
            if (a == 0) {
                cpu->pc += (d) << 2;
            }
        } break;

        case 5: {
            if (a != 0) {
                cpu->pc += (d) << 2;
            }
        } break;

        case 6: {
            if ((int64_t) a > 0) {
                cpu->pc += (d) << 2;
            }
        } break;

        case 7: {
            if ((int64_t) a <= 0) {
                cpu->pc += (d) << 2;
            }
        } break;
    }
}

void lcca64_ls_2(lcca_t *cpu, uint32_t inst) {
    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = LS_DISP(inst);
    d = EXT15(d);

    lcca_error_t e = 0;
    uint64_t result;
    int writeback = 1;

    uint64_t addr;

    // TODO: Translate address and check access rights

    switch (FN(inst)) {
        case 0: {
            addr = c + d;
            result = read_u1b(cpu->bus, addr, &e);
            result = EXT8(result);
        } break;

        case 1: {
            addr = c + d;
            result = read_u1b(cpu->bus, addr, &e);
        } break;

        case 2: {
            addr = (c + (d << 1));
            result = read_u2b(cpu->bus, addr, &e);
            result = EXT16(result);
        } break;

        case 3: {
            addr = (c + (d << 1));
            result = read_u2b(cpu->bus, addr, &e);
        } break;

        case 4: {
            addr = (c + (d << 2));
            result = read_u4b(cpu->bus, addr, &e);
            result = EXT32(result);
        } break;

        case 5: {
            writeback = 0;
            addr = c + d;
            write_1b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
        } break;

        case 6: {
            writeback = 0;
            addr = (c + (d << 1));
            write_2b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
        } break;

        case 7: {
            writeback = 0;
            addr = (c + (d << 2));
            write_4b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
        } break;
    }

    if (e) {
        error(cpu, e, inst, addr);
    }

    else if (writeback) {
        set_reg_q(cpu, RA(inst), result);
    }
}

void lcca64_im_3(lcca_t *cpu, uint32_t inst) {
    uint64_t d = IM_IMM(inst);
    d = EXT23(d);
    set_reg_q(cpu, RA(inst), d);
}

void lcca64_im_4(lcca_t *cpu, uint32_t inst) {
    uint64_t d = IM_IMM(inst);
    d = EXT23(d);
    set_reg_q(cpu, RA(inst), d << 9);
}

const char *abi_names[] = {
    " ZR", " CT", NULL,
    
    " RV", " P0", " P1", " P2", " P3", " P4", NULL,

    " A0", " A1", " A2", " A3", " A4", " A5", " A6", " A7",
    " A8", " A9", "A10", "A11", NULL,

    " X0", " X1", " X2", " X3", " X4", " X5", " X6", " X7",
    " X8", " X9", NULL,

    " SP", " LR", NULL, NULL
};

const char *cr_names[] = {
    "PSQ", "APC", "APS", "ASP", NULL,
    " IA", "EIM", "EIP", " FI", " FA", NULL, NULL
};

static inline void print_regs(uint64_t *regs, const char *names[]) {
    int index = 0, r_index = 0;
    int newline_ctr = 0;

    while (1) {
        printf("%s: %16lX ", names[index], regs[r_index++]);
        if (names[++index] == NULL) {
            printf("\n\n");
            newline_ctr = 0;
            if (names[++index] == NULL) {
                break;
            }
        }

        else if (++newline_ctr == 3) {
            printf("\n");
            newline_ctr = 0;
        }
    }
}

const char *psq_bits = "OTAWPI";
const char *d_bits = "XWRxwBCDE*";

static inline void print_bits(uint64_t n, const char *bits) {
    int len = strlen(bits);
    for (int i = len; i > 0;  i--) {
        int shamt = i - 1;
        printf("%c", (n >> shamt) & 1 ? bits[len - i] : ' ');
    }
}

void simdbg_0(lcca_t *cpu, uint32_t inst) {
    uint64_t a = RA(inst);
    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = LS_DISP(inst);
    d = EXT15(d);

    switch (a) {
        case 0: {
            printf(" PC: %16lX PSQ: %016lX (", cpu->pc, cpu->c_regs[CR_PSQ]);
            print_bits(cpu->c_regs[CR_PSQ], psq_bits);
            printf(")\n\n");
        } break;

        case 1: {
            print_regs(&(cpu->regs[1]), &(abi_names[1]));
        } break;

        case 2: {
            print_regs(&(cpu->c_regs[1]), &(cr_names[1]));
        } break;

        case 3: {
            for (int i = 0; i < 16; i++) {
                printf("Object %01X: %14lX (", i, cpu->c_regs[CR_OB0 + i] >> 10);
                print_bits(cpu->c_regs[CR_OD0 + i], d_bits);
                printf(") +%-13lX\n", cpu->c_regs[CR_OD0 + i] >> 10);
            }
        } break;

        case 4: {
            int rc = (int) c;
            exit(c);
        } break;
    }
}

void lcca64_ls_ap_5(lcca_t *cpu, uint32_t inst) {
    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = LS_DISP(inst);
    d = EXT15(d);

    lcca_error_t e = 0;
    uint64_t result;
    int writeback = 1;

    uint64_t addr;

    // TODO: Translate address and check access rights

    switch (FN(inst)) {
        case 0: {
            addr = (c + (d << 2));
            result = read_u4b(cpu->bus, addr, &e);
        } break;

        case 1: {
            addr = (c + (d << 3));
            result = read_8b(cpu->bus, addr, &e);
        } break;

        case 2: {
            writeback = 0;
            addr = (c + (d << 3));
            write_8b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
        } break;

        case 3: {
            if (cpu->c_regs[CR_PSQ] && CR_PSQ_PL) {
                error(cpu, IPLV, inst, 0);
                return;
            }

            if (d < 0 || d > CR_MAX) {
                error(cpu, EMLT, inst, 0);
                return;
            }

            else {
                result = cpu->c_regs[d] & (~c);
            }
        } break;

        case 4: {
            if (cpu->c_regs[CR_PSQ] && CR_PSQ_PL) {
                error(cpu, IPLV, inst, 0);
                return;
            }

            if (d < 0 || d > CR_MAX) {
                error(cpu, EMLT, inst, 0);
                return;
            }

            else {
                writeback = 0;
                cpu->c_regs[d] = get_reg_q(cpu, RA(inst)) | c;
            }
        } break;
        
        case 5: {
            if (d != 0) {
                error(cpu, SVCT, inst, 0);
                return;
            }

            if (cpu->c_regs[CR_PSQ] && CR_PSQ_PL) {
                error(cpu, IPLV, inst, 0);
                return;
            }

            else {
                result = c;
                intr_restore(cpu);
            }
        } break;

        case 6: {
            intr_internal(cpu, d & 63, inst, cpu->pc);
            return;
        }

        case 7: {
            if (cpu->c_regs[CR_PSQ] && CR_PSQ_PL) {
                error(cpu, IPLV, inst, 0);
                return;
            }

            simdbg_0(cpu, inst);
            return;
        }
    }

    if (e) {
        error(cpu, e, inst, addr);
    }

    else if (writeback) {
        set_reg_q(cpu, RA(inst), result);
    }
}

void lcca64_print(lcca_t *cpu) {
    printf("%%PC: %016lX \n", cpu->pc);
    for (int i = 0; i < 32; i += 2) {
        printf(
            "%%%02d: %016lX %%%02d: %016lX \n",
            i, get_reg_q(cpu, i),
            i + 1, get_reg_q(cpu, i + 1)
        );
    }
}

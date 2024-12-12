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

    // TODO: Check storage key

    switch (FN(inst)) {
        case 0: {
            result = read_u1b(cpu->bus, (c + d), &e);
            result = EXT8(result);
        } break;

        case 1: {
            result = read_u1b(cpu->bus, (c + d), &e);
        } break;

        case 2: {
            result = read_u2b(cpu->bus, (c + (d << 1)), &e);
            result = EXT16(result);
        } break;

        case 3: {
            result = read_u2b(cpu->bus, (c + (d << 1)), &e);
        } break;

        case 4: {
            result = read_u4b(cpu->bus, (c + (d << 2)), &e);
            result = EXT32(result);
        } break;

        case 5: {
            writeback = 0;
            write_1b(cpu->bus, (c + d), get_reg_q(cpu, RA(inst)), &e);
        } break;

        case 6: {
            writeback = 0;
            write_2b(cpu->bus, (c + (d << 1)), get_reg_q(cpu, RA(inst)), &e);
        } break;

        case 7: {
            writeback = 0;
            write_4b(cpu->bus, (c + (d << 2)), get_reg_q(cpu, RA(inst)), &e);
        } break;
    }

    if (e) {
        error(cpu, e);
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

void lcca64_ls_ap_5(lcca_t *cpu, uint32_t inst) {
    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = LS_DISP(inst);
    d = EXT15(d);

    lcca_error_t e = 0;
    uint64_t result;
    int writeback = 1;

    // TODO: Check storage key

    switch (FN(inst)) {
        case 0: {
            result = read_u4b(cpu->bus, (c + (d << 2)), &e);
        } break;

        case 1: {
            result = read_8b(cpu->bus, (c + (d << 3)), &e);
        } break;

        case 2: {
            writeback = 0;
            write_8b(cpu->bus, (c + (d << 3)), get_reg_q(cpu, RA(inst)), &e);
        } break;

        default: {
            error(cpu, ILLEGAL_INSTRUCTION);
            return;
        }
    }

    if (e) {
        error(cpu, e);
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

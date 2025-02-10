#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "byteswap.h"
#include "error.h"
#include "mmio.h"
#include "lcca.h"

#define SIGN_BIT 0x8000000000000000

void lcca64_rr_0(lcca_t *cpu, uint32_t inst) {
    uint64_t b = get_reg_q(cpu, RB(inst));
    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = RR_IMM(inst);
    d = EXT10(d);
    uint64_t fn = FN(inst);

    if ((cpu->c_regs[CR_PSQ] & CR_PSQ_OC) && (fn < 2)) {
        int overflow;
        uint64_t addend = c + d;
        if (fn == 1) addend = ~addend + 1;
        uint64_t sum = b + addend;

        overflow = ((b & SIGN_BIT) && (addend & SIGN_BIT) && !(sum & SIGN_BIT))
            || (!(b & SIGN_BIT) && !(addend & SIGN_BIT) && (sum & SIGN_BIT));

        if (overflow) {
            error(cpu, OVRF, 0, 0);
            return;
        }
    }

    switch (fn) {
        case 0: set_reg_q(cpu, RA(inst), b + c + d); break;
        case 1: set_reg_q(cpu, RA(inst), b - (c + d)); break;
        case 2: set_reg_q(cpu, RA(inst), b & (c | d)); break;
        case 3: set_reg_q(cpu, RA(inst), b | (c | d)); break;
        case 4: set_reg_q(cpu, RA(inst), b ^ (c ^ d)); break;
        case 5: set_reg_q(cpu, RA(inst), sh(b, c + d)); break;
        case 6: set_reg_q(cpu, RA(inst), sha(b, c + d)); break;
        case 7: set_reg_q(cpu, RA(inst), (b > c + d) ? 1 : 0); break;
    }
}

static inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

void lcca64_xmu_7(lcca_t *cpu, uint32_t inst) {
    if (cpu->c_regs[CR_PSQ] & CR_PSQ_PL) {
        error(cpu, IPLT, inst, 0);
        return;
    }

    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = LS_DISP(inst);
    d = EXT15(d);
    uint64_t fn = FN(inst);

    switch (fn) {
        case 0: {
            uint64_t z = (c += UINT64_C(0x9E3779B97F4A7C15));
            z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
            z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
            z = z ^ (z >> 31);
            cpu->rng[0] = z;
            
            z = (c += UINT64_C(0x9E3779B97F4A7C15));
            z = (z ^ (z >> 30)) * UINT64_C(0xBF58476D1CE4E5B9);
            z = (z ^ (z >> 27)) * UINT64_C(0x94D049BB133111EB);
            z = z ^ (z >> 31);
            cpu->rng[1] = z;
        } break;
        
        case 1: {
            const uint64_t s0 = cpu->rng[0];
            uint64_t s1 = cpu->rng[1];
            const uint64_t result = rotl(s0 * 5, 7) * 9;

            s1 ^= s0;
            cpu->rng[0] = rotl(s0, 24) ^ s1 ^ (s1 << 16); // a, b
            cpu->rng[1] = rotl(s1, 37); // c
            set_reg_q(cpu, RA(inst), result);
        } break;
        
        case 2: {
            lcca_error_t e = 0;
            uint64_t addr = translate(cpu, c + d, CHAR, READ, &e);
            if (e) {
                error(cpu, e, inst, addr);
                return;
            }
            set_reg_q(cpu, RA(inst), addr);
        } break;

        default: {
            error(cpu, EMLT, inst, 0);
            return;
        }
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
            cpu->pc = a + (d << 2);
        } break;

        case 2: {
            set_reg_q(cpu, R_ABI_LR, cpu->pc);
            cpu->pc += a + (d << 2);
        } break;

        case 3: {
            set_reg_q(cpu, R_ABI_LR, cpu->pc);
            cpu->pc = a + (d << 2);
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

uint64_t translate(lcca_t *cpu, uint64_t addr, lcca_size_t size, lcca_access_t access_type, lcca_error_t *e) {
    if (!(cpu->c_regs[CR_PSQ] & CR_PSQ_PGID)) return addr;

    if (addr & ((1 << size) - 1)) {
        *e = access_type == FETCH ? XALT : DALT;
        return addr;
    }

    uint64_t object = addr >> 60;
    uint64_t base = cpu->c_regs[CR_OB0 + object] & 0xFFFFFFFFFFFFFC00;

    uint64_t limit = cpu->c_regs[CR_OD0 + object] & 0x0FFFFFFFFFFFFC00;
    uint64_t rights = (cpu->c_regs[CR_OD0 + object] & 0x3FF) | (1 << 10);

    uint64_t offset = addr & 0x0FFFFFFFFFFFFFFF;
    if (offset >> 10 > limit >> 10) {
        switch (access_type) {
            case READ: *e = RSGV; break;
            case WRITE: *e = WSGV; break;
            case FETCH: *e = XSGV; break;
        }
        return addr;
    }

    rights &= ((cpu->c_regs[CR_PSQ] & CR_PSQ_PL)
        ? (CR_OD_R | CR_OD_x | CR_OD_w)
        : (CR_OD_X | CR_OD_W | (1 << 10)))
        | 31;

    if (!(rights & access_type)) {
        switch (access_type) {
            case READ: *e = RSGV; break;
            case WRITE: *e = WSGV; break;
            case FETCH: *e = XSGV; break;
        }
        return addr;
    }

    if (access_type == WRITE) {
        cpu->c_regs[CR_OD0 + object] |= CR_OD_D;
    }

    addr = base + offset;

    // fprintf(stderr, "A %lX\n", base + offset);
    if (!(rights & CR_OD_P)) {
        return addr;
    }

    else {
        // TODO: TLB lookup
        *e = NXMU;
        return addr;
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

    switch (FN(inst)) {
        case 0: {
            addr = translate(cpu, c + d, CHAR, READ, &e);
            if (!e) {
                result = read_u1b(cpu->bus, addr, &e);
                result = EXT8(result);
            }
        } break;

        case 1: {
            addr = translate(cpu, c + d, CHAR, READ, &e);
            if (!e) {
                result = read_u1b(cpu->bus, addr, &e);
            }
        } break;

        case 2: {
            addr = translate(cpu, c + (d << 1), WORD, READ, &e);
            if (!e) {
                result = read_u2b(cpu->bus, addr, &e);
                result = EXT16(result);
            }
        } break;

        case 3: {
            addr = translate(cpu, c + (d << 1), WORD, READ, &e);
            if (!e) {
                result = read_u2b(cpu->bus, addr, &e);
            }
        } break;

        case 4: {
            addr = translate(cpu, c + (d << 2), LONG, READ, &e);
            if (!e) {
                result = read_u4b(cpu->bus, addr, &e);
                result = EXT32(result);
            }
        } break;

        case 5: {
            writeback = 0;
            addr = translate(cpu, c + d, CHAR, WRITE, &e);
            if (!e) {
                write_1b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
            }
        } break;

        case 6: {
            writeback = 0;
            addr = translate(cpu, c + (d << 1), WORD, WRITE, &e);
            if (!e) {
                write_2b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
            }
        } break;

        case 7: {
            writeback = 0;
            addr = translate(cpu, c + (d << 2), LONG, WRITE, &e);
            if (!e) {
                write_4b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
            }
        } break;
    }

    if (e) {
        error(cpu, e, inst, addr);
    }

    else if (writeback) {
        set_reg_q(cpu, RA(inst), result);
    }
}

uint64_t selector(uint64_t seg, uint64_t base, uint64_t disp, uint64_t size) {
    uint64_t va = ((disp << size) + base) & 0x0FFFFFFFFFFFFFFF;
    return va | (seg << 60);
}

void lcca64_ls_a(lcca_t *cpu, uint32_t inst) {
    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = LSS_DISP(inst);
    d = EXT12(d);
    uint64_t s = LSS_SEL(inst);
    if (!(cpu->c_regs[CR_PSQ] & CR_PSQ_PL)) {
        s += 4;
    }

    lcca_error_t e = 0;
    uint64_t result;
    int writeback = 1;

    uint64_t addr;

    switch (FN(inst)) {
        case 0: {
            addr = translate(cpu, selector(s, c, d, 0), CHAR, READ, &e);
            if (!e) {
                result = read_u1b(cpu->bus, addr, &e);
                result = EXT8(result);
            }
        } break;

        case 1: {
            addr = translate(cpu, selector(s, c, d, 0), CHAR, READ, &e);
            if (!e) {
                result = read_u1b(cpu->bus, addr, &e);
            }
        } break;

        case 2: {
            addr = translate(cpu, selector(s, c, d, 1), WORD, READ, &e);
            if (!e) {
                result = read_u2b(cpu->bus, addr, &e);
                result = EXT16(result);
            }
        } break;

        case 3: {
            addr = translate(cpu, selector(s, c, d, 1), WORD, READ, &e);
            if (!e) {
                result = read_u2b(cpu->bus, addr, &e);
            }
        } break;

        case 4: {
            addr = translate(cpu, selector(s, c, d, 2), LONG, READ, &e);
            if (!e) {
                result = read_u4b(cpu->bus, addr, &e);
                result = EXT32(result);
            }
        } break;

        case 5: {
            writeback = 0;
            addr = translate(cpu, selector(s, c, d, 0), CHAR, WRITE, &e);
            if (!e) {
                write_1b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
            }
        } break;

        case 6: {
            writeback = 0;
            addr = translate(cpu, selector(s, c, d, 1), WORD, WRITE, &e);
            if (!e) {
                write_2b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
            }
        } break;

        case 7: {
            writeback = 0;
            addr = translate(cpu, selector(s, c, d, 2), LONG, WRITE, &e);
            if (!e) {
                write_4b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
            }
        } break;
    }

    if (e) {
        error(cpu, e, inst, addr);
    }

    else if (writeback) {
        set_reg_q(cpu, RA(inst), result);
    }
}

void lcca64_im_4(lcca_t *cpu, uint32_t inst) {
    uint64_t d = IM_IMM(inst);
    d = EXT23(d);
    set_reg_q(cpu, RA(inst), d);
}

void lcca64_im_5(lcca_t *cpu, uint32_t inst) {
    uint64_t d = IM_IMM(inst);
    d = EXT23(d);
    set_reg_q(cpu, RA(inst), d << 9);
}

void lcca64_im_6(lcca_t *cpu, uint32_t inst) {
    uint64_t d = IM_IMM(inst);
    d = EXT23(d);
    set_reg_q(cpu, RA(inst), d << 41);
    cpu->c_regs[CR_PSQ] |= CR_PSQ_LG;
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
        fprintf(stderr, "%s: %16lX ", names[index], regs[r_index++]);
        if (names[++index] == NULL) {
            fprintf(stderr, "\n\n");
            newline_ctr = 0;
            if (names[++index] == NULL) {
                break;
            }
        }

        else if (++newline_ctr == 3) {
            fprintf(stderr, "\n");
            newline_ctr = 0;
        }
    }
}

const char *psq_bits = "AOLWPI";
const char *d_bits = "XWRxwBCDEP";

void print_bits(uint64_t n, const char *bits) {
    int len = strlen(bits);
    for (int i = len; i > 0;  i--) {
        int shamt = i - 1;
        fprintf(stderr, "%c", (n >> shamt) & 1 ? bits[len - i] : '-');
    }
}

void simdbg_0(lcca_t *cpu, uint32_t inst) {
    uint64_t a = RA(inst);
    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = LS_DISP(inst);
    d = EXT15(d);

    switch (a) {
        case 0: {
            fprintf(stderr, " PC: %16lX PSQ: %016lX (%03lX", cpu->pc, cpu->c_regs[CR_PSQ], (cpu->c_regs[CR_PSQ] & CR_PSQ_PGID) >> 6);
            print_bits(cpu->c_regs[CR_PSQ], psq_bits);
            fprintf(stderr, ")\n\n");
        } break;

        case 1: {
            print_regs(&(cpu->regs[1]), &(abi_names[1]));
        } break;

        case 2: {
            print_regs(&(cpu->c_regs[1]), &(cr_names[1]));
        } break;

        case 3: {
            int printed = 0;
            for (int i = 0; i < 16; i++) {
                if (cpu->c_regs[CR_OB0 + i] || cpu->c_regs[CR_OD0 + i]) {
                    printed = 1;
                    fprintf(stderr, "Object %01X: %16lX (", i, cpu->c_regs[CR_OB0 + i]);
                    print_bits(cpu->c_regs[CR_OD0 + i], d_bits);
                    fprintf(stderr, ") + %15lX\n", cpu->c_regs[CR_OD0 + i] & 0x0FFFFFFFFFFFFC00);
                }
            }
            if (!printed) {
                fprintf(stderr, "No objects in use\n");
            }
        } break;
        
        case 4: {
            fprintf(stderr, "%2d: %lX\n", RC(inst), c);
        } break;

        case 5:
        case 6: {
            uint64_t va = c;
            char ch = 0;
            lcca_error_t e = 0;
            uint64_t addr = 0;
            while (1) {
                addr = translate(cpu, va++, CHAR, READ, &e);
                if (e) {
                    error(cpu, e, inst, addr);
                    break;
                }

                ch = read_u1b(cpu->bus, addr, &e);
                if (e || !ch) {
                    if (e) {
                        error(cpu, e, inst, addr);
                    }
                    break;
                }

                putc(ch, stderr);
            }

            if (!e && a == 6) {
                putc('\n', stderr);
            }
        } break;

        case 31: {
            int rc = (int) c;
            exit(rc);
        } break;
    }
}

void lcca64_ls_3(lcca_t *cpu, uint32_t inst) {
    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = LS_DISP(inst);
    d = EXT15(d);

    lcca_error_t e = 0;
    uint64_t result;
    int writeback = 1;

    uint64_t addr;

    switch (FN(inst)) {
        case 0: {
            addr = translate(cpu, c + (d << 2), LONG, READ, &e);
            if (!e) {
                result = read_u4b(cpu->bus, addr, &e);
            }
        } break;

        case 1: {
            addr = translate(cpu, c + (d << 3), QUAD, READ, &e);
            if (!e) {
                result = read_8b(cpu->bus, addr, &e);
            }
        } break;

        case 2: {
            result = c + d;
        } break;

        case 3: {
            writeback = 0;
            addr = translate(cpu, c + (d << 3), QUAD, WRITE, &e);
            if (!e) {
                write_8b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
            }
        } break;

        case 4: {
            uint64_t e = (d >> 6) & 0x3FF;
            uint64_t f = shr(c, e);
            uint64_t mask = shl(sar(1L << 63, 63 - (d & 0x3F)), 1);

            result = (f & shl(1L, d & 0x3F))
                ? f | mask
                : f & (~(mask));
        } break;

        case 5: {
            uint64_t e = (d >> 6) & 0x3FF;
            uint64_t mask = shl(sar(1L << 63, 63 - (d & 0x3F)), 1);
            uint64_t mask2 = shl(sar(1L << 63, 63 - (e & 0x3F)), 1);

            result = (c & shl(1L, d & 0x3F))
                ? c | mask
                : c & (~(mask));
            result &= ~mask2;
        } break;
        
        case 6: {
            uint64_t e = (d >> 6) & 0x3FF;
            uint64_t mask = shl(sar(1L << 63, 63 - (d & 0x3F)), 1);
            result = sar(c, e) & (~(mask));
        } break;

        default: {
            error(cpu, EMLT, inst, 0);
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

void lcca64_ls_b(lcca_t *cpu, uint32_t inst) {
    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = LSS_DISP(inst);
    d = EXT12(d);
    uint64_t s = LSS_SEL(inst);
    if (!(cpu->c_regs[CR_PSQ] & CR_PSQ_PL)) {
        s += 4;
    }

    lcca_error_t e = 0;
    uint64_t result;
    int writeback = 1;

    uint64_t addr;

    switch (FN(inst)) {
        case 0: {
            addr = translate(cpu, selector(s, c, d, 2), LONG, READ, &e);
            if (!e) {
                result = read_u4b(cpu->bus, addr, &e);
            }
        } break;

        case 1: {
            addr = translate(cpu, selector(s, c, d, 3), QUAD, READ, &e);
            if (!e) {
                result = read_8b(cpu->bus, addr, &e);
            }
        } break;

        case 2: {
            result = selector(s, c, d, 0);
        } break;

        case 3: {
            writeback = 0;
            addr = translate(cpu, selector(s, c, d, 3), QUAD, WRITE, &e);
            if (!e) {
                write_8b(cpu->bus, addr, get_reg_q(cpu, RA(inst)), &e);
            }
        } break;

        default: {
            error(cpu, EMLT, inst, 0);
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

void lcca64_ls_e(lcca_t *cpu, uint32_t inst) {
    uint64_t c = get_reg_q(cpu, RC(inst));
    uint64_t d = LS_DISP(inst);
    d = EXT15(d);

    lcca_error_t e = 0;
    uint64_t result;
    int writeback = 1;

    uint64_t addr;

    switch (FN(inst)) {
        case 0: {
            error(cpu, SVCT, inst, 0);
            return;
        }

        case 3: {
            if (cpu->c_regs[CR_PSQ] & CR_PSQ_PL) {
                error(cpu, IPLT, inst, 0);
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
            if (cpu->c_regs[CR_PSQ] & CR_PSQ_PL) {
                error(cpu, IPLT, inst, 0);
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
            if (cpu->c_regs[CR_PSQ] & CR_PSQ_PL) {
                error(cpu, IPLT, inst, 0);
                return;
            }

            else {
                result = c;
                intr_restore_disp(cpu, d << 2);
            }
        } break;

        case 6: {
            if (cpu->c_regs[CR_PSQ] & CR_PSQ_PL) {
                error(cpu, IPLT, inst, 0);
                return;
            }
            
            intr_internal(cpu, d & 63, inst, cpu->pc);
            return;
        }

        case 7: {
            if (cpu->c_regs[CR_PSQ] & CR_PSQ_PL) {
                error(cpu, IPLT, inst, 0);
                return;
            }

            simdbg_0(cpu, inst);
            return;
        }

        default: {
            error(cpu, EMLT, inst, 0);
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
    fprintf(stderr, "%%PC: %016lX \n", cpu->pc);
    for (int i = 0; i < 32; i += 2) {
        fprintf(stderr, 
            "%%%02d: %016lX %%%02d: %016lX \n",
            i, get_reg_q(cpu, i),
            i + 1, get_reg_q(cpu, i + 1)
        );
    }
}

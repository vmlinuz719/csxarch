#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "asm.h"

// asminst - Instruction generation

#define ERR_LGISL -2
#define ERR_NO_LABEL -3

struct instruction_def {
    char *mnemonic;
    int opcode;
    int fn;
    int size;
    uint64_t (*assemble) (struct input_ctx *ic, uint64_t pc, int opcode, int fn, int *err);
};

uint64_t asm_rr(struct input_ctx *ic, uint64_t pc, int opcode, int fn, int *err);

static struct instruction_def opcodes[] = {
    {"a",       0, 0, 1, asm_rr},
    {"s",       0, 1, 1, asm_rr},
    {"n",       0, 2, 1, asm_rr},
    {"o",       0, 3, 1, asm_rr},
    {"x",       0, 4, 1, asm_rr},
    {"sl",      0, 5, 1, asm_rr},
    {"asl",     0, 6, 1, asm_rr},
    {"setxc",   0, 7, 1, asm_rr},

    {"b",       1, 0, 1, NULL},
    {"bl",      1, 1, 1, NULL},
    {"blr",     1, 2, 1, NULL},
    {"j",       1, 3, 1, NULL},
    {"bez",     1, 4, 1, NULL},
    {"bnz",     1, 5, 1, NULL},
    {"bgz",     1, 6, 1, NULL},
    {"blez",    1, 7, 1, NULL},

    {"lc",      2, 0, 1, NULL},
    {"lcz",     2, 1, 1, NULL},
    {"lw",      2, 2, 1, NULL},
    {"lwz",     2, 3, 1, NULL},
    {"l",       2, 4, 1, NULL},
    {"stc",     2, 5, 1, NULL},
    {"stw",     2, 6, 1, NULL},
    {"st",      2, 7, 1, NULL},

    {"li",      3, 0, 1, NULL},
    {"lisl",    4, 0, 1, NULL},

    {"lz",      5, 0, 1, NULL},
    {"lq",      5, 1, 1, NULL},
    {"stq",     5, 2, 1, NULL},
    {"mfcr",    5, 3, 1, NULL},
    {"mtcr",    5, 4, 1, NULL},
    {"rex",     5, 5, 1, NULL},
    {"svc",     5, 5, 1, NULL},
    {"trap",    5, 6, 1, NULL},
    {"hvc",     5, 7, 1, NULL},
};

uint64_t asm_any(struct input_ctx *ic, uint64_t pc, char *mnemonic, int *err) {
    for (int i = 0; i < sizeof(opcodes) / sizeof(opcodes[0]); i++) {
        if (!strcmp(mnemonic, opcodes[i].mnemonic)) {
            return opcodes[i].assemble(ic, pc, opcodes[i].opcode, opcodes[i].fn, err);
        }
    }

    *err = -1;
    return 0;
}

uint64_t asm_rr(struct input_ctx *ic, uint64_t pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 2 && got_args != 3 && got_args != 4) {
        *err = -1;
        return 0;
    }

    int a = 0, b = 0, c = 0, d = 0;
    switch (got_args) {
        case 2: {
            a = get_register_literal(args[0], err); if (*err) return 0;
            b = a;
            c = get_register_literal(args[1], err); if (*err) return 0;
        } break;
        case 3: {
            a = get_register_literal(args[0], err); if (*err) return 0;
            b = get_register_literal(args[1], err); if (*err) return 0;
            c = get_register_literal(args[2], err); if (*err) return 0;
        } break;
        case 4: {
            a = get_register_literal(args[0], err); if (*err) return 0;
            b = get_register_literal(args[1], err); if (*err) return 0;
            c = get_register_literal(args[2], err); if (*err) return 0;
            d = get_number(args[3], 0x3FF, err); if (*err) return 0;
        } break;
    }

    result |= (a & 0x1F) << 23;
    result |= (b & 0x1F) << 5;
    result |= (c & 0x1F);
    result |= (d & 0x3FF) << 10;
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <in_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    struct input_ctx *in = open_input(argv[1]);
    if (in == NULL) {
        fprintf(stderr, "fatal: could not open %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    char event[MAX_EVENT_LEN];
    int len, err = 0;
    uint64_t pc = 0;
    while ((len = get_token(in->input, &in->line, &in->col, event, MAX_EVENT_LEN)) > 0) {
        if (event[len - 1] != ':') {
            pc += 4;
            printf("%08lX\n", asm_any(in, pc, event, &err));
            if (err == -1) {
                printf("Syntax error near %d:%d\n", in->line, in->col);
                break;
            } else if (err == ERR_LGISL) {
                pc += 4;
            }
        }
        else {
            event[len - 1] = '\0';
            struct label_def *r = register_label(in->ll, event, pc);
            if (r == NULL) {
                printf("Bad label near %d:%d\n", in->line, in->col);
            }
            else {
                printf("%16lX:%s\n", r->value, r->label);
            }
        }
    }

    close_input(in);
    return 0;
}
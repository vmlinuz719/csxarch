#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "asm.h"

// asminst - Instruction generation

#define INFO_LGISL -2
#define ERR_NO_LABEL -3

void emit(struct input_ctx *ic, uint64_t value, int size) {
    if (ic->output != NULL) {
        for (int i = 0; i < size; i++) {
            int shamt = ((size - 1) * 8) - (i * 8);
            char ch = (char)((value >> shamt) & 0xFF);
            fwrite(&ch, 1, 1, ic->output);
        }
    }
}

int is_power_2(uint64_t i) {
    uint64_t test = 1;
    for (int j = 0; j < 64; j++) {
        if (i == test << j) return 1;
    }
    return 0;
}

struct instruction_def {
    char *mnemonic;
    int opcode;
    int fn;
    int size;
    uint64_t (*assemble) (struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
};

struct label_cmd_def {
    char *mnemonic;
    uint64_t (*command) (uint64_t pc, uint64_t label);
};

uint64_t asm_rr(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_mv(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_id(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_br(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_br_j(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_ls_c(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_ls_w(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_ls_l(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_ls_q(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_lss_c(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_lss_w(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_lss_l(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_lss_q(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_ls_rex(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_bifn(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_im(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_lgisl(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_cr(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_tr(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_ret(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_srand(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asm_rand(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);

uint64_t define(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t align(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t asciz(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t ascii(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t origin(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);
uint64_t data(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err);

static struct instruction_def opcodes[] = {
    {"add",     0, 0, 4, asm_rr},
    {"mov",     0, 0, 4, asm_mv},
    {"inc",     0, 0, 4, asm_id},
    {"sub",     0, 1, 4, asm_rr},
    {"dec",     0, 1, 4, asm_id},
    {"and",     0, 2, 4, asm_rr},
    {"or",      0, 3, 4, asm_rr},
    {"xor",     0, 4, 4, asm_rr},
    {"shl",     0, 5, 4, asm_rr},
    {"sal",     0, 6, 4, asm_rr},
    {"setxc",   0, 7, 4, asm_rr},

    {"br",      1, 0, 4, asm_br},
    {"jmp",     1, 1, 4, asm_br_j},
    {"ret",     1, 1, 4, asm_ret},
    {"call",    1, 2, 4, asm_br},
    {"callf",   1, 3, 4, asm_br_j},
    {"bez",     1, 4, 4, asm_br},
    {"bnz",     1, 5, 4, asm_br},
    {"bgz",     1, 6, 4, asm_br},
    {"blez",    1, 7, 4, asm_br},

    {"ldc",     2, 0, 4, asm_ls_c},
    {"ldcz",    2, 1, 4, asm_ls_c},
    {"ldw",     2, 2, 4, asm_ls_w},
    {"ldwz",    2, 3, 4, asm_ls_w},
    {"ld",      2, 4, 4, asm_ls_l},
    {"stc",     2, 5, 4, asm_ls_c},
    {"stw",     2, 6, 4, asm_ls_w},
    {"st",      2, 7, 4, asm_ls_l},
    
    {"ldz",     3, 0, 4, asm_ls_l},
    {"ldq",     3, 1, 4, asm_ls_q},
    {"ldca",    3, 2, 4, asm_ls_c},
    {"stq",     3, 3, 4, asm_ls_q},
    
    {"extn",    3, 4, 4, asm_bifn},
    {"extnm",   3, 5, 4, asm_bifn},
    {"bifn",    3, 6, 4, asm_bifn},
    
    {"ldi",     4, 0, 4, asm_im},
    {"ldis",    5, 0, 4, asm_im},
    {"ldgis",   6, 0, 8, asm_lgisl},

    {"ldlin",   7, 0, 4, asm_ls_c},
    {"ldat",    7, 1, 4, asm_ls_c},
    {"ldtr",    7, 2, 4, asm_ls_c},
    {"invpg",   7, 3, 4, asm_invpg},
    {"mfva",    7, 4, 4, asm_ls_c},
    {"mtva",    7, 5, 4, asm_ls_c},
    {"mfpa",    7, 6, 4, asm_ls_c},
    {"mtpa",    7, 7, 4, asm_ls_c},
    
    {"ldcs",     10, 0, 4, asm_lss_c},
    {"ldczs",    10, 1, 4, asm_lss_c},
    {"ldws",     10, 2, 4, asm_lss_w},
    {"ldwzs",    10, 3, 4, asm_lss_w},
    {"lds",      10, 4, 4, asm_lss_l},
    {"stcs",     10, 5, 4, asm_lss_c},
    {"stws",     10, 6, 4, asm_lss_w},
    {"sts",      10, 7, 4, asm_lss_l},
    
    {"ldzs",     11, 0, 4, asm_lss_l},
    {"ldqs",     11, 1, 4, asm_lss_q},
    {"ldcas",    11, 2, 4, asm_lss_c},
    {"stqs",     11, 3, 4, asm_lss_q},

    {"svc",     14, 0, 4, asm_ls_c},
    {"srand",   14, 1, 4, asm_srand},
    {"rand",    14, 2, 4, asm_rand},
    {"mfcr",    14, 3, 4, asm_cr},
    {"mtcr",    14, 4, 4, asm_cr},
    {"rex",     14, 5, 4, asm_ls_rex},
    {"trap",    14, 6, 4, asm_tr},
    {"hvc",     14, 7, 4, asm_ls_c},

    {"define",  0,0,0, define},
    {"align",   0,0,0, align},
    {"asciz",   0,0,0, asciz},
    {"ascii",   0,0,0, ascii},
    {"origin",  0,0,0, origin},

    {"dc",   8,0,1, data},
    {"dw",   16,0,2, data},
    {"dl",   32,0,4, data},
    {"dq",   0,0,8, data},
};

uint64_t label_cmd_gh(uint64_t pc, uint64_t label) {
    return label >> 14;
}

uint64_t label_cmd_gl(uint64_t pc, uint64_t label) {
    return label & 0x3FFF;
}

uint64_t label_cmd_h(uint64_t pc, uint64_t label) {
    return label >> 9;
}

uint64_t label_cmd_l(uint64_t pc, uint64_t label) {
    return label & 0x1FF;
}

static struct label_cmd_def label_cmds[] = {
    {"gh",      label_cmd_gh},
    {"gl",      label_cmd_gl},
    {"h",       label_cmd_h },
    {"l",       label_cmd_l },
};

uint64_t label_cmd(struct input_ctx *ic, uint64_t pc, char *label, int *err) {
    char *label_name = label;
    char *label_cmd = NULL;
    
    int len = strlen(label);
    for (int i = 0; i < len; i++) {
        if (label[i] == '@') {
            label[i] = '\0';
            label_cmd = label_name + i + 1;
            break;
        }
    }
    
    uint64_t *label_ptr = search_label(ic->ll, label_name);
    if (label_ptr == NULL) {
        *err = ERR_NO_LABEL;
        return 0;
    }
    
    uint64_t label_value = *label_ptr;
    if (label_cmd == NULL) return label_value;
    
    for (int i = 0; i < sizeof(label_cmds) / sizeof(label_cmds[0]); i++) {
        if (!strcmp(label_cmd, label_cmds[i].mnemonic)) {
            return label_cmds[i].command(pc, label_value);
        }
    }
    
    uint64_t *rel_ptr = search_label(ic->ll, label_cmd);
    if (rel_ptr == NULL) {
        *err = -1;
        return 0;
    }

    uint64_t rel_value = label_value - *rel_ptr;
    return rel_value;
}

uint64_t label_or_num(struct input_ctx *ic, uint64_t pc, char *label, uint64_t n_max, int *is_label, int *err) {
    if (('0' <= label[0] && label[0] <= '9') || label[0] == '-') {
        if (is_label != NULL) *is_label = 0;
        return get_number(label, n_max, err);
    } else {
        if (is_label != NULL) *is_label = 1;
        return label_cmd(ic, pc, label, err);
    }
}

uint64_t asm_any(struct input_ctx *ic, uint64_t *pc, char *mnemonic, int *err) {
    for (int i = 0; i < sizeof(opcodes) / sizeof(opcodes[0]); i++) {
        if (!strcmp(mnemonic, opcodes[i].mnemonic)) {
            *pc += opcodes[i].size;
            uint64_t result = opcodes[i].assemble(ic, pc, opcodes[i].opcode, opcodes[i].fn, err);
            emit(ic, result, opcodes[i].size);
            return result;
        }
    }

    *err = -1;
    return 0;
}

uint64_t define(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 2) {
        *err = -1;
        return 0;
    }

    uint64_t d = label_or_num(ic, *pc, args[1], 0xFFFFFFFFFFFFFFFF, NULL, err); if (*err) return 0;
    struct label_def *r = register_label(ic->ll, args[0], d);

    return r == NULL ? -1 : 0;
}

uint64_t align(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1) {
        *err = -1;
        return 0;
    }

    uint64_t d = label_or_num(ic, *pc, args[0], 0xFFFFFFFFFFFFFFFF, NULL, err); if (*err) return 0;

    if (!is_power_2(d)) { *err = -1; return 0; }

    if (*pc % d) {
        uint64_t new_pc = (*pc + d) & (~(d - 1));
        if (ic->output != NULL) {
            for (uint64_t i = 0; i < (new_pc - *pc); i++) {
                // TODO: escape sequences with deduplicated handling
                emit(ic, 0, 1);
            }
        }
        *pc = new_pc;
    }

    return 0;
}

uint64_t asciz(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1) {
        *err = -1;
        return 0;
    }

    if (args[0][0] != '"') {
        *err = -1;
        return 0;
    }

    int len = strlen(args[0]);
    if (len <= 2) return 0;

    args[0][len - 1] = '\0';
    len -= 2;

    char *str = (args[0]) + 1;

    for (int i = 0; i <= len; i++) {
        // TODO: escape sequences with deduplicated handling
        emit(ic, str[i], 1);
    }

    *pc += len + 1;

    return 0;
}

uint64_t ascii(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1) {
        *err = -1;
        return 0;
    }

    if (args[0][0] != '"') {
        *err = -1;
        return 0;
    }

    int len = strlen(args[0]);
    if (len <= 2) return 0;

    args[0][len - 1] = '\0';
    len -= 2;

    char *str = (args[0]) + 1;

    for (int i = 0; i < len; i++) {
        emit(ic, str[i], 1);
    }

    *pc += len;

    return 0;
}

uint64_t data(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1) {
        *err = -1;
        return 0;
    }

    uint64_t max = (opcode) ? (1L << opcode) - 1 : 0xFFFFFFFFFFFFFFFF;

    uint64_t d = label_or_num(ic, *pc, args[0], max, NULL, err); if (*err) return 0;

    return d;
}

uint64_t origin(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1) {
        *err = -1;
        return 0;
    }

    uint64_t max = (opcode) ? (1L << opcode) - 1 : 0xFFFFFFFFFFFFFFFF;

    uint64_t d = label_or_num(ic, *pc, args[0], max, NULL, err); if (*err) return 0;
    *pc = d;

    return 0;
}

uint64_t asm_rr(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
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
            d = label_or_num(ic, *pc, args[3], 0x3FF, NULL, err); if (*err) return 0;
        } break;
    }

    result |= (a & 0x1F) << 23;
    result |= (b & 0x1F) << 5;
    result |= (c & 0x1F);
    result |= (d & 0x3FF) << 10;
    return result;
}

uint64_t asm_mv(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 2) {
        *err = -1;
        return 0;
    }

    int a = 0, b = 0;
    
    a = get_register_literal(args[0], err); if (*err) return 0;
    b = get_register_literal(args[1], err); if (*err) return 0;

    result |= (a & 0x1F) << 23;
    result |= (b & 0x1F) << 5;
    return result;
}

uint64_t asm_srand(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1) {
        *err = -1;
        return 0;
    }

    int c = 0;
    
    c = get_register_literal(args[0], err); if (*err) return 0;

    result |= (c & 0x1F);
    return result;
}

uint64_t asm_rand(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1) {
        *err = -1;
        return 0;
    }

    int a = 0;
    
    a = get_register_literal(args[0], err); if (*err) return 0;

    result |= (a & 0x1F) << 23;
    return result;
}

uint64_t asm_id(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1) {
        *err = -1;
        return 0;
    }

    int a = 0, b = 0, d = 1;
    
    a = get_register_literal(args[0], err); if (*err) return 0;
    b = a;

    result |= (a & 0x1F) << 23;
    result |= (b & 0x1F) << 5;
    result |= (d & 0x3FF) << 10;
    return result;
}

uint64_t asm_br(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1 && got_args != 2) {
        *err = -1;
        return 0;
    }

    int a = 0, d = 0;

    int i = 0;

    if (got_args == 2) {
        a = get_register_literal(args[i++], err);
        if (*err) return 0;
    }
    
    
    int is_label;
    d = label_or_num(ic, *pc, args[i], 0x3FFFFF, &is_label, err); if (*err) return 0;
    if (is_label) d -= *pc;

    result |= (a & 0x1F) << 23;
    result |= (d >> 2) & 0xFFFFF;
    return result;
}

uint64_t asm_br_j(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1 && got_args != 2) {
        *err = -1;
        return 0;
    }

    int a = 0, d = 0;

    int i = 0;

    if (got_args == 2) {
        d = label_or_num(ic, *pc, args[1], 0x3FFFFF, NULL, err); if (*err) return 0;
    }

    a = get_register_literal(args[0], err); if (*err) return 0;

    result |= (a & 0x1F) << 23;
    result |= (d >> 2) & 0xFFFFF;
    return result;
}

uint64_t asm_invpg(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1 && got_args != 2) {
        *err = -1;
        return 0;
    }

    int c = 0, d = 0;

    int i = 0;

    if (got_args == 2) {
        d = label_or_num(ic, *pc, args[1], 0x3FF, NULL, err); if (*err) return 0;
    }

    c = get_register_literal(args[0], err); if (*err) return 0;

    result |= (d & 0x3FF) << 5;
    result |= (c & 0x1F);
    return result;
}

uint64_t asm_im(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 2) {
        *err = -1;
        return 0;
    }

    int a = 0, c = 0, d = 0;

    a = get_register_literal(args[0], err); if (*err) return 0;

    d = label_or_num(ic, *pc, args[1], 0x7FFFFF, NULL, err); if (*err) return 0;

    result |= (a & 0x1F) << 23;
    result |= (d & 0x7FFFFF);
    return result;
}

uint64_t asm_tr(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 1) {
        *err = -1;
        return 0;
    }

    int d = 0;

    d = label_or_num(ic, *pc, args[0], 0x7FFF, NULL, err); if (*err) return 0;

    result |= (d & 0x7FFF) << 5;
    return result;
}

uint64_t asm_ret(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);
    int a = 31;
    result |= (a & 0x1F) << 23;
    return result;
}

uint64_t asm_lgisl(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint64_t result = ((uint64_t) opcode << 60);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 2) {
        *err = -1;
        return 0;
    }

    int a = 0, c = 0;
    uint64_t d = 0;

    a = get_register_literal(args[0], err); if (*err) return 0;

    d = label_or_num(ic, *pc, args[1], 0x3FFFFFFFFFFFF, NULL, err); if (*err) return 0;
    d &= 0x3FFFFFFFFFFFF;

    result |= (uint64_t) (a & 0x1F) << 55;
    result |= (a & 0x1F) << 23;
    result |= (d & 0x7FFFFF);
    result |= (d & ~(0x7FFFFF)) << 5;
    return result;
}

uint64_t asm_ls(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err, int shamt) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 2 && got_args != 3) {
        *err = -1;
        return 0;
    }

    int a = 0, c = 0, d = 0;

    a = get_register_literal(args[0], err); if (*err) return 0;
    c = get_register_literal(args[1], err); if (*err) return 0;
    
    if (got_args == 3) {
        d = label_or_num(ic, *pc, args[2], 0x7FFF, NULL, err); if (*err) return 0;
    }

    result |= (a & 0x1F) << 23;
    result |= ((d >> shamt) & 0x7FFF) << 5;
    result |= (c & 0x1F);
    return result;
}

uint64_t asm_lss(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err, int shamt) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 3 && got_args != 4) {
        *err = -1;
        return 0;
    }

    int a = 0, s = 0, c = 0, d = 0;

    a = get_register_literal(args[0], err); if (*err) return 0;
    s = get_register_literal(args[1], err); if (*err) return 0;
    c = get_register_literal(args[2], err); if (*err) return 0;
    
    if (got_args == 4) {
        d = label_or_num(ic, *pc, args[3], 0xFFF, NULL, err); if (*err) return 0;
    }

    result |= (a & 0x1F) << 23;
    result |= (s & 0x7) << 17;
    result |= ((d >> shamt) & 0xFFF) << 5;
    result |= (c & 0x1F);
    return result;
}

uint64_t asm_ls_rex(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args > 3) {
        *err = -1;
        return 0;
    }

    int a = 0, c = 0, d = 0;

    if (got_args == 2 || got_args == 3) {
        a = get_register_literal(args[0], err); if (*err) return 0;
        c = get_register_literal(args[1], err); if (*err) return 0;
    }
    
    if (got_args == 1 || got_args == 3) {
        d = label_or_num(ic, *pc, args[got_args - 1], 0x7FFF, NULL, err); if (*err) return 0;
    }

    result |= (a & 0x1F) << 23;
    result |= (d & 0x7FFF) << 5;
    result |= (c & 0x1F);
    return result;
}

uint64_t asm_cr(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 2 && got_args != 3) {
        *err = -1;
        return 0;
    }

    int a = 0, c = 0, d = 0, i = 1;

    a = get_register_literal(args[0], err); if (*err) return 0;
    
    if (got_args == 3) {
        c = get_register_literal(args[i++], err); if (*err) return 0;
    }
    
    d = get_control_register_literal(args[i], err); if (*err) return 0;

    result |= (a & 0x1F) << 23;
    result |= (d & 0x7FFF) << 5;
    result |= (c & 0x1F);
    return result;
}

uint64_t asm_ls_c(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    return asm_ls(ic, pc, opcode, fn, err, 0);
}

uint64_t asm_ls_w(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    return asm_ls(ic, pc, opcode, fn, err, 1);
}

uint64_t asm_ls_l(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    return asm_ls(ic, pc, opcode, fn, err, 2);
}

uint64_t asm_ls_q(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    return asm_ls(ic, pc, opcode, fn, err, 3);
}

uint64_t asm_lss_c(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    return asm_lss(ic, pc, opcode, fn, err, 0);
}

uint64_t asm_lss_w(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    return asm_lss(ic, pc, opcode, fn, err, 1);
}

uint64_t asm_lss_l(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    return asm_lss(ic, pc, opcode, fn, err, 2);
}

uint64_t asm_lss_q(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    return asm_lss(ic, pc, opcode, fn, err, 3);
}

uint64_t asm_bifn(struct input_ctx *ic, uint64_t *pc, int opcode, int fn, int *err) {
    uint32_t result = (opcode << 28) | (fn << 20);

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int got_args = get_args(ic->input, &ic->line, &ic->col, args, MAX_ARGS, event, MAX_EVENT_LEN);
    if (got_args != 4) {
        *err = -1;
        return 0;
    }

    int a = 0, c = 0, shamt = 0, bit = 0;

    a = get_register_literal(args[0], err); if (*err) return 0;
    shamt = get_number(args[1], 0x3F, err); if (*err) return 0;
    bit = get_number(args[2], 0x3F, err); if (*err) return 0;
    c = get_register_literal(args[3], err); if (*err) return 0;
    
    int d = bit | (shamt << 6);

    result |= (a & 0x1F) << 23;
    result |= (d & 0xFFF) << 5;
    result |= (c & 0x1F);
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <in_file> <out_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    struct input_ctx *in = open_input(argv[1]);
    if (in == NULL) {
        fprintf(stderr, "fatal: could not open %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    char event[MAX_EVENT_LEN];
    int len, err = 0;
    uint64_t pc = 0x10000;
    while ((len = get_token(in->input, &in->line, &in->col, event, MAX_EVENT_LEN)) > 0) {
        if (event[len - 1] != ':') {
            asm_any(in, &pc, event, &err);
            if (err == -1) {
                printf("Syntax error near %d:%d\n", in->line, in->col);
                break;
            }
        }
        else {
            event[len - 1] = '\0';
            struct label_def *r = register_label(in->ll, event, pc);
            if (r == NULL) {
                printf("Bad label near %d:%d\n", in->line, in->col);
                break;
            }
            else {
                printf("%16lX:%s\n", r->value, r->label);
            }
        }
    }

    if (err == -1) {
        close_input(in);
        exit(EXIT_FAILURE);
    }

    if (open_output(in, argv[2])) {
        fprintf(stderr, "fatal: could not open %s\n", argv[1]);
        close_input(in);
        exit(EXIT_FAILURE);
    }
    
    rewind(in->input);
    pc = 0x10000;
    err = 0;
    in->line = 0;
    in->col = 0;

    while ((len = get_token(in->input, &in->line, &in->col, event, MAX_EVENT_LEN)) > 0) {
        if (event[len - 1] != ':') {
            asm_any(in, &pc, event, &err);
            if (err == -1) {
                printf("Syntax error near %d:%d\n", in->line, in->col);
                break;
            }
            else if (err == ERR_NO_LABEL) {
                printf("Unresolved label near %d:%d\n", in->line, in->col);
                break;
            }
        }
    }

    close_input(in);
    return 0;
}
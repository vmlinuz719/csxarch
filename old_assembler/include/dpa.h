// Dumb Potato Assembler

#ifndef _DPA_H_
#define _DPA_H_

#include <stdio.h>
#include <stdint.h>

/* Constants */

#define MAX_EVENT_LEN 256
#define MAX_LABELS 256
#define MAX_LABEL_LEN 16
#define MAX_OPCODE_LEN 12

#define ERROR_EOF -1
#define ERROR_EVENT_TOO_LONG -2
#define ERROR_LABEL_TOO_LONG -3
#define ERROR_LABELS_FULL -4
#define ERROR_BAD_OPCODE -5
#define ERROR_BAD_REG_LITERAL -6
#define ERROR_BAD_NUM_LITERAL -7
#define ERROR_BAD_STR_LITERAL -8
#define ERROR_BAD_LABEL_REF -9
#define ERROR_NOT_ALLOWED -10
#define EXPECTED_EOF 1

/* Types */

struct parser_ctx {
    FILE *in;
    FILE *out;

    uint64_t current_pc;

    char event_buf[MAX_EVENT_LEN];

    char labels[MAX_LABELS][MAX_LABEL_LEN];
    uint64_t label_pcs[MAX_LABELS];
    int num_labels;
};

struct opcode_def {
    char mnemonic[MAX_OPCODE_LEN];
    int opcode;
    int (*assemble)(struct parser_ctx *, int, int);
};

#endif
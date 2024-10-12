#ifndef _PARSEVT_H_
#define _PARSEVT_H_

#include <stdint.h>
#include "dpa.h"

int lookahead(struct parser_ctx *ctx, int n);

int read_reg_literal(struct parser_ctx *ctx);

int read_c_reg_literal(struct parser_ctx *ctx);

uint64_t label_cmd(
    struct parser_ctx *ctx,
    uint64_t raw,
    char *cmd,
    int *status
);

uint64_t read_num_literal_or_label_ref(
    struct parser_ctx *ctx,
    int allowed_types,
    uint64_t max,
    int *status,
    int pass
);

int read_address(
    struct parser_ctx *ctx,
    uint64_t *i1,
    uint64_t i1_max,
    int *b1,
    int *x1,
    int pass
);

#endif
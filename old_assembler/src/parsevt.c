#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "dpa.h"
#include "eventio.h"
#include "assemble.h"

const char *regs[] = {
    "%0" , "%Z",    // Zero

    "%1" , "%CT",   // Constant Table
    "%2" , "%SP",   // Stack Pointer
    "%3" , "%FP",   // Frame Pointer
    "%4" , "%LR",   // Link Register

    "%5" , "%A" ,   // Accumulator 1
    "%6" , "%X" ,   // Index 1

    "%7" , "%A2",   // Accumulator 2
    "%8" , "%A3",   // Accumulator 3
    "%9" , "%A4",   // Accumulator 4
    "%10", "%A5",   // Accumulator 5
    "%11", "%A6",   // Accumulator 6
    "%12", "%A7",   // Accumulator 7

    "%13", "%X2",   // Index 2
    "%14", "%X3",   // Index 3
    "%15", "%X4",   // Index 4
};

const char *c_regs[] = {
    "%0" , "%PSW",  // Processor Status Word
    "%1" , "%SR",  // Supervisor Return
    "%2" , "%SRPSW",  // Supervisor Return PSW
    "%3" , "%EADDR", // Exception Address
    "%4" , "%ETYPE", // Exception Type
    "%5" , "%TIMER", // Interrupt Timer
};

int lookahead(struct parser_ctx *ctx, int n) {
    long int oldpos = ftell(ctx->in);
    
    for (int i = 0; i < n; i++) {
        if (consume_whitespace(ctx->in) == -1) return ERROR_EOF;
        if (read_event_content(ctx->in, ctx->event_buf, MAX_EVENT_LEN) == -1)
            return ERROR_EVENT_TOO_LONG;
    }
    
    fseek(ctx->in, oldpos, SEEK_SET);
    
    return 0;
}

int read_reg_literal(struct parser_ctx *ctx) {
    if (consume_whitespace(ctx->in) == -1) return ERROR_EOF;
    if (read_event_content(ctx->in, ctx->event_buf, MAX_EVENT_LEN) == -1)
        return ERROR_EVENT_TOO_LONG;

    int type = get_event_type(ctx->event_buf);

    if (type != EVT_REG_LITERAL) return ERROR_BAD_REG_LITERAL;

    for (int i = 0; i < sizeof(regs) / sizeof(regs[0]); i++) {
        if (!strcmp(ctx->event_buf, regs[i])) return i >> 1;
    }

    return ERROR_BAD_REG_LITERAL;
}

int read_c_reg_literal(struct parser_ctx *ctx) {
    if (consume_whitespace(ctx->in) == -1) return ERROR_EOF;
    if (read_event_content(ctx->in, ctx->event_buf, MAX_EVENT_LEN) == -1)
        return ERROR_EVENT_TOO_LONG;

    int type = get_event_type(ctx->event_buf);

    if (type != EVT_REG_LITERAL) return ERROR_BAD_REG_LITERAL;

    for (int i = 0; i < sizeof(c_regs) / sizeof(c_regs[0]); i++) {
        if (!strcmp(ctx->event_buf, c_regs[i])) return i >> 1;
    }

    return ERROR_BAD_REG_LITERAL;
}

uint64_t label_cmd(
    struct parser_ctx *ctx,
    uint64_t raw,
    char *cmd,
    int *status
) {
    *status = 0;
    
    if (*cmd == '\0') return raw;
    
    else if (!strcmp(cmd, "@REL")) return raw - ctx->current_pc;
    
    else if (!strcmp(cmd, "@BR")) return (raw - ctx->current_pc) >> 1;
    
    else if (!strcmp(cmd, "@H")) return (raw & 0xFFFFFFFF00000000) >> 32;
    
    else if (!strcmp(cmd, "@HH")) return (raw & 0xFFFF000000000000) >> 48;
    
    else {
        *status = ERROR_BAD_LABEL_REF;
        return 0;
    }
}

uint64_t read_num_literal_or_label_ref(
    struct parser_ctx *ctx,
    int allowed_types,
    uint64_t max,
    int *status,
    int pass
) {
    if (consume_whitespace(ctx->in) == -1) {
        *status = ERROR_EOF;
        return 0;
    }
    if (read_event_content(ctx->in, ctx->event_buf, MAX_EVENT_LEN) == -1) {
        *status = ERROR_EVENT_TOO_LONG;
        return 0;
    }
    
    int type = get_event_type(ctx->event_buf);
    
    if (type & ~allowed_types) {
        *status = ERROR_NOT_ALLOWED;
        return 0;
    }
    
    int paren_index = strlen(ctx->event_buf) - 1;
    
    if (type & EVT_BEGIN_ADDRESS) {
        if (paren_index == 0) {
            *status = 0;
            return 0;
        }
        ctx->event_buf[paren_index] = '\0';
    }
    
    if (type & EVT_LABEL_REF) {
        // ctx->event_buf[strlen(ctx->event_buf) - 1] = '\0';
        
        if (pass) {
            int cmdi = 1;
            while (ctx->event_buf[cmdi] && ctx->event_buf[cmdi] != '@')
                cmdi++;
            
            for (int i = 0; i < ctx->num_labels; i++) {
                if (!strncmp(ctx->event_buf + 1, ctx->labels[i], cmdi - 1)) {
                    uint64_t result = label_cmd(
                        ctx, ctx->label_pcs[i],
                        ctx->event_buf + cmdi,
                        status
                    );
                    if (type & EVT_BEGIN_ADDRESS) {
                        ctx->event_buf[paren_index] = '(';
                    }
                    return result;
                }
            }
            
            *status = ERROR_BAD_LABEL_REF;
            return 0;
        } else {
            *status = 0;
            if (type & EVT_BEGIN_ADDRESS) {
                ctx->event_buf[paren_index] = '(';
            }
            return 0xDEADBEEF;
        }
    }
    
    else if (type & EVT_NUM_LITERAL) {
        int base = (ctx->event_buf[0] == '#') ? 16 : 10;
        char *endptr = NULL;
        
        uint64_t result = (base == 16) ?
            strtoul(ctx->event_buf + 1, &endptr, base) :
            (uint64_t) strtol(ctx->event_buf + 1, &endptr, base);
        
        if (errno == ERANGE || *endptr != '\0') {
            errno = 0;
            *status = ERROR_BAD_NUM_LITERAL;
            return 0;
        }
        
        int64_t s_min = -((int64_t) ((max >> 1) + 1));
        if (((int64_t) result >= 0 && result > max)
            || (int64_t) result < s_min) {
            *status = ERROR_BAD_NUM_LITERAL;
            return 0;
        }
        
        *status = 0;
        if (type & EVT_BEGIN_ADDRESS) ctx->event_buf[paren_index] = '(';
        return result;
    }
    
    *status = ERROR_NOT_ALLOWED;
    return 0;
}

int read_address(
    struct parser_ctx *ctx,
    uint64_t *i1,
    uint64_t i1_max,
    int *b1,
    int *x1,
    int pass
) {
    *b1 = -1;
    *x1 = -1;
    
    int literal_status = 0;
    *i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF | EVT_BEGIN_ADDRESS,
        i1_max,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;
    
    int type = get_event_type(ctx->event_buf);
    if (!(type & EVT_BEGIN_ADDRESS)) return 0;
    
    *b1 = read_reg_literal(ctx); if (*b1 < 0) return *b1;
    
    *x1 = read_reg_literal(ctx);
    
    if (*x1 < 0) {
        type = get_event_type(ctx->event_buf);
        if (type & EVT_END_ADDRESS) return 0;
        else return *x1;
    }
    
    read_reg_literal(ctx);
    type = get_event_type(ctx->event_buf);
    if (!(type & EVT_END_ADDRESS)) return ERROR_NOT_ALLOWED;
    else return 0;
}

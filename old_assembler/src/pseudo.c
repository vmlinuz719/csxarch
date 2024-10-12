#include <stdint.h>
#include <stdio.h>

#include "dpa.h"
#include "eventio.h"
#include "parsevt.h"

int mark(struct parser_ctx *ctx, int opcode, int pass) {
    int literal_status = 0;
    uint64_t i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL,
        0xFFFFFFFFFFFFFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;
    
    ctx->current_pc = i1;

#ifdef TEST_PARSEVT
    printf("Debug: PS MARK %lx\n", i1);
#endif

    return 0;
}

int dchar(struct parser_ctx *ctx, int opcode, int pass) {
    int literal_status = 0;
    uint64_t i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;
    
    if (pass) {
        fputc(i1 & 0xFF, ctx->out);
    }
    
    ctx->current_pc++;
    
#ifdef TEST_PARSEVT
    printf("Debug: PS DCHAR %hhx\n", (uint8_t) i1);
#endif

    return 0;
}

int dwide(struct parser_ctx *ctx, int opcode, int pass) {
    int literal_status = 0;
    uint64_t i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;
    
    if (pass) {
        fputc((i1 & 0xFF00) >> 8, ctx->out);
        fputc(i1 & 0xFF, ctx->out);
    }
    
    ctx->current_pc += 2;
    
#ifdef TEST_PARSEVT
    printf("Debug: PS DWIDE %hx\n", (uint16_t) i1);
#endif

    return 0;
}

int dhalf(struct parser_ctx *ctx, int opcode, int pass) {
    int literal_status = 0;
    uint64_t i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFFFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;
    
    if (pass) {
        fputc((i1 & 0xFF000000) >> 24, ctx->out);
        fputc((i1 & 0xFF0000) >> 16, ctx->out);
        fputc((i1 & 0xFF00) >> 8, ctx->out);
        fputc(i1 & 0xFF, ctx->out);
    }
    
    ctx->current_pc += 4;
    
#ifdef TEST_PARSEVT
    printf("Debug: PS DHALF %x\n", (uint32_t) i1);
#endif

    return 0;
}

int dword(struct parser_ctx *ctx, int opcode, int pass) {
    int literal_status = 0;
    uint64_t i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFFFFFFFFFFFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;
    
    if (pass) {
        fputc((i1 & 0xFF00000000000000) >> 56, ctx->out);
        fputc((i1 & 0xFF000000000000) >> 48, ctx->out);
        fputc((i1 & 0xFF0000000000) >> 40, ctx->out);
        fputc((i1 & 0xFF00000000) >> 32, ctx->out);
        fputc((i1 & 0xFF000000) >> 24, ctx->out);
        fputc((i1 & 0xFF0000) >> 16, ctx->out);
        fputc((i1 & 0xFF00) >> 8, ctx->out);
        fputc(i1 & 0xFF, ctx->out);
    }
    
    ctx->current_pc += 8;
    
#ifdef TEST_PARSEVT
    printf("Debug: PS DWORD %lx\n", (uint64_t) i1);
#endif

    return 0;
}

int dcstr(struct parser_ctx *ctx, int opcode, int pass) {
    if (consume_whitespace(ctx->in) == -1) return ERROR_EOF;
    if (read_event_content(ctx->in, ctx->event_buf, MAX_EVENT_LEN) == -1)
        return ERROR_EVENT_TOO_LONG;

    int type = get_event_type(ctx->event_buf);

    if (type != EVT_STR_LITERAL) return ERROR_BAD_STR_LITERAL;
    
#ifdef TEST_PARSEVT
    printf("Debug: PS DCSTR ");
#endif
    
    char *str = ctx->event_buf + 1;
    while (*str != '"') {
        if (pass) {
            fputc(*str, ctx->out);
        }
        
#ifdef TEST_PARSEVT
        printf("%c", *str);
#endif
        
        ctx->current_pc++;
        str++;
    }
    
#ifdef TEST_PARSEVT
    printf("\n");
#endif
    
    return 0;
}

int align(struct parser_ctx *ctx, int opcode, int pass) {
    int literal_status = 0;
    uint64_t i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFFFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;
    
    while (ctx->current_pc % i1) {
        if (pass) {
            fputc(0, ctx->out);
        }
    
        ctx->current_pc++;
    }
    
#ifdef TEST_PARSEVT
    printf("Debug: PS ALIGN %x\n", (uint32_t) i1);
#endif

    return 0;
}

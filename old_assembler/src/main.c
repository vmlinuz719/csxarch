#include <stdio.h>
#include <stdlib.h>
#include "dpa.h"
#include "assemble.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "usage: %s <in_file> <out_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    struct parser_ctx ctx;
    ctx.num_labels = 0;
    ctx.current_pc = 0;
    
    ctx.in = fopen(argv[1], "r");
    if (ctx.in == NULL) {
        fprintf(stderr, "fatal: could not open %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    ctx.out = fopen(argv[2], "wb");
    if (ctx.out == NULL) {
        fprintf(stderr, "fatal: could not open %s\n", argv[1]);
        fclose(ctx.in);
        exit(EXIT_FAILURE);
    }

    int result = 0;

    while (!(result = read_new_label_or_opcode(&ctx, 0)));
    
    if (result == EXPECTED_EOF) {
        printf("Pass 0 (label registration) succeeded\n");
        for (int i = 0; i < ctx.num_labels; i++)
            printf("%16lx: %s\n", ctx.label_pcs[i], ctx.labels[i]);
    } else {
        printf("Pass 0 (label registration) FAILED (error %d)\n", result);
        fclose(ctx.in);
        fclose(ctx.out);
        exit(EXIT_FAILURE);
    }
    
    rewind(ctx.in);
    ctx.current_pc = 0;
    while (!(result = read_new_label_or_opcode(&ctx, 1)));
    
    if (result == EXPECTED_EOF) {
        printf("Pass 1 (code generation) succeeded\n");
    } else {
        printf("Pass 1 (code generation) FAILED (error %d)\n", result);
        fclose(ctx.in);
        fclose(ctx.out);
        exit(EXIT_FAILURE);
    }
    
    fclose(ctx.in);
    fclose(ctx.out);
    
    return 0;
}

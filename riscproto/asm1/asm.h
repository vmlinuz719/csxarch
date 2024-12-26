#ifndef _ASM_H_
#define _ASM_H_

#define MAX_EVENT_LEN 256
#define MAX_ARGS 32
#define MAX_LABEL_LEN 16

#include <stdio.h>
#include <stdint.h>

uint64_t get_number(
    char *input,
    uint64_t max,
    int *err
);

int get_args(FILE *f, int *line, int *col,
    char **results, int maxargs, char *buf, int buflen);

int get_token(FILE *f, int *line, int *col, char *result, int maxlen);

int consume_whitespace(FILE *f, int *line, int *col);

#endif
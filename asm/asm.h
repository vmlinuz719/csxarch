#ifndef _ASM_H_
#define _ASM_H_

#define MAX_EVENT_LEN 256
#define MAX_ARGS 32
#define MAX_LABEL_LEN 16

#include <stdio.h>
#include <stdint.h>

struct input_ctx {
    FILE *input, *output;
    int line, col;
    struct label_list *ll;
};

int get_args(FILE *f, int *line, int *col,
    char **results, int maxargs, char *buf, int buflen);
int get_token(FILE *f, int *line, int *col, char *result, int maxlen);
int consume_whitespace(FILE *f, int *line, int *col);
struct input_ctx *open_input(char *fname);
int open_output(struct input_ctx *ic, char *fname);
void close_input(struct input_ctx *ic);

uint64_t get_number(
    char *input,
    uint64_t max,
    int *err
);
uint64_t get_register_literal(
    char *input,
    int *err
);
uint64_t get_control_register_literal(
    char *input,
    int *err
);

struct label_def {
    char label[MAX_LABEL_LEN + 1];
    uint64_t value;
};

struct label_list {
    int n_labels, current_max_labels;
    struct label_def *labels;
};

struct label_list *init_label_list();
void destroy_label_list(struct label_list *ll);
uint64_t *search_label(struct label_list *ll, char *label);
struct label_def *register_label(
    struct label_list *ll,
    char *label,
    uint64_t value
);

#endif
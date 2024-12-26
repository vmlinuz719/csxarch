#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"

// asmfile - File I/O

int consume_whitespace(FILE *f, int *line, int *col) {
    int num_consumed = 0;
    int ch = 0;
    int comment = 0;

    while ((ch = fgetc(f)) != EOF) {
        if (ch == '\r' || ch == '\n' || ch == '\t' || ch == ' '// || ch == ','
            || ch == ';' || comment) {
            *col += 1;
            num_consumed++;
            if (ch == ';') comment = 1;
            if (ch == '\n') {
                comment = 0;
                *col = 0;
                *line += 1;
            }
        } else {
            fseek(f, -1, SEEK_CUR);
            return num_consumed;
        }
    }

    return -1;
}

int get_token(FILE *f, int *line, int *col, char *result, int maxlen) {
    int ch = 0, i = 0;
    int quoted = 0;
    
    if (consume_whitespace(f, line, col) == -1) return 0;

    while (i < maxlen - 2 && (ch = fgetc(f)) != EOF) {
        if (ch == '"') quoted ^= 1;
        if (!quoted && (ch == '\r' ||
                        ch == '\n' ||
                        ch == '\t' ||
                        ch == ' '  ||
                        ch == ';'  ||
                        ch == ','  ||
                        ch == ':'
        )) {
            if (ch == ',' || ch == ':') {
                *col += 1;
                result[i++] = ch;
            } else {
                fseek(f, -1, SEEK_CUR);
            }
            result[i] = '\0';
            return i;
        } else if (quoted && ch == '\n') {
            *col = 0;
            *line += 1;
        } else {
            *col += 1;
        }

        result[i++] = ch;
    }

    if (i != 0 && i < maxlen - 1) {
        result[i] = '\0';
        return i;
    }
    
    return -1;
}

int get_args(FILE *f, int *line, int *col,
    char **results, int maxargs, char *buf, int buflen) {
    char event[MAX_EVENT_LEN];
    int buf_index = 0;
    int comma = 1;
    int got_args = 0;
    
    while (comma && got_args < maxargs) {
        int len = get_token(f, line, col, event, MAX_EVENT_LEN);
        if (len == -1 || len == 0
            || buf_index + len + 1 >= MAX_EVENT_LEN) return -1;
        
        if (event[len - 1] == ',') event[len - 1] = '\0';
        else comma = 0;
        
        strcpy(buf + buf_index, event);
        results[got_args++] = buf + buf_index; buf_index += len;
    }
    
    return got_args;
}

struct input_ctx *open_input(char *fname) {
    struct input_ctx *ic = malloc(sizeof(struct input_ctx));
    ic->input = fopen(fname, "r");
    if (ic->input == NULL) {
        free(ic);
        return NULL;
    }
    ic->ll = init_label_list();
    ic->line = 0;
    ic->col = 0;
    return ic;
}

void close_input(struct input_ctx *ic) {
    fclose(ic->input);
    destroy_label_list(ic->ll);
    free(ic);
}
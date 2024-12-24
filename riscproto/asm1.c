#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EVENT_LEN 256
#define MAX_ARGS 32

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
    
    if (consume_whitespace(f, line, col) == -1) return 0;

    while (i < maxlen - 2 && (ch = fgetc(f)) != EOF) {
        if (ch == '\r' || ch == '\n' || ch == '\t' || ch == ' ' || ch == ';'
            || ch == ',' || ch == ':') {
            if (ch == ',' || ch == ':') {
                *col += 1;
                result[i++] = ch;
            } else {
                fseek(f, -1, SEEK_CUR);
            }
            result[i] = '\0';
            return i;
        } else {
            *col += 1;
        }

        result[i++] = ch;
    }

    if (i) {
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <in_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    FILE *in = fopen(argv[1], "r");
    if (in == NULL) {
        fprintf(stderr, "fatal: could not open %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    char event[MAX_EVENT_LEN];
    char *args[MAX_ARGS];
    int line = 0, col = 0;
    
    int len;
    while ((len = get_token(in, &line, &col, event, MAX_EVENT_LEN))) {
        printf("%8d:%-8d^%s$ ", line, col, event);
        
        if (event[len - 1] != ':' && event[len - 1] != ',') {
            int got_args = get_args(in, &line, &col, args, MAX_ARGS, event, MAX_EVENT_LEN);
            for (int i = 0; i < got_args; i++) {
                printf("arg %d: ^%s$ ", i, args[i]);
            }
        }
        
        printf("\n");
    }
    
    return 0;
}

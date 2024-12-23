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
            result[i++] = '\0';
            return i - 1;
        } else {
            *col += 1;
        }

        result[i++] = ch;
    }

    return -1;
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
    int line = 0, col = 0;
    int len;
    while ((len = get_token(in, &line, &col, event, MAX_EVENT_LEN))) {
        printf("%8d:%-8d^%s$\n", line, col, event);
    }

    return 0;
}
// label: instruction a,b,c,d
// label2: instruction2 a, b, c, d ; some comments
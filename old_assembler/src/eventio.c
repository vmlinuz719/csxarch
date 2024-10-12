#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "dpa.h"
#include "eventio.h"

/* I/O routines needed by the assembler */

int consume_whitespace(FILE *f) {
    int num_consumed = 0;
    int ch = 0;
    int comment = 0;

    while ((ch = fgetc(f)) != EOF) {
        if (ch == '\r' || ch == '\n' || ch == '\t' || ch == ' ' || ch == ','
            || ch == ';' || comment) {
            num_consumed++;
            if (ch == ';') comment = 1;
            if (ch == '\n') comment = 0;
        } else {
            fseek(f, -1, SEEK_CUR);
            return num_consumed;
        }
    }

    return -1;
}

int read_event_content(FILE *f, char *buf, int len) {
    int num_consumed = 0;
    int ch = 0;
    int quoted = 1;

    while (num_consumed < len - 1 && (ch = fgetc(f)) != EOF) {
        if (quoted) {
            if (buf[num_consumed - 1] == '(') break;
            if (buf[num_consumed - 1] == ')') break;
            if (buf[num_consumed - 1] == ':') break;
        }
        if (buf[num_consumed - 1] == '"') quoted ^= 1;

        if (
            quoted
            && (ch == '\r' || ch == '\n' || ch == '\t' || ch == ' '
                || ch == ',' || (num_consumed != 0 && ch == ')'))
        ) break;

        buf[num_consumed++] = ch;
    }

    if (!feof(f)) fseek(f, -1, SEEK_CUR);
    
    if (num_consumed >= len - 1) return -1;
    else {
        buf[num_consumed] = '\0';
        return num_consumed;
    }
}

int get_event_type(char *contents) {
    int result = 0;
    int last_char = strlen(contents) - 1;

    // Basic event types

    if (last_char > 0) {
        if (contents[0] == '%')
            result |= EVT_REG_LITERAL;
        else if (contents[0] == '$' || contents[0] == '#')
            result |= EVT_NUM_LITERAL;
        else if (contents[0] == '"')
            result |= EVT_STR_LITERAL;
        else if (contents[0] == '@')
            result |= EVT_LABEL_REF;
    }

    // Other event types
    if (last_char != 0 && contents[last_char] == ':')
        result |= EVT_NEW_LABEL;
    else if (contents[last_char] == '(')
        result |= EVT_BEGIN_ADDRESS;
    else if (contents[last_char] == ')')
        result |= EVT_END_ADDRESS;

    // If it was none of the above, verify it's a valid opcode
    
    if (!result) {
        for (int i = 0; i <= last_char; i++) {
            char ch = contents[i];
            if (!(('A' <= ch && ch <= 'Z')
                    || ('a' <= ch && ch <= 'z')
                    || (ch == '.')
                    || ('0' <= ch && ch <= '9')))
                return EVT_ERROR;
        }
        result |= EVT_OPCODE;
    }

    return result;
}

#ifdef TEST_EVENTIO
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "fatal: could not open %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    char content[256];

    while (1) {
        int ws_size = consume_whitespace(fp);
        if (ws_size == -1) {
            printf("End of file\n");
            break;
        }

        int evt_size = read_event_content(fp, content, 256);
        if (evt_size == -1) {
            printf("Error reading event\n");
            break;
        } else {
            printf("0x%03x: %s\n", get_event_type(content), content);
        }
    }

    fclose(fp);
    return 0;
}
#endif
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"

// asmparse - String processing

struct reg_def {
    char *name;
    int number;
};

static struct reg_def base_regs[] = {
    {"zr",  0 },
    
    {"ct",  1 },
    
    {"rv",  2 },
    {"p0",  3 },
    {"p1",  4 },
    {"p2",  5 },
    {"p3",  6 },
    {"p4",  7 },
    
    {"a0",  8 },
    {"a1",  9 },
    {"a2",  10},
    {"a3",  11},
    {"a4",  12},
    {"a5",  13},
    {"a6",  14},
    {"a7",  15},
    {"a8",  16},
    {"a9",  17},
    {"a10", 18},
    {"a11", 19},
    
    {"x0",  20},
    {"x1",  21},
    {"x2",  22},
    {"x3",  23},
    {"x4",  24},
    {"x5",  25},
    {"x6",  26},
    {"x7",  27},
    
    {"x8",  28},
    {"x9",  29},
    
    {"sp",  30},
    {"lr",  31},
};

static struct reg_def control_regs[] = {
    {"psq", 0 },
    {"apc", 1 },
    {"aps", 2 },
    {"asp", 3 },
    {"ia",  4 },
    {"eim", 5 },
    {"eip", 6 },
    {"fi",  7 },
    {"fa",  8 },
    {"tcr", 9 },
    
    {"b0",  16},
    {"b1",  17},
    {"b2",  18},
    {"b3",  19},
    {"b4",  20},
    {"b5",  21},
    {"b6",  22},
    {"b7",  23},
    {"b8",  24},
    {"b9",  25},
    {"b10", 26},
    {"b11", 27},
    {"b12", 28},
    {"b13", 29},
    {"b14", 30},
    {"b15", 31},
    
    {"d0",  32},
    {"d1",  33},
    {"d2",  34},
    {"d3",  35},
    {"d4",  36},
    {"d5",  37},
    {"d6",  38},
    {"d7",  39},
    {"d8",  40},
    {"d9",  41},
    {"d10", 42},
    {"d11", 43},
    {"d12", 44},
    {"d13", 45},
    {"d14", 46},
    {"d15", 47},
};

uint64_t get_number(
    char *input,
    uint64_t max,
    int *err
) {
    int base, offset;
    if (input[0] == '0') {
        if (input[1] == 'x') {
            offset = 2;
            base = 16;
        }
        else {
            offset = 1;
            base = 8;
        }
    }
    else {
        offset = 0;
        base = 10;
    }
    
    char *endptr = NULL;
    
    uint64_t result = (base == 16) ?
        strtoul(input + offset, &endptr, base) :
        (uint64_t) strtol(input + offset, &endptr, base);
    
    if (errno == ERANGE || *endptr != '\0') {
        errno = 0;
        *err = -1;
        return 0;
    }
    
    int64_t s_min = -((int64_t) ((max >> 1) + 1));
    if (((int64_t) result >= 0 && result > max)
        || (int64_t) result < s_min) {
        *err = -1;
        return 0;
    }
    
    *err = 0;
    return result;
}

uint64_t get_register_literal(
    char *input,
    int *err
) {
    // printf("^%s$\n", input);
    for (int i = 0; i < sizeof(base_regs) / sizeof(base_regs[0]); i++) {
        if (!strcmp(input, base_regs[i].name)) {
            return base_regs[i].number;
        }
    }
    
    return get_number(input, 31, err);
}

uint64_t get_control_register_literal(
    char *input,
    int *err
) {
    for (int i = 0; i < sizeof(control_regs) / sizeof(control_regs[0]); i++) {
        if (!strcmp(input, control_regs[i].name)) {
            return control_regs[i].number;
        }
    }
    
    return get_number(input, 0x7FFF, err);
}
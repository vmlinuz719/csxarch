#include <errno.h>
#include <stdint.h>
#include <stdlib.h>

#include "asm.h"

// asmparse - String processing

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

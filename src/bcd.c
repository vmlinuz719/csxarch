#include <stdio.h>
#include <stdint.h>

#include "bcd.h"

// Adapted from https://homepage.divms.uiowa.edu/~jones/bcd/bcd.html

int old_bcd_valid(uint64_t x) {
    uint64_t t1 = x + 0x0666666666666666;
    uint64_t t2 = t1 ^ x;
    uint64_t t3 = t2 ^ 0x1111111111111110;
    return !t3;
}

int bcd_valid(uint64_t x) {
    return old_bcd_valid(x >> 32) && old_bcd_valid(x & 0xFFFFFFFF);
}

uint64_t old_bcd_add(uint64_t a, uint64_t b) {
    uint64_t t1 = a + 0x0666666666666666;
    uint64_t t2 = t1 + b;
    uint64_t t3 = t1 ^ b;
    uint64_t t4 = t2 ^ t3;
    uint64_t t5 = ~t4 & 0x1111111111111110;
    uint64_t t6 = (t5 >> 2) | (t5 >> 3);
    return t2 - t6;
}

uint64_t tc2csx(uint64_t i) {
    uint64_t d = i >> 60;
    if (d >= 5) {
        return i + 0x6000000000000000;
    } else {
        return i;
    }
}

uint64_t csx2tc(uint64_t i) {
    uint64_t d = i >> 60;
    if (d >= 0xB) {
        return i - 0x6000000000000000;
    } else {
        return i;
    }
}

uint64_t csx2valid(uint64_t i) {
    uint64_t d = i >> 60;
    if (d < 0xB && d >= 0x5) {
        return 0xFFFFFFFFFFFFFFFF;
    } else {
        return csx2tc(i);
    }
}

uint64_t bcd_add(uint64_t a, uint64_t b, uint64_t *carry) {
    uint64_t result_lo32 = old_bcd_add(a & 0xFFFFFFFF, old_bcd_add(b & 0xFFFFFFFF, *carry));
    uint64_t result_hi32 = old_bcd_add(a >> 32, old_bcd_add(b >> 32, result_lo32 >> 32));
    *carry = result_hi32 >> 32;
    uint64_t result = (result_hi32 << 32) | (result_lo32 & 0xFFFFFFFF);
    return result;
}

uint64_t bcd_add_no_carry(uint64_t a, uint64_t b) {
    uint64_t result_lo32 = old_bcd_add(a & 0xFFFFFFFF, b & 0xFFFFFFFF);
    uint64_t result_hi32 = old_bcd_add(a >> 32, old_bcd_add(b >> 32, result_lo32 >> 32));
    uint64_t result = (result_hi32 << 32) | (result_lo32 & 0xFFFFFFFF);
    return result;
}

uint64_t bcd_neg(uint64_t a) {
    uint64_t carry = 0;
    return bcd_add(0x9999999999999999 - a, 1, &carry);
}

uint64_t bcd_inv(uint64_t a) {
    return 0x9999999999999999 - a;
}

uint64_t bcd_bin2dec(uint64_t x) {
    int is_negative = ((int64_t) x < 0);
    if (is_negative) x = -((int64_t) x);

    uint64_t result = 0;
    int count = 0;
    while (count <= 15 && x != 0) {
        result |= (x % 10) << (4 * count++);
        x /= 10;
    }

    return is_negative ? bcd_neg(result) : result;
}

uint64_t bcd_dec2bin(uint64_t x) {
    int is_negative = (x >> 60 >= 5);
    if (is_negative) x = bcd_neg(x);

    uint64_t result = 0;
    int count = 15;
    while (count >= 0) {
        result *= 10;
        result += ((x >> (4 * count--)) & 0xF);
    }

    return is_negative ? -result : result;
}

uint64_t bcd_sub(uint64_t a, uint64_t b, uint64_t *carry) {
    uint64_t c = *carry ? bcd_add_no_carry(b, 1) : b;
    *carry = 0;
    uint64_t result = bcd_add(a, bcd_neg(c), carry);
    *carry = !(*carry);
    return result;
}

uint64_t csx_8to16(uint64_t a) {
    if (((a >> 28) & 0xF) >= 0xB) {
        return (a | 0xF999999900000000) - 0x60000000;
    } else {
        return a;
    }
}

uint64_t csx_16to8(uint64_t a) {
    if (((a >> 28) & 0xF) >= 0x5) {
        return (a + 0x60000000) & 0xFFFFFFFF;
    } else {
        return a & 0xFFFFFFFF;
    }
}



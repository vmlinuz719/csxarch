#include <stdio.h>
#include <stdint.h>

// Adapted from https://homepage.divms.uiowa.edu/~jones/bcd/bcd.html

int bcd_valid(uint64_t x) {
    uint64_t t1 = x + 0x0666666666666666;
    uint64_t t2 = t1 ^ x;
    uint64_t t3 = t2 ^ 0x1111111111111110;
    return !t3;
}

uint64_t bcd_add(uint64_t a, uint64_t b) {
    uint64_t t1 = a + 0x0666666666666666;
    uint64_t t2 = t1 + b;
    uint64_t t3 = t1 ^ b;
    uint64_t t4 = t2 ^ t3;
    uint64_t t5 = ~t4 & 0x1111111111111110;
    uint64_t t6 = (t5 >> 2) | (t5 >> 3);
    return t2 - t6;
}

uint64_t bcd_neg(uint64_t x) {
    uint64_t t1 = 0xFFFFFFFFFFFFFFFF - x;
    uint64_t t2 = -x;
    uint64_t t3 = t1 ^ 1;
    uint64_t t4 = t2 ^ t3;
    uint64_t t5 = ~t4 & 0x1111111111111110;
    uint64_t t6 = (t5 >> 2) | (t5 >> 3);
    return t2 - t6;
}

uint64_t bcd_sub(uint64_t a, uint64_t b) {
    return bcd_add(a, bcd_neg(b));
}

uint64_t bcd_bin2dec(uint64_t x) {
    int is_negative = ((int64_t) x < 0);
    if (is_negative) x = -((int64_t) x);

    uint64_t result = 0;
    int count = 0;
    while (count < 15 && x != 0) {
        result |= (x % 10) << (4 * count++);
        x /= 10;
    }

    return is_negative ? bcd_neg(result) : result;
}

uint64_t bcd_dec2bin(uint64_t x) {
    int is_negative = ((int64_t) x < 0);
    if (is_negative) x = bcd_neg(x);

    uint64_t result = 0;
    int count = 15;
    while (count >= 0) {
        result *= 10;
        result += ((x >> (4 * count--)) & 0xF);
    }

    return is_negative ? -result : result;
}

uint64_t bcd_ext7(uint64_t x) {
    if (((x >> 28) & 0xF) == 0xF) return (x & 0x0FFFFFFF) | 0xF999999990000000;
    else return x;
}

uint64_t bcd_trunc7(uint64_t x) {
    if (((x >> 60) & 0xF) == 0xF) return (x & 0x0FFFFFFF) | 0xF0000000;
    else return x & 0x0FFFFFFF;
}

uint64_t new_bcd_add(uint64_t a, uint64_t b, uint64_t *carry) {
    uint64_t result_lo32 = bcd_add(a, bcd_add(b, *carry));
    uint64_t result_hi32 = bcd_add(a >> 32, bcd_add(b >> 32, result_lo32 >> 32));
    *carry = result_hi32 >> 32;
    return (result_hi32 << 32) | (result_lo32 & 0xFFFFFFFF);
}


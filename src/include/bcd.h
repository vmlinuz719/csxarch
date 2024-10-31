#ifndef _BCD_
#define _BCD_

#include <stdint.h>

int bcd_valid(uint64_t x);
uint64_t tc2csx(uint64_t i);
uint64_t csx2tc(uint64_t i);
uint64_t bcd_add(uint64_t a, uint64_t b, uint64_t *carry);
uint64_t bcd_sub(uint64_t a, uint64_t b, uint64_t *carry)
uint64_t bcd_neg(uint64_t a);
uint64_t bcd_inv(uint64_t a);
uint64_t bcd_bin2dec(uint64_t x);
uint64_t bcd_dec2bin(uint64_t x);
uint64_t csx_8to16(uint64_t a)
uint64_t csx_16to8(uint64_t a)

#endif
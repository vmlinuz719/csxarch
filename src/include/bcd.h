#ifndef _BCD_H_
#define _BCD_H_

int bcd_valid(uint64_t x);
uint64_t bcd_add(uint64_t a, uint64_t b);
uint64_t bcd_neg(uint64_t x);
uint64_t bcd_sub(uint64_t a, uint64_t b);
uint64_t bcd_bin2dec(uint64_t x);
uint64_t bcd_dec2bin(uint64_t x);
uint64_t bcd_ext7(uint64_t x);
uint64_t bcd_trunc7(uint64_t x);

#endif

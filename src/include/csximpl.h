#ifndef _CSXIMPL_
#define _CSXIMPL_

#include <stdint.h>
#include "csx.h"

void inst_conv_clr_block(em3_regs_t *r, uint64_t i);
void inst_LSI(em3_regs_t *r, uint64_t i);
void inst_LSIS(em3_regs_t *r, uint64_t i);
void inst_ext05_block_0f(em3_regs_t *r, uint64_t i);

void inst_MVR(em3_regs_t *r, uint64_t i);
void inst_load_imm_block(em3_regs_t *r, uint64_t i);
void inst_LA(em3_regs_t *r, uint64_t i);
void inst_LAL(em3_regs_t *r, uint64_t i);
void inst_LAPR(em3_regs_t *r, uint64_t i);
void inst_LAPRL(em3_regs_t *r, uint64_t i);

void inst_MVBL(em3_regs_t *r, uint64_t i);
void inst_MVST(em3_regs_t *r, uint64_t i);
void inst_MVSTR(em3_regs_t *r, uint64_t i);
void inst_CST(em3_regs_t *r, uint64_t i);
void inst_SKST(em3_regs_t *r, uint64_t i);
void inst_SCAST(em3_regs_t *r, uint64_t i);
void inst_XLTST(em3_regs_t *r, uint64_t i);
void inst_ext05_block_2f(em3_regs_t *r, uint64_t i);

// Register Math Block
void inst_AR(em3_regs_t *r, uint64_t i);
void inst_SR(em3_regs_t *r, uint64_t i);
void inst_NR(em3_regs_t *r, uint64_t i);
void inst_OR(em3_regs_t *r, uint64_t i);
void inst_XR(em3_regs_t *r, uint64_t i);
void inst_SLR(em3_regs_t *r, uint64_t i);
void inst_SRR(em3_regs_t *r, uint64_t i);
void inst_SRAR(em3_regs_t *r, uint64_t i);
void inst_ECR(em3_regs_t *r, uint64_t i);
void inst_imm_wide_block(em3_regs_t *r, uint64_t i);
void inst_imm_half_block(em3_regs_t *r, uint64_t i);
void inst_imm_long_block(em3_regs_t *r, uint64_t i);
void inst_ARCK(em3_regs_t *r, uint64_t i);
void inst_SRCK(em3_regs_t *r, uint64_t i);
void inst_MASR(em3_regs_t *r, uint64_t i);
void inst_DMR(em3_regs_t *r, uint64_t i);

// Load/Store Block
void inst_LC(em3_regs_t *r, uint64_t i);
void inst_LCS(em3_regs_t *r, uint64_t i);
void inst_LW(em3_regs_t *r, uint64_t i);
void inst_LWS(em3_regs_t *r, uint64_t i);
void inst_LH(em3_regs_t *r, uint64_t i);
void inst_LHS(em3_regs_t *r, uint64_t i);
void inst_L(em3_regs_t *r, uint64_t i);
void inst_STC(em3_regs_t *r, uint64_t i);
void inst_STW(em3_regs_t *r, uint64_t i);
void inst_STH(em3_regs_t *r, uint64_t i);
void inst_ST(em3_regs_t *r, uint64_t i);
void inst_NC(em3_regs_t *r, uint64_t i);
void inst_OC(em3_regs_t *r, uint64_t i);
void inst_XC(em3_regs_t *r, uint64_t i);
void inst_NW(em3_regs_t *r, uint64_t i);
void inst_OW(em3_regs_t *r, uint64_t i);
void inst_XW(em3_regs_t *r, uint64_t i);
void inst_AH(em3_regs_t *r, uint64_t i);
void inst_AHS(em3_regs_t *r, uint64_t i);
void inst_SH(em3_regs_t *r, uint64_t i);
void inst_SHS(em3_regs_t *r, uint64_t i);
void inst_NH(em3_regs_t *r, uint64_t i);
void inst_OH(em3_regs_t *r, uint64_t i);
void inst_XH(em3_regs_t *r, uint64_t i);
void inst_A(em3_regs_t *r, uint64_t i);
void inst_S(em3_regs_t *r, uint64_t i);
void inst_N(em3_regs_t *r, uint64_t i);
void inst_O(em3_regs_t *r, uint64_t i);
void inst_X(em3_regs_t *r, uint64_t i);

// Decimal Block
void inst_ADR(em3_regs_t *r, uint64_t i);
void inst_SDR(em3_regs_t *r, uint64_t i);
void inst_AD(em3_regs_t *r, uint64_t i);
void inst_SD(em3_regs_t *r, uint64_t i);
void inst_NED(em3_regs_t *r, uint64_t i);

// Stack Block
void inst_LCIN(em3_regs_t *r, uint64_t i);
void inst_LWIN(em3_regs_t *r, uint64_t i);
void inst_LHIN(em3_regs_t *r, uint64_t i);
void inst_LIN(em3_regs_t *r, uint64_t i);
void inst_STCIN(em3_regs_t *r, uint64_t i);
void inst_STWIN(em3_regs_t *r, uint64_t i);
void inst_STHIN(em3_regs_t *r, uint64_t i);
void inst_STIN(em3_regs_t *r, uint64_t i);
void inst_DESTC(em3_regs_t *r, uint64_t i);
void inst_DESTW(em3_regs_t *r, uint64_t i);
void inst_DESTH(em3_regs_t *r, uint64_t i);
void inst_DEST(em3_regs_t *r, uint64_t i);
void inst_LSK(em3_regs_t *r, uint64_t i);
void inst_STSK(em3_regs_t *r, uint64_t i);
void inst_LM(em3_regs_t *r, uint64_t i);
void inst_STM(em3_regs_t *r, uint64_t i);

// Control Register Block
void inst_MFCR(em3_regs_t *r, uint64_t i);
void inst_MTCR(em3_regs_t *r, uint64_t i);
void inst_LCR(em3_regs_t *r, uint64_t i);
void inst_STCR(em3_regs_t *r, uint64_t i);
void inst_REX(em3_regs_t *r, uint64_t i);

// Address Translation Control Block
void inst_MFPM(em3_regs_t *r, uint64_t i);
void inst_MTPM(em3_regs_t *r, uint64_t i);
void inst_page_map_block(em3_regs_t *r, uint64_t i);

// Channel Block
void inst_SSCH(em3_regs_t *r, uint64_t i);
void inst_XIO(em3_regs_t *r, uint64_t i);

// Branch Block
void inst_BAS(em3_regs_t *r, uint64_t i);
void inst_BASR(em3_regs_t *r, uint64_t i);
void inst_BASQ(em3_regs_t *r, uint64_t i);
void inst_BCASR(em3_regs_t *r, uint64_t i);

// Conditional Arithmetic Block
void inst_MVRcc_ARcc(em3_regs_t *r, uint64_t i);
void inst_SELcc(em3_regs_t *r, uint64_t i);
void inst_SRcc(em3_regs_t *r, uint64_t i);
void inst_NRcc(em3_regs_t *r, uint64_t i);
void inst_ORcc(em3_regs_t *r, uint64_t i);
void inst_XRcc(em3_regs_t *r, uint64_t i);
void inst_ECRcc(em3_regs_t *r, uint64_t i);
        
// Conditional Branch Block
void inst_BRcc(em3_regs_t *r, uint64_t i);

// Conditional Branch Immediate Block
void inst_BICcc(em3_regs_t *r, uint64_t i);

void inst_ex(em3_regs_t *r, uint64_t i);

#endif
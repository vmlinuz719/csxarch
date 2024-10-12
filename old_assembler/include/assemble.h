#ifndef _ASSEMBLE_H_
#define _ASSEMBLE_H_

#include "dpa.h"

int mark(struct parser_ctx *ctx, int opcode, int pass);

int dchar(struct parser_ctx *ctx, int opcode, int pass);

int dwide(struct parser_ctx *ctx, int opcode, int pass);

int dhalf(struct parser_ctx *ctx, int opcode, int pass);

int dword(struct parser_ctx *ctx, int opcode, int pass);

int dcstr(struct parser_ctx *ctx, int opcode, int pass);

int align(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rr(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rr_cr(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_none(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_none_2(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_br(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rm(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rm_lm(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rm_cr(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rm_stk(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rm_st(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rm_alt(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rb(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rw(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rw_alt(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rw_lwi(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_bi(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_bi_alt(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_b8(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_bc(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_ca(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_ca_3(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_wb(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_ic(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_ri_0(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_ri_w(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_ri_h(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_ri_l(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_rr_stk(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_ext05_rr(struct parser_ctx *ctx, int opcode, int pass);
int assemble_type_ext05_ri(struct parser_ctx *ctx, int opcode, int pass);
int assemble_type_e5_rm_st(struct parser_ctx *ctx, int opcode, int pass);

int assemble_type_bcasr(struct parser_ctx *ctx, int opcode, int pass);

int read_new_label_or_opcode(struct parser_ctx *ctx, int pass);

#endif
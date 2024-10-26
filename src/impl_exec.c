#include <stdint.h>
#include <stdio.h>
#include "csx.h"
#include "csximpl.h"

void inst_ex(em3_regs_t *r, uint64_t i) {
    // printf("%02lX@%016lX\n", MAJOR_OPCODE(i), r->pc);
    switch(MAJOR_OPCODE(i)) {
        // Unary Block
        case 0x00: inst_conv_clr_block  (r, i); break;
        case 0x01: inst_conv_clr_block  (r, i); break;
        case 0x02: inst_conv_clr_block  (r, i); break;
        case 0x03: inst_LSI             (r, i); break;
        case 0x04: inst_conv_clr_block  (r, i); break;
        case 0x05: inst_conv_clr_block  (r, i); break;
        case 0x06: inst_conv_clr_block  (r, i); break;
        case 0x07: inst_LSIS            (r, i); break;
        case 0x08: inst_conv_clr_block  (r, i); break;
        case 0x09: inst_conv_clr_block  (r, i); break;
        case 0x0A: inst_conv_clr_block  (r, i); break;
        case 0x0B: inst_conv_clr_block  (r, i); break;
        case 0x0C: inst_conv_clr_block  (r, i); break;
        case 0x0D: inst_conv_clr_block  (r, i); break;
        case 0x0E: inst_conv_clr_block  (r, i); break;
        case 0x0F: inst_ext05_block_0f  (r, i); break;
        
        // Move/Immediate Block
        case 0x10:  inst_MVR            (r, i); break;
        case 0x11:  inst_load_imm_block (r, i); break;
        case 0x12:  inst_LA             (r, i); break;
        case 0x13:  inst_LAL            (r, i); break;
        case 0x14:  inst_LAPR           (r, i); break;
        case 0x15:  inst_LAPRL          (r, i); break;
        
        // Move Multiple Block
        case 0x20:  inst_MVBL           (r, i); break;
        case 0x21:  inst_MVST           (r, i); break;
        case 0x22:  inst_MVSTR          (r, i); break;
        case 0x23:  inst_CST            (r, i); break;
        case 0x24:  inst_SKST           (r, i); break;
        case 0x25:  inst_SCAST          (r, i); break;
        case 0x26:  inst_XLTST          (r, i); break;
        case 0x2F:  inst_ext05_block_2f (r, i); break;

        // Register Math Block
        case 0x30:  inst_AR             (r, i); break;
        case 0x31:  inst_SR             (r, i); break;
        case 0x32:  inst_NR             (r, i); break;
        case 0x33:  inst_OR             (r, i); break;
        case 0x34:  inst_XR             (r, i); break;
        case 0x35:  inst_SLR            (r, i); break;
        case 0x36:  inst_SRR            (r, i); break;
        case 0x37:  inst_SRAR           (r, i); break;
        case 0x38:  inst_ECR            (r, i); break;

        case 0x39:  inst_imm_wide_block (r, i); break;
        case 0x3A:  inst_imm_half_block (r, i); break;
        case 0x3B:  inst_imm_long_block (r, i); break;
        
        case 0x3C:  inst_ARCK           (r, i); break;
        case 0x3D:  inst_SRCK           (r, i); break;
        
        case 0x3E:  inst_MASR           (r, i); break;
        case 0x3F:  inst_DMR            (r, i); break;

        // Load/Store Block
        case 0x40:  inst_LC             (r, i); break;
        case 0x41:  inst_LCS            (r, i); break;
        case 0x42:  inst_LW             (r, i); break;
        case 0x43:  inst_LWS            (r, i); break;
        case 0x44:  inst_LH             (r, i); break;
        case 0x45:  inst_LHS            (r, i); break;
        case 0x46:  inst_L              (r, i); break;

        case 0x47:  inst_STC            (r, i); break;
        case 0x48:  inst_STW            (r, i); break;
        case 0x49:  inst_STH            (r, i); break;
        case 0x4A:  inst_ST             (r, i); break;

        case 0x4B:  inst_NC             (r, i); break;
        case 0x4C:  inst_OC             (r, i); break;
        case 0x4D:  inst_XC             (r, i); break;

        case 0x4E:  inst_NW             (r, i); break;
        case 0x4F:  inst_OW             (r, i); break;
        case 0x50:  inst_XW             (r, i); break;

        case 0x51:  inst_AH             (r, i); break;
        case 0x52:  inst_AHS            (r, i); break;
        case 0x53:  inst_SH             (r, i); break;
        case 0x54:  inst_SHS            (r, i); break;
        case 0x55:  inst_NH             (r, i); break;
        case 0x56:  inst_OH             (r, i); break;
        case 0x57:  inst_XH             (r, i); break;

        case 0x58:  inst_A              (r, i); break;
        case 0x59:  inst_S              (r, i); break;
        case 0x5A:  inst_N              (r, i); break;
        case 0x5B:  inst_O              (r, i); break;
        case 0x5C:  inst_X              (r, i); break;

        // Stack Block
        case 0x60:  inst_LCIN           (r, i); break;
        case 0x61:  inst_LWIN           (r, i); break;
        case 0x62:  inst_LHIN           (r, i); break;
        case 0x63:  inst_LIN            (r, i); break;
        case 0x64:  inst_STCIN          (r, i); break;
        case 0x65:  inst_STWIN          (r, i); break;
        case 0x66:  inst_STHIN          (r, i); break;
        case 0x67:  inst_STIN           (r, i); break;
        case 0x68:  inst_DESTC          (r, i); break;
        case 0x69:  inst_DESTW          (r, i); break;
        case 0x6A:  inst_DESTH          (r, i); break;
        case 0x6B:  inst_DEST           (r, i); break;
        case 0x6C:  inst_LSK            (r, i); break;
        case 0x6D:  inst_STSK           (r, i); break;
        case 0x6E:  inst_LM             (r, i); break;
        case 0x6F:  inst_STM            (r, i); break;

        // Control Register Block
        case 0x70:  inst_MFCR           (r, i); break;
        case 0x71:  inst_MTCR           (r, i); break;
        case 0x72:  inst_LCR            (r, i); break;
        case 0x73:  inst_STCR           (r, i); break;
        case 0x74:  inst_REX            (r, i); break;

        // Address Translation Control Block
        case 0xA0:  inst_MFPM           (r, i); break;
        case 0xA1:  inst_MTPM           (r, i); break;
		case 0xA2:	inst_page_map_block (r, i); break;

        // Channel Block
        case 0xA3:  inst_SSCH           (r, i); break;
        case 0xA4:  inst_XIO            (r, i); break;

        // Branch Block
        case 0xB0:  inst_BAS            (r, i); break;
        case 0xB1:  inst_BASR           (r, i); break;
        case 0xB2:  inst_BASQ           (r, i); break;
        case 0xB3:  inst_BCASR          (r, i); break;

        // Conditional Arithmetic Block
        case 0xC0:  inst_MVRcc_ARcc     (r, i); break;
        case 0xC1:  inst_SELcc          (r, i); break;
        case 0xC2:  inst_SRcc           (r, i); break;
        case 0xC3:  inst_NRcc           (r, i); break;
        case 0xC4:  inst_ORcc           (r, i); break;
        case 0xC5:  inst_XRcc           (r, i); break;
        case 0xC6:  inst_ECRcc          (r, i); break;
        
        // Conditional Branch Block
        case 0xD1:  inst_BRcc           (r, i); break;
        case 0xD2:  inst_BRcc           (r, i); break;
        case 0xD3:  inst_BRcc           (r, i); break;
        case 0xD4:  inst_BRcc           (r, i); break;
        case 0xD5:  inst_BRcc           (r, i); break;
        case 0xD6:  inst_BRcc           (r, i); break;
        case 0xD7:  inst_BRcc           (r, i); break;
        case 0xD8:  inst_BRcc           (r, i); break;
        case 0xD9:  inst_BRcc           (r, i); break;
        case 0xDA:  inst_BRcc           (r, i); break;
        case 0xDB:  inst_BRcc           (r, i); break;
        case 0xDC:  inst_BRcc           (r, i); break;
        case 0xDD:  inst_BRcc           (r, i); break;
        case 0xDE:  inst_BRcc           (r, i); break;
        case 0xDF:  inst_BRcc           (r, i); break;
        
        // Conditional Branch Immediate Block
        case 0xE1:  inst_BICcc          (r, i); break;
        case 0xE2:  inst_BICcc          (r, i); break;
        case 0xE3:  inst_BICcc          (r, i); break;
        case 0xE4:  inst_BICcc          (r, i); break;
        case 0xE5:  inst_BICcc          (r, i); break;
        case 0xE6:  inst_BICcc          (r, i); break;
        case 0xE7:  inst_BICcc          (r, i); break;
        case 0xE8:  inst_BICcc          (r, i); break;
        case 0xE9:  inst_BICcc          (r, i); break;
        case 0xEA:  inst_BICcc          (r, i); break;
        case 0xEB:  inst_BICcc          (r, i); break;
        case 0xEC:  inst_BICcc          (r, i); break;
        case 0xED:  inst_BICcc          (r, i); break;

        default:
            
            INST_LEN(r, 0);
            error(r, ILLEGAL_INSTRUCTION);
    }
}

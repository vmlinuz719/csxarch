#include <stdint.h>
#include <string.h>
#include "csx.h"
#include "csximpl.h"

void inst_MVBL(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t count = GET_PSW_CTR(r);
    uint64_t step = count & 0x8000;
    count &= 0x7FFF;
    uint64_t size = RM_I12(i);
    uint64_t dst = RM_RD(i);
    uint64_t src = RM_RB(i);
    uint64_t ctr = RM_RX(i);
    
    // printf("%016lX\n", get_reg(r, CR_PSW));
    // printf("count %ld\n", count);
    
    if (!step) {
        // Load step
        // printf("load %016lX\n", get_reg(r, src) + count);
        em3_access_error_t e = OK;
        uint64_t result = vread_c(r, get_reg(r, src) + count, &e);
        if (e) {
            error(r, e);
            return;
        }
        
        SET_PSW_TMP(r, result);
        SET_PSW_CTR(r, count | 0x8000);
    } else {
        // printf("store %016lX\n", get_reg(r, dst) + count);
        // Store step
        em3_access_error_t e = OK;
        vwrite_c(r, get_reg(r, dst) + count, GET_PSW_TMP(r), &e);
        if (e) {
            error(r, e);
            return;
        }
        
        set_reg(r, ctr, get_reg(r, ctr) + 1);
        
        if (count == size) {
            // done
            r->increment = 4;
            return;
        } else {
            // printf("inc\n");
            // increment
            SET_PSW_CTR(r, count + 1);
            return;
        }
    }
}

void inst_MVST(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t count = GET_PSW_CTR(r);
    uint64_t term = RB_I8(i);
    uint64_t dst = RM_RD(i);
    uint64_t src = RM_RB(i);
    uint64_t ctr = RM_RX(i);
    uint64_t lim = B_RD(i);
    
    if (!count) {
        // Load step
        // printf("load %016lX\n", get_reg(r, src) + count);
        em3_access_error_t e = OK;
        uint64_t result = vread_c(r, get_reg(r, src) + get_reg(r, ctr), &e);
        if (e) {
            error(r, e);
            return;
        }
        
        SET_PSW_TMP(r, result);
        SET_PSW_CTR(r, 1);
    } else if (count == 1) {
        uint64_t ctr_value = get_reg(r, ctr);
        uint64_t lim_value = get_reg(r, lim);
        uint64_t chr = GET_PSW_TMP(r);

        em3_access_error_t e = OK;
        vwrite_c(r, get_reg(r, dst) + ctr_value, chr, &e);
        
        if (e) {
            error(r, e);
            return;
        }
        
        ctr_value++;
        set_reg(r, ctr, ctr_value);
        
        if (ctr_value == lim_value || chr == term) {
            // done
            r->increment = 4;
            return;
        } else {
            // printf("inc\n");
            // increment
            SET_PSW_CTR(r, 0);
            return;
        }
    } else {
        error(r, ILLEGAL_INSTRUCTION);
        return;
    }
}

void inst_MVSTR(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t count = GET_PSW_CTR(r);
    uint64_t term = RB_I8(i);
    uint64_t dst = RM_RD(i);
    uint64_t src = RM_RB(i);
    uint64_t ctr = RM_RX(i);
    uint64_t lim = B_RD(i);
    
    if (!count) {
        // Load step
        // printf("load %016lX\n", get_reg(r, src) + count);
        em3_access_error_t e = OK;
        uint64_t result = vread_c(r, get_reg(r, src) + get_reg(r, ctr), &e);
        if (e) {
            error(r, e);
            return;
        }
        
        if (result == term) {
            r->increment = 4;
            return;
        }
        
        SET_PSW_TMP(r, result);
        SET_PSW_CTR(r, 1);
    } else if (count == 1) {
        uint64_t ctr_value = get_reg(r, ctr);
        uint64_t lim_value = get_reg(r, lim);
        uint64_t chr = GET_PSW_TMP(r);

        em3_access_error_t e = OK;
        vwrite_c(r, get_reg(r, dst) + ctr_value, chr, &e);
        
        if (e) {
            error(r, e);
            return;
        }
        
        ctr_value++;
        set_reg(r, ctr, ctr_value);
        
        if (ctr_value == lim_value) {
            // done
            r->increment = 4;
            return;
        } else {
            // printf("inc\n");
            // increment
            SET_PSW_CTR(r, 0);
            return;
        }
    } else {
        error(r, ILLEGAL_INSTRUCTION);
        return;
    }
}

void inst_CST(em3_regs_t *r, uint64_t i) {
    INST_LEN(r, 4);

    uint64_t count = GET_PSW_CTR(r);
    uint64_t term = RB_I8(i);
    uint64_t dst = RM_RD(i);
    uint64_t src = RM_RB(i);
    uint64_t ctr = RM_RX(i);
    uint64_t lim = B_RD(i);

    if (!count) {
        // Load step
        // printf("load %016lX\n", get_reg(r, src) + count);
        em3_access_error_t e = OK;
        uint64_t result = vread_c(r, get_reg(r, src) + get_reg(r, ctr), &e);
        if (e) {
            error(r, e);
            return;
        }

        SET_PSW_TMP(r, result);
        SET_PSW_CTR(r, 1);
    } else if (count == 1) {
        uint64_t ctr_value = get_reg(r, ctr);
        uint64_t lim_value = get_reg(r, lim);
        uint64_t chr = GET_PSW_TMP(r);

        em3_access_error_t e = OK;
        uint64_t chr2 = vread_c(r, get_reg(r, dst) + ctr_value, &e);

        if (e) {
            error(r, e);
            return;
        }

        if (ctr_value + 1 == lim_value || chr == term || chr2 == term
            || chr != chr2) {
            // done
            r->increment = 4;
            return;
        } else {
            ctr_value++;
            set_reg(r, ctr, ctr_value);
            // printf("inc\n");
            // increment
            SET_PSW_CTR(r, 0);
            return;
        }
    } else {
        error(r, ILLEGAL_INSTRUCTION);
        return;
    }
}

void inst_SKST(em3_regs_t *r, uint64_t i) {
    INST_LEN(r, 4);

    uint64_t chr0 = RB_I8(i);
    uint64_t dst = RM_RD(i);
    uint64_t src = RM_RB(i);
    uint64_t ctr_r = RM_RX(i);
    uint64_t ctr = get_reg(r, ctr_r);

    uint64_t term2 = get_reg(r, B_RD(i));
    uint8_t chrn[8];
    memcpy(chrn, &term2, 8);

    em3_access_error_t e = OK;
    uint64_t result = vread_c(r, get_reg(r, src) + ctr, &e);
    if (e) {
        error(r, e);
        return;
    }

    if (
        result == chr0
        || result == chrn[0]
        || result == chrn[1]
        || result == chrn[2]
        || result == chrn[3]
        || result == chrn[4]
        || result == chrn[5]
        || result == chrn[6]
        || result == chrn[7]
    ) {
        set_reg(r, ctr_r, ctr + 1);
        return;
    }

    else {
        set_reg(r, dst, result);
        r->increment = 4;
        return;
    }
}

void inst_SCAST(em3_regs_t *r, uint64_t i) {
    INST_LEN(r, 4);

    uint64_t chr0 = RB_I8(i);
    uint64_t dst = RM_RD(i);
    uint64_t src = RM_RB(i);
    uint64_t ctr_r = RM_RX(i);
    uint64_t ctr = get_reg(r, ctr_r);

    uint64_t term2 = get_reg(r, B_RD(i));
    uint8_t chrn[8];
    memcpy(chrn, &term2, 8);

    em3_access_error_t e = OK;
    uint64_t result = vread_c(r, get_reg(r, src) + ctr, &e);
    if (e) {
        error(r, e);
        return;
    }   

    if (
        result == chr0 
        || result == chrn[0]
        || result == chrn[1]
        || result == chrn[2]
        || result == chrn[3]
        || result == chrn[4]
        || result == chrn[5]
        || result == chrn[6]
        || result == chrn[7] 
    ) {
        set_reg(r, dst, result);
        r->increment = 4;
        return;
    }

    else {
        set_reg(r, ctr_r, ctr + 1);
        return;
    }
}

void inst_XLTST(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t count = GET_PSW_CTR(r);
    uint64_t term = RB_I8(i);
    uint64_t dst = RM_RD(i);
    uint64_t src = RM_RB(i);
    uint64_t ctr = RM_RX(i);
    uint64_t tbl = B_RD(i);
    
    if (!count) {
        // Load step
        // printf("load %016lX\n", get_reg(r, src) + count);
        em3_access_error_t e = OK;
        uint64_t result = vread_c(r, get_reg(r, src) + get_reg(r, ctr), &e);
        if (e) {
            error(r, e);
            return;
        }
        
        SET_PSW_TMP(r, result);
        SET_PSW_CTR(r, 1);
    } else if (count == 1) {
        // Load step 2
        em3_access_error_t e = OK;
        uint64_t chr = GET_PSW_TMP(r);
        // printf("load %016lX\n", get_reg(r, tbl) + chr);
        uint64_t result = vread_c(r, get_reg(r, tbl) + chr, &e);
        if (e) {
            error(r, e);
            return;
        }
        
        SET_PSW_TMP(r, result);
        SET_PSW_CTR(r, chr == term ? 3 : 2);
    } else if (count == 2 || count == 3) {
        uint64_t ctr_value = get_reg(r, ctr);
        uint64_t chr = GET_PSW_TMP(r);

        em3_access_error_t e = OK;
        vwrite_c(r, get_reg(r, dst) + ctr_value, chr, &e);
        
        if (e) {
            error(r, e);
            return;
        }
        
        ctr_value++;
        set_reg(r, ctr, ctr_value);
        
        if (count == 3) {
            // done
            r->increment = 4;
            return;
        } else {
            // printf("inc\n");
            // increment
            SET_PSW_CTR(r, 0);
            return;
        }
    } else {
        error(r, ILLEGAL_INSTRUCTION);
        return;
    }
}

void inst_MVBLC(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    uint64_t count = GET_PSW_CTR(r);
    uint64_t dst = EXT05_RR_RD(i);
    uint64_t src = EXT05_RC_RB(i);
    uint64_t ctr = EXT05_RC_RX(i);
    uint64_t lim = EXT05_RC_RC(i);
    
    if (!count) {
        // Load step
        // printf("load %016lX\n", get_reg(r, src) + count);
        em3_access_error_t e = OK;
        uint64_t result = vread_c(r, get_reg(r, src) + get_reg(r, ctr), &e);
        if (e) {
            error(r, e);
            return;
        }
        
        SET_PSW_TMP(r, result);
        SET_PSW_CTR(r, 1);
    } else if (count == 1) {
        uint64_t ctr_value = get_reg(r, ctr);
        uint64_t chr = GET_PSW_TMP(r);

        em3_access_error_t e = OK;
        vwrite_c(r, get_reg(r, dst) + ctr_value, chr, &e);
        
        if (e) {
            error(r, e);
            return;
        }
        
        ctr_value++;
        set_reg(r, ctr, ctr_value);
        
        if (ctr_value == get_reg(r, lim)) {
            // done
            r->increment = 4;
            return;
        } else {
            // printf("inc\n");
            // increment
            SET_PSW_CTR(r, 0);
            return;
        }
    } else {
        error(r, ILLEGAL_INSTRUCTION);
        return;
    }
}

void inst_ext05_block_2f(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
    

    switch(EXT05_MINOR(i)) {
        case 0:
            inst_MVBLC(r, i);
            break;

        default:
            r->increment = 0;
            error(r, ILLEGAL_INSTRUCTION);
    }
}

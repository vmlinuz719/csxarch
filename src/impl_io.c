#include <stdint.h>
#include <stdio.h>
#include "csx.h"
#include "csximpl.h"

void inst_SSCH(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    if ((get_reg(r, CR_PSW) & NEW_CR_PSW_PL)) {
        error(r, PRIVILEGE_VIOLATION);
        return;
    }

    int rd = RR_RD(i);

    uint64_t channel = get_reg(r, RM_RX(i))
        + RM_I12(i);
    
    int reg = RM_RB(i);

    uint64_t result;

    if (
        channel >= r->num_units
        || r->mmio[channel].ctx == NULL
        || r->mmio[channel].sense == NULL
    ) {
        result = 0;
    } else {
        result = r->mmio[channel].sense(
            r->mmio[channel].ctx,
            reg
        );
    }

    r->increment = 4;
    set_reg(r, RM_RD(i), result);
}

void inst_XIO(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 4);
	
    if ((get_reg(r, CR_PSW) & NEW_CR_PSW_PL)) {
        error(r, PRIVILEGE_VIOLATION);
        return;
    }

    int rd = RR_RD(i);

    uint64_t channel = get_reg(r, RM_RX(i))
        + RM_I12(i);
    
    int reg = RM_RB(i);

    uint64_t command = get_reg(r, rd);

    em3_access_error_t e = OK;

    if (
        channel >= r->num_units
        || r->mmio[channel].ctx == NULL
        || r->mmio[channel].command == NULL
    ) {
        error(r, BUS_ERROR);
        return;
    } else {
        (void) r->mmio[channel].command(
            r->mmio[channel].ctx,
            reg,
            command,
            &e
        );
    }

    if (e) {
        error(r, e);
    } else {
        r->increment = 4;
    }
}
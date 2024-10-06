#include <stdint.h>
#include "csx.h"
#include "pagemap.h"
#include "csximpl.h"

void inst_MFPM(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    if ((get_reg(r, CR_PSW) & NEW_CR_PSW_PL))
        error(r, PRIVILEGE_VIOLATION);
    else {
        r->increment = 2;
        
        uint64_t query = get_reg(r, RR_RS(i));
        
        int map_index = lookup_no_global(
            &r->map,
            query >> 12,
            query & 0xFFF
        );
        
        if (map_index < 0) 
            set_reg(r, RR_RD(i), 0);
        else {
            uint64_t result =
                (r->map.map[map_index].pa << 12)
                | (r->map.map[map_index].flags); 
            set_reg(r, RR_RD(i), result);
        }
    }
}

void inst_MTPM(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    if ((get_reg(r, CR_PSW) & NEW_CR_PSW_PL))
        error(r, PRIVILEGE_VIOLATION);
    else {
        r->increment = 2;
        
        uint64_t va = get_reg(r, RR_RS(i));
        uint64_t pa = get_reg(r, RR_RD(i));
        
        int result = upsert(
            &(r->map),
            (em3_page_map_entry_t)
            {
                va >> 12,
                pa >> 12,
                va & 0xFFF,
                pa & 0x7FF
            }
        );
        
        // #define PM_ERROR_MAX_PINNED -1
        // #define PM_ERROR_GLOBAL_OVERWRITE -2
        
        if (result == PM_ERROR_MAX_PINNED) {
            set_reg(r, RR_RD(i), PM_STICKY);
        } else if (result == PM_ERROR_GLOBAL_OVERWRITE) {
            set_reg(r, RR_RD(i), PM_VALID);
        } else {
            set_reg(r, RR_RD(i), 0);
        }
    }
}

void inst_page_map_block(em3_regs_t *r, uint64_t i) {
	INST_LEN(r, 2);
	
    if ((get_reg(r, CR_PSW) & NEW_CR_PSW_PL))
        error(r, PRIVILEGE_VIOLATION);
    else {
        r->increment = 2;
		
		switch(MINOR_OPCODE_0(i)) {
			case 0:
				if (r->map.num_present < PM_MAP_SIZE) {
					set_reg(r, RR_RS(i), 0);
				} else {
					int map_index = r->map.next_victim;
					
					uint64_t result = 
						(r->map.map[map_index].va << 12)
						| (r->map.map[map_index].asid & 0xFFF); 
					set_reg(r, RR_RS(i), result);
				}
				break;
			
			default:
				r->increment = 0;
				error(r, ILLEGAL_INSTRUCTION);
		}
	}
}

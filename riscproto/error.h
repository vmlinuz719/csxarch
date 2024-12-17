#ifndef _ERROR_
#define _ERROR_

typedef enum {
    MCHK = 0,  // Machine Check
    CPRC,      // Coprocessor Exception
    PWRF,      // Power Failure Imminent
    WDOG,      // Non-Maskable Interrupt
    EXTN,      // External Interrupt
    PBRK,      // Program Counter Breakpoint
    BERR,      // Bus Error
    EMLT,      // Emulation Trap/Control Register Not Present

    RSGV,      // Read Segmentation Fault
    WSGV,      // Write Segmentation Fault
    XSGV,      // Execute Segmentation Fault
    DALT,      // Data Alignment Trap
    XALT,      // Execute Alignment Trap
    IPLT,      // Instruction Protection Level Trap

    OVRF,      // Arithmetic Overflow Trap
    DIVZ,      // Divide By Zero Trap

    TIME = 32, // Timer
    SVCT = 48  // Supervisor Call Trap
} lcca_error_t;

#endif


#ifndef _ERROR_
#define _ERROR_

typedef enum {
    OK = 0,
    PAGE_FAULT,
    BUS_ERROR,
    ERR_ALIGN,
    ILLEGAL_INSTRUCTION,
    PRIVILEGE_VIOLATION,
    DIVIDE_BY_ZERO,
    DECIMAL_FORMAT
} lcca_error_t;

#endif


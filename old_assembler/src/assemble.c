#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dpa.h"
#include "eventio.h"
#include "parsevt.h"
#include "assemble.h"

// #define TEST_PARSEVT

struct opcode_def opcodes[] = {
    {   "CC",       0x00,       assemble_type_rr    },
    {   "CW",       0x01,       assemble_type_rr    },
    {   "CH",       0x02,       assemble_type_rr    },
    {   "LSI",      0x03,       assemble_type_rr    },
    {   "CLC",      0x04,       assemble_type_rr    },
    {   "CLW",      0x05,       assemble_type_rr    },
    {   "CLH",      0x06,       assemble_type_rr    },
    {   "LSIS",     0x07,       assemble_type_rr    },
    {   "I",        0x08,       assemble_type_rr    },
    {   "NE",       0x09,       assemble_type_rr    },
    {   "IN",       0x0A,       assemble_type_rr    },
    {   "DE",       0x0B,       assemble_type_rr    },
    {   "SLS",      0x0C,       assemble_type_rr    },
    {   "SRS",      0x0D,       assemble_type_rr    },
    {   "SRAS",     0x0E,       assemble_type_rr    },
    
    {   "BSWPW",    0x0F00,     assemble_type_ext05_rr},
    {   "BSWPH",    0x0F01,     assemble_type_ext05_rr},
    {   "BSWP",     0x0F02,     assemble_type_ext05_rr},
    {   "SL",       0x0F03,     assemble_type_ext05_ri},
    {   "SLA",      0x0F04,     assemble_type_ext05_ri},
    {   "SLD",      0x0F05,     assemble_type_ext05_ri},
    
    
    {   "MVR",      0x10,       assemble_type_rr    },
    
    {   "LWIS",     0x111,      assemble_type_ri_w  },
    {   "LHI",      0x112,      assemble_type_ri_h  },
    {   "LHIS",     0x113,      assemble_type_ri_h  },
    {   "LLI",      0x114,      assemble_type_ri_l  },
    {   "LLIS",     0x115,      assemble_type_ri_l  },

    {   "LA",       0x12,       assemble_type_rm    },
    {   "ARM",      0x12,       assemble_type_rm_alt}, // legacy
    {   "AER",      0x12,       assemble_type_rm_alt},
    
    {   "LAL",      0x13,       assemble_type_rw    },
    {   "AWI",      0x13,       assemble_type_rw_alt},
    {   "LWI",      0x13,       assemble_type_rw_lwi},
    
    {   "LAR",      0x14,       assemble_type_rm    },
    {   "LARL",     0x15,       assemble_type_rw    },
    
    {   "MVBL",     0x20,       assemble_type_rm    },
    {   "MVST",     0x21,       assemble_type_rm_st },
    {   "MVSTR",    0x22,       assemble_type_rm_st },
    {   "CST",      0x23,       assemble_type_rm_st },
    {   "SKST",     0x24,       assemble_type_rm_st },
    {   "SCAST",    0x25,       assemble_type_rm_st },
    {   "XLTST",    0x26,       assemble_type_rm_st },
    {   "MVBLC",    0x2F00,     assemble_type_e5_rm_st},

    {   "AR",       0x30,       assemble_type_rr    },
    {   "SR",       0x31,       assemble_type_rr    },
    {   "NR",       0x32,       assemble_type_rr    },
    {   "OR",       0x33,       assemble_type_rr    },
    {   "XR",       0x34,       assemble_type_rr    },
    {   "SLR",      0x35,       assemble_type_rr    },
    {   "SRR",      0x36,       assemble_type_rr    },
    {   "SRAR",     0x37,       assemble_type_rr    },
    {   "ECR",      0x38,       assemble_type_rr    },
    
    {   "SWI",      0x391,      assemble_type_ri_w  },
    {   "NWI",      0x392,      assemble_type_ri_w  },
    {   "OWI",      0x393,      assemble_type_ri_w  },
    {   "OIHH",     0x394,      assemble_type_ri_w  },
    {   "XWI",      0x395,      assemble_type_ri_w  },
    
    {   "AHI",      0x3A0,      assemble_type_ri_h  },
    {   "SHI",      0x3A1,      assemble_type_ri_h  },
    {   "NHI",      0x3A2,      assemble_type_ri_h  },
    {   "OHI",      0x3A3,      assemble_type_ri_h  },
    {   "OIH",      0x3A4,      assemble_type_ri_h  },
    {   "XHI",      0x3A5,      assemble_type_ri_h  },
    
    {   "AI",       0x3B0,      assemble_type_ri_l  },
    {   "SI",       0x3B1,      assemble_type_ri_l  },
    {   "NI",       0x3B2,      assemble_type_ri_l  },
    {   "OI",       0x3B3,      assemble_type_ri_l  },
    {   "XI",       0x3B4,      assemble_type_ri_l  },
    
    {   "ARCK",     0x3C,       assemble_type_rm_alt},
    {   "SRCK",     0x3D,       assemble_type_rm_alt},
    
    {   "MASR",     0x3E,       assemble_type_rm_alt},
    {   "DMR",      0x3F,       assemble_type_rm_alt},
    
    {   "LC",       0x40,       assemble_type_rm    },
    {   "LCS",      0x41,       assemble_type_rm    },
    {   "LW",       0x42,       assemble_type_rm    },
    {   "LWS",      0x43,       assemble_type_rm    },
    {   "LH",       0x44,       assemble_type_rm    },
    {   "LHS",      0x45,       assemble_type_rm    },
    {   "L",        0x46,       assemble_type_rm    },
    
    {   "STC",      0x47,       assemble_type_rm    },
    {   "STW",      0x48,       assemble_type_rm    },
    {   "STH",      0x49,       assemble_type_rm    },
    {   "ST",       0x4A,       assemble_type_rm    },

    {   "NC",       0x4B,       assemble_type_rm    },
    {   "OC",       0x4C,       assemble_type_rm    },
    {   "XC",       0x4D,       assemble_type_rm    },
    
    {   "NW",       0x4E,       assemble_type_rm    },
    {   "OW",       0x4F,       assemble_type_rm    },
    {   "XW",       0x50,       assemble_type_rm    },
    
    {   "AH",       0x51,       assemble_type_rm    },
    {   "AHS",      0x52,       assemble_type_rm    },
    {   "SH",       0x53,       assemble_type_rm    },
    {   "SHS",      0x54,       assemble_type_rm    },
    {   "NH",       0x55,       assemble_type_rm    },
    {   "OH",       0x56,       assemble_type_rm    },
    {   "XH",       0x57,       assemble_type_rm    },
    
    {   "A",        0x58,       assemble_type_rm    },
    {   "S",        0x59,       assemble_type_rm    },
    {   "N",        0x5A,       assemble_type_rm    },
    {   "O",        0x5B,       assemble_type_rm    },
    {   "X",        0x5C,       assemble_type_rm    },

    {   "LCIN",     0x60,       assemble_type_rm    }, // legacy
    {   "LWIN",     0x61,       assemble_type_rm    },
    {   "LHIN",     0x62,       assemble_type_rm    },
    {   "LIN",      0x63,       assemble_type_rm    },
    
    {   "LCTC",     0x60,       assemble_type_rm    },
    {   "LCTW",     0x61,       assemble_type_rm    },
    {   "LCTH",     0x62,       assemble_type_rm    },
    {   "LCT",      0x63,       assemble_type_rm    },
    
    {   "POPC",     0x60,       assemble_type_rm_stk},
    {   "POPW",     0x61,       assemble_type_rm_stk},
    {   "POPH",     0x62,       assemble_type_rm_stk},

    {   "POP",      0x6C,       assemble_type_rr_stk},
    
    {   "STCIN",    0x64,       assemble_type_rm    }, // legacy
    {   "STWIN",    0x65,       assemble_type_rm    },
    {   "STHIN",    0x66,       assemble_type_rm    },
    {   "STIN",     0x67,       assemble_type_rm    },
    
    {   "STCTC",    0x64,       assemble_type_rm    },
    {   "STCTW",    0x65,       assemble_type_rm    },
    {   "STCTH",    0x66,       assemble_type_rm    },
    {   "STCT",     0x67,       assemble_type_rm    },
    
    {   "DESTC",    0x68,       assemble_type_rm    }, // legacy
    {   "DESTW",    0x69,       assemble_type_rm    },
    {   "DESTH",    0x6A,       assemble_type_rm    },
    {   "DEST",     0x6B,       assemble_type_rm    },
    
    {   "STCDC",    0x68,       assemble_type_rm    },
    {   "STCDW",    0x69,       assemble_type_rm    },
    {   "STCDH",    0x6A,       assemble_type_rm    },
    {   "STCD",     0x6B,       assemble_type_rm    },
    
    {   "PUSHC",    0x68,       assemble_type_rm_stk},
    {   "PUSHW",    0x69,       assemble_type_rm_stk},
    {   "PUSHH",    0x6A,       assemble_type_rm_stk},

    {   "PUSH",     0x6D,       assemble_type_rr_stk},

    {   "LSK",      0x6C,       assemble_type_rr    }, // legacy
    {   "STSK",     0x6D,       assemble_type_rr    },
    
    {   "LCTR",     0x6C,       assemble_type_rr    },
    {   "STCDR",    0x6D,       assemble_type_rr    },
    
    {   "LM",       0x6E,       assemble_type_rm_lm },
    {   "STM",      0x6F,       assemble_type_rm_lm },

    {   "MFCR",     0x70,       assemble_type_rr_cr },
    {   "MTCR",     0x71,       assemble_type_rr_cr },
    {   "LCR",      0x72,       assemble_type_rm_cr },
    {   "STCR",     0x73,       assemble_type_rm_cr },
    {   "REX",      0x74,       assemble_type_none  },
    {   "SVC",      0x74,       assemble_type_ic    },

    {   "ADR",      0x80,       assemble_type_rm_alt},
    {   "SDR",      0x81,       assemble_type_rm_alt},
    {   "AD",       0x82,       assemble_type_rm_st },
    {   "SD",       0x83,       assemble_type_rm_st },
    {   "NED",      0x84,       assemble_type_rr    },

    {   "MFPM",     0xA0,       assemble_type_rr    },
    {   "MTPM",     0xA1,       assemble_type_rr    },
    {   "EVLA",     0xA20,      assemble_type_ri_0  },

    {   "SIO",      0xA3,       assemble_type_rm_alt},
    {   "XIO",      0xA4,       assemble_type_rm_alt},
    
    {   "B",        0xB0,       assemble_type_rb    },
    {   "BAS",      0xB0,       assemble_type_rmb   },
    
    {   "BR",       0xB1,       assemble_type_wb    },
    {   "BASR",     0xB1,       assemble_type_rw    },
    
    {   "BX",       0xB2,       assemble_type_br    },
    {   "BASX",     0xB2,       assemble_type_rr    },
    {   "RET",      0xB204,     assemble_type_none_2},

    {   "BCASR",    0xB3,       assemble_type_bcasr    },
    
    {   "AR.LT",     0xC02,      assemble_type_ca    }, // legacy
    {   "AR.GE",     0xC03,      assemble_type_ca    },
    {   "AR.LE",     0xC04,      assemble_type_ca    },
    {   "AR.GT",     0xC05,      assemble_type_ca    },
    {   "AR.SC",     0xC06,      assemble_type_ca    },
    {   "AR.XE",     0xC07,      assemble_type_ca    },
    {   "AR.SE",     0xC08,      assemble_type_ca    },
    {   "AR.XC",     0xC09,      assemble_type_ca    },
    {   "AR.EQ",     0xC0A,      assemble_type_ca    },
    {   "AR.NE",     0xC0B,      assemble_type_ca    },
    {   "AR.ST",     0xC0C,      assemble_type_ca    },
    {   "AR.CL",     0xC0D,      assemble_type_ca    },
    {   "AR.SI",     0xC0E,      assemble_type_ca    },
    {   "AR.CI",     0xC0F,      assemble_type_ca    },
    
    {   "ARLT",      0xC02,      assemble_type_ca    },
    {   "ARGE",      0xC03,      assemble_type_ca    },
    {   "ARLE",      0xC04,      assemble_type_ca    },
    {   "ARGT",      0xC05,      assemble_type_ca    },
    {   "ARSC",      0xC06,      assemble_type_ca    },
    {   "ARXE",      0xC07,      assemble_type_ca    },
    {   "ARSE",      0xC08,      assemble_type_ca    },
    {   "ARXC",      0xC09,      assemble_type_ca    },
    {   "AREQ",      0xC0A,      assemble_type_ca    },
    {   "ARNE",      0xC0B,      assemble_type_ca    },
    {   "ARST",      0xC0C,      assemble_type_ca    },
    {   "ARCL",      0xC0D,      assemble_type_ca    },
    {   "ARSI",      0xC0E,      assemble_type_ca    },
    {   "ARCI",      0xC0F,      assemble_type_ca    },
    
    {   "SEL.LT",    0xC12,      assemble_type_ca    }, // legacy
    {   "SEL.GE",    0xC13,      assemble_type_ca    },
    {   "SEL.LE",    0xC14,      assemble_type_ca    },
    {   "SEL.GT",    0xC15,      assemble_type_ca    },
    {   "SEL.SC",    0xC16,      assemble_type_ca    },
    {   "SEL.XE",    0xC17,      assemble_type_ca    },
    {   "SEL.SE",    0xC18,      assemble_type_ca    },
    {   "SEL.XC",    0xC19,      assemble_type_ca    },
    {   "SEL.EQ",    0xC1A,      assemble_type_ca    },
    {   "SEL.NE",    0xC1B,      assemble_type_ca    },
    {   "SEL.ST",    0xC1C,      assemble_type_ca    },
    {   "SEL.CL",    0xC1D,      assemble_type_ca    },
    {   "SEL.SI",    0xC1E,      assemble_type_ca    },
    {   "SEL.CI",    0xC1F,      assemble_type_ca    },
    
    {   "SELLT",     0xC12,      assemble_type_ca    },
    {   "SELGE",     0xC13,      assemble_type_ca    },
    {   "SELLE",     0xC14,      assemble_type_ca    },
    {   "SELGT",     0xC15,      assemble_type_ca    },
    {   "SELSC",     0xC16,      assemble_type_ca    },
    {   "SELXE",     0xC17,      assemble_type_ca    },
    {   "SELSE",     0xC18,      assemble_type_ca    },
    {   "SELXC",     0xC19,      assemble_type_ca    },
    {   "SELEQ",     0xC1A,      assemble_type_ca    },
    {   "SELNE",     0xC1B,      assemble_type_ca    },
    {   "SELST",     0xC1C,      assemble_type_ca    },
    {   "SELCL",     0xC1D,      assemble_type_ca    },
    {   "SELSI",     0xC1E,      assemble_type_ca    },
    {   "SELCI",     0xC1F,      assemble_type_ca    },
    
    {   "SR3",       0xC20,      assemble_type_ca_3  }, // legacy
    {   "SR.LT",     0xC22,      assemble_type_ca    },
    {   "SR.GE",     0xC23,      assemble_type_ca    },
    {   "SR.LE",     0xC24,      assemble_type_ca    },
    {   "SR.GT",     0xC25,      assemble_type_ca    },
    {   "SR.SC",     0xC26,      assemble_type_ca    },
    {   "SR.XE",     0xC27,      assemble_type_ca    },
    {   "SR.SE",     0xC28,      assemble_type_ca    },
    {   "SR.XC",     0xC29,      assemble_type_ca    },
    {   "SR.EQ",     0xC2A,      assemble_type_ca    },
    {   "SR.NE",     0xC2B,      assemble_type_ca    },
    {   "SR.ST",     0xC2C,      assemble_type_ca    },
    {   "SR.CL",     0xC2D,      assemble_type_ca    },
    {   "SR.SI",     0xC2E,      assemble_type_ca    },
    {   "SR.CI",     0xC2F,      assemble_type_ca    },
    
    {   "SER",       0xC20,      assemble_type_ca_3  },
    {   "SRLT",      0xC22,      assemble_type_ca    },
    {   "SRGE",      0xC23,      assemble_type_ca    },
    {   "SRLE",      0xC24,      assemble_type_ca    },
    {   "SRGT",      0xC25,      assemble_type_ca    },
    {   "SRSC",      0xC26,      assemble_type_ca    },
    {   "SRXE",      0xC27,      assemble_type_ca    },
    {   "SRSE",      0xC28,      assemble_type_ca    },
    {   "SRXC",      0xC29,      assemble_type_ca    },
    {   "SREQ",      0xC2A,      assemble_type_ca    },
    {   "SRNE",      0xC2B,      assemble_type_ca    },
    {   "SRST",      0xC2C,      assemble_type_ca    },
    {   "SRCL",      0xC2D,      assemble_type_ca    },
    {   "SRSI",      0xC2E,      assemble_type_ca    },
    {   "SRCI",      0xC2F,      assemble_type_ca    },
    
    {   "NR3",       0xC30,      assemble_type_ca_3  }, // legacy
    {   "NR.LT",     0xC32,      assemble_type_ca    },
    {   "NR.GE",     0xC33,      assemble_type_ca    },
    {   "NR.LE",     0xC34,      assemble_type_ca    },
    {   "NR.GT",     0xC35,      assemble_type_ca    },
    {   "NR.SC",     0xC36,      assemble_type_ca    },
    {   "NR.XE",     0xC37,      assemble_type_ca    },
    {   "NR.SE",     0xC38,      assemble_type_ca    },
    {   "NR.XC",     0xC39,      assemble_type_ca    },
    {   "NR.EQ",     0xC3A,      assemble_type_ca    },
    {   "NR.NE",     0xC3B,      assemble_type_ca    },
    {   "NR.ST",     0xC3C,      assemble_type_ca    },
    {   "NR.CL",     0xC3D,      assemble_type_ca    },
    {   "NR.SI",     0xC3E,      assemble_type_ca    },
    {   "NR.CI",     0xC3F,      assemble_type_ca    },
    
    {   "NER",       0xC30,      assemble_type_ca_3  },
    {   "NRLT",      0xC32,      assemble_type_ca    },
    {   "NRGE",      0xC33,      assemble_type_ca    },
    {   "NRLE",      0xC34,      assemble_type_ca    },
    {   "NRGT",      0xC35,      assemble_type_ca    },
    {   "NRSC",      0xC36,      assemble_type_ca    },
    {   "NRXE",      0xC37,      assemble_type_ca    },
    {   "NRSE",      0xC38,      assemble_type_ca    },
    {   "NRXC",      0xC39,      assemble_type_ca    },
    {   "NREQ",      0xC3A,      assemble_type_ca    },
    {   "NRNE",      0xC3B,      assemble_type_ca    },
    {   "NRST",      0xC3C,      assemble_type_ca    },
    {   "NRCL",      0xC3D,      assemble_type_ca    },
    {   "NRSI",      0xC3E,      assemble_type_ca    },
    {   "NRCI",      0xC3F,      assemble_type_ca    },
    
    {   "OR3",       0xC40,      assemble_type_ca_3  }, // legacy
    {   "OR.LT",     0xC42,      assemble_type_ca    },
    {   "OR.GE",     0xC43,      assemble_type_ca    },
    {   "OR.LE",     0xC44,      assemble_type_ca    },
    {   "OR.GT",     0xC45,      assemble_type_ca    },
    {   "OR.SC",     0xC46,      assemble_type_ca    },
    {   "OR.XE",     0xC47,      assemble_type_ca    },
    {   "OR.SE",     0xC48,      assemble_type_ca    },
    {   "OR.XC",     0xC49,      assemble_type_ca    },
    {   "OR.EQ",     0xC4A,      assemble_type_ca    },
    {   "OR.NE",     0xC4B,      assemble_type_ca    },
    {   "OR.ST",     0xC4C,      assemble_type_ca    },
    {   "OR.CL",     0xC4D,      assemble_type_ca    },
    {   "OR.SI",     0xC4E,      assemble_type_ca    },
    {   "OR.CI",     0xC4F,      assemble_type_ca    },
    
    {   "OER",       0xC40,      assemble_type_ca_3  },
    {   "ORLT",      0xC42,      assemble_type_ca    },
    {   "ORGE",      0xC43,      assemble_type_ca    },
    {   "ORLE",      0xC44,      assemble_type_ca    },
    {   "ORGT",      0xC45,      assemble_type_ca    },
    {   "ORSC",      0xC46,      assemble_type_ca    },
    {   "ORXE",      0xC47,      assemble_type_ca    },
    {   "ORSE",      0xC48,      assemble_type_ca    },
    {   "ORXC",      0xC49,      assemble_type_ca    },
    {   "OREQ",      0xC4A,      assemble_type_ca    },
    {   "ORNE",      0xC4B,      assemble_type_ca    },
    {   "ORST",      0xC4C,      assemble_type_ca    },
    {   "ORCL",      0xC4D,      assemble_type_ca    },
    {   "ORSI",      0xC4E,      assemble_type_ca    },
    {   "ORCI",      0xC4F,      assemble_type_ca    },
    
    {   "XR3",       0xC50,      assemble_type_ca_3  }, // legacy
    {   "XR.LT",     0xC52,      assemble_type_ca    },
    {   "XR.GE",     0xC53,      assemble_type_ca    },
    {   "XR.LE",     0xC54,      assemble_type_ca    },
    {   "XR.GT",     0xC55,      assemble_type_ca    },
    {   "XR.SC",     0xC56,      assemble_type_ca    },
    {   "XR.XE",     0xC57,      assemble_type_ca    },
    {   "XR.SE",     0xC58,      assemble_type_ca    },
    {   "XR.XC",     0xC59,      assemble_type_ca    },
    {   "XR.EQ",     0xC5A,      assemble_type_ca    },
    {   "XR.NE",     0xC5B,      assemble_type_ca    },
    {   "XR.ST",     0xC5C,      assemble_type_ca    },
    {   "XR.CL",     0xC5D,      assemble_type_ca    },
    {   "XR.SI",     0xC5E,      assemble_type_ca    },
    {   "XR.CI",     0xC5F,      assemble_type_ca    },
    
    {   "XER",       0xC50,      assemble_type_ca_3  },
    {   "XRLT",      0xC52,      assemble_type_ca    },
    {   "XRGE",      0xC53,      assemble_type_ca    },
    {   "XRLE",      0xC54,      assemble_type_ca    },
    {   "XRGT",      0xC55,      assemble_type_ca    },
    {   "XRSC",      0xC56,      assemble_type_ca    },
    {   "XRXE",      0xC57,      assemble_type_ca    },
    {   "XRSE",      0xC58,      assemble_type_ca    },
    {   "XRXC",      0xC59,      assemble_type_ca    },
    {   "XREQ",      0xC5A,      assemble_type_ca    },
    {   "XRNE",      0xC5B,      assemble_type_ca    },
    {   "XRST",      0xC5C,      assemble_type_ca    },
    {   "XRCL",      0xC5D,      assemble_type_ca    },
    {   "XRSI",      0xC5E,      assemble_type_ca    },
    {   "XRCI",      0xC5F,      assemble_type_ca    },
    
    {   "ECR.LT",    0xC62,      assemble_type_ca    }, // legacy
    {   "ECR.GE",    0xC63,      assemble_type_ca    },
    {   "ECR.LE",    0xC64,      assemble_type_ca    },
    {   "ECR.GT",    0xC65,      assemble_type_ca    },
    {   "ECR.SC",    0xC66,      assemble_type_ca    },
    {   "ECR.XE",    0xC67,      assemble_type_ca    },
    {   "ECR.SE",    0xC68,      assemble_type_ca    },
    {   "ECR.XC",    0xC69,      assemble_type_ca    },
    {   "ECR.EQ",    0xC6A,      assemble_type_ca    },
    {   "ECR.NE",    0xC6B,      assemble_type_ca    },
    {   "ECR.ST",    0xC6C,      assemble_type_ca    },
    {   "ECR.CL",    0xC6D,      assemble_type_ca    },
    {   "ECR.SI",    0xC6E,      assemble_type_ca    },
    {   "ECR.CI",    0xC6F,      assemble_type_ca    },
    
    {   "ECRLT",     0xC62,      assemble_type_ca    },
    {   "ECRGE",     0xC63,      assemble_type_ca    },
    {   "ECRLE",     0xC64,      assemble_type_ca    },
    {   "ECRGT",     0xC65,      assemble_type_ca    },
    {   "ECRSC",     0xC66,      assemble_type_ca    },
    {   "ECRXE",     0xC67,      assemble_type_ca    },
    {   "ECRSE",     0xC68,      assemble_type_ca    },
    {   "ECRXC",     0xC69,      assemble_type_ca    },
    {   "ECREQ",     0xC6A,      assemble_type_ca    },
    {   "ECRNE",     0xC6B,      assemble_type_ca    },
    {   "ECRST",     0xC6C,      assemble_type_ca    },
    {   "ECRCL",     0xC6D,      assemble_type_ca    },
    {   "ECRSI",     0xC6E,      assemble_type_ca    },
    {   "ECRCI",     0xC6F,      assemble_type_ca    },
    
    {   "BR.NV",     0xD1,       assemble_type_b8    }, // legacy
    {   "BR.LT",     0xD2,       assemble_type_b8    },
    {   "BR.GE",     0xD3,       assemble_type_b8    },
    {   "BR.LE",     0xD4,       assemble_type_b8    },
    {   "BR.GT",     0xD5,       assemble_type_b8    },
    {   "BR.SC",     0xD6,       assemble_type_b8    },
    {   "BR.XE",     0xD7,       assemble_type_b8    },
    {   "BR.SE",     0xD8,       assemble_type_b8    },
    {   "BR.XC",     0xD9,       assemble_type_b8    },
    {   "BR.EQ",     0xDA,       assemble_type_b8    },
    {   "BR.NE",     0xDB,       assemble_type_b8    },
    {   "BR.ST",     0xDC,       assemble_type_b8    },
    {   "BR.CL",     0xDD,       assemble_type_b8    },
    {   "BR.SI",     0xDE,       assemble_type_b8    },
    {   "BR.CI",     0xDF,       assemble_type_b8    },
    
    {   "BRNV",      0xD1,       assemble_type_b8    },
    {   "BRLT",      0xD2,       assemble_type_b8    },
    {   "BRGE",      0xD3,       assemble_type_b8    },
    {   "BRLE",      0xD4,       assemble_type_b8    },
    {   "BRGT",      0xD5,       assemble_type_b8    },
    {   "BRSC",      0xD6,       assemble_type_b8    },
    {   "BRXE",      0xD7,       assemble_type_b8    },
    {   "BRSE",      0xD8,       assemble_type_b8    },
    {   "BRXC",      0xD9,       assemble_type_b8    },
    {   "BREQ",      0xDA,       assemble_type_b8    },
    {   "BRNE",      0xDB,       assemble_type_b8    },
    {   "BRST",      0xDC,       assemble_type_b8    },
    {   "BRCL",      0xDD,       assemble_type_b8    },
    {   "BRSI",      0xDE,       assemble_type_b8    },
    {   "BRCI",      0xDF,       assemble_type_b8    },
    
    {   "BIC.NV",    0xE1,       assemble_type_bc    }, // legacy
    {   "BIC.LT",    0xE2,       assemble_type_bc    },
    {   "BIC.GE",    0xE3,       assemble_type_bc    },
    {   "BIC.LE",    0xE4,       assemble_type_bc    },
    {   "BIC.GT",    0xE5,       assemble_type_bc    },
    {   "BIC.SC",    0xE6,       assemble_type_bc    },
    {   "BIC.XE",    0xE7,       assemble_type_bc    },
    {   "BIC.SE",    0xE8,       assemble_type_bc    },
    {   "BIC.XC",    0xE9,       assemble_type_bc    },
    {   "BIC.EQ",    0xEA,       assemble_type_bc    },
    {   "BIC.NE",    0xEB,       assemble_type_bc    },
    {   "BIC.ST",    0xEC,       assemble_type_bc    },
    {   "BIC.CL",    0xED,       assemble_type_bc    },
    
    {   "BCNV",      0xE1,       assemble_type_bc    },
    {   "BCLT",      0xE2,       assemble_type_bc    },
    {   "BCGE",      0xE3,       assemble_type_bc    },
    {   "BCLE",      0xE4,       assemble_type_bc    },
    {   "BCGT",      0xE5,       assemble_type_bc    },
    {   "BCSC",      0xE6,       assemble_type_bc    },
    {   "BCXE",      0xE7,       assemble_type_bc    },
    {   "BCSE",      0xE8,       assemble_type_bc    },
    {   "BCXC",      0xE9,       assemble_type_bc    },
    {   "BCEQ",      0xEA,       assemble_type_bc    },
    {   "BCNE",      0xEB,       assemble_type_bc    },
    {   "BCST",      0xEC,       assemble_type_bc    },
    {   "BCCL",      0xED,       assemble_type_bc    },
    
    {   "MARK",      0x00,       mark                },
    {   "DCHAR",     0x00,       dchar               },
    {   "DWIDE",     0x00,       dwide               },
    {   "DHALF",     0x00,       dhalf               },
    {   "DWORD",     0x00,       dword               },
    {   "DCSTR",     0x00,       dcstr               },
    {   "DC",        0x00,       dchar               },
    {   "DW",        0x00,       dwide               },
    {   "DH",        0x00,       dhalf               },
    {   "DL",        0x00,       dword               },
    {   "DCS",       0x00,       dcstr               },
    {   "ALIGN",     0x00,       align               },
};

int assemble_type_rr(struct parser_ctx *ctx, int opcode, int pass) {
    uint16_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    int s1 = read_reg_literal(ctx); if (s1 < 0) return s1;

    result |= opcode << 8;
    result |= (d1 & 0xF) << 4;
    result |= (s1 & 0xF);

    if (pass) {
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 2;

#ifdef TEST_PARSEVT
    char b0 = (result & 0xFF00) >> 8;
    char b1 = result & 0xFF;
    printf("Debug: RR %02hhx %02hhx\n", b0, b1);
#endif

    return 0;
}

int assemble_type_ext05_rr(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    int s1 = read_reg_literal(ctx); if (s1 < 0) return s1;

    result |= opcode << 16;
    result |= (d1 & 0xF) << 12;
    result |= (s1 & 0xF) << 8;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RR %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_ext05_ri(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    int s1 = read_reg_literal(ctx); if (s1 < 0) return s1;
    
    int literal_status = 0;
    int i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL,
        0xFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= opcode << 16;
    result |= (d1 & 0xF) << 12;
    result |= (s1 & 0xF) << 8;
    result |= (i1 & 0xFF);

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RI %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_e5_rm_st(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    int b1 = read_reg_literal(ctx); if (b1 < 0) return b1;
    int x1 = read_reg_literal(ctx); if (x1 < 0) return x1;
    int c1 = read_reg_literal(ctx); if (c1 < 0) return c1;

    result |= opcode << 16;
    result |= (d1 & 0xF) << 12;
    result |= (b1 & 0xF) << 8;
    result |= (x1 & 0xF) << 4;
    result |= (c1 & 0xF);

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RM %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_rr_cr(struct parser_ctx *ctx, int opcode, int pass) {
    uint16_t result = 0;

    int d1 = read_c_reg_literal(ctx); if (d1 < 0) return d1;
    int s1 = read_reg_literal(ctx); if (s1 < 0) return s1;

    result |= opcode << 8;
    result |= (d1 & 0xF) << 4;
    result |= (s1 & 0xF);

    if (pass) {
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 2;

#ifdef TEST_PARSEVT
    char b0 = (result & 0xFF00) >> 8;
    char b1 = result & 0xFF;
    printf("Debug: RR %02hhx %02hhx\n", b0, b1);
#endif

    return 0;
}

int assemble_type_none(struct parser_ctx *ctx, int opcode, int pass) {
    uint16_t result = 0;

    

    result |= opcode << 8;

    if (pass) {
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 2;

#ifdef TEST_PARSEVT
    char b0 = (result & 0xFF00) >> 8;
    char b1 = result & 0xFF;
    printf("Debug: RR %02hhx %02hhx\n", b0, b1);
#endif

    return 0;
}

int assemble_type_none_2(struct parser_ctx *ctx, int opcode, int pass) {
    uint16_t result = 0;

    

    result |= opcode;

    if (pass) {
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 2;

#ifdef TEST_PARSEVT
    char b0 = (result & 0xFF00) >> 8;
    char b1 = result & 0xFF;
    printf("Debug: RR %02hhx %02hhx\n", b0, b1);
#endif

    return 0;
}

int assemble_type_br(struct parser_ctx *ctx, int opcode, int pass) {
    uint16_t result = 0;

    int s1 = read_reg_literal(ctx); if (s1 < 0) return s1;

    result |= opcode << 8;
    result |= (s1 & 0xF);

    if (pass) {
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 2;

#ifdef TEST_PARSEVT
    char b0 = (result & 0xFF00) >> 8;
    char b1 = result & 0xFF;
    printf("Debug: BR %02hhx %02hhx\n", b0, b1);
#endif

    return 0;
}

int assemble_type_rm(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    
    uint64_t i1;
    int b1, x1;
    int address_status = read_address(
        ctx,
        &i1, 4095,
        &b1, &x1,
        pass
    );
    if (address_status) return address_status;

    result |= opcode << 24;
    result |= (d1 & 0xF) << 20;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= x1 >= 0 ? (x1 & 0xF) << 12 : 0;
    result |= i1 & 0xFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RM %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}


int assemble_type_rmb(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    
    uint64_t i1;
    int b1, x1;
    int address_status = read_address(
        ctx,
        &i1, 8191,
        &b1, &x1,
        pass
    );
    if (address_status) return address_status;

    i1 >>= 1;

    result |= opcode << 24;
    result |= (d1 & 0xF) << 20;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= x1 >= 0 ? (x1 & 0xF) << 12 : 0;
    result |= i1 & 0xFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RM %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_rm_lm(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    int c1 = read_reg_literal(ctx); if (c1 < 0) return c1;
    
    uint64_t i1;
    int b1, x1;
    int address_status = read_address(
        ctx,
        &i1, 4095,
        &b1, &x1,
        pass
    );
    if (x1 >= 0) return ERROR_NOT_ALLOWED;
    if (address_status) return address_status;

    result |= opcode << 24;
    result |= (d1 & 0xF) << 20;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= c1 >= 0 ? (c1 & 0xF) << 12 : 0;
    result |= i1 & 0xFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RM %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_rm_st(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    
    int c2 = read_reg_literal(ctx); if (c2 < 0) return c2;
    
    uint64_t i1;
    int b1, x1;
    int address_status = read_address(
        ctx,
        &i1, 255,
        &b1, &x1,
        pass
    );
    if (address_status) return address_status;

    result |= opcode << 24;
    result |= (d1 & 0xF) << 20;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= x1 >= 0 ? (x1 & 0xF) << 12 : 0;
    result |= i1 & 0xFF | ((c2 & 0xF) << 8);

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RM %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_rm_cr(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int d1 = read_c_reg_literal(ctx); if (d1 < 0) return d1;
    
    uint64_t i1;
    int b1, x1;
    int address_status = read_address(
        ctx,
        &i1, 4095,
        &b1, &x1,
        pass
    );
    if (address_status) return address_status;

    result |= opcode << 24;
    result |= (d1 & 0xF) << 20;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= x1 >= 0 ? (x1 & 0xF) << 12 : 0;
    result |= i1 & 0xFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RM %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_rm_stk(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    
    uint64_t i1 = 0;
    int b1 = 0, x1 = 2;

    result |= opcode << 24;
    result |= (d1 & 0xF) << 20;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= x1 >= 0 ? (x1 & 0xF) << 12 : 0;
    result |= i1 & 0xFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RM %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_rr_stk(struct parser_ctx *ctx, int opcode, int pass) {
    uint16_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    int s1 = 2;

    result |= opcode << 8;
    result |= (d1 & 0xF) << 4;
    result |= (s1 & 0xF);

    if (pass) {
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 2;

#ifdef TEST_PARSEVT
    char b0 = (result & 0xFF00) >> 8;
    char b1 = result & 0xFF;
    printf("Debug: RR %02hhx %02hhx\n", b0, b1);
#endif

    return 0;
}

int assemble_type_rm_alt(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    uint64_t i1 = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    int b1 = read_reg_literal(ctx); if (b1 < 0) return b1;
    int x1 = 0;
    
    int la_x1 = lookahead(ctx, 1); if (la_x1) return la_x1;
    if (!(get_event_type(ctx->event_buf)
        & (EVT_NUM_LITERAL | EVT_LABEL_REF | EVT_REG_LITERAL)))
        return ERROR_NOT_ALLOWED;
    if (get_event_type(ctx->event_buf) == EVT_REG_LITERAL) {
        x1 = read_reg_literal(ctx);
        if (x1 < 0) return x1;
    }

    int la_i1 = lookahead(ctx, 1); if (la_i1) return la_i1;
    if (get_event_type(ctx->event_buf) & (EVT_NUM_LITERAL | EVT_LABEL_REF)) {
        int literal_status = 0;
        i1 = read_num_literal_or_label_ref(
            ctx,
            EVT_NUM_LITERAL | EVT_LABEL_REF,
            0xFFF,
            &literal_status,
            pass
        );
        if (literal_status) return literal_status;
    }

    result |= opcode << 24;
    result |= (d1 & 0xF) << 20;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= x1 >= 0 ? (x1 & 0xF) << 12 : 0;
    result |= i1 & 0xFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RM %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_rb(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;
    
    uint64_t i1;
    int b1, x1;
    int address_status = read_address(
        ctx,
        &i1, 8191,
        &b1, &x1,
        pass
    );
    if (address_status) return address_status;
    
    i1 >>= 1;

    result |= opcode << 24;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= x1 >= 0 ? (x1 & 0xF) << 12 : 0;
    result |= i1 & 0xFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RB %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_rw(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    
    uint64_t i1;
    int b1, x1;
    int address_status = read_address(
        ctx,
        &i1, 65535,
        &b1, &x1,
        pass
    );
    if (address_status) return address_status;
    if (x1 >= 0) return ERROR_NOT_ALLOWED;

    result |= opcode << 24;
    result |= (d1 & 0xF) << 20;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= i1 & 0xFFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RW %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_rw_alt(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    uint64_t i1 = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    int b1 = 0;
    
    b1 = read_reg_literal(ctx);
    if (b1 < 0) return b1;

    int literal_status = 0;
    i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= opcode << 24;
    result |= (d1 & 0xF) << 20;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= i1 & 0xFFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RW %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_rw_lwi(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    uint64_t i1 = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    int b1 = 0;

    int literal_status = 0;
    i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= opcode << 24;
    result |= (d1 & 0xF) << 20;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= i1 & 0xFFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RW %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_bi(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    uint64_t i1 = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;

    int literal_status = 0;
    i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= opcode << 20;
    result |= (d1 & 0xF) << 16;
    result |= i1 & 0xFFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: BI %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_bi_alt(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    uint64_t i1 = 0;

    int d1 = 0;

    int literal_status = 0;
    i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= opcode << 20;
    result |= (d1 & 0xF) << 16;
    result |= i1 & 0xFFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: BI %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_b8(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    uint64_t i1 = 0;

    int c1 = read_reg_literal(ctx); if (c1 < 0) return c1;
    int c2 = read_reg_literal(ctx); if (c2 < 0) return c2;

    int literal_status = 0;
    i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= opcode << 24;
    result |= (c1 & 0xF) << 20;
    result |= (c2 & 0xF) << 16;
    result |= i1 & 0xFFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: B8 %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_bc(struct parser_ctx *ctx, int opcode, int pass) {
    uint64_t result = 0;

    uint64_t i2 = 0;
    uint64_t i1 = 0;

    int literal_status = 0;
    i2 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    long int c2 = read_reg_literal(ctx); if (c2 < 0) return c2;

    literal_status = 0;
    i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= ((uint64_t) opcode) << 40;
    result |= (c2 & 0xF) << 32;
    result |= (i1 & 0xFFFF) << 16;
    result |= (i2 & 0xFFFF) | (i2 & 0xF0000) << 20;
    

    if (pass) {
        fputc((result & 0xFF0000000000) >> 40, ctx->out);
        fputc((result & 0xFF00000000) >> 32, ctx->out);
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 6;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF0000000000) >> 40;
    char re1 = (result & 0xFF00000000) >> 32;
    char re2 = (result & 0xFF000000) >> 24;
    char re3 = (result & 0xFF0000) >> 16;
    char re4 = (result & 0xFF00) >> 8;
    char re5 = result & 0xFF;
    printf(
        "Debug: BC %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n", 
        re0, re1, re2, re3, re4, re5
    );
#endif

    return 0;
}

int assemble_type_bcasr(struct parser_ctx *ctx, int opcode, int pass) {
    uint64_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    
    int s1 = read_reg_literal(ctx); if (s1 < 0) return s1;
    
    uint64_t i1;
    int b1, x1;
    int literal_status, address_status = read_address(
        ctx,
        &i1, 4095,
        &b1, &x1,
        pass
    );
    if (address_status) return address_status;

    uint64_t i2 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= ((uint64_t)opcode) << 40;
    result |= (uint64_t)(d1 & 0xF) << 36;
    result |= b1 >= 0 ? (uint64_t)(b1 & 0xF) << 32 : 0;
    result |= x1 >= 0 ? (uint64_t)(x1 & 0xF) << 28 : 0;
    result |= (i1 & 0xFFF) << 16;
    result |= s1 >= 0 ? (s1 & 0xF) << 12 : 0;
    result |= (i2 & 0xFFF);

    if (pass) {
        fputc((result & 0xFF0000000000) >> 40, ctx->out);
        fputc((result & 0xFF00000000) >> 32, ctx->out);
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 6;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF0000000000) >> 40;
    char re1 = (result & 0xFF00000000) >> 32;
    char re2 = (result & 0xFF000000) >> 24;
    char re3 = (result & 0xFF0000) >> 16;
    char re4 = (result & 0xFF00) >> 8;
    char re5 = result & 0xFF;
    printf(
        "Debug: CS %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n", 
        re0, re1, re2, re3, re4, re5
    );
#endif

    return 0;
}

int assemble_type_ca(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int c1 = read_reg_literal(ctx); if (c1 < 0) return c1;
    int c2 = read_reg_literal(ctx); if (c2 < 0) return c2;
    
    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    int s1 = read_reg_literal(ctx); if (s1 < 0) return s1;
    int s2 = read_reg_literal(ctx); if (s2 < 0) return s2;

    result |= opcode << 20;
    result |= (c1 & 0xF) << 16;
    result |= (c2 & 0xF) << 12;
    result |= (d1 & 0xF) << 8;
    result |= (s1 & 0xF) << 4;
    result |= (s2 & 0xF);

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: CA %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_ca_3(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int c1 = 0;
    int c2 = 0;
    
    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;
    int s1 = read_reg_literal(ctx); if (s1 < 0) return s1;
    int s2 = read_reg_literal(ctx); if (s2 < 0) return s2;

    result |= opcode << 20;
    result |= (c1 & 0xF) << 16;
    result |= (c2 & 0xF) << 12;
    result |= (d1 & 0xF) << 8;
    result |= (s1 & 0xF) << 4;
    result |= (s2 & 0xF);

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: CA %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_wb(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;
    
    uint64_t i1;
    int b1, x1;
    int address_status = read_address(
        ctx,
        &i1, 65535,
        &b1, &x1,
        pass
    );
    if (address_status) return address_status;
    if (x1 >= 0) return ERROR_NOT_ALLOWED;

    result |= opcode << 24;
    result |= b1 >= 0 ? (b1 & 0xF) << 16 : 0;
    result |= i1 & 0xFFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: WB %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_ic(struct parser_ctx *ctx, int opcode, int pass) {
    uint16_t result = 0;
    
    int literal_status = 0;
    uint64_t i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL,
        0xFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= opcode << 8;
    result |= (i1 & 0xFF);

    if (pass) {
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 2;

#ifdef TEST_PARSEVT
    char b0 = (result & 0xFF00) >> 8;
    char b1 = result & 0xFF;
    printf("Debug: IC %02hhx %02hhx\n", b0, b1);
#endif

    return 0;
}

int assemble_type_ri_0(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;

    result |= ((opcode & 0xFF0) << 4) | ((opcode & 0xF));
    result |= (d1 & 0xF) << 4;

    if (pass) {
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 2;

#ifdef TEST_PARSEVT
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RR %02hhx %02hhx\n", re2, re3);
#endif

    return 0;
}

int assemble_type_ri_w(struct parser_ctx *ctx, int opcode, int pass) {
    uint32_t result = 0;

    uint64_t i1 = 0;

    int d1 = read_reg_literal(ctx); if (d1 < 0) return d1;

    int literal_status = 0;
    i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= ((opcode & 0xFF0) << 20) | ((opcode & 0xF) << 16);
    result |= (d1 & 0xF) << 20;
    result |= i1 & 0xFFFF;

    if (pass) {
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 4;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF000000) >> 24;
    char re1 = (result & 0xFF0000) >> 16;
    char re2 = (result & 0xFF00) >> 8;
    char re3 = result & 0xFF;
    printf("Debug: RI %02hhx %02hhx %02hhx %02hhx\n", re0, re1, re2, re3);
#endif

    return 0;
}

int assemble_type_ri_h(struct parser_ctx *ctx, int opcode, int pass) {
    uint64_t result = 0;

    uint64_t i1 = 0;

    uint64_t d1 = read_reg_literal(ctx); if (d1 < 0) return d1;

    int64_t l_opcode = opcode;

    int literal_status = 0;
    i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFFFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= ((l_opcode & 0xFF0) << 36) | ((l_opcode & 0xF) << 32);
    result |= (d1 & 0xF) << 36;
    result |= i1 & 0xFFFFFFFF;

    if (pass) {
        fputc((result & 0xFF0000000000) >> 40, ctx->out);
        fputc((result & 0xFF00000000) >> 32, ctx->out);
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 6;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF0000000000) >> 40;
    char re1 = (result & 0xFF00000000) >> 32;
    char re2 = (result & 0xFF000000) >> 24;
    char re3 = (result & 0xFF0000) >> 16;
    char re4 = (result & 0xFF00) >> 8;
    char re5 = result & 0xFF;
    printf(
        "Debug: RI %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
        re0, re1, re2, re3, re4, re5
    );
#endif

    return 0;
}

int assemble_type_ri_l(struct parser_ctx *ctx, int opcode, int pass) {
    uint64_t result = 0;

    uint64_t i1 = 0;

    uint64_t d1 = read_reg_literal(ctx); if (d1 < 0) return d1;

    int64_t l_opcode = opcode;

    int literal_status = 0;
    i1 = read_num_literal_or_label_ref(
        ctx,
        EVT_NUM_LITERAL | EVT_LABEL_REF,
        0xFFFFFFFFFFFF,
        &literal_status,
        pass
    );
    if (literal_status) return literal_status;

    result |= ((l_opcode & 0xFF0) << 52) | ((l_opcode & 0xF) << 48);
    result |= (d1 & 0xF) << 52;
    result |= i1 & 0xFFFFFFFFFFFF;

    if (pass) {
        fputc((result & 0xFF00000000000000) >> 56, ctx->out);
        fputc((result & 0xFF000000000000) >> 48, ctx->out);
        fputc((result & 0xFF0000000000) >> 40, ctx->out);
        fputc((result & 0xFF00000000) >> 32, ctx->out);
        fputc((result & 0xFF000000) >> 24, ctx->out);
        fputc((result & 0xFF0000) >> 16, ctx->out);
        fputc((result & 0xFF00) >> 8, ctx->out);
        fputc(result & 0xFF, ctx->out);
    }

    ctx->current_pc += 8;

#ifdef TEST_PARSEVT
    char re0 = (result & 0xFF00000000000000) >> 56;
    char re1 = (result & 0xFF000000000000) >> 48;
    char re2 = (result & 0xFF0000000000) >> 40;
    char re3 = (result & 0xFF00000000) >> 32;
    char re4 = (result & 0xFF000000) >> 24;
    char re5 = (result & 0xFF0000) >> 16;
    char re6 = (result & 0xFF00) >> 8;
    char re7 = result & 0xFF;
    printf(
        "Debug: RI %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx %02hhx\n",
        re0, re1, re2, re3, re4, re5, re6, re7
    );
#endif

    return 0;
}

int read_new_label_or_opcode(struct parser_ctx *ctx, int pass) {
    if (consume_whitespace(ctx->in) == -1) return EXPECTED_EOF;
    if (read_event_content(ctx->in, ctx->event_buf, MAX_EVENT_LEN) == -1)
        return ERROR_EVENT_TOO_LONG;
    
    int type = get_event_type(ctx->event_buf);
    
    if (type == EVT_NEW_LABEL) {
        if (!pass) {
            if (ctx->num_labels == MAX_LABELS) return ERROR_LABELS_FULL;
                
            int label_len = strlen(ctx->event_buf);
            if (label_len > MAX_LABEL_LEN) return ERROR_LABEL_TOO_LONG;
            
            memcpy(ctx->labels[ctx->num_labels], ctx->event_buf, label_len - 1);
            ctx->labels[ctx->num_labels][label_len - 1] = '\0';
            ctx->label_pcs[ctx->num_labels++] = ctx->current_pc;
        }
        return 0;
    }
    
    else if (type == EVT_OPCODE) {
        for (int i = 0; i < sizeof(opcodes) / sizeof(opcodes[0]); i++) {
            if (!strcmp(ctx->event_buf, opcodes[i].mnemonic))
                return opcodes[i].assemble(ctx, opcodes[i].opcode, pass);
        }
    }

    return ERROR_BAD_OPCODE;
}

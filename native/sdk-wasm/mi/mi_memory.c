#include <nitro/types.h>
#include <nitro/mi/memory.h>

#define HALFW_CONDAL  0xe0000000
#define HALFW_CONDNE  0x10000000
#define HALFW_CONDEQ  0x00000000

#define HALFW_OFF_PL  0x00800000
#define HALFW_OFF_MI  0x00000000
#define HALFW_LOAD    0x00100000
#define HALFW_STORE   0x00000000
#define HALFW_RN(n)   ((n) << 16)
#define HALFW_RD(n)   ((n) << 12)

#define HALFW_DEF1    0x004000B0
#define HALFW_DEF2    0x014000B0

#define HALFW_IMM(n)   (((n) & 0xf) | (((n) & 0xf0) << 4))

#define HALFW_DCD(cond, d, n, offset, sign, ldst, def) \
    dcd (def) | (cond) | (sign) | (ldst) | HALFW_RN(n) | HALFW_RD(d) | HALFW_IMM(offset)

#define LDRH_AD1(cond, d, n, offset) \

#define LDRH_AD2(cond, d, n, offset) \

#define LDRH_AD3(cond, d, n, offset) \

#define LDRH_AD4(cond, d, n, offset) \

#define STRH_AD1(cond, d, n, offset) \

#define STRH_AD2(cond, d, n, offset) \

#define STRH_AD3(cond, d, n, offset) \

#define STRH_AD4(cond, d, n, offset) \

#include <nitro/code32.h>

    #ifndef CW_BUG_FOR_LDRH_AND_STRH
    #else
    #endif
    #ifndef CW_BUG_FOR_LDRH_AND_STRH
    #else
    #endif
    #ifdef SP1P3_BUG_FOR_CONDITIONAL_ASM_INSTRUCTIONS
    #else
    #endif
    #ifndef CW_BUG_FOR_LDRH_AND_STRH
    #else
    #endif
    #ifdef SP1P3_BUG_FOR_CONDITIONAL_ASM_INSTRUCTIONS
    #else
    #endif
    #ifndef CW_BUG_FOR_LDRH_AND_STRH
        #ifdef SP1P3_BUG_FOR_CONDITIONAL_ASM_INSTRUCTIONS
        #else
        #endif
    #else
    #endif
    #ifdef SP1P3_BUG_FOR_CONDITIONAL_ASM_INSTRUCTIONS
    #else
    #endif
    #ifndef CW_BUG_FOR_LDRH_AND_STRH
    #else
    #endif
    #ifndef CW_BUG_FOR_LDRH_AND_STRH
    #else
    #endif

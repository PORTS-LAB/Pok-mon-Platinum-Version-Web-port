/* wasm-seam link stubs for BIOS/SVC calls. Semantics match hardware;
 * Halt traps (it must never return, same as on device). */
#include <stdlib.h>
#include <nitro/types.h>

void SVC_Halt(void) {
    abort();
}

s32 SVC_Div(s32 number, s32 denom) {
    return denom != 0 ? number / denom : 0;
}

s32 SVC_DivRem(s32 number, s32 denom) {
    return denom != 0 ? number % denom : 0;
}

u16 SVC_Sqrt(u32 src) {
    u32 r = 0, bit = 1u << 30;
    while (bit > src)
        bit >>= 2;
    while (bit) {
        if (src >= r + bit) {
            src -= r + bit;
            r = (r >> 1) + bit;
        } else {
            r >>= 1;
        }
        bit >>= 2;
    }
    return (u16)r;
}

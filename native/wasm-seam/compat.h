#pragma once
/*
 * wasm-seam compat header, injected via -include BEFORE any SDK/game header.
 * The SDK only declares SVC_* when SDK_ARM9/SDK_ARM7/SDK_TS are defined
 * (mwcc build flags). The wasm build defines none of those, so the
 * declarations are missing while system.h's OS_Halt() still calls SVC_Halt.
 * This header supplies exactly the missing declarations. Definitions live
 * in stubs.c. No SDK header is modified.
 */
#include <nitro/types.h>
/* ARM9 MMIO register map: plain address-cast macros, parse-safe on wasm.
 * The real build gets these via SDK_ARM9 code paths (which also drag in
 * mwcc-only syntax, so we include the register map directly instead). */
#include <nitro/hw/ARM9/ioreg.h>

#ifdef __cplusplus
extern "C" {
#endif

void SVC_Halt(void);
s32 SVC_Div(s32 number, s32 denom);
s32 SVC_DivRem(s32 number, s32 denom);
u16 SVC_Sqrt(u32 src);

/* Use the SDK's own portability hook for CLZ instead of its mwcc-only
 * `asm { clz }` block: MATH_CountLeadingZeros() maps to the builtin. */
#define PLATFORM_INTRINSIC_FUNCTION_BIT_CLZ32(x) __builtin_clz(x)

#ifdef __cplusplus
}
#endif

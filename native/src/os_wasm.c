#include "os_wasm.h"

#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* ---- minimal OS state ---- */
static volatile int g_vblank_pending = 0;
static void (*g_vblank_fn)(void) = 0;
static OSTick g_tick = 0;

void OS_Init(void) {
    /* no-op: hardware init is not applicable on wasm. */
}

void OS_InitTick(void) {
    g_tick = 0;
}

void OS_InitArena(void) {
    /* arenas are provided by the wasm heap; nothing to set up. */
}

void OS_InitIrqTable(void) {
}

void OS_SetIrqFunction(u32 intrBit, void (*func)(void)) {
    (void)intrBit;
    if (intrBit == 1u /* OS_IE_V_BLANK */) {
        g_vblank_fn = func;
    }
}

void OS_EnableIrqMask(u32 intrBit) {
    (void)intrBit;
}

void OS_EnableIrq(void) {
}

void OS_WaitIrq(BOOL clear, u32 intrBit) {
    (void)clear;
    (void)intrBit;
    /* Busy-wait is unacceptable on the web; the real loop is driven by
     * requestAnimationFrame in JS. This stub just yields once. */
#ifdef __EMSCRIPTEN__
    emscripten_sleep(0);
#endif
    g_vblank_pending = 0;
}

OSTick OS_GetTick(void) {
    return g_tick;
}

u16 OS_GetTickLo(void) {
    return (u16)(g_tick & 0xFFFF);
}

void OSi_WaitVCount0(void) {
}

void *OS_AllocFromMainArenaLo(u32 size, u32 align) {
    if (align == 0)
        align = 1;
    void *p = malloc(size + align);
    if (!p)
        return 0;
    /* align up */
    uintptr_t a = (uintptr_t)p;
    uintptr_t aligned = (a + align - 1) & ~(align - 1);
    if (aligned == a)
        return p;
    /* leak the unaligned prefix (acceptable for a boot probe) */
    return (void *)aligned;
}

/* ---- exported boot + frame entry ---- */
void plat_os_boot(void) {
    hal_init(0, 0);
    /* NitroMain is the decomp's entry; calling it directly runs its
     * InitSystem/InitVRAM/... then enters while(TRUE). We do NOT call it
     * here because it never returns; instead JS calls plat_os_run_frame. */
}

void plat_os_run_frame(void) {
    g_tick++;
    g_vblank_pending = 1;
    if (g_vblank_fn)
        g_vblank_fn();
}

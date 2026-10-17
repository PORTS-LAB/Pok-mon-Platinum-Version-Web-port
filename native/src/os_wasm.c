#include "os_wasm.h"

#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

/* undef the SDK's static-inline arena allocators so we can provide bodies */
#ifdef OS_AllocFromMainArenaLo
#undef OS_AllocFromMainArenaLo
#endif
#ifdef OS_AllocFromMainArenaHi
#undef OS_AllocFromMainArenaHi
#endif

/* ---- wasm-global system work (replaces HW_MAIN_MEM_SYSTEM) ---- */
static OSSystemWork g_systemWork;

OSSystemWork *plat_os_system_work(void) {
    return &g_systemWork;
}

/* ---- minimal OS state ---- */
static volatile int g_vblank_pending = 0;
static void (*g_vblank_fn)(void) = 0;
static OSTick g_tick = 0;

void OS_Init(void) {
    /* Hardware init (PXI, MI, CTRDG, CARD, PM, VCount0) are no-ops on wasm. */
    memset(&g_systemWork, 0, sizeof(g_systemWork));
    /* sensible defaults */
    memset(&g_systemWork, 0, sizeof(g_systemWork));
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

void *plat_os_alloc_main_arena_lo(u32 size, u32 align) {
    if (align == 0)
        align = 1;
    void *p = malloc(size + align);
    if (!p)
        return 0;
    uintptr_t a = (uintptr_t)p;
    uintptr_t aligned = (a + align - 1) & ~(align - 1);
    if (aligned == a)
        return p;
    return (void *)aligned;
}

void *OS_AllocFromMainArenaLo(u32 size, u32 align) {
    return plat_os_alloc_main_arena_lo(size, align);
}

void *OS_AllocFromMainArenaHi(u32 size, u32 align) {
    return OS_AllocFromMainArenaLo(size, align);
}

void *OS_AllocFromArenaLo(u32 id, u32 size, u32 align) {
    (void)id;
    return OS_AllocFromMainArenaLo(size, align);
}

void *OS_AllocFromArenaHi(u32 id, u32 size, u32 align) {
    (void)id;
    return OS_AllocFromMainArenaLo(size, align);
}

void *OS_GetArenaLo(u32 id) {
    (void)id;
    return 0;
}

void *OS_GetArenaHi(u32 id) {
    (void)id;
    return (void *)0x1000000;
}

void OS_SetArenaLo(u32 id, void *p) {
    (void)id;
    (void)p;
}

void OS_SetArenaHi(u32 id, void *p) {
    (void)id;
    (void)p;
}

void OS_InitLock(void) {
}

void OS_InitThread(void) {
}

void OS_InitVAlarm(void) {
}

void OS_InitException(void) {
}

void OS_InitReset(void) {
}

void OS_GetLowEntropyData(u32 buffer[8]) {
    buffer[0] = (u32)((0 << 16) | OS_GetTickLo());
    buffer[1] = 0;
    buffer[2] = 0;
    buffer[3] = 0;
    buffer[4] = 0;
    buffer[5] = 0;
    buffer[6] = 0;
    buffer[7] = 0;
}

void OS_ResetSystem(int resetParam) {
    (void)resetParam;
}

int OS_GetResetParameter(void) {
    return 0; /* RESET_CLEAN */
}

void OS_DisableIrq(void) {
}

void OS_DisableIrqMask(u32 intrBit) {
    (void)intrBit;
}

u32 OS_GetIrqMask(void) {
    return 0;
}

void OS_RestoreInterrupts(u32 mask) {
    (void)mask;
}

u32 OS_DisableInterrupts(void) {
    return 0;
}

void plat_os_boot(void) {
    hal_init(0, 0);
}

void plat_os_run_frame(void) {
    g_tick++;
    g_vblank_pending = 1;
    if (g_vblank_fn)
        g_vblank_fn();
}

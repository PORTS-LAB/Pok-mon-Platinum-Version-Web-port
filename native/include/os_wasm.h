#ifndef PLAT_OS_WASM_H
#define PLAT_OS_WASM_H
/*
 * OS shim for the wasm port. Implements the NitroSDK OS entry points that
 * NitroMain (src/main.c) calls directly, backed by hal.h + Emscripten.
 * Each function here is a deliberate, documented simplification of the
 * hardware behaviour; the goal is "boots NitroMain to its first frame",
 * not bit-exact DS timing.
 */
#include "hal.h"
#include <stddef.h>
#include <stdint.h>

#define u8  unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define s32 int
#define OSTick unsigned long long
#define BOOL int
#define TRUE 1
#define FALSE 0

/* Forward-declare OSSystemWork without pulling the SDK's arena.h (which
 * declares OS_AllocFromMainArenaLo as a static inline, conflicting with
 * our implementation). */
typedef struct { char _pad[0x400]; } OSSystemWork;
OSSystemWork *plat_os_system_work(void);

/* Override OS_GetSystemWork() macro from SDK to return our wasm global. */
#ifdef OS_GetSystemWork
#undef OS_GetSystemWork
#endif
#define OS_GetSystemWork() plat_os_system_work()

/* VBlank state, polled by OS_WaitIrq and the main loop. */
void OS_Init(void);
void OS_InitTick(void);
void OS_InitArena(void);
void OS_InitIrqTable(void);
void OS_SetIrqFunction(u32 intrBit, void (*func)(void));
void OS_EnableIrqMask(u32 intrBit);
void OS_EnableIrq(void);
void OS_WaitIrq(BOOL clear, u32 intrBit);
OSTick OS_GetTick(void);
u16 OS_GetTickLo(void);

/* VBlank callback plumbing used by GX_VBlankIntr / gSystem.vblankCallback. */
void OSi_WaitVCount0(void);

/* OS_AllocFromMainArenaLo is a static inline in the SDK header; the
 * implementation above is the wasm body. Rename to avoid the conflict. */
#ifdef OS_AllocFromMainArenaLo
#undef OS_AllocFromMainArenaLo
#endif
#define OS_AllocFromMainArenaLo(size, align) plat_os_alloc_main_arena_lo(size, align)

#ifdef OS_AllocFromMainArenaHi
#undef OS_AllocFromMainArenaHi
#endif
#define OS_AllocFromMainArenaHi(size, align) plat_os_alloc_main_arena_lo(size, align)

void *plat_os_alloc_main_arena_lo(u32 size, u32 align);

/* Main-loop entry: exported to JS. */
void plat_os_run_frame(void);

/* Called once from JS after the module loads. */
void plat_os_boot(void);

#endif

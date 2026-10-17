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
#define u8  unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define s32 int
#define OSTick unsigned long long
#define BOOL int
#define TRUE 1
#define FALSE 0

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

/* Heap stubs used by InitSystem (OS_AllocFromMainArenaLo). */
void *OS_AllocFromMainArenaLo(u32 size, u32 align);

/* Main-loop entry: exported to JS. */
void plat_os_run_frame(void);

/* Called once from JS after the module loads. */
void plat_os_boot(void);

#endif

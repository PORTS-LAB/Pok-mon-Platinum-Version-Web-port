#include <nitro/code32.h>
#include <nitro/types.h>
#include <nitro/os/common/interrupt.h>
#include <nitro/os/common/thread.h>
#include <nitro/os/common/systemCall.h>

#ifdef SDK_ARM9
    #include <nitro/hw/ARM9/mmap_global.h>
    #include <nitro/hw/ARM9/ioreg_OS.h>
#else
    #include <nitro/hw/ARM7/mmap_global.h>
    #include <nitro/hw/ARM7/ioreg_OS.h>
#endif

#ifdef SDK_ARM9
    #include <nitro/dtcm_begin.h>
#endif

#ifndef SDK_THREAD_INFINITY
    OSThreadQueue OSi_IrqThreadQueue = 0;
#else
    OSThreadQueue OSi_IrqThreadQueue = { NULL, NULL };
#endif

#ifdef SDK_ARM9
    #include <nitro/dtcm_end.h>
    #include <nitro/itcm_begin.h>
#endif

#ifdef SDK_ARM9
    #include <nitro/itcm_end.h>
#endif

void OS_WaitIrq (BOOL clear, OSIrqMask irqFlags)
{
#ifdef SDK_NO_THREAD
    OS_WaitInterrupt(clear, irqFlags);
#else
    OSIntrMode enabled = OS_DisableInterrupts();

    if (clear) {
        (void)OS_ClearIrqCheckFlag(irqFlags);
    }

    (void)OS_RestoreInterrupts(enabled);

    while (!(OS_GetIrqCheckFlag() & irqFlags)) {
        OS_SleepThread(&OSi_IrqThreadQueue);
    }
#endif
}

void OS_WaitAnyIrq (void)
{
#ifdef SDK_NO_THREAD
    OS_Halt();
#else
    OS_SleepThread(&OSi_IrqThreadQueue);
#endif
}

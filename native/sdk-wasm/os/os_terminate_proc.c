#include <nitro/os/common/system.h>

SDK_WEAK_SYMBOL void OS_Terminate(void)
{
    while (TRUE) {
        OS_DisableInterrupts();
        OS_Halt();
    }
}

#include <nitro/code32.h>

#include <nitro/codereset.h>

SDK_WEAK_SYMBOL void OS_Exit(int status)
{
#ifdef SDK_FINALROM
#pragma unused(status)
#endif
    OS_DisableInterrupts();
    OS_Printf("\n" OS_EXIT_STRING, status);
    OS_Terminate();
}

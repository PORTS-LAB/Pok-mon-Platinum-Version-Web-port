#include <nitro/os.h>
#include <nitro/memorymap.h>

#ifdef SDK_ARM9
    #include <nitro/cp/context.h>
#endif

#include <nitro/code32.h>

#include <nitro/codereset.h>

void OS_DumpContext (OSContext * context)
{
#ifndef SDK_FINALROM
    s32 i;

    OS_Printf("context=%08x\n", context);
    if (context) {
        OS_Printf("CPSR  %08x\n", context->cpsr);
        for (i = 0; i < 13; i++) {
            OS_Printf("R%02d   %08x\n", i, context->r[i]);
        }
        OS_Printf("SP    %08x\n", context->sp);
        OS_Printf("LR    %08x\n", context->lr);
        OS_Printf("PC+4  %08x\n", context->pc_plus4);
#ifdef  SDK_CONTEXT_HAS_SP_SVC
        OS_Printf("SPsvc %08x\n", context->sp_svc);
#endif
    }
#else
    (void)context;
#endif
}

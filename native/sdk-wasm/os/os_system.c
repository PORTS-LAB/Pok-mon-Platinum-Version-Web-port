#include <nitro/os.h>
#include <nitro/code32.h>

OSIntrMode OS_EnableInterrupts (void)
{
    return 0;
}

OSIntrMode OS_DisableInterrupts (void)
{
    return 0;
}

OSIntrMode OS_RestoreInterrupts (OSIntrMode state)
{
    (void)state;
    return 0;
}

OSIntrMode OS_EnableInterrupts_IrqAndFiq (void)
{
    return 0;
}

OSIntrMode OS_DisableInterrupts_IrqAndFiq (void)
{
    return 0;
}

OSIntrMode OS_RestoreInterrupts_IrqAndFiq (OSIntrMode state)
{
    (void)state;
    return 0;
}

OSIntrMode_Irq OS_GetCpsrIrq (void)
{
    return 0;
}

OSProcMode OS_GetProcMode (void)
{
    return OS_PROCMODE_SVC;
}

#include <nitro/codereset.h>

#ifdef SDK_ARM9
    #include <nitro/code32.h>
    void OS_SpinWait (u32 cycle)
    {
        SVC_WaitByLoop((s32)cycle / 4);
    }
    #include <nitro/codereset.h>
#else
    void OS_SpinWait (u32 cycle)
    {
        SVC_WaitByLoop((s32)cycle / 4);
    }
#endif

void OS_WaitInterrupt (BOOL clear, OSIrqMask irqFlags)
{
    OSIntrMode cpsrIrq = OS_DisableInterrupts();
    BOOL ime = OS_EnableIrq();

    if (clear) {
        (void)OS_ClearIrqCheckFlag(irqFlags);
    }

    while (!(OS_GetIrqCheckFlag() & irqFlags)) {
        OS_Halt();
        (void)OS_EnableInterrupts();
        (void)OS_DisableInterrupts();
    }

    (void)OS_ClearIrqCheckFlag(irqFlags);
    (void)OS_RestoreIrq(ime);
    (void)OS_RestoreInterrupts(cpsrIrq);
}

void OS_WaitVBlankIntr (void)
{
    SVC_WaitByLoop(1);
#if defined(SDK_ENABLE_ARM7_PRINT) && defined(SDK_ARM9)

    OS_PrintServer();
#endif
    OS_WaitIrq(TRUE, OS_IE_V_BLANK);
}
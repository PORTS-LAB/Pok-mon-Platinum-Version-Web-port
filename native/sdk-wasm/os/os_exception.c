#include <nitro/hw/common/armArch.h>
#include <nitro/memorymap.h>
#include <nitro/os.h>

#ifdef SDK_ARM9
    #define OSi_ExPrintf OS_Printf
#else
    #define OSi_ExPrintf OS_Printf
#endif

static void OSi_DisplayExContext(void);

typedef struct {
    OSContext context;
    u32 cp15;
    u32 spsr;
    u32 exinfo;
    u32 debug[4];
} OSiExContext;

static OSiExContext OSi_ExContext;
static OSExceptionHandler OSi_UserExceptionHandler;
static void * OSi_UserExceptionHandlerArg;
static void * OSi_DebuggerHandler = NULL;

#include <nitro/code32.h>

#include <nitro/codereset.h>

#define HW_EXCP_VECTOR_BUF_FOR_DEBUGGER  0x027ffd9c

void OS_InitException (void)
{
    if (0x2600000 <= *(u32 *)HW_EXCP_VECTOR_BUF_FOR_DEBUGGER
        && *(u32 *)HW_EXCP_VECTOR_BUF_FOR_DEBUGGER < 0x2800000) {
        OSi_DebuggerHandler = *(void **)HW_EXCP_VECTOR_BUF_FOR_DEBUGGER;
    } else {
        OSi_DebuggerHandler = NULL;
    }

    if (!OSi_DebuggerHandler) {
        *(u32 *)(HW_EXCP_VECTOR_BUF_FOR_DEBUGGER) = 0;
        *(u32 *)(HW_EXCP_VECTOR_BUF) = 0;
    }

    OSi_UserExceptionHandler = NULL;
}

#if defined(SDK_ARM9)
    #include <nitro/code32.h>

    static u32 OSi_ExceptionHookStack[8];


    #include <nitro/codereset.h>

    void OS_EnableUserExceptionHandlerOnDebugger (void)
    {
        if (OSi_DebuggerHandler) {
            *(u32 *)(HW_EXCP_VECTOR_BUF_FOR_DEBUGGER) = 0;
        }
    }
#endif

void OS_SetUserExceptionHandler (OSExceptionHandler handler, void * arg)
{
    OSi_UserExceptionHandler = handler;
    OSi_UserExceptionHandlerArg = arg;
}

#include <nitro/code32.h>

static void OSi_DisplayExContext (void)
{
#ifndef SDK_FINALROM
    int i;

    OSi_ExPrintf("**** Exception Occurred ****\n");

    for (i = 0; i < 13; i++) {
        OSi_ExPrintf("R%02d=%08X  %c", i, OSi_ExContext.context.r[i], ((i & 3) == 3) ? '\n' : ' ');
    }

    OSi_ExPrintf("SP =%08X   ", OSi_ExContext.context.sp);
    OSi_ExPrintf("LR =%08X   ", OSi_ExContext.context.lr);
    OSi_ExPrintf("PC =%08X\n", OSi_ExContext.context.pc_plus4);

#ifdef SDK_ARM9
    OSi_ExPrintf("  CPSR=%08X  SPSR=%08X  CP15=%08X\n",
                 OSi_ExContext.context.cpsr, OSi_ExContext.spsr, OSi_ExContext.cp15);
#else
    OSi_ExPrintf("  CPSR=%08X  SPSR=%08X\n", OSi_ExContext.context.cpsr, OSi_ExContext.spsr);
#endif
    OSi_ExPrintf("\n\n");
#endif
    if (OSi_UserExceptionHandler) {
    }
}

#include <nitro/codereset.h>

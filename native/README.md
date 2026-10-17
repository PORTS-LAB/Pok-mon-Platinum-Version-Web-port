# Native web port — full step-by-step roadmap

Status: **compilation solved** (1012/1012 upstream `.c` files build to wasm).
What remains is the **runtime**, in this order. Each step is a separate,
verifiable milestone. Do not start step N+1 until step N's verification passes.

## Step 1 — OS / timing  (in progress: `native/src/os_wasm.c`)

Functions to implement behind `hal.h`:
- `OS_Init`, `OS_InitTick`, `OS_GetTick`, `OS_GetTickLo`
- `OS_InitArena`, `OS_SetArenaLo/Hi`, `OS_GetArenaLo/Hi`, `OS_AllocFromArenaLo/Hi`
- `OS_InitIrqTable`, `OS_SetIrqFunction`, `OS_EnableIrqMask`, `OS_EnableIrq`, `OS_WaitIrq`
- `OS_InitException`, `OS_InitThread`, `OS_InitVAlarm`, `OS_InitReset`, `OS_InitLock`
- `OS_GetLowEntropyData`, `OS_ResetSystem`, `OS_GetResetParameter`

Verification: `NitroMain` runs `InitSystem()` without trapping; vblank callback fires.

Implementation notes:
- VBlank is driven by `requestAnimationFrame` in JS; `OS_WaitIrq` yields via
  `emscripten_sleep(0)` instead of spinning.
- Tick counter increments per frame (60 Hz).
- Arena = one wasm linear-memory region; allocators bump-pointer.
- PXI/CTRDG/CARD/PM are no-ops on wasm (no cartridge, no power mgmt).

## Step 2 — Heap  (`src/heap.c`)

- `Heap_InitSystem` already calls `OS_AllocFromMainArenaLo` (bump allocator) —
  works once Step 1's allocator works.
- `Heap_Alloc`/`Heap_Free`/`Heap_FreeExplicit` route through NNS Fnd heaps
  (NNS_FndCreateExpHeap). Compile NNS FND sources to wasm.

Verification: `Heap_Alloc(HEAP_ID_SYSTEM, 64)` returns non-null; no trap.

## Step 3 — FS + overlay loader

- `FS_Init(default_dma_no)` — no-op DMA; set up FS archive.
- `FS_LoadTable` — read the FAT/FNT from the bundled NARC (IDBFS).
- `FS_OpenFile`/`FS_ReadFile`/`FS_CloseFile` — file I/O against the NARC.
- `Overlay_LoadByID` / `Overlay_UnloadByID` — overlay swap (no-op on wasm
  since all code is already linked; just call the init/exit hooks).
- `FS_EXTERN_OVERLAY` macros — resolve to function pointers.

Verification: `FS_OpenFile(&file, "data/...")` succeeds; `InitApplication`
enqueues `game_opening`.

## Step 4 — FX + GX (2D renderer)

- `FX_Init` — no-op (table is const, already in SDK).
- `GX_Init`, `GX_SetPower`, `GX_SetBankForLCDC`, `GX_InitEx`.
- `GX_DispOff`/`GX_DispOn`, `GX_SetDispSelect`.
- `GX_VBlankIntr(TRUE)` — registers the vblank IRQ callback.
- `MI_CpuClearFast`/`MI_CpuFillFast`/`MI_CpuCopyFast` — memset/memcpy.
- `G2`/`G3` 2D background + sprite + window + palette registers.
- `GX` VRAM → WebGL texture upload (new: GL blit in `hal_web.c`).

Verification: first frame renders the title-screen background to the canvas.

## Step 5 — Sound

- `NNS_SndInit` — WebAudio context + worklet.
- `NNS_SndArcInit` — load `pl_sound_data.sdat` from IDBFS.
- `SoundSystem_Init`, `SoundSystem_Tick`, `Sound_PlayEffect`, `Sound_PlayBGM`.
- Mic: `getUserMedia` RMS → `SVC_MicWriteSamples` (already stubbed).

Verification: title-screen BGM plays; button SFX on input.

## Step 6 — Application manager + overlays

- `ApplicationManager_New` / `_Exec` / `_Free`.
- `EnqueueApplication` → `game_opening` → title screen.
- Overlay init/exit hooks (already linked).

Verification: title screen renders and responds to input.

## Step 7 — Polish

- Touch input → `ReadKeypadAndTouchpad` (already wired via `hal_input_poll`).
- Save: `SaveData_Init` → IDBFS `platinum.sav`.
- Performance: frame pacing, asset streaming.
- Full game playthrough test.

## What's already done (committed, pushed)

- `native/include/hal.h` — HAL interface (video, audio, input, fs, save, mic).
- `native/include/os_wasm.h` + `native/src/os_wasm.c` — OS shim scaffold.
- `native/wasm-seam/` — seam recipe: 1012/1012 files compile to wasm.
- `native/Makefile` — `make wasm` (emsdk) produces `dist-native/platinum.html`.
- `native/src/hal_web.c` — WebGL blit + keyboard + IDBFS saves + mic.
- `native/src/hal_host.c` — host smoke test (gcc, 120 frames verified).
- `native/tools/extract_assets.py` — NARC manifest from owned ROM.
- `native/third_party/fetch-decomp.sh` — build-time decomp fetch.

## The honest bottom line

Steps 1–7 are the project. Step 1 (OS) is started. Each step is a
separate, verifiable milestone; I'll build them in order so each commit
is independently checkable. The emulator build at repo root remains the
playable path today.
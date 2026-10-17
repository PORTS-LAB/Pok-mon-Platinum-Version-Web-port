# Reproducing the matching build (no sudo, no pip)

All user-space. Verified 2026-10-16 on Arch Linux (Omarchy), x86-64.

## 1. Drivers (build/, added to PATH first)

| Tool | Source |
|---|---|
| `meson` 1.12.1 | wheel from PyPI unpacked to `pymods/`, run via wrapper (`meson` script) — Python 3.14 has no working pip/ensurepip |
| `ninja` 1.12.1 | GitHub release binary |
| `nasm` 2.16.03 | built from source (`./configure --prefix=$HOME/.local && make -j16`) |
| `wget` | shim script → `curl -sL -o` (only `-O`+URL usage needed) |

## 2. ARM toolchain (the hard part)

`pokeplatinum` needs `arm-none-eabi-gcc` + `as`/`objcopy` that accept
`-march=armv4t`. Findings, in order:

1. ARM's official CDN tarball: **too slow, times out** (curl exit 28).
2. Arch `arm-none-eabi-gcc` package: **truncated mirrors**, and Arch's
   binutils 2.47 snapshot **removed armv4t** (`invalid -march= option`).
3. **Working combo**: Ubuntu `gcc-arm-none-eabi 13.2.rel1-2` .deb +
   Ubuntu `binutils-arm-none-eabi 2.42` .deb (kernel.org mirror),
   extracted with `ar x` + `tar xf` (no dpkg needed).

Three integration traps and fixes (all in `build/` scripts area):

- The gcc driver calls bare `as`, which resolved to the **host x86**
  assembler → `arm-none-eabi-gcc` was replaced with a wrapper adding
  `-B arm-prefix/`, where `arm-prefix/as` → the ARM assembler. Host
  builds are unaffected (verified: both combos compile).
- `make_script_bin.sh` invokes the driver by **absolute path**, so the
  fix had to live *at* that path, not just in PATH.
- Host `gcc -m32` is needed for any x86 baseline comparisons (the SDK's
  struct-packing asserts assume 32-bit; wasm32 satisfies them, x86-64
  does not — evidence wasm is the faithful target).

## 3. Build

```bash
git clone --depth 1 https://github.com/pret/pokeplatinum
cd pokeplatinum && make   # auto-fetches metroskrew + SDK subprojects
sha1sum build/pokeplatinum.us.nds
# 0862ec35b24de5c7e2dcb88c9eea0873110d755c9  == upstream rev1, bit-matching
```

## 4. wasm-seam (this dir)

Compile the decomp to WebAssembly without touching the decomp. Files here:

| File | Role |
|---|---|
| `compat.h` | SVC decls + ARM9 MMIO map + CLZ portability hook (`-include`, before SDK) |
| `stubs.c` | `SVC_Halt`/`Div`/`DivRem`/`Sqrt` implementations, hardware-accurate |
| `shim/extras.h` | shadows proprietary MSL `<extras.h>` (would poison wasm libc); provides `time_t` |
| `shim/link_contest_records.h` | shadows upstream header that **reuses contest.h's include guard** |
| `game_protos.h` | prototypes upstream declares implicitly (mwcc tolerates, clang does not) |
| `gen_patches.py` | mechanical TU patches: drops `static` where upstream declares non-static |
| `test_ascii.c` + `seam-check.sh` | known-answer test: emcc/wasm under node vs host `gcc -m32` |

### Status (verified 2026-10-17)

- `src/ascii_util.c`: wasm vs `-m32` output **IDENTICAL**.
- Top-level `src/*.c`: **334 / 334** compile to wasm objects.
- Whole tree (`src/**/*.c`, 1012 upstream-compiled files): **1012 / 1012**.
  `-fdeclspec` + `gen_patches.py` close most of the remainder.

### Compiler flags that matter

The real build's `-D` set is mirrored (`SDK_ARM9 SDK_CW SDK_TS SDK_4M …`
`GAME_VERSION=VERSION_PLATINUM GAME_LANGUAGE=LANGUAGE_ENGLISH NNS_FINALROM
PM_KEEP_ASSERTS`) — required, since SDK headers branch on them. Plus:

- `-D_NITRO` — puts GameSpy `nonport.h` on the same branch the DS build takes.
- `-include include/pch/global_pch.h` — the decomp's implicit header (`ALIGN_4`,
  `NELEMS`, `GF_ASSERT`, …). Without it 146 files fail.
- `-fdeclspec` — `__declspec` appears in SDK headers.
- `-Wno-int-conversion` — mwcc tolerates pointer/int sloppiness clang warns on.
- `PLATFORM_INTRINSIC_FUNCTION_BIT_CLZ32` → `__builtin_clz`: uses the SDK's own
  portability hook to bypass its mwcc-only `asm { clz }`.

### Traps found (each cost a round-trip; documented so they cost nothing next time)

1. `link_contest_records.h` and `contest.h` share guard `POKEPLATINUM_CONTEST_H`
   → including both silently empties one. Fixed by shadowing the former.
2. `poffin.h` nests an anonymous struct inside an anonymous union (mwcc
   extension) → clang exposes only the union. Fixed via a byte-identical
   named view (`POFFIN_FIELDS`/`POFFIN_OBJ`).
3. `overlay_manager.c` assigns a struct containing a `const` member →
   clang rejects; memcpy is the exact equivalent.
4. `src/unk_02095CD4.c` is an **orphan**: present in `src/` but absent from
   `build.ninja`, i.e. not compiled upstream. Excluded, not patched.
5. Struct-packing asserts in the SDK assume 32-bit pointers → x86-64 host
   baseline fails them, wasm32 does not. wasm is the faithful target; use
   `gcc -m32` for host comparisons.
6. SVC declarations come from the shim (the SDK declares them only under
   mwcc-flag combos); the MMIO map is included directly (`hw/ARM9/ioreg.h`)
   to avoid mwcc-only syntax pulled in by the `SDK_ARM9` code paths.

## 5. What is left for a *playable* native port

Compiling is solved. The remaining work is implementing, behind `hal.h`,
what the objects still reference — measured with `llvm-nm --undefined-only`
over the compiled set:

| Layer | Scope |
|---|---|
| NitroSDK OS/timing/DMA/IRQ | timers, vblank, interrupts, DMA |
| GX 2D (backgrounds, sprites, windows, affine) | the 2D renderer |
| GX 3D (geometry engine, rasterizer) | Distortion World, battle 3D |
| FS + overlay loader | NARC reads, overlay swap |
| GXSND | sound streams, banks, mic |
| Wifi/DWC | stub (no online play in a web port) |

Each is a HAL implementation behind the same interface `native/include/hal.h`
already defines, driven from the decomp's own entry points rather than emulated.

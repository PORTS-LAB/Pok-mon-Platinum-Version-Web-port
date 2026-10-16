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

`compat.h` + `stubs.c` + `test_ascii.c` + `seam-check.sh`: compile a real
decomp TU with emcc, run under node, diff against the `-m32` baseline.
Proven IDENTICAL for `src/ascii_util.c`. Seam notes:

- Real build `-D` set mirrored (SDK_ARM9/SDK_CW/SDK_TS/…): faithful, and
  required (SDK headers branch on them).
- `PLATFORM_INTRINSIC_FUNCTION_BIT_CLZ32` → `__builtin_clz`: uses the
  SDK's own portability hook to skip the mwcc-only `asm { clz }` block.
- SVC declarations come from the shim (SDK declares them only under
  mwcc-flag combos); MMIO map included directly (`hw/ARM9/ioreg.h`) to
  avoid mwcc-only syntax pulled in by the `SDK_ARM9` code paths.

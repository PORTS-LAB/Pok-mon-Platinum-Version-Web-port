# True native port — project scaffold (not yet playable)

This directory is the honest start of a native WASM port: the game logic
compiled to WebAssembly instead of emulating the DS. Status: **scaffold +
verified tooling. No playable build yet.** See "What remains" below.

## Layout

```
native/
  include/hal.h        Hardware Abstraction Layer: the ONLY surface the game
                       core may touch (video, audio, input, fs, save, mic).
  src/main.c           Boot + frame loop. Calls game_step() each frame.
  src/core_stub.c      Stand-in game core (test pattern). The decomp core
                       replaces this file.
  src/hal_host.c       Host HAL (Linux/macOS): proves the interface compiles
                       and runs. No graphics/audio output, just counters.
  src/hal_web.c        Emscripten HAL: WebGL blit + keyboard input +
                       IDBFS saves. Needs emcc (not vendored).
  tools/extract_assets.py  Reads YOUR owned .nds, emits assets/manifest.json
                       (paths/offsets/sizes/hashes only — no ROM bytes).
  third_party/fetch-decomp.sh  Fetches pret/pokeplatinum at build time
                       (never vendored: huge + needs your ROM to build).
  Makefile             assets | check | wasm
```

## What I verified on this machine

```bash
make -C native assets ROM=/path/to/Platinum.nds   # parses FNT/FAT, writes manifest
make -C native check                               # gcc build + 120-frame smoke test
```

`wasm` target is written but NOT verified here (no emcc installed).
CI (`.github/workflows/native.yml`) installs emsdk and verifies it.

## What remains (real scope, no sugar)

| # | Work | Size |
|---|------|------|
| 1 | Build pret/pokeplatinum with your ROM (needs their toolchain + `armips`) | hours |
| 2 | Reimplement NitroSDK OS/FS/GX per-module against hal.h | weeks |
| 3 | 2D GPU (backgrounds, sprites, windows) in WebGL2 | weeks |
| 4 | 3D GPU (geometry engine + rasterizer) in WebGL2 | weeks–months |
| 5 | SPU sound + streaming + mic path in WebAudio | weeks |
| 6 | Overlay loader, ITCM/DTCM/wram layout, timers, DMA, IPC | weeks |
| 7 | Wifi/mic minigames stubbed or reimplemented | days |
| 8 | Perf: frame pacing, save compat with emulator builds | ongoing |

The emulator build at repo root (`index.html`) remains the playable path
until this table is done. Both builds share `rom/` chunk + save formats.

#!/usr/bin/env bash
# wasm-seam check: compile ONE real decomp TU with emcc, run under node,
# compare against host gcc -m32 baseline. No pip/sudo/emsdk-in-repo needed.
# Env: DECOMP=/path/to/pokeplatinum (shallow clone), EMSDK=/path/to/emsdk.
set -euo pipefail
DECOMP="${DECOMP:?set DECOMP=/path/to/pokeplatinum}"
# shellcheck disable=SC1091
source "${EMSDK:?set EMSDK=/path/to/emsdk}/emsdk_env.sh" >/dev/null
cd "$DECOMP"

DEFS="-DPOKEPLATINUM_REVISION=1 -DPOKEPLATINUM_GENERATED_ENUM -DSDK_4M -DSDK_ARM9 -DSDK_CODE_ARM -DSDK_CW -DSDK_CW_FORCE_EXPORT_SUPPORT -DSDK_FINALROM -DSDK_TS -DNNS_FINALROM -DSP1P3_BUG_FOR_CONDITIONAL_ASM_INSTRUCTIONS -D_MSL_RESTRICT="
INCS="-I include -I build -I build/subprojects/NitroSDK-4.2.30001/gen -I subprojects/NitroSDK-4.2.30001/include -I subprojects/NitroSystem-071126.1/include"
SEAM="$(cd "$(dirname "$0")" && pwd)"

# shellcheck disable=SC2086
emcc -std=gnu11 -O2 $DEFS $INCS -include "$SEAM/compat.h" \
    src/ascii_util.c "$SEAM/stubs.c" "$SEAM/test_ascii.c" -I include \
    -o /tmp/seam_wasm.js
node /tmp/seam_wasm.js > /tmp/seam_wasm.txt

# shellcheck disable=SC2086
gcc -m32 -std=gnu11 -O2 $DEFS $INCS -include "$SEAM/compat.h" \
    src/ascii_util.c "$SEAM/stubs.c" "$SEAM/test_ascii.c" -I include \
    -o /tmp/seam_host
/tmp/seam_host > /tmp/seam_host.txt

diff /tmp/seam_wasm.txt /tmp/seam_host.txt && echo "SEAM CHECK: IDENTICAL"

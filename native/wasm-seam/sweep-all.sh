#!/usr/bin/env bash
# Full-tree sweep: every src/**/*.c (excluding upstream orphans).
set -u
cd "${DECOMP:?set DECOMP=/path/to/pokeplatinum}"
source "${EMSDK:?set EMSDK=/path/to/emsdk}/emsdk_env.sh" >/dev/null 2>&1
SEAM="$(cd "$(dirname "$0")" && pwd)"
DEFS="-DPOKEPLATINUM_REVISION=1 -DPOKEPLATINUM_GENERATED_ENUM -DSDK_4M -DSDK_ARM9 -DSDK_CODE_ARM -DSDK_CW -DSDK_CW_FORCE_EXPORT_SUPPORT -DSDK_FINALROM -DSDK_TS -DNNS_FINALROM -DSP1P3_BUG_FOR_CONDITIONAL_ASM_INSTRUCTIONS -D_NITRO -DGAME_VERSION=VERSION_PLATINUM -DGAME_LANGUAGE=LANGUAGE_ENGLISH -DPM_KEEP_ASSERTS -DLINK_PPWLOBBY -D_MSL_RESTRICT="
INCS="-I $SEAM/shim -I include -I build -I build/res/graphics/end_credits -I build/generated -I build/subprojects/NitroSDK-4.2.30001/gen -I subprojects/NitroSDK-4.2.30001/include -I subprojects/NitroSystem-071126.1/include $(cat /tmp/seam-extra-incs.txt) $(cat /tmp/seam-res-incs.txt) -I build/res/graphics/end_credits -include stdlib.h -include $SEAM/compat.h -include include/pch/global_pch.h -include $SEAM/game_protos.h"
CFLAGS_EXTRA="-Wno-int-conversion -fdeclspec -Wno-implicit-int"
mkdir -p /tmp/seam-all/objs /tmp/seam-all/errs
export DEFS INCS CFLAGS_EXTRA SEAM
# regenerate mechanical patches, then layer the hand-written ones on top
python3 "$SEAM/gen_patches.py" "$DECOMP" /tmp/seam-patched >/dev/null
if [ -d "$SEAM/patches" ]; then
  (cd "$SEAM/patches" && find . -name '*.c' -exec cp --parents {} /tmp/seam-patched/ \;)
fi
find src -name "*.c" | while read -r f; do
  b=$(echo "$f" | tr '/' '_')
  # upstream orphan check: not compiled by the real build
  grep -q "$(basename $f)" build/build.ninja || continue
  echo "$f $b"
done > /tmp/seam-all/filelist.txt
wc -l < /tmp/seam-all/filelist.txt
cat /tmp/seam-all/filelist.txt | xargs -P 8 -n2 bash -c '
  f="$0"; b="$1"
  src="$f"; tag=""
  if [ -f /tmp/seam-patched/$f ]; then src=/tmp/seam-patched/$f; tag=" (patched)"; fi
  if emcc -std=gnu11 -O2 $CFLAGS_EXTRA $DEFS $INCS -c "$src" -o /tmp/seam-all/objs/$b.o 2>/tmp/seam-all/errs/$b.err; then
    echo "PASS $f$tag"
  else
    echo "FAIL $f"
  fi' > /tmp/seam-all/results.txt 2>&1
echo "pass=$(grep -c ^PASS /tmp/seam-all/results.txt) fail=$(grep -c ^FAIL /tmp/seam-all/results.txt)"

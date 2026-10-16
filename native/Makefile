# native/ build
#   make check [HOST_FRAMES=120]  gcc smoke test (verified here)
#   make assets ROM="...nds"      asset manifest from your ROM (verified here)
#   make wasm                     Emscripten build (needs emsdk; CI-verified only)
CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -O2 -Inative/include -Iinclude
SRC_HOST = native/src/main.c native/src/core_stub.c native/src/hal_host.c

.PHONY: check assets wasm clean

check:
	$(CC) $(CFLAGS) $(SRC_HOST) -o /tmp/plat-native-check
	HAL_ASSET_PATH="$(ROM)" /tmp/plat-native-check $(HOST_FRAMES)

assets:
	python3 native/tools/extract_assets.py "$(ROM)" native/assets/manifest.json

wasm:
	@test -n "$(EMSDK)" || (echo "set EMSDK=/path/to/emsdk (see https://emscripten.org/docs/getting_started/downloads.html)"; exit 1)
	. "$(EMSDK)/emsdk_env.sh" && emcc -std=gnu11 -O2 -Inative/include \
	  native/src/main.c native/src/core_stub.c native/src/hal_web.c \
	  -s USE_WEBGL2=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 \
	  -s FORCE_FILESYSTEM=1 -lidbfs.js \
	  --preload-file native/assets@/assets \
	  --shell-file native/shell.html \
	  -o dist-native/platinum.html

clean:
	rm -f /tmp/plat-native-check host.sav

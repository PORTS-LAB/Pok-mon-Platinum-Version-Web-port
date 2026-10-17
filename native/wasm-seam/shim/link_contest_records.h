#pragma once
/* Seam shadow for upstream include/link_contest_records.h.
 * Upstream reuses the guard POKEPLATINUM_CONTEST_H from contest.h, so the
 * two headers cannot coexist (one silently becomes empty). This shim
 * redirects to the wasm-seam header that carries the real declarations. */
#include "/home/voidblade/Projects/poke/native/wasm-seam/game_protos.h"

#pragma once
/* Seam shadow for MSL's <extras.h> (pulled in by GameSpy nonport.h).
 * The game TUs in this port never consume MSL extras content directly;
 * the real header drags the entire proprietary MSL C library header set
 * (ansi_parms.h, stdio.posix.h, ...) which would shadow the wasm libc.
 * This stub provides only what the SDK/GameSpy headers transitively relied
 * on (time_t via <time.h>). If a future TU needs an MSL extra, add a
 * portable C implementation here, never by including MSL. */
#include <time.h>

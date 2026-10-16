#ifndef PLAT_HAL_H
#define PLAT_HAL_H
/*
 * Hardware Abstraction Layer — the ONLY surface the game core may touch.
 * Two implementations: hal_host.c (dev smoke test) and hal_web.c (Emscripten).
 * No Nintendo SDK types leak through here. No direct GL/Audio/FS calls
 * from core code. Ever.
 */
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_FB_W 256
#define HAL_FB_H 192
#define HAL_FB_PIXELS (HAL_FB_W * HAL_FB_H)

/* NDS-style key bits (bit = pressed). Matches KEYINPUT order. */
#define HAL_KEY_A      (1u << 0)
#define HAL_KEY_B      (1u << 1)
#define HAL_KEY_SELECT (1u << 2)
#define HAL_KEY_START  (1u << 3)
#define HAL_KEY_RIGHT  (1u << 4)
#define HAL_KEY_LEFT   (1u << 5)
#define HAL_KEY_UP     (1u << 6)
#define HAL_KEY_DOWN   (1u << 7)
#define HAL_KEY_R      (1u << 8)
#define HAL_KEY_L      (1u << 9)
#define HAL_KEY_X      (1u << 10)
#define HAL_KEY_Y      (1u << 11)

/* Lifecycle */
bool     hal_init(int argc, char **argv);
void     hal_shutdown(void);

/* Video: two RGB565 framebuffers owned by HAL. Core draws, HAL presents. */
uint16_t *hal_framebuffer(int screen); /* 0 = top, 1 = bottom */
void      hal_present(void);

/* Audio: interleaved stereo s16, 32768 Hz. HAL queues, never blocks. */
void hal_audio_push(const int16_t *stereo, size_t frames);

/* Input / mic */
uint32_t hal_input_poll(void);  /* combination of HAL_KEY_* */
float    hal_mic_level(void);   /* 0.0 .. 1.0 */

/* Savegame (Platinum flash = 512 KiB raw). */
size_t hal_save_read(uint8_t *out, size_t cap);
bool   hal_save_write(const uint8_t *data, size_t len);

/* Asset filesystem. path like "data/fielddata/map_matrix/map_matrix.narc".
 * HAL allocates *out (caller frees with hal_fs_free). */
bool  hal_fs_read(const char *path, uint8_t **out, size_t *len);
void  hal_fs_free(uint8_t *ptr);

/* Misc */
uint32_t hal_ticks_ms(void);
void     hal_log(const char *msg);

/* Game core entry, implemented by core_stub.c today, by the decomp core later. */
void game_step(void);

#ifdef __cplusplus
}
#endif

#endif /* PLAT_HAL_H */

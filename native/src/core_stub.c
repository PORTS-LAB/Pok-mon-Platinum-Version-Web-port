#include "hal.h"

/*
 * Stand-in game core. Draws a deterministic test pattern so the HAL,
 * frame loop, audio queue and save path can be verified TODAY.
 * Deleted the day the decomp core plugs in — same symbol: game_step().
 */
static unsigned tick;

static uint16_t rgb565(int r, int g, int b) {
    return (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

void game_step(void) {
    tick++;
    for (int s = 0; s < 2; s++) {
        uint16_t *fb = hal_framebuffer(s);
        for (int y = 0; y < HAL_FB_H; y++) {
            for (int x = 0; x < HAL_FB_W; x++) {
                int bar = (x + (int)(tick * 2)) & 0xFF;
                int r = s == 0 ? bar : x & 0xFF;
                int g = y & 0xFF;
                int b = s == 0 ? (x ^ y) & 0xFF : bar;
                fb[y * HAL_FB_W + x] = rgb565(r, g, b);
            }
        }
    }
    hal_present();

    /* 1 frame worth of silent-ish audio at 32768 Hz (~546 frames). */
    static int16_t pcm[546 * 2];
    for (size_t i = 0; i < 546 * 2; i++)
        pcm[i] = (int16_t)((i & 0xFF) - 128);
    hal_audio_push(pcm, 546);

    (void)hal_input_poll();
    (void)hal_mic_level();
}

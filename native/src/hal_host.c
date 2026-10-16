#include "hal.h"

/*
 * Host HAL: proves hal.h is coherent without a browser or an emulator.
 * Framebuffers live in RAM, audio goes to a counter, input is canned,
 * saves go to ./host.sav next to the working directory.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

static uint16_t fb[2][HAL_FB_PIXELS];
static unsigned presented;
static unsigned long audio_frames;
static uint8_t *asset_blob; /* single-file test asset, optional */
static size_t asset_len;

bool hal_init(int argc, char **argv) {
    (void)argc;
    (void)argv;
    memset(fb, 0, sizeof fb);
    /* Optional: HAL_ASSET_PATH points at any file to serve via hal_fs_read. */
    const char *p = getenv("HAL_ASSET_PATH");
    if (p && *p) {
        FILE *f = fopen(p, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long n = ftell(f);
            fseek(f, 0, SEEK_END);
            fseek(f, 0, SEEK_SET);
            if (n > 0 && n < 512L * 1024 * 1024) {
                asset_blob = malloc((size_t)n);
                if (asset_blob && fread(asset_blob, 1, (size_t)n, f) == (size_t)n)
                    asset_len = (size_t)n;
            }
            fclose(f);
        }
    }
    return true;
}

void hal_shutdown(void) {
    uint32_t sum[2] = {0, 0};
    for (int s = 0; s < 2; s++)
        for (int i = 0; i < HAL_FB_PIXELS; i++)
            sum[s] += fb[s][i];
    printf("host-hal: presented=%u audio_frames=%lu fb_sum=%u/%u\n",
           presented, audio_frames, sum[0], sum[1]);
    free(asset_blob);
}

uint16_t *hal_framebuffer(int screen) {
    if (screen < 0 || screen > 1)
        return fb[0];
    return fb[screen];
}

void hal_present(void) {
    presented++;
}

void hal_audio_push(const int16_t *stereo, size_t frames) {
    (void)stereo;
    audio_frames += (unsigned long)frames;
}

uint32_t hal_input_poll(void) {
    return 0; /* no keyboard on host smoke test; web HAL does the real map */
}

float hal_mic_level(void) {
    return 0.0f;
}

size_t hal_save_read(uint8_t *out, size_t cap) {
    FILE *f = fopen("host.sav", "rb");
    if (!f)
        return 0;
    size_t n = fread(out, 1, cap, f);
    fclose(f);
    return n;
}

bool hal_save_write(const uint8_t *data, size_t len) {
    FILE *f = fopen("host.sav", "wb");
    if (!f)
        return false;
    size_t n = fwrite(data, 1, len, f);
    fclose(f);
    return n == len;
}

bool hal_fs_read(const char *path, uint8_t **out, size_t *len) {
    /* Host smoke path: only serves the single HAL_ASSET_PATH blob. */
    if (!path || !out || !len || !asset_blob)
        return false;
    (void)path;
    uint8_t *copy = malloc(asset_len ? asset_len : 1);
    if (!copy)
        return false;
    memcpy(copy, asset_blob, asset_len);
    *out = copy;
    *len = asset_len;
    return true;
}

void hal_fs_free(uint8_t *ptr) {
    free(ptr);
}

uint32_t hal_ticks_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void hal_log(const char *msg) {
    fprintf(stderr, "[hal] %s\n", msg ? msg : "");
}

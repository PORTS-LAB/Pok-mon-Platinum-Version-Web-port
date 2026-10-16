#include "hal.h"

/*
 * Web HAL (Emscripten). Compiled with emcc; NOT verified on this machine
 * (no emcc here) — CI installs emsdk and builds it. Kept minimal on purpose:
 * RGB565 -> RGBA upload to two WebGL textures, keyboard -> NDS keymask,
 * saves via IDBFS, assets via --preload-file, mic via getUserMedia RMS.
 */
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <GLES2/gl2.h>
#endif

static uint16_t fb[2][HAL_FB_PIXELS];
static unsigned long audio_frames;

#ifdef __EMSCRIPTEN__
static GLuint tex[2];
static GLuint prog, vbo;

static const char *VSRC =
    "attribute vec2 p; varying vec2 t;"
    "void main(){ t = vec2(p.x*0.5+0.5, 0.5-p.y*0.5);"
    " gl_Position = vec4(p, 0.0, 1.0); }";
static const char *FSRC =
    "precision mediump float; varying vec2 t; uniform sampler2D s;"
    "void main(){ gl_FragColor = texture2D(s, t); }";

static GLuint compile(const char *src, GLenum type) {
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, 0);
    glCompileShader(sh);
    GLuint p = glCreateProgram();
    /* NOTE: minimal error checking; full log path is a TODO for bring-up. */
    glAttachShader(p, sh);
    glLinkProgram(p);
    return p;
}
#endif

bool hal_init(int argc, char **argv) {
    (void)argc;
    (void)argv;
    memset(fb, 0, sizeof fb);
#ifdef __EMSCRIPTEN__
    EM_ASM({
        /* IDBFS for /save; assets come from --preload-file /assets. */
        try { FS.mkdir('/save'); } catch (e) {}
        try { FS.mount(IDBFS, {}, '/save'); } catch (e) {}
        FS.syncfs(true, function() {});
    });
    prog = compile(VSRC, GL_VERTEX_SHADER);
    GLuint fp = compile(FSRC, GL_FRAGMENT_SHADER);
    glAttachShader(prog, fp);
    glLinkProgram(prog);
    glGenTextures(2, tex);
    for (int i = 0; i < 2; i++) {
        glBindTexture(GL_TEXTURE_2D, tex[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    static const float quad[] = {-1, -1, 1, -1, -1, 1, 1, 1};
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof quad, quad, GL_STATIC_DRAW);
#endif
    return true;
}

void hal_shutdown(void) {
#ifdef __EMSCRIPTEN__
    EM_ASM({ try { FS.syncfs(function() {}); } catch (e) {} });
#endif
}

uint16_t *hal_framebuffer(int screen) {
    if (screen < 0 || screen > 1)
        return fb[0];
    return fb[screen];
}

void hal_present(void) {
#ifdef __EMSCRIPTEN__
    /* Expand RGB565 -> RGBA and blit top/bottom stacked. */
    static uint8_t rgba[HAL_FB_W * HAL_FB_H * 4];
    int w, h;
    emscripten_get_canvas_element_size("#screen", &w, &h);
    glViewport(0, 0, w, h);
    glUseProgram(prog);
    for (int s = 0; s < 2; s++) {
        for (int i = 0; i < HAL_FB_PIXELS; i++) {
            uint16_t p = fb[s][i];
            rgba[i * 4 + 0] = (uint8_t)(((p >> 11) & 0x1F) * 255 / 31);
            rgba[i * 4 + 1] = (uint8_t)(((p >> 5) & 0x3F) * 255 / 63);
            rgba[i * 4 + 2] = (uint8_t)((p & 0x1F) * 255 / 31);
            rgba[i * 4 + 3] = 255;
        }
        glBindTexture(GL_TEXTURE_2D, tex[s]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, HAL_FB_W, HAL_FB_H, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, rgba);
        /* Top screen upper half, bottom screen lower half. */
        glViewport(0, s == 0 ? h / 2 : 0, w, h / 2);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
#else
    /* Host fallback never uses this file; main links hal_host instead. */
#endif
}

void hal_audio_push(const int16_t *stereo, size_t frames) {
    audio_frames += (unsigned long)frames;
#ifdef __EMSCRIPTEN__
    /* TODO bring-up: queue into a ScriptProcessor/AudioWorklet ring.
     * Counting only, so the core/audio contract is testable before
     * the worklet lands. */
    (void)stereo;
#endif
}

/* KeyboardEvent.code -> HAL_KEY_*. Mirrors the emulator build's map. */
uint32_t hal_input_poll(void) {
#ifdef __EMSCRIPTEN__
    return (uint32_t)EM_ASM_INT({
        var k = (typeof __platKeys !== 'undefined') ? __platKeys : 0;
        return k | 0;
    });
#else
    return 0;
#endif
}

float hal_mic_level(void) {
#ifdef __EMSCRIPTEN__
    return (float)EM_ASM_DOUBLE({
        var m = (typeof __platMic !== 'undefined') ? __platMic : 0;
        return +m || 0;
    });
#else
    return 0.0f;
#endif
}

size_t hal_save_read(uint8_t *out, size_t cap) {
#ifdef __EMSCRIPTEN__
    FILE *f = fopen("/save/platinum.sav", "rb");
#else
    FILE *f = fopen("host.sav", "rb");
#endif
    if (!f)
        return 0;
    size_t n = fread(out, 1, cap, f);
    fclose(f);
    return n;
}

bool hal_save_write(const uint8_t *data, size_t len) {
#ifdef __EMSCRIPTEN__
    FILE *f = fopen("/save/platinum.sav", "wb");
#else
    FILE *f = fopen("host.sav", "wb");
#endif
    if (!f)
        return false;
    size_t n = fwrite(data, 1, len, f);
    fclose(f);
#ifdef __EMSCRIPTEN__
    EM_ASM({ try { FS.syncfs(function() {}); } catch (e) {} });
#endif
    return n == len;
}

bool hal_fs_read(const char *path, uint8_t **out, size_t *len) {
    char full[512];
    snprintf(full, sizeof full, "/assets/%s", path ? path : "");
    FILE *f = fopen(full, "rb");
    if (!f)
        return false;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (n <= 0 || n > 256L * 1024 * 1024) {
        fclose(f);
        return false;
    }
    uint8_t *buf = malloc((size_t)n);
    if (!buf) {
        fclose(f);
        return false;
    }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) {
        free(buf);
        fclose(f);
        return false;
    }
    fclose(f);
    *out = buf;
    *len = (size_t)n;
    return true;
}

void hal_fs_free(uint8_t *ptr) {
    free(ptr);
}

uint32_t hal_ticks_ms(void) {
#ifdef __EMSCRIPTEN__
    return (uint32_t)(emscripten_get_now());
#else
    return 0;
#endif
}

void hal_log(const char *msg) {
#ifdef __EMSCRIPTEN__
    EM_ASM({ console.log('[plat]', UTF8ToString($0)); }, msg ? msg : "");
#else
    (void)msg;
#endif
}

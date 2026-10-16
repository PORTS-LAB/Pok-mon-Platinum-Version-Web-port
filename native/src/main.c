#include "hal.h"

/*
 * Boot + frame loop. Platform-agnostic: all hardware goes through hal.h.
 * HOST_FRAMES env (or argv[1]) overrides the frame count for testing.
 */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int frames = 120;
    if (argc > 1) {
        frames = atoi(argv[1]);
        if (frames <= 0 || frames > 1000000)
            frames = 120;
    }

    if (!hal_init(argc, argv)) {
        fprintf(stderr, "hal_init failed\n");
        return 1;
    }
    hal_log("platinum-native boot");

    /* Touch the save path so wiring faults show up early. */
    {
        uint8_t probe[16];
        size_t n = hal_save_read(probe, sizeof probe);
        hal_log(n > 0 ? "save present" : "no save yet");
    }

    for (int i = 0; i < frames; i++)
        game_step();

    hal_shutdown();
    printf("ran %d frames\n", frames);
    return 0;
}

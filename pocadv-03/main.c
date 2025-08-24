// main.c
#define POCADV_IMPLEMENTATION
#include "pocadv.h"

#include <stdio.h>

#define TARGET_FPS 60
#define TARGET_FRAME_TIME (1000 / TARGET_FPS)

int main(int argc, char *argv[]) {
    if (pocadv_init("Pocadv Audio Example", 800, 600) != 0) {
        printf("Failed to initialize pocadv\n");
        return 1;
    }

    int sound1 = pocadv_load_wav("sound1.wav");
    int sound2 = pocadv_load_wav("sound2.wav");

    if (sound1 < 0 || sound2 < 0) {
        printf("Failed to load one or more sounds\n");
    }

    SDL_Color white = {255, 255, 255, 255};

    int running = 1;
    while (running) {
        Uint32 frame_start = SDL_GetTicks();

        SDL_Event event;
        while (pocadv_poll_event(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        pocadv_update_input();

        float dt = pocadv_get_delta_time();

        if (pocadv_key_down(SDL_SCANCODE_ESCAPE)) {
            running = 0;
        }

        // Play sounds when keys pressed
        if (pocadv_key_down(SDL_SCANCODE_1)) {
            pocadv_play_sound(sound1);
        }
        if (pocadv_key_down(SDL_SCANCODE_2)) {
            pocadv_play_sound(sound2);
        }

        int mx, my;
        pocadv_get_mouse_pos(&mx, &my);

        pocadv_set_color((SDL_Color){0, 0, 0, 255});
        pocadv_clear();

        pocadv_set_color(white);
        pocadv_draw_line(10, 10, 790, 10);

        pocadv_present();

        Uint32 frame_end = SDL_GetTicks();
        Uint32 frame_duration = frame_end - frame_start;
        if (frame_duration < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frame_duration);
        }
    }

    pocadv_free_all_audio();
    pocadv_quit();

    return 0;
}


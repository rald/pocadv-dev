#define POCADV_IMPLEMENTATION
#include "pocadv.h"

int main(int argc, char *argv[]) {
    if (pocadv_init("Pocadv SDL2 Single Header Example", 640, 480) != 0) {
        printf("Failed to initialize SDL2\n");
        return 1;
    }

    SDL_Texture *texture = pocadv_load_texture("example.bmp");
    if (!texture) {
        printf("Failed to load texture. Make sure example.bmp is in the working directory.\n");
    }

    int running = 1;
    while (running) {
        SDL_Event event;
        while (pocadv_poll_event(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        pocadv_update_input();

        float dt = pocadv_get_delta_time();
        printf("Delta time: %.4f seconds\n", dt);

        if (pocadv_key_down(SDL_SCANCODE_ESCAPE)) {
            running = 0;
        }

        int mouseX, mouseY;
        pocadv_get_mouse_pos(&mouseX, &mouseY);
        if (pocadv_mouse_button_down(SDL_BUTTON_LEFT)) {
            printf("Left mouse button down at (%d, %d)\n", mouseX, mouseY);
        }

        pocadv_clear();

        if (texture) {
            pocadv_draw_texture(texture, mouseX - 32, mouseY - 32);
        }

        pocadv_present();

        SDL_Delay(16); // ~60 FPS
    }

    if (texture) SDL_DestroyTexture(texture);
    pocadv_quit();
    return 0;
}


#define POCADV_IMPLEMENTATION
#include "pocadv.h"

#include <stdio.h>

// Target FPS
#define TARGET_FPS 60
#define TARGET_FRAME_TIME (1000 / TARGET_FPS)


int main(int argc, char *argv[]) {

	int frame=0;

	SDL_Rect clips[] = {
		{ 0, 0,16,16},
		{16, 0,16,16},
	};

    if (pocadv_init("Pocadv Full Example", 800, 600) != 0) {
        printf("Failed to initialize\n");
        return 1;
    }

    // Load textures
    SDL_Texture *texture = pocadv_load_texture("star.bmp");
    if (!texture) {
        printf("Failed to load example.bmp\n");
    }

    // Color for primitives
    SDL_Color red = {255, 0, 0, 255};
    SDL_Color green = {0, 255, 0, 255};
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
//        printf("Delta time: %.4f seconds\n", dt);

        if (pocadv_key_down(SDL_SCANCODE_ESCAPE)) {
            running = 0;
        }

        int mx, my;
        pocadv_get_mouse_pos(&mx, &my);

        // Clear screen
        pocadv_set_color((SDL_Color){0, 0, 0, 255});
        pocadv_clear();

        // Draw some primitives
        pocadv_set_color(red);
        pocadv_draw_line(50, 50, 750, 50);
        pocadv_draw_rect(50, 100, 200, 100);

        pocadv_set_color(green);
        pocadv_draw_rect_filled(300, 100, 200, 100);
        pocadv_draw_circle(650, 150, 50);
        pocadv_draw_circle_filled(650, 300, 50);

        // Draw polygon
        SDL_Point poly[5] = {{300,400},{350,450},{325,500},{275,500},{250,450}};
        pocadv_set_color(red);
        pocadv_draw_poly_filled(poly, 5);
        pocadv_set_color(white);
        pocadv_draw_poly(poly, 5);

        // Draw texture at mouse
        if (texture)
            pocadv_draw_texture_clipped(texture, mx - 32, my - 32,&clips[frame/10%2]);

        pocadv_present();

        // Frame time management
        Uint32 frame_end = SDL_GetTicks();
        Uint32 frame_duration = frame_end - frame_start;

        if (frame_duration < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frame_duration);
        }
        
        frame++;
    }

    if (texture) SDL_DestroyTexture(texture);
    pocadv_quit();
    return 0;
}


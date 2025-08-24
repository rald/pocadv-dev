#ifndef POCADV_H
#define POCADV_H

#include <SDL2/SDL.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Initialization and cleanup
int pocadv_init(const char *title, int width, int height);
void pocadv_quit();

// Rendering
void pocadv_clear();
void pocadv_present();

SDL_Texture* pocadv_load_texture(const char *file);
void pocadv_draw_texture(SDL_Texture *tex, int x, int y);
void pocadv_draw_texture_clipped(SDL_Texture *tex, int x, int y, const SDL_Rect *clip);

// Event and input handling
int pocadv_poll_event(SDL_Event *event);
void pocadv_update_input();

int pocadv_key_down(SDL_Scancode key);
int pocadv_key_up(SDL_Scancode key);

int pocadv_mouse_button_down(Uint8 button);
int pocadv_mouse_button_up(Uint8 button);

void pocadv_get_mouse_pos(int *x, int *y);

// Delta time
float pocadv_get_delta_time();

// 2D Primitives
void pocadv_draw_point(int x, int y);
void pocadv_draw_line(int x1, int y1, int x2, int y2);
void pocadv_draw_rect(int x, int y, int w, int h);
void pocadv_draw_rect_filled(int x, int y, int w, int h);
void pocadv_draw_circle(int x, int y, int radius);
void pocadv_draw_circle_filled(int x, int y, int radius);

// Draw polygon outline from vertex array (points)
void pocadv_draw_poly(const SDL_Point *points, int count);

// Draw filled polygon from vertex array (points)
void pocadv_draw_poly_filled(const SDL_Point *points, int count);

void pocadv_set_color(SDL_Color color);



#ifdef POCADV_IMPLEMENTATION

static SDL_Window *pocadv_window = NULL;
static SDL_Renderer *pocadv_renderer = NULL;

static const Uint8 *pocadv_keyboard_state = NULL;
static Uint32 pocadv_prev_mouse_buttons = 0;
static Uint32 pocadv_curr_mouse_buttons = 0;
static int pocadv_mouse_x = 0, pocadv_mouse_y = 0;

static Uint64 pocadv_last_counter = 0;
static float pocadv_delta_time = 0.0f;

int pocadv_init(const char *title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;
    pocadv_window = SDL_CreateWindow(title,
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, 0);
    if (!pocadv_window) return -1;
    pocadv_renderer = SDL_CreateRenderer(pocadv_window, -1, SDL_RENDERER_ACCELERATED);
    if (!pocadv_renderer) return -1;

    pocadv_keyboard_state = SDL_GetKeyboardState(NULL);
    pocadv_curr_mouse_buttons = SDL_GetMouseState(&pocadv_mouse_x, &pocadv_mouse_y);
    pocadv_prev_mouse_buttons = pocadv_curr_mouse_buttons;

    pocadv_last_counter = SDL_GetPerformanceCounter();

    return 0;
}

void pocadv_quit() {
    if (pocadv_renderer) SDL_DestroyRenderer(pocadv_renderer);
    if (pocadv_window) SDL_DestroyWindow(pocadv_window);
    SDL_Quit();
}

void pocadv_clear() {
    SDL_SetRenderDrawColor(pocadv_renderer, 0, 0, 0, 255);
    SDL_RenderClear(pocadv_renderer);
}

void pocadv_present() {
    SDL_RenderPresent(pocadv_renderer);
}

SDL_Texture* pocadv_load_texture(const char *file) {
    SDL_Surface *surf = SDL_LoadBMP(file);
    if (!surf) return NULL;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(pocadv_renderer, surf);
    SDL_FreeSurface(surf);
    return tex;
}

void pocadv_draw_texture(SDL_Texture *tex, int x, int y) {
    SDL_Rect dst = {x, y, 0, 0};
    SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    SDL_RenderCopy(pocadv_renderer, tex, NULL, &dst);
}

void pocadv_draw_texture_clipped(SDL_Texture *tex, int x, int y, const SDL_Rect *clip) {
    if (!tex || !clip) return;

    SDL_Rect dst = {x, y, clip->w, clip->h};
    SDL_RenderCopy(pocadv_renderer, tex, clip, &dst);
}

int pocadv_poll_event(SDL_Event *event) {
    return SDL_PollEvent(event);
}

void pocadv_update_input() {
    pocadv_prev_mouse_buttons = pocadv_curr_mouse_buttons;
    pocadv_curr_mouse_buttons = SDL_GetMouseState(&pocadv_mouse_x, &pocadv_mouse_y);
}

int pocadv_key_down(SDL_Scancode key) {
    if (pocadv_keyboard_state == NULL) return 0;
    return pocadv_keyboard_state[key];
}

int pocadv_key_up(SDL_Scancode key) {
    if (pocadv_keyboard_state == NULL) return 1;
    return !pocadv_keyboard_state[key];
}

int pocadv_mouse_button_down(Uint8 button) {
    return (pocadv_curr_mouse_buttons & SDL_BUTTON(button)) != 0;
}

int pocadv_mouse_button_up(Uint8 button) {
    return (pocadv_curr_mouse_buttons & SDL_BUTTON(button)) == 0;
}

void pocadv_get_mouse_pos(int *x, int *y) {
    if (x) *x = pocadv_mouse_x;
    if (y) *y = pocadv_mouse_y;
}

float pocadv_get_delta_time() {
    Uint64 current_counter = SDL_GetPerformanceCounter();
    Uint64 counter_diff = current_counter - pocadv_last_counter;
    Uint64 freq = SDL_GetPerformanceFrequency();
    pocadv_delta_time = (float)counter_diff / (float)freq;
    pocadv_last_counter = current_counter;
    return pocadv_delta_time;
}

void pocadv_draw_point(int x, int y) {
    SDL_RenderDrawPoint(pocadv_renderer, x, y);
}

void pocadv_draw_line(int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(pocadv_renderer, x1, y1, x2, y2);
}

void pocadv_draw_rect(int x, int y, int w, int h) {
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderDrawRect(pocadv_renderer, &rect);
}

void pocadv_draw_rect_filled(int x, int y, int w, int h) {
    SDL_Rect rect = {x, y, w, h};
    SDL_RenderFillRect(pocadv_renderer, &rect);
}

// Midpoint circle algorithm for filled circle approximation
void pocadv_draw_circle(int cx, int cy, int radius) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        SDL_RenderDrawPoint(pocadv_renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(pocadv_renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(pocadv_renderer, cx - y, cy + x);
        SDL_RenderDrawPoint(pocadv_renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(pocadv_renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(pocadv_renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(pocadv_renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(pocadv_renderer, cx + x, cy - y);

        y += 1;
        if (err <= 0) {
            err += 2*y + 1;
        } else {
            x -= 1;
            err += 2*(y - x) + 1;
        }
    }
}

void pocadv_draw_circle_filled(int cx, int cy, int radius) {
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y) {
        // Draw horizontal lines between the points to fill the circle
        SDL_RenderDrawLine(pocadv_renderer, cx - x, cy + y, cx + x, cy + y);
        SDL_RenderDrawLine(pocadv_renderer, cx - y, cy + x, cx + y, cy + x);
        SDL_RenderDrawLine(pocadv_renderer, cx - x, cy - y, cx + x, cy - y);
        SDL_RenderDrawLine(pocadv_renderer, cx - y, cy - x, cx + y, cy - x);

        y++;
        if (err <= 0) {
            err += 2*y + 1;
        } else {
            x--;
            err += 2*(y - x) + 1;
        }
    }
}

// Draw polygon outline by connecting all points and closing the loop
void pocadv_draw_poly(const SDL_Point *points, int count) {
    if (count < 2) return;
    for (int i = 0; i < count - 1; i++) {
        SDL_RenderDrawLine(pocadv_renderer,
                           points[i].x, points[i].y,
                           points[i + 1].x, points[i + 1].y);
    }
    // Close the polygon by connecting last to first
    SDL_RenderDrawLine(pocadv_renderer,
                       points[count - 1].x, points[count - 1].y,
                       points[0].x, points[0].y);
}

// Filled polygon using scanline fill algorithm (even-odd rule)
// For simplicity, assumes polygon is simple and convex or concave without holes.
void pocadv_draw_poly_filled(const SDL_Point *points, int count) {
    if (count < 3) return;

    // Find bounding box
    int min_y = points[0].y, max_y = points[0].y;
    for (int i = 1; i < count; i++) {
        if (points[i].y < min_y) min_y = points[i].y;
        if (points[i].y > max_y) max_y = points[i].y;
    }

    // For each scanline
    for (int y = min_y; y <= max_y; y++) {
        // Find intersections with polygon edges
        int intersections[64];
        int n_intersections = 0;

        for (int i = 0; i < count; i++) {
            int j = (i + 1) % count;
            int y0 = points[i].y;
            int y1 = points[j].y;
            int x0 = points[i].x;
            int x1 = points[j].x;

            if ((y0 < y && y1 >= y) || (y1 < y && y0 >= y)) {
                int x = x0 + (y - y0) * (x1 - x0) / (y1 - y0);
                if (n_intersections < 64) { // avoid overflow
                    intersections[n_intersections++] = x;
                }
            }
        }

        // Sort intersections
        for (int i = 0; i < n_intersections - 1; i++) {
            for (int j = i + 1; j < n_intersections; j++) {
                if (intersections[j] < intersections[i]) {
                    int tmp = intersections[i];
                    intersections[i] = intersections[j];
                    intersections[j] = tmp;
                }
            }
        }

        // Draw spans between pairs of intersections
        for (int i = 0; i < n_intersections; i += 2) {
            if (i + 1 < n_intersections) {
                SDL_RenderDrawLine(pocadv_renderer, intersections[i], y, intersections[i + 1], y);
            }
        }
    }
}

void pocadv_set_color(SDL_Color color) {
    SDL_SetRenderDrawColor(pocadv_renderer, color.r, color.g, color.b, color.a);
}

#ifdef __cplusplus
}
#endif

#endif // POCADV_IMPLEMENTATION

#endif // POCADV_H


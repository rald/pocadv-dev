#ifndef POCADV_H
#define POCADV_H

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

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

// Input
int pocadv_poll_event(SDL_Event *event);
void pocadv_update_input();

int pocadv_key_down(SDL_Scancode key);
int pocadv_key_up(SDL_Scancode key);

int pocadv_mouse_button_down(Uint8 button);
int pocadv_mouse_button_up(Uint8 button);

void pocadv_get_mouse_pos(int *x, int *y);

// Drawing primitives
void pocadv_set_color(SDL_Color color);

void pocadv_draw_point(int x, int y);
void pocadv_draw_line(int x1, int y1, int x2, int y2);
void pocadv_draw_rect(int x, int y, int w, int h);
void pocadv_draw_rect_filled(int x, int y, int w, int h);
void pocadv_draw_circle(int x, int y, int radius);
void pocadv_draw_circle_filled(int x, int y, int radius);
void pocadv_draw_poly(const SDL_Point *points, int count);
void pocadv_draw_poly_filled(const SDL_Point *points, int count);

// Timing
float pocadv_get_delta_time();

// Audio management struct
typedef struct {
    SDL_AudioDeviceID device;
    SDL_AudioSpec spec;
    Uint8 *buffer;
    Uint32 length;

    int loops_remaining; // -1 means infinite looping
} pocadv_Audio;

// Audio functions
pocadv_Audio* pocadv_audio_load(const char *file);

// Modified: loop_count - 0 means infinite looping
int pocadv_audio_play(pocadv_Audio *audio, int loop_count);

void pocadv_audio_stop(pocadv_Audio *audio);
void pocadv_audio_pause(pocadv_Audio *audio);
void pocadv_audio_unpause(pocadv_Audio *audio);
void pocadv_audio_free(pocadv_Audio *audio);

// Call this regularly (e.g. once per frame) to handle looping playback
void pocadv_audio_update(pocadv_Audio *audio);

#ifdef POCADV_IMPLEMENTATION

static SDL_Window *pocadv_window = NULL;
static SDL_Renderer *pocadv_renderer = NULL;

static const Uint8 *pocadv_keyboard_state = NULL;
static Uint32 pocadv_prev_mouse_buttons = 0;
static Uint32 pocadv_curr_mouse_buttons = 0;
static int pocadv_mouse_x = 0, pocadv_mouse_y = 0;

static Uint64 pocadv_last_counter = 0;
static float pocadv_delta_time = 0.0f;

// ----------------------- Initialization ----------------------

int pocadv_init(const char *title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) return -1;

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
    // Stop audio and close audio device if open
    if (pocadv_renderer) SDL_DestroyRenderer(pocadv_renderer);
    if (pocadv_window) SDL_DestroyWindow(pocadv_window);

    SDL_Quit();
}

// ----------------------- Rendering ----------------------

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
    SDL_SetColorKey(surf, SDL_TRUE, SDL_MapRGB(surf->format, 0xFF, 0x00, 0xFF));
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

// ----------------------- Input ----------------------

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

// ----------------------- Color ----------------------

void pocadv_set_color(SDL_Color color) {
    SDL_SetRenderDrawColor(pocadv_renderer, color.r, color.g, color.b, color.a);
}

// --------------------- Primitives ----------------------

void pocadv_draw_point(int x, int y) {
    SDL_RenderDrawPoint(pocadv_renderer, x, y);
}

void pocadv_draw_line(int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(pocadv_renderer, x1, y1, x2, y2);
}

void pocadv_draw_rect(int x, int y, int w, int h) {
    SDL_Rect r = {x, y, w, h};
    SDL_RenderDrawRect(pocadv_renderer, &r);
}

void pocadv_draw_rect_filled(int x, int y, int w, int h) {
    SDL_Rect r = {x, y, w, h};
    SDL_RenderFillRect(pocadv_renderer, &r);
}

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

void pocadv_draw_poly(const SDL_Point *points, int count) {
    if (count < 2) return;
    for (int i = 0; i < count - 1; i++) {
        SDL_RenderDrawLine(pocadv_renderer,
                           points[i].x, points[i].y,
                           points[i + 1].x, points[i + 1].y);
    }
    SDL_RenderDrawLine(pocadv_renderer,
                       points[count - 1].x, points[count - 1].y,
                       points[0].x, points[0].y);
}

void pocadv_draw_poly_filled(const SDL_Point *points, int count) {
    if (count < 3) return;

    int min_y = points[0].y, max_y = points[0].y;
    for (int i = 1; i < count; i++) {
        if (points[i].y < min_y) min_y = points[i].y;
        if (points[i].y > max_y) max_y = points[i].y;
    }

    for (int y = min_y; y <= max_y; y++) {
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
                if (n_intersections < 64) {
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

        for (int i = 0; i < n_intersections; i += 2) {
            if (i + 1 < n_intersections) {
                SDL_RenderDrawLine(pocadv_renderer, intersections[i], y, intersections[i + 1], y);
            }
        }
    }
}

// ----------------------- Timing ----------------------

float pocadv_get_delta_time() {
    Uint64 current_counter = SDL_GetPerformanceCounter();
    Uint64 counter_diff = current_counter - pocadv_last_counter;
    Uint64 freq = SDL_GetPerformanceFrequency();
    pocadv_delta_time = (float)counter_diff / (float)freq;
    pocadv_last_counter = current_counter;
    return pocadv_delta_time;
}


// Audio implementation

pocadv_Audio* pocadv_audio_load(const char *file) {
    if (!file) return NULL;

    pocadv_Audio *audio = (pocadv_Audio*)malloc(sizeof(pocadv_Audio));
    if (!audio) return NULL;

    audio->buffer = NULL;
    audio->length = 0;
    audio->device = 0;
    audio->loops_remaining = 0;

    if (SDL_LoadWAV(file, &audio->spec, &audio->buffer, &audio->length) == NULL) {
        free(audio);
        return NULL;
    }

    // Open audio device for playback with WAV's spec
    audio->device = SDL_OpenAudioDevice(NULL, 0, &audio->spec, NULL, 0);
    if (audio->device == 0) {
        SDL_FreeWAV(audio->buffer);
        free(audio);
        return NULL;
    }
    return audio;
}

// Modified to accept loop_count:
//  loop_count == 0 means infinite loop
int pocadv_audio_play(pocadv_Audio *audio, int loop_count) {
    if (!audio || audio->device == 0) return -1;

    if (loop_count == 0)
        audio->loops_remaining = -1; // infinite loops
    else
        audio->loops_remaining = loop_count;

    SDL_ClearQueuedAudio(audio->device);

    if (SDL_QueueAudio(audio->device, audio->buffer, audio->length) < 0) {
        return -1;
    }

    SDL_PauseAudioDevice(audio->device, 0); // Start playing
    return 0;
}

// Call this regularly (e.g. once per frame) to maintain looping playback
void pocadv_audio_update(pocadv_Audio *audio) {
    if (!audio || audio->device == 0) return;

    Uint32 queued = SDL_GetQueuedAudioSize(audio->device);
    if (queued == 0) {
        if (audio->loops_remaining == 0) {
            SDL_PauseAudioDevice(audio->device, 1); // stop playback
        } else {
            if (audio->loops_remaining > 0)
                audio->loops_remaining--;
            SDL_QueueAudio(audio->device, audio->buffer, audio->length);
            SDL_PauseAudioDevice(audio->device, 0); // ensure playing
        }
    }
}

void pocadv_audio_stop(pocadv_Audio *audio) {
    if (!audio || audio->device == 0) return;

    SDL_ClearQueuedAudio(audio->device);
    SDL_PauseAudioDevice(audio->device, 1);
}

void pocadv_audio_pause(pocadv_Audio *audio) {
    if (!audio || audio->device == 0) return;

    SDL_PauseAudioDevice(audio->device, 1);
}

void pocadv_audio_unpause(pocadv_Audio *audio) {
    if (!audio || audio->device == 0) return;

    SDL_PauseAudioDevice(audio->device, 0);
}

void pocadv_audio_free(pocadv_Audio *audio) {
    if (!audio) return;

    if (audio->device != 0) {
        SDL_CloseAudioDevice(audio->device);
        audio->device = 0;
    }
    if (audio->buffer) {
        SDL_FreeWAV(audio->buffer);
        audio->buffer = NULL;
    }
    free(audio);
}

#ifdef __cplusplus
}
#endif

#endif // POCADV_IMPLEMENTATION

#endif // POCADV_H


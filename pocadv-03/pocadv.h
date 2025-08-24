// pocadv.h
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

// Audio system
// Returns sound ID or -1 on failure
int pocadv_load_wav(const char *file);

// Control playback of sounds by sound ID
void pocadv_play_sound(int sound_id);
void pocadv_pause_sound(int sound_id);
void pocadv_unpause_sound(int sound_id);
void pocadv_stop_sound(int sound_id);

// Audio system cleanup
void pocadv_free_all_audio();

#ifdef POCADV_IMPLEMENTATION

static void pocadv_audio_callback(void *userdata, Uint8 *stream, int len);

// Internal state
static SDL_Window *pocadv_window = NULL;
static SDL_Renderer *pocadv_renderer = NULL;

static const Uint8 *pocadv_keyboard_state = NULL;
static Uint32 pocadv_prev_mouse_buttons = 0;
static Uint32 pocadv_curr_mouse_buttons = 0;
static int pocadv_mouse_x = 0, pocadv_mouse_y = 0;

static Uint64 pocadv_last_counter = 0;
static float pocadv_delta_time = 0.0f;

// Audio
typedef struct {
    Uint8 *buffer;
    Uint32 length;
    Uint32 position;
    int playing;       // 0 = stopped, 1 = playing, 2 = paused
    SDL_AudioSpec spec;
} PocadvSound;

static PocadvSound *pocadv_sounds = NULL;
static int pocadv_sound_count = 0;
static int pocadv_sound_capacity = 0;

static SDL_AudioSpec pocadv_device_spec;
static SDL_AudioDeviceID pocadv_audio_device = 0;

// -------- Initialization & Quit --------

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

    // Initialize audio device spec (standard 44100 Hz stereo 16bit)
    SDL_zero(pocadv_device_spec);
    pocadv_device_spec.freq = 44100;
    pocadv_device_spec.format = AUDIO_S16SYS;
    pocadv_device_spec.channels = 2;
    pocadv_device_spec.samples = 4096;
    pocadv_device_spec.callback = NULL; // set later

    // Open audio device with callback
    pocadv_audio_device = SDL_OpenAudioDevice(NULL, 0,
                                              &pocadv_device_spec,
                                              &pocadv_device_spec,
                                              SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (pocadv_audio_device == 0) return -1;

    // Set callback after device open
    pocadv_device_spec.callback = pocadv_audio_callback;
    pocadv_device_spec.userdata = NULL;
    SDL_PauseAudioDevice(pocadv_audio_device, 0);

    return 0;
}

void pocadv_quit() {
    pocadv_free_all_audio();

    if (pocadv_audio_device != 0) SDL_CloseAudioDevice(pocadv_audio_device);
    pocadv_audio_device = 0;

    if (pocadv_renderer) SDL_DestroyRenderer(pocadv_renderer);
    if (pocadv_window) SDL_DestroyWindow(pocadv_window);

    SDL_Quit();
}

// -------- Audio Callback (mix multiple sounds) --------

void pocadv_audio_callback(void *userdata, Uint8 *stream, int len) {
    SDL_memset(stream, 0, len);

    for (int i = 0; i < pocadv_sound_count; i++) {
        PocadvSound *s = &pocadv_sounds[i];
        if (s->playing != 1) continue;

        Uint32 bytes_left = s->length - s->position;
        Uint32 to_mix = (Uint32)len > bytes_left ? bytes_left : (Uint32)len;

        for (Uint32 j = 0; j < to_mix; j += 2) {
            // Mix 16-bit samples (assuming AUDIO_S16SYS)
            Sint16 *dst = (Sint16*)(stream + j);
            Sint16 *src = (Sint16*)(s->buffer + s->position + j);
            int mixed = *dst + *src;

            if (mixed > 32767) mixed = 32767;
            else if (mixed < -32768) mixed = -32768;

            *dst = (Sint16)mixed;
        }
        s->position += to_mix;
        if (s->position >= s->length) {
            s->playing = 0; // Finished
            s->position = 0;
        }
    }
}

// -------- Rendering functions --------

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

// -------- Input functions --------

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

// -------- Color Setting --------

void pocadv_set_color(SDL_Color color) {
    SDL_SetRenderDrawColor(pocadv_renderer, color.r, color.g, color.b, color.a);
}

// -------- Drawing primitives --------

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

// -------- Timing --------

float pocadv_get_delta_time() {
    Uint64 current_counter = SDL_GetPerformanceCounter();
    Uint64 counter_diff = current_counter - pocadv_last_counter;
    Uint64 freq = SDL_GetPerformanceFrequency();
    pocadv_delta_time = (float)counter_diff / (float)freq;
    pocadv_last_counter = current_counter;
    return pocadv_delta_time;
}

// -------- Audio system API --------

int pocadv_load_wav(const char *file) {
    SDL_AudioSpec spec;
    Uint8 *buffer = NULL;
    Uint32 length = 0;

    if (SDL_LoadWAV(file, &spec, &buffer, &length) == NULL) {
        printf("SDL_LoadWAV error: %s\n", SDL_GetError());
        return -1;
    }

    if (spec.format != pocadv_device_spec.format ||
        spec.freq != pocadv_device_spec.freq ||
        spec.channels != pocadv_device_spec.channels) {

        SDL_AudioCVT cvt;
        if (SDL_BuildAudioCVT(&cvt,
                              spec.format, spec.channels, spec.freq,
                              pocadv_device_spec.format, pocadv_device_spec.channels, pocadv_device_spec.freq) < 0) {
            SDL_FreeWAV(buffer);
            printf("SDL_BuildAudioCVT error: %s\n", SDL_GetError());
            return -1;
        }
        cvt.len = length;
        cvt.buf = (Uint8*)malloc(length * cvt.len_mult);
        if (!cvt.buf) {
            SDL_FreeWAV(buffer);
            printf("Failed to allocate memory for audio conversion\n");
            return -1;
        }
        memcpy(cvt.buf, buffer, length);
        if (SDL_ConvertAudio(&cvt) < 0) {
            free(cvt.buf);
            SDL_FreeWAV(buffer);
            printf("SDL_ConvertAudio error: %s\n", SDL_GetError());
            return -1;
        }
        SDL_FreeWAV(buffer);
        buffer = cvt.buf;
        length = cvt.len_cvt;
        spec.format = pocadv_device_spec.format;
        spec.freq = pocadv_device_spec.freq;
        spec.channels = pocadv_device_spec.channels;
    }

    if (pocadv_sound_count >= pocadv_sound_capacity) {
        int new_capacity = pocadv_sound_capacity == 0 ? 16 : pocadv_sound_capacity * 2;
        PocadvSound *new_sounds = (PocadvSound*)realloc(pocadv_sounds, new_capacity * sizeof(PocadvSound));
        if (!new_sounds) {
            SDL_FreeWAV(buffer);
            printf("Failed to realloc for audio buffer\n");
            return -1;
        }
        for (int i = pocadv_sound_capacity; i < new_capacity; i++) {
            new_sounds[i].buffer = NULL;
            new_sounds[i].playing = 0;
            new_sounds[i].length = 0;
            new_sounds[i].position = 0;
        }
        pocadv_sounds = new_sounds;
        pocadv_sound_capacity = new_capacity;
    }

    pocadv_sounds[pocadv_sound_count].buffer = buffer;
    pocadv_sounds[pocadv_sound_count].length = length;
    pocadv_sounds[pocadv_sound_count].position = 0;
    pocadv_sounds[pocadv_sound_count].playing = 0;
    pocadv_sounds[pocadv_sound_count].spec = spec;

    return pocadv_sound_count++;
}

void pocadv_play_sound(int sound_id) {
    if (sound_id < 0 || sound_id >= pocadv_sound_count) return;
    PocadvSound *s = &pocadv_sounds[sound_id];
    if (!s->buffer) return;

    s->position = 0;
    s->playing = 1;
}

void pocadv_pause_sound(int sound_id) {
    if (sound_id < 0 || sound_id >= pocadv_sound_count) return;
    PocadvSound *s = &pocadv_sounds[sound_id];
    if (s->playing == 1)
        s->playing = 2;
}

void pocadv_unpause_sound(int sound_id) {
    if (sound_id < 0 || sound_id >= pocadv_sound_count) return;
    PocadvSound *s = &pocadv_sounds[sound_id];
    if (s->playing == 2)
        s->playing = 1;
}

void pocadv_stop_sound(int sound_id) {
    if (sound_id < 0 || sound_id >= pocadv_sound_count) return;
    PocadvSound *s = &pocadv_sounds[sound_id];
    s->playing = 0;
    s->position = 0;
}

void pocadv_free_all_audio() {
    if (pocadv_sounds) {
        for (int i = 0; i < pocadv_sound_count; i++) {
            if (pocadv_sounds[i].buffer) {
                free(pocadv_sounds[i].buffer);
                pocadv_sounds[i].buffer = NULL;
            }
            pocadv_sounds[i].playing = 0;
            pocadv_sounds[i].length = 0;
            pocadv_sounds[i].position = 0;
        }
        free(pocadv_sounds);
        pocadv_sounds = NULL;
        pocadv_sound_count = 0;
        pocadv_sound_capacity = 0;
    }
}

#ifdef __cplusplus
}
#endif

#endif // POCADV_IMPLEMENTATION

#endif // POCADV_H


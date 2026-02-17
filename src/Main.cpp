#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "SDLHelper.h"
#include "Logger.h"

bool is_running = false;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
uint32_t* color_buffer = NULL;
SDL_Texture* color_buffer_texture = nullptr;

// Logical size
int window_width = 800;
int window_height = 600;
// Pjhy
int renderW = 0;
int renderH = 0;
float scale = 0;

bool initialize_window() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error initializing SDL");
        return false;
    }
    SDL_WindowFlags windowFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
    windowFlags = windowFlags | SDL_WINDOW_RESIZABLE;
    window = SDL_CreateWindow("3d renderer", 1024, 768, windowFlags);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating SDL Window");
        return false;
    }
    SDLHelper::SetRenderSizeInPixels(window, &renderW, &renderH, &scale);
    renderer = SDL_CreateRenderer(window, NULL);
    SDL_SetRenderLogicalPresentation(renderer, window_width, window_height, SDL_LOGICAL_PRESENTATION_STRETCH);
    return true;
}

void setup() {
    color_buffer = new uint32_t[window_width * window_height];
    color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
}

void process_input() {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
        case SDL_EVENT_QUIT:
            is_running = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_ESCAPE) {
                is_running = false;
            }
            break;
    }
}

void update() {

}

void destroy_window() {
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void clear_color_buffer(uint32_t color) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            color_buffer[y * window_width + x] = color;
        }
    }
}

void draw_grid() {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            if (y % 10 == 0 || x % 10 == 0) {
                color_buffer[y * window_width + x] = 0xFF000000;
            }
        }
    }
}

void render_color_buffer() {
    SDL_UpdateTexture(color_buffer_texture, nullptr, color_buffer, window_width * sizeof(uint32_t));
    SDL_RenderTexture(renderer, color_buffer_texture, nullptr, nullptr);
}
void render() {

    SDL_RenderClear(renderer);
    clear_color_buffer(0xFFFFFF00);
    draw_grid();
    render_color_buffer();
    SDL_RenderPresent(renderer);
}
int main(int argc, char* argv[]) {
    SDL_SetLogPriority(MY_LOG_SDL, SDL_LOG_PRIORITY_DEBUG);
    is_running = initialize_window();
    setup();
    while (is_running) {
        process_input();
        update();
        render();
    }
    destroy_window();
    return 0;
}
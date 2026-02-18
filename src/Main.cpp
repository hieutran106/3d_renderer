#include <stdlib.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "Display.h"
#include "Logger.h"

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
void render() {
    SDL_RenderClear(renderer);
    clear_color_buffer(0xFFFFFF00);
    // draw_grid();
    draw_rect(window_width/2 ,0, 900 , window_height/2, 0xFFFF0000);
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
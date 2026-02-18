#include <stdlib.h>
// #include <SDL3/SDL_main.h>

#include "Display.h"
#include "Logger.h"
#include "Vector.h"

const int N_POINTS = 9 * 9 * 9;
vec3_t cube_points[N_POINTS];
vec2_t projected_points[N_POINTS];
bool is_running = false;

float fov_factor = 120;

void setup() {
    color_buffer = new uint32_t[window_width * window_height];
    color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
    // Start loading my array of vectors
    // From -1 to 1
    int point_count = 0;
    for (float x = -1; x <=1; x+= 0.25) {

        for (float y = -1; y <=1; y+= 0.25) {
            for (float z = -1; z <= 1; z += 0.25) {
                vec3_t new_point = {x, y, z};
                cube_points[point_count++] = new_point;
            }
        }
    }
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

// Function that receives a 3D vector and returns a projected 2D point
vec2_t project(vec3_t point) {
    vec2_t projected_point = {
        .x = fov_factor * point.x,
        .y = fov_factor * point.y
    };
    return projected_point;
}

void update() {
    for (int i =0; i <  N_POINTS;i++) {
        vec3_t point = cube_points[i];
        // Project the current point
        vec2_t projected_point = project(point);
        projected_points[i] = projected_point;
    }
}
void render() {
    clear_color_buffer(0xFF000000);
    draw_grid();
    for (int i = 0; i <N_POINTS;i++) {
        vec2_t projected_point = projected_points[i];
        draw_rect(projected_point.x + window_width/ 2, projected_point.y + window_height/2, 4, 4, 0xFFFFFF00);
    }
    render_color_buffer();
    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    SDL_SetLogPriority(MY_LOG_SDL, SDL_LOG_PRIORITY_DEBUG);
    is_running = initialize_window();
    setup();
    vec3_t myvec = {0.1f, 2, 4};
    while (is_running) {
        process_input();
        update();
        render();
    }
    destroy_window();
    return 0;
}
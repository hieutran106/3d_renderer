#include "Display.h"
#include "SDLHelper.h"



SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
uint32_t* color_buffer = nullptr;
SDL_Texture* color_buffer_texture = nullptr;

// Logical size
int window_width = 800;
int window_height = 600;
// Physical pixels
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

void draw_grid() {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            if (y % 10 == 0 || x % 10 == 0) {
                color_buffer[y * window_width + x] = 0xFF333333;
            }
        }
    }
}

void draw_pixel(int x, int y, uint32_t color) {
    if (x >=0 && x < window_width && y>=0 &&  y < window_height) {
        color_buffer[y * window_width + x] = color;
    }
}

void draw_rect(int x, int y, int width, int height, uint32_t color) {
    for (int i =0 ; i < width; i++) {
        for (int j =0 ;j < height;j++) {
            int current_x = x + i;
            int current_y = y + j;
            draw_pixel(current_x, current_y, color);
        }
    }

}

void render_color_buffer() {
    SDL_UpdateTexture(color_buffer_texture, nullptr, color_buffer, window_width * sizeof(uint32_t));
    SDL_RenderTexture(renderer, color_buffer_texture, nullptr, nullptr);
}

void clear_color_buffer(uint32_t color) {
    for (int y = 0; y < window_height; y++) {
        for (int x = 0; x < window_width; x++) {
            color_buffer[y * window_width + x] = color;
        }
    }
}
void destroy_window() {
    free(color_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
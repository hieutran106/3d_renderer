#pragma once
#include <SDL3/SDL.h>
#include <stdlib.h>



extern bool is_running;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern uint32_t* color_buffer;
extern SDL_Texture* color_buffer_texture;

// Logical size
extern int window_width;
extern int window_height;
// Physical pixels
extern int renderW;
extern int renderH;
extern float scale;

bool initialize_window();
void draw_grid();
void draw_rect(int x, int y, int width, int height, uint32_t color);
void render_color_buffer();
void clear_color_buffer(uint32_t color);
void destroy_window();
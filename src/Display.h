#pragma once
#include <SDL3/SDL.h>
#include <stdlib.h>

#define FPS               30
#define FRAME_TARGET_TIME (1000 / FPS)

// Render mode
enum cull_method { CULL_NONE, CULL_BACKFACE };
enum render_method { RENDER_WIRE, RENDER_WIRE_VERTEX, RENDER_FILL_TRIANGLE, RENDER_FILL_TRIANGLE_WIRE };

extern cull_method cull_method;
extern render_method render_method;


extern bool is_running;
extern bool is_paused;
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
void draw_pixel(int x, int y, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
// void draw_filled_triangle();
void render_color_buffer();
void clear_color_buffer(uint32_t color);
void destroy_window();

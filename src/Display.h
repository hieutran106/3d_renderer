#pragma once
#include "Triangle.h"
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#define FPS 30
#define FRAME_TARGET_TIME (1000 / FPS)

// Render mode
enum cull_method
{
	CULL_NONE,
	CULL_BACKFACE
};
enum render_method
{
	RENDER_WIRE,
	RENDER_WIRE_VERTEX,
	RENDER_FILL_TRIANGLE,
	RENDER_FILL_TRIANGLE_WIRE,
	RENDER_TEXTURED,
	RENDER_TEXTURED_WIRE
};

extern cull_method cull_method;
extern render_method render_method;

extern bool is_running;
extern bool is_paused;
extern SDL_Window * window;
extern SDL_Renderer * renderer;
extern uint32_t * color_buffer;
extern float * z_buffer;
extern SDL_Texture * color_buffer_texture;
extern TTF_Font * font;

// Logical size
extern int window_width;
extern int window_height;
// Physical pixels
extern int renderW;
extern int renderH;
extern float scale;

// Triangle to render
constexpr int MAX_TRIANGLE_PER_MESH = 10000;
extern triangle_t triangles_to_render[MAX_TRIANGLE_PER_MESH];
extern int num_triangles_to_render;

bool initialize_window();
void draw_grid();
void draw_pixel(int x, int y, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void render_color_buffer();
void render_text();
void clear_color_buffer(uint32_t color);
void clear_z_buffer();
void destroy_window();

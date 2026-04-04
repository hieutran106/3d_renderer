#pragma once
#include "Texture.h"
#include "Vector.h"
#include <cstdint>

struct face_t
{
	int a;
	int b;
	int c;
	tex2_t a_uv;
	tex2_t b_uv;
	tex2_t c_uv;
	uint32_t color;
};

struct triangle_t
{
	vec2_t points[3];
	tex2_t texcoords[3];
	uint32_t color;
	float avg_depth;
};

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
// TODO: void draw_textured_triangle(...)

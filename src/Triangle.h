#pragma once
#include "Texture.h"
#include "Vector.h"
#include <array>

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
	std::array<vec4_t, 3> points;
	std::array<tex2_t, 3> texcoords;
	uint32_t color;
	// upng_t * texture;
	png_texture_t png_texture;
};

void draw_filled_triangle(
	int x0,
	int y0,
	float z0,
	float w0,
	int x1,
	int y1,
	float z1,
	float w1,
	int x2,
	int y2,
	float z2,
	float w2,
	uint32_t color
);
void draw_textured_triangle(
	int x0,
	int y0,
	float z0,
	float w0,
	float u0,
	float v0,
	int x1,
	int y1,
	float z1,
	float w1,
	float u1,
	float v1,
	int x2,
	int y2,
	float z2,
	float w2,
	float u2,
	float v2,
	const png_texture_t & texture
);

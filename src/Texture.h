#pragma once
#include "upng.h"
#include <cstdint>

struct tex2_t
{
	float u, v;

	tex2_t operator-(const tex2_t & other) const
	{
		return {.u = u - other.u, .v = v - other.v};
	}

	tex2_t operator+(const tex2_t & other) const
	{
		return {.u = u + other.u, .v = v + other.v};
	}

	tex2_t operator*(float scalar) const
	{
		return {.u = u * scalar, .v = v * scalar};
	}

	static tex2_t lerp(const tex2_t & a, const tex2_t & b, float t)
	{
		return {a.u + t * (b.u - a.u), a.v + t * (b.v - a.v)};
	}
};

extern int texture_width;
extern int texture_height;
extern const uint8_t REDBRICK_TEXTURE[];
extern const uint32_t * mesh_texture;
extern const uint32_t * mesh_texture_debug;
// upng
extern upng_t * png_texture;
void load_png_texture_data(const char * filename);
#pragma once
#include <cstdint>

struct png_texture_t
{
	int width;
	int height;
	int channels;
	uint32_t * texture;
};

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
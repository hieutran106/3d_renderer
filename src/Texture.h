#pragma once
#include "upng.h"
#include <cstdint>

struct tex2_t
{
	float u, v;
};

extern int texture_width;
extern int texture_height;
extern const uint8_t REDBRICK_TEXTURE[];
extern const uint32_t * mesh_texture;
extern const uint32_t * mesh_texture_debug;
// upng
extern upng_t * png_texture;
void load_png_texture_data(const char * filename);
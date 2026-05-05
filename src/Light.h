#pragma once
#include "Vector.h"
#include <cstdint>

struct light_t
{
	vec3_t direction;
};

// extern light_t light;
void init_light(vec3_t direction);
vec3_t get_light_direction();
uint32_t light_apply_intensity(uint32_t color, float percent);

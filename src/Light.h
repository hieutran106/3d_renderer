#pragma once
#include "Vector.h"
#include <cstdint>

struct light_t
{
	vec3_t direction;
};

extern light_t light;
uint32_t light_apply_intensity(uint32_t color, float percent);

#pragma once
#include "Vector.h"

struct camera_t
{
	vec3_t position;
	vec3_t direction;
};

extern camera_t camera;
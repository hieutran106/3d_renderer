#pragma once
#include "Vector.h"

struct camera_t
{
	vec3_t position;
	vec3_t direction;
	vec3_t forward_velocity;
	// Rotation around y-axis, angle in radian
	float yaw;
};

extern camera_t camera;
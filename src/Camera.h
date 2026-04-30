#pragma once
#include "Vector.h"

struct camera_t
{
	vec3_t position;
	vec3_t direction;
	vec3_t forward_velocity;
	// Movement: turning left or right, Rotation around vertical axis, angle in radian
	float yaw;
};

extern camera_t camera;
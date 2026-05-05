#pragma once
#include "Vector.h"

struct camera_t
{
	vec3_t position;
	vec3_t direction;
	vec3_t forward_velocity;
	// Movement: turning left or right, Rotation around vertical axis, angle in radian
	float yaw;
	float pitch;
};

void init_camera(vec3_t position, vec3_t direction);

vec3_t get_camera_position();
vec3_t get_camera_direction();
vec3_t get_camera_forward_velocity();
float get_camera_yaw();
float get_camera_pitch();

void update_camera_position(vec3_t position);
void update_camera_direction(vec3_t direction);
void update_camera_forward_velocity(vec3_t forward_velocity);

void rotate_camera_yaw(float angle);
void rotate_camera_pitch(float angle);

vec3_t get_camera_lookat_target();
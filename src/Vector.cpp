#include "Vector.h"

#include <cmath>

vec2_t vec2_new(float x, float y)
{
	vec2_t result = {x, y};
	return result;
}
float vec2_length(vec2_t v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}
vec2_t vec2_add(vec2_t a, vec2_t b)
{
	return {.x = a.x + b.x, .y = a.y + b.y};
}
vec2_t vec2_sub(vec2_t a, vec2_t b)
{
	return {.x = a.x - b.x, .y = a.y - b.y};
}
vec2_t vec2_mul(vec2_t v, float factor)
{
	return {.x = v.x * factor, .y = v.y * factor};
}
vec2_t vec2_div(vec2_t v, float factor)
{
	return {.x = v.x / factor, .y = v.y / factor};
}
vec2_t vec2_from_vec4(vec4_t v)
{
	return {.x = v.x, .y = v.y};
}
float vec2_cross(vec2_t a, vec2_t b)
{
	return a.x * b.y - a.y * b.x;
}
void vec2_normalize(vec2_t * v)
{
	const float len = vec2_length(*v);
	v->x = v->x / len;
	v->y = v->y / len;
}
vec3_t vec3_new(float x, float y, float z)
{
	vec3_t result = {x, y, z};
	return result;
}

////////////////////////////////////////////////
// Implementations of Vector 3D functions
////////////////////////////////////////////////
float vec3_length(vec3_t v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}
vec3_t vec3_add(vec3_t a, vec3_t b)
{
	return {.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}
vec3_t vec3_sub(vec3_t a, vec3_t b)
{
	return {.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}
vec3_t vec3_mul(vec3_t v, float factor)
{
	return {.x = v.x * factor, .y = v.y * factor, .z = v.z * factor};
}
vec3_t vec3_div(vec3_t v, float factor)
{
	return {.x = v.x / factor, .y = v.y / factor, .z = v.z / factor};
}
vec3_t vec3_cross(vec3_t a, vec3_t b)
{
	// This project uses left-handed coordinate system
	vec3_t result = {.x = a.y * b.z - a.z * b.y, .y = a.z * b.x - a.x * b.z, .z = a.x * b.y - a.y * b.x};
	return result;
}
float vec3_dot(vec3_t a, vec3_t b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
void vec3_normalize(vec3_t * v)
{
	const float len = vec3_length(*v);
	v->x = v->x / len;
	v->y = v->y / len;
	v->z = v->z / len;
}
vec3_t vec3_rotate_x(vec3_t v, float angle)
{
	using std::cos, std::sin;
	vec3_t rotated_vector = {.x = v.x, .y = v.y * cos(angle) - v.z * sin(angle), .z = v.y * sin(angle) + v.z * cos(angle)};
	return rotated_vector;
}

vec3_t vec3_rotate_y(vec3_t v, float angle)
{
	using std::cos, std::sin;
	vec3_t rotated_vector = {.x = v.x * cos(angle) - v.z * sin(angle), .y = v.y, .z = v.x * sin(angle) + v.z * cos(angle)};
	return rotated_vector;
}

vec3_t vec3_rotate_z(vec3_t v, float angle)
{
	using std::cos, std::sin;
	vec3_t rotated_vector = {.x = v.x * cos(angle) - v.y * sin(angle), .y = v.x * sin(angle) + v.y * cos(angle), .z = v.z};
	return rotated_vector;
}

vec3_t barycentric_weights(vec2_t a, vec2_t b, vec2_t c, vec2_t p)
{
	float abc = std::abs(vec2_cross(vec2_sub(c, a), vec2_sub(b, a)));
	float pbc = std::abs(vec2_cross(vec2_sub(c, p), vec2_sub(b, p)));

	float apc = std::abs(vec2_cross(vec2_sub(a, p), vec2_sub(c, p)));

	float alpha = pbc / abc;
	float beta = apc / abc;
	float gamma = 1 - alpha - beta;
	return {alpha, beta, gamma};
}

vec4_t vec4_from_vec3(vec3_t v)
{
	return {.x = v.x, .y = v.y, .z = v.z, .w = 1};
}
vec3_t vec3_from_vec4(vec4_t v)
{
	return {v.x, v.y, v.z};
}

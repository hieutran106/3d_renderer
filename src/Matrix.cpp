#include "Matrix.h"

mat4_t mat4_identity()
{
	mat4_t m = {
		.m = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}}
	};
	return m;
}
mat4_t mat4_make_scale(float sx, float sy, float sz)
{
	mat4_t m = {
		.m = {{sx, 0, 0, 0}, {0, sy, 0, 0}, {0, 0, sz, 0}, {0, 0, 0, 1}}
	};
	return m;
}
vec4_t mat4_mul_vec4(mat4_t m1, vec4_t v)
{
	vec4_t result;
	auto m = m1.m;
	result.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w;
	result.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w;
	result.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w;
	result.w = m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w;

	return result;
}

#pragma once
#include "Vector.h"

struct mat4_t
{
	float m[4][4];

	mat4_t operator*(const mat4_t & b) const
	{
		mat4_t result;
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 4; j++)
			{
				result.m[i][j] = m[i][0] * b.m[0][j] + m[i][1] * b.m[1][j] + m[i][2] * b.m[2][j] + m[i][3] * b.m[3][j];
			}
		}
		return result;
	}
};

mat4_t mat4_identity();
mat4_t mat4_make_scale(float sx, float sy, float sz);
mat4_t mat4_make_translation(float tx, float ty, float tz);
mat4_t mat4_make_rotation_x(float angle);
mat4_t mat4_make_rotation_y(float angle);
mat4_t mat4_make_rotation_z(float angle);
vec4_t mat4_mul_vec4(mat4_t m, vec4_t v);
mat4_t mat4_mul_mat4(mat4_t a, mat4_t b);
mat4_t mat4_make_perspective(float fov, float aspect, float znear, float zfar);
vec4_t mat4_mul_vec4_project(mat4_t mat_proj, vec4_t v);
mat4_t mat4_look_at(vec3_t eye, vec3_t target, vec3_t up);
#pragma once
#include "Triangle.h"
#include "Vector.h"
#include <array>

enum
{
	LEFT_FRUSTUM_PLANE,
	RIGHT_FRUSTUM_PLANE,
	TOP_FRUSTUM_PLANE,
	BOTTOM_FRUSTUM_PLANE,
	NEAR_FRUSTUM_PLANE,
	FAR_FRUSTUM_PLANE
};

struct plane_t
{
	vec3_t point;
	vec3_t normal;
};

constexpr int MAX_NUM_POLY_TRIANGLES = 10;
constexpr int MAX_NUM_POLY_VERTICES = 10;
struct polygon_t
{
	std::array<vec3_t, MAX_NUM_POLY_VERTICES> vertices;
	int num_vertices;
};

void init_frustum_planes(float fov_x, float fov_y, float z_near, float z_far);
polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2);
void clip_polygon(polygon_t * polygon);
void triangles_from_polygon(polygon_t * polygon, std::array<triangle_t, MAX_NUM_POLY_TRIANGLES> & outTriangles, int & num_triangles_after_clipping);
#include "Clipping.h"
#include <cmath>

constexpr int NUM_PLANES = 6;
plane_t frustum_planes[NUM_PLANES];

void init_frustum_planes(float fov, float z_near, float z_far)
{
	float cos_half_fov = std::cos(fov / 2);
	float sin_half_fov = std::sin(fov / 2);

	frustum_planes[LEFT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.x = cos_half_fov;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.z = sin_half_fov;

	frustum_planes[RIGHT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.x = -cos_half_fov;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.z = sin_half_fov;

	frustum_planes[TOP_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[TOP_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.y = -cos_half_fov;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.z = sin_half_fov;

	frustum_planes[BOTTOM_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y = cos_half_fov;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.z = sin_half_fov;

	frustum_planes[NEAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_near);
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.z = 1;

	frustum_planes[FAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_far);
	frustum_planes[FAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.z = -1;
}

polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2)
{
	polygon_t result = {
		.vertices = {v0, v1, v2},
          .num_vertices = 3
	};
	return result;
}

void clip_polygon_against_plane(polygon_t * polygon, int plane)
{
	// Hieu add check
	if(polygon->num_vertices == 0)
	{
		return;
	}
	auto [plane_point, plane_normal] = frustum_planes[plane];

	std::array<vec3_t, MAX_NUM_POLY_VERTICES> inside_vertices;
	int num_inside_vertices = 0;

	vec3_t * current_vertex = &polygon->vertices[0];
	vec3_t * previous_vertex = &polygon->vertices[polygon->num_vertices - 1];

	float current_dot = 0;
	float previous_dot = vec3_dot(*previous_vertex - plane_point, plane_normal);
	// Loop while the current vertex pointer is different from the last vertex
	while(current_vertex != &polygon->vertices[polygon->num_vertices])
	{
		current_dot = vec3_dot(*current_vertex - plane_point, plane_normal);
		// If we change from inside to outside or vice versa
		if(current_dot * previous_dot < 0)
		{
			// Calculate the interpolation factor t, t = dotQp/(dotQp - dotQc)
			float t = previous_dot / (previous_dot - current_dot);
			// Calculate the intersection point, I = Qp + t(Qc-Qp)
			vec3_t intersection_point = *previous_vertex + (*current_vertex - *previous_vertex) * t;
			// Insert the new intersection point in the list of inside vertices
			inside_vertices[num_inside_vertices] = vec3_clone(&intersection_point);
			num_inside_vertices++;
		}
		// If the current point is inside the plane
		if(current_dot > 0)
		{
			inside_vertices[num_inside_vertices] = vec3_clone(current_vertex);
			num_inside_vertices++;
		}

		// Move to the next vertex
		previous_dot = current_dot;
		previous_vertex = current_vertex;
		current_vertex++;
	}
	// Override inside_vertices into polygon
	for(int i = 0; i < num_inside_vertices; i++)
	{
		polygon->vertices[i] = vec3_clone(&inside_vertices[i]);
	}
	polygon->num_vertices = num_inside_vertices;
}
void clip_polygon(polygon_t * polygon)
{
	clip_polygon_against_plane(polygon, LEFT_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, RIGHT_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, TOP_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, BOTTOM_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, NEAR_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, FAR_FRUSTUM_PLANE);
}

void triangles_from_polygon(polygon_t * polygon, std::array<triangle_t, MAX_NUM_POLY_TRIANGLES> & outTriangles, int & num_triangles_after_clipping)
{
	for(int i = 0; i < polygon->num_vertices - 2; i++)
	{
		int index0 = 0;
		int index1 = i + 1;
		int index2 = i + 2;
		outTriangles[i].points[0] = vec4_from_vec3(polygon->vertices[index0]);
		outTriangles[i].points[1] = vec4_from_vec3(polygon->vertices[index1]);
		outTriangles[i].points[2] = vec4_from_vec3(polygon->vertices[index2]);
	}
	num_triangles_after_clipping = polygon->num_vertices - 2;
}

#include "Clipping.h"
#include <cmath>

constexpr int NUM_PLANES = 6;
plane_t frustum_planes[NUM_PLANES];

void init_frustum_planes(float fov_x, float fov_y, float z_near, float z_far)
{
	float cos_half_fov_x = std::cos(fov_x / 2);
	float sin_half_fov_x = std::sin(fov_x / 2);

	float cos_half_fov_y = std::cos(fov_y / 2);
	float sin_half_fov_y = std::sin(fov_y / 2);

	frustum_planes[LEFT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.x = cos_half_fov_x;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.z = sin_half_fov_x;

	frustum_planes[RIGHT_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.x = -cos_half_fov_x;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.z = sin_half_fov_x;

	frustum_planes[TOP_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[TOP_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.y = -cos_half_fov_y;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.z = sin_half_fov_y;

	frustum_planes[BOTTOM_FRUSTUM_PLANE].point = vec3_new(0, 0, 0);
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y = cos_half_fov_y;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.z = sin_half_fov_y;

	frustum_planes[NEAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_near);
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.z = 1;

	frustum_planes[FAR_FRUSTUM_PLANE].point = vec3_new(0, 0, z_far);
	frustum_planes[FAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.z = -1;
}

polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2, tex2_t t0, tex2_t t1, tex2_t t2)
{
	polygon_t result = {
		.vertices = {v0, v1, v2},
          .texcoords = {t0, t1, t2},
          .num_vertices = 3
	};
	return result;
}
float float_lerp(float a, float b, float t)
{
	return a + t * (b - a);
}
void clip_polygon_against_plane(polygon_t * polygon, int plane)
{
	// HT: add boundary check
	if(polygon->num_vertices == 0)
	{
		return;
	}
	auto [plane_point, plane_normal] = frustum_planes[plane];

	std::array<vec3_t, MAX_NUM_POLY_VERTICES> inside_vertices;
	std::array<tex2_t, MAX_NUM_POLY_VERTICES> inside_texcoords;
	int num_inside_vertices = 0;

	vec3_t * current_vertex = &polygon->vertices[0];
	tex2_t * current_texcoord = &polygon->texcoords[0];

	vec3_t * previous_vertex = &polygon->vertices[polygon->num_vertices - 1];
	tex2_t * previous_texcoord = &polygon->texcoords[polygon->num_vertices - 1];

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
			// vec3_t intersection_point = *previous_vertex + (*current_vertex - *previous_vertex) * t;
			vec3_t intersection_point = vec3_t::lerp(*previous_vertex, *current_vertex, t);
			// Linear interpolation
			tex2_t interpolated_texcoord = tex2_t::lerp(*previous_texcoord, *current_texcoord, t);
			// Insert the new intersection point in the list of inside vertices
			inside_vertices[num_inside_vertices] = intersection_point;
			inside_texcoords[num_inside_vertices] = interpolated_texcoord;
			num_inside_vertices++;
		}
		// If the current point is inside the plane
		if(current_dot > 0)
		{
			inside_vertices[num_inside_vertices] = *current_vertex;
			inside_texcoords[num_inside_vertices] = *current_texcoord;
			num_inside_vertices++;
		}

		// Move to the next vertex
		previous_dot = current_dot;
		previous_vertex = current_vertex;
		previous_texcoord = current_texcoord;
		current_vertex++;
		current_texcoord++;
	}
	// Override inside_vertices into polygon
	for(int i = 0; i < num_inside_vertices; i++)
	{
		polygon->vertices[i] = inside_vertices[i];
		polygon->texcoords[i] = inside_texcoords[i];
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
		// uv
		outTriangles[i].texcoords[0] = polygon->texcoords[index0];
		outTriangles[i].texcoords[1] = polygon->texcoords[index1];
		outTriangles[i].texcoords[2] = polygon->texcoords[index2];
	}
	num_triangles_after_clipping = polygon->num_vertices - 2;
}

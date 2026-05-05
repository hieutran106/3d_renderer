#include "Array.h"
#include "Camera.h"
#include "Clipping.h"
#include "Display.h"
#include "Light.h"
#include "Logger.h"
#include "Matrix.h"
#include "Mesh.h"
#include "Profiler/ProfileTimer.h"
#include "Profiler/Profiler.h"
#include "Vector.h"
#include <numbers>
#include <stdlib.h>
#include <string>

bool is_running = false;
bool is_paused = false;

bool rotate_x = false;
bool rotate_y = false;
bool rotate_z = false;
vec3_t mesh_rotation;

/////////////////////////////////////////////////////////////////////////////////////
/// Global matrices
/////////////////////////////////////////////////////////////////////////////////////
mat4_t projection_matrix;
mat4_t world_matrix;
mat4_t view_matrix;

int previous_frame_time = 0;

namespace helper
{
mat4_t initializePerspectiveProjectionMatrix()
{
	float degress = 60.0;
	float fov_radians = degress * (std::numbers::pi / 180.0);
	float aspect = static_cast<float>(window_height) / window_width;
	float znear = 0.1;
	float zfar = 100.0;
	return mat4_make_perspective(fov_radians, aspect, znear, zfar);
}

mat4_t initializeTransformationMatrix(mesh_t & mesh)
{
	auto scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
	auto rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
	auto rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
	auto rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
	auto translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

	// Create a World Matrix combining scale, rotation, and translation matrices
	// Order matters: First scale, then rotate, then translate. [T]*[R]*[S]*v
	mat4_t world_matrix = translation_matrix * rotation_matrix_x * rotation_matrix_y * rotation_matrix_z * scale_matrix;
	return world_matrix;
}

vec3_t getFaceNormalVector(const std::array<vec4_t, 3> & transformed_vertices)
{
	vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
	vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
	vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);

	auto vector_ab = vector_b - vector_a;
	auto vector_ac = vector_c - vector_a;
	vec3_t normal = vec3_cross(vector_ab, vector_ac);

	// Normalize the face normal vector - in-place update
	vec3_normalize(&normal);
	return normal;
}

std::array<vec4_t, 3> projectTriangle(const mat4_t & projection_matrix, const std::array<vec4_t, 3> & triangle)
{
	std::array<vec4_t, 3> projected_points{};
	for(int j = 0; j < 3; j++)
	{
		// Project the current point
		projected_points[j] = mat4_mul_vec4_project(projection_matrix, triangle[j]);
		// Scale into the view
		projected_points[j].x *= window_width / 2.0;
		projected_points[j].y *= window_height / 2.0;
		// Invert the y value to account for flipped screen y coordinate
		projected_points[j].y *= -1;
		// Translate the projected points to the middle of the screen
		projected_points[j].x += window_width / 2.0;
		projected_points[j].y += window_height / 2.0;
	}
	return projected_points;
}

void updateMeshAnimation(mesh_t & mesh, float deltaTime)
{
	if(rotate_x)
	{
		mesh.rotation.x += 1.0 * deltaTime;
	}
	if(rotate_y)
	{
		mesh.rotation.y += 1.0 * deltaTime;
	}
	if(rotate_z)
	{
		mesh.rotation.z += 1.0 * deltaTime;
	}
	mesh.translation.z = 6;
}

void setupMeshRotation(vec3_t & mesh_rotation, float x, float y, float z)
{
	if(x > 0)
	{
		rotate_x = true;
		mesh_rotation.x = x;
	}
	if(y > 0)
	{
		rotate_y = true;
		mesh_rotation.y = y;
	}
	if(z > 0)
	{
		rotate_z = true;
		mesh_rotation.z = z;
	}
}
}

void setup()
{
	// Initialize render mode and triangle culling mode
	render_method = RENDER_TEXTURED;
	cull_method = CULL_BACKFACE;

	helper::setupMeshRotation(mesh_rotation, 0.0, 0.0, 0.0);

	// Initialize the scene light direction
	vec3_t light_direction = {0, 0, 1};
	init_light(light_direction);

	// Initialize the perspective projection matrix
	float aspecty = static_cast<float>(window_height) / window_width;
	float aspectx = static_cast<float>(window_width) / window_height;

	float fov_y_degrees = 60.0;
	float fov_y_radians = fov_y_degrees * (std::numbers::pi / 180.0);
	// https://en.wikipedia.org/wiki/Field_of_view_in_video_games
	float fov_x_radians = 2 * std::atan(std::tan(fov_y_radians / 2) * aspectx);

	float z_near = 0.1;
	float z_far = 20.0;
	projection_matrix = mat4_make_perspective(fov_y_radians, aspecty, z_near, z_far);

	// -- Initialize frustum planes with a point and normal
	init_frustum_planes(fov_x_radians, fov_y_radians, z_near, z_far);

	// load_cube_mesh_data();
	load_obj_file_data("../assets/f117.obj");

	// Load the texture information from an external PNG file
	load_png_texture_data("../assets/f117.png");
}

void process_input()
{
	float deltaTimeMs = static_cast<float>(deltaTime) / 1000.0f;
	SDL_Event event;
	SDL_PollEvent(&event);
	switch(event.type)
	{
		case SDL_EVENT_QUIT:
			is_running = false;
			break;
		case SDL_EVENT_KEY_DOWN:
			if(event.key.key == SDLK_ESCAPE)
			{
				is_running = false;
			}
			else if(event.key.key == SDLK_1)
			{
				render_method = render_method == RENDER_WIRE_VERTEX ? RENDER_WIRE : RENDER_WIRE_VERTEX;
			}
			else if(event.key.key == SDLK_2)
			{
				render_method = render_method == RENDER_FILL_TRIANGLE ? RENDER_FILL_TRIANGLE_WIRE : RENDER_FILL_TRIANGLE;
			}
			else if(event.key.key == SDLK_3)
			{
				render_method = render_method == RENDER_TEXTURED ? RENDER_TEXTURED_WIRE : RENDER_TEXTURED;
			}
			else if(event.key.key == SDLK_C)
			{
				cull_method = cull_method == CULL_BACKFACE ? CULL_NONE : CULL_BACKFACE;
			}
			else if(event.key.key == SDLK_SPACE)
			{
				is_paused = !is_paused;
			}
			else if(event.key.key == SDLK_X)
			{
				rotate_x = !rotate_x;
			}
			else if(event.key.key == SDLK_Y)
			{
				rotate_y = !rotate_y;
			}
			else if(event.key.key == SDLK_Z)
			{
				rotate_z = !rotate_z;
			}
			else if(event.key.key == SDLK_W)
			{
				rotate_camera_pitch(3.0 * deltaTimeMs);
			}
			else if(event.key.key == SDLK_S)
			{
				rotate_camera_pitch(-3.0 * deltaTimeMs);
			}
			else if(event.key.key == SDLK_RIGHT)
			{
				rotate_camera_yaw(1.0 * deltaTimeMs);
			}
			else if(event.key.key == SDLK_LEFT)
			{
				rotate_camera_yaw(-1.0 * deltaTimeMs);
			}
			else if(event.key.key == SDLK_UP)
			{
				update_camera_forward_velocity(vec3_mul(get_camera_direction(), 10.0 * deltaTimeMs));
				update_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
			}
			else if(event.key.key == SDLK_DOWN)
			{
				update_camera_forward_velocity(vec3_mul(get_camera_direction(), 10.0 * deltaTimeMs));
				update_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
			}
			break;
	}
}

void update()
{
	PROFILE_FUNCTION();
	const auto current_time = SDL_GetTicks();
	deltaTime = current_time - previous_frame_time;
	int time_to_wait = FRAME_TARGET_TIME - deltaTime;

	SDL_LogDebug(MY_LOG_RENDER, "deltaTime=%d time_to_wait=%d", deltaTime, time_to_wait);

	if(time_to_wait > 0)
	{
		SDL_Delay(time_to_wait);
	}

	previous_frame_time = current_time;
	if(is_paused)
	{
		return;
	}

	float deltaTimeMs = static_cast<float>(deltaTime) / 1000.0f;
	// Initialize the counter of triangles to render for the current frame
	num_triangles_to_render = 0;

	helper::updateMeshAnimation(mesh, deltaTimeMs);

	// Update camera look at target to create view matrix
	vec3_t target = get_camera_lookat_target();
	vec3_t up_direction = {0, 1, 0};
	view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

	mat4_t world_matrix = helper::initializeTransformationMatrix(mesh);

	int num_faces = array_length(mesh.faces);
	for(int i = 0; i < num_faces; i++)
	{
		// if(i != 4)
		// 	continue;
		const face_t & mesh_face = mesh.faces[i];

		const vec3_t face_vertices[3] = {mesh.vertices[mesh_face.a - 1], mesh.vertices[mesh_face.b - 1], mesh.vertices[mesh_face.c - 1]};

		std::array<vec4_t, 3> transformed_vertices;
		// Loop all three vertices of this current face and apply transformation
		for(int j = 0; j < 3; j++)
		{
			vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);
			transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);
			// Transform the world space to camera space
			transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);
			transformed_vertices[j] = transformed_vertex;
		}

		vec3_t normal = helper::getFaceNormalVector(transformed_vertices);
		// Check backface culling test to see if the current face should be projected
		if(cull_method == CULL_BACKFACE)
		{
			vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
			// Find the vector between a point in the triangle and the camera origin
			vec3_t origin = {0, 0, 0};
			vec3_t camera_ray = origin - vector_a;

			// Calculate how aligned the camera ray is with the face normal (Using dot product)
			auto culling = vec3_dot(normal, camera_ray);
			if(culling < 0)
			{
				continue;
			}
		}

		// Create a polygon from the original transformed triangle to be clipped
		polygon_t polygon = create_polygon_from_triangle(
			vec3_from_vec4(transformed_vertices[0]),
			vec3_from_vec4(transformed_vertices[1]),
			vec3_from_vec4(transformed_vertices[2]),
			mesh_face.a_uv,
			mesh_face.b_uv,
			mesh_face.c_uv
		);
		// Clip the polygon and returns a new polygon with potential new vertices
		clip_polygon(&polygon);

		std::array<triangle_t, MAX_NUM_POLY_TRIANGLES> triangles_after_clipping;
		int num_triangles_after_clipping = 0;

		triangles_from_polygon(&polygon, triangles_after_clipping, num_triangles_after_clipping);

		for(int t = 0; t < num_triangles_after_clipping; t++)
		{
			triangle_t triangle = triangles_after_clipping[t];
			auto triangle_points = triangle.points;
			// Loop all three vertices to perform a projection
			std::array<vec4_t, 3> projected_points = helper::projectTriangle(projection_matrix, triangle_points);

			////////////////////////////////////////////////////////////////////////////////////////////////
			/// Flat shading and light
			// Calculate the shade intensity based on how aligned is the face normal and the opposite of the light direction
			float light_intensity_factor = -vec3_dot(normal, get_light_direction());

			// Calculate the triangle color based on the light angle
			uint32_t shading = light_apply_intensity(mesh_face.color, light_intensity_factor);

			triangle_t triangle_to_render = {.color = shading, .points = projected_points, .texcoords = triangle.texcoords};
			// Save the projected triangle in the array of triangles to render
			triangles_to_render[num_triangles_to_render] = triangle_to_render;
			num_triangles_to_render++;
		}
	}
}
void render_scene_to_buffer()
{
	if(is_paused)
	{
		return;
	}

	clear_color_buffer(0xFF000000);
	clear_z_buffer();
	// draw_grid();
	// Loop all projected triangles and render them
	for(int i = 0; i < num_triangles_to_render; i++)
	{
		const triangle_t & triangle = triangles_to_render[i];
		auto & points = triangle.points;
		// Draw filled triangle
		if(render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
		{
			draw_filled_triangle(
				triangle.points[0].x,
				triangle.points[0].y,
				triangle.points[0].z,
				triangle.points[0].w, // vertex A
				triangle.points[1].x,
				triangle.points[1].y,
				triangle.points[1].z,
				triangle.points[1].w, // vertex B
				triangle.points[2].x,
				triangle.points[2].y,
				triangle.points[2].z,
				triangle.points[2].w, // vertex C
				triangle.color
			);
		}
		if(render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE)
		{
			auto & texcoords = triangle.texcoords;
			draw_textured_triangle(
				points[0].x,
				points[0].y,
				points[0].z,
				points[0].w,
				texcoords[0].u,
				texcoords[0].v,
				points[1].x,
				points[1].y,
				points[1].z,
				points[1].w,
				texcoords[1].u,
				texcoords[1].v,
				points[2].x,
				points[2].y,
				points[2].z,
				points[2].w,
				texcoords[2].u,
				texcoords[2].v,
				mesh_texture
			);
		}
		// Draw triangle wireframe
		if(render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX || render_method == RENDER_FILL_TRIANGLE_WIRE
		   || render_method == RENDER_TEXTURED_WIRE)
		{
			draw_triangle(
				triangle.points[0].x,
				triangle.points[0].y, // vertex A
				triangle.points[1].x,
				triangle.points[1].y, // vertex B
				triangle.points[2].x,
				triangle.points[2].y, // vertex C
				0xFFDDDDDD
			);
		}
		// Draw triangle vertex points
		if(render_method == RENDER_WIRE_VERTEX)
		{
			draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000); // vertex A
			draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000); // vertex B
			draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000); // vertex C
		}
	}
}
void render()
{
	PROFILE_FUNCTION();
	render_scene_to_buffer();
	render_color_buffer();
	render_stats_text();
	SDL_RenderPresent(renderer);
}

void free_resource()
{
	free_display_resource();
	upng_free(png_texture);
	array_free(mesh.faces);
	array_free(mesh.vertices);
}

int main(int argc, char * argv[])
{
	PROFILE_FUNCTION();
	SDL_SetLogPriorities(SDL_LOG_PRIORITY_INFO);
	is_running = initialize_window();
	setup();

	while(is_running)
	{
		process_input();
		update();
		render();
	}
	destroy_window();
	free_resource();
	return 0;
}

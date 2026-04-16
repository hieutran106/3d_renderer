#include "Array.h"
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

constexpr int MAX_TRIANGLE_PER_MESH = 10000;
triangle_t triangles_to_render[MAX_TRIANGLE_PER_MESH];
int num_triangles_to_render = 0;

vec3_t camera_position = {0, 0, 0};

bool is_running = false;
bool is_paused = false;
bool rotate_x = true;
bool rotate_y = false;
bool rotate_z = false;

mat4_t projection_matrix;

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

std::array<vec4_t, 3> projectVertices(const mat4_t & projection_matrix, const std::array<vec4_t, 3> & transformed_vertices)
{
	std::array<vec4_t, 3> projected_points{};
	for(int j = 0; j < 3; j++)
	{
		// Project the current point
		projected_points[j] = mat4_mul_vec4_project(projection_matrix, transformed_vertices[j]);
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
}

void setup()
{
	// Initialize render mode and triangle culling mode
	render_method = RENDER_FILL_TRIANGLE;
	cull_method = CULL_NONE;

	color_buffer = new uint32_t[window_width * window_height];
	color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
	z_buffer = new float[window_width * window_height];

	// Initialize perspective projection matrix
	projection_matrix = helper::initializePerspectiveProjectionMatrix();
	// load_cube_mesh_data();
	load_obj_file_data("../assets/f117.obj");

	// Load the texture information from an external PNG file
	load_png_texture_data("../assets/f117.png");
}

void process_input()
{
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
				render_method = RENDER_WIRE_VERTEX;
			}
			else if(event.key.key == SDLK_2)
			{
				render_method = RENDER_WIRE;
			}
			else if(event.key.key == SDLK_3)
			{
				render_method = RENDER_FILL_TRIANGLE;
			}
			else if(event.key.key == SDLK_4)
			{
				render_method = RENDER_FILL_TRIANGLE_WIRE;
			}
			else if(event.key.key == SDLK_5)
			{
				render_method = render_method == RENDER_TEXTURED ? RENDER_TEXTURED_WIRE : RENDER_TEXTURED;
			}
			else if(event.key.key == SDLK_C)
			{
				cull_method = cull_method == CULL_BACKFACE ? CULL_NONE : CULL_BACKFACE;
			}
			else if(event.key.key == SDLK_P)
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
			break;
	}
}

void update()
{
	PROFILE_FUNCTION();
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
	// time_to_wait <= FRAME_TARGET_TIME is mostly useless,
	if(time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
	{
		SDL_Delay(time_to_wait);
	}

	if(is_paused)
	{
		return;
	}
	previous_frame_time = SDL_GetTicks();

	// Initialize the counter of triangles to render for the current frame
	num_triangles_to_render = 0;

	if(rotate_x)
	{
		mesh.rotation.x += 0.02;
	}
	if(rotate_y)
	{
		mesh.rotation.y += 0.01;
	}

	if(rotate_z)
	{
		mesh.rotation.z += 0.01;
	}

	mesh.translation.z = 5;

	mat4_t world_matrix = helper::initializeTransformationMatrix(mesh);

	int num_faces = array_length(mesh.faces);
	for(int i = 0; i < num_faces; i++)
	{
		face_t mesh_face = mesh.faces[i];

		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		std::array<vec4_t, 3> transformed_vertices;
		// Loop all three vertices of this current face and apply transformation
		for(int j = 0; j < 3; j++)
		{
			vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);
			transformed_vertices[j] = mat4_mul_vec4(world_matrix, transformed_vertex);
		}

		vec3_t normal = helper::getFaceNormalVector(transformed_vertices);
		// Check backface culling test to see if the current face should be projected
		if(cull_method == CULL_BACKFACE)
		{
			vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
			// Find the vector between a point in the triangle and the camera origin
			auto camera_ray = camera_position - vector_a;

			// Calculate how aligned the camera ray is with the face normal (Using dot product)
			auto culling = vec3_dot(normal, camera_ray);
			if(culling < 0)
			{
				continue;
			}
		}

		// Loop all three vertices to perform a projection
		std::array<vec4_t, 3> projected_points = helper::projectVertices(projection_matrix, transformed_vertices);

		////////////////////////////////////////////////////////////////////////////////////////////////
		/// Flat shading and light
		// Calculate the shade intensity based on how aligned is the face normal and the opposite of the light direction
		float light_intensity_factor = -vec3_dot(normal, light.direction);

		// Calculate the triangle color based on the light angle
		uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

		triangle_t projected_triangle = {
			.color = triangle_color,
			.points = projected_points,
			.texcoords = {
						  {mesh_face.a_uv.u, mesh_face.a_uv.v}, {mesh_face.b_uv.u, mesh_face.b_uv.v}, {mesh_face.c_uv.u, mesh_face.c_uv.v}

			}
		};
		// Save the projected triangle in the array of triangles to render
		triangles_to_render[num_triangles_to_render] = projected_triangle;
		num_triangles_to_render++;
	}
}
void render()
{
	PROFILE_FUNCTION();
	clear_color_buffer(0xFF000000);
	clear_z_buffer();
	draw_grid();
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

	// array_free(triangles_to_render);
	render_color_buffer();
	render_text();
	SDL_RenderPresent(renderer);
}

void free_resource()
{
	delete[] color_buffer;
	delete[] z_buffer;
	upng_free(png_texture);
	array_free(mesh.faces);
	array_free(mesh.vertices);
}

int main(int argc, char * argv[])
{
	PROFILE_FUNCTION();
	SDL_SetLogPriorities(SDL_LOG_PRIORITY_DEBUG);
	SDL_SetLogPriority(MY_LOG_OBJ, SDL_LOG_PRIORITY_DEBUG);
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

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

triangle_t * triangles_to_render = nullptr;

vec3_t camera_position = {0, 0, 0};

bool is_running = false;
bool is_paused = false;

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
	auto rotation_matrix_y = mat4_make_rotation_x(mesh.rotation.y);
	auto rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
	auto translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

	// Create a World Matrix combining scale, rotation, and translation matrices
	// Order matters: First scale, then rotate, then translate. [T]*[R]*[S]*v
	mat4_t world_matrix = translation_matrix * rotation_matrix_x * rotation_matrix_y * rotation_matrix_z * scale_matrix;
	return world_matrix;
}

vec3_t getFaceNormalVector(const vec4_t (&transformed_vertices)[3])
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

std::array<vec4_t, 3> projectVertices(const mat4_t & projection_matrix, const vec4_t (&transformed_vertices)[3])
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
	render_method = RENDER_WIRE;
	cull_method = CULL_BACKFACE;

	color_buffer = new uint32_t[window_width * window_height];
	color_buffer_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

	// Initialize perspective projection matrix
	projection_matrix = helper::initializePerspectiveProjectionMatrix();
	// Manually load the hardcoded texture data from the static array
	mesh_texture = reinterpret_cast<const uint32_t *>(REDBRICK_TEXTURE);
	texture_width = 64;
	texture_height = 64;

	load_cube_mesh_data();
	// load_obj_file_data("../assets/f22.obj");
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
				render_method = RENDER_TEXTURED;
			}
			else if(event.key.key == SDLK_6)
			{
				render_method = RENDER_TEXTURED_WIRE;
			}
			else if(event.key.key == SDLK_C)
			{
				cull_method = CULL_BACKFACE;
			}
			else if(event.key.key == SDLK_D)
			{
				cull_method = CULL_NONE;
			}
			else if(event.key.key == SDLK_P)
			{
				is_paused = !is_paused;
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

	// Initialize the array to triangles to render
	array_free(triangles_to_render);
	triangles_to_render = nullptr;

	mesh.rotation.x += 0.01;
	mesh.rotation.y += 0.01;

	// mesh.translation.x += 0.01;
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

		vec4_t transformed_vertices[3];
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

		// Calculate the avg depth for each face based on the vertices after the transformation
		float avg_depth = (transformed_vertices[0].z + transformed_vertices[1].z + transformed_vertices[2].z) / 3.0f;

		////////////////////////////////////////////////////////////////////////////////////////////////
		/// Flat shading and light
		// Calculate the shade intensity based on how aliged is the face normal and the opposite of the light direction
		float light_intensity_factor = -vec3_dot(normal, light.direction);

		// Calculate the triangle color based on the light angle
		uint32_t triangle_color = light_apply_intensity(mesh_face.color, light_intensity_factor);

		triangle_t projected_triangle = {
			.color = triangle_color,
			.points =
				{
						 {projected_points[0].x, projected_points[0].y},
						 {projected_points[1].x, projected_points[1].y},
						 {projected_points[2].x, projected_points[2].y},

						 },
			.texcoords =
				{
						 {mesh_face.a_uv.u, mesh_face.a_uv.v}, {mesh_face.b_uv.u, mesh_face.b_uv.v}, {mesh_face.c_uv.u, mesh_face.c_uv.v}

				},
			.avg_depth = avg_depth
		};
		// Save the projected triangle in the array of triangles to render
		array_push(triangles_to_render, projected_triangle);
	}
	// Sort the faces to render based on the avg_depth in descending order
	int num_triangles = array_length(triangles_to_render);
	for(int i = 0; i < num_triangles - 1; i++)
	{
		for(int j = i + 1; j < num_triangles; j++)
		{
			if(triangles_to_render[i].avg_depth < triangles_to_render[j].avg_depth)
			{
				// Swap the triangles positions in the array
				triangle_t temp = triangles_to_render[i];
				triangles_to_render[i] = triangles_to_render[j];
				triangles_to_render[j] = temp;
			}
		}
	}
}
void render()
{
	PROFILE_FUNCTION();
	clear_color_buffer(0xFF000000);
	draw_grid();
	// Loop all projected triangles and render them
	int num_faces = array_length(triangles_to_render);
	for(int i = 0; i < num_faces; i++)
	{
		const triangle_t & triangle = triangles_to_render[i];
		auto & points = triangle.points;
		// Draw filled triangle
		if(render_method == RENDER_FILL_TRIANGLE || render_method == RENDER_FILL_TRIANGLE_WIRE)
		{
			draw_filled_triangle(
				triangle.points[0].x,
				triangle.points[0].y, // vertex A
				triangle.points[1].x,
				triangle.points[1].y, // vertex B
				triangle.points[2].x,
				triangle.points[2].y, // vertex C
				triangle.color
			);
		}
		if(render_method == RENDER_TEXTURED || render_method == RENDER_TEXTURED_WIRE)
		{
			auto & texcoords = triangle.texcoords;
			draw_textured_triangle(
				points[0].x,
				points[0].y,
				texcoords[0].u,
				texcoords[0].v,
				points[1].x,
				points[1].y,
				texcoords[1].u,
				texcoords[1].v,
				points[2].x,
				points[2].y,
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

	// Clear the array of triangles to render every frame loop
	// array_free(triangles_to_render);
	render_color_buffer();
	render_text();
	SDL_RenderPresent(renderer);
}

void free_resource()
{
	free(color_buffer);
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

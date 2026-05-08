#include "Camera.h"
#include "Clipping.h"
#include "Display.h"
#include "Light.h"
#include "Logger.h"
#include "Matrix.h"
#include "Mesh.h"
#include "Vector.h"
#include "backends/imgui_impl_sdl3.h"
#include <SDL3/SDL_main.h>
#include <numbers>
#include <stdlib.h>

bool is_running = false;

/// Global matrices
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

	mat4_t initializeTransformationMatrix(const mesh_t & mesh)
	{
		auto scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
		auto rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
		auto rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
		auto rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);
		auto translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);

		// Create a World Matrix combining scale, rotation, and translation matrices
		// Order matters: First scale, then rotate, then translate. [T]*[R]*[S]*v
		mat4_t world_matrix =
			translation_matrix * rotation_matrix_x * rotation_matrix_y * rotation_matrix_z * scale_matrix;
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

	void updateMeshTransformation(mesh_t & mesh, float deltaTime, const AnimationConfig & animConfig)
	{
		const auto & [rotate_x, rotate_y, rotate_z] = animConfig;
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

	/**
	 * @brief Transforms triangle vertices from model space to camera space.
	 *
	 * This function takes a set of 3D vertices and applies the world matrix (to position
	 * the object in the scene) followed by the view matrix (to position the object
	 * relative to the camera).
	 *
	 * @param face_vertices An array of three 3D vectors representing the triangle in model space.
	 * @param world_matrix The transformation matrix for the mesh (scale, rotation, translation).
	 * @param view_matrix The camera's view matrix.
	 * @return std::array<vec4_t, 3> The transformed vertices in camera (view) space.
	 */
	std::array<vec4_t, 3> transformVertices(
		const std::array<vec3_t, 3> & face_vertices,
		const mat4_t & world_matrix,
		const mat4_t & view_matrix
	)
	{
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
		return transformed_vertices;
	}
}

void setup()
{
	// Initialize render mode and triangle culling mode
	render_method = RENDER_TEXTURED;
	cull_method = CULL_BACKFACE;

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

	float z_near = 1.0;
	float z_far = 50.0;
	projection_matrix = mat4_make_perspective(fov_y_radians, aspecty, z_near, z_far);

	// -- Initialize frustum planes with a point and normal
	init_frustum_planes(fov_x_radians, fov_y_radians, z_near, z_far);

	//load_cat_mesh();
	load_runway_scene();
}

void handle_keydown_event(SDL_Keycode key, float deltaTimeMs)
{
	switch(key)
	{
		case SDLK_ESCAPE:
			is_running = false;
			break;
		case SDLK_1:
			render_method = (render_method == RENDER_WIRE_VERTEX) ? RENDER_WIRE : RENDER_WIRE_VERTEX;
			break;
		case SDLK_2:
			render_method = (render_method == RENDER_FILL_TRIANGLE) ? RENDER_FILL_TRIANGLE_WIRE : RENDER_FILL_TRIANGLE;
			break;
		case SDLK_3:
			render_method = (render_method == RENDER_TEXTURED) ? RENDER_TEXTURED_WIRE : RENDER_TEXTURED;
			break;
		case SDLK_C:
			cull_method = (cull_method == CULL_BACKFACE) ? CULL_NONE : CULL_BACKFACE;
			break;
		case SDLK_X:
			toggle_rotation_x();
			break;
		case SDLK_Y:
			toggle_rotation_y();
			break;
		case SDLK_Z:
			toggle_rotation_z();
			break;
		case SDLK_W:
			handle_key_w(deltaTimeMs);
			break;
		case SDLK_S:
			handle_key_s(deltaTimeMs);
			break;
		case SDLK_RIGHT:
			handle_key_right(deltaTimeMs);
			break;
		case SDLK_LEFT:
			handle_key_left(deltaTimeMs);
			break;
		case SDLK_UP:
			handle_key_up(deltaTimeMs);
			break;
		case SDLK_DOWN:
			handle_key_down(deltaTimeMs);
			break;
		default:
			break;
	}
}

void process_input()
{
	float deltaTimeMs = static_cast<float>(deltaTime) / 1000.0f;
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		SDL_ConvertEventToRenderCoordinates(renderer, &event);
		ImGui_ImplSDL3_ProcessEvent(&event);
		if(event.type == SDL_EVENT_QUIT)
		{
			is_running = false;
		}
		else if(event.type == SDL_EVENT_KEY_DOWN)
		{
			handle_keydown_event(event.key.key, deltaTimeMs);
		}
	}
}

void process_graphics_pipeline_stages(const mesh_t & mesh, float deltaTimeMs)
{

	// helper::updateMeshTransformation(mesh, deltaTimeMs, get_amin_config());
	mat4_t world_matrix = helper::initializeTransformationMatrix(mesh);
	for(const face_t & face : mesh.faces)
	{
		std::array<vec3_t, 3> face_vertices = {
			mesh.vertices[face.a - 1], mesh.vertices[face.b - 1], mesh.vertices[face.c - 1]
		};
		std::array<vec4_t, 3> transformed_vertices =
			helper::transformVertices(face_vertices, world_matrix, view_matrix);

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
			face.a_uv,
			face.b_uv,
			face.c_uv
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
			uint32_t shading = light_apply_intensity(face.color, light_intensity_factor);

			triangle_t triangle_to_render = {
				.color = shading,
				.points = projected_points,
				.texcoords = triangle.texcoords,
				.png_texture = mesh.png_texture
			};
			// Save the projected triangle in the array of triangles to render
			if(num_triangles_to_render < MAX_TRIANGLE_PER_MESH)
			{
				triangles_to_render[num_triangles_to_render] = triangle_to_render;
				num_triangles_to_render++;
			}
		}
	}
}

void update()
{
	const auto current_time = SDL_GetTicks();
	deltaTime = current_time - previous_frame_time;
	int time_to_wait = FRAME_TARGET_TIME - deltaTime;

	SDL_LogDebug(MY_LOG_RENDER, "deltaTime=%llu time_to_wait=%d", deltaTime, time_to_wait);

	if(time_to_wait > 0)
	{
		SDL_Delay(time_to_wait);
	}

	previous_frame_time = current_time;

	float deltaTimeMs = static_cast<float>(deltaTime) / 1000.0f;
	handle_touch_controls();
	// Initialize the counter of triangles to render for the current frame
	num_triangles_to_render = 0;

	// Update camera look at target to create view matrix
	vec3_t target = get_camera_lookat_target();
	vec3_t up_direction = {0, 1, 0};
	view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

	for(mesh_t & mesh : get_meshes())
	{
		helper::updateMeshTransformation(mesh, deltaTimeMs, get_amin_config());
		process_graphics_pipeline_stages(mesh, deltaTimeMs);
	}
}
/**
 *
 */
void render_scene_to_buffer()
{

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
				triangle.png_texture
				// mesh_texture
			);
		}
		// Draw triangle wireframe
		if(render_method == RENDER_WIRE || render_method == RENDER_WIRE_VERTEX
		   || render_method == RENDER_FILL_TRIANGLE_WIRE || render_method == RENDER_TEXTURED_WIRE)
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
	render_scene_to_buffer();
	render_color_buffer();
	// render_stats_text();
	render_imgui();
	SDL_RenderPresent(renderer);
}

void free_resource()
{
	free_display_resource();
	free_meshes_resource();
}

int main(int argc, char * argv[])
{
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

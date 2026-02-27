#include "Array.h"
#include "Display.h"
#include "Logger.h"
#include "Mesh.h"
#include "Vector.h"
#include <stdlib.h>
#include <string>


triangle_t* triangles_to_render = nullptr;

vec3_t camera_position = {0, 0, 0};


bool is_running = false;

float fov_factor = 640;

int previous_frame_time = 0;

void setup() {
    color_buffer = new uint32_t[window_width * window_height];
    color_buffer_texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

    // load_cube_mesh_data();
    load_obj_file_data("../assets/cube.obj");
}

void process_input() {
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type) {
    case SDL_EVENT_QUIT:
        is_running = false;
        break;
    case SDL_EVENT_KEY_DOWN:
        if (event.key.key == SDLK_ESCAPE) {
            is_running = false;
        }
        break;
    }
}

// Function that receives a 3D vector and returns a projected 2D point
vec2_t project(vec3_t point) {
    vec2_t projected_point = {.x = fov_factor * point.x / point.z, .y = fov_factor * point.y / point.z};
    return projected_point;
}

void update() {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);
    // time_to_wait <= FRAME_TARGET_TIME is mostly useless,
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }

    previous_frame_time = SDL_GetTicks();

    // Initialize the array to triangles to render
    triangles_to_render = nullptr;

    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.00;
    mesh.rotation.z += 0.00;

    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        vec3_t transformed_vertices[3];
        // Loop all three vertices of this current face and apply transformation
        for (int j = 0; j < 3; j++) {
            transformed_vertices[j] = face_vertices[j];
            transformed_vertices[j] = vec3_rotate_x(transformed_vertices[j], mesh.rotation.x);
            transformed_vertices[j] = vec3_rotate_y(transformed_vertices[j], mesh.rotation.y);
            transformed_vertices[j] = vec3_rotate_z(transformed_vertices[j], mesh.rotation.z);

            transformed_vertices[j].z += 5;
        }

        // Check backface culling
        vec3_t vector_a = transformed_vertices[0];
        vec3_t vector_b = transformed_vertices[1];
        vec3_t vector_c = transformed_vertices[2];

        auto vector_ab = vector_b - vector_a;
        auto vector_ac = vector_c - vector_a;
        vec3_t normal  = vec3_cross(vector_ab, vector_ac);

        // Normalize the face normal vector - in-place update
        vec3_normalize(&normal);
        // Find the vector between a point in the triangle and the camera origin
        auto camera_ray = camera_position - vector_a;

        // Calculate how aligned the camera ray is with the face normal (Using dot product)
        auto culling = vec3_dot(normal, camera_ray);
        if (culling < 0) {
            continue;
        }

        // Loop all three vertices to perform a projection
        triangle_t projected_triangle;
        for (int j = 0; j < 3; j++) {
            // Project the current point
            vec2_t projected_point = project(transformed_vertices[j]);
            // Scale and translate the projected points to the middle of the screen
            projected_point.x += window_width / 2;
            projected_point.y += window_height / 2;

            projected_triangle.points[j] = projected_point;
        }
        // Save the projected triangle in the array of triangles to render
        array_push(triangles_to_render, projected_triangle);
    }
}
void render() {
    clear_color_buffer(0xFF000000);
    draw_grid();
    // Loop all projected triangles and render them
    int num_faces = array_length(triangles_to_render);
    for (int i = 0; i < num_faces; i++) {
        const triangle_t& triangle = triangles_to_render[i];

        // Draw filled triangle
        draw_filled_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y,
            triangle.points[2].x, triangle.points[2].y, 0xFFFFFFFF);

        draw_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y,
            triangle.points[2].x, triangle.points[2].y, 0xFF000000);
    }

    // draw_triangle(300, 100, 50, 400, 500, 700, 0xFF00FF00);
    // draw_filled_triangle(300, 100, 50, 400, 500, 700, 0xFF00FF00);
    // Clear the array of triangles to render every frame loop
    array_free(triangles_to_render);
    render_color_buffer();
    SDL_RenderPresent(renderer);
}

void free_resource() {
    free(color_buffer);
    array_free(mesh.faces);
    array_free(mesh.vertices);
}

int main(int argc, char* argv[]) {
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_DEBUG);
    SDL_SetLogPriority(MY_LOG_OBJ, SDL_LOG_PRIORITY_DEBUG);
    is_running = initialize_window();
    setup();

    while (is_running) {
        process_input();
        update();
        render();
    }
    destroy_window();
    free_resource();
    return 0;
}

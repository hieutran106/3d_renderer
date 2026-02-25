#include "Array.h"
#include "Display.h"
#include "Logger.h"
#include "Mesh.h"
#include "Vector.h"
#include <stdlib.h>


triangle_t* triangles_to_render = nullptr;

vec3_t camera_position = {0, 0, -5};


bool is_running = false;

float fov_factor = 640;

int previous_frame_time = 0;

void setup() {
    color_buffer = new uint32_t[window_width * window_height];
    color_buffer_texture =
        SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

    // load_cube_mesh_data();
    load_obj_file_data("../assets/f22.obj");
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

        triangle_t projected_triangle;
        // Loop all three vertices of this current face and apply transformation
        for (int j = 0; j < 3; j++) {
            vec3_t transformed_vertex = face_vertices[j];
            transformed_vertex        = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex        = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex        = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            transformed_vertex.z -= camera_position.z;
            // Project the current point
            vec2_t projected_point = project(transformed_vertex);
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
    // draw_grid();

    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++) {
        const triangle_t& triangle = triangles_to_render[i];
        draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
        draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
        draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);

        // Draw unfilled triangle
        draw_triangle(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y,
            triangle.points[2].x, triangle.points[2].y, 0xFF00FF00);
    }

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
    // Test
    int x         = 5;
    auto z        = 10.5;
    decltype(x) y = 10.5;
    // End test
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

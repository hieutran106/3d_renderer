#pragma once
#include "Triangle.h"
#include "Vector.h"

constexpr int N_CUBE_VERTICES = 8;
extern vec3_t cube_vertices[N_CUBE_VERTICES];

constexpr int N_CUBE_FACES = 6 * 2;
extern face_t cube_faces[N_CUBE_FACES];

// Define a struct for dynamic size meshes
struct mesh_t
{
	vec3_t * vertices; // Dynamic array of vertices
	face_t * faces;
	vec3_t rotation;
	vec3_t scale; // Scale with x,y, and z
	vec3_t translation;
};

extern mesh_t mesh;

void load_cube_mesh_data();
void load_obj_file_data(const char * filename);

#pragma once
#include "Triangle.h"
#include "Vector.h"
#include <vector>

// Define a struct for dynamic size meshes
struct mesh_t
{
	std::vector<vec3_t> vertices; // Dynamic array of vertices
	std::vector<face_t> faces;

	vec3_t rotation;
	vec3_t scale; // Scale with x,y, and z
	vec3_t translation;
};

extern mesh_t mesh;

void load_cube_mesh_data();
void load_obj_file_data(const char * filename);

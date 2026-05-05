#pragma once
#include "Triangle.h"
#include "Vector.h"
#include "upng.h"
#include <vector>

// Define a struct for dynamic size meshes
struct mesh_t
{
	std::vector<vec3_t> vertices; // Dynamic array of vertices
	std::vector<face_t> faces;
	upng_t * texture;
	vec3_t rotation;
	vec3_t scale; // Scale with x,y, and z
	vec3_t translation;
};

void load_obj_file_data(const char * filename);
void load_mesh(const char * obj_filename, const char * png_filename, vec3_t scale, vec3_t translation, vec3_t rotation);
int get_num_meshes();
mesh_t & get_mesh(int index);
void free_meshes_resource();
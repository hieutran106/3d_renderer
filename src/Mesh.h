#pragma once

#include "Texture.h"
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
	// stb
	png_texture_t png_texture;
};

void load_mesh(const char * obj_filename, const char * png_filename, vec3_t scale, vec3_t translation, vec3_t rotation);

void free_meshes_resource();
std::vector<mesh_t> & get_meshes();
///////////////////////////////////
void load_runway_scene();
void load_cat_mesh();
void load_runway_mesh();
void load_f22_mesh();
void load_efa_mesh();
void load_f117_mesh();

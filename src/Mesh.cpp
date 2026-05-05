#include "Mesh.h"

#include "Logger.h"
#include <SDL3/SDL.h>

constexpr int MAX_NUM_MESHES = 10;
static std::array<mesh_t, MAX_NUM_MESHES> meshes;
static int mesh_count = 0;

void load_mesh_obj_data(mesh_t & mesh, const char * filename)
{
	FILE * file;
	file = fopen(filename, "r");
	char line[1024];

	std::vector<tex2_t> texcoords;

	while(fgets(line, 1024, file))
	{
		// Vertex information
		if(strncmp(line, "v ", 2) == 0)
		{
			vec3_t vertex;
			sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
			mesh.vertices.push_back(vertex);
		}
		// Texture coordinate information
		if(strncmp(line, "vt ", 3) == 0)
		{
			tex2_t texcoord;
			sscanf(line, "vt %f %f", &texcoord.u, &texcoord.v);
			texcoords.push_back(texcoord);
		}
		// Face information
		if(strncmp(line, "f ", 2) == 0)
		{
			int vertex_indices[3];
			int texture_indices[3];
			int normal_indices[3];
			sscanf(
				line,
				"f %d/%d/%d %d/%d/%d %d/%d/%d",
				&vertex_indices[0],
				&texture_indices[0],
				&normal_indices[0],
				&vertex_indices[1],
				&texture_indices[1],
				&normal_indices[1],
				&vertex_indices[2],
				&texture_indices[2],
				&normal_indices[2]
			);
			face_t face = {
				.a = vertex_indices[0],
				.b = vertex_indices[1],
				.c = vertex_indices[2],
				.a_uv = texcoords[texture_indices[0] - 1],
				.b_uv = texcoords[texture_indices[1] - 1],
				.c_uv = texcoords[texture_indices[2] - 1],
				.color = 0xFFFFFFFF
			};
			mesh.faces.push_back(face);
		}
	}
}

void load_mesh_png_data(mesh_t & mesh, const char * filename)
{
	upng_t * png_image = upng_new_from_file(filename);
	if(png_image != nullptr)
	{
		upng_decode(png_image);
		if(upng_get_error(png_image) == UPNG_EOK)
		{
			mesh.texture = png_image;
		}
	}
}

void load_mesh(const char * obj_filename, const char * png_filename, vec3_t scale, vec3_t translation, vec3_t rotation)
{
	mesh_t & mesh = meshes[mesh_count];
	load_mesh_obj_data(mesh, obj_filename);
	load_mesh_png_data(mesh, png_filename);

	mesh.scale = scale;
	mesh.translation = translation;
	mesh.rotation = rotation;
	mesh_count++;
}

int get_num_meshes()
{
	return mesh_count;
}
mesh_t & get_mesh(int index)
{
	return meshes[index];
}

void free_meshes_resource()
{
	for(int i = 0; i < mesh_count; i++)
	{
		const auto & m = meshes[i];
		upng_free(m.texture);
	}
}
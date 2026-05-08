#include "Mesh.h"
#include "SDLHelper.h"
#include <SDL3/SDL.h>
#include <numbers>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

static std::vector<mesh_t> meshes;

void load_mesh_obj_data(mesh_t & mesh, const char * filename)
{
	auto abs_filename = SDLHelper::GetResourcePath(filename);
	FILE * file = fopen(abs_filename.c_str(), "r");
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
			int v[4]; // vertex indices
			int vt[4]; // texture indices
			int vn[4]; // normal indices
			int count = sscanf(
				line,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&v[0],
				&vt[0],
				&vn[0],
				&v[1],
				&vt[1],
				&vn[1],
				&v[2],
				&vt[2],
				&vn[2],
				&v[3],
				&vt[3],
				&vn[3]
			);
			face_t face1 = {
				.a = v[0],
				.b = v[1],
				.c = v[2],
				.a_uv = texcoords[vt[0] - 1],
				.b_uv = texcoords[vt[1] - 1],
				.c_uv = texcoords[vt[2] - 1],
				.color = 0xFFFFFFFF
			};
			mesh.faces.push_back(face1);
			if(count == 12)
			{
				// Read a quad
				face_t face2 = {
					.a = v[0],
					.b = v[2],
					.c = v[3],
					.a_uv = texcoords[vt[0] - 1],
					.b_uv = texcoords[vt[2] - 1],
					.c_uv = texcoords[vt[3] - 1],
					.color = 0xFFFFFFFF
				};
				mesh.faces.push_back(face2);
			}
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
	// new stbi
	int width, height, channels;
	// Force 4 channels (RGBA) to ensure 32-bit alignment
	unsigned char * data = stbi_load(filename, &width, &height, &channels, 4);
	if(data)
	{
		png_texture_t png_texture = {
			.width = width, .height = height, .channels = channels, .texture = reinterpret_cast<uint32_t *>(data)
		};
		mesh.png_texture = png_texture;
	}
}

void load_mesh(const char * obj_filename, const char * png_filename, vec3_t scale, vec3_t translation, vec3_t rotation)
{
	mesh_t mesh;
	load_mesh_obj_data(mesh, obj_filename);
	load_mesh_png_data(mesh, png_filename);

	mesh.scale = scale;
	mesh.translation = translation;
	mesh.rotation = rotation;

	meshes.push_back(mesh);
}

void free_meshes_resource()
{
	for(const auto & m : meshes)
	{
		upng_free(m.texture);
		stbi_image_free(m.png_texture.texture);
	}
}

std::vector<mesh_t> & get_meshes()
{
	return meshes;
}
void load_runway_scene()
{
	load_runway_mesh();
	load_f22_mesh();
	load_efa_mesh();
	load_f117_mesh();
}
void load_cat_mesh()
{
	load_mesh(
		"./assets/cat.obj",
		"./assets/f22.png",
		vec3_new(1, 1, 1),
		vec3_new(0, 0, +5),
		vec3_new(0, std::numbers::pi / 2, 0)
	);
}

void load_runway_mesh()
{
	load_mesh(
		"./assets/runway.obj", "./assets/runway.png", vec3_new(1, 1, 1), vec3_new(0, -1.5, +23), vec3_new(0, 0, 0)
	);
}
void load_f22_mesh()
{
	load_mesh(
		"./assets/f22.obj",
		"./assets/f22.png",
		vec3_new(1, 1, 1),
		vec3_new(0, -1.3, +5),
		vec3_new(0, -std::numbers::pi / 2, 0)
	);
}
void load_efa_mesh()
{
	load_mesh(
		"./assets/efa.obj",
		"./assets/efa.png",
		vec3_new(1, 1, 1),
		vec3_new(-2, -1.3, +9),
		vec3_new(0, -std::numbers::pi / 2, 0)
	);
}
void load_f117_mesh()
{
	load_mesh(
		"./assets/f117.obj",
		"./assets/f117.png",
		vec3_new(1, 1, 1),
		vec3_new(+2, -1.3, +9),
		vec3_new(0, -std::numbers::pi / 2, 0)
	);
}
#pragma once
#include "Triangle.h"
#include "Vector.h"

constexpr int N_MESH_VERTICES = 8;
extern vec3_t mesh_vertices[N_MESH_VERTICES];

constexpr int N_MESH_FACES = 6 * 2;
extern face_t mesh_faces[N_MESH_FACES];
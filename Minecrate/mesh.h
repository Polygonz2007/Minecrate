#ifndef MESH_H
#define MESH_H

#include "vec2.h"
#include "vec3.h"
#include "block.h"
#include <raylib.h>

// Generate base plane meshes
// Struct used to store data for a single mesh plane
struct mesh_base_plane {
    Vector3 pos1;
    Vector3 pos2;
    Vector3 pos3;
    Vector3 pos4;

    Vector3 normal;

    Vector2 uv1;
    Vector2 uv2;
    Vector2 uv3;
    Vector2 uv4;
};

// Struct for mesh generating buffer, this is if theres a face at 0, if theres one at 1 it will be on next over
struct mesh_sides {
    block_t x;          // type of block on this face
    _Bool x_normals;    // false: negative, true: positive

    block_t y;
    _Bool y_normals;

    block_t z;
    _Bool z_normals;
};



int load_chunk_mesh(vec2i16_t chunk_pos);
int unload_chunk_mesh(vec2i16_t chunk_pos);

struct mesh_base_plane gen_plane_blueprint(vec3i16_t offset, vec3i8_t dir);
Mesh GenChunkMesh(vec2i16_t chunk_pos);

#endif
#ifndef MESHES_H
#define MESHES_H

#ifndef RAYLIB_H
#include <raylib.h>
#endif


struct mesh_base_plane gen_plane_blueprint(vec3i16_t offset, vec3i8_t dir);
Mesh GenChunkMesh(vec2i16_t chunk_pos);

#endif
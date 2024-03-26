
#include <stdio.h>
#include <malloc.h>

#include <math.h>
#include <raylib.h>
#include <raymath.h>

#include "mesh.h"
#include "block.h"
#include "chunk.h"
#include "texture.h"

uint32_t mesh_gen_buffer_size;
struct mesh_sides* mesh_gen_buffer;	// Used for storing sides to load in a mesh
block_t mesh_gen_blocks_buffer;

// for mesh gen
float *verts_p;
float *uvs_p;
float *norms_p;


// graaah
// Vector3 utility (move later)
static inline Vector3 add_vector3(Vector3 a, Vector3 b) {
    return (Vector3) { a.x + b.x, a.y + b.y, a.z + b.z };
}

static inline struct mesh_sides mesh_sides_empty() {
    return (struct mesh_sides) {
        BLOCK_UNDEFINED, false, BLOCK_UNDEFINED, false, BLOCK_UNDEFINED, false
    };
}




// INIT MESH GEN
int init_mesh_gen() {
    mesh_gen_buffer_size = chunk_size.x * chunk_size.y * chunk_size.z;
    //mesh_gen_buffer_size -= chunk_size.y; // remove last bit we dont need

    mesh_gen_buffer = malloc(mesh_gen_buffer_size * sizeof(struct mesh_sides));
    chunk_mem_usage += mesh_gen_buffer_size * sizeof(struct mesh_sides);

    printf("\nInitiated mesh gen buffer.");
    return 0;
}


// INIT MESH GEN
int free_mesh_gen() {
    free(mesh_gen_buffer);

    printf("\nFree'd mesh gen buffer.");
    return 0;
}

// This can be called on thread
int load_chunk_mesh(vec2i16_t chunk_pos) {
    uint16_t index = get_chunk_index(chunk_pos);

    if (index == -1) // Make sure the chunk were loading the mesh for exists
        return -1;

    if (chunk_status[index] == CHUNK_LOADED_MESH || chunk_status[index] == CHUNK_LOADED_MODEL)
        return -1; // We already have mesh dont load a new one idot

    // Load mesh
    chunk_meshes[index] = GenChunkMesh(chunk_pos);
    chunk_status[index] = CHUNK_LOADED_MESH;

    return 0;
}

// This needs to be called on main thread, to upload mesh
int load_chunk_model(vec2i16_t chunk_pos) {
    uint16_t index = get_chunk_index(chunk_pos);

    if (index == -1)
        return -1;

    if (chunk_status[index] == CHUNK_LOADED_MESH) {
        // Upload mesh and load model
        UploadMesh(&chunk_meshes[index], false);
        chunk_models[index] = LoadModelFromMesh(chunk_meshes[index]);

        // Texture
        chunk_models[index].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture_atlas;

        chunk_status[index] = CHUNK_LOADED_MODEL;
        return 0;
    }

    return -1;
}

// Call anywhere you go!
int unload_chunk_model_and_mesh(vec2i16_t chunk_pos) {
    uint16_t index = get_chunk_index(chunk_pos);

    if (index == -1) // Make sure the mesh and model we're trying to unload exists
        return -1;

    // Empty model
    //UnloadModel(chunk_models[index]);
    chunk_models[index] = (Model) { 0 };

    // TODO: fix lol
    // Free the data from mesh and empty mesh
    chunk_meshes[index] = (Mesh) { 0 };

    // Set status
    chunk_status[index] = CHUNK_LOADED;

    return 0;
}

// Generate chunk
Mesh GenChunkMesh(vec2i16_t chunk_pos) {
    for (uint32_t i = 0; i < mesh_gen_buffer_size; ++i) {
        mesh_gen_buffer[i] = mesh_sides_empty();
    }

    // Calculate how big mesh will be (Loop through chunk and count and store)
    uint32_t tot_tris = 0;
    int32_t chunk_index = get_chunk_index(chunk_pos);

    if (chunk_index == -1)
        return (Mesh) { 0 };    // Cannot load an empty chunk, so return empty mesh >:D

    // Chunk index offsets
    uint32_t cind = chunk_index * chunk_data_size;
    int16_t chunk_neg_x = get_chunk_index((vec2i16_t) { chunk_pos.x - 1, chunk_pos.y });
    int16_t chunk_neg_z = get_chunk_index((vec2i16_t) { chunk_pos.x, chunk_pos.y - 1 });

    uint32_t cnxind = chunk_neg_x * chunk_data_size;
    uint32_t cnzind = chunk_neg_z * chunk_data_size;

    // Use to get block above, below, sides, etc of this block
    const uint16_t plus_x = 1;
    const uint16_t plus_y = chunk_size.x;
    const uint16_t plus_z = chunk_size.x * chunk_size.y;

    // Fill buffer for blocks needed for THIS chunk.
    vec3u16_t sides_size = { chunk_size.x, chunk_size.y, chunk_size.z };

    // Calculate all blocks where a face is needed. Add to the total.
    for (uint16_t x = 0; x < chunk_size.x; ++x) {
        for (uint16_t y = 0; y < chunk_size.y; ++y) {
            for (uint16_t z = 0; z < chunk_size.z; ++z) {

                // Get index, and start moving data
                uint32_t block_index = x + (y * chunk_size.x) + (z * chunk_size.x * chunk_size.y) + cind;
                uint32_t write_index = x + (y * sides_size.x) + (z * sides_size.x * sides_size.y);

                // Check blocks
                block_t cb = chunk_data[block_index]; //chunk_data[cind + index];

                // Get block here - 1x, if we are at 0 check next chunk.
                block_t nx = block_t_new(BLOCK_UNDEFINED);
                if (x != 0) {
                    nx = chunk_data[block_index - plus_x];
                } else if (chunk_neg_x != -1) {
                    nx = chunk_data[cnxind + 15 + (y * chunk_size.x) + (z * chunk_size.y * chunk_size.x)];
                }

                // Get block here - 1y, if at 0 return undefined.
                block_t ny = block_t_new(BLOCK_UNDEFINED);
                if (y != 0) {
                    ny = chunk_data[block_index - plus_y];
                }

                // Get block here - 1z, if we are at 0 check next chunk.
                block_t nz = block_t_new(BLOCK_UNDEFINED);
                if (z != 0) {
                    nz = chunk_data[block_index - plus_z];
                } else if (chunk_neg_z != -1) {
                    nz = chunk_data[cnzind + x + (y * chunk_size.x) + (15 * chunk_size.y * chunk_size.x)];
                }



                // Edit values for blocks ON THIS BLOCK (no offset)
                if (block_is_clear[cb.type] != block_is_clear[nx.type] && nx.type != BLOCK_UNDEFINED) {
                    tot_tris += 2;
                    mesh_gen_buffer[write_index].x = block_is_clear[nx.type] ? cb : nx;
                    mesh_gen_buffer[write_index].x_normals = block_is_clear[nx.type];
                }

                if (block_is_clear[cb.type] != block_is_clear[ny.type] && ny.type != BLOCK_UNDEFINED) {
                    tot_tris += 2;
                    mesh_gen_buffer[write_index].y = block_is_clear[ny.type] ? cb : ny;
                    mesh_gen_buffer[write_index].y_normals = block_is_clear[ny.type];
                }

                if (block_is_clear[cb.type] != block_is_clear[nz.type] && nz.type != BLOCK_UNDEFINED) {
                    tot_tris += 2;
                    mesh_gen_buffer[write_index].z = block_is_clear[nz.type] ? cb : nz;
                    mesh_gen_buffer[write_index].z_normals = block_is_clear[nz.type];
                }
            }
        }
    }

    printf("past\n");

    // Init mesh object
    Mesh mesh = { 0 };

    // Return empty if no faces
    if (tot_tris == 0)
        return mesh;

    // Use total triangle number to calculate mesh size!
    mesh.triangleCount = (int)tot_tris;
    mesh.vertexCount = mesh.triangleCount * 3;
    mesh.vertices = (float *)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float *)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float *)MemAlloc(mesh.vertexCount * 3 * sizeof(float));

    // Giddy up
    verts_p = mesh.vertices;
    uvs_p = mesh.texcoords;
    norms_p = mesh.normals;
    uint32_t face_ind = 0;

    // Loop trough all blocks, and add faces where it is nessecarry
    for (uint16_t x = 0; x < chunk_size.x; ++x) {
        for (uint16_t y = 0; y < chunk_size.y; ++y) {
            for (uint16_t z = 0; z < chunk_size.z; ++z) {

                // Get index and make mesh_sides for this block
                const uint32_t index = x + (y * sides_size.x) + (z * sides_size.x * sides_size.y);
                const struct mesh_sides sides = mesh_gen_buffer[index];

                // X FACE
                if (sides.x.type != BLOCK_UNDEFINED) {

                    // We want a face on this side, so generate blueprint..
                    const struct mesh_base_plane bp = gen_plane_blueprint(
                        (vec3i16_t) { x, y, z }, // offset
                        side_t_new(sides.x_normals ? SIDE_LEFT : SIDE_RIGHT), // direction (normal)
                        sides.x.type // block type (for uvs)
                    );

                    // .. and add the verts!
                    add_vert(bp.pos1, bp.uv1, bp.normal, &face_ind);
                    add_vert(bp.pos3, bp.uv3, bp.normal, &face_ind);
                    add_vert(bp.pos2, bp.uv2, bp.normal, &face_ind);
                    add_vert(bp.pos2, bp.uv2, bp.normal, &face_ind);
                    add_vert(bp.pos3, bp.uv3, bp.normal, &face_ind);
                    add_vert(bp.pos4, bp.uv4, bp.normal, &face_ind);
                }

                // Y FACE
                if (sides.y.type != BLOCK_UNDEFINED) {
                    // We want a face on this side, so generate blueprint..
                    const struct mesh_base_plane bp = gen_plane_blueprint(
                        (vec3i16_t) { x, y, z },                        // OFFSET
                        side_t_new(sides.y_normals ? SIDE_BOTTOM : SIDE_TOP),   // DIRECTION (NORMAL)
                        sides.y.type
                    );

                    // .. and add the verts!
                    add_vert(bp.pos1, bp.uv1, bp.normal, &face_ind);
                    add_vert(bp.pos3, bp.uv3, bp.normal, &face_ind);
                    add_vert(bp.pos2, bp.uv2, bp.normal, &face_ind);
                    add_vert(bp.pos2, bp.uv2, bp.normal, &face_ind);
                    add_vert(bp.pos3, bp.uv3, bp.normal, &face_ind);
                    add_vert(bp.pos4, bp.uv4, bp.normal, &face_ind);
                }

                // Z FACE
                if (sides.z.type != BLOCK_UNDEFINED) {
                    // We want a face on this side, so generate blueprint..
                    const struct mesh_base_plane bp = gen_plane_blueprint(
                        (vec3i16_t) { x, y, z },                        // OFFSET
                        side_t_new(sides.z_normals ? SIDE_BACK : SIDE_FRONT),   // DIRECTION (NORMAL)
                        sides.z.type
                    );

                    // .. and add the verts!
                    add_vert(bp.pos1, bp.uv1, bp.normal, &face_ind);
                    add_vert(bp.pos3, bp.uv3, bp.normal, &face_ind);
                    add_vert(bp.pos2, bp.uv2, bp.normal, &face_ind);
                    add_vert(bp.pos2, bp.uv2, bp.normal, &face_ind);
                    add_vert(bp.pos3, bp.uv3, bp.normal, &face_ind);
                    add_vert(bp.pos4, bp.uv4, bp.normal, &face_ind);
                }

            }
        }
    }

    printf("dipped\n");

    // We upload mesh on main thread, so do nothing here :)
    return mesh;
}



static int add_vert(Vector3 pos, Vector2 uv, Vector3 normal, uint32_t* index) {
    // Calculate indexes
    uint32_t ind3 = *index * 3;
    uint32_t ind2 = *index * 2;

    // Set data
    verts_p[ind3 + 0] = pos.x;
    verts_p[ind3 + 1] = pos.y;
    verts_p[ind3 + 2] = pos.z;

    uvs_p[ind2 + 0] = uv.x;
    uvs_p[ind2 + 1] = uv.y;

    norms_p[ind3 + 0] = normal.x;
    norms_p[ind3 + 1] = normal.y;
    norms_p[ind3 + 2] = normal.z;

    // Increment and return
    ++(*index);
    return 0;
}





// Dir should be either positive 1 or negative 1 in only one of the axis.
struct mesh_base_plane gen_plane_blueprint(vec3i16_t offset, side_t dir, uint8_t block) {
    // Set up variables to generate plane
    vec3i8_t normal8 = side_normals[dir.i];
    Vector3 normal = { normal8.x, normal8.y, normal8.z };

    Vector3 basePos = { (float)offset.x, (float)offset.y, (float)offset.z };
    Vector3 tan1 = { 0, 0, 0 };
    Vector3 tan2 = { 0, 0, 0 };

    // Find tangents (should always be positive)
    if (dir.i == SIDE_LEFT || dir.i == SIDE_RIGHT) {
        tan1 = (Vector3) { 0, 1, 0 };
        tan2 = (Vector3) { 0, 0, 1 };
    }
    else if (dir.i == SIDE_TOP || dir.i == SIDE_BOTTOM) {
        tan1 = (Vector3){ 1, 0, 0 };
        tan2 = (Vector3){ 0, 0, 1 };
    }
    else if (dir.i == SIDE_BACK || dir.i == SIDE_FRONT) {
        tan1 = (Vector3){ 1, 0, 0 };
        tan2 = (Vector3){ 0, 1, 0 };
    }

    if (dir.i == SIDE_RIGHT || dir.i == SIDE_BOTTOM || dir.i == SIDE_FRONT) {
        const Vector3 temp = tan1;
        tan1 = tan2;
        tan2 = temp; // Flip order of tangents, so face is on other side

        normal.x = -normal.x;
        normal.y = -normal.y;
        normal.z = -normal.z; // Flip normal
    }

    // Generate verticies
    struct mesh_base_plane plane = { 0 };

    plane.pos1 = basePos;
    plane.pos2 = add_vector3(basePos, tan1);
    plane.pos3 = add_vector3(basePos, tan2);
    plane.pos4 = add_vector3(basePos, add_vector3(tan1, tan2));


    // UVS
    // Since the plane starts at 0, which is below 1, and the uvs start at 0 ABOVE 1, we flip that axis
    Vector2 start = get_texcoords_atlas(block_t_new(block), side_t_new(dir.i)); // -1 bc of none
    Vector2 end = Vector2Add(start, get_texcoord_block_size());

    plane.uv1 = (Vector2) { start.x,   end.y };
    plane.uv2 = (Vector2) {   end.x,   end.y };
    plane.uv3 = (Vector2) { start.x, start.y };
    plane.uv4 = (Vector2) {   end.x, start.y };

    // Swap around uvs if flipped
    if (dir.i == SIDE_LEFT || dir.i == SIDE_BOTTOM || dir.i == SIDE_FRONT) {
        // Since we flipped order of tangents, we need to flip the two only-one-axis uvs
        Vector2 temp = plane.uv2;
        plane.uv2 = plane.uv3;
        plane.uv3 = temp;
    }

    // Normal.
    plane.normal = normal;

    // Return it
    return plane;
}
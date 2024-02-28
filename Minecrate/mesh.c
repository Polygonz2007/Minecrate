
#include <stdio.h>
#include <malloc.h>

#include <math.h>
#include <raylib.h>
#include <raymath.h>

#include "mesh.h"
#include "block.h"
#include "chunk.h"

static struct mesh_sides* mesh_gen_buffer;	// Used for storing sides to load in a mesh



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
    mesh_gen_buffer = malloc(chunk_data_size * sizeof(struct mesh_sides));
    chunk_mem_usage += chunk_data_size * sizeof(struct mesh_sides);

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
        // Load texture (replace with texture atlas in future
        Image img = LoadImage("res/block_textures/BLOCK_SAND.png");

        Texture2D texture = LoadTextureFromImage(img);
        UnloadImage(img);

        // Upload mesh and load model
        UploadMesh(&chunk_meshes[index], false);
        chunk_models[index] = LoadModelFromMesh(chunk_meshes[index]);

        // Texture
        chunk_models[index].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

        chunk_status[index] = CHUNK_LOADED_MODEL;
    }
}

// Call anywhere
int unload_chunk_model_and_mesh(vec2i16_t chunk_pos) {
    uint16_t index = get_chunk_index(chunk_pos);

    if (index == -1) // Make sure the mesh and model we're trying to unload exists
        return -1;

    // Empty model
    chunk_models[index] = (Model) { 0 };

    // Free the data from mesh and empty mesh
    //MemFree(chunk_meshes[index].vertices);
    //MemFree(chunk_meshes[index].texcoords);
    //MemFree(chunk_meshes[index].normals);
    chunk_meshes[index] = (Mesh) { 0 };

    // Set status
    chunk_status[index] = CHUNK_LOADED;

    return 0;
}

// Generate chunk
Mesh GenChunkMesh(vec2i16_t chunk_pos) {

    // Fill buffer with empty data
    for (uint32_t i = 0; i < chunk_data_size; ++i) {
        mesh_gen_buffer[i] = mesh_sides_empty();
    }

    // Calculate how big mesh will be (Loop through chunk and count and store)
    uint32_t tot_tris = 0;
    int32_t chunk_index = get_chunk_index(chunk_pos);

    if (chunk_index == -1)
        return (Mesh) { 0 };    // Cannot load an empty chunk, so return empty mesh >:D

    uint32_t cind = chunk_index * chunk_data_size;

    int32_t chunk_neg_x_index = get_chunk_index((vec2i16_t) { chunk_pos.x - 1, chunk_pos.y });
    int32_t chunk_neg_z_index = get_chunk_index((vec2i16_t) { chunk_pos.x, chunk_pos.y - 1 });
    uint32_t cx_ind = chunk_neg_x_index * chunk_data_size;
    uint32_t cz_ind = chunk_neg_z_index * chunk_data_size;

    // Use to get block above, below, sides, etc of this block
    uint16_t plus_x = 1;
    uint16_t plus_y = chunk_size.x;
    uint16_t plus_z = chunk_size.x * chunk_size.y;

    for (uint16_t x = 0; x < chunk_size.x; ++x) {
        for (uint16_t y = 0; y < chunk_size.y; ++y) {
            for (uint16_t z = 0; z < chunk_size.z; ++z) {
                uint32_t index = x + (y * chunk_size.x) + (z * chunk_size.y * chunk_size.x);
                block_t cb = chunk_data[cind + index];

                if (cb.type == BLOCK_AIR || cb.type == BLOCK_WATER) {
                    // We are in a transparent block, so check all sides and add to buffer if we need plane
                    // Make sure its not outside of chunk
                    block_t px = block_t_new(BLOCK_UNDEFINED);
                    if (x + 1 < chunk_size.x)
                        px = chunk_data[cind + index + plus_x];

                    block_t py = block_t_new(BLOCK_UNDEFINED);
                    if (y + 1 < chunk_size.y)
                        py = chunk_data[cind + index + plus_y];

                    block_t pz = block_t_new(BLOCK_UNDEFINED);
                    if (z + 1 < chunk_size.z)
                        pz = chunk_data[cind + index + plus_z];

                    // Negative, so make sure more than 0
                    block_t nx = block_t_new(BLOCK_UNDEFINED);
                    if (x > 0)
                        nx = chunk_data[cind + index - plus_x];
                    if (x == 0 && chunk_neg_x_index != -1) {
                        uint32_t nx_ind = 15 + (y * chunk_size.x) + (z * chunk_size.y * chunk_size.x);
                        nx = chunk_data[cx_ind + nx_ind];
                    }

                    block_t ny = block_t_new(BLOCK_UNDEFINED);
                    if (y > 0)
                        ny = chunk_data[cind + index - plus_y];

                    block_t nz = block_t_new(BLOCK_UNDEFINED);
                    if (z > 0)
                        nz = chunk_data[cind + index - plus_z];
                    if (z == 0 && chunk_neg_z_index != -1) {
                        uint32_t nz_ind = x + (y * chunk_size.x) + (15 * chunk_size.y * chunk_size.x);
                        nz = chunk_data[cz_ind + nz_ind];
                    }




                    // Edit values for blocks, this block bc negative bc 0
                    if (nx.type != BLOCK_UNDEFINED && nx.type != BLOCK_AIR && nx.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index].x = nx;
                        mesh_gen_buffer[index].x_normals = true;
                    }

                    if (ny.type != BLOCK_UNDEFINED && ny.type != BLOCK_AIR && ny.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index].y = ny;
                        mesh_gen_buffer[index].y_normals = false;
                    }

                    if (nz.type != BLOCK_UNDEFINED && nz.type != BLOCK_AIR && nz.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index].z = nz;
                        mesh_gen_buffer[index].z_normals = true;
                    }

                    // Edit values for blocks with OFFSET!!! (wow, so cool!) bc 1, with offset! (and positive)
                    if (px.type != BLOCK_UNDEFINED && px.type != BLOCK_AIR && px.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index + plus_x].x = px;
                        mesh_gen_buffer[index + plus_x].x_normals = false;
                    }

                    if (py.type != BLOCK_UNDEFINED && py.type != BLOCK_AIR && py.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index + plus_y].y = py;
                        mesh_gen_buffer[index + plus_y].y_normals = true;
                    }

                    if (pz.type != BLOCK_UNDEFINED && pz.type != BLOCK_AIR && pz.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index + plus_z].z = pz;
                        mesh_gen_buffer[index + plus_z].z_normals = false;
                    }

                    // gaah, now go to next and after fill in the data...
                }
            }
        }
    }

    // Init mesh object
    Mesh mesh = { 0 };
    mesh.triangleCount = tot_tris;
    mesh.vertexCount = mesh.triangleCount * 3;
    mesh.vertices = (float *)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float *)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float *)MemAlloc(mesh.vertexCount * 3 * sizeof(float));

    // Fill in the data
    uint32_t face_ind = 0;

    for (uint16_t x = 0; x < chunk_size.x; ++x) {
        for (uint16_t y = 0; y < chunk_size.y; ++y) {
            for (uint16_t z = 0; z < chunk_size.z; ++z) {
                const uint32_t index = x + (y * chunk_size.x) + (z * chunk_size.y * chunk_size.x);

                // todo: generate the things and put into the verticices and stuff
                const struct mesh_sides sides = mesh_gen_buffer[index];

                // X FACE
                if (sides.x.type != BLOCK_UNDEFINED) {
                    // We want a face on this side, so generate blueprint..
                    const struct mesh_base_plane px = gen_plane_blueprint(
                        (vec3i16_t) {
                        x, y, z
                    },                        // OFFSET
                        (vec3i8_t) {
                        sides.x_normals ? -1 : 1, 0, 0
                    }   // DIRECTION (NORMAL)
                    );

                    // ..and add the verticies. (we add 6 verticies, and each uses 3 dimensions...)
                    // and also 6 uvs, but 2 dimensions!
                    const ind3 = face_ind * 18; // current index
                    const ind2 = face_ind * 12; // current index for texcoords (4, because

                    // VERT 0 0
                    mesh.vertices[ind3 + 0] = px.pos1.x;
                    mesh.vertices[ind3 + 1] = px.pos1.y;
                    mesh.vertices[ind3 + 2] = px.pos1.z;

                    mesh.texcoords[ind2 + 0] = 0.0f;
                    mesh.texcoords[ind2 + 1] = 0.0f;

                    mesh.normals[ind3 + 0] = px.normal.x;
                    mesh.normals[ind3 + 1] = px.normal.y;
                    mesh.normals[ind3 + 2] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 3] = px.pos3.x;
                    mesh.vertices[ind3 + 4] = px.pos3.y;
                    mesh.vertices[ind3 + 5] = px.pos3.z;

                    mesh.texcoords[ind2 + 2] = 0.5f;
                    mesh.texcoords[ind2 + 3] = 0.0f;

                    mesh.normals[ind3 + 3] = px.normal.x;
                    mesh.normals[ind3 + 4] = px.normal.y;
                    mesh.normals[ind3 + 5] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 6] = px.pos2.x;
                    mesh.vertices[ind3 + 7] = px.pos2.y;
                    mesh.vertices[ind3 + 8] = px.pos2.z;

                    mesh.texcoords[ind2 + 4] = 0.0f;
                    mesh.texcoords[ind2 + 5] = 1.0f;

                    mesh.normals[ind3 + 6] = px.normal.x;
                    mesh.normals[ind3 + 7] = px.normal.y;
                    mesh.normals[ind3 + 8] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 9] = px.pos2.x;
                    mesh.vertices[ind3 + 10] = px.pos2.y;
                    mesh.vertices[ind3 + 11] = px.pos2.z;

                    mesh.texcoords[ind2 + 6] = 0.0f;
                    mesh.texcoords[ind2 + 7] = 1.0f;

                    mesh.normals[ind3 + 9] = px.normal.x;
                    mesh.normals[ind3 + 10] = px.normal.y;
                    mesh.normals[ind3 + 11] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 12] = px.pos3.x;
                    mesh.vertices[ind3 + 13] = px.pos3.y;
                    mesh.vertices[ind3 + 14] = px.pos3.z;

                    mesh.texcoords[ind2 + 8] = 0.5f;
                    mesh.texcoords[ind2 + 9] = 0.0f;

                    mesh.normals[ind3 + 12] = px.normal.x;
                    mesh.normals[ind3 + 13] = px.normal.y;
                    mesh.normals[ind3 + 14] = px.normal.z;

                    // VERT 1 1
                    mesh.vertices[ind3 + 15] = px.pos4.x;
                    mesh.vertices[ind3 + 16] = px.pos4.y;
                    mesh.vertices[ind3 + 17] = px.pos4.z;

                    mesh.texcoords[ind2 + 10] = 0.5f;
                    mesh.texcoords[ind2 + 11] = 1.0f;

                    mesh.normals[ind3 + 15] = px.normal.x;
                    mesh.normals[ind3 + 16] = px.normal.y;
                    mesh.normals[ind3 + 17] = px.normal.z;

                    // Finally, increment face_ind so we dont overwrite this face!
                    face_ind++;
                }

                // Y FACE
                if (sides.y.type != BLOCK_UNDEFINED) {
                    // We want a face on this side, so generate blueprint..
                    const struct mesh_base_plane px = gen_plane_blueprint(
                        (vec3i16_t) {
                        x, y, z
                    },                        // OFFSET
                        (vec3i8_t) {
                        0, sides.y_normals ? -1 : 1, 0
                    }   // DIRECTION (NORMAL)
                    );

                    // ..and add the verticies. (we add 6 verticies, and each uses 3 dimensions...)
                    // and also 6 uvs, but 2 dimensions!
                    const ind3 = face_ind * 18; // current index
                    const ind2 = face_ind * 12; // current index for texcoords (4, because

                    // VERT 0 0
                    mesh.vertices[ind3 + 0] = px.pos1.x;
                    mesh.vertices[ind3 + 1] = px.pos1.y;
                    mesh.vertices[ind3 + 2] = px.pos1.z;

                    mesh.texcoords[ind2 + 0] = 0.5f;
                    mesh.texcoords[ind2 + 1] = 0.0f;

                    mesh.normals[ind3 + 0] = px.normal.x;
                    mesh.normals[ind3 + 1] = px.normal.y;
                    mesh.normals[ind3 + 2] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 3] = px.pos3.x;
                    mesh.vertices[ind3 + 4] = px.pos3.y;
                    mesh.vertices[ind3 + 5] = px.pos3.z;

                    mesh.texcoords[ind2 + 2] = 1.0f;
                    mesh.texcoords[ind2 + 3] = 0.0f;

                    mesh.normals[ind3 + 3] = px.normal.x;
                    mesh.normals[ind3 + 4] = px.normal.y;
                    mesh.normals[ind3 + 5] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 6] = px.pos2.x;
                    mesh.vertices[ind3 + 7] = px.pos2.y;
                    mesh.vertices[ind3 + 8] = px.pos2.z;

                    mesh.texcoords[ind2 + 4] = 0.5f;
                    mesh.texcoords[ind2 + 5] = 1.0f;

                    mesh.normals[ind3 + 6] = px.normal.x;
                    mesh.normals[ind3 + 7] = px.normal.y;
                    mesh.normals[ind3 + 8] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 9] = px.pos2.x;
                    mesh.vertices[ind3 + 10] = px.pos2.y;
                    mesh.vertices[ind3 + 11] = px.pos2.z;

                    mesh.texcoords[ind2 + 6] = 0.5f;
                    mesh.texcoords[ind2 + 7] = 1.0f;

                    mesh.normals[ind3 + 9] = px.normal.x;
                    mesh.normals[ind3 + 10] = px.normal.y;
                    mesh.normals[ind3 + 11] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 12] = px.pos3.x;
                    mesh.vertices[ind3 + 13] = px.pos3.y;
                    mesh.vertices[ind3 + 14] = px.pos3.z;

                    mesh.texcoords[ind2 + 8] = 1.0f;
                    mesh.texcoords[ind2 + 9] = 0.0f;

                    mesh.normals[ind3 + 12] = px.normal.x;
                    mesh.normals[ind3 + 13] = px.normal.y;
                    mesh.normals[ind3 + 14] = px.normal.z;

                    // VERT 1 1
                    mesh.vertices[ind3 + 15] = px.pos4.x;
                    mesh.vertices[ind3 + 16] = px.pos4.y;
                    mesh.vertices[ind3 + 17] = px.pos4.z;

                    mesh.texcoords[ind2 + 10] = 1.0f;
                    mesh.texcoords[ind2 + 11] = 1.0f;

                    mesh.normals[ind3 + 15] = px.normal.x;
                    mesh.normals[ind3 + 16] = px.normal.y;
                    mesh.normals[ind3 + 17] = px.normal.z;

                    // Finally, increment face_ind so we dont overwrite this face!
                    face_ind++;
                }

                // Z FACE
                if (sides.z.type != BLOCK_UNDEFINED) {
                    // We want a face on this side, so generate blueprint..
                    const struct mesh_base_plane px = gen_plane_blueprint(
                        (vec3i16_t) {
                        x, y, z
                    },                        // OFFSET
                        (vec3i8_t) {
                        0, 0, sides.z_normals ? -1 : 1
                    }   // DIRECTION (NORMAL)
                    );

                    // ..and add the verticies. (we add 6 verticies, and each uses 3 dimensions...)
                    // and also 6 uvs, but 2 dimensions!
                    const ind3 = face_ind * 18; // current index
                    const ind2 = face_ind * 12; // current index for texcoords (4, because

                    // VERT 0 0
                    mesh.vertices[ind3 + 0] = px.pos1.x;
                    mesh.vertices[ind3 + 1] = px.pos1.y;
                    mesh.vertices[ind3 + 2] = px.pos1.z;

                    mesh.texcoords[ind2 + 0] = 0.0f;
                    mesh.texcoords[ind2 + 1] = 0.0f;

                    mesh.normals[ind3 + 0] = px.normal.x;
                    mesh.normals[ind3 + 1] = px.normal.y;
                    mesh.normals[ind3 + 2] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 3] = px.pos3.x;
                    mesh.vertices[ind3 + 4] = px.pos3.y;
                    mesh.vertices[ind3 + 5] = px.pos3.z;

                    mesh.texcoords[ind2 + 2] = 1.0f;
                    mesh.texcoords[ind2 + 3] = 0.0f;

                    mesh.normals[ind3 + 3] = px.normal.x;
                    mesh.normals[ind3 + 4] = px.normal.y;
                    mesh.normals[ind3 + 5] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 6] = px.pos2.x;
                    mesh.vertices[ind3 + 7] = px.pos2.y;
                    mesh.vertices[ind3 + 8] = px.pos2.z;

                    mesh.texcoords[ind2 + 4] = 0.0f;
                    mesh.texcoords[ind2 + 5] = 1.0f;

                    mesh.normals[ind3 + 6] = px.normal.x;
                    mesh.normals[ind3 + 7] = px.normal.y;
                    mesh.normals[ind3 + 8] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 9] = px.pos2.x;
                    mesh.vertices[ind3 + 10] = px.pos2.y;
                    mesh.vertices[ind3 + 11] = px.pos2.z;

                    mesh.texcoords[ind2 + 6] = 0.0f;
                    mesh.texcoords[ind2 + 7] = 1.0f;

                    mesh.normals[ind3 + 9] = px.normal.x;
                    mesh.normals[ind3 + 10] = px.normal.y;
                    mesh.normals[ind3 + 11] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 12] = px.pos3.x;
                    mesh.vertices[ind3 + 13] = px.pos3.y;
                    mesh.vertices[ind3 + 14] = px.pos3.z;

                    mesh.texcoords[ind2 + 8] = 1.0f;
                    mesh.texcoords[ind2 + 9] = 0.0f;

                    mesh.normals[ind3 + 12] = px.normal.x;
                    mesh.normals[ind3 + 13] = px.normal.y;
                    mesh.normals[ind3 + 14] = px.normal.z;

                    // VERT 1 1
                    mesh.vertices[ind3 + 15] = px.pos4.x;
                    mesh.vertices[ind3 + 16] = px.pos4.y;
                    mesh.vertices[ind3 + 17] = px.pos4.z;

                    mesh.texcoords[ind2 + 10] = 1.0f;
                    mesh.texcoords[ind2 + 11] = 1.0f;

                    mesh.normals[ind3 + 15] = px.normal.x;
                    mesh.normals[ind3 + 16] = px.normal.y;
                    mesh.normals[ind3 + 17] = px.normal.z;

                    // Finally, increment face_ind so we dont overwrite this face!
                    face_ind++;
                }

            }
        }
    }

    return mesh;
}






// Dir should be either positive 1 or negative 1 in only one of the axis.
struct mesh_base_plane gen_plane_blueprint(vec3i16_t offset, vec3i8_t dir) {
    // Set up variables to generate plane
    Vector3 normal = { (float)dir.x, (float)dir.y, (float)dir.z };

    Vector3 basePos = { (float)offset.x, (float)offset.y, (float)offset.z };
    Vector3 tan1 = { 0, 0, 0 };
    Vector3 tan2 = { 0, 0, 0 };

    // Find tangents (should always be positive)
    if (dir.x != 0) {
        tan1 = (Vector3) { 0, 1, 0 };
        tan2 = (Vector3) { 0, 0, 1 };
    }
    else if (dir.y != 0) {
        tan1 = (Vector3){ 1, 0, 0 };
        tan2 = (Vector3){ 0, 0, 1 };
    }
    else if (dir.z != 0) {
        tan1 = (Vector3){ 1, 0, 0 };
        tan2 = (Vector3){ 0, 1, 0 };
    }

    if (dir.x + dir.y + dir.z < 0) {
        Vector3 temp = tan1;
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

    plane.uv1 = (Vector2){ 0, 0 };
    plane.uv2 = (Vector2){ 1, 0 };
    plane.uv3 = (Vector2){ 0, 1 };
    plane.uv4 = (Vector2){ 1, 1 };

    plane.normal = normal;

    // Return it
    return plane;
}
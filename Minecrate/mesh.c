
#include <stdio.h>
#include <malloc.h>

#include <math.h>
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

    printf("\nInitiated mesh gen buffer.");
    return 0;
}


// INIT MESH GEN
int free_mesh_gen() {
    free(mesh_gen_buffer);

    printf("\nFree'd mesh gen buffer.");
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
    uint32_t chunk_index = get_chunk_index(chunk_pos);

    // Use to get block above, below, sides, etc of this block
    uint16_t plus_x = 1;
    uint16_t plus_y = chunk_size.x;
    uint16_t plus_z = chunk_size.x * chunk_size.y;

    for (uint16_t x = 0; x < chunk_size.x; ++x) {
        for (uint16_t y = 0; y < chunk_size.y; ++y) {
            for (uint16_t z = 0; z < chunk_size.z; ++z) {
                uint32_t index = x + (y * chunk_size.x) + (z * chunk_size.y * chunk_size.x);

                uint32_t cind = chunk_index * chunk_data_size;
                block_t cb = chunk_data[cind + index];

                if (cb.type == BLOCK_AIR || cb.type == BLOCK_WATER) {
                    // We are in a transparent block, so check all sides and add to buffer if we need plane
                    block_t px = chunk_data[cind + index + plus_x];
                    block_t py = chunk_data[cind + index + plus_y];
                    block_t pz = chunk_data[cind + index + plus_z];
                    block_t nx = chunk_data[cind + index - plus_x];
                    block_t ny = chunk_data[cind + index - plus_y];
                    block_t nz = chunk_data[cind + index - plus_z];

                    // Edit values for blocks, this block bc negative bc 0
                    if (nx.type != BLOCK_AIR && nx.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index].x = nx;
                        mesh_gen_buffer[index].x_normals = true;
                    }

                    if (ny.type != BLOCK_AIR && ny.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index].y = ny;
                        mesh_gen_buffer[index].y_normals = true;
                    }

                    if (nz.type != BLOCK_AIR && nz.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index].z = nz;
                        mesh_gen_buffer[index].z_normals = true;
                    }

                    // Edit values for blocks with OFFSET!!! (wow, so cool!) bc 1, with offset! (and positive)
                    if (px.type != BLOCK_AIR && px.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index + plus_x].x = px;
                        mesh_gen_buffer[index + plus_x].x_normals = false;
                    }

                    if (py.type != BLOCK_AIR && py.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index + plus_y].y = py;
                        mesh_gen_buffer[index].y_normals = false;
                    }

                    if (pz.type != BLOCK_AIR && pz.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index + plus_z].z = pz;
                        mesh_gen_buffer[index].z_normals = false;
                    }

                    // gaah, now go to next and after fill in the data...
                }
            }
        }
    }

    // Init mesh object
    Mesh mesh = { 0 };
    mesh.triangleCount = chunk_size.x * chunk_size.z;
    mesh.vertexCount = mesh.triangleCount * 3;
    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));

    printf("Initiated chunk mesh, with %d triangles and %d verticies.", mesh.triangleCount, mesh.vertexCount);

    // Fill in the data
    uint32_t face_ind = 0;

    for (uint16_t x = 0; x < chunk_size.x; ++x) {
        for (uint16_t y = 0; y < chunk_size.y; ++y) {
            for (uint16_t z = 0; z < chunk_size.z; ++z) {
                uint32_t index = x + (y * chunk_size.x) + (z * chunk_size.y * chunk_size.x);

                // todo: generate the things and put into the verticices and stuff
                struct mesh_sides sides = mesh_gen_buffer[index];

                // X FACE
                if (sides.x.type != BLOCK_UNDEFINED) {
                    // We want a face on this side, so generate blueprint..
                    struct mesh_base_plane px = gen_plane_blueprint(
                        (vec3i16_t) { x, y, z },                        // OFFSET
                        (vec3i8_t) { sides.x_normals ? -1 : 1, 0, 0 }   // DIRECTION (NORMAL)
                    ); 

                    // ..and add the verticies. (we add 6 verticies, and each uses 3 dimensions...)
                    // and also 6 uvs, but 2 dimensions!
                    const ind3 = face_ind * 18; // current index
                    const ind2 = face_ind * 12; // current index for texcoords (4, because

                    // VERT 0 0
                    mesh.vertices[ind3 + 0] = px.pos1.x;
                    mesh.vertices[ind3 + 1] = px.pos1.y;
                    mesh.vertices[ind3 + 2] = px.pos1.z;

                    mesh.texcoords[ind2 + 0] = 0;
                    mesh.texcoords[ind2 + 1] = 0;

                    mesh.normals[ind3 + 0] = px.normal.x;
                    mesh.normals[ind3 + 1] = px.normal.y;
                    mesh.normals[ind3 + 2] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 3] = px.pos1.x;
                    mesh.vertices[ind3 + 4] = px.pos1.y;
                    mesh.vertices[ind3 + 5] = px.pos1.z;

                    mesh.texcoords[ind2 + 2] = 0;
                    mesh.texcoords[ind2 + 3] = 0;

                    mesh.normals[ind3 + 3] = px.normal.x;
                    mesh.normals[ind3 + 4] = px.normal.y;
                    mesh.normals[ind3 + 5] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 6] = px.pos1.x;
                    mesh.vertices[ind3 + 7] = px.pos1.y;
                    mesh.vertices[ind3 + 8] = px.pos1.z;

                    mesh.texcoords[ind2 + 4] = 0;
                    mesh.texcoords[ind2 + 5] = 0;

                    mesh.normals[ind3 + 6] = px.normal.x;
                    mesh.normals[ind3 + 7] = px.normal.y;
                    mesh.normals[ind3 + 8] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 9] = px.pos1.x;
                    mesh.vertices[ind3 + 10] = px.pos1.y;
                    mesh.vertices[ind3 + 11] = px.pos1.z;

                    mesh.texcoords[ind2 + 6] = 0;
                    mesh.texcoords[ind2 + 7] = 0;

                    mesh.normals[ind3 + 9] = px.normal.x;
                    mesh.normals[ind3 + 10] = px.normal.y;
                    mesh.normals[ind3 + 11] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 12] = px.pos1.x;
                    mesh.vertices[ind3 + 13] = px.pos1.y;
                    mesh.vertices[ind3 + 14] = px.pos1.z;

                    mesh.texcoords[ind2 + 8] = 0;
                    mesh.texcoords[ind2 + 9] = 0;

                    mesh.normals[ind3 + 12] = px.normal.x;
                    mesh.normals[ind3 + 13] = px.normal.y;
                    mesh.normals[ind3 + 14] = px.normal.z;

                    // VERT 1 1
                    mesh.vertices[ind3 + 15] = px.pos1.x;
                    mesh.vertices[ind3 + 16] = px.pos1.y;
                    mesh.vertices[ind3 + 17] = px.pos1.z;

                    mesh.texcoords[ind2 + 10] = 0;
                    mesh.texcoords[ind2 + 11] = 0;

                    mesh.normals[ind3 + 15] = px.normal.x;
                    mesh.normals[ind3 + 16] = px.normal.y;
                    mesh.normals[ind3 + 17] = px.normal.z;

                    // Finally, increment face_ind so we dont overwrite this face!
                    face_ind++;
                }
                // Y FACE
                if (sides.y.type != BLOCK_UNDEFINED) {
                    // We want a face on this side, so generate blueprint..
                    struct mesh_base_plane px = gen_plane_blueprint(
                        (vec3i16_t) { x, y, z },                        // OFFSET
                        (vec3i8_t) { 0, sides.x_normals ? -1 : 1, 0 }   // DIRECTION (NORMAL)
                    );

                    // ..and add the verticies. (we add 6 verticies, and each uses 3 dimensions...)
                    // and also 6 uvs, but 2 dimensions!
                    const ind3 = face_ind * 18; // current index
                    const ind2 = face_ind * 12; // current index for texcoords (4, because

                    // VERT 0 0
                    mesh.vertices[ind3 + 0] = px.pos1.x;
                    mesh.vertices[ind3 + 1] = px.pos1.y;
                    mesh.vertices[ind3 + 2] = px.pos1.z;

                    mesh.texcoords[ind2 + 0] = 0;
                    mesh.texcoords[ind2 + 1] = 0;

                    mesh.normals[ind3 + 0] = px.normal.x;
                    mesh.normals[ind3 + 1] = px.normal.y;
                    mesh.normals[ind3 + 2] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 3] = px.pos1.x;
                    mesh.vertices[ind3 + 4] = px.pos1.y;
                    mesh.vertices[ind3 + 5] = px.pos1.z;

                    mesh.texcoords[ind2 + 2] = 0;
                    mesh.texcoords[ind2 + 3] = 0;

                    mesh.normals[ind3 + 3] = px.normal.x;
                    mesh.normals[ind3 + 4] = px.normal.y;
                    mesh.normals[ind3 + 5] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 6] = px.pos1.x;
                    mesh.vertices[ind3 + 7] = px.pos1.y;
                    mesh.vertices[ind3 + 8] = px.pos1.z;

                    mesh.texcoords[ind2 + 4] = 0;
                    mesh.texcoords[ind2 + 5] = 0;

                    mesh.normals[ind3 + 6] = px.normal.x;
                    mesh.normals[ind3 + 7] = px.normal.y;
                    mesh.normals[ind3 + 8] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 9] = px.pos1.x;
                    mesh.vertices[ind3 + 10] = px.pos1.y;
                    mesh.vertices[ind3 + 11] = px.pos1.z;

                    mesh.texcoords[ind2 + 6] = 0;
                    mesh.texcoords[ind2 + 7] = 0;

                    mesh.normals[ind3 + 9] = px.normal.x;
                    mesh.normals[ind3 + 10] = px.normal.y;
                    mesh.normals[ind3 + 11] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 12] = px.pos1.x;
                    mesh.vertices[ind3 + 13] = px.pos1.y;
                    mesh.vertices[ind3 + 14] = px.pos1.z;

                    mesh.texcoords[ind2 + 8] = 0;
                    mesh.texcoords[ind2 + 9] = 0;

                    mesh.normals[ind3 + 12] = px.normal.x;
                    mesh.normals[ind3 + 13] = px.normal.y;
                    mesh.normals[ind3 + 14] = px.normal.z;

                    // VERT 1 1
                    mesh.vertices[ind3 + 15] = px.pos1.x;
                    mesh.vertices[ind3 + 16] = px.pos1.y;
                    mesh.vertices[ind3 + 17] = px.pos1.z;

                    mesh.texcoords[ind2 + 10] = 0;
                    mesh.texcoords[ind2 + 11] = 0;

                    mesh.normals[ind3 + 15] = px.normal.x;
                    mesh.normals[ind3 + 16] = px.normal.y;
                    mesh.normals[ind3 + 17] = px.normal.z;

                    // Finally, increment face_ind so we dont overwrite this face!
                    face_ind++;
                }

                // Z FACE
                if (sides.y.type != BLOCK_UNDEFINED) {
                    // We want a face on this side, so generate blueprint..
                    struct mesh_base_plane px = gen_plane_blueprint(
                        (vec3i16_t) { x, y, z },                        // OFFSET
                        (vec3i8_t) { 0, 0, sides.z_normals ? -1 : 1 }   // DIRECTION (NORMAL)
                    );

                    // ..and add the verticies. (we add 6 verticies, and each uses 3 dimensions...)
                    // and also 6 uvs, but 2 dimensions!
                    const ind3 = face_ind * 18; // current index
                    const ind2 = face_ind * 12; // current index for texcoords (4, because

                    // VERT 0 0
                    mesh.vertices[ind3 + 0] = px.pos1.x;
                    mesh.vertices[ind3 + 1] = px.pos1.y;
                    mesh.vertices[ind3 + 2] = px.pos1.z;

                    mesh.texcoords[ind2 + 0] = 0;
                    mesh.texcoords[ind2 + 1] = 0;

                    mesh.normals[ind3 + 0] = px.normal.x;
                    mesh.normals[ind3 + 1] = px.normal.y;
                    mesh.normals[ind3 + 2] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 3] = px.pos1.x;
                    mesh.vertices[ind3 + 4] = px.pos1.y;
                    mesh.vertices[ind3 + 5] = px.pos1.z;

                    mesh.texcoords[ind2 + 2] = 0;
                    mesh.texcoords[ind2 + 3] = 0;

                    mesh.normals[ind3 + 3] = px.normal.x;
                    mesh.normals[ind3 + 4] = px.normal.y;
                    mesh.normals[ind3 + 5] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 6] = px.pos1.x;
                    mesh.vertices[ind3 + 7] = px.pos1.y;
                    mesh.vertices[ind3 + 8] = px.pos1.z;

                    mesh.texcoords[ind2 + 4] = 0;
                    mesh.texcoords[ind2 + 5] = 0;

                    mesh.normals[ind3 + 6] = px.normal.x;
                    mesh.normals[ind3 + 7] = px.normal.y;
                    mesh.normals[ind3 + 8] = px.normal.z;

                    // VERT 1 0
                    mesh.vertices[ind3 + 9] = px.pos1.x;
                    mesh.vertices[ind3 + 10] = px.pos1.y;
                    mesh.vertices[ind3 + 11] = px.pos1.z;

                    mesh.texcoords[ind2 + 6] = 0;
                    mesh.texcoords[ind2 + 7] = 0;

                    mesh.normals[ind3 + 9] = px.normal.x;
                    mesh.normals[ind3 + 10] = px.normal.y;
                    mesh.normals[ind3 + 11] = px.normal.z;

                    // VERT 0 1
                    mesh.vertices[ind3 + 12] = px.pos1.x;
                    mesh.vertices[ind3 + 13] = px.pos1.y;
                    mesh.vertices[ind3 + 14] = px.pos1.z;

                    mesh.texcoords[ind2 + 8] = 0;
                    mesh.texcoords[ind2 + 9] = 0;

                    mesh.normals[ind3 + 12] = px.normal.x;
                    mesh.normals[ind3 + 13] = px.normal.y;
                    mesh.normals[ind3 + 14] = px.normal.z;

                    // VERT 1 1
                    mesh.vertices[ind3 + 15] = px.pos1.x;
                    mesh.vertices[ind3 + 16] = px.pos1.y;
                    mesh.vertices[ind3 + 17] = px.pos1.z;

                    mesh.texcoords[ind2 + 10] = 0;
                    mesh.texcoords[ind2 + 11] = 0;

                    mesh.normals[ind3 + 15] = px.normal.x;
                    mesh.normals[ind3 + 16] = px.normal.y;
                    mesh.normals[ind3 + 17] = px.normal.z;

                    // Finally, increment face_ind so we dont overwrite this face!
                    face_ind++;
                }

            }
        }
    }

    printf("It somehow worked, and we are now loading the mesh.");

    UploadMesh(&mesh, false);

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
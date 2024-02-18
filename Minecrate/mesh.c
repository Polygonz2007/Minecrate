
#include <math.h>
#include <raymath.h>

#include "mesh.h"
#include "block.h"
#include "chunk.h"

static struct mesh_sides* mesh_gen_buffer;	// Used for storing sides to load in a mesh

// INIT MESH GEN
int init_mesh_gen() {
    mesh_gen_buffer = malloc(chunk_data_size * sizeof(struct mesh_sides));

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
                    }

                    if (ny.type != BLOCK_AIR && ny.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index].y = ny;
                    }

                    if (nz.type != BLOCK_AIR && nz.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index].z = nz;
                    }

                    // Edit values for blocks with OFFSET!!! (wow, so cool!) bc 1, with offset! (and positive)
                    if (px.type != BLOCK_AIR && px.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index + plus_x].x = px;
                    }

                    if (py.type != BLOCK_AIR && py.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index + plus_y].x = py;
                    }

                    if (pz.type != BLOCK_AIR && pz.type != BLOCK_WATER) {
                        tot_tris += 2;
                        mesh_gen_buffer[index + plus_z].x = pz;
                    }

                    // gaah, now go to next and after fill inn the data...
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

    // Fill in the data
    for (uint16_t x = 0; x < chunk_size.x; ++x) {
        for (uint16_t y = 0; y < chunk_size.y; ++y) {
            for (uint16_t z = 0; z < chunk_size.z; ++z) {
                uint32_t index = x + (y * chunk_size.x) + (z * chunk_size.y * chunk_size.x);

                // todo: generate the things and put into the verticices and stuff
                struct mesh_sides sides = mesh_gen_buffer[index];

                if (sides.x.type != BLOCK_UNDEFINED) {
                    struct mesh_base_plane px = gen_plane_blueprint((vec3i16_t) { x, y, z }, (vec3i8_t) { 1, 0, 0 });
                }
            }
        }
    }

    UploadMesh(&mesh, false);

    return mesh;

}









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
} mesh_base_plane;


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


// Vector3 utility (move later)
static inline Vector3 add_vector3(Vector3 a, Vector3 b) {
    return (Vector3) { a.x + b.x, a.y + b.y, a.z + b.z };
}


// Struct for mesh generating buffer, this is if theres a face at 0, if theres one at 1 it will be on next over
struct mesh_sides {
    block_t x;
    block_t y;
    block_t z;
};

static inline struct mesh_sides mesh_sides_empty() {
    return (struct mesh_sides) {
        BLOCK_UNDEFINED, BLOCK_UNDEFINED, BLOCK_UNDEFINED
    };
}
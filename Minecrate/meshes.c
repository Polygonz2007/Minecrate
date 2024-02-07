#include "meshes.h"

// test
Mesh generate_mesh_plane(Vector3 offset) {
    Mesh mesh = { 0 };
    mesh.triangleCount = 2;
    mesh.vertexCount = mesh.triangleCount * 3;
    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));

    int ox = offset.x;
    int oy = offset.y;
    int oz = offset.z;

    // Vertex 0 0
    mesh.vertices[0] = 0 + ox;
    mesh.vertices[1] = 0 + oy;
    mesh.vertices[2] = 0 + oz;
    mesh.normals[0] = 0;
    mesh.normals[1] = 1;
    mesh.normals[2] = 0;
    mesh.texcoords[0] = 0;
    mesh.texcoords[1] = 0;

    // Vertex 1 0
    mesh.vertices[3] = 0 + ox;
    mesh.vertices[4] = 1 + oy;
    mesh.vertices[5] = 1 + oz;
    mesh.normals[3] = 0;
    mesh.normals[4] = 1;
    mesh.normals[5] = 0;
    mesh.texcoords[2] = 0;
    mesh.texcoords[3] = 1;

    // Vertex 0 1
    mesh.vertices[6] = 1 + ox;
    mesh.vertices[7] = 0 + oy;
    mesh.vertices[8] = 0 + oz;
    mesh.normals[6] = 0;
    mesh.normals[7] = 1;
    mesh.normals[8] = 0;
    mesh.texcoords[4] = 1;
    mesh.texcoords[5] = 0;


    // Vertex 1 1
    mesh.vertices[9] = 1 + ox;
    mesh.vertices[10] = 0 + oy;
    mesh.vertices[11] = 1 + oz;
    mesh.normals[9] = 0;
    mesh.normals[10] = 1;
    mesh.normals[11] = 0;
    mesh.texcoords[6] = 1;
    mesh.texcoords[7] = 1;

    // Vertex 1 0
    mesh.vertices[12] = 1 + ox;
    mesh.vertices[13] = 0 + oy;
    mesh.vertices[14] = 0 + oz;
    mesh.normals[12] = 0;
    mesh.normals[13] = 1;
    mesh.normals[14] = 0;
    mesh.texcoords[8] = 1;
    mesh.texcoords[9] = 0;

    // Vertex 0 1
    mesh.vertices[15] = 0 + ox;
    mesh.vertices[16] = 1 + oy;
    mesh.vertices[17] = 1 + oz;
    mesh.normals[15] = 0;
    mesh.normals[16] = 1;
    mesh.normals[17] = 0;
    mesh.texcoords[10] = 0;
    mesh.texcoords[11] = 1;

    return mesh;
}

Mesh GenPlate() {

    Mesh mesh = generate_mesh_plane((Vector3){0,1,0});

    UploadMesh(&mesh, false);

	return mesh;

}


// Generate chunk

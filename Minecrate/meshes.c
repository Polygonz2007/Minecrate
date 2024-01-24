#include "meshes.h"

Mesh GenPlate() {

	Mesh mesh = { 0 };
    mesh.triangleCount = 2;
    mesh.vertexCount = mesh.triangleCount * 3;
    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));


    // Vertex 0 0
    mesh.vertices[0] = 0;
    mesh.vertices[1] = 0;
    mesh.vertices[2] = 0;
    mesh.normals[0] = 0;
    mesh.normals[1] = 1;
    mesh.normals[2] = 0;
    mesh.texcoords[0] = 0;
    mesh.texcoords[1] = 0;

    // Vertex 1 0
    mesh.vertices[3] = 1;
    mesh.vertices[4] = 0;
    mesh.vertices[5] = 0;
    mesh.normals[3] = 0;
    mesh.normals[4] = 1;
    mesh.normals[5] = 0;
    mesh.texcoords[2] = 1;
    mesh.texcoords[3] = 0;

    // Vertex 0 1
    mesh.vertices[6] = 0;
    mesh.vertices[7] = 0;
    mesh.vertices[8] = 1;
    mesh.normals[6] = 0;
    mesh.normals[7] = 1;
    mesh.normals[8] = 0;
    mesh.texcoords[4] = 0;
    mesh.texcoords[5] = 1;

    // Vertex 1 0
    mesh.vertices[9] = 1;
    mesh.vertices[10] = 0;
    mesh.vertices[11] = 0;
    mesh.normals[9] = 0;
    mesh.normals[10] = 1;
    mesh.normals[11] = 0;
    mesh.texcoords[6] = 1;
    mesh.texcoords[7] = 0;

    // Vertex 0 1
    mesh.vertices[12] = 0;
    mesh.vertices[13] = 0;
    mesh.vertices[14] = 1;
    mesh.normals[12] = 0;
    mesh.normals[13] = 1;
    mesh.normals[14] = 0;
    mesh.texcoords[8] = 0;
    mesh.texcoords[9] = 1;

    // Vertex 1 1
    mesh.vertices[15] = 1;
    mesh.vertices[16] = 0;
    mesh.vertices[17] = 1;
    mesh.normals[15] = 0;
    mesh.normals[16] = 1;
    mesh.normals[17] = 0;
    mesh.texcoords[10] = 1;
    mesh.texcoords[11] = 1;


    UploadMesh(&mesh, false);

	return mesh;

}
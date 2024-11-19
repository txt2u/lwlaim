#ifndef MESH_H
#define MESH_H

#include <pipeline/buffers.h>

#include <glad/glad.h>
#include <cglm/cglm.h>

// Structure to store the mesh data
typedef struct {
    GLuint textureID;      // Texture ID (optional)
    int vertexCount;    // Number of vertices
    int indexCount;     // Number of indices
	int program_id; // Shader Program ID

	Buffers buffers;
    mat4 modelMatrix;      // Model transformation matrix (for translation, rotation, scale)
} Mesh;

// Function declarations
Mesh* mesh_create(
	float* vertices, int vertexCount, 
	unsigned int* indices, int indexCount, 
	float* texcoords, int texcoordsCount, 
	float* normals, int normalsCount, 
	const char* textureFile, int program_id
);
void mesh_render(Mesh* mesh, mat4 viewMatrix, mat4 projectionMatrix);
void mesh_destroy(Mesh* mesh);

// Transformation functions
void mesh_translate(Mesh* mesh, vec3 translation);
void mesh_rotate(Mesh* mesh, float angle, vec3 axis);
void mesh_scale(Mesh* mesh, vec3 scale);

#endif // MESH_H
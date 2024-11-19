#include <entites/mesh.h>
#include <pipeline/buffers.h>

#include <glad/glad.h>
#include <stdlib.h>
#include <stb_image.h>  // for texture loading, you need stb_image library
#include <cglm/cglm.h>  // for cglm transformations

// Create a new mesh
#include <entites/mesh.h>
#include <pipeline/buffers.h>
#include <stdio.h>
#include <stdlib.h>
#include <stb_image.h>

// Create a new mesh
Mesh* mesh_create(float* vertices, int vertexCount, 
				  unsigned int* indices, int indexCount, 
				  float* texcoords, int texcoordsCount, 
				  float* normals, int normalsCount, 
				  const char* textureFile, int program_id
) {
    Mesh* mesh = (Mesh*)malloc(sizeof(Mesh));
    mesh->program_id = program_id;

    if (!mesh) {
        return NULL; // Allocation failed
    }

	mesh->buffers = buffers_create_empty();

	// Create buffers and upload model data to GPU
    buffers_bind_vao(mesh->buffers.VAO);

    // Create VBO for vertex positions
    mesh->buffers.VBO = buffers_create_vbo(vertices, vertexCount * 3);
    buffers_bind_vbo(mesh->buffers.VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Create VBO for texture coordinates
    if (texcoordsCount > 0) {
        mesh->buffers.TexCoordVBO = buffers_create_vbo(texcoords, texcoordsCount * 2);
        buffers_bind_vbo(mesh->buffers.TexCoordVBO);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
    }
    
    // Create VBO for normals
    if (normalsCount > 0) {
        mesh->buffers.NormalVBO = buffers_create_vbo(normals, normalsCount * 3);
        buffers_bind_vbo(mesh->buffers.NormalVBO);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);
    }

    // Create EBO for indices
    mesh->buffers.EBO = buffers_create_ebo(indices, indexCount);
    buffers_bind_ebo(mesh->buffers.EBO);

    // Load texture (if applicable)
    if (textureFile) {
        buffers_bind_vbo(mesh->buffers.TexCoordVBO);
        int width, height, channels;
        unsigned char* data = stbi_load(textureFile, &width, &height, &channels, 0);
        if (data) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glGenTextures(1, &mesh->textureID);
            glBindTexture(GL_TEXTURE_2D, mesh->textureID);

			GLenum format, internalFormat;

			// Check the number of channels and set the corresponding format
			if (channels == 1) {
				internalFormat = GL_RED;
				format = GL_RED;
			} else if (channels == 3) {
				internalFormat = GL_RGB;
				format = GL_RGB;
			} else if (channels == 4) {
				internalFormat = GL_RGBA;
				format = GL_RGBA;
			} else {
				printf("Unsupported texture format with %d channels.\n", channels);
				stbi_image_free(data);
				return NULL;
			}

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);
        } else {
            printf("Failed to load texture: %s\n", textureFile);
        }
    }

    // Set mesh properties
    mesh->vertexCount = vertexCount;
    mesh->indexCount = indexCount;

    // Initialize the model matrix as the identity matrix
	glm_mat4_identity(mesh->modelMatrix);

    return mesh;
}


// Render the mesh
void mesh_render(Mesh* mesh, mat4 viewMatrix, mat4 projectionMatrix) {
    if (mesh->program_id == 0) {
        printf("INVALID SHADER PROGRAM ID!\n");
        return; // Exit if shader program is invalid
    }

    glUseProgram(mesh->program_id);

    // Bind VAO
    buffers_bind_vao(mesh->buffers.VAO);

    // Bind texture coordinates buffer only if there are valid texcoords
    if (mesh->buffers.TexCoordVBO) {
        buffers_bind_vbo(mesh->buffers.TexCoordVBO);
    }

    // Bind normals buffer if available
    if (mesh->buffers.NormalVBO) {
        buffers_bind_vbo(mesh->buffers.NormalVBO);
    }

    // Bind texture if it exists
    if (mesh->textureID != 0) {
		// Bind texture to texture unit 0
		GLuint textureLocation = glGetUniformLocation(mesh->program_id, "texture1");
		glUniform1i(textureLocation, 0); // Tell shader to use texture unit 0
		glBindTexture(GL_TEXTURE_2D, mesh->textureID);
	}

    // Get uniform locations
    GLint modelLoc = glGetUniformLocation(mesh->program_id, "model");
    GLint viewLoc = glGetUniformLocation(mesh->program_id, "view");
    GLint projectionLoc = glGetUniformLocation(mesh->program_id, "projection");

    // Set uniform matrix values
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)mesh->modelMatrix);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat*)viewMatrix);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat*)projectionMatrix);

    // Draw the mesh
    buffers_bind_ebo(mesh->buffers.EBO);
    glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error: %d\n", err);
    }

    buffers_unbind_vao();
    buffers_unbind_vbo();
    buffers_unbind_ebo();
}

// Destroy the mesh
void mesh_destroy(Mesh* mesh) {
    if (mesh) {
        glDeleteTextures(1, &mesh->textureID);
        buffers_destroy(&mesh->buffers);
        free(mesh);
    }
}

// Apply translation to the model matrix
void mesh_translate(Mesh* mesh, vec3 translation) {
    mat4 translationMatrix;
    glm_translate_make(translationMatrix, translation);
    glm_mat4_mul(translationMatrix, mesh->modelMatrix, mesh->modelMatrix);
}

// Apply rotation to the model matrix
void mesh_rotate(Mesh* mesh, float angle, vec3 axis) {
    mat4 rotationMatrix;
    glm_rotate_make(rotationMatrix, angle, axis);
    glm_mat4_mul(rotationMatrix, mesh->modelMatrix, mesh->modelMatrix);
}

// Apply scaling to the model matrix
void mesh_scale(Mesh* mesh, vec3 scale) {
    mat4 scaleMatrix;
    glm_scale_make(scaleMatrix, scale);
    glm_mat4_mul(scaleMatrix, mesh->modelMatrix, mesh->modelMatrix);
}

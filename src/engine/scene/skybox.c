#include <scenes/skybox.h>
#include <pipeline/shader.h>
#include <pipeline/buffers.h>

#include <stdio.h>
#include <string.h>

#include <glad/glad.h>
#include <stb_image.h>

#include <cglm/cglm.h>

static float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
};

// Helper function to load the cubemap textures
GLuint loadCubemap(const char* faces[6]) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (GLuint i = 0; i < 6; i++) {
        char path[1024];
        snprintf(path, sizeof(path), "%s", faces[i]);  // Construct the path for each face
        unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            printf("Cubemap texture failed to load at path: %s\n", path);
            stbi_image_free(data);
        }
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void skybox_init(Skybox* skybox, const char* source[6], size_t program_id) {
    // Store the program ID and texture faces
    skybox->program_id = program_id;
    memcpy(skybox->faces, source, sizeof(skybox->faces));

    // Load skybox textures (6 images for each side of the cube)
    GLuint texture_id = loadCubemap(skybox->faces); // Helper function to load cubemap textures
    skybox->texture_id = texture_id;

	// Initialize the buffers for the mesh
    skybox->buffers = buffers_create_empty();
    buffers_bind_vao(skybox->buffers.VAO);

    // Load vertex positions (VBO)
    skybox->buffers.VBO = buffers_create_vbo(skyboxVertices, sizeof(skyboxVertices));
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind buffers after setup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void skybox_use(Skybox* skybox, mat4 projection, mat4 view, mat4 model) {
    glUseProgram(skybox->program_id);
    
    // Set the matrices for the skybox shader
    glUniformMatrix4fv(glGetUniformLocation(skybox->program_id, "model"), 1, GL_TRUE, (const GLfloat*)model);
    glUniformMatrix4fv(glGetUniformLocation(skybox->program_id, "view"), 1, GL_TRUE, (const GLfloat*)view);
    glUniformMatrix4fv(glGetUniformLocation(skybox->program_id, "projection"), 1, GL_TRUE, (const GLfloat*)projection);

    // Bind the cubemap texture
	glUniform1i(glGetUniformLocation(skybox->program_id, "skybox"), 0); // Texture unit 0
	glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->texture_id);

    // Render the skybox cube (disable depth writing to ensure it's rendered in the background)
	glDepthFunc(GL_LEQUAL);  // Draw the skybox behind everything else
	glDepthMask(GL_FALSE);

    buffers_bind_vao(skybox->buffers.VAO);
    buffers_bind_vbo(skybox->buffers.VBO);

    glDrawArrays(GL_TRIANGLES, 0, 36);

	buffers_unbind_vbo();
	buffers_unbind_vao();

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);  // Draw the skybox behind everything else
}

void skybox_destroy(Skybox* skybox) {
    // Cleanup resources
    glDeleteTextures(1, (const GLuint *)&skybox->texture_id);
    glDeleteBuffers(1, &skybox->buffers.VBO);
    glDeleteVertexArrays(1, &skybox->buffers.VAO);
}

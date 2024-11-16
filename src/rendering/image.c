#include "image.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stb_image.h>
#include "buffers.h"
#include "kbd.h"
#include <cglm/cglm.h>

// Helper function to create a texture from image data
static GLuint create_texture_from_data(unsigned char *data, int width, int height, int nrChannels) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
	if (nrChannels == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return texture;
}

void image_init(Image *image, const char *image_path, GLuint shader_program) {
    // Load the image
    int width, height, nrChannels;
    unsigned char *data = stbi_load(image_path, &width, &height, &nrChannels, 0);
    if (!data) {
        fprintf(stderr, "Failed to load image: %s\n", image_path);
        return;
    }

    // Create the texture
    image->shader_program = shader_program;
    image->texture_id = create_texture_from_data(data, width, height, nrChannels);
    image->width = width;
    image->height = height;

    // Create the buffers (VAO, VBO, etc.)
    image->buffers = buffers_create_empty();

    // Setup the vertex data (quad for the image)
    float vertices[4][4] = {
        { 0.0f, (float)height, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f, 1.0f },
        { (float)width, (float)height, 1.0f, 0.0f },
        { (float)width, 0.0f, 1.0f, 1.0f }
    };
    
    GLuint indices[6] = {
        0, 1, 2,
        1, 3, 2
    };

    buffers_bind_vao(image->buffers.VAO);
    
    glGenBuffers(1, &image->buffers.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, image->buffers.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    image->buffers.EBO = buffers_create_ebo(indices, sizeof(indices));

    // Set up vertex attributes
    glBindVertexArray(image->buffers.VAO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));
    glEnableVertexAttribArray(1);

    // Cleanup
    stbi_image_free(data);
}

void image_set_dimensions(Image *image, int new_width, int new_height) {
    if (image == NULL) {
        fprintf(stderr, "Error: Image is NULL.\n");
        return;
    }

    // Update the width and height of the image
    image->width = new_width;
    image->height = new_height;

    // Update the vertex data with the new dimensions
    float vertices[4][4] = {
        { 0.0f, (float)new_height, 0.0f, 0.0f },           // Top-left
        { 0.0f, 0.0f, 0.0f, 1.0f },                          // Bottom-left
        { (float)new_width, (float)new_height, 1.0f, 0.0f }, // Top-right
        { (float)new_width, 0.0f, 1.0f, 1.0f }               // Bottom-right
    };

    // Update the VBO with new vertex data
    glBindBuffer(GL_ARRAY_BUFFER, image->buffers.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}

void image_set_rotation_by_shader(Image *image, float angle_degrees) {
    if (image == NULL || image->shader_program == 0) {
        fprintf(stderr, "Error: Invalid image or shader program.\n");
        return;
    }

    // Convert the angle from degrees to radians
    float angle_radians = glm_rad(angle_degrees);

    // Create the rotation matrix using cglm
    mat4 model;
    glm_mat4_identity(model);
    glm_rotate_z(model, angle_radians, model); // Rotate around Z-axis

	image->rotation = angle_radians;

    // Get the location of the model matrix uniform in the shader
    GLuint model_location = glGetUniformLocation(image->shader_program, "model");
    if (model_location == -1) {
        fprintf(stderr, "Error: Could not find 'model' uniform.\n");
        return;
    }

    // Send the model matrix to the shader
    glUseProgram(image->shader_program);
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (const GLfloat *)model);
}

void image_set_rotation_by_shader_dirty(Image *image, float angle_degrees) {
    if (image == NULL || image->shader_program == 0) {
        fprintf(stderr, "Error: Invalid image or shader program.\n");
        return;
    }

    // Convert the angle from degrees to radians
    float angle_radians = glm_rad(angle_degrees);

    // Create the rotation matrix using cglm
    mat4 model;
    glm_mat4_identity(model);
    glm_rotate_z(model, angle_radians, model); // Rotate around Z-axis

	image->rotation = angle_radians;
	image->model_dirty = true;

    // Get the location of the model matrix uniform in the shader
    GLuint model_location = glGetUniformLocation(image->shader_program, "model");
    if (model_location == -1) {
        fprintf(stderr, "Error: Could not find 'model' uniform.\n");
        return;
    }

    // Send the model matrix to the shader
    glUseProgram(image->shader_program);
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (const GLfloat *)model);
}


// Function to change dimensions by altering the model matrix in the shader
void image_set_dimensions_by_shader(Image *image, float new_width, float new_height) {
    if (image == NULL || image->shader_program == 0) {
        fprintf(stderr, "Error: Invalid image or shader program.\n");
        return;
    }

	image->width = new_width;
	image->height = new_height;

    // Create the scale matrix using cglm
    mat4 model;
    glm_mat4_identity(model);  // Identity matrix
    glm_scale(model, (vec3){new_width, new_height, 1.0f});  // Apply scaling

    // Get the location of the model matrix uniform in the shader
    GLuint model_location = glGetUniformLocation(image->shader_program, "model");
    if (model_location == -1) {
        fprintf(stderr, "Error: Could not find 'model' uniform.\n");
        return;
    }

    // Send the model matrix to the shader
    glUseProgram(image->shader_program);
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (const GLfloat *)model);
}

void image_render(Image *image, float x, float y) {
    if (image == NULL || image->texture_id == 0 || image->buffers.VAO == 0 || image->buffers.VBO == 0) {
        fprintf(stderr, "Error: Invalid image or buffer state.\n");
        return;
    }

    glUseProgram(image->shader_program);  // Use the shader program

    // Set the texture uniform
    GLuint texture_location = glGetUniformLocation(image->shader_program, "texture_sampler");
    if (texture_location == -1) {
        fprintf(stderr, "Error: Could not find 'texture_sampler' uniform.\n");
        return;
    }
    glUniform1i(texture_location, 0);  // Set the texture unit (0)

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);  // Make sure texture unit 0 is active
    glBindTexture(GL_TEXTURE_2D, image->texture_id);

    // Always update the model matrix
    mat4 model;
    glm_mat4_identity(model);  // Initialize to identity matrix

    if (image->model_dirty) {
        // Only center the rotation if the image is dirty
        glm_translate(model, (vec3){x + image->width / 2.0f, y + image->height / 2.0f, 0.0f});  // Move to center
        glm_rotate_z(model, image->rotation, model); // Apply rotation around the center
        glm_translate(model, (vec3){-image->width / 2.0f, -image->height / 2.0f, 0.0f}); // Move back to original position
    } else {
        // Apply normal translation (without centering)
        glm_translate(model, (vec3){x, y, 0.0f});  // Apply normal translation for rendering
    }

    // Apply scaling
    glm_scale(model, (vec3){image->width, image->height, 1.0f});

    // Set the model matrix in the shader
    GLuint model_location = glGetUniformLocation(image->shader_program, "model");
    if (model_location != -1) {
        glUseProgram(image->shader_program);
        glUniformMatrix4fv(model_location, 1, GL_FALSE, (const GLfloat*)model);
    }

    // After updating the model matrix, mark as clean (this flag is optional depending on your workflow)
    image->model_dirty = false; // Reset the dirty flag after update

    // Prepare the vertices (Position and texture coordinates)
    float vertices[4][4] = {
        { 0.0f, 1.0f, 0.0f, 0.0f },  // Top-left
        { 0.0f, 0.0f, 0.0f, 1.0f },  // Bottom-left
        { 1.0f, 1.0f, 1.0f, 0.0f },  // Top-right
        { 1.0f, 0.0f, 1.0f, 1.0f }   // Bottom-right
    };

    // Bind the VAO and VBO
    buffers_bind_vao(image->buffers.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, image->buffers.VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    // Render the quad
    if (image->buffers.EBO != 0) {
        buffers_bind_ebo(image->buffers.EBO);  // Use EBO if it exists
    }

    glDisable(GL_CULL_FACE); // Disable depth test while rendering 2D text
    glDisable(GL_DEPTH_TEST); // Disable face culling while rendering 2D text
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Render in solid mode

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  // Draw the quad using vertex data

    glEnable(GL_DEPTH_TEST); // Re-enable depth test after rendering

    if (wireframeMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Re-enable wireframe mode
    if (!cullingMode)  glEnable(GL_CULL_FACE); // Re-enable face culling after rendering
}

void image_cleanup(Image *image) {
    glDeleteTextures(1, &image->texture_id);
    glDeleteBuffers(1, &image->buffers.VBO);
    if (image->buffers.EBO != 0) {
        glDeleteBuffers(1, &image->buffers.EBO);
    }
    glDeleteVertexArrays(1, &image->buffers.VAO);
}
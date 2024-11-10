#include <GL/glew.h>

#define GLFW_EXPOSE_NATIVE_WGL
#ifdef _WIN64
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <stdio.h>
#include <stdlib.h>

#include "kbd.h"
#include "shader.h"
#include "buffers.h"
#include "qreader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cglm/cglm.h>

#include "camera.h"

double xpos, ypos;
void cursor_pos_callback(GLFWwindow* window, double x_pos, double y_pos) {
	xpos = x_pos;
	ypos = y_pos;
}

int main() {
	// Initialize glfw, if didn't, fprintf and return an error code of -1
	if(!glfwInit()) {
		fprintf(stderr, "Failed to initialize glfw!\n");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DECORATED, FALSE);

	GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
	if (!primary_monitor) {
		fprintf(stderr, "Failed to get primary monitor!\n");
		glfwTerminate();
		return -1;
	}

	// Get the video mode of the primary monitor
	const GLFWvidmode* video_mode = glfwGetVideoMode(primary_monitor);
	if (!video_mode) {
		fprintf(stderr, "Failed to get video mode!\n");
		glfwTerminate();
		return -1;
	}

	// Access screen width and height
	int screen_w = video_mode->width;
	int screen_h = video_mode->height;

	// Create a fullscreen-borderless window
	GLFWwindow* window = glfwCreateWindow(screen_w, screen_h, "lwlaim", NULL, NULL);

	if(!window) {
		fprintf(stderr, "Failed to create window!\n");
		glfwTerminate();
		return -3;
	}

	// Create the opengl context for the window
	glfwMakeContextCurrent(window);

	glewExperimental = GLU_TRUE;
	if(glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize glew!\n");
		return -2;
	}

	// Keyboard callback function is in kbd.c
	glfwSetKeyCallback(window, keyboard_callback);
	// Mouse callback function
	glfwSetCursorPosCallback(window, cursor_pos_callback);

	// Compile shaders and create shader program
	char* vertexShaderSource = read_file("resources/shaders/vertex.glsl");
	char* fragmentShaderSource = read_file("resources/shaders/fragment.glsl");
	if (!vertexShaderSource || !fragmentShaderSource) {
		fprintf(stderr, "Failed to load shader sources!\n");
		return -5;
	}

	ShaderProgram shader = shader_create(vertexShaderSource, fragmentShaderSource);
	free(vertexShaderSource);
	free(fragmentShaderSource);

	if (shader.id == 0) {
		fprintf(stderr, "Shader program creation failed!\n");
		return -4;
	}

	float vertices[] = {
		// Positions
		-0.5f,  0.5f, 0.0f,
		0.5f,  0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f
	};

	float tex_coords[] = {
		// Texture coordinates
		0.0f, 1.0f,  // Top-left
		1.0f, 1.0f,  // Top-right
		1.0f, 0.0f,  // Bottom-right
		0.0f, 0.0f   // Bottom-left
	};

	unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

	// Create buffers with vertex positions and texture coordinates
	Buffers buffers = buffers_create_empty();
	buffers_bind_vao(buffers.VAO);

	// Create VBO for vertex positions
	buffers.VBO = buffers_create_vbo(vertices, 12);
	buffers_bind_vbo(buffers.VBO);

	// Enable the vertex attribute array for position and define it
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);  // Position attribute
	glEnableVertexAttribArray(0);  // Enable position attribute array

	// Create EBO
	buffers.EBO = buffers_create_ebo(indices, 6);
	buffers_bind_ebo(buffers.EBO);

	// Create VBO for texture coordinates
	buffers.TexCoordVBO = buffers_create_vbo(tex_coords, 8);
	buffers_bind_vbo(buffers.TexCoordVBO);

	// Enable the vertex attribute array for texture coordinates and define it
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);  // Texture coord attribute
	glEnableVertexAttribArray(1);  // Enable texture coordinate attribute array

	// You can unbind the VBO and VAO after configuring everything
	buffers_unbind_vbo();
	buffers_unbind_vao();

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int width, height, nrChannels;
	unsigned char *data = stbi_load("resources/prototype/image.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// Texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	} else {
		fprintf(stderr, "Failed to load texture!\n");
		return -6;
	}
	stbi_image_free(data);

	Camera camera;
    camera_init(&camera, (vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 1.0f, 0.0f}, -90.0f, 0.0f);

    float deltaTime = 0.0f, lastFrame = 0.0f;
    mat4 view, projection;

	glfwSwapInterval(1);

	// Main loop
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

		glfwPollEvents();

		// Get framebuffer size
		int framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

		// Set the viewport to cover the entire framebuffer
		glViewport(0, 0, framebufferWidth, framebufferHeight);

        // Handle input
        camera_process_keyboard(&camera, window, deltaTime);
		camera_process_mouse(&camera, xpos, ypos);

		// Use the shader program
		shader_use(&shader);

		camera_update(&camera);

		// Set up the MVP matrices
        mat4 model = GLM_MAT4_IDENTITY_INIT;
        camera_get_view_matrix(&camera, view);  // Pass the view matrix to be updated
        camera_get_projection_matrix(&camera, projection, framebufferWidth, framebufferHeight);  // Pass the projection matrix

        // Render the scene
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        GLint modelLoc = glGetUniformLocation(shader.id, "model");
        GLint viewLoc = glGetUniformLocation(shader.id, "view");
        GLint projectionLoc = glGetUniformLocation(shader.id, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)model);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat*)view);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat*)projection);

		// Bind texture to texture unit 0
		buffers_bind_vbo(buffers.TexCoordVBO);
		GLuint textureLocation = glGetUniformLocation(shader.id, "texture1");
		glUniform1i(textureLocation, 0); // Tell shader to use texture unit 0
		glActiveTexture(GL_TEXTURE0);    // Activate texture unit 0
		glBindTexture(GL_TEXTURE_2D, texture);

		// Bind buffers and draw
		buffers_bind_vao(buffers.VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		buffers_unbind_vao();

		// Swap buffers
		glfwSwapBuffers(window);
	}

	// Clean up resources
	shader_destroy(&shader);
	buffers_destroy(&buffers);
	glDeleteTextures(1, &texture);

	// Cleanup
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
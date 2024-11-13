#include <GL/glew.h>

#define GLFW_EXPOSE_NATIVE_WGL
#ifdef _WIN64
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <stdio.h>
#include <stdlib.h>

#include "kbd.h"
#include "mue.h"

#include "shader.h"
#include "buffers.h"
#include "qreader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cglm/cglm.h>
#include "camera.h"
#include "crosshair.h"
#include "swp.h"

#include "model_loader.h"  // Include the model loader header
#include "text.h"
#include "text_projection.h"

Crosshair crosshair;

int main() {
    // Initialize glfw
    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialize glfw!\n");
        return -1;
    }
    printf("GLFW initialized.\n");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, FALSE);
	glfwWindowHint(GLFW_SAMPLES, 16);

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
    printf("Window created.\n");

    // Create the OpenGL context for the window
    glfwMakeContextCurrent(window);

    glewExperimental = GLU_TRUE;
    if(glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize glew!\n");
        return -2;
    }
    printf("GLEW initialized.\n");

    // Keyboard and mouse callback functions
    glfwSetKeyCallback(window, keyboard_callback);
    glfwSetCursorPosCallback(window, cursor_callback);

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
    printf("Main shader program created.\n");

    // Load the OBJ model
    Model model;
    if (!load_obj("resources/models/sphere.obj", &model)) {
        fprintf(stderr, "Failed to load OBJ file!\n");
        return -6;
    }
    printf("OBJ model loaded.\n");

    // Create buffers and upload model data to GPU
    Buffers buffers = buffers_create_empty();
    buffers_bind_vao(buffers.VAO);

    // Create VBO for vertex positions
    buffers.VBO = buffers_create_vbo(model.vertices, model.vertex_count * 3);
    buffers_bind_vbo(buffers.VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Create VBO for texture coordinates
    if (model.texcoord_count > 0) {
        buffers.TexCoordVBO = buffers_create_vbo(model.texcoords, model.texcoord_count * 2);
        buffers_bind_vbo(buffers.TexCoordVBO);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
    }
    
    // Create VBO for normals
    if (model.normal_count > 0) {
        buffers.NormalVBO = buffers_create_vbo(model.normals, model.normal_count * 3);
        buffers_bind_vbo(buffers.NormalVBO);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);
    }

    // Create EBO for indices
    buffers.EBO = buffers_create_ebo(model.indices, model.index_count);
    buffers_bind_ebo(buffers.EBO);

    // Unbind buffers and VAO
    buffers_unbind_vbo();
    buffers_unbind_vao();
    printf("Buffers and VAOs created and bound.\n");

    // Load the texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

	stbi_set_flip_vertically_on_load(1);

	int width, height, nrChannels;
	unsigned char *data = stbi_load("resources/prototype/image.jpg", &width, &height, &nrChannels, 0);
	if (data) {
		// Check if the image has an alpha channel (RGBA)
		GLenum format = GL_RGB;
		if (nrChannels == 4) {
			format = GL_RGBA;  // Use RGBA format for textures with an alpha channel
		}

		// Create the texture
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

		// Generate mipmaps for better performance at various distances
		glGenerateMipmap(GL_TEXTURE_2D);

		// Set texture parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		printf("Texture loaded with %d channels.\n", nrChannels);  // Output channel info
	} else {
		fprintf(stderr, "Failed to load texture!\n");
		return -7;
	}
	stbi_image_free(data);

    Camera camera;
    camera_init(&camera, (vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 1.0f, 0.0f}, -90.0f, 0.0f);
    printf("Camera initialized.\n");

    float deltaTime = 0.0f, lastFrame = 0.0f;
    mat4 view, projection, text_projection;
	

    glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);  // Enable back-face culling
    glCullFace(GL_BACK);     // Cull the back faces (if not front-facing)
    glFrontFace(GL_CCW);     // Set front face counter-clockwise (default)

    glfwSwapInterval(0);

	vec3 crosshairColor = {1.0f, 1.0f, 0.0f}; // White color
    float crosshairSize = 2.0f; // Adjust crosshair size as needed
    float crosshairThickness = 4.0f; // Adjust crosshair size as needed
    crosshair_init(&crosshair, crosshairSize, crosshairThickness, crosshairColor);

	Ray ray;
	vec3 color = {1.0f, 1.0f, 1.0f};  // White color

	// Initialize text renderer
	// Compile shaders and create shader program
    char* t_vertexShaderSource = read_file("resources/shaders/text/vertex.glsl");
    char* t_fragmentShaderSource = read_file("resources/shaders/text/fragment.glsl");
    if (!t_vertexShaderSource || !t_fragmentShaderSource) {
        fprintf(stderr, "Failed to load text shader sources!\n");
        return -5;
    }

	ShaderProgram text_shader = shader_create(t_vertexShaderSource, t_fragmentShaderSource);
    free(t_vertexShaderSource);
    free(t_fragmentShaderSource);

    if (text_shader.id == 0) {
        fprintf(stderr, "Shader program creation failed!\n");
        return -4;
    }
    printf("Text shader program created.\n");

	// Initialize Font
    Font font;
    font_init(&font, "resources/opensans-light.ttf", 22.0f, text_shader.id);  // Adjust path and size as needed

    // Variables to calculate FPS
	float frameCount = 0;
	float lastTime = 0.0f;
	float fps = 0.0f;

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();

		// Calculate FPS every second
		frameCount++;
		if (currentFrame - lastTime >= 1.0f) { // If one second has passed
			fps = frameCount;
			frameCount = 0;
			lastTime = currentFrame;
		}

		// Get framebuffer size
		int framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
		glViewport(0, 0, framebufferWidth, framebufferHeight);

		// Handle input
		camera_process_keyboard(&camera, window, deltaTime);
		camera_process_mouse(&camera, cursor_x_position, cursor_y_position);

		// Use the shader program
		shader_use(&shader);

		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);

		// Render the scene
		camera_update(&camera);

		// Set up the MVP matrices
		mat4 modelMatrix = GLM_MAT4_IDENTITY_INIT;
		camera_get_view_matrix(&camera, view);
		camera_get_projection_matrix(&camera, projection, framebufferWidth, framebufferHeight);

		screen_to_ray(
			framebufferWidth/2, framebufferHeight/2, 
			framebufferWidth, framebufferHeight, 
			view, projection, &ray);

		GLint modelLoc = glGetUniformLocation(shader.id, "model");
		GLint viewLoc = glGetUniformLocation(shader.id, "view");
		GLint projectionLoc = glGetUniformLocation(shader.id, "projection");

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)modelMatrix);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat*)view);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat*)projection);

		// Bind texture to texture unit 0
		GLuint textureLocation = glGetUniformLocation(shader.id, "texture1");
		glUniform1i(textureLocation, 0); // Tell shader to use texture unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Bind buffers and draw
		buffers_bind_vao(buffers.VAO);
		glDrawElements(GL_TRIANGLES, model.index_count, GL_UNSIGNED_INT, 0);
		buffers_unbind_vao();
		buffers_unbind_vbo();
		buffers_unbind_ebo();

		crosshair_render(&crosshair, framebufferWidth, framebufferHeight);

		buffers_unbind_vao();
		buffers_unbind_vbo();
		buffers_unbind_ebo();

		// Use text shader program
		shader_use(&text_shader);

		// Text projection setup
		setup_text_projection(framebufferWidth, framebufferHeight, text_projection);

		// Set projection matrix in shader
		GLuint proj_loc = glGetUniformLocation(font.shader_program, "projection");
		glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (const GLfloat*)text_projection);

		// Render info text
		font_render_text(&font, "lwlaim beta v0.0", 4.0f, 0.0f, color);
		font_render_text(&font, "lightweight aim training", 4.0f, 24.0f, color);
		// Render FPS text
		char fpsText[32];
		snprintf(fpsText, sizeof(fpsText), "frames per second: %.0f", fps);
		font_render_text(&font, fpsText, 4.0f, 46.0f, color); // Display at top-left

		// Swap buffers
		glfwSwapBuffers(window);
	}

    // Clean up resources
	font_cleanup(&font);
	crosshair_destroy(&crosshair);
    shader_destroy(&shader);
    buffers_destroy(&buffers);
    glDeleteTextures(1, &texture);
    printf("Resources cleaned up.\n");

    // Close window and terminate
    glfwDestroyWindow(window);
    glfwTerminate();
    printf("GLFW terminated.\n");

    return 0;
}
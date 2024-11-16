#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "scene.h"
#include "camera.h"
#include "mue.h"
#include "kbd.h"
#include "shader.h"
#include "buffers.h"
#include "crosshair.h"
#include "model_loader.h"
#include "text.h"
#include "ortho_projection.h"
#include "image.h"
#include "qreader.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cglm/cglm.h>

static ShaderProgram shader;
static ShaderProgram image_shader;
static ShaderProgram text_shader;
static Buffers buffers;

static Crosshair crosshair;

static Model model;
static GLuint texture;
static Camera camera;
    
static float deltaTime = 0.0f, lastFrame = 0.0f;
static mat4 view, projection, text_projection, image_projection;

static vec3 crosshairColor = {1.0f, 1.0f, 0.0f}; // White color
static float crosshairSize = 2.0f; // Adjust crosshair size as needed
static float crosshairThickness = 4.0f; // Adjust crosshair size as needed

static Font font;
static vec3 color = { 1.0f, 1.0f, 1.0f };
static float font_size = 32.0f;

// Variables to calculate FPS
static float frameCount = 0;
static float lastTime = 0.0f;
static float fps = 0.0f;

// Declare an image
static Image background_image;
	
void default_scene_update(Scene* self) {
	// Get framebuffer size
	int framebufferWidth, framebufferHeight;
	glfwGetFramebufferSize(self->window, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);

    float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	// Calculate FPS every second
	frameCount++;
	if (currentFrame - lastTime >= 1.0f) { // If one second has passed
		fps = frameCount;
		frameCount = 0;
		lastTime = currentFrame;
	}

	// Handle input
	camera_process_keyboard(&camera, self->window, deltaTime);
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

	// screen_to_ray(
	// 	framebufferWidth/2, framebufferHeight/2, 
	// 	framebufferWidth, framebufferHeight, 
	// 	view, projection, &ray);

	GLint modelLoc = glGetUniformLocation(shader.id, "model");
	GLint viewLoc = glGetUniformLocation(shader.id, "view");
	GLint projectionLoc = glGetUniformLocation(shader.id, "projection");

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, (const GLfloat*)modelMatrix);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, (const GLfloat*)view);
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, (const GLfloat*)projection);

	// Bind texture to texture unit 0
	GLuint textureLocation = glGetUniformLocation(shader.id, "texture1");
	glUniform1i(textureLocation, 0); // Tell shader to use texture unit 0
	glBindTexture(GL_TEXTURE_2D, texture);

	// Bind buffers and draw
	buffers_bind_vao(buffers.VAO);
	glDrawElements(GL_TRIANGLES, model.index_count, GL_UNSIGNED_INT, 0);
	buffers_unbind_vao();
	buffers_unbind_vbo();
	buffers_unbind_ebo();

	// Use image shader program
	shader_use(&image_shader);

	// Ortho projection setup for image
	setup_ortho_projection(framebufferWidth, framebufferHeight, image_projection);
	
	// Set projection matrix in shader
	GLuint img_proj_loc = glGetUniformLocation(background_image.shader_program, "projection");
	glUniformMatrix4fv(img_proj_loc, 1, GL_FALSE, (const GLfloat*)image_projection);

	// Render 2D Image (background)
	image_set_dimensions_by_shader(&background_image, 260.0f, 260.0f);
	// image_set_rotation_by_shader(&background_image, glfwGetTime() * 150.0f);
	image_render(&background_image, 4.0f, 140.0f); // Render the loaded background image
	// Use text shader program
	shader_use(&text_shader);

	// Ortho projection setup for text
	setup_ortho_projection(framebufferWidth, framebufferHeight, text_projection);
	
	// Set projection matrix in shader
	GLuint proj_loc = glGetUniformLocation(font.shader_program, "projection");
	glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (const GLfloat*)text_projection);

	// Render info text
	font_render_text(&font, "lwlaim beta v0.0", 4.0f, 0.0f, color);
	font_render_text(&font, "lightweight aim training", 4.0f, (font_size + 2.0f), color);
	// Render FPS text
	char fpsText[32];
	snprintf(fpsText, sizeof(fpsText), "frames per second: %.0f", fps);
	font_render_text(&font, fpsText, 4.0f, ((font_size * 2.0f) + 2.0f), color); // Display at top-left

	// Render Player Health
	char health[32]; // Declare the array
	const char *player_health = scene_state_get(&self->state, "player_health");
	// Copy the string into the array
	snprintf(health, sizeof(health), "Player health: %.0f", atof(player_health)); // Assume it returns a numeric value as string
	font_render_text(&font, health, 4.0f, ((font_size * 3.0f) + 2.0f), color); // Display at top-left

	// Render crosshair
	crosshair_render(&crosshair, framebufferWidth, framebufferHeight);

	buffers_unbind_vao();
	buffers_unbind_vbo();
	buffers_unbind_ebo();
}

void default_scene_render(Scene* self) {
	// Compile shaders and create shader program
    char* vertexShaderSource = read_file("resources/shaders/vertex.glsl");
    char* fragmentShaderSource = read_file("resources/shaders/fragment.glsl");
    if (!vertexShaderSource || !fragmentShaderSource) {
        fprintf(stderr, "Failed to load shader sources!\n");
        return;
    }

    shader = shader_create(vertexShaderSource, fragmentShaderSource);
    free(vertexShaderSource);
    free(fragmentShaderSource);

    if (shader.id == 0) {
        fprintf(stderr, "Shader program creation failed!\n");
        return;
    }
    printf("Main shader program created.\n");

	// Initialize text renderer
	// Compile shaders and create shader program
    char* t_vertexShaderSource = read_file("resources/shaders/text/vertex.glsl");
    char* t_fragmentShaderSource = read_file("resources/shaders/text/fragment.glsl");
    if (!t_vertexShaderSource || !t_fragmentShaderSource) {
        fprintf(stderr, "Failed to load text shader sources!\n");
        return;
    }

	text_shader = shader_create(t_vertexShaderSource, t_fragmentShaderSource);
    free(t_vertexShaderSource);
    free(t_fragmentShaderSource);

    if (text_shader.id == 0) {
        fprintf(stderr, "Shader program creation failed!\n");
        return;
    }
    printf("Text shader program created.\n");

	// Initialize image renderer
	// Compile shaders and create shader program
    char* i_vertexShaderSource = read_file("resources/shaders/image/vertex.glsl");
    char* i_fragmentShaderSource = read_file("resources/shaders/image/fragment.glsl");
    if (!t_vertexShaderSource || !t_fragmentShaderSource) {
        fprintf(stderr, "Failed to load image shader sources!\n");
        return;
    }

	image_shader = shader_create(i_vertexShaderSource, i_fragmentShaderSource);
    free(i_vertexShaderSource);
    free(i_fragmentShaderSource);

    if (image_shader.id == 0) {
        fprintf(stderr, "Shader program creation failed!\n");
        return;
    }
    printf("Image shader program created.\n");

	if (!load_obj("resources/models/sphere.obj", &model)) {
        fprintf(stderr, "Failed to load OBJ file!\n");
        return;
    }
    printf("OBJ model loaded.\n");

	buffers = buffers_create_empty();
	  // Create buffers and upload model data to GPU
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
		return;
	}
	stbi_image_free(data);

	camera_init(&camera, (vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 1.0f, 0.0f}, -90.0f, 0.0f);
    printf("Camera initialized.\n");

	crosshair_init(&crosshair, crosshairSize, crosshairThickness, crosshairColor);

	// Initialize Font
	float font_size = 24.0f;
    font_init(&font, "resources/helvetica.ttf", font_size, text_shader.id);  // Adjust path and size as needed

	// Initialize background image
	image_init(&background_image, "resources/prototype/image.png", image_shader.id);
	image_set_dimensions(&background_image, 1024, 1024);
	printf("Initialized background_image\n");
}

void default_scene_cleanup() {
	// Clean up resources
	image_cleanup(&background_image);
	font_cleanup(&font);
	crosshair_destroy(&crosshair);
    shader_destroy(&shader);
    buffers_destroy(&buffers);
    glDeleteTextures(1, &texture);
    printf("Resources cleaned up.\n");
}
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "scene.h"
#include "mue.h"
#include "kbd.h"
#include "shader.h"
#include "buffers.h"
#include "text.h"
#include "ortho_projection.h"
#include "image.h"
#include "qreader.h"

#include <stb_image.h>
#include <cglm/cglm.h>

static ShaderProgram image_shader;
static ShaderProgram text_shader;
static Buffers buffers;

static float deltaTime = 0.0f, lastFrame = 0.0f;
static mat4 view, projection, text_projection, image_projection;

static vec3 crosshairColor = {1.0f, 1.0f, 0.0f}; // White color
static float crosshairSize = 2.0f; // Adjust crosshair size as needed
static float crosshairThickness = 4.0f; // Adjust crosshair size as needed

static Font font;
static vec3 color = { 1.0f, 1.0f, 1.0f };
static float font_size = 34.0f;

static const char* loading_text = "Processing data..";
static const char* notify_text = "This might take some time, have a snack while it's loading!";

// Declare an image
static Image background_image;

static double start_time;  // Store the start time
	
void splash_scene_update(Scene* self) {
	// Get framebuffer size
	int framebufferWidth, framebufferHeight;
	glfwGetFramebufferSize(self->window, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);

	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Use image shader program
	shader_use(&image_shader);

	// Ortho projection setup for image
	setup_ortho_projection(framebufferWidth, framebufferHeight, image_projection);
	
	// Set projection matrix in shader
	GLuint img_proj_loc = glGetUniformLocation(background_image.shader_program, "projection");
	glUniformMatrix4fv(img_proj_loc, 1, GL_FALSE, (const GLfloat*)image_projection);

	// Render 2D Image (background)
	float img_width = 36.0f, img_height = 36.0f; 
	image_set_dimensions_by_shader(&background_image, img_width, img_height);
	image_set_rotation_by_shader_dirty(&background_image, glfwGetTime() * -400.0f);
	image_render(&background_image, (framebufferWidth/2.0f) - (img_width/2.0f), (framebufferHeight/2.0f) - (img_height/2.0f)); // Render the loaded background image
	// Use text shader program
	shader_use(&text_shader);

	// Ortho projection setup for text
	setup_ortho_projection(framebufferWidth, framebufferHeight, text_projection);

	// Set projection matrix in shader
	GLuint proj_loc = glGetUniformLocation(font.shader_program, "projection");
	glUniformMatrix4fv(proj_loc, 1, GL_FALSE, (const GLfloat*)text_projection);

	// Calculate text width and height
	float text_width = 0.0f;
	float text_m_width = 0.0f;
	float text_height = 0.0f;
	font_get_text_dimensions(&font, loading_text, &text_width, &text_height);
	font_get_text_dimensions(&font, notify_text, &text_m_width, &text_height);

	// Calculate centered position
	float text_x = (framebufferWidth - text_width) / 2.0f;
	float text_xm = (framebufferWidth - text_m_width) / 2.0f;
	float text_y = (framebufferHeight - text_height) / 2.0f;

	// Render text at the calculated position
	font_render_text(&font, loading_text, text_x, text_y + font_size, color); // Centered text
	font_render_text(&font, notify_text, text_xm, text_y + (font_size * 2.0f) + 2.0f, color); // Centered text

	buffers_unbind_vao();
	buffers_unbind_vbo();
	buffers_unbind_ebo();

	// Get the current time
    double current_time = glfwGetTime();

    // If 8 seconds have passed, update the state to "loaded"
    if (current_time - start_time >= 8.0) {
        scene_state_set(&self->state, "loaded", "1"); // Set the loaded state
    }
}

void splash_scene_render(Scene* self) {
	start_time = glfwGetTime();
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

	// Initialize Font
    font_init(&font, "resources/ss-m.ttf", font_size, text_shader.id);  // Adjust path and size as needed

	// Initialize background image
	image_init(&background_image, "resources/prototype/loading.png", image_shader.id);
	image_set_dimensions(&background_image, 1024, 1024);
	printf("Initialized background_image\n");
}

void splash_scene_cleanup() {
	image_cleanup(&background_image);
	font_cleanup(&font);
}
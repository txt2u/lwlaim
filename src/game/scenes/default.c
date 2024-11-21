#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <scenes/scene.h>

#include <pipeline/shader.h>
#include <pipeline/buffers.h>

#include <projections/camera.h>
#include <projections/ortho.h>

#include <input/mue.h>
#include <input/kbd.h>

#include <ui/crosshair.h>
#include <ui/text.h>
#include <ui/image.h>

#include <qreader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cglm/cglm.h>

#include <entities/drawable.h>
#include <entities/mesh.h>
#include <entities/model.h>
#include <entities/ecs.h>

static ShaderProgram shader;
static ShaderProgram image_shader;
static ShaderProgram text_shader;
static Buffers buffers;

static Crosshair crosshair;

static GLuint texture;
static Camera camera;
    
static float deltaTime = 0.0f, lastFrame = 0.0f;
static mat4 view, projection, text_projection, image_projection;

static vec4 crosshairColor = {1.0f, 1.0f, 1.0f, 0.2f}; // White color
static float crosshairSize = 4.0f; // Adjust crosshair size as needed
static float crosshairThickness = 6.0f; // Adjust crosshair size as needed

static Font font;
static vec3 color = { 1.0f, 1.0f, 1.0f };
static float font_size = 18.0f;

// Variables to calculate FPS
static float frameCount = 0;
static float lastTime = 0.0f;
static float fps = 0.0f;

// Declare an image
static Image background_image;

static Model model; // A struct to hold GLTF model data
static Drawable drawable;

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

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Render the scene
	camera_update(&camera);

	// Set up the MVP matrices
	camera_get_view_matrix(&camera, view);
	camera_get_projection_matrix(&camera, projection, framebufferWidth, framebufferHeight);
	
	// Set projection matrix in shader
	GLuint model_loc = glGetUniformLocation(shader.id, "model");
	GLuint view_loc = glGetUniformLocation(shader.id, "view");
	GLuint projection_loc = glGetUniformLocation(shader.id, "projection");

	glUniformMatrix4fv(model_loc, 1, GL_FALSE, (const GLfloat*)drawable.model_matrix);
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, (const GLfloat*)view);
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, (const GLfloat*)projection);

	draw_manager_draw(&drawable);
	draw_manager_translate(&drawable, (vec3){ 1.0f, 1.0f, 1.0f });
	draw_manager_scale(&drawable, (vec3){ 1.0f, 1.0f, 1.0f });

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

	// ! LOAD A BASIC GLTF MODEL HERE
    if (!model_load_gltf(&model, "resources/models/cube.gltf")) {
        fprintf(stderr, "Failed to load GLTF model!\n");
        return;
    }
    printf("GLTF model loaded successfully.\n");
	draw_manager_init_from_mesh(&drawable, model.meshes[0]);

	stbi_set_flip_vertically_on_load(1);

	camera_init(&camera, (vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 1.0f, 0.0f}, -90.0f, 0.0f);
    printf("Camera initialized.\n");

	crosshair_init(&crosshair, crosshairSize, crosshairThickness, crosshairColor);

	// Initialize Font
    font_init(&font, "resources/vcr_osd_mono.ttf", font_size, 3.0f, text_shader.id);  // Adjust path and size as needed

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
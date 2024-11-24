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
#include <ui/widgets/button.h>

#include <qreader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cglm/cglm.h>

#include <entities/drawable.h>
#include <entities/mesh.h>
#include <entities/model.h>
#include <entities/ecs.h>

#include <output/sound.h>
#include <wav.h>

static ShaderProgram shader;
static ShaderProgram image_shader;
static ShaderProgram text_shader;
static ShaderProgram button_shader;
static Buffers buffers;

static Crosshair crosshair;

static GLuint texture;
static Camera camera;
    
static float deltaTime = 0.0f, lastFrame = 0.0f;
static mat4 view, projection, text_projection, image_projection, button_projection;

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

static Sound sound;
static Button my_button;

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

	// Pass the model matrix, view matrix, and projection matrix to the shader
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, (const GLfloat*)model.transform_matrix); // Global model matrix
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, (const GLfloat*)view);
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, (const GLfloat*)projection);

	// Bind the texture
	glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
	glBindTexture(GL_TEXTURE_2D, model.texture_id);

	// Pass the texture to the shader
	GLuint texture_loc = glGetUniformLocation(shader.id, "texture1");
	glUniform1i(texture_loc, 0);  // Set it to texture unit 0

	// For each mesh in the model, apply the mesh's local transformation
	for (int i = 0; i < model.mesh_count; i++) {
		// Combine the mesh's local transformation with the model's global transformation
		// ! vvvv enable if apply transform to parent is set to "true" vvvv
		// mat4 combined_transform;
		// glm_mat4_mul(model.transform_matrix, model.meshes[i]->transform_matrix, combined_transform);
		// ! ^^^^ enable if apply transform to parent is set to "true" ^^^^

		// Pass the combined transformation matrix to the shader
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, (const GLfloat*)model.transform_matrix);  // Use combined_transform here

		// Draw the mesh with the combined transformation
		draw_manager_draw(&drawable, model.meshes[i]->name);
	}

	// Use image shader program
	shader_use(&image_shader);

	// Ortho projection setup for image
	setup_ortho_projection(framebufferWidth, framebufferHeight, image_projection);
	
	// Set projection matrix in shader
	GLuint img_proj_loc = glGetUniformLocation(background_image.shader_program, "projection");
	glUniformMatrix4fv(img_proj_loc, 1, GL_FALSE, (const GLfloat*)image_projection);

	// Render 2D Image (background)
	image_set_dimensions_by_shader(&background_image, 64.0f, 64.0f);
	// image_set_rotation_by_shader(&background_image, glfwGetTime() * 150.0f);
	image_render(&background_image, 4.0f, 140.0f); // Render the loaded background image

	// Use Button shader program
	shader_use(&button_shader);

	// Ortho projection setup for image
	setup_ortho_projection(framebufferWidth, framebufferHeight, button_projection);

	// Set projection matrix in shader
	GLuint btn_proj_loc = glGetUniformLocation(my_button.shader_program, "projection");
	glUniformMatrix4fv(btn_proj_loc, 1, GL_FALSE, (const GLfloat*)button_projection);

    button_render(&my_button, 0.0f, 240.0f, framebufferWidth, framebufferHeight);

	bool hover = button_check_hover(&my_button, cursor_x_position, cursor_y_position);
	// bool click = button_check_click(&my_button, cursor_x_position, cursor_y_position, glfwGetMouseButton(self->window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

	if (hover) {
		button_scale(&my_button, 1.1f, 1.1f);
		button_change_color(&my_button, (vec4){0.0f, 0.0f, 0.7f, 1.0f});
	} else {
		button_scale(&my_button, 1.0f, 1.0f);
		button_change_color(&my_button, (vec4){0.0f, 0.0f, 0.7f, 0.5f});
	}

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
    if (!i_vertexShaderSource || !i_fragmentShaderSource) {
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

	// Initialize image renderer
	// Compile shaders and create shader program
    char* b_vertexShaderSource = read_file("resources/shaders/button/vertex.glsl");
    char* b_fragmentShaderSource = read_file("resources/shaders/button/fragment.glsl");
    if (!b_vertexShaderSource || !b_fragmentShaderSource) {
        fprintf(stderr, "Failed to load image shader sources!\n");
        return;
    }

	button_shader = shader_create(b_vertexShaderSource, b_fragmentShaderSource);
    free(b_vertexShaderSource);
    free(b_fragmentShaderSource);

    if (button_shader.id == 0) {
        fprintf(stderr, "Shader program creation failed!\n");
        return;
    }
    printf("Button shader program created.\n");

	// ! LOAD GLTF MODEL HERE
    if (!model_load_gltf(
		&model, 
		"resources/static/psx_male_character.jpg", 
		"resources/static/psx_male_character.gltf",
		false)
	) {
        fprintf(stderr, "Failed to load GLTF model!\n");
        return;
    }
    printf("Loaded gltf model!\n");

	model_set_position(&model, GLM_VEC3_ZERO);
	model_set_scale(&model, (vec3){ 4.0f, 4.0f, 4.0f });
	model_set_rotation(&model, (vec4){ 0.0f, 0.0f, 0.0f, 1.0f });
	model_apply_transform(&model);

	// Initialize the meshes as drawables
    for (int i = 0; i < model.mesh_count; i++) {
        // Initialize drawable for each mesh
        draw_manager_init_from_mesh(&drawable, model.meshes[i], model.meshes[i]->name);
    }

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

	button_init(
		&my_button, "Hover me", 
		100.0f, 100.0f, 200.0f, 60.0f, 
		button_shader.id, BUTTON_TYPE_COLOR, 0, 
		(vec4){0.2f, 0.0f, 0.0f, 1.0f}, 
		&font, (vec3){1.0f, 1.0f, 1.0f}
	);

	sound_initialize();
	alGenBuffers(1, &sound.buffer);
	alGenSources(1, &sound.source);

	ALsizei size, freq;
	ALenum format;
	ALvoid* data;
	load_wav("resources/audio/stress.wav", &format, &data, &size, &freq); // Implement your loader
	alBufferData(sound.buffer, format, data, size, freq);
	free(data);

	// Attach the buffer to the source
	sound_attach_buffer(&sound);

	// Initialize sound properties
	sound_set_volume(&sound, 0.0f);

	// Play the sound
	sound_play_once(&sound);
}

void default_scene_cleanup() {
	// Clean up resources
	image_cleanup(&background_image);
	font_cleanup(&font);
	model_free(&model);
	draw_manager_destroy(&drawable);
	sound_cleanup();
	crosshair_destroy(&crosshair);
    shader_destroy(&shader);
    buffers_destroy(&buffers);
    glDeleteTextures(1, &texture);
    printf("Resources cleaned up.\n");
}
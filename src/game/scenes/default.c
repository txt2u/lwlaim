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

#include <lighting/light.h>
#include <entities/static/cube.h>

#include <scenes/skybox.h>

#include <output/sound.h>
#include <wav.h>

static ShaderProgram shader, image_shader, text_shader, button_shader, skybox_shader;
static Buffers buffers;

static Crosshair crosshair;

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

static Model player_model;
static Drawable p_drawable;

static Sound sound;
static Button my_button;

static Cube DebugLightCube;
static Light PointLight;
static vec3 LightPosition;

// ^ >>>>>>>>>>>>>> Skybox 
static Skybox skybox;
static mat4 skybox_view, skybox_projection, skybox_model;
// ^ <<<<<<<<<<<<<< Skybox 

// & Default scene shaders
static void setup_default_scene_shaders() {
	// ! Default Shader
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

	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// ! Text Shader
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
	
	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// ! Image Shader
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
	
	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// ! Button Shader
	// Initialize button renderer
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
	
	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// ! Skybox Shader
	// Initialize skybox renderer
	// Compile shaders and create shader program
    char* sk_vertexShaderSource = read_file("resources/shaders/skybox/vertex.glsl");
    char* sk_fragmentShaderSource = read_file("resources/shaders/skybox/fragment.glsl");
    if (!sk_vertexShaderSource || !sk_fragmentShaderSource) {
        fprintf(stderr, "Failed to load image shader sources!\n");
        return;
    }

	skybox_shader = shader_create(sk_vertexShaderSource, sk_fragmentShaderSource);
    free(sk_vertexShaderSource);
    free(sk_fragmentShaderSource);
    printf("Skybox shader program created.\n");
	
	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

// Function to print a 4x4 matrix for debugging
void print_matrix(const char* name, mat4 matrix) {
    printf("Matrix: %s\n", name);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            printf("%8.3f ", matrix[row][col]);
        }
        printf("\n");
    }
    printf("\n");
}

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

	// Get the MVP matrices
	camera_get_view_matrix(&camera, view);
	camera_get_projection_matrix(&camera, projection, (float)framebufferWidth, (float)framebufferHeight);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the skybox shader program
	shader_use(&skybox_shader);

	// Start with the identity matrix for the skybox view
	glm_mat4_identity(skybox_view);
	glm_mat4_identity(skybox_model);

	glm_rotate(skybox_view, glm_rad(-camera.yaw), (vec3){ 0.0f, 1.0f, 0.0f });
	glm_rotate(skybox_view, glm_rad(camera.pitch), (vec3){ 1.0f, 0.0f, 0.0f });

	// Set translation part of the view matrix to 0, keeping only the rotation for the skybox
	skybox_view[3][0] = 0.0f; // tx
	skybox_view[3][1] = 0.0f; // ty
	skybox_view[3][2] = 0.0f; // tz
	skybox_view[3][3] = 1.0f; // hom

	// Apply a scaling transformation to the skybox model matrix
	float scale_factor = 0.01f; // Adjust this value to control the size of the skybox
	glm_scale(skybox_model, (vec3){scale_factor, scale_factor, scale_factor}); // Scale the model matrix

	glm_perspective(glm_rad(camera.fov), 
		(float)framebufferWidth / (float)framebufferHeight, 
		camera.near, 
		camera.far * 1000.0f, 
		skybox_projection);

	// Use the skybox for rendering
	skybox_use(&skybox, skybox_projection, skybox_view, skybox_model);

	// Use the shader program
	shader_use(&shader);

	// Render the scene
	camera_update(&camera);

	// Set projection matrix in shader
	GLuint model_loc = glGetUniformLocation(shader.id, "model");
	GLuint view_loc = glGetUniformLocation(shader.id, "view");
	GLuint projection_loc = glGetUniformLocation(shader.id, "projection");

	// Pass the model matrix, view matrix, and projection matrix to the shader
	glUniformMatrix4fv(model_loc, 1, GL_FALSE, (const GLfloat*)model.transform_matrix); // Global model matrix
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, (const GLfloat*)view);
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, (const GLfloat*)projection);

	// Pass the texture to the shader
	GLuint texture_loc = glGetUniformLocation(shader.id, "texture1");
	glUniform1i(texture_loc, 0);  // Set it to texture unit 0

	// ! Point light.
	create_point_light(&PointLight, LightPosition, (vec3){0.9f, 0.87f, 0.9f}, 1.0f);

	// Pass the camera position into the fragment shader uniform.
	GLuint camera_position = glGetUniformLocation(shader.id, "cameraPosition");
    glUniform3fv(camera_position, 1, camera.position);

	// For each mesh in the model, apply the mesh's local transformation
	for (int i = 0; i < model.mesh_count; i++) {
		// Combine the mesh's local transformation with the model's global transformation
		// ! vvvv enable if apply transform to parent is set to "true" vvvv
		mat4 combined_transform;
		glm_mat4_mul(model.transform_matrix, model.meshes[i]->transform_matrix, combined_transform);
		// ! ^^^^ enable if apply transform to parent is set to "true" ^^^^

		// Pass the combined transformation matrix to the shader
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, (const GLfloat*)combined_transform);  // Use combined_transform here
		// ! ^^^^ change this to "combined_transform" if apply transform to parent is set to "true" ^^^^
		material_apply(model.meshes[i]->material, shader.id);

		// Draw the mesh with the combined transformation
		draw_manager_draw(&drawable, model.meshes[i]->name);
	}

	// Set the scale for the player model
	model_set_position(&player_model, (vec3){camera.position[0], camera.position[1] - 2.0f, camera.position[2]}); // Use camera position for the player's position
	model_set_rotation(&player_model, (vec4){ 0.0f, 0.0f, 0.0f, 1.0f });

	model_apply_transform(&player_model);

	// Draw each mesh with the updated transformation
	for (int i = 0; i < player_model.mesh_count; i++) {
		// Pass the combined transformation matrix to the shader
		glUniformMatrix4fv(model_loc, 1, GL_FALSE, (const GLfloat*)player_model.transform_matrix);  // Use combined_transform here

		// Draw the mesh with the combined transformation
		draw_manager_draw(&p_drawable, player_model.meshes[i]->name);
	}

	// ! DEBUG LIGHT CUBE
	shader_use(&DebugLightCube.shader_program);

	set_debug_cube_model_matrix(&DebugLightCube);
	set_debug_cube_view_matrix(&DebugLightCube, view);
	set_debug_cube_projection_matrix(&DebugLightCube, projection);

	draw_debug_cube(&DebugLightCube);
	
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

	sound_play_once_rtwp(&sound, (vec3){0.0f, 0.0f, 0.0f}, camera.position, camera.front, camera.worldUp);

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
	// * Setup the shaders of the scene
	setup_default_scene_shaders();

	// ! Test Multi-mesh Model
    if (!model_load_gltf(
		&model, 
		"resources/static/copyrighted/anime_girl_texture/agirl.gltf",
		true)
	) {
        fprintf(stderr, "Failed to load Agirl GLTF model!\n");
        return;
    }
    printf("[GLTF] Loaded anime_girl_texture/agirl.gltf model.\n");

	model_set_scale(&model, (vec3){ 0.5f, 0.5f, 0.5f });
	model_set_rotation(&model, (vec4){ -90.0f, 0.0f, 0.0f, 1.0f });
	model_apply_transform(&model);

	// Initialize the meshes as drawables
    for (int i = 0; i < model.mesh_count; i++)
        draw_manager_init_from_mesh(&drawable, model.meshes[i], model.meshes[i]->name);

	// ! Player Character Model
    if (!model_load_gltf(
		&player_model, 
		"resources/static/cylinder.glb",
		false)
	) {
        fprintf(stderr, "Failed to load Cylinder GLTF model!\n");
        return;
    }
    printf("[GLTF] Loaded static/cylinder.glb model.\n");

	model_set_scale(&player_model, (vec3){ 0.3f, 0.3f, 0.3f });
	model_set_rotation(&player_model, (vec4){ -90.0f, 0.0f, 0.0f, 1.0f });
	model_apply_transform(&player_model);

	// * Initialize the meshes as drawables
    for (int i = 0; i < player_model.mesh_count; i++)
        draw_manager_init_from_mesh(&p_drawable, player_model.meshes[i], player_model.meshes[i]->name);

	// * Make stbi flip the image vertically
	stbi_set_flip_vertically_on_load(1);

	// * Initialize Main Scene Camera
	camera_init(&camera, (vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 1.0f, 0.0f}, -90.0f, 0.0f);
    printf("Camera initialized.\n");

	// * Initialize the Crosshair
	crosshair_init(&crosshair, crosshairSize, crosshairThickness, crosshairColor);

	// * Initialize VCR_OSD_MONO Font
    font_init(&font, "resources/vcr_osd_mono.ttf", font_size, 3.0f, text_shader.id);  // Adjust path and size as needed

	// * Initialize Image
	image_init(&background_image, "resources/prototype/image.png", image_shader.id);
	image_set_dimensions(&background_image, 1024, 1024);
	printf("Initialized background_image\n");

	// * Initialize Button
	button_init(
		&my_button, "Hover me", 
		100.0f, 100.0f, 200.0f, 60.0f, 
		button_shader.id, BUTTON_TYPE_COLOR, 0, 
		(vec4){0.2f, 0.0f, 0.0f, 1.0f}, 
		&font, (vec3){1.0f, 1.0f, 1.0f}
	);

	// * Initialize Sound System;
	sound_initialize();
	alGenBuffers(1, &sound.buffer);
	alGenSources(1, &sound.source);

	ALsizei size, freq;
	ALenum format;
	ALvoid* data;
	load_wav("resources/audio/copyrighted/crystal.wav", &format, &data, &size, &freq); // Implement your loader
	alBufferData(sound.buffer, format, data, size, freq);
	free(data);

	// Attach the buffer to the source
	sound_attach_buffer(&sound);

	// * Initialize sound properties
	sound_set_volume(&sound, 0.0f);

	// ! Debug light cube
	// * Create the cube shaders
	create_debug_cube_shaders(
		&DebugLightCube, 
		"resources/shaders/debug/vertex.glsl", 
		"resources/shaders/debug/fragment.glsl"
	);

	vec3 c_size = {0.3f, 0.3f, 0.3f};

	glm_vec3_copy((vec3){ 0.0f, 12.0f, 6.0f }, LightPosition);
	vec4 c_color = {1.0f, 1.0f, 1.0f, 1.0f};

	create_debug_cube(&DebugLightCube, c_size, LightPosition, c_color);

	// ! Light
	create_light(&PointLight, shader.id);

	// ! Skybox
	const char* faces[6] = {
		"resources/static/cubemap/px.png", // Positive X
		"resources/static/cubemap/nx.png", // Negative X
		"resources/static/cubemap/py.png", // Positive Y
		"resources/static/cubemap/ny.png", // Negative Y
		"resources/static/cubemap/pz.png", // Positive Z
		"resources/static/cubemap/nz.png"  // Negative Z
	};

	// * Stop stbi from flipping the image vertically
	stbi_set_flip_vertically_on_load(0);
	skybox_init(&skybox, faces, skybox_shader.id);
}

void default_scene_cleanup() {
	// ** Clean up resources **

	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// * Destroy sound objects
	sound_cleanup();

	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// * Destroy UI Components
	button_cleanup(&my_button);
	image_cleanup(&background_image);
	font_cleanup(&font);
	crosshair_destroy(&crosshair);

	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// * Destroy drawables
	model_free(&model);
	model_free(&player_model);

	draw_manager_destroy(&drawable);
	draw_manager_destroy(&p_drawable);

	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// * Destroy Skybox
	skybox_destroy(&skybox);

	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// * Destroy Shaders
    shader_destroy(&shader);
	
    shader_destroy(&image_shader);
    shader_destroy(&text_shader);
    shader_destroy(&button_shader);
    shader_destroy(&skybox_shader);

	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>

	// * Destroy Buffers
    buffers_destroy(&buffers);

	// & >>>>>>>>>>>>>>>>>>>>>>>>>>>>

    printf("Resources cleaned up.\n");
}
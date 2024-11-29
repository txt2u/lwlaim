#include <entities/static/cube.h>

#include <pipeline/shader.h>
#include <pipeline/buffers.h>

#include <cglm/cglm.h>
#include <stdbool.h>

#include <qreader.h>

void create_debug_cube_shaders(Cube* cube, const char* vertex_shader, const char* fragment_shader) {
	// Compile shaders and create shader program
    char* vertexShaderSource = read_file(vertex_shader);
    char* fragmentShaderSource = read_file(fragment_shader);
    if (!vertexShaderSource || !fragmentShaderSource) {
        fprintf(stderr, "Failed to load debug shader sources!\n");
        return;
    }

	ShaderProgram shader;

    shader = shader_create(vertexShaderSource, fragmentShaderSource);
    free(vertexShaderSource);
    free(fragmentShaderSource);

    if (shader.id == 0) {
        fprintf(stderr, "Debug shader program creation failed!\n");
        return;
    }
    printf("Debug shader program created.\n");

	cube->shader_program = shader;
}

void create_debug_cube(Cube* cube, vec3 size, vec3 position, vec4 color) {
	Buffers buffers = buffers_create_empty();
	buffers_bind_vao(buffers.VAO);

	// * Cube data
	float vertices[] = {
		// Front face
		-1.0f, -1.0f,  1.0f,  // 0
		1.0f, -1.0f,  1.0f,  // 1
		1.0f,  1.0f,  1.0f,  // 2
		-1.0f,  1.0f,  1.0f,  // 3

		// Back face
		-1.0f, -1.0f, -1.0f,  // 4
		1.0f, -1.0f, -1.0f,  // 5
		1.0f,  1.0f, -1.0f,  // 6
		-1.0f,  1.0f, -1.0f,  // 7

		// Top face
		-1.0f,  1.0f,  1.0f,  // 3
		1.0f,  1.0f,  1.0f,  // 2
		1.0f,  1.0f, -1.0f,  // 6
		-1.0f,  1.0f, -1.0f,  // 7

		// Bottom face
		-1.0f, -1.0f,  1.0f,  // 0
		1.0f, -1.0f,  1.0f,  // 1
		1.0f, -1.0f, -1.0f,  // 5
		-1.0f, -1.0f, -1.0f,  // 4

		// Right face
		1.0f, -1.0f,  1.0f,  // 1
		1.0f,  1.0f,  1.0f,  // 2
		1.0f,  1.0f, -1.0f,  // 6
		1.0f, -1.0f, -1.0f,  // 5

		// Left face
		-1.0f, -1.0f,  1.0f,  // 0
		-1.0f,  1.0f,  1.0f,  // 3
		-1.0f,  1.0f, -1.0f,  // 7
		-1.0f, -1.0f, -1.0f   // 4
	};
	unsigned int indices[] = {
		// Front face
		0, 1, 2,
		0, 2, 3,

		// Back face
		4, 5, 6,
		4, 6, 7,

		// Top face
		3, 2, 6,
		3, 6, 7,

		// Bottom face
		0, 1, 5,
		0, 5, 4,

		// Right face
		1, 2, 6,
		1, 6, 5,

		// Left face
		0, 3, 7,
		0, 7, 4
	};

	// ! I'm not sure if we need the vbo
	buffers.VBO = buffers_create_vbo(vertices, sizeof(vertices));
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
	
	buffers.EBO = buffers_create_ebo(indices, sizeof(indices));
	cube->indices = indices;
	cube->index_count = sizeof(indices) / sizeof(indices[0]);

	glm_vec3_copy(size, cube->size);
	glm_vec3_copy(position, cube->position);
	glm_vec4_copy(color, cube->color);

	cube->buffers = buffers;
}

void draw_debug_cube(Cube* cube) {
	// Get the cubeColor uniform's location
	GLuint color_loc = glGetUniformLocation(cube->shader_program.id, "cubeColor");
	if (color_loc == -1) {
		fprintf(stderr, "Uniform 'cubeColor' not found or optimized out in shader.\n");
		return;
	}

	// Set the vec4 uniform of the cube color
	glUniform4fv(color_loc, 1, (const GLfloat*)cube->color);

	buffers_bind_vao(cube->buffers.VAO);
	buffers_bind_ebo(cube->buffers.EBO);

	glDrawElements(GL_TRIANGLES, cube->index_count, GL_UNSIGNED_INT, 0);

	buffers_unbind_ebo();
	buffers_unbind_vao();
}

void set_debug_cube_model_matrix(Cube* cube) {
    // Get the location of the "model" uniform in the shader
    GLuint model_loc = glGetUniformLocation(cube->shader_program.id, "model");
    if (model_loc == -1) {
        fprintf(stderr, "Failed to find 'model' uniform in shader.\n");
        return;
    }

    // Create the transformation matrix (identity, scale, translate)
    mat4 transform;
    glm_mat4_identity(transform);

    // Apply scaling
    glm_scale(transform, cube->size);

    // Apply translation
    glm_translate(transform, cube->position);

    // Pass the model matrix to the shader
    glUniformMatrix4fv(model_loc, 1, GL_FALSE, (const GLfloat*)transform);
}

void set_debug_cube_projection_matrix(Cube* cube, mat4 projection) {
	GLuint projection_loc = glGetUniformLocation(cube->shader_program.id, "projection");
	glUniformMatrix4fv(projection_loc, 1, GL_FALSE, (const GLfloat*)projection);
}

void set_debug_cube_view_matrix(Cube* cube, mat4 view) {
	GLuint view_loc = glGetUniformLocation(cube->shader_program.id, "view");
	glUniformMatrix4fv(view_loc, 1, GL_FALSE, (const GLfloat*)view);
}
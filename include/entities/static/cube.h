#include <pipeline/shader.h>
#include <pipeline/buffers.h>

#include <cglm/cglm.h>
#include <stdbool.h>

typedef struct {
	vec3 size;
	vec3 position;
	vec4 color;

	unsigned int* indices;
	int index_count;

	Buffers buffers;

    ShaderProgram shader_program;
} Cube;

void create_debug_cube_shaders(Cube* cube, const char* vertex_shader, const char* fragment_shader);
void create_debug_cube(Cube* cube, vec3 size, vec3 position, vec4 color);

void set_debug_cube_model_matrix(Cube* cube);
void set_debug_cube_projection_matrix(Cube* cube, mat4 projection);
void set_debug_cube_view_matrix(Cube* cube, mat4 view);

void draw_debug_cube(Cube* cube);
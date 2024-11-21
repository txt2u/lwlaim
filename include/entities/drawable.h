#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <cglm/cglm.h>
#include <pipeline/buffers.h>
#include <entities/ecs.h>
#include <entities/mesh.h>
#include <entities/model.h>

typedef struct {
    mat4 model_matrix;
    Buffers buffers;
	Mesh *mesh;
} Drawable;

// Initializes a drawable object from a mesh
void draw_manager_init_from_mesh(Drawable* p_drawable, Mesh* mesh);

// Draws a drawable object
void draw_manager_draw(Drawable* drawable);

// Applies translation to the drawable object
void draw_manager_translate(Drawable* drawable, vec3 translation);

// Applies scaling to the drawable object
void draw_manager_scale(Drawable* drawable, vec3 scale);

// Applies rotation to the drawable object
void draw_manager_rotate(Drawable* drawable, float angle, vec3 axis);

// Cleans up drawable buffers
void draw_manager_destroy(Drawable* drawable);

#endif // DRAWABLE_H

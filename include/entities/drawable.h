#ifndef DRAWABLE_H
#define DRAWABLE_H

#include <cglm/cglm.h>
#include <pipeline/buffers.h>
#include <entities/ecs.h>
#include <entities/mesh.h>
#include <entities/model.h>

#define MAX_MESHES 24  // Limit number of meshes per drawable

typedef struct {
    Buffers buffers;
    Mesh *mesh;
    const char *name;  // Name of the mesh
} DrawableMesh;

typedef struct {
    DrawableMesh meshes[MAX_MESHES];  // Array of meshes (a drawable can hold multiple meshes)
    int mesh_count;                   // Number of meshes in the drawable
    mat4 model_matrix;
    vec3 translation;
    vec3 scale;
    vec3 rotation;
} Drawable;

// Initializes a drawable object from a mesh
void draw_manager_init_from_mesh(Drawable* p_drawable, Mesh* mesh, const char* name);

// Draws a drawable object (with a specific mesh)
void draw_manager_draw(Drawable* drawable, const char* mesh_name);

// Applies translation to the drawable object
void draw_manager_translate(Drawable* drawable, vec3 translation);

// Applies scaling to the drawable object
void draw_manager_scale(Drawable* drawable, vec3 scale);

// Applies rotation to the drawable object
void draw_manager_rotate(Drawable* drawable, vec3 angle);

// Cleans up drawable buffers
void draw_manager_destroy(Drawable* drawable);

#endif // DRAWABLE_H

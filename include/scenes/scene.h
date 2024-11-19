#ifndef SCENE_H
#define SCENE_H

#include <GLFW/glfw3.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Define a simple key-value pair for the scene state
typedef struct KeyValuePair {
    char *key;    // The key (name of the variable)
    char *value;  // The value (stored as a string for simplicity)
} KeyValuePair;

// Define the scene state structure
typedef struct SceneState {
    KeyValuePair *data;  // Array of key-value pairs
    size_t count;        // Number of key-value pairs
} SceneState;

// Define the Scene structure
typedef struct Scene {
    char* name;
    GLFWwindow* window;
    SceneState state;

    void (*update)(struct Scene* self);
    void (*render)(struct Scene* self);
    void (*cleanup)(struct Scene* self);
} Scene;

// Scene lifecycle functions
Scene *scene_create(const char *name, GLFWwindow *window);
void scene_destroy(Scene *scene);

// State management functions
bool scene_state_set(SceneState *state, const char *key, const char *value);
const char *scene_state_get(const SceneState *state, const char *key);

// Scene management functions
void scene_update(Scene *scene);
void scene_render(Scene *scene);
void scene_cleanup(Scene *scene);

#endif // SCENE_H
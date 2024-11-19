#include <scenes/scene.h>

// Create a new scene
Scene *scene_create(const char *name, GLFWwindow *window) {
    Scene *scene = (Scene *)malloc(sizeof(Scene));
    if (!scene) return NULL;

    scene->name = strdup(name);
    scene->window = window;
    scene->state.data = NULL;
    scene->state.count = 0;
    scene->update = NULL;
    scene->render = NULL;
    scene->cleanup = NULL;

    return scene;
}

// Destroy a scene
void scene_destroy(Scene *scene) {
    if (!scene) return;

    free(scene->name);

    // Free state data
    for (size_t i = 0; i < scene->state.count; i++) {
        free(scene->state.data[i].key);
        free(scene->state.data[i].value);
    }
    free(scene->state.data);

    free(scene);
}

// Set a key-value pair in the scene state
bool scene_state_set(SceneState *state, const char *key, const char *value) {
    if (!state || !key || !value) return false;

    // Check if key already exists
    for (size_t i = 0; i < state->count; i++) {
        if (strcmp(state->data[i].key, key) == 0) {
            free(state->data[i].value);
            state->data[i].value = strdup(value);
            return true;
        }
    }

    // Add a new key-value pair
    KeyValuePair *new_data = (KeyValuePair *)realloc(state->data, (state->count + 1) * sizeof(KeyValuePair));
    if (!new_data) return false;

    state->data = new_data;
    state->data[state->count].key = strdup(key);
    state->data[state->count].value = strdup(value);
    state->count++;
    return true;
}

// Get a value from the scene state by key
const char *scene_state_get(const SceneState *state, const char *key) {
    if (!state || !key) return NULL;

    for (size_t i = 0; i < state->count; i++) {
        if (strcmp(state->data[i].key, key) == 0) {
            return state->data[i].value;
        }
    }
    return NULL;
}

// Call the update function of the scene
void scene_update(Scene *scene) {
    if (scene && scene->update) {
        scene->update(scene);
    }
}

// Call the render function of the scene
void scene_render(Scene* scene) {
    if (scene && scene->render) {
        scene->render(scene);
    }
}

// Call the cleanup function of the scene
void scene_cleanup(Scene *scene) {
    if (scene && scene->cleanup) {
        scene->cleanup(scene);
    }
}
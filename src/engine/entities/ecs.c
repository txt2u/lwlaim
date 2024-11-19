#include <entites/ecs.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Initialize the ECS
void ecs_init(ECS* ecs) {
    ecs->entity_count = 0;
    for (size_t i = 0; i < MAX_COMPONENTS; ++i) {
        ecs->components[i].data = NULL;
        ecs->components[i].size = 0;
        ecs->components[i].count = 0;
        ecs->components[i].capacity = 0;
    }
    memset(ecs->entity_component_mask, 0, sizeof(ecs->entity_component_mask));
}

// Create a new entity
Entity ecs_create_entity(ECS* ecs) {
    if (ecs->entity_count >= MAX_ENTITIES) {
        fprintf(stderr, "Error: Maximum number of entities reached!\n");
        return UINT32_MAX;
    }
    Entity entity = ecs->entity_count++;
    ecs->entities[entity] = entity;
    return entity;
}

// Destroy an entity
void ecs_destroy_entity(ECS* ecs, Entity entity) {
    if (entity >= ecs->entity_count) {
        fprintf(stderr, "Error: Invalid entity ID!\n");
        return;
    }
    for (size_t i = 0; i < MAX_COMPONENTS; ++i) {
        if (ecs->entity_component_mask[entity][i]) {
            ecs_remove_component(ecs, entity, i);
        }
    }
    ecs->entities[entity] = UINT32_MAX; // Mark entity as destroyed
}

// Register a component type
size_t ecs_register_component(ECS* ecs, size_t component_size) {
    for (size_t i = 0; i < MAX_COMPONENTS; ++i) {
        if (ecs->components[i].data == NULL) {
            ecs->components[i].data = malloc(component_size * 8); // Initial capacity of 8
            ecs->components[i].size = component_size;
            ecs->components[i].count = 0;
            ecs->components[i].capacity = 8;
            return i;
        }
    }
    fprintf(stderr, "Error: Maximum number of components reached!\n");
    return UINT32_MAX;
}

// Add a component to an entity
void* ecs_add_component(ECS* ecs, Entity entity, size_t component_id) {
    if (component_id >= MAX_COMPONENTS || ecs->components[component_id].data == NULL) {
        fprintf(stderr, "Error: Invalid component ID!\n");
        return NULL;
    }
    if (ecs->entity_component_mask[entity][component_id]) {
        fprintf(stderr, "Error: Entity already has this component!\n");
        return NULL;
    }

    ComponentArray* array = &ecs->components[component_id];
    if (array->count >= array->capacity) {
        array->capacity *= 2;
        array->data = realloc(array->data, array->size * array->capacity);
    }
    void* component = (char*)array->data + (array->count * array->size);
    memset(component, 0, array->size); // Initialize to zero
    array->count++;
    ecs->entity_component_mask[entity][component_id] = true;
    return component;
}

// Get a component from an entity
void* ecs_get_component(ECS* ecs, Entity entity, size_t component_id) {
    if (component_id >= MAX_COMPONENTS || ecs->components[component_id].data == NULL) {
        fprintf(stderr, "Error: Invalid component ID!\n");
        return NULL;
    }
    if (!ecs->entity_component_mask[entity][component_id]) {
        return NULL; // Entity does not have this component
    }
    ComponentArray* array = &ecs->components[component_id];
    return (char*)array->data + (entity * array->size);
}

// Remove a component from an entity
void ecs_remove_component(ECS* ecs, Entity entity, size_t component_id) {
    if (component_id >= MAX_COMPONENTS || ecs->components[component_id].data == NULL) {
        fprintf(stderr, "Error: Invalid component ID!\n");
        return;
    }
    if (!ecs->entity_component_mask[entity][component_id]) {
        fprintf(stderr, "Error: Entity does not have this component!\n");
        return;
    }
    ecs->entity_component_mask[entity][component_id] = false;
}

// Apply a system to all entities with a specific component
void ecs_for_each(ECS* ecs, size_t component_id, void (*callback)(Entity, void*)) {
    if (component_id >= MAX_COMPONENTS || ecs->components[component_id].data == NULL) {
        fprintf(stderr, "Error: Invalid component ID!\n");
        return;
    }
    ComponentArray* array = &ecs->components[component_id];
    for (size_t i = 0; i < ecs->entity_count; ++i) {
        if (ecs->entity_component_mask[i][component_id]) {
            void* component = (char*)array->data + (i * array->size);
            callback(i, component);
        }
    }
}
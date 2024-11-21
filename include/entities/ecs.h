#ifndef ECS_H
#define ECS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// Max limits
#define MAX_ENTITIES 1000
#define MAX_COMPONENTS 32

// Entity type (just an ID)
typedef uint32_t Entity;

// Component storage structure
typedef struct {
    void* data;
    size_t size;
    size_t count;
    size_t capacity;
} ComponentArray;

// ECS Manager
typedef struct {
    Entity entities[MAX_ENTITIES];
    size_t entity_count;

    ComponentArray components[MAX_COMPONENTS];
    bool entity_component_mask[MAX_ENTITIES][MAX_COMPONENTS];
} ECS;

// Initialize ECS
void ecs_init(ECS* ecs);

// Entity management
Entity ecs_create_entity(ECS* ecs);
void ecs_destroy_entity(ECS* ecs, Entity entity);

// Component management
size_t ecs_register_component(ECS* ecs, size_t component_size);
void* ecs_add_component(ECS* ecs, Entity entity, size_t component_id);
void* ecs_get_component(ECS* ecs, Entity entity, size_t component_id);
void ecs_remove_component(ECS* ecs, Entity entity, size_t component_id);

// System management
void ecs_for_each(ECS* ecs, size_t component_id, void (*callback)(Entity, void*));

#endif // ECS_H
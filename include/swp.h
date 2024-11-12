#pragma once
#include <cglm/cglm.h>

typedef struct {
    vec3 origin;
    vec3 direction;
} Ray;

void screen_to_ray(int screenX, int screenY, int screenWidth, int screenHeight, mat4 viewMatrix, mat4 projMatrix, Ray *ray);
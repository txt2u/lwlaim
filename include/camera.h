#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>

typedef struct {
    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;
    vec3 worldUp;
    float yaw;
    float pitch;
    float movementSpeed;
    float mouseSensitivity;
    float fov;
} Camera;

void camera_init(Camera* camera, vec3 position, vec3 up, float yaw, float pitch);
void camera_update(Camera* camera);
void camera_process_keyboard(Camera* camera, GLFWwindow* window, float deltaTime);
void camera_process_mouse(Camera* camera, double xpos, double ypos);
void camera_get_view_matrix(Camera* camera, mat4 view);
void camera_get_projection_matrix(Camera* camera, mat4 projection, int width, int height);

#endif
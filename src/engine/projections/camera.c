#include <projections/camera.h>

void camera_init(Camera* camera, vec3 position, vec3 up, float yaw, float pitch) {
    glm_vec3_copy(position, camera->position);
    glm_vec3_copy(up, camera->worldUp);
    camera->yaw = yaw;
    camera->pitch = pitch;
    glm_vec3_copy((vec3){0.0f, 0.0f, -1.0f}, camera->front);
    glm_cross(camera->front, camera->worldUp, camera->right);
    glm_cross(camera->right, camera->front, camera->up);
    camera->movementSpeed = 2.5f;
    camera->mouseSensitivity = 0.1f;
    camera->fov = 80.0f;

	camera->near = 0.01f;
	camera->far = 800.0f;
}

void set_camera_near(Camera* camera, float near) {
	camera->near = near;
}

void set_camera_far(Camera* camera, float far) {
	camera->far = far;
}

void camera_update(Camera* camera) {
    glm_cross(camera->front, camera->worldUp, camera->right);
    glm_cross(camera->right, camera->front, camera->up);
}

void get_camera_directions(Camera* camera, float* camera_x, float* camera_y, float* camera_z) {
    *camera_x = camera->right[0];  // X component of the right vector
    *camera_y = camera->up[1];     // Y component of the up vector
    *camera_z = camera->front[2];  // Z component of the front vector
}

void camera_process_keyboard(Camera* camera, GLFWwindow* window, float deltaTime) {
    // Calculate the velocity based on movement speed and deltaTime
    float velocity = camera->movementSpeed * deltaTime;

    // Move the camera based on key presses
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        vec3 movement;
        glm_vec3_scale(camera->front, velocity, movement); // Scale the front vector by velocity
        glm_vec3_add(camera->position, movement, camera->position); // Add to camera position
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        vec3 movement;
        glm_vec3_scale(camera->front, velocity, movement); // Scale the front vector by velocity
        glm_vec3_sub(camera->position, movement, camera->position); // Subtract from camera position
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vec3 movement;
        glm_vec3_scale(camera->right, velocity, movement); // Scale the right vector by velocity
        glm_vec3_sub(camera->position, movement, camera->position); // Subtract from camera position
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        vec3 movement;
        glm_vec3_scale(camera->right, velocity, movement); // Scale the right vector by velocity
        glm_vec3_add(camera->position, movement, camera->position); // Add to camera position
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        vec3 movement;
        glm_vec3_scale(camera->up, velocity, movement); // Scale the up vector by velocity
        glm_vec3_add(camera->position, movement, camera->position); // Add to camera position
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        vec3 movement;
        glm_vec3_scale(camera->up, velocity, movement); // Scale the up vector by velocity
        glm_vec3_sub(camera->position, movement, camera->position); // Subtract from camera position
    }
}

void camera_process_mouse(Camera* camera, double xpos, double ypos) {
    static float lastX = 0.0f, lastY = 0.0f;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xOffset *= camera->mouseSensitivity;
    yOffset *= camera->mouseSensitivity;

    camera->yaw += xOffset;
    camera->pitch += yOffset;

    if (camera->pitch > 89.0f)
        camera->pitch = 89.0f;
    if (camera->pitch < -89.0f)
        camera->pitch = -89.0f;

    vec3 front = {0.0f, 0.0f, 0.0f};
    front[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    front[1] = sin(glm_rad(camera->pitch));
    front[2] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    glm_vec3_copy(front, camera->front);
}

void camera_get_view_matrix(Camera* camera, mat4 view) {
    vec3 target;
    glm_vec3_add(camera->position, camera->front, target);  // Add the position and front vectors
    glm_lookat(camera->position, target, camera->up, view); // Pass the target vector to glm_lookat
}

void camera_get_projection_matrix(Camera* camera, mat4 projection, int width, int height) {
    glm_perspective(glm_rad(camera->fov), (float)width / (float)height, camera->near, camera->far, projection);
}
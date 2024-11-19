#include <output/raycast.h>
#include <cglm/cglm.h>

void screen_to_ray(int screenX, int screenY, int screenWidth, int screenHeight, mat4 viewMatrix, mat4 projMatrix, Ray *ray) {
    // Convert screen coordinates to normalized device coordinates
    float ndcX = (2.0f * screenX) / screenWidth - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY) / screenHeight; // Invert Y-axis for OpenGL
    float ndcZ = 1.0f; // Far plane in NDC space (for farthest visible point)

    // Create a vector in NDC space for the far plane (ray direction)
    vec4 clipSpacePos = { ndcX, ndcY, ndcZ, 1.0f };

    // Calculate the inverse of the combined projection and view matrix
    mat4 invProjView;
    glm_mat4_mul(projMatrix, viewMatrix, invProjView); // proj * view
    glm_mat4_inv(invProjView, invProjView);            // Invert the result

    // Transform the clip space position into world space (this gives us a point in the world space)
    vec4 worldSpacePos;
    glm_mat4_mulv(invProjView, clipSpacePos, worldSpacePos);

    // Perspective divide to convert from homogeneous coordinates (clip space to world space)
    worldSpacePos[0] /= worldSpacePos[3];
    worldSpacePos[1] /= worldSpacePos[3];
    worldSpacePos[2] /= worldSpacePos[3];

    // Set the ray origin (camera position in world space) and direction
    // Assuming the camera is at the origin or you have access to its position (for example, viewMatrix[3] gives camera position)
    glm_vec3_copy(viewMatrix[3], ray->origin); // Camera position is in the last row of the view matrix
    glm_vec3_copy((vec3){worldSpacePos[0], worldSpacePos[1], worldSpacePos[2]}, ray->direction);
    
    // Normalize the direction vector to get a direction
    glm_vec3_normalize(ray->direction);
}

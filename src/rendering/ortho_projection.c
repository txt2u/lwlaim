#include "ortho_projection.h"

void setup_ortho_projection(int screenWidth, int screenHeight, mat4 projection) {
    glm_ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f, projection);
}
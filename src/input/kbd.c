#include <GLFW/glfw3.h>

#include "kbd.h"

int wireframeMode = 0;  // Variable to remember wireframe mode state
int cullingMode = 0;    // Variable to remember back-face mode state
int cursorLocked = 0;   // Variable to remember cursor state (locked or not)

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    // Toggle cursor visibility and locking on F1 key press
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        if (cursorLocked) {
            // Unlock and show the cursor
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            // Lock and hide the cursor
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        // Toggle the cursor state
        cursorLocked = !cursorLocked;
    }

    // Toggle wireframe mode on F3 key press
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
        if (wireframeMode) {
            // Disable wireframe mode (render in solid mode)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            // Enable wireframe mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        // Toggle the wireframe mode state
        wireframeMode = !wireframeMode;
    }

	// Toggle culling mode on F4 key press
    if (key == GLFW_KEY_F4 && action == GLFW_PRESS) {
        if (cullingMode) {
			glEnable(GL_CULL_FACE);  // Enable back-face culling
        } else {
			glDisable(GL_CULL_FACE);  // Disable back-face culling
        }

        // Toggle the wireframe mode state
        cullingMode = !cullingMode;
    }


}
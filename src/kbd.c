#include <GLFW/glfw3.h>

#include "kbd.h"

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}

	// Toggle cursor visibility and locking on F1 key press
	if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
		static int cursorLocked = 1;  // Static variable to remember cursor state (locked or not)

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
}

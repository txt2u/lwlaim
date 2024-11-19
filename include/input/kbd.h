#pragma once

#include <GLFW/glfw3.h>

extern int wireframeMode;  // Static variable to remember wireframe mode state
extern int cullingMode;  // Static variable to remember back-face mode state
extern int cursorLocked;  // Static variable to remember cursor state (locked or not)

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
#include "mue.h"

#include <GLFW/glfw3.h>
#include <stdlib.h>

// Define the global variables here
double cursor_x_position = 0.0; // Initial value
double cursor_y_position = 0.0; // Initial value

void cursor_callback(GLFWwindow* window, double x_pos, double y_pos) {
    cursor_x_position = x_pos;  // Update global variable
    cursor_y_position = y_pos;  // Update global variable
}
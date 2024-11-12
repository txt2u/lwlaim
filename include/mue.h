#pragma once

#include <GLFW/glfw3.h>
#include <stdlib.h>

extern double cursor_x_position;
extern double cursor_y_position;
void cursor_callback(GLFWwindow* window, double x_pos, double y_pos);
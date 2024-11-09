#include <GL/glew.h>

#define GLFW_EXPOSE_NATIVE_WGL
#ifdef _WIN64
	#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <stdio.h>
#include <stdlib.h>

#include "kbd.h"

int main() {
	// Initialize glfw, if didn't, fprintf and return an error code of -1
	if(!glfwInit()) {
		fprintf(stderr, "Failed to initialize glfw!\n");
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, FALSE);

	GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
	if (!primary_monitor) {
        fprintf(stderr, "Failed to get primary monitor!\n");
        glfwTerminate();
        return -1;
    }

    // Get the video mode of the primary monitor
    const GLFWvidmode* video_mode = glfwGetVideoMode(primary_monitor);
    if (!video_mode) {
        fprintf(stderr, "Failed to get video mode!\n");
        glfwTerminate();
        return -1;
    }

    // Access screen width and height
    int screen_w = video_mode->width;
    int screen_h = video_mode->height;
    
	// Create a fullscreen-borderless window
	GLFWwindow* window = glfwCreateWindow(screen_w, screen_h, "lwlaim", NULL, NULL);
	
	if(!window) {
		fprintf(stderr, "Failed to create window!\n");
		glfwTerminate();
		return -3;
	}

	// Create the opengl context for the window
	glfwMakeContextCurrent(window);

	glewExperimental = GLU_TRUE;
	if(glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize glew!\n");
		return -2;
	}

	// Keyboard callback function is in kbd.c
	glfwSetKeyCallback(window, keyboard_callback);

	// While the app is running
	while(!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Make the background color green?
		glClearColor(0.5, 0.8, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glfwSwapBuffers(window);
	}

	// Cleanup
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
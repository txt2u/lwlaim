#include <GL/glew.h>

#define GLFW_EXPOSE_NATIVE_WGL
#ifdef _WIN64
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <stdio.h>
#include <stdlib.h>

#include "kbd.h"
#include "mue.h"

#include "scene.h"
#include <scenes/default.h>
#include <scenes/splash.h>

int main() {
    // Initialize glfw
    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialize glfw!\n");
        return -1;
    }
    printf("GLFW initialized.\n");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, FALSE);
	glfwWindowHint(GLFW_SAMPLES, 16);

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
    printf("Window was created successfully.\n");

    // Create the OpenGL context for the window
    glfwMakeContextCurrent(window);

	const char* version = (const char*)glGetString(GL_VERSION);
	printf("OpenGL version: %s\n", version);

    glewExperimental = GLU_TRUE;
    if(glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize glew!\n");
        return -2;
    }
    printf("GLEW initialized.\n");

    // Keyboard and mouse callback functions
    glfwSetKeyCallback(window, keyboard_callback);
    glfwSetCursorPosCallback(window, cursor_callback);
  
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);  // Enable back-face culling
    glCullFace(GL_BACK);     // Cull the back faces (if not front-facing)
    glFrontFace(GL_CCW);     // Set front face counter-clockwise (default)

    glfwSwapInterval(0);

	Scene *main_scene = scene_create("main#0", window);
    scene_state_set(&main_scene->state, "player_health", "100");
    main_scene->update = default_scene_update;
    main_scene->render = default_scene_render;
    main_scene->cleanup = default_scene_cleanup;

	Scene *splash_screen = scene_create("splash#0", window);
    scene_state_set(&splash_screen->state, "loaded", "0");
    splash_screen->update = splash_scene_update;
    splash_screen->render = splash_scene_render;
    splash_screen->cleanup = splash_scene_cleanup;

	splash_screen->render(splash_screen);
	main_scene->render(main_scene);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Check the state of the splash screen
		const char *state_value = scene_state_get(&splash_screen->state, "loaded");

		if (state_value != NULL && strcmp(state_value, "1") == 0) {
			// If the state is "1", switch to the main scene
			main_scene->update(main_scene);
		} else {
			// Otherwise, keep showing the splash screen
			splash_screen->update(splash_screen);
		}

		// Swap buffers to display the updated scene
		glfwSwapBuffers(window);
	}

	main_scene->cleanup(main_scene);
	splash_screen->cleanup(splash_screen);

    // Close window and terminate
    glfwDestroyWindow(window);
    glfwTerminate();
    printf("GLFW terminated.\n");

    return 0;
}
#include <lighting/light.h>
#include <cglm/cglm.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <pipeline/shader.h>
#include <pipeline/buffers.h>

void create_light(Light* light, int shader_program) {
	light->shader_program = shader_program;
	printf("New shader_program id: %i\n", light->shader_program);
}

void create_point_light(Light* light, vec3 position, vec3 color, float intensity) {
	glm_vec3_copy(position, light->position);
	glm_vec3_copy(color, light->color);
    light->intensity = intensity;

    // Assuming you're passing the point light to a shader, you can set uniforms like so:
    GLuint position_loc = glGetUniformLocation(light->shader_program, "light.position");
    GLuint color_loc = glGetUniformLocation(light->shader_program, "light.color");
    // GLuint intensity_loc = glGetUniformLocation(light->shader_program, "light.intensity");

    // Check if the uniforms were found in the shader
    if (position_loc == -1 || color_loc == -1) {
        fprintf(stderr, "Failed to find one or more light uniforms in the shader.\n");
        return;
    }

    // Set the light properties in the shader (these are sent as uniforms)
    glUniform3fv(position_loc, 1, light->position);  // Light position
    glUniform3fv(color_loc, 1, light->color);        // Light color
    // glUniform1f(intensity_loc, light->intensity);    // Light intensity

    // You can also handle attenuation factors here if you want to implement distance-based light decay.
    // Point lights often have attenuation factors like `constant`, `linear`, and `quadratic`.
}

// void create_directional_light(Light* light, vec3 direction, vec3 color, float intensity);
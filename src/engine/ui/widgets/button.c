#include <pipeline/buffers.h>
#include <ui/widgets/button.h>
#include <ui/text.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <projections/ortho.h>
#include <input/kbd.h>

void button_init(Button *button, const char *text, float x, float y, float width, float height, GLuint shader_program, ButtonType type, GLuint texture_id, vec4 bg_color, Font *font, vec3 text_color) {
    button->x = x;
    button->y = y;
    button->width = width;
    button->height = height;

    button->shader_program = shader_program;
    button->type = type;
    button->texture_id = texture_id;

    glm_vec2_copy((vec2){ 1.0f, 1.0f }, button->scale);
    glm_vec4_copy(bg_color, button->bg_color);
    glm_vec3_copy(text_color, button->text_color);

    button->text = text;
    button->font = *font;

    // Calculate text position for alignment
    float text_width, text_height;
    font_get_text_dimensions(font, text, &text_width, &text_height);

    if (type == BUTTON_TYPE_COLOR) {
        // If it's a color background, we don't need a texture
        button->texture_id = 0;
    }

    // Default text alignment (centered)
    button->text_offset_x = (width - text_width) / 2.0f;
    button->text_offset_y = (height - text_height) / 2.0f;

    // Set up button geometry (quad) with position, color, and texture coordinates
    GLfloat vertices[] = {
        // Positions
        0.0f, 0.0f, 0.0f,
        width, 0.0f, 0.0f,
        0.0f, height, 0.0f,
        width, height, 0.0f,
    };

    // Texture coordinates
    GLfloat texcoords[] = {
        0.0f, 0.0f,  // bottom-left
        1.0f, 0.0f,  // bottom-right
        0.0f, 1.0f,  // top-left
        1.0f, 1.0f   // top-right
    };

    unsigned int indices[] = {
        0, 1, 2,  // First triangle
        1, 3, 2   // Second triangle
    };

    // Initialize buffers using the buffers API
    button->buffers = buffers_create_empty();

    // Bind VAO to start setting up buffers
    buffers_bind_vao(button->buffers.VAO);

    // Fill VBO with vertex data (positions)
    button->buffers.VBO = buffers_create_vbo(vertices, (sizeof(vertices) * 3) / sizeof(GLfloat));
    buffers_bind_vbo(button->buffers.VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0); // Position
    glEnableVertexAttribArray(0);

    // Fill VBO with texture coordinate data
    button->buffers.TexCoordVBO = buffers_create_vbo(texcoords, sizeof(texcoords) / sizeof(GLfloat));
    buffers_bind_vbo(button->buffers.TexCoordVBO);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0); // Texture coordinates
    glEnableVertexAttribArray(1);

    // Create and fill EBO with index data
    button->buffers.EBO = buffers_create_ebo(indices, sizeof(indices) / sizeof(unsigned int));
    buffers_bind_ebo(button->buffers.EBO);

    // Unbind all buffers
    buffers_unbind_vao();
    buffers_unbind_vbo();
    buffers_unbind_ebo();
}

// Render the button
void button_render(Button *button, float x, float y, int framebufferWidth, int framebufferHeight) {
    glUseProgram(button->shader_program);  // Use the shader program

    if (button->buffers.VAO == 0 || button->buffers.VBO == 0 || button->buffers.EBO == 0) {
        printf("Buffer not properly initialized!\n");
        return;  // Handle buffer initialization error
    }

    buffers_bind_vao(button->buffers.VAO);
    buffers_bind_vbo(button->buffers.VBO);

	// Set model matrix (button's position, scaling, etc.)
	mat4 model;
	glm_mat4_identity(model);

	// Calculate the center of the button
	vec3 center = {
		button->x + (button->width / 2.0f),
		button->y + (button->height / 2.0f),
		0.0f
	};

	// Translate to the center of the button
	glm_translate(model, center);

	// Apply rotation around the center (Z-axis)
	glm_rotate(model, button->rotation, (vec3){0.0f, 0.0f, 1.0f});

	// Apply scaling around the center
	glm_scale(model, (vec3){button->scale[0], button->scale[1], 1.0f});

	// Translate back to the original position (undo centering)
	glm_translate(model, (vec3){
		-(button->width / 2.0f),
		-(button->height / 2.0f),
		0.0f
	});

	button->x = x;
	button->y = y;

    GLuint model_location = glGetUniformLocation(button->shader_program, "model");
    glUniformMatrix4fv(model_location, 1, GL_FALSE, (const GLfloat *)model);

    // Set background color or texture
    GLuint use_image_location = glGetUniformLocation(button->shader_program, "use_image");
    glUniform1i(use_image_location, button->type == BUTTON_TYPE_IMAGE);

    if (button->type == BUTTON_TYPE_IMAGE) {
        buffers_bind_vbo(button->buffers.TexCoordVBO);
        GLuint texture_location = glGetUniformLocation(button->shader_program, "texture_sampler");
        glUniform1i(texture_location, 0);  // Texture unit 0
        glBindTexture(GL_TEXTURE_2D, button->texture_id);
    }

    // Set button color if using a solid color background
    GLuint bg_color_location = glGetUniformLocation(button->shader_program, "fragColor");
    if (button->type == BUTTON_TYPE_COLOR) {
        buffers_bind_vbo(button->buffers.ColorVBO);
        glUniform4fv(bg_color_location, 1, (const GLfloat *)&button->bg_color);
    }

    // Render the button (quad)
    buffers_bind_ebo(button->buffers.EBO);

    glDisable(GL_CULL_FACE); // Disable depth test while rendering 2D text
    glDisable(GL_DEPTH_TEST); // Disable face culling while rendering 2D text
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Disable wireframe mode (render in solid mode)

    // Render the button
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);  // Draw the button using indices

    glEnable(GL_DEPTH_TEST); // Re-enable depth test after rendering

    if (wireframeMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Re-enable wireframe mode
    if (!cullingMode) glEnable(GL_CULL_FACE); // Re-enable face culling after rendering

    // Render the text (for the button's label)
    font_render_text(&button->font, button->text, button->text_offset_x + (x/2), button->text_offset_y + y, button->text_color);

    buffers_unbind_vao();
    buffers_unbind_vbo();
    buffers_unbind_ebo();
}


// Cleanup resources used by the button
void button_cleanup(Button *button) {
    // Clean up the texture if necessary
    if (button->type == BUTTON_TYPE_IMAGE) {
        glDeleteTextures(1, &button->texture_id);
    }

    // Clean up buffers using buffers.h
    buffers_destroy(&button->buffers);

    // Clean up font
    font_cleanup(&button->font);
}

bool button_check_hover(Button *button, float mouse_x, float mouse_y) {
    // Get the bounding box of the button
    float button_left = button->x;
    float button_right = button->x + button->width;
    float button_top = button->y;
    float button_bottom = button->y + button->height;

	
    // // Print the current hover bounds for debugging
    // printf("Button Hover Bounds: Left: %.2f, Right: %.2f, Top: %.2f, Bottom: %.2f\n",
    //        button_left, button_right, button_top, button_bottom);
    // printf("Mouse Position: X: %.2f, Y: %.2f\n", mouse_x, mouse_y);

    // Check if the mouse is inside the button's bounding box
    if (mouse_x >= button_left && mouse_x <= button_right &&
        mouse_y >= button_top && mouse_y <= button_bottom) {
        button->hovered = true;
        return true;
    }
    
    button->hovered = false;
    return false;
}

bool button_check_click(Button *button, float mouse_x, float mouse_y, bool mouse_pressed) {
    // Check if the button was clicked based on mouse position and mouse button state
    if (button_check_hover(button, mouse_x, mouse_y) && mouse_pressed) {
        button->clicked = true;
        return true;
    }

    button->clicked = false;
    return false;
}
void button_scale(Button *button, float scale_x, float scale_y) {
    // Update the button's scale
    glm_vec2_copy((vec2){ scale_x, scale_y }, button->scale);

    // Scale the text accordingly
    float text_width, text_height;
    font_get_text_dimensions(&button->font, button->text, &text_width, &text_height);

    text_width *= scale_x;
    text_height *= scale_y;

    // Update the text offset based on the new scaled dimensions
    button->text_offset_x = (button->width * scale_x - text_width) / 2.0f;
    button->text_offset_y = (button->height * scale_y - text_height) / 2.0f;
}

void button_translate(Button *button, float translate_x, float translate_y) {
    button->x += translate_x;
    button->y += translate_y;
}

void button_rotate(Button *button, float angle) {
    button->rotation += angle;
}

void button_change_texture(Button *button, GLuint new_texture_id) {
    if (button->type == BUTTON_TYPE_IMAGE) {
        glDeleteTextures(1, &button->texture_id); // Clean up old texture
        button->texture_id = new_texture_id;
    } else {
        printf("Button is not of type BUTTON_TYPE_IMAGE. Cannot change texture.\n");
    }
}

void button_change_color(Button *button, vec4 new_color) {
    if (button->type == BUTTON_TYPE_COLOR) {
        glm_vec4_copy(new_color, button->bg_color);
    } else {
        printf("Button is not of type BUTTON_TYPE_COLOR. Cannot change color.\n");
    }
}
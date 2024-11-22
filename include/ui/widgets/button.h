#ifndef BUTTON_H
#define BUTTON_H

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <pipeline/buffers.h>
#include <ui/text.h>

// Enum for button types
typedef enum {
    BUTTON_TYPE_COLOR,
    BUTTON_TYPE_IMAGE
} ButtonType;

// Structure to hold the button data
typedef struct {
    float x, y, width, height;  // Button position and size
    GLuint shader_program;      // Shader program used for rendering
    ButtonType type;            // Type of button (color or image)
    GLuint texture_id;          // Texture ID for button image (if applicable)
    vec4 bg_color;             // Background color (if BUTTON_TYPE_COLOR)
    vec3 text_color;           // Text color
    const char *text;           // Button label text
    Font font;                  // Font used for button text
    float text_offset_x, text_offset_y;  // Text offset for positioning
    Buffers buffers;          // Buffers (VAO, VBO, etc.) for button geometry
	bool hovered;
	bool clicked;
	float rotation;  // Rotation in radians
	vec2 scale;
} Button;

// Function declarations

// Initialize the button with text, position, size, and other properties
void button_init(Button *button, const char *text, float x, float y, float width, float height, GLuint shader_program, ButtonType type, GLuint texture_id, vec4 bg_color, Font *font, vec3 text_color);

// Render the button (draw the button and text)
void button_render(Button *button, float x, float y, int framebufferWidth, int framebufferHeight);

// Check if the mouse is hovering over the button
bool button_check_hover(Button *button, float mouse_x, float mouse_y);

// Check if the button was clicked
bool button_check_click(Button *button, float mouse_x, float mouse_y, bool mouse_pressed);

void button_change_color(Button *button, vec4 new_color);
void button_change_texture(Button *button, GLuint new_texture_id);
void button_rotate(Button *button, float angle);
void button_translate(Button *button, float translate_x, float translate_y);
void button_scale(Button *button, float scale_x, float scale_y);

// Clean up resources used by the button
void button_cleanup(Button *button);

#endif // BUTTON_H

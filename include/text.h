#ifndef TEXT_H
#define TEXT_H

#include <cglm/cglm.h>
#include <GL/glew.h>
#include <stb_truetype.h>

typedef struct {
    GLuint texture_id;
    int width, height;
    int x_offset, y_offset;
    int advance;
} Character;

typedef struct {
    Character characters[128]; // ASCII characters
    float size;
    GLuint VAO, VBO;
    GLuint shader_program;
} Font;

void font_init(Font *font, const char *font_path, float font_size, GLuint shader_program);
void font_get_text_dimensions(Font *font, const char *text, float *width, float *height);
void font_render_text(Font *font, const char *text, float x, float y, vec3 color);
void font_cleanup(Font *font);

#endif
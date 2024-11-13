#include "text.h"
#include "kbd.h"
#include <stdio.h>
#include <stdlib.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

static GLuint create_texture_from_bitmap(const unsigned char *bitmap, int width, int height) {
    // Create a new array to store the flipped bitmap
    unsigned char *flipped_bitmap = (unsigned char *)malloc(width * height);
    if (!flipped_bitmap) {
        fprintf(stderr, "Failed to allocate memory for flipped bitmap\n");
        return 0;
    }

    // Flip the bitmap vertically
    for (int y = 0; y < height; ++y) {
        memcpy(
            flipped_bitmap + y * width,                // Destination row
            bitmap + (height - 1 - y) * width,         // Source row (flipped)
            width                                      // Copy width bytes
        );
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, flipped_bitmap);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Clean up the temporary flipped bitmap
    free(flipped_bitmap);
    return texture;
}

void font_init(Font *font, const char *font_path, float font_size, GLuint shader_program) {
    font->size = font_size;
    font->shader_program = shader_program;

    // Load font data
    FILE *file = fopen(font_path, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open font file: %s\n", font_path);
        return;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    unsigned char *font_buffer = (unsigned char *)malloc(file_size);
    fread(font_buffer, 1, file_size, file);
    fclose(file);

    stbtt_fontinfo info;
    if (!stbtt_InitFont(&info, font_buffer, stbtt_GetFontOffsetForIndex(font_buffer, 0))) {
        fprintf(stderr, "Failed to initialize font.\n");
        free(font_buffer);
        return;
    }

    // Calculate a single scale factor for the font size
    float scale = stbtt_ScaleForPixelHeight(&info, font_size);

    // Generate textures for each character
    for (unsigned char c = 0; c < 128; ++c) {
        int width, height, x_offset, y_offset;

		// Special handling for the space character
        if (c == ' ') {
            continue; // Skip rendering this character
        }
        
        // Generate the bitmap for the current character
        unsigned char *bitmap = stbtt_GetCodepointBitmap(&info, 0, scale, c, &width, &height, &x_offset, &y_offset);
        
        if (bitmap) { // Check if bitmap was created successfully
            font->characters[c].texture_id = create_texture_from_bitmap(bitmap, width, height);
            font->characters[c].width = width;
            font->characters[c].height = height;
            font->characters[c].x_offset = x_offset;
            font->characters[c].y_offset = y_offset;

            int advance;
            stbtt_GetCodepointHMetrics(&info, c, &advance, NULL);
            font->characters[c].advance = (int)(advance * scale);

            stbtt_FreeBitmap(bitmap, NULL); // Free bitmap data after texture creation
        } else {
            fprintf(stderr, "Failed to generate bitmap for character %c\n", c);
            font->characters[c].texture_id = 0; // Set to 0 to indicate an invalid texture
        }
    }

    free(font_buffer);

    // Set up VAO/VBO for rendering quads (not triangles)
    glGenVertexArrays(1, &font->VAO);
    glGenBuffers(1, &font->VBO);
    glBindVertexArray(font->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, font->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 4, NULL, GL_DYNAMIC_DRAW); // 4 vertices per quad

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
}

void font_render_text(Font *font, const char *text, float x, float y, vec3 color) {
    glUseProgram(font->shader_program);
    glUniform3fv(glGetUniformLocation(font->shader_program, "textColor"), 1, color);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(font->VAO);

    for (const char *p = text; *p; p++) {
        Character ch = font->characters[(unsigned char)*p];

		// Special handling for the space character
        if (*p == ' ') {
            // Reduce the spacing for the space character
            x += font->size / 3.0f; // Adjust the multiplier to your preference (3.0f makes the space tighter)
            continue; // Skip rendering this character
        }

        // Adjust Y position for descenders like 'y', 'g', etc.
        float ypos = y + font->size + ch.y_offset;

        // Adjust X position based on character width
        float xpos = x + ch.x_offset;

        float w = ch.width;
        float h = ch.height;

        // Create a quad for each character (4 vertices)
        float vertices[4][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, font->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glDisable(GL_CULL_FACE); // Disable depth test while rendering 2D text
        glDisable(GL_DEPTH_TEST); // Disable face culling while rendering 2D text
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Disable wireframe mode (render in solid mode)
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // Use triangle strip (faster than 2 triangles)
        glEnable(GL_DEPTH_TEST); // Re-enable depth test after rendering

		if(wireframeMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Re-enable wireframe mode
		if(!cullingMode) glEnable(GL_CULL_FACE); // Re-enable face culling after rendering

        x += (ch.width + 2.0f);
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void font_cleanup(Font *font) {
    for (unsigned char c = 0; c < 128; ++c) {
        glDeleteTextures(1, &font->characters[c].texture_id);
    }
    glDeleteVertexArrays(1, &font->VAO);
    glDeleteBuffers(1, &font->VBO);
}

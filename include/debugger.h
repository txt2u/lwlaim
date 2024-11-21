#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <glad/glad.h>

void enableOpenGLDebugging();
void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
							GLsizei length, const GLchar* message, const void* userParam);
							
#endif // DEBUGGER_H
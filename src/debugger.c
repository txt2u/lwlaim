#include <debugger.h>   // Debugger
#include <stdio.h>   // For printf
#include <stddef.h>  // For NULL
#include <glad/glad.h>
#include <GL/gl.h>   // For OpenGL functions

// Function to enable OpenGL Debugging
void enableOpenGLDebugging() {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback((GLDEBUGPROC)debugCallback, NULL);  // NULL is now defined in stddef.h
}

// The debug callback function
void GLAPIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                               GLsizei length, const GLchar* message, const void* userParam) {
    // Ignore non-significant notifications
    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) {
        return;
    }

    // Print out the error information
    printf("[GL CALLBACK]\n>> H_Type: %s (0x%x)\n>> Severity: 0x%x\n# Message:\n  %s\n\n",
           (type == GL_DEBUG_TYPE_ERROR ? "ERROR" : "OTHER"),
           type, severity, message);

    // Optionally: you can add more conditions to handle different types of errors or debug info
    if (severity == GL_DEBUG_SEVERITY_HIGH) {
        printf("High severity issue occurred!\n");
    }
}
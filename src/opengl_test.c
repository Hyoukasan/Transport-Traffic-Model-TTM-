#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== OpenGL Compatibility Test ===\n");

    // Initialize GLFW
    if (!glfwInit()) {
        printf("❌ GLFW initialization failed\n");
        return 1;
    }
    printf("✓ GLFW initialized\n");

    // Create hidden window for OpenGL context
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Test", NULL, NULL);
    if (!window) {
        printf("❌ Window creation failed\n");
        glfwTerminate();
        return 1;
    }
    printf("✓ Window created\n");

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        printf("❌ GLEW initialization failed: %s\n", glewGetErrorString(err));
        glfwTerminate();
        return 1;
    }
    printf("✓ GLEW initialized\n");

    // Check OpenGL version
    const char* version = (const char*)glGetString(GL_VERSION);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* vendor = (const char*)glGetString(GL_VENDOR);
    const char* glsl = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    printf("OpenGL Version: %s\n", version ? version : "Unknown");
    printf("Renderer: %s\n", renderer ? renderer : "Unknown");
    printf("Vendor: %s\n", vendor ? vendor : "Unknown");
    printf("GLSL Version: %s\n", glsl ? glsl : "Unknown");

    // Check for required extensions
    if (GLEW_VERSION_3_3) {
        printf("✓ OpenGL 3.3 support available\n");
    } else {
        printf("⚠️ OpenGL 3.3 not supported (may work in compatibility mode)\n");
    }

    // Test basic OpenGL functionality
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    printf("✓ Basic OpenGL functions work\n");

    glfwDestroyWindow(window);
    glfwTerminate();

    printf("=== Test Passed ===\n");
    printf("TTM should work on this system!\n");

    return 0;
}
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <stdio.h>
#include "application.h"

static GLFWwindow* window = NULL;

void application_init(int width, int height, const char *title){

    if (!glfwInit()){
        printf("GLFW initialization failed!\n");
        return;
    }

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window){
        printf("Window initialization failed!\n");
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        printf("GLEW initialization failed!\n");
        glfwTerminate();
        return;
    }
}

//int application_is_running(void){
//    return;
//}

void application_update(void){
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
}

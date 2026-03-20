#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "application.h"
#include "menu.h"

static GLFWwindow* window = NULL;

static AppState app_state = APP_STATE_MAIN_MENU;
static Menu* menu = {0};

int application_init(int width, int height, const char *title){

    if (!glfwInit()){
        printf("GLFW initialization failed!\n");
        return 1;
    }

    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window){
        printf("Window initialization failed!\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    if(glewInit() != GLEW_OK){
        printf("GLEW initialization failed!\n");
        glfwTerminate();
        return 1;
    }

    return 0;
}

bool application_is_running(void){
    return !glfwWindowShouldClose(window);
}

void application_update(void){
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void application_shutdown(void){
    glfwTerminate();
}

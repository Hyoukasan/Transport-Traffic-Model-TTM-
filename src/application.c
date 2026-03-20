#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "application.h"
#include "menu.h"

static GLFWwindow* window = NULL;
static app_state = APP_STATE_MAIN_MENU;
static Menu* menu;
static int prev_lmb = GLFW_RELEASE;

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

    menu_init(menu);

    return 0;
}

bool application_is_running(void){
    return !glfwWindowShouldClose(window) && app_state != APP_STATE_EXIT;
}

void application_update(void){
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    
    int lmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    bool click = (lmb == GLFW_PRESS && prev_lmb == GLFW_RELEASE);
    prev_lmb = lmb;

    if(app_state == APP_STATE_MAIN_MENU || app_state == APP_STATE_SETTINGS_MENU){
        menu_update(menu, (int)mx, (int)my, click);
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void application_shutdown(void){
    glfwTerminate();
}

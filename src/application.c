#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "application.h"
#include "traffic_manager.h"
#include "traffic_config.h"
#include "menu.h"
#include "renderer.h"
#include "input.h"

static GLFWwindow* window = NULL;
static Menu_t menu = {0};
static InputState input = {0};
static TrafficManager tm = {0};

static AppState app_state = APP_STATE_MAIN_MENU;
static double last_frame_time = 0.0;

int application_init(const char *title){
    if (!glfwInit()){
        fprintf(stderr, "GLFW initialization failed!\n");
        return 1;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int screen_width = mode->width;
    int screen_height = mode->height;

    if (screen_width == 0 || screen_height == 0) {
        screen_width = 1920;
        screen_height = 1080;
    }

    window = glfwCreateWindow(screen_width, screen_height, title, glfwGetPrimaryMonitor(), NULL);
    if (window == NULL){
        fprintf(stderr, "Window initialization failed!\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if(glewInit() != GLEW_OK){
        fprintf(stderr, "GLEW initialization failed!\n");
        glfwTerminate();
        return 1;
    }

    menu_init(&menu);

    TrafficConfig congif = {
        .scenario = SCENARIO_HIGHWAY,
        .lane_count = 2,
        .max_cars = 100
    };

    if (traffic_manager_init(&manager, &congif) != 0) {
        return 1;
    }

    srand((unsigned int)time(NULL));

    renderer_init();
    
    last_frame_time = glfwGetTime();
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

    menu_update(&menu, (int)mx, (int)my, click);

    glClearColor(0.2f, 0.7f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    menu_render(&menu);

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void application_shutdown(void){
    for (int i = 0; i < car_count; i++) {
        car_destroy(&cars[i]);
    }
    renderer_shutdown();

    if (graph) {
        graph_destroy(graph);
    }
    glfwTerminate();
}

Graph* application_get_graph(void){
    return graph;
}

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

static GLFWwindow* window = NULL;
static Menu_t menu = {0};
static TrafficManager manager = {0};
static int prev_lmb = GLFW_RELEASE;
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

    srand((unsigned int)time(NULL));

    last_frame_time = glfwGetTime();

    renderer_init();
    

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

    if (graph != NULL) {
        double time_now = glfwGetTime();
        float dt = (float)(time_now - last_frame_time);
        if (dt <= 0.0f) {
            dt = 1.0f / 60.0f;
        }
        last_frame_time = time_now;

        for (int i = 0; i < car_count; i++) {
            car_update(&cars[i], graph, dt);
        }

        glColor3f(0.35f, 0.35f, 0.35f);
        renderer_draw_grid(graph);

        glColor3f(1.0f, 1.0f, 1.0f);
        renderer_draw_roads(graph);

        renderer_draw_cars(graph, cars, car_count);

        glColor3f(1.0f, 0.0f, 0.0f);
        renderer_draw_nodes(graph);
    }

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

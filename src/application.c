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
static TrafficManager manager = {0};

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
    input_init(&input);
    renderer_init();
    
    last_frame_time = glfwGetTime();
    return 0;
}

bool application_is_running(void){
    return !glfwWindowShouldClose(window) && app_state != APP_STATE_EXIT;
}

void application_update(void){
    input_update(window, &input);

    double now = glfwGetTime();
    float dt = (float)(now - last_frame_time);
    if (dt <= 0.0f) dt = 1.0f / 60.0f;
    last_frame_time = now;

    glClearColor(0.2f, 0.7f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    switch (app_state){
        case APP_STATE_MAIN_MENU:
            menu_update(&menu, (int)input.mouse_x, (int)input.mouse_y, input.lmb_click);

            if(menu.current_state == APP_STATE_SIMULATION_CONFIG) {
                app_state = APP_STATE_SIMULATION_CONFIG;
            } else if(menu.current_state == APP_STATE_INFO) {
                app_state = APP_STATE_INFO;
            } else if(menu.current_state == APP_STATE_EXIT) {
                app_state = APP_STATE_EXIT;
            } else if(menu.current_state == APP_STATE_CREATE_SIMULATION) {
                app_state = APP_STATE_CREATE_SIMULATION;
            }

            break;

        case APP_STATE_CREATE_SIMULATION:
            TrafficConfig config = {
                .scenario = SCENARIO_HIGHWAY,
                .lane_count = 2,
                .max_cars = 100
            };

            if(traffic_manager_init(&manager, &config) == 0) {
                renderer_upload_graph(manager.graph);
                menu.current_state = APP_STATE_RUNNING_SIMULATION;
            } else {
                menu.current_state = APP_STATE_MAIN_MENU;
            }

            break;
        
        case APP_STATE_RUNNING_SIMULATION:
            if (input.key_esc_click) {
                app_state = APP_STATE_SIMULATION_PAUSE; 
                break;
            }

            traffic_manager_update(&manager, dt);

            glColor3f(0.35f, 0.35f, 0.35f);
            renderer_draw_grid(manager.graph);

            glColor3f(1.0f, 1.0f, 1.0f);
            renderer_draw_roads(manager.graph);

            renderer_draw_cars(manager.graph, manager.cars, manager.car_count);

            glColor3f(1.0f, 0.0f, 0.0f);
            renderer_draw_nodes(manager.graph);
            break;

        case APP_STATE_INFO:
            if (input.key_esc_click) {
                app_state = APP_STATE_MAIN_MENU;
            }

            menu_render(&menu);
            break;
        
        case APP_STATE_EXIT:
            
            break;

        default:
            break;
    }
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void application_shutdown(void){
    traffic_manager_clear(&manager);
    renderer_shutdown();
    glfwTerminate();
}

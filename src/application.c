#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "application.h"
#include "app_manager.h"
#include "traffic_manager.h"
#include "traffic_config.h"
#include "menu.h"
#include "renderer.h"
#include "input.h"

static AppManager app = {0};
static GLFWwindow* window = NULL;
static Menu_t menu = {0};
static InputState input = {0};
static TrafficManager manager = {0};

static double last_frame_time = 0.0;

int application_init(const char *title){
    if (!glfwInit()){
        fprintf(stderr, "GLFW initialization failed!\n");
        return 1;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    app.screen_width        = mode->width;
    app.screen_height       = mode->height;

    if (app.screen_width == 0 || app.screen_height == 0) {
        app.screen_width  = 1920;
        app.screen_height = 1080;
    }

    window = glfwCreateWindow(app.screen_width, app.screen_height, title, glfwGetPrimaryMonitor(), NULL);
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
    return !glfwWindowShouldClose(window) && app.current_state != APP_STATE_CLOSED;
}

void application_update(void){
    input_update(window, &input);

    double now = glfwGetTime();
    float frame = (float)(now - last_frame_time);
    if (frame <= 0.0f) {
        frame = 1.0f / 60.0f;
    }
    last_frame_time = now;

    glClearColor(0.2f, 0.7f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    switch(menu.current_state) {
        case MENU_STATE_MAIN_MENU:
            menu_update(&menu, (int)input.mouse_x, (int)input.mouse_y, input.lmb_click);

            if(input.key_esc_click) {
                app.current_state = APP_STATE_CLOSED;
                menu.current_state = MENU_STATE_EXIT;
                break;
            } else {
                menu_render(&menu, app.screen_width, app.screen_height);
            }

            break;

        case MENU_STATE_CREATE_SIMULATION:
            if(input.key_esc_click) {
                menu.current_state = MENU_STATE_MAIN_MENU;
                break;
            }

            TrafficConfig config = {
                .scenario = SCENARIO_HIGHWAY,
                .lane_count = 2,
                .max_cars = 100
            };

            if(traffic_manager_init(&manager, &config) == 0) {
                renderer_upload_graph(manager.graph);
                app.current_state = APP_STATE_RUNNING_SIMULATION;
                menu.current_state = MENU_STATE_RUNNING_SIMULATION;
            } else {
                app.current_state = APP_STATE_IDLE;
                menu.current_state = MENU_STATE_MAIN_MENU;
            }

            break;

        case MENU_STATE_RUNNING_SIMULATION:
            break;

        case MENU_STATE_INFO:
            if (input.key_esc_click) {
                menu.current_state = MENU_STATE_MAIN_MENU;
                break;
            } else {
                menu_render(&menu, app.screen_width, app.screen_height);
            }

            break;
        
        case MENU_STATE_EXIT:
            app.current_state = APP_STATE_CLOSED;
            break;

        default:
            break;
    }

    switch(app.current_state) {
        case APP_STATE_IDLE:
            break;

        case APP_STATE_RUNNING_SIMULATION:
            if (input.key_esc_click) {
                menu.current_state = MENU_STATE_SIMULATION_PAUSE;
                app.current_state = APP_STATE_SIMULATION_PAUSE;
                break;
            }

            traffic_manager_update(&manager, frame);

            glColor3f(0.35f, 0.35f, 0.35f);
            renderer_draw_grid(manager.graph);

//            glColor3f(1.0f, 1.0f, 1.0f);
//            renderer_draw_roads(manager.graph);

//            renderer_draw_cars(manager.graph, manager.cars, manager.car_count);

//            glColor3f(1.0f, 0.0f, 0.0f);
//            renderer_draw_nodes(manager.graph);
            break;
        case APP_STATE_CLOSED:
            break;

        case APP_STATE_SIMULATION_PAUSE:
            if(input.key_esc_click) {
                menu.current_state = MENU_STATE_RUNNING_SIMULATION;
                app.current_state = APP_STATE_RUNNING_SIMULATION;
                break;
            }
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

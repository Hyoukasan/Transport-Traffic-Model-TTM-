#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "application.h"
#include "app_manager.h"
#include "audio_manager.h"
#include "traffic_manager.h"
#include "config_manager.h"
#include "debug_overlay.h"
#include "menu.h"
#include "renderer.h"
#include "input.h"

static AppManager app = {0};
static GLFWwindow* window = NULL;
static Menu_t menu = {0};
static Menu_t tools_menu = {0};
static InputState input = {0};
static TrafficManager manager = {0};
static ConfigManager config = {0};

static double last_frame_time = 0.0;

static void application_update_settings_text(void) {
    snprintf(menu.buttons[0].profile_text, sizeof(menu.buttons[0].profile_text), "LANES: %d", config.lane_count);
    snprintf(menu.buttons[1].profile_text, sizeof(menu.buttons[1].profile_text), "CARS: %d", config.max_cars);
}

static int slot_from_button_id(ButtonId button_id) {
    switch (button_id) {
        case BUTTON_ID_SLOT_1:
            return 1;
        case BUTTON_ID_SLOT_2:
            return 2;
        case BUTTON_ID_SLOT_3:
            return 3;
        case BUTTON_ID_SLOT_4:
            return 4;
        default:
            return 0;
    }
}

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

    menu_init(&menu, app.screen_width, app.screen_height);
    menu_init_tools_overlay(&tools_menu, app.screen_width, app.screen_height);
    input_init(&input);
    renderer_init();

    if (audio_init() != 0) {
        audio_start_menu_music();
    }      
    
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

    if(menu.current_state != MENU_STATE_SIMULATION_PAUSE && menu.current_state != MENU_STATE_SIMULATION_CONFIG_PAUSE) {
        if (app.current_state == APP_STATE_RUNNING_SIMULATION) {
            glClearColor(0.45f, 0.65f, 0.35f, 1.0f);
        } else {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        }

        glClear(GL_COLOR_BUFFER_BIT);
    }

    switch(menu.current_state) {
        case MENU_STATE_MAIN_MENU:
            menu_update(&menu, (int)input.mouse_x, (int)input.mouse_y, input.lmb_click);

            if(input.key_esc_click) {
                app.current_state = APP_STATE_CLOSED;
                menu_set_state(&menu, MENU_STATE_EXIT);
                break;
            } else {
                menu_render(&menu, app.screen_width, app.screen_height);
            }

            break;

        case MENU_STATE_SCENARIO_SELECT:
            if(input.key_esc_click) {
                menu_set_state(&menu, MENU_STATE_MAIN_MENU);
                break;
            }

            menu_update(&menu, (int)input.mouse_x, (int)input.mouse_y, input.lmb_click);
            menu_render(&menu, app.screen_width, app.screen_height);
            break;

        case MENU_STATE_SCENARIO_HIGHWAY:
            if(input.key_esc_click) {
                menu_set_state(&menu, MENU_STATE_SCENARIO_SELECT);
                break;
            }

            config.scenario   = SCENARIO_HIGHWAY;
            config.lane_count = 4;
            config.max_cars   = 10;

            menu_set_state(&menu, MENU_STATE_SIMULATION_CONFIG_SETTING);
            break;

        case MENU_STATE_SCENARIO_SINGLE_INTERSECTION:
            if(input.key_esc_click) {
                menu_set_state(&menu, MENU_STATE_SCENARIO_SELECT);
                break;
            } 
        
            config.scenario   = SCENARIO_SINGLE_INTERSECTION;
            config.lane_count = 4;
            config.max_cars   = 10;

            menu_set_state(&menu, MENU_STATE_SIMULATION_CONFIG_SETTING);
            break;

        case MENU_STATE_SCENARIO_MULTI_INTERSECTION:
            if(input.key_esc_click) {
                menu_set_state(&menu, MENU_STATE_SCENARIO_SELECT);
                break;
            } 
        
            config.scenario   = SCENARIO_MULTI_INTERSECTION;
            config.lane_count = 4;
            config.max_cars   = 10;

            menu_set_state(&menu, MENU_STATE_SIMULATION_CONFIG_SETTING);
            break;

        case MENU_STATE_SIMULATION_CONFIG_SETTING:
            if(input.key_esc_click) {
                menu_set_state(&menu, MENU_STATE_SCENARIO_SELECT);
                break;
            }

            application_update_settings_text();
            menu_update(&menu, (int)input.mouse_x, (int)input.mouse_y, input.lmb_click);
            if (menu.current_state != MENU_STATE_SIMULATION_CONFIG_SETTING) {
                break;
            }

            switch (menu.last_pressed_button) {
                case BUTTON_ID_SUB_2_LANES:
                    if (config.lane_count > 2) {
                        config.lane_count -= 2;
                    }
                    break;
                case BUTTON_ID_ADD_2_LANES:
                    if (config.lane_count < 8) {
                        config.lane_count += 2;
                    }
                    break;
                case BUTTON_ID_SUB_5_CARS:
                    if (config.max_cars > 5) {
                        config.max_cars -= 5;
                    }
                    break;
                case BUTTON_ID_ADD_5_CARS:
                    if (config.max_cars < 100) {
                        config.max_cars += 5;
                    }
                    break;
                default:
                    break;
            }

            if(config.max_cars < 10) {
                config.max_cars = 10;
            } else if(config.max_cars > 100) {
                config.max_cars = 100;
            }

            if(config.lane_count < 2) {
                config.lane_count = 2;
            } else if(config.lane_count > 8) {
                config.lane_count = 8;
            }

            application_update_settings_text();
            menu_render(&menu, app.screen_width, app.screen_height);
            break;

        case MENU_STATE_START_SIMULATION:
            if(traffic_manager_init(&manager, &config) == 0) {
                renderer_upload_graph(manager.graph);
                app.current_state = APP_STATE_RUNNING_SIMULATION;
                menu_set_state(&menu, MENU_STATE_IDLE);
            } else {
                app.current_state = APP_STATE_IDLE;
                menu_set_state(&menu, MENU_STATE_MAIN_MENU);
            }
            break;

        case MENU_STATE_INFO:
            if(input.key_esc_click) {
                menu_set_state(&menu, MENU_STATE_MAIN_MENU);
                break;
            } else {
                menu_render(&menu, app.screen_width, app.screen_height);
            }

            break;

        case MENU_STATE_LOAD_PROFILE:
            if(input.key_esc_click) {
                menu_set_state(&menu, MENU_STATE_MAIN_MENU);
                break;
            }
            
            menu_update(&menu, (int)input.mouse_x, (int)input.mouse_y, input.lmb_click);
            int slot = slot_from_button_id(menu.last_pressed_button);
            if (slot > 0) {
                float saved_time = 0.0f;
                if (config_manager_load_profile(&config, slot, &saved_time) == 0) {
                    menu_set_state(&menu, MENU_STATE_START_SIMULATION);
                }
            }

            menu_render(&menu, app.screen_width, app.screen_height);
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
                menu_set_state(&menu, MENU_STATE_SIMULATION_PAUSE);
                app.current_state = APP_STATE_SIMULATION_PAUSE;
                break;
            }

            traffic_manager_update(&manager, frame);
            menu_update(&tools_menu, (int)input.mouse_x, (int)input.mouse_y, input.lmb_click);

//            glColor3f(0.35f, 0.35f, 0.35f);
            renderer_draw_grid(manager.graph);

//            glColor3f(1.0f, 0.0f, 0.0f);
            renderer_draw_roads(manager.graph);

            renderer_draw_cars(manager.graph, manager.cars, manager.car_count);

            renderer_draw_traffic_lights(manager.graph, manager.lights, manager.light_count, manager.light_textures);

            debug_overlay_draw(&manager, app.screen_width, app.screen_height);
            menu_render(&tools_menu, app.screen_width, app.screen_height);

            break;
            
        case APP_STATE_CLOSED:
            break;

        case APP_STATE_SIMULATION_PAUSE:
            menu_update(&menu, (int)input.mouse_x, (int)input.mouse_y, input.lmb_click);

            int slot = slot_from_button_id(menu.last_pressed_button);
            if (slot > 0 && menu.current_state == MENU_STATE_SIMULATION_CONFIG_PAUSE) {
                config_manager_save_profile(&config, slot, manager.time);
                menu_set_state(&menu, MENU_STATE_SIMULATION_PAUSE);
            }

            if(menu.current_state == MENU_STATE_IDLE) {
                app.current_state = APP_STATE_RUNNING_SIMULATION;
                break;
            }

            if(menu.current_state == MENU_STATE_MAIN_MENU) {
                traffic_manager_clear(&manager);
                app.current_state = APP_STATE_IDLE;
                break;
            }

            renderer_draw_grid(manager.graph);
            renderer_draw_roads(manager.graph);
            renderer_draw_traffic_lights(manager.graph, manager.lights, manager.light_count, manager.light_textures);
            renderer_draw_cars(manager.graph, manager.cars, manager.car_count);
            debug_overlay_draw(&manager, app.screen_width, app.screen_height);

            menu_render(&menu, app.screen_width, app.screen_height);

            if(input.key_esc_click) {
                menu_set_state(&menu, MENU_STATE_IDLE);
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
    audio_shutdown();
    traffic_manager_clear(&manager);
    renderer_shutdown();
    glfwTerminate();
}

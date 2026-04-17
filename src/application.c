#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "application.h"
#include "graph.h"
#include "car.h"
#include "road_generator.h"
#include "menu.h"
#include "renderer.h"

static GLFWwindow* window = NULL;
static AppState app_state = APP_STATE_MAIN_MENU;
static Menu_t menu = {0};
static int prev_lmb = GLFW_RELEASE;
static Graph* graph = NULL;
static Car cars[8];
static int car_count = 0;
static double last_frame_time = 0.0;

int application_init(const char *title){
    // Создаем лог-файл для отладки
    FILE *logfile = fopen("debug_log.txt", "w");
    
    if (!glfwInit()){
        fprintf(stderr, "GLFW initialization failed!\n");
        if (logfile) { fprintf(logfile, "GLFW initialization failed!\n"); fclose(logfile); }
        return 1;
    }

    // Получаем размеры главного монитора
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int screen_width = mode->width;
    int screen_height = mode->height;
    
    // Для Linux можно добавить fallback на фиксированный размер
    if (screen_width == 0 || screen_height == 0) {
        screen_width = 1920;
        screen_height = 1080;
        printf("Warning: Could not get monitor resolution, using 1920x1080\n");
    }
    
    if (logfile) fprintf(logfile, "Screen resolution: %dx%d\n", screen_width, screen_height);
    printf("Screen resolution: %dx%d\n", screen_width, screen_height);

    window = glfwCreateWindow(screen_width, screen_height, title, glfwGetPrimaryMonitor(), NULL);
    if (!window){
        fprintf(stderr, "Window initialization failed!\n");
        if (logfile) { fprintf(logfile, "Window initialization failed!\n"); fclose(logfile); }
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    if(glewInit() != GLEW_OK){
        fprintf(stderr, "GLEW initialization failed!\n");
        if (logfile) { fprintf(logfile, "GLEW initialization failed!\n"); fclose(logfile); }
        glfwTerminate();
        return 1;
    }

    // Проверка версии OpenGL
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Vendor: %s\n", glGetString(GL_VENDOR));

    if (logfile) {
        fprintf(logfile, "OpenGL Version: %s\n", glGetString(GL_VERSION));
        fprintf(logfile, "GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
        fprintf(logfile, "Renderer: %s\n", glGetString(GL_RENDERER));
        fprintf(logfile, "Vendor: %s\n", glGetString(GL_VENDOR));
    }

    menu_init(&menu);

    // Инициализируем граф сетки дорог
    int chunk_size = 50;  // размер одного чанка в пикселях
    int padding = 1;      // отступ от края в чанках
    graph = graph_create(screen_width, screen_height, chunk_size, padding);
    if (!graph) {
        fprintf(stderr, "Failed to create road graph\n");
        if (logfile) { fprintf(logfile, "Failed to create road graph\n"); fclose(logfile); }
        glfwTerminate();
        return 1;
    }

    srand((unsigned int)time(NULL));
    
    if (logfile) {
        fprintf(logfile, "Graph created\n");
        fprintf(logfile, "  Grid size: %dx%d chunks\n", graph->grid_width, graph->grid_height);
        fprintf(logfile, "  Chunk size: %d\n", chunk_size);
    }
    
    // Генерируем и строим дороги
    int min_roads = 2;
    int max_roads = graph->grid_width / 2 + graph->grid_height / 2;
    if (max_roads < min_roads) {
        max_roads = min_roads;
    }
    int num_roads = min_roads + rand() % (max_roads - min_roads + 1);
    printf("Generating %d roads...\n", num_roads);
    RoadGenerator* gen = road_gen_create(num_roads);
    if (!gen) {
        fprintf(stderr, "Failed to create road generator\n");
        if (logfile) { fprintf(logfile, "Failed to create road generator\n"); fclose(logfile); }
        graph_destroy(graph);
        glfwTerminate();
        return 1;
    }

    if (logfile) fprintf(logfile, "Road generator created\n");
    
    road_gen_generate_points(gen, graph);
    road_gen_build_roads(gen, graph);
    
    if (logfile) {
        fprintf(logfile, "\nRoad network info:\n");
        fprintf(logfile, "  Total roads: %d\n", graph->road_count);
        fprintf(logfile, "\nRoads:\n");
        for (int i = 0; i < graph->road_count; i++) {
            RoadSegment *road = &graph->roads[i];
            fprintf(logfile, "  Road %d: (%d,%d)->(%d,%d) type=%s speed=%.1f lanes=%d accident=%s\n",
                   i, road->x1, road->y1, road->x2, road->y2,
                   road->type == ROAD_HORIZONTAL ? "H" : road->type == ROAD_VERTICAL ? "V" : "D",
                   road->speed_limit, road->lanes, road->accident ? "yes" : "no");
        }
        fflush(logfile);
    }
    
    road_gen_destroy(gen);

    if (graph->road_count > 0) {
        car_init(&cars[0], 0, 0, 0.8f, 1.0f, 0, 0.0f);
        car_init(&cars[1], 1, graph->road_count > 1 ? 1 : 0, 0.6f, 1.0f, 0, 0.0f);
        cars[1].color[0] = 0.2f;
        cars[1].color[1] = 0.4f;
        cars[1].color[2] = 0.9f;
        car_count = graph->road_count > 1 ? 2 : 1;

        FILE *car_file = fopen("car.png", "rb");
        if (car_file) {
            fclose(car_file);
            int tex_w = 0, tex_h = 0;
            unsigned int car_texture = car_load_texture("car.png", &tex_w, &tex_h);
            if (car_texture != 0) {
                car_set_texture(&cars[0], car_texture, (float)tex_w, (float)tex_h);
            }
        }
    }

    last_frame_time = glfwGetTime();

    printf("\nGrid info:\n");
    printf("  Grid size: %dx%d chunks\n", graph->grid_width, graph->grid_height);
    printf("  Chunk size: %dpx\n", chunk_size);
    printf("  Road segments: %d\n", graph->road_count);
    printf("  Cars: %d\n", car_count);
    printf("  Window: %dx%d\n", screen_width, screen_height);

    menu_init(&menu);
    
    // Инициализируем рендерер
    renderer_init();
    
    if (logfile) {
        fprintf(logfile, "\nApplication initialized successfully!\n");
        fclose(logfile);
    }

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

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (graph) {
        double time_now = glfwGetTime();
        float dt = (float)(time_now - last_frame_time);
        if (dt <= 0.0f) {
            dt = 1.0f / 60.0f;
        }
        last_frame_time = time_now;

        for (int i = 0; i < car_count; i++) {
            car_update(&cars[i], graph, dt);
        }

        // Draw grid background to make сетку видимой
        glColor3f(0.35f, 0.35f, 0.35f);
        renderer_draw_grid(graph);

        // Set color for roads (white)
        glColor3f(1.0f, 1.0f, 1.0f);
        renderer_draw_roads(graph);

        // Draw cars on roads
        renderer_draw_cars(graph, cars, car_count);

        // Set color for nodes (red)
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

#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdbool.h>

#include "application.h"
#include "graph.h"
#include "road_generator.h"
#include "menu.h"
#include "renderer.h"

static GLFWwindow* window = NULL;
static AppState app_state = APP_STATE_MAIN_MENU;
static Menu menu = {0};
static int prev_lmb = GLFW_RELEASE;
static Graph* graph = NULL;

int application_init(const char *title){
    // Создаем лог-файл для отладки
    FILE *logfile = fopen("debug_log.txt", "w");
    
    if (!glfwInit()){
        printf("GLFW initialization failed!\n");
        if (logfile) { fprintf(logfile, "GLFW initialization failed!\n"); fclose(logfile); }
        return 1;
    }

    // Получаем размеры главного монитора
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int screen_width = mode->width;
    int screen_height = mode->height;
    
    if (logfile) fprintf(logfile, "Screen resolution: %dx%d\n", screen_width, screen_height);
    printf("Screen resolution: %dx%d\n", screen_width, screen_height);

    window = glfwCreateWindow(screen_width, screen_height, title, glfwGetPrimaryMonitor(), NULL);
    if (!window){
        printf("Window initialization failed!\n");
        if (logfile) { fprintf(logfile, "Window initialization failed!\n"); fclose(logfile); }
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    if(glewInit() != GLEW_OK){
        printf("GLEW initialization failed!\n");
        if (logfile) { fprintf(logfile, "GLEW initialization failed!\n"); fclose(logfile); }
        glfwTerminate();
        return 1;
    }

    // Инициализируем граф сетки дорог
    int chunk_size = 50;  // размер одного чанка в пикселях
    int padding = 1;      // отступ от края в чанках
    graph = graph_create(screen_width, screen_height, chunk_size, padding);
    
    if (logfile) {
        fprintf(logfile, "Graph created\n");
        fprintf(logfile, "  Grid size: %dx%d chunks\n", graph->grid_width, graph->grid_height);
        fprintf(logfile, "  Chunk size: %d\n", chunk_size);
    }
    
    // Генерируем и строим дороги
    int num_roads = 4;  // количество дорог
    RoadGenerator* gen = road_gen_create(num_roads);
    
    if (logfile) fprintf(logfile, "Road generator created\n");
    
    road_gen_generate_points(gen, graph);
    road_gen_build_roads(gen, graph);
    
    if (logfile) {
        fprintf(logfile, "\nRoad network info:\n");
        fprintf(logfile, "  Total nodes: %d\n", graph->node_count);
        fprintf(logfile, "  Total edges: %d\n", graph->edge_count);
        fprintf(logfile, "\nNodes:\n");
        for (int i = 0; i < graph->node_count; i++) {
            fprintf(logfile, "  Node %d: grid(%d,%d) pixel(%d,%d) roads=%d\n", 
                   i, graph->nodes[i].grid_x, graph->nodes[i].grid_y,
                   graph->nodes[i].pixel_x, graph->nodes[i].pixel_y,
                   graph->nodes[i].road_count);
        }
        fprintf(logfile, "\nEdges:\n");
        for (int i = 0; i < graph->edge_count; i++) {
            fprintf(logfile, "  Edge %d: %d->%d type=%s length=%d\n", 
                   i, graph->edges[i].from, graph->edges[i].to,
                   graph->edges[i].type == ROAD_HORIZONTAL ? "H" : "V",
                   graph->edges[i].length);
        }
        fflush(logfile);
    }
    
    road_gen_destroy(gen);
    
    printf("\nGrid info:\n");
    printf("  Grid size: %dx%d chunks\n", graph->grid_width, graph->grid_height);
    printf("  Chunk size: %dpx\n", chunk_size);
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

    if(app_state == APP_STATE_MAIN_MENU || app_state == APP_STATE_SETTINGS_MENU){
        menu_update(&menu, (int)mx, (int)my, click);
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Render roads and intersections
    if (graph) {
        // Set color for roads (white)
        glColor3f(1.0f, 1.0f, 1.0f);
        renderer_draw_roads(graph);
        
        // Set color for nodes (red)
        glColor3f(1.0f, 0.0f, 0.0f);
        renderer_draw_nodes(graph);
    }
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void application_shutdown(void){
    renderer_shutdown();
    
    if (graph) {
        graph_destroy(graph);
    }
    glfwTerminate();
}

Graph* application_get_graph(void){
    return graph;
}

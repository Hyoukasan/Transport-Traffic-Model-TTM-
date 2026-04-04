#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "application_headless.h"
#include "graph.h"
#include "road_generator.h"

static Graph* graph = NULL;
static int simulation_step = 0;
static bool running = true;

int application_init_headless(void) {
    printf("=== Transport Traffic Model (Headless Mode) ===\n");

    // Фиксированные размеры для headless режима
    int screen_width = 1920;
    int screen_height = 1080;
    int chunk_size = 50;
    int padding = 1;

    printf("Initializing graph: %dx%d pixels, chunk_size=%d\n",
           screen_width, screen_height, chunk_size);

    graph = graph_create(screen_width, screen_height, chunk_size, padding);

    printf("Grid size: %dx%d chunks\n", graph->grid_width, graph->grid_height);

    // Генерируем дороги
    int num_roads = 4;
    printf("Generating %d roads...\n", num_roads);

    RoadGenerator* gen = road_gen_create(num_roads);
    road_gen_generate_points(gen, graph);
    road_gen_build_roads(gen, graph);
    road_gen_destroy(gen);

    printf("Road network created:\n");
    printf("  Nodes (intersections): %d\n", graph->node_count);
    printf("  Edges (roads): %d\n", graph->edge_count);

    // Выводим информацию о перекрестках
    printf("\nIntersections:\n");
    for (int i = 0; i < graph->node_count; i++) {
        Node* node = &graph->nodes[i];
        printf("  #%d: grid(%d,%d) pixel(%d,%d) roads=%d\n",
               i, node->grid_x, node->grid_y,
               node->pixel_x, node->pixel_y, node->road_count);
    }

    // Выводим информацию о дорогах
    printf("\nRoads:\n");
    for (int i = 0; i < graph->edge_count; i++) {
        Edge* edge = &graph->edges[i];
        printf("  #%d: %d -> %d (%s, length=%d)\n",
               i, edge->from, edge->to,
               edge->type == ROAD_HORIZONTAL ? "horizontal" : "vertical",
               edge->length);
    }

    printf("\nHeadless initialization complete!\n");
    printf("Press Ctrl+C to exit\n\n");

    return 0;
}

bool application_is_running_headless(void) {
    return running;
}

void application_update_headless(void) {
    simulation_step++;

    // Имитация симуляции
    printf("Simulation step %d\n", simulation_step);

    // Здесь будет логика движения машин
    // Пока просто выводим статистику

    if (simulation_step % 10 == 0) {
        printf("  Network status: %d intersections, %d roads\n",
               graph->node_count, graph->edge_count);
    }

    // Имитация задержки
    for (volatile int i = 0; i < 1000000; i++) {}

    // Выход через 100 шагов для тестирования
    if (simulation_step >= 100) {
        running = false;
    }
}

void application_shutdown_headless(void) {
    if (graph) {
        graph_destroy(graph);
    }
    printf("\nSimulation ended after %d steps\n", simulation_step);
}

Graph* application_get_graph_headless(void) {
    return graph;
}
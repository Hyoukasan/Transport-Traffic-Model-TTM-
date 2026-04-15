#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

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
    if (!graph) {
        fprintf(stderr, "Headless: failed to create graph\n");
        return 1;
    }

    printf("Grid size: %dx%d chunks\n", graph->grid_width, graph->grid_height);

    srand((unsigned int)time(NULL));

    // Генерируем дороги
    int min_roads = 2;
    int max_roads = graph->grid_width / 2 + graph->grid_height / 2;
    if (max_roads < min_roads) {
        max_roads = min_roads;
    }
    int num_roads = min_roads + rand() % (max_roads - min_roads + 1);
    printf("Generating %d roads...\n", num_roads);

    RoadGenerator* gen = road_gen_create(num_roads);
    if (!gen) {
        fprintf(stderr, "Headless: failed to create road generator\n");
        graph_destroy(graph);
        return 1;
    }
    road_gen_generate_points(gen, graph);
    road_gen_build_roads(gen, graph);
    road_gen_destroy(gen);

    printf("Road network created:\n");
    printf("  Total roads: %d\n", graph->road_count);

    printf("\nRoads:\n");
    for (int i = 0; i < graph->road_count; i++) {
        RoadSegment *road = &graph->roads[i];
        printf("  #%d: (%d,%d)->(%d,%d) type=%s speed=%.1f lanes=%d accident=%s\n",
               i, road->x1, road->y1, road->x2, road->y2,
               road->type == ROAD_HORIZONTAL ? "horizontal" : road->type == ROAD_VERTICAL ? "vertical" : "diagonal",
               road->speed_limit, road->lanes, road->accident ? "yes" : "no");
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
    // Покачто только статистика

    if (simulation_step % 10 == 0) {
        printf("  Network status: %d roads\n", graph->road_count);
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
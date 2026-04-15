#include "road_generator.h"
#include <stdlib.h>
#include <stdio.h>

RoadGenerator* road_gen_create(int num_roads) {
    if (num_roads <= 0) {
        fprintf(stderr, "road_gen_create: invalid road count %d\n", num_roads);
        return NULL;
    }

    RoadGenerator *gen = malloc(sizeof(RoadGenerator));
    if (!gen) {
        fprintf(stderr, "road_gen_create: failed to allocate RoadGenerator\n");
        return NULL;
    }

    gen->point_count = num_roads;
    gen->max_points = num_roads;
    gen->points = malloc(sizeof(Point) * num_roads);
    if (!gen->points) {
        fprintf(stderr, "road_gen_create: failed to allocate points\n");
        free(gen);
        return NULL;
    }

    gen->horizontal_roads = num_roads / 2;
    gen->vertical_roads = num_roads - gen->horizontal_roads;

    return gen;
}

void road_gen_generate_points(RoadGenerator *gen, Graph *graph) {
    if (!gen || !graph || gen->point_count == 0) {
        return;
    }

    for (int i = 0; i < gen->horizontal_roads; i++) {
        gen->points[i].x = rand() % graph->grid_width;
        gen->points[i].y = rand() % graph->grid_height;
    }

    for (int i = gen->horizontal_roads; i < gen->point_count; i++) {
        gen->points[i].x = rand() % graph->grid_width;
        gen->points[i].y = rand() % graph->grid_height;
    }

    printf("Generated %d road points:\n", gen->point_count);
    printf("  Horizontal roads: %d\n", gen->horizontal_roads);
    printf("  Vertical roads: %d\n", gen->vertical_roads);

    for (int i = 0; i < gen->point_count; i++) {
        printf("  Point %d: (%d, %d)\n", i, gen->points[i].x, gen->points[i].y);
    }
}

void road_gen_build_roads(RoadGenerator *gen, Graph *graph) {
    if (!gen || !graph) {
        return;
    }

    for (int i = 0; i < gen->horizontal_roads; i++) {
        Point *p = &gen->points[i];
        int y = p->y;
        float speed_limit = 0.8f + (rand() % 5) * 0.1f;
        int road_id = graph_add_road(graph, 0, y, graph->grid_width - 1, y, ROAD_HORIZONTAL, speed_limit);
        if (road_id >= 0) {
            printf("Horizontal road %d: from (0, %d) to (%d, %d), speed=%.1f\n",
                   road_id, y, graph->grid_width - 1, y, speed_limit);
        }
    }

    for (int i = 0; i < gen->vertical_roads; i++) {
        Point *p = &gen->points[gen->horizontal_roads + i];
        int x = p->x;
        float speed_limit = 0.8f + (rand() % 5) * 0.1f;
        int road_id = graph_add_road(graph, x, 0, x, graph->grid_height - 1, ROAD_VERTICAL, speed_limit);
        if (road_id >= 0) {
            printf("Vertical road %d: from (%d, 0) to (%d, %d), speed=%.1f\n",
                   road_id, x, x, graph->grid_height - 1, speed_limit);
        }
    }

    printf("\nRoad network created:\n");
    printf("  Total roads: %d\n", graph->road_count);
}

void road_gen_destroy(RoadGenerator *gen) {
    if (!gen) return;
    
    free(gen->points);
    free(gen);
}

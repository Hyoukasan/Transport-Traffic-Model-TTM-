#include "road_generator.h"
#include "graph.h"
#include "texture.h"
#include <stdlib.h>
#include <stdio.h>

static int road_count_for_scenario(int scenario) {
    switch (scenario) {
        case ROAD_SCENARIO_HIGHWAY:
            return 1;
        case ROAD_SCENARIO_SINGLE_INTERSECTION:
            return 2;
        case ROAD_SCENARIO_MULTI_INTERSECTION:
            return 4;
        default:
            return 2;
    }
}

static void set_point(Point *point, int x, int y) {
    point->x = x;
    point->y = y;
}

static float random_speed_limit(void) {
    return 0.8f + (rand() % 5) * 0.1f;
}

static void build_roads_range(RoadGenerator *gen, Graph *graph, int start_index, int count, RoadType type) {
    for (int i = 0; i < count; i++) {
        Point *p = &gen->points[start_index + i];
        if (type == ROAD_HORIZONTAL) {
            float speed_limit = (gen->scenario == ROAD_SCENARIO_HIGHWAY) ? 1.0f : random_speed_limit();
            int road_id = graph_add_road(graph, 0, p->y, graph->grid_width - 1, p->y, ROAD_HORIZONTAL, speed_limit, 4);
            if (road_id >= 0) {
                unsigned int texture = texture_load("Data/textures/background_menu.png", NULL, NULL);
                graph_set_road_texture(graph, road_id, texture);
                printf("горизонтальная дорога %d: от (0, %d) до (%d, %d), скорость=%.1f\n",
                       road_id, p->y, graph->grid_width - 1, p->y, speed_limit);
            }
        } else {
            float speed_limit = random_speed_limit();
            int road_id = graph_add_road(graph, p->x, 0, p->x, graph->grid_height - 1, ROAD_VERTICAL, speed_limit, 4);
            if (road_id >= 0) {
                unsigned int texture = texture_load("Data/textures/background_menu.png", NULL, NULL);
                graph_set_road_texture(graph, road_id, texture);
                printf("вертикальная дорога %d: от (%d, 0) до (%d, %d), скорость=%.1f\n",
                       road_id, p->x, p->x, graph->grid_height - 1, speed_limit);
            }
        }
    }
}

static RoadGenerator* road_gen_create_internal(int num_roads) {
    if (num_roads <= 0) {
        fprintf(stderr, "road_gen_create: неправильное количество дорог %d\n", num_roads);
        return NULL;
    }

    RoadGenerator *gen = malloc(sizeof(RoadGenerator));
    if (!gen) {
        fprintf(stderr, "road_gen_create: не удалось выделить память для RoadGenerator\n");
        return NULL;
    }

    gen->point_count = num_roads;
    gen->max_points = num_roads;
    gen->points = malloc(sizeof(Point) * num_roads);
    if (!gen->points) {
        fprintf(stderr, "road_gen_create: не удалось выделить память для точек\n");
        free(gen);
        return NULL;
    }

    gen->horizontal_roads = num_roads / 2;
    gen->vertical_roads = num_roads - gen->horizontal_roads;
    gen->scenario = ROAD_SCENARIO_SINGLE_INTERSECTION;

    return gen;
}

RoadGenerator* road_gen_create(int num_roads) {
    return road_gen_create_internal(num_roads);
}

RoadGenerator* road_gen_create_with_scenario(int scenario) {
    int num_roads = road_count_for_scenario(scenario);
    RoadGenerator *gen = road_gen_create_internal(num_roads);
    if (!gen) {
        return NULL;
    }

    gen->scenario = scenario;
    return gen;
}

void road_gen_generate_points(RoadGenerator *gen, Graph *graph) {
    if (!gen || !graph || gen->max_points == 0) {
        return;
    }

    if (graph->grid_width <= 0 || graph->grid_height <= 0) {
        return;
    }

    switch (gen->scenario) {
        case ROAD_SCENARIO_HIGHWAY:
            gen->point_count = 1;
            gen->horizontal_roads = 1;
            gen->vertical_roads = 0;
            set_point(&gen->points[0], 0, graph->grid_height / 2);
            break;
        case ROAD_SCENARIO_SINGLE_INTERSECTION:
            gen->point_count = 2;
            gen->horizontal_roads = 1;
            gen->vertical_roads = 1;
            set_point(&gen->points[0], 0, graph->grid_height / 2);
            set_point(&gen->points[1], graph->grid_width / 2, 0);
            break;
        case ROAD_SCENARIO_MULTI_INTERSECTION:
            gen->point_count = 5;
            gen->horizontal_roads = 3;
            gen->vertical_roads = 2;
            set_point(&gen->points[0], 0, graph->grid_height / 3);
            set_point(&gen->points[1], 0, (graph->grid_height * 2) / 3);
            set_point(&gen->points[2], graph->grid_width / 3, 0);
            set_point(&gen->points[3], (graph->grid_width * 2) / 3, 0);
            break;
        default:
            gen->point_count = 2;
            gen->horizontal_roads = 1;
            gen->vertical_roads = 1;
            set_point(&gen->points[0], 0, graph->grid_height / 2);
            set_point(&gen->points[1], graph->grid_width / 2, 0);
            break;
    }

    if (gen->point_count > gen->max_points) {
        gen->point_count = gen->max_points;
    }

    printf("Сгенерировано %d точек дорог для сценария %d:\n", gen->point_count, gen->scenario);
    printf("  Горизонтальные дороги: %d\n", gen->horizontal_roads);
    printf("  вертикальные дороги: %d\n", gen->vertical_roads);

    for (int i = 0; i < gen->point_count; i++) {
        printf("  точка %d: (%d, %d)\n", i, gen->points[i].x, gen->points[i].y);
    }
}

// Выполнить полный шаг: выбрать кейс, сгенерировать точки и построить дороги.
void road_gen_generate_and_build(RoadGenerator *gen, Graph *graph, int scenario) {
    if (!gen || !graph) {
        return;
    }

    gen->scenario = scenario;
    road_gen_generate_points(gen, graph);
    road_gen_build_roads(gen, graph);
}

void road_gen_build_roads(RoadGenerator *gen, Graph *graph) {
    if (!gen || !graph) {
        return;
    }

    if (gen->horizontal_roads > 0) {
        build_roads_range(gen, graph, 0, gen->horizontal_roads, ROAD_HORIZONTAL);
    }

    if (gen->vertical_roads > 0) {
        build_roads_range(gen, graph, gen->horizontal_roads, gen->vertical_roads, ROAD_VERTICAL);
    }

    printf("\nсеть дорог создана:\n");
    printf("  всего дорог: %d\n", graph->road_count);
}

void road_gen_destroy(RoadGenerator *gen) {
    if (!gen) return;
    
    free(gen->points);
    free(gen);
}
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "traffic_manager.h"
#include "traffic_config.h"
#include "texture.h"
#include "car.h"
#include "graph.h"
#include "road_generator.h"

static void traffic_manager_load_car_textures(TrafficManager* manager) {
    manager->car_textures[CAR_COLOR_YELLOW] =
        texture_load("Data/textures/car_color_yellow.png", NULL, NULL);

    manager->car_textures[CAR_COLOR_BLACK] =
        texture_load("Data/textures/car_color_black.png", NULL, NULL);

    manager->car_textures[CAR_COLOR_RED] =
        texture_load("Data/textures/car_color_red.png", NULL, NULL);

    manager->car_textures[CAR_COLOR_GREEN] =
        texture_load("Data/textures/car_color_green.png", NULL, NULL);

    manager->car_textures[CAR_COLOR_BLUE] =
        texture_load("Data/textures/car_color_blue.png", NULL, NULL);
}

static int traffic_manager_build_roads(TrafficManager* manager, int scenario, int lane_count) {
    RoadGenerator *road_gen = road_gen_create_with_scenario(scenario, lane_count);
    if (road_gen == NULL) {
        return -1;
    }

    road_gen_generate_and_build(road_gen, manager->graph, scenario);
    road_gen_destroy(road_gen);
    graph_build_intersections(manager->graph);
    return 0;
}

static void traffic_manager_spawn_cars(TrafficManager* manager, const TrafficConfig* config) {
    if (manager == NULL || manager->graph == NULL || config == NULL || manager->graph->road_count <= 0) {
        return;
    }

    int roads = manager->graph->road_count;
    int total_cars = config->max_cars > 0 ? config->max_cars : 0;
    if (total_cars > manager->max_cars) {
        total_cars = manager->max_cars;
    }

    for (int i = 0; i < total_cars; i++) {
        int road_id = i % roads;
        RoadSegment *road = &manager->graph->roads[road_id];
        int lane = config->lane_count > 0 ? (i % config->lane_count) : 0;

        float speed_factor = 0.6f + (float)(rand() % 41) / 100.0f;
        float desired_speed = road->speed_limit * speed_factor;

        Car* car = &manager->cars[manager->car_count];

        car_init(car, manager->next_car_id++, road_id, desired_speed, 1.0f, lane, 0.0f);
        car->position = 0.05f + ((float)i / (float)total_cars) * 0.4f;

        CarColor color = (CarColor)(rand() % 5);
        car->color = color;
        car_set_texture(car, manager->car_textures[color]);

        manager->car_count++;
    }
}

int traffic_manager_init(TrafficManager* manager, const TrafficConfig* config) {
    if(manager == NULL || config == NULL) {
        fprintf(stderr, "Invalid args!\n");
        return -1;
    }

    memset(manager, 0, sizeof(*manager));

    if (config->max_cars <= 0) {
        fprintf(stderr, "Count cars must be > 0\n");
        return -1;
    }

    manager->max_cars = config->max_cars;
    manager->cars = (Car*)calloc((size_t)manager->max_cars, sizeof(Car));
    if(manager->cars == NULL) {
        fprintf(stderr, "Cars initialization failed!\n");
        traffic_manager_clear(manager);
        return -1;
    }
    
    manager->max_accidents = 16;
    manager->accidents = (AccidentDTP*)calloc((size_t)manager->max_accidents, sizeof(AccidentDTP));
    if(manager->accidents == NULL) {
        fprintf(stderr, "Accidents initialization failed!\n");
        traffic_manager_clear(manager);
        return -1;
    }

    manager->graph = graph_create(1920, 1080, 40, 0, config->max_roads);
    if(manager->graph == NULL) {
        fprintf(stderr, "Graph initialization failed!\n");
        traffic_manager_clear(manager);
        return -1;
    }

    if (traffic_manager_build_roads(manager, config->scenario, config->lane_count) != 0) {
        fprintf(stderr, "Road generation failed!\n");
        traffic_manager_clear(manager);
        return -1;
    }

    manager->car_count = 0;
    manager->accident_count = 0;
    manager->light_count = 0;
    manager->time = 0.0f;
    manager->next_car_id = 0;

    traffic_manager_load_car_textures(manager);
    traffic_manager_spawn_cars(manager, config);

    return 0;
}

int traffic_manager_update(TrafficManager *manager, float dt) {
    if (manager == NULL || manager->graph == NULL) {
        return -1;
    }

    for (int i = 0; i < manager->car_count; i++) {
        car_update(&manager->cars[i], manager->graph, dt);
    }

    manager->time += dt;
    return 0;
}

void traffic_manager_clear(TrafficManager *manager) {
    if (manager == NULL) {
        return;
    }

    if(manager->graph != NULL) {
        graph_destroy(manager->graph);
        manager->graph = NULL;
    }

    if(manager->cars != NULL) {
        //for(int i = 0; i < manager->car_count; i++) {
        //    car_destroy(&manager->cars[i]);
        //}
    }
    
    free(manager->cars);
    manager->cars = NULL;

    free(manager->lights);
    manager->lights = NULL;

    free(manager->accidents);
    manager->accidents = NULL;

    manager->car_count = 0;
    manager->max_cars = 0;
    manager->light_count = 0;
    manager->max_lights = 0;
    manager->accident_count = 0;
    manager->max_accidents = 0;
    manager->time = 0.0f;
    manager->next_car_id = 0;
}


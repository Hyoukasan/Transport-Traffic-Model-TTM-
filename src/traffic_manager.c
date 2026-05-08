#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "traffic_manager.h"
#include "config_manager.h"
#include "texture.h"
#include "car.h"
#include "geometry.h"
#include "graph.h"
#include "road_generator.h"

static void traffic_manager_load_car_textures(TrafficManager* manager);

static const Car* traffic_manager_find_front_car(TrafficManager* manager, const Car* car, float search_radius);
static bool traffic_manager_update_lane_change(TrafficManager* manager, Car* car);
static bool traffic_manager_try_start_lane_change(TrafficManager* manager, Car* car);


static void traffic_manager_load_car_textures(TrafficManager* manager) {
    manager->car_textures[CAR_COLOR_YELLOW] =
        texture_load("data/textures/car_color_yellow.png", NULL, NULL);

    manager->car_textures[CAR_COLOR_BLACK] =
        texture_load("data/textures/car_color_black.png", NULL, NULL);

    manager->car_textures[CAR_COLOR_RED] =
        texture_load("data/textures/car_color_red.png", NULL, NULL);

    manager->car_textures[CAR_COLOR_GREEN] =
        texture_load("data/textures/car_color_green.png", NULL, NULL);

    manager->car_textures[CAR_COLOR_BLUE] =
        texture_load("data/textures/car_color_blue.png", NULL, NULL);
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

static int traffic_manager_max_roads_for_scenario(ScenarioType scenario) {
    switch (scenario) {
        case SCENARIO_HIGHWAY:
            return 1;
        case SCENARIO_SINGLE_INTERSECTION:
            return 2;
        case SCENARIO_MULTI_INTERSECTION:
            return 6;
        default:
            return 1;
    }
}

static void traffic_manager_spawn_cars(TrafficManager* manager, const ConfigManager* config) {
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
        int lane = road->lanes > 0 ? (i % road->lanes) : 0;

        float speed_factor = 0.6f + (float)(rand() % 41) / 100.0f;
        float desired_speed = road->speed_limit * speed_factor;

        Car* car = &manager->cars[manager->car_count];

        car_init(car, manager->next_car_id++, road_id, desired_speed, 1.0f, lane, 0.0f);

        int road_car_index = i / roads;
        float travel_position = 0.02f + (float)road_car_index * 0.06f;
        if (travel_position > 0.30f) {
            travel_position = 0.30f;
        }

        RoadDirection spawn_direction = road->direction;
        if (spawn_direction == ROAD_DIR_NONE) {
            int lanes = road->lanes > 0 ? road->lanes : 1;
            int half = lanes / 2;
            if (road->type == ROAD_HORIZONTAL) {
                spawn_direction = lane < half ? ROAD_DIR_WEST : ROAD_DIR_EAST;
            } else if (road->type == ROAD_VERTICAL) {
                spawn_direction = lane < half ? ROAD_DIR_SOUTH : ROAD_DIR_NORTH;
            }
        }

        if (spawn_direction == ROAD_DIR_WEST || spawn_direction == ROAD_DIR_NORTH) {
            car->position = 1.0f - travel_position;
        } else {
            car->position = travel_position;
        }

        car->angle = direction_to_angle(spawn_direction);

        CarColor color = (CarColor)(rand() % 5);
        car->color = color;
        car_set_texture(car, manager->car_textures[color]);

        manager->car_count++;
    }
}

static int traffic_manager_init_lane_lists(TrafficManager* manager) {
    if (manager == NULL || manager->graph == NULL || manager->lane_lists == NULL) {
        return -1;
    }

    int index = 0;

    for (int road_id = 0; road_id < manager->graph->road_count; road_id++) {
        RoadSegment *road = &manager->graph->roads[road_id];

        for (int lane = 0; lane < road->lanes; lane++) {
            if (index >= manager->lane_list_count) {
                return -1;
            }

            LaneCarList *list = &manager->lane_lists[index];

            list->road_id = road->id;
            list->lane = lane;
            list->car_count = 0;
            list->car_indices = (int*)malloc(sizeof(int) * manager->max_cars);

            if (list->car_indices == NULL) {
                return -1;
            }

            index++;
        }
    }

    return 0;
}

int traffic_manager_init(TrafficManager* manager, const ConfigManager* config) {
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

    int max_roads = traffic_manager_max_roads_for_scenario(config->scenario);
    manager->graph = graph_create(1920, 1080, 40, 0, max_roads);
    if(manager->graph == NULL) {
        fprintf(stderr, "Graph initialization failed!\n");
        traffic_manager_clear(manager);
        return -1;
    }

    if(traffic_manager_build_roads(manager, config->scenario, config->lane_count) != 0) {
        fprintf(stderr, "Road generation failed!\n");
        traffic_manager_clear(manager);
        return -1;
    }

    manager->lane_list_count = manager->graph->road_count * config->lane_count;
    manager->lane_lists = (LaneCarList*)malloc(sizeof(LaneCarList) * manager->lane_list_count);
    if(traffic_manager_init_lane_lists(manager) != 0) {
        fprintf(stderr, "Lane lists initialization failed!\n");
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

static const Car* traffic_manager_find_front_car(TrafficManager* manager, const Car* car, float search_radius) {
    if(manager == NULL || car == NULL) {
        return NULL;
    }                                            
}

static bool traffic_manager_try_start_lane_change(TrafficManager* manager, Car* car) {

}

static bool traffic_manager_update_lane_change(TrafficManager* manager, Car* car) {
    if(manager == NULL || manager->graph == NULL || car == NULL) {
        return false;
    }

    if (car->road_id < 0 || car->road_id >= manager->graph->road_count) {
        return false;
    }

    if(car->state == CAR_STATE_NORMAL && car->target_lane == -1 && !car->at_intersection) {
        RoadSegment *road = &manager->graph->roads[car->road_id];
        RoadDirection current_dir = graph_get_lane_direction(road, car->lane);

        int left_lane = car->lane > 0 ? car->lane - 1 : -1;
        int right_lane = car->lane < road->lanes - 1 ? car->lane + 1 : -1;
        int desired_lane = -1;

        if(left_lane >= 0 && graph_get_lane_direction(road, left_lane) != current_dir) {
            left_lane = -1;
        }

        if(right_lane >= 0 && graph_get_lane_direction(road, right_lane) != current_dir) {
            right_lane = -1;
        }

        if(left_lane >= 0 && right_lane >= 0) {
            desired_lane = (rand() % 2 == 0) ? left_lane : right_lane;
        } else if(left_lane >= 0) {
            desired_lane = left_lane;
        } else {
            desired_lane = right_lane;
        }

        if(desired_lane >= 0) {
            car_start_lane_change(car, desired_lane);
            return true;
        } 
    }

    return false;
}

int traffic_manager_update(TrafficManager *manager, float dt) {
    if (manager == NULL || manager->graph == NULL) {
        return -1;
    }

    for (int i = 0; i < manager->car_count; i++) {
        traffic_manager_update_lane_change(manager, &manager->cars[i]);

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


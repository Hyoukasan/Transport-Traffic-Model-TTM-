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


static int traffic_manager_init_lane_lists(TrafficManager* manager);
static void traffic_manager_update_lane_lists(TrafficManager* manager);
static LaneCarList* traffic_manager_get_lane_list(TrafficManager* manager, int road_id, int lane);

static void traffic_manager_load_car_textures(TrafficManager* manager);
static void traffic_manager_load_light_textures(TrafficManager* manager);
static bool traffic_manager_spawn_car(TrafficManager* manager, int sequence_index);
static bool traffic_manager_spawn_car_on_lane(TrafficManager* manager, int road_id, int lane, float travel_fraction, bool require_clear_area);

static const Car* traffic_manager_find_front_car(TrafficManager* manager, const Car* car, float search_radius);
static bool traffic_manager_update_lane_change(TrafficManager* manager, Car* car, float dt);

static int traffic_manager_init_lights(TrafficManager *manager);
static void traffic_manager_update_lights(TrafficManager *manager, float dt);
static void traffic_light_advance(TrafficLight *light);

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

static void traffic_manager_load_light_textures(TrafficManager* manager) {
    manager->light_textures[LIGHT_RED] =
        texture_load("data/textures/light_red.png", NULL, NULL);

    manager->light_textures[LIGHT_GREEN] =
        texture_load("data/textures/light_green.png", NULL, NULL);
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

static int traffic_manager_init_lights(TrafficManager *manager) {
    if(manager->lights == NULL) {
        return -1;
    }

    for(size_t i = 0; i < (size_t)(manager->light_count); i++) {
        const Intersection *intersection = &manager->graph->intersections[i];

        manager->lights[i].horizontal_state_light = LIGHT_RED;
        manager->lights[i].vertical_state_light   = LIGHT_GREEN;

        manager->lights[i].intersection_x = intersection->x;
        manager->lights[i].intersection_y = intersection->y;

        manager->lights[i].timer = 0.0f;
    
    }

    return 0;
}

static void traffic_manager_update_lights(TrafficManager *manager, float dt) {
    const float switch_time = 5.0f;

    for(size_t i = 0; i < (size_t)(manager->light_count); i++) {
        TrafficLight *light = &manager->lights[i];

        light->timer += dt;

        if(light->timer >= switch_time) {
            traffic_light_advance(light);
        }
    }
}

static void traffic_light_advance(TrafficLight *light) {
    if(light->horizontal_state_light == LIGHT_GREEN) {
        light->horizontal_state_light = LIGHT_RED;
        light->vertical_state_light = LIGHT_GREEN;
    } else {
        light->horizontal_state_light = LIGHT_GREEN;
        light->vertical_state_light = LIGHT_RED;
    }

    light->timer = 0.0f;
}

static float traffic_manager_random_spawn_delay(void) {
    return 2.0f + (float)(rand() % 250) / 100.0f;
}

static RoadDirection traffic_manager_spawn_direction(const RoadSegment *road, int lane) {
    if (road == NULL) {
        return ROAD_DIR_NONE;
    }

    if (road->direction != ROAD_DIR_NONE) {
        return road->direction;
    }

    int lanes = road->lanes > 0 ? road->lanes : 1;
    int half = lanes / 2;
    if (road->type == ROAD_HORIZONTAL) {
        return lane < half ? ROAD_DIR_WEST : ROAD_DIR_EAST;
    }
    if (road->type == ROAD_VERTICAL) {
        return lane < half ? ROAD_DIR_SOUTH : ROAD_DIR_NORTH;
    }

    return ROAD_DIR_NONE;
}

static bool traffic_manager_direction_aligned_with_road(const RoadSegment *road, RoadDirection direction) {
    if (road == NULL) {
        return true;
    }

    if (road->type == ROAD_HORIZONTAL) {
        if (road->x2 >= road->x1) {
            return direction == ROAD_DIR_EAST;
        }
        return direction == ROAD_DIR_WEST;
    }

    if (road->type == ROAD_VERTICAL) {
        if (road->y2 >= road->y1) {
            return direction == ROAD_DIR_SOUTH;
        }
        return direction == ROAD_DIR_NORTH;
    }

    return true;
}

static float traffic_manager_position_to_travel_fraction(const RoadSegment *road, RoadDirection direction, float position) {
    if (traffic_manager_direction_aligned_with_road(road, direction)) {
        return position;
    }
    return 1.0f - position;
}

static float traffic_manager_travel_fraction_to_position(const RoadSegment *road, RoadDirection direction, float travel_fraction) {
    if (traffic_manager_direction_aligned_with_road(road, direction)) {
        return travel_fraction;
    }
    return 1.0f - travel_fraction;
}

static bool traffic_manager_spawn_area_clear(TrafficManager* manager, int road_id, int lane, float spawn_fraction, float min_gap) {
    if (manager->graph == NULL || road_id < 0 || road_id >= manager->graph->road_count) {
        return false;
    }

    const RoadSegment *road = &manager->graph->roads[road_id];
    RoadDirection direction = graph_get_lane_direction(road, lane);

    for (int i = 0; i < manager->car_count; i++) {
        const Car *car = &manager->cars[i];
        if (car->road_id != road_id || car->lane != lane) {
            continue;
        }

        float travel_fraction = traffic_manager_position_to_travel_fraction(road, direction, car->position);
        float gap = travel_fraction - spawn_fraction;
        if (gap < 0.0f) {
            gap = -gap;
        }

        if (gap < min_gap) {
            return false;
        }
    }

    return true;
}

static int traffic_manager_road_start_edge(const RoadSegment *road) {
    int lanes = road->lanes > 0 ? road->lanes : 1;

    if (road->type == ROAD_HORIZONTAL) {
        if (road->direction == ROAD_DIR_EAST) {
            return road->y1;
        } else if (road->direction == ROAD_DIR_WEST) {
            return road->y1 - (lanes - 1);
        }
        return road->y1 - (lanes / 2);
    }

    if (road->type == ROAD_VERTICAL) {
        if (road->direction == ROAD_DIR_NORTH) {
            return road->x1;
        } else if (road->direction == ROAD_DIR_SOUTH) {
            return road->x1 - (lanes - 1);
        }
        return road->x1 - (lanes / 2);
    }

    return road->x1 - (lanes / 2);
}

static bool traffic_manager_spawn_car_on_lane(TrafficManager* manager, int road_id, int lane, float travel_fraction, bool require_clear_area) {
    if (manager->graph == NULL || manager->graph->road_count <= 0 || manager->cars == NULL) {
        return false;
    }

    if (manager->car_count >= manager->max_cars) {
        return false;
    }

    if (road_id < 0 || road_id >= manager->graph->road_count) {
        return false;
    }

    RoadSegment *road = &manager->graph->roads[road_id];
    int lanes = road->lanes > 0 ? road->lanes : 1;
    if (lane < 0 || lane >= lanes) {
        return false;
    }

    float speed_factor = 0.6f + (float)(rand() % 41) / 100.0f;
    float desired_speed = road->speed_limit * speed_factor;
    RoadDirection spawn_direction = traffic_manager_spawn_direction(road, lane);

    if (require_clear_area && !traffic_manager_spawn_area_clear(manager, road_id, lane, travel_fraction, 0.10f)) {
        return false;
    }

    Car* car = &manager->cars[manager->car_count];
    car_init(car, manager->next_car_id++, road_id, desired_speed, 1.0f, lane, 0.0f);

    car->position = traffic_manager_travel_fraction_to_position(road, spawn_direction, travel_fraction);
    car->angle = direction_to_angle(spawn_direction);

    CarColor color = (CarColor)(rand() % 5);
    car->color = color;
    car_set_texture(car, manager->car_textures[color]);

    manager->car_count++;
    return true;
}

static bool traffic_manager_spawn_car(TrafficManager* manager, int sequence_index) {
    if (manager->graph == NULL || manager->graph->road_count <= 0) {
        return false;
    }

    int roads = manager->graph->road_count;
    int road_id = sequence_index >= 0 ? sequence_index % roads : rand() % roads;
    RoadSegment *road = &manager->graph->roads[road_id];
    int lanes = road->lanes > 0 ? road->lanes : 1;
    int lane = sequence_index >= 0 ? sequence_index % lanes : rand() % lanes;
    float travel_fraction = 0.005f;

    if (sequence_index >= 0) {
        int road_car_index = sequence_index / roads;
        travel_fraction += (float)road_car_index * 0.08f;
        if (travel_fraction > 0.45f) {
            travel_fraction = 0.45f;
        }
    }

    return traffic_manager_spawn_car_on_lane(manager, road_id, lane, travel_fraction, true);
}

static bool traffic_manager_car_finished(TrafficManager* manager, const Car* car) {
    if (manager->graph == NULL || car == NULL) {
        return false;
    }

    if (car->state == CAR_STATE_TURNING || car->road_id < 0 || car->road_id >= manager->graph->road_count) {
        return false;
    }

    const RoadSegment *road = &manager->graph->roads[car->road_id];
    RoadDirection direction = graph_get_lane_direction(road, car->lane);
    float travel_fraction = traffic_manager_position_to_travel_fraction(road, direction, car->position);

    return travel_fraction >= 0.995f;
}

static void traffic_manager_remove_car(TrafficManager* manager, int index) {
    if (index < 0 || index >= manager->car_count) {
        return;
    }

    car_destroy(&manager->cars[index]);

    if (index != manager->car_count - 1) {
        manager->cars[index] = manager->cars[manager->car_count - 1];
        car_destroy(&manager->cars[manager->car_count - 1]);
    }
    manager->car_count--;
}

static void traffic_manager_spawn_cars(TrafficManager* manager, const ConfigManager* config) {
    if (manager->graph == NULL || config == NULL || manager->graph->road_count <= 0) {
        return;
    }

    int total_cars = config->max_cars > 0 ? config->max_cars : 0;
    if (total_cars > manager->max_cars) {
        total_cars = manager->max_cars;
    }
    total_cars = (total_cars + 1) / 2;

    for (int i = 0; i < total_cars; i++) {
        traffic_manager_spawn_car(manager, i);
    }
}

static int traffic_manager_init_lane_lists(TrafficManager* manager) {
    if(manager->lane_lists == NULL) {
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

static LaneCarList* traffic_manager_get_lane_list(TrafficManager* manager, int road_id, int lane) {
    if (manager->lane_lists == NULL) {
        return NULL;
    }

    for(int i = 0; i < manager->lane_list_count; i++) {
        LaneCarList *list = &manager->lane_lists[i];

        if (list->road_id == road_id && list->lane == lane) {
            return list;
        }
    }

    return NULL;
}

static void traffic_manager_update_lane_lists(TrafficManager* manager) {
    if (manager->lane_lists == NULL) {
        return;
    }

    for (size_t i = 0; i < (size_t)manager->lane_list_count; i++) {
        manager->lane_lists[i].car_count = 0;
    }

    for (size_t i = 0; i < (size_t)manager->car_count; i++) {
        Car *car = &manager->cars[i];

        LaneCarList *list = traffic_manager_get_lane_list(manager, car->road_id, car->lane);
        if (list == NULL) {
            continue;
        }

        if (list->car_count < manager->max_cars) {
            list->car_indices[list->car_count++] = (int)i;
        }
    }
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
    manager->lane_lists = (LaneCarList*)calloc((size_t)manager->lane_list_count, sizeof(LaneCarList));
    if(traffic_manager_init_lane_lists(manager) != 0) {
        fprintf(stderr, "Lane lists initialization failed!\n");
        traffic_manager_clear(manager);
        return -1;
    }
    
    manager->light_count = manager->graph->intersection_count;

    if(manager->light_count > 0) {
        manager->lights = (TrafficLight*)calloc((size_t)manager->light_count, sizeof(TrafficLight));
        if(traffic_manager_init_lights(manager) != 0) {
            fprintf(stderr, "Lights initialization failed!\n");
            traffic_manager_clear(manager);
            return -1;
        }
    }

    manager->selected_lane    = -1;
    manager->selected_road_id = -1;
    manager->car_count = 0;
    manager->accident_count = 0;
    manager->time = 0.0f;
    manager->spawn_timer = traffic_manager_random_spawn_delay();
    manager->manual_spawn_cooldown = 0.0f;
    manager->next_car_id = 0;

    traffic_manager_load_car_textures(manager);
    traffic_manager_load_light_textures(manager);
    traffic_manager_spawn_cars(manager, config);
    traffic_manager_update_lane_lists(manager);

    return 0;
}

static const Car* traffic_manager_find_front_car(TrafficManager* manager, const Car* car, float search_radius) {
    if(manager->graph == NULL || car == NULL) {
        return NULL;
    }

    if(car->road_id < 0 || car->road_id >= manager->graph->road_count) {
        return NULL;
    }
    
    LaneCarList *list = traffic_manager_get_lane_list(manager, car->road_id, car->lane);
    if(list == NULL) {
        return NULL;
    }

    if(list->car_count <= 1) {
        return NULL;
    }

    RoadSegment *road = &manager->graph->roads[car->road_id];
    RoadDirection dir = graph_get_lane_direction(road, car->lane);

    const Car *front_car = NULL;
    float min_distance = search_radius;

    for(size_t i = 0; i < (size_t)list->car_count; i++) {
        int car_index = list->car_indices[i];
        Car *other_car = &manager->cars[car_index];

        if (other_car == car) {
            continue;
        }

        float distance;

        if(dir == ROAD_DIR_EAST || dir == ROAD_DIR_SOUTH) {
            distance = other_car->position - car->position;
        } else {
            distance = car->position - other_car->position;
        }

        distance *= (float)road->length;

        if (distance > 0.0f && distance < min_distance) {
            min_distance = distance;
            front_car = other_car;
        }
    }

    return front_car;
}

static bool traffic_manager_update_lane_change(TrafficManager* manager, Car* car, float dt) {
    if(manager->graph == NULL || car == NULL) {
        return false;
    }

    if (car->road_id < 0 || car->road_id >= manager->graph->road_count) {
        return false;
    }

    if(car->state == CAR_STATE_NORMAL && car->target_lane == -1 && !car->at_intersection) {
        car->lane_change_timer -= dt;
        if (car->lane_change_timer > 0.0f) {
            return false;
        }

        const Car* front_car = traffic_manager_find_front_car(manager, car, 2.0f);
        if(front_car == NULL) {
            car->lane_change_timer = (float)(rand() % 121 + 120) / 60.0f;
            return false;
        }

        if (front_car->speed >= car->speed) {
            car->lane_change_timer = (float)(rand() % 121 + 120) / 60.0f;
            return false;
        }

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
            car->lane_change_timer = (float)(rand() % 121 + 120) / 60.0f;
            return true;
        } 

        car->lane_change_timer = (float)(rand() % 121 + 120) / 60.0f;
    }

    return false;
}

bool traffic_manager_select_lane_at_pixel(TrafficManager* manager, int mouse_x, int mouse_y) {
    if(manager == NULL || manager->graph == NULL) {
        return false;
    }

    float grid_x = (float)mouse_x / (float)manager->graph->chunk_size - (float)manager->graph->padding;
    float grid_y = (float)mouse_y / (float)manager->graph->chunk_size - (float)manager->graph->padding;

    for(size_t i = 0; i < (size_t)manager->graph->road_count; i++) {
        const RoadSegment* road = &manager->graph->roads[i];
        int lanes = road->lanes > 0 ? road->lanes : 1;

        if (road->type == ROAD_HORIZONTAL) {
            int min_x = coord_min(road->x1, road->x2);
            int max_x = coord_max(road->x1, road->x2);
            int start_y = traffic_manager_road_start_edge(road);

            if (grid_x >= min_x && grid_x <= max_x + 1 &&
                grid_y >= start_y && grid_y < start_y + lanes) {
                manager->selected_road_id = road->id;
                manager->selected_lane = (int)(grid_y - start_y);
                return true;
            }
        }

        if (road->type == ROAD_VERTICAL) {
            int min_y = coord_min(road->y1, road->y2);
            int max_y = coord_max(road->y1, road->y2);
            int start_x = traffic_manager_road_start_edge(road);

            if (grid_y >= min_y && grid_y <= max_y + 1 &&
                grid_x >= start_x && grid_x < start_x + lanes) {
                manager->selected_road_id = road->id;
                manager->selected_lane = (int)(grid_x - start_x);
                return true;
            }
        }
    }

    return false;
}

bool traffic_manager_spawn_car_on_selected_lane(TrafficManager* manager) {
    if (manager == NULL) {
        return false;
    }

    if (manager->selected_road_id < 0 || manager->selected_lane < 0) {
        return false;
    }

    if (manager->manual_spawn_cooldown > 0.0f) {
        return false;
    }

    if (!traffic_manager_spawn_car_on_lane(manager, manager->selected_road_id, manager->selected_lane, 0.005f, false)) {
        return false;
    }

    manager->manual_spawn_cooldown = 2.0f;
    traffic_manager_update_lane_lists(manager);
    return true;
}

bool traffic_manager_add_accident_on_selected_lane(TrafficManager* manager) {
    if(manager == NULL) {
        return false;
    }

    int road_id = manager->selected_road_id;
    int road_lane_id = manager->selected_lane;
    if (road_id < 0 || road_lane_id < 0) {
        return false;
    }    

    LaneCarList* list = traffic_manager_get_lane_list(manager, road_id, road_lane_id);
    if(list == NULL || list->car_count < 2) {
        return false;
    }

    return true;
}

int traffic_manager_update(TrafficManager *manager, float dt) {
    if (manager == NULL || manager->graph == NULL) {
        return -1;
    }

    traffic_manager_update_lights(manager, dt);

    if (manager->manual_spawn_cooldown > 0.0f) {
        manager->manual_spawn_cooldown -= dt;
        if (manager->manual_spawn_cooldown < 0.0f) {
            manager->manual_spawn_cooldown = 0.0f;
        }
    }
    
    traffic_manager_update_lane_lists(manager);

    for (int i = 0; i < manager->car_count; i++) {
        if(traffic_manager_update_lane_change(manager, &manager->cars[i], dt)) {
            
        }

        car_update(&manager->cars[i], manager->graph, dt);
    }

    for (int i = manager->car_count - 1; i >= 0; i--) {
        if (traffic_manager_car_finished(manager, &manager->cars[i])) {
            traffic_manager_remove_car(manager, i);
        }
    }

    manager->spawn_timer -= dt;
    if (manager->spawn_timer <= 0.0f) {
        if (traffic_manager_spawn_car(manager, -1)) {
            manager->spawn_timer = traffic_manager_random_spawn_delay();
        } else {
            manager->spawn_timer = 0.25f;
        }
    }

    traffic_manager_update_lane_lists(manager);

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
    manager->accident_count = 0;
    manager->max_accidents = 0;
    manager->time = 0.0f;
    manager->spawn_timer = 0.0f;
    manager->manual_spawn_cooldown = 0.0f;
    manager->next_car_id = 0;
    manager->selected_lane    = -1;
    manager->selected_road_id = -1;
}

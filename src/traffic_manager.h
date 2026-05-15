#ifndef TRAFFIC_MANAGER_H
#define TRAFFIC_MANAGER_H

#include <stdbool.h>

struct Car;
struct Graph;
struct ConfigManager;

typedef enum {
    LIGHT_RED,
    LIGHT_GREEN
} LightState;

typedef struct {
    int        intersection_x;
    int        intersection_y;

    LightState horizontal_state_light;
    LightState vertical_state_light;

    float      timer;
} TrafficLight;

typedef struct {
    int   road_id;
    int   lane;
    float clear_timer;
    int   released_cars;
    bool  active;
} AccidentDTP;

typedef struct {
    int road_id;
    int lane;
    int car_count;
    int *car_indices;
} LaneCarList;

typedef struct TrafficManager { 
    struct Car          *cars;
    unsigned int        car_textures[5];
    unsigned int        light_textures[2];
    int                 car_count;
    int                 max_cars;

    LaneCarList         *lane_lists;
    int                 lane_list_count;

    TrafficLight        *lights;
    int                 light_count;

    AccidentDTP         *accidents;
    int                 accident_count;
    int                 max_accidents;

    struct Graph       *graph;

    float               time;
    float               spawn_timer;
    float               manual_spawn_cooldown;
    int                 next_car_id;
    int                 selected_road_id;
    int                 selected_lane;
} TrafficManager;

int traffic_manager_init(TrafficManager* manager, const struct ConfigManager* config);
void traffic_manager_clear(TrafficManager* manager);
int traffic_manager_update(TrafficManager* manager, float dt);
const struct Graph* traffic_manager_get_graph(const TrafficManager* manager);
const struct Car* traffic_manager_get_cars(const TrafficManager* manager, int* out_count);
bool traffic_manager_select_lane_at_pixel(TrafficManager* manager, int mouse_x, int mouse_y);
bool traffic_manager_spawn_car_on_selected_lane(TrafficManager* manager);
bool traffic_manager_add_accident_on_selected_lane(TrafficManager* manager);
bool traffic_manager_selected_lane_has_accident(const TrafficManager* manager);



#endif

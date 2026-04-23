#ifndef TRAFFIC_MANAGER_H
#define TRAFFIC_MANAGER_H

#include <stdbool.h>

struct Car;
struct Graph;
struct TrafficConfig;

typedef enum {
    LIGHT_RED,
    LIGHT_YELLOW,
    LIGHT_GREEN
} LightState;

typedef struct {
    int        intersection_x;
    int        intersection_y;
    LightState state_light;
    float      timer;
} TrafficLight;

typedef struct {
    int   road_id;
    float position;
    int   lane;
    float clear_timer;
    bool  active;
} AccidentDTP;

typedef struct {
    Car          *cars;
    int          car_count;
    int          max_cars;

    TrafficLight *lights;
    int           light_count;
    int           max_lights;

    AccidentDTP *accidents;
    int          accident_count;
    int          max_accidents;

    Graph       *graph;

    float        time;
    int          next_car_id;
} TrafficManager;

int traffic_manager_init(TrafficManager* manager, const struct TrafficConfig* config);
void traffic_manager_clear(TrafficManager *manager);

#endif

#include <stdlib.h>

#include "traffic_manager.h"
#include "traffic_config.h"
#include "car.h"
#include "graph.h"
#include "renderer.h"



TrafficManager* traffic_manager_init(TrafficManager* name_space, const TrafficConfig* config) {
    name_space = (TrafficManager*)malloc(sizeof(TrafficManager));
    if(name_space == NULL || config == NULL) {
        return NULL;
    }

    name_space->max_cars = config->max_cars;

    return name_space;
}
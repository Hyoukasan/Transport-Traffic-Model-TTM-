#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "traffic_manager.h"
#include "traffic_config.h"
#include "car.h"
#include "graph.h"
#include "renderer.h"



int traffic_manager_init(TrafficManager* manager, const TrafficConfig* config) {
    if(config == NULL) {
        fprintf(stderr, "Config undefined!\n");
        return -1;
    }
    
    
    manager = (TrafficManager*)calloc(1,sizeof(TrafficManager));
    if(manager == NULL) {
        fprintf(stderr, "Manager initialization failed!\n");
        return -1;
    }

    manager->max_cars = config->max_cars;
    manager->cars = (Car*)malloc((size_t)manager->max_cars * sizeof(Car));
    if(manager->cars == NULL) {
        fprintf(stderr, "Cars initialization failed!\n");
        free(manager);
        return -1;
    }
    
    manager->accident_count = 16;
    manager->accidents = (AccidentDTP*)malloc((size_t)manager->accident_count * sizeof(AccidentDTP));
    if(manager->accidents == NULL) {
        fprintf(stderr, "Accidents initialization failed!\n");
        free(manager->cars);
        free(manager);
        return -1;
    }

    manager->graph = graph_create(1920, 1080, 50, 1);
    if(manager->graph == NULL) {
        fprintf(stderr, "Graph initialization failed!\n");
        free(manager->cars);
        free(manager->accidents);
        free(manager);
        return -1;
    }

    manager->car_count = 0;
    manager->accident_count = 0;
    manager->light_count = 0;
    manager->time = 0.0f;
    manager->next_car_id = 0; 

    return 0;
}
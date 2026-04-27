#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "traffic_manager.h"
#include "traffic_config.h"
#include "road_generator.h"
#include "car.h"
#include "graph.h"

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

    manager->graph = graph_init(1920, 1080, 50, 0);
    if(manager->graph == NULL) {
        fprintf(stderr, "Graph initialization failed!\n");
        traffic_manager_clear(manager);
        return -1;
    }



    manager->car_count = 0;
    manager->accident_count = 0;
    manager->light_count = 0;
    manager->time = 0.0f;
    manager->next_car_id = 0; 

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
        for(size_t i = 0; i < manager->car_count; i++) {
            car_destroy(manager->cars + i * sizeof(Car));
        }
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





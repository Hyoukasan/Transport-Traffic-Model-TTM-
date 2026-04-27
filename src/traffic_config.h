#ifndef TRAFFIC_CONFIG_H
#define TRAFFIC_CONFIG_H

#include <stdbool.h>

typedef enum {
    SCENARIO_HIGHWAY,
    SCENARIO_SINGLE_INTERSECTION,
    SCENARIO_MULTI_INTERSECTION
} ScenarioType;

typedef struct TrafficConfig {
    ScenarioType scenario;  

    int lane_count;            
    int max_cars;
    int max_roads;                    
 
} TrafficConfig;

#endif
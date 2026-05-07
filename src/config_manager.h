#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <stdbool.h>

typedef enum {
    SCENARIO_HIGHWAY = 1,
    SCENARIO_SINGLE_INTERSECTION,
    SCENARIO_MULTI_INTERSECTION
} ScenarioType;

typedef struct ConfigManager {
    ScenarioType scenario;  

    int lane_count;            
    int max_cars;
    int max_roads;                    
 
} ConfigManager;

#endif

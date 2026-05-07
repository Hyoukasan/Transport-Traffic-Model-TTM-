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
} ConfigManager;

int config_manager_save_profile(const ConfigManager* config, int slot, float time);
bool config_manager_is_empty_slot(int slot);
int config_manager_load_profile(ConfigManager* config, int slot, float* out_time);

#endif

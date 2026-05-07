#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "config_manager.h"

static void config_manager_get_slot_path(int slot, char* buffer, int buffer_size) {
    snprintf(buffer, buffer_size, "../data/profiles/profile_%d.cfg", slot);
}

int config_manager_save_profile(const ConfigManager* config, int slot, float time) {
    if (config == NULL) {
        fprintf(stderr, "Not created config\n");
        return -1;
    }

    if (slot < 1 || slot > 4) {
        fprintf(stderr, "Not found current slot\n");
        return -1;
    }

    char filename[64];
    config_manager_get_slot_path(slot, filename, sizeof(filename));

    FILE* f = fopen(filename, "w");
    if(f == NULL) {
        fprintf(stderr, "Error in created file\n");
        return -1;
    }

    fprintf(f, "profile %d\n", slot);
    fprintf(f, "scenario %d\n", config->scenario);
    fprintf(f, "lane_count %d\n", config->lane_count);
    fprintf(f, "max_cars %d\n", config->max_cars);
    fprintf(f, "time %.2f\n", time);

    fclose(f);
    return 0;
}   

bool config_manager_is_empty_slot(int slot) {
    if (slot < 1 || slot > 4) {
        return true;
    }

    char filename[64];
    config_manager_get_slot_path(slot, filename, sizeof(filename));

    FILE* f = fopen(filename, "r");
    if(f == NULL) {
        return true;
    }

    fseek(f, 0, SEEK_END);
    long size_file = ftell(f);
    fclose(f);

    return size_file <= 0;
}

int config_manager_load_profile(ConfigManager* config, int slot, float* out_time) {
    if (slot < 1 || slot > 4) {
        fprintf(stderr, "Not found current slot\n");
        return -1;
    }    

    if(config_manager_is_empty_slot(slot)) {
        fprintf(stderr, "Not found profile\n");
        return -1;
    }

    char filename[64];
    config_manager_get_slot_path(slot, filename, sizeof(filename));
    
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        return -1;
    }
    
    char key[32];
    int scenario = 0;
    int lane_count = 0;
    int max_cars = 0;
    float time = 0.0f;

    while(fscanf(f, "%31s", key) == 1) {
        if(strcmp(key, "scenario") == 0) {
            fscanf(f, "%d", &scenario);
        } else if(strcmp(key, "lane_count") == 0) {
            fscanf(f, "%d", &lane_count);
        } else if(strcmp(key, "max_cars") == 0) {
            fscanf(f, "%d", &max_cars);
        } else if(strcmp(key, "time") == 0) {
            fscanf(f, "%f", &time);
        }
    }

    fclose(f);

    config->scenario = (ScenarioType)scenario;
    config->lane_count = lane_count;
    config->max_cars = max_cars;

    if (out_time != NULL) {
        *out_time = time;
    }

    return 0;
}

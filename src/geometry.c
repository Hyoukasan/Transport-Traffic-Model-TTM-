#include "geometry.h"
#include "graph.h" 

float pixel_to_normalized_x(int px, int width) {
    return (2.0f * px / width) - 1.0f;
}

float pixel_to_normalized_y(int py, int height) {
    return 1.0f - (2.0f * py / height);
}

float grid_center_to_normalized_x(int grid_x, int chunk_size, int padding, int width) {
    float pixel = (grid_x + padding) * chunk_size + chunk_size * 0.5f;
    return (2.0f * pixel / width) - 1.0f;
}

float grid_center_to_normalized_y(int grid_y, int chunk_size, int padding, int height) {
    float pixel = (grid_y + padding) * chunk_size + chunk_size * 0.5f;
    return 1.0f - (2.0f * pixel / height);
}

float grid_edge_to_normalized_x(int grid_x, int chunk_size, int padding, int width) {
    float pixel = (grid_x + padding) * chunk_size;
    return (2.0f * pixel / width) - 1.0f;
}

float grid_edge_to_normalized_y(int grid_y, int chunk_size, int padding, int height) {
    float pixel = (grid_y + padding) * chunk_size;
    return 1.0f - (2.0f * pixel / height);
}

int coord_min(int a, int b) {
    return a < b ? a : b;
}

int coord_max(int a, int b) {
    return a > b ? a : b;
}

bool point_in_range(int value, int a, int b) {
    int min = (a < b) ? a : b;
    int max = (a > b) ? a : b;

    return value >= min && value <= max;
}

// Новые функции для манёвров
float direction_to_angle(RoadDirection dir) {
    switch (dir) {
        case ROAD_DIR_EAST: return 90.0f;
        case ROAD_DIR_WEST: return -90.0f;
        case ROAD_DIR_NORTH: return 180.0f;
        case ROAD_DIR_SOUTH: return 0.0f;
        default: return 0.0f;
    }
}

static float normalize_angle(float angle) {
    while (angle > 180.0f) angle -= 360.0f;
    while (angle <= -180.0f) angle += 360.0f;
    return angle;
}

float lerp_angle(float a, float b, float t) {
    float diff = normalize_angle(b - a);
    return a + diff * t;
}

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float smoothstep(float t) {
    return t * t * (3.0f - 2.0f * t);
}

void road_get_basis(RoadDirection dir, int *dir_x, int *dir_y, int *right_x, int *right_y) {
    switch (dir) {
        case ROAD_DIR_EAST:  *dir_x=1; *dir_y=0; *right_x=0; *right_y=1; break;
        case ROAD_DIR_WEST:  *dir_x=-1; *dir_y=0; *right_x=0; *right_y=-1; break;
        case ROAD_DIR_SOUTH: *dir_x=0; *dir_y=1; *right_x=-1; *right_y=0; break;
        case ROAD_DIR_NORTH: *dir_x=0; *dir_y=-1; *right_x=1; *right_y=0; break;
        default: *dir_x=0; *dir_y=0; *right_x=0; *right_y=0; break;
    }
}

void road_local_to_grid(float start_x, float start_y, RoadDirection dir, float s, float d, float *out_x, float *out_y) {
    int dir_x, dir_y, right_x, right_y;
    road_get_basis(dir, &dir_x, &dir_y, &right_x, &right_y);
    *out_x = start_x + dir_x * s + right_x * d;
    *out_y = start_y + dir_y * s + right_y * d;
}

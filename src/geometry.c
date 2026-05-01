#include "geometry.h"

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
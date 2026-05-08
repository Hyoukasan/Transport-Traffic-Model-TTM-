#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "graph.h"  // для RoadDirection

float pixel_to_normalized_x(int px, int width);
float pixel_to_normalized_y(int py, int height);
float grid_center_to_normalized_x(int grid_x, int chunk_size, int padding, int width);
float grid_center_to_normalized_y(int grid_y, int chunk_size, int padding, int height);
float grid_edge_to_normalized_x(int grid_x, int chunk_size, int padding, int width);
float grid_edge_to_normalized_y(int grid_y, int chunk_size, int padding, int height);
int coord_min(int a, int b);
int coord_max(int a, int b);

// Новые функции для манёвров
float direction_to_angle(RoadDirection dir);
float lerp_angle(float a, float b, float t);
float approach_angle(float current, float target, float speed, float dt);
float lerp(float a, float b, float t);
float smoothstep(float t);
void road_get_basis(RoadDirection dir, int *dir_x, int *dir_y, int *right_x, int *right_y);
void road_local_to_grid(float start_x, float start_y, RoadDirection dir, float s, float d, float *out_x, float *out_y);

#endif 
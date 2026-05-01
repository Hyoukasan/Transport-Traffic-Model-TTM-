#ifndef GEOMETRY_H
#define GEOMETRY_H

float pixel_to_normalized_x(int px, int width);
float pixel_to_normalized_y(int py, int height);
float grid_center_to_normalized_x(int grid_x, int chunk_size, int padding, int width);
float grid_center_to_normalized_y(int grid_y, int chunk_size, int padding, int height);
float grid_edge_to_normalized_x(int grid_x, int chunk_size, int padding, int width);
float grid_edge_to_normalized_y(int grid_y, int chunk_size, int padding, int height);
int coord_min(int a, int b);
int coord_max(int a, int b);

#endif 
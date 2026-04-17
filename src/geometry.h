#ifndef GEOMETRY_H
#define GEOMETRY_H

float pixel_to_normalized_x(int px, int width);
float pixel_to_normalized_y(int py, int height);
float grid_to_normalized_x(int grid_x, int chunk_size, int padding, int width);
float grid_to_normalized_y(int grid_y, int chunk_size, int padding, int height);

#endif 
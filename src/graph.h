#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

typedef enum {
    ROAD_HORIZONTAL,
    ROAD_VERTICAL,
    ROAD_DIAGONAL
} RoadType;

typedef struct {
    int id;
    int x1;
    int y1;
    int x2;
    int y2;
    RoadType type;
    int length;        // длина в ячейках
    float speed_limit; // скорость на дороге
    bool accident;     // авария на сегменте
    int lanes;
} RoadSegment;

typedef struct {
    RoadSegment *roads;
    int road_count;
    int max_roads;

    int grid_width;
    int grid_height;
    int chunk_size;   // размер чанка в пикселях

    int window_width;
    int window_height;
    int padding;      // отступ от края окна в чанках
} Graph;

Graph* graph_create(int window_width, int window_height, int chunk_size, int padding);
int graph_add_road(Graph *g, int x1, int y1, int x2, int y2, RoadType type, float speed_limit);
void graph_destroy(Graph *g);

#endif

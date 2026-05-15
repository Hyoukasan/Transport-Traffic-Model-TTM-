#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

typedef enum RoadType {
    ROAD_HORIZONTAL,
    ROAD_VERTICAL
} RoadType;

typedef enum RoadDirection {
    ROAD_DIR_NONE,
    ROAD_DIR_EAST,
    ROAD_DIR_WEST,
    ROAD_DIR_NORTH,
    ROAD_DIR_SOUTH
} RoadDirection;

typedef struct {
    int id;
    int x1;
    int y1;
    int x2;
    int y2;

    RoadType type;
    RoadDirection direction;
    int length;        // длина в ячейках
    float speed_limit; // скорость на дороге
    bool accident;     // авария на сегменте
    int lanes;
    unsigned int texture;
} RoadSegment;

typedef struct {
    int x;
    int y;

    int left_edge;
    int right_edge;
    int top_edge;
    int bottom_edge;

    int road_count;
    int roads[8];
} Intersection;

typedef struct Graph {
    RoadSegment* roads;
    int road_count;
    int max_roads;

    Intersection *intersections;
    int intersection_count;
    int max_intersections;

    int grid_width;
    int grid_height;
    int chunk_size;   // размер чанка в пикселях

    int window_width;
    int window_height;
    int padding;      // отступ от края окна в чанках
} Graph;

Graph* graph_create(int window_width, int window_height, int chunk_size, int padding, int max_roads);
int graph_add_road(Graph *g, int x1, int y1, int x2, int y2, RoadType type, RoadDirection direction, float speed_limit, int lanes);
void graph_set_road_texture(Graph *g, int road_id, unsigned int texture);
void graph_set_road_direction(Graph *g, int road_id, RoadDirection direction);
void graph_build_intersections(Graph *g);
void graph_destroy(Graph *g);

int graph_road_lane_count(const RoadSegment *road);
RoadDirection graph_get_lane_direction(const RoadSegment *road, int lane);

#endif

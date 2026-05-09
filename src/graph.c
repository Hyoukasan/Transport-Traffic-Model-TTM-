#include <stdlib.h>
#include <stdio.h>

#include "graph.h"
#include "texture.h"

#define INITIAL_CAPACITY 50
#define INITIAL_INTERSECTION_CAPACITY 16

static bool graph_expand_roads(Graph *g) {
    if (g == NULL) {
        return false;
    }

    int new_max = g->max_roads * 2;
    RoadSegment *new_roads = realloc(g->roads, sizeof(RoadSegment) * new_max);
    if (new_roads == NULL) {
        fprintf(stderr, "graph_expand_roads: failed to allocate expanded road array\n");
        return false;
    }

    g->roads = new_roads;
    g->max_roads = new_max;
    return true;
}

Graph* graph_create(int window_width, int window_height, int chunk_size, int padding, int max_roads) {
    Graph *g = malloc(sizeof(Graph));
    if (g == NULL) {
        fprintf(stderr, "graph_create: failed to allocate Graph\n");
        return NULL;
    }

    g->window_width = window_width;
    g->window_height = window_height;
    g->chunk_size = chunk_size;
    g->padding = padding;

    g->grid_width = (window_width / chunk_size) - (2 * padding);
    g->grid_height = (window_height / chunk_size) - (2 * padding);
    if (g->grid_width <= 0) {
        g->grid_width = 1;
    }
    if (g->grid_height <= 0) {
        g->grid_height = 1;
    }

    g->max_roads = max_roads > 0 ? max_roads : INITIAL_CAPACITY;
    g->road_count = 0;
    g->roads = malloc(sizeof(RoadSegment) * g->max_roads);
    if (g->roads == NULL) {
        fprintf(stderr, "graph_create: failed to allocate roads array\n");
        free(g);
        return NULL;
    }

    g->max_intersections = INITIAL_INTERSECTION_CAPACITY;
    g->intersection_count = 0;
    g->intersections = malloc(sizeof(Intersection) * g->max_intersections);
    if (g->intersections == NULL) {
        fprintf(stderr, "graph_create: failed to allocate intersections array\n");
        free(g->roads);
        free(g);
        return NULL;
    }

    return g;
}
// текстурка
void graph_set_road_texture(Graph *g, int road_id, unsigned int texture) {
    if (g == NULL || road_id < 0 || road_id >= g->road_count) {
        return;
    }
    g->roads[road_id].texture = texture;
}
//направление
void graph_set_road_direction(Graph *g, int road_id, RoadDirection direction) {
    if (g == NULL || road_id < 0 || road_id >= g->road_count) {
        return;
    }
    g->roads[road_id].direction = direction;
}
// скорость на дороге 
static bool point_in_range(int value, int a, int b) {
    if (a <= b) {
        return value >= a && value <= b;
    }
    return value >= b && value <= a;
}
// перекресток
static bool graph_add_intersection(Graph *g, int x, int y, int road_id) {
    if (g == NULL) {
        return false;
    }

    for (int i = 0; i < g->intersection_count; i++) {
        if (g->intersections[i].x == x && g->intersections[i].y == y) {
            for (int j = 0; j < g->intersections[i].road_count; j++) {
                if (g->intersections[i].roads[j] == road_id) {
                    return true;
                }
            }
            if (g->intersections[i].road_count < 8) {
                g->intersections[i].roads[g->intersections[i].road_count++] = road_id;
            }
            return true;
        }
    }

    if (g->intersection_count >= g->max_intersections) {
        int new_max = g->max_intersections * 2;
        Intersection *new_points = realloc(g->intersections, sizeof(Intersection) * new_max);
        if (new_points == NULL) {
            fprintf(stderr, "graph_add_intersection: failed to allocate expanded intersections array\n");
            return false;
        }
        g->intersections = new_points;
        g->max_intersections = new_max;
    }

    Intersection *intersection = &g->intersections[g->intersection_count];
    intersection->x = x;
    intersection->y = y;
    intersection->road_count = 0;
    intersection->roads[intersection->road_count++] = road_id;
    g->intersection_count++;
    return true;
}
// дорога
int graph_add_road(Graph *g, int x1, int y1, int x2, int y2, RoadType type, RoadDirection direction, float speed_limit, int lanes) {
    if (g == NULL) {
        fprintf(stderr, "graph_add_road: graph is NULL\n");
        return -1;
    }

    if (x1 < 0 || x1 > g->grid_width || x2 < 0 || x2 > g->grid_width ||
        y1 < 0 || y1 > g->grid_height || y2 < 0 || y2 > g->grid_height) {
        fprintf(stderr, "graph_add_road: invalid road bounds (%d,%d)-(%d,%d)\n", x1, y1, x2, y2);
        return -1;
    }

    if (g->road_count >= g->max_roads) {
        if (!graph_expand_roads(g)) {
            return -1;
        }
    }

    RoadSegment *road = &g->roads[g->road_count];
    road->id = g->road_count;
    road->x1 = x1;
    road->y1 = y1;
    road->x2 = x2;
    road->y2 = y2;
    road->type = type;
    road->direction = direction;
    road->length = abs(x2 - x1) + abs(y2 - y1);
    road->speed_limit = speed_limit;
    road->accident = false;
    road->lanes = lanes > 0 ? lanes : 1;
    road->texture = 0;

    return g->road_count++;
}

void graph_build_intersections(Graph *g) {
    if (g == NULL || g->road_count < 2) {
        return;
    }

    g->intersection_count = 0;

    for (int i = 0; i < g->road_count; i++) {
        const RoadSegment *road_a = &g->roads[i];
        for (int j = i + 1; j < g->road_count; j++) {
            const RoadSegment *road_b = &g->roads[j];
            if (road_a->type == ROAD_HORIZONTAL && road_b->type == ROAD_VERTICAL) {
                int x = road_b->x1;
                int y = road_a->y1;
                if (point_in_range(x, road_a->x1, road_a->x2) && point_in_range(y, road_b->y1, road_b->y2)) {
                    graph_add_intersection(g, x, y, road_a->id);
                    graph_add_intersection(g, x, y, road_b->id);
                }
            } else if (road_a->type == ROAD_VERTICAL && road_b->type == ROAD_HORIZONTAL) {
                int x = road_a->x1;
                int y = road_b->y1;
                if (point_in_range(x, road_b->x1, road_b->x2) && point_in_range(y, road_a->y1, road_a->y2)) {
                    graph_add_intersection(g, x, y, road_a->id);
                    graph_add_intersection(g, x, y, road_b->id);
                }
            }
        }
    }
}

void graph_destroy(Graph *g) {
    if (g == NULL) {
        return;
    }

    for (int i = 0; i < g->road_count; i++) {
        if (g->roads[i].texture != 0) {
            texture_delete(g->roads[i].texture);
        }
    }
    free(g->intersections);
    free(g->roads);
    free(g);
}

int graph_road_lane_count(const RoadSegment *road) {
    if (road == NULL) {
        return 0;
    }

    int lanes = road->lanes > 0 ? road->lanes : 2;
    
    return lanes;
}

RoadDirection graph_get_lane_direction(const RoadSegment *road, int lane) {
    if (road == NULL) {
        return ROAD_DIR_NONE;
    }

    if (road->direction != ROAD_DIR_NONE) {
        return road->direction;
    }

    int lanes = road->lanes > 0 ? road->lanes : 2;
    int half = lanes / 2;
    if (road->type == ROAD_HORIZONTAL) {
        return lane < half ? ROAD_DIR_WEST : ROAD_DIR_EAST;
    }
    if (road->type == ROAD_VERTICAL) {
        return lane < half ? ROAD_DIR_SOUTH : ROAD_DIR_NORTH;
    }

    return ROAD_DIR_NONE;
}



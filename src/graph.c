#include "graph.h"
#include <stdlib.h>
#include <stdio.h>

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

Graph* graph_create(int window_width, int window_height, int chunk_size, int padding) {
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

    return g;
}

static bool point_in_range(int value, int a, int b) {
    if (a <= b) {
        return value >= a && value <= b;
    }
    return value >= b && value <= a;
}

static bool graph_add_intersection(Graph *g, int x, int y) {
    if (g == NULL) {
        return false;
    }

    for (int i = 0; i < g->intersection_count; i++) {
        if (g->intersections[i].x == x && g->intersections[i].y == y) {
            return true;
        }
    }

    if (g->intersection_count >= g->max_intersections) {
        int new_max = g->max_intersections * 2;
        GridPoint *new_points = realloc(g->intersections, sizeof(GridPoint) * new_max);
        if (new_points == NULL) {
            fprintf(stderr, "graph_add_intersection: failed to allocate expanded intersections array\n");
            return false;
        }
        g->intersections = new_points;
        g->max_intersections = new_max;
    }

    g->intersections[g->intersection_count].x = x;
    g->intersections[g->intersection_count].y = y;
    g->intersection_count++;
    return true;
}

int graph_add_road(Graph *g, int x1, int y1, int x2, int y2, RoadType type, float speed_limit) {
    if (g == NULL) {
        fprintf(stderr, "graph_add_road: graph is NULL\n");
        return -1;
    }

    if (x1 < 0 || x1 >= g->grid_width || x2 < 0 || x2 >= g->grid_width ||
        y1 < 0 || y1 >= g->grid_height || y2 < 0 || y2 >= g->grid_height) {
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
    road->length = abs(x2 - x1) + abs(y2 - y1) + 1;
    road->speed_limit = speed_limit;
    road->accident = false;
    road->lanes = 1;

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
                    graph_add_intersection(g, x, y);
                }
            } else if (road_a->type == ROAD_VERTICAL && road_b->type == ROAD_HORIZONTAL) {
                int x = road_a->x1;
                int y = road_b->y1;
                if (point_in_range(x, road_b->x1, road_b->x2) && point_in_range(y, road_a->y1, road_a->y2)) {
                    graph_add_intersection(g, x, y);
                }
            }
        }
    }
}

void graph_destroy(Graph *g) {
    if (g == NULL) {
        return;
    }

    free(g->intersections);
    free(g->roads);
    free(g);
}

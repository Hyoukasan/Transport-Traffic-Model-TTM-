#include "graph.h"
#include <stdlib.h>
#include <stdio.h>

#define INITIAL_CAPACITY 50

static bool graph_expand_roads(Graph *g) {
    if (!g) {
        return false;
    }

    int new_max = g->max_roads * 2;
    RoadSegment *new_roads = realloc(g->roads, sizeof(RoadSegment) * new_max);
    if (!new_roads) {
        fprintf(stderr, "graph_expand_roads: failed to allocate expanded road array\n");
        return false;
    }

    g->roads = new_roads;
    g->max_roads = new_max;
    return true;
}

Graph* graph_create(int window_width, int window_height, int chunk_size, int padding) {
    Graph *g = malloc(sizeof(Graph));
    if (!g) {
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

    g->road_count = 0;
    g->max_roads = INITIAL_CAPACITY;
    g->roads = malloc(sizeof(RoadSegment) * g->max_roads);
    if (!g->roads) {
        fprintf(stderr, "graph_create: failed to allocate roads array\n");
        free(g);
        return NULL;
    }

    return g;
}

int graph_add_road(Graph *g, int x1, int y1, int x2, int y2, RoadType type, float speed_limit) {
    if (!g) {
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

void graph_destroy(Graph *g) {
    if (!g) {
        return;
    }

    free(g->roads);
    free(g);
}

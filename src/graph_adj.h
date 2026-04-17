#ifndef GRAPH_ADJ_H
#define GRAPH_ADJ_H

#include <stdbool.h>

typedef struct AdjEdge {
    int to;
    float weight;
    struct AdjEdge* next;
} AdjEdge;

typedef struct {
    int node_count;
    AdjEdge** edges;
} AdjGraph;

typedef struct {
    int source;
    int node_count;
    float *dist;
    int *prev;
} DijkstraResult;

AdjGraph* adjgraph_create(int node_count);
void adjgraph_destroy(AdjGraph *graph);

bool adjgraph_add_edge(AdjGraph *graph, int from, int to, float weight);

DijkstraResult* dijkstra_shortest_path(const AdjGraph *graph, int source);
int dijkstra_reconstruct_path(const DijkstraResult *result, int target, int *out_path, int max_nodes);
void dijkstra_destroy(DijkstraResult *result);

#endif // GRAPH_ADJ_H

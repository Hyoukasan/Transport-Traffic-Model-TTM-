#include "graph_adj.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static AdjEdge* adjedge_create(int to, float weight) {
    AdjEdge *edge = malloc(sizeof(AdjEdge));
    if (!edge) {
        return NULL;
    }
    edge->to = to;
    edge->weight = weight;
    edge->next = NULL;
    return edge;
}

AdjGraph* adjgraph_create(int node_count) {
    if (node_count <= 0) {
        return NULL;
    }

    AdjGraph *graph = malloc(sizeof(AdjGraph));
    if (!graph) {
        return NULL;
    }

    graph->node_count = node_count;
    graph->edges = calloc(node_count, sizeof(AdjEdge*));
    if (!graph->edges) {
        free(graph);
        return NULL;
    }

    return graph;
}

void adjgraph_destroy(AdjGraph *graph) {
    if (!graph) {
        return;
    }

    for (int i = 0; i < graph->node_count; i++) {
        AdjEdge *edge = graph->edges[i];
        while (edge) {
            AdjEdge *next = edge->next;
            free(edge);
            edge = next;
        }
    }
    free(graph->edges);
    free(graph);
}

bool adjgraph_add_edge(AdjGraph *graph, int from, int to, float weight) {
    if (!graph || from < 0 || from >= graph->node_count || to < 0 || to >= graph->node_count) {
        return false;
    }

    AdjEdge *edge = adjedge_create(to, weight);
    if (!edge) {
        return false;
    }

    edge->next = graph->edges[from];
    graph->edges[from] = edge;
    return true;
}

typedef struct {
    int vertex;
    float dist;
} HeapNode;

typedef struct {
    HeapNode *data;
    int size;
    int capacity;
} MinHeap;

static MinHeap* heap_create(int capacity) {
    if (capacity <= 0) {
        return NULL;
    }

    MinHeap *heap = malloc(sizeof(MinHeap));
    if (!heap) {
        return NULL;
    }

    heap->data = malloc(sizeof(HeapNode) * capacity);
    if (!heap->data) {
        free(heap);
        return NULL;
    }

    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

static void heap_destroy(MinHeap *heap) {
    if (!heap) {
        return;
    }
    free(heap->data);
    free(heap);
}

static void heap_swap(HeapNode *a, HeapNode *b) {
    HeapNode tmp = *a;
    *a = *b;
    *b = tmp;
}

static void heap_sift_up(MinHeap *heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (heap->data[parent].dist <= heap->data[index].dist) {
            break;
        }
        heap_swap(&heap->data[parent], &heap->data[index]);
        index = parent;
    }
}

static void heap_sift_down(MinHeap *heap, int index) {
    int smallest = index;
    while (true) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;

        if (left < heap->size && heap->data[left].dist < heap->data[smallest].dist) {
            smallest = left;
        }
        if (right < heap->size && heap->data[right].dist < heap->data[smallest].dist) {
            smallest = right;
        }
        if (smallest == index) {
            break;
        }
        heap_swap(&heap->data[index], &heap->data[smallest]);
        index = smallest;
    }
}

static bool heap_push(MinHeap *heap, int vertex, float dist) {
    if (!heap || heap->size >= heap->capacity) {
        return false;
    }
    heap->data[heap->size].vertex = vertex;
    heap->data[heap->size].dist = dist;
    heap_sift_up(heap, heap->size);
    heap->size++;
    return true;
}

static bool heap_pop(MinHeap *heap, HeapNode *out_node) {
    if (!heap || heap->size == 0 || !out_node) {
        return false;
    }
    *out_node = heap->data[0];
    heap->size--;
    if (heap->size > 0) {
        heap->data[0] = heap->data[heap->size];
        heap_sift_down(heap, 0);
    }
    return true;
}

DijkstraResult* dijkstra_shortest_path(const AdjGraph *graph, int source) {
    if (!graph || source < 0 || source >= graph->node_count) {
        return NULL;
    }

    DijkstraResult *result = malloc(sizeof(DijkstraResult));
    if (!result) {
        return NULL;
    }

    result->source = source;
    result->node_count = graph->node_count;
    result->dist = malloc(sizeof(float) * graph->node_count);
    result->prev = malloc(sizeof(int) * graph->node_count);
    if (!result->dist || !result->prev) {
        free(result->dist);
        free(result->prev);
        free(result);
        return NULL;
    }

    for (int i = 0; i < graph->node_count; i++) {
        result->dist[i] = INFINITY;
        result->prev[i] = -1;
    }
    result->dist[source] = 0.0f;

    MinHeap *heap = heap_create(graph->node_count * 2);
    if (!heap) {
        dijkstra_destroy(result);
        return NULL;
    }
    heap_push(heap, source, 0.0f);

    while (heap->size > 0) {
        HeapNode node;
        heap_pop(heap, &node);
        int u = node.vertex;
        float best_dist = node.dist;

        if (best_dist > result->dist[u]) {
            continue;
        }

        AdjEdge *edge = graph->edges[u];
        while (edge) {
            int v = edge->to;
            float alt = best_dist + edge->weight;
            if (alt < result->dist[v]) {
                result->dist[v] = alt;
                result->prev[v] = u;
                heap_push(heap, v, alt);
            }
            edge = edge->next;
        }
    }

    heap_destroy(heap);
    return result;
}

int dijkstra_reconstruct_path(const DijkstraResult *result, int target, int *out_path, int max_nodes) {
    if (!result || !out_path || max_nodes <= 0 || target < 0 || target >= result->node_count) {
        return 0;
    }
    if (result->dist[target] == INFINITY) {
        return 0;
    }

    int count = 0;
    int current = target;
    int *temp = malloc(sizeof(int) * max_nodes);
    if (!temp) {
        return 0;
    }

    while (current >= 0 && count < max_nodes) {
        temp[count++] = current;
        if (current == result->source) {
            break;
        }
        current = result->prev[current];
    }

    if (current != result->source) {
        free(temp);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        out_path[i] = temp[count - 1 - i];
    }
    free(temp);
    return count;
}

void dijkstra_destroy(DijkstraResult *result) {
    if (!result) {
        return;
    }
    free(result->dist);
    free(result->prev);
    free(result);
}

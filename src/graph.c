#include "graph.h"
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 50

Graph* graph_create(int window_width, int window_height, int chunk_size, int padding) {
    Graph *g = (Graph*)malloc(sizeof(Graph));
    
    g->window_width = window_width;
    g->window_height = window_height;
    g->chunk_size = chunk_size;
    g->padding = padding;
    
    // Вычисляем размер сетки с учетом отступов
    g->grid_width = (window_width / chunk_size) - (2 * padding);
    g->grid_height = (window_height / chunk_size) - (2 * padding);
    
    g->node_count = 0;
    g->max_nodes = INITIAL_CAPACITY;
    g->nodes = (Node*)malloc(sizeof(Node) * g->max_nodes);
    
    g->edge_count = 0;
    g->max_edges = INITIAL_CAPACITY;
    g->edges = (Edge*)malloc(sizeof(Edge) * g->max_edges);
    
    g->adjacency = (int**)malloc(sizeof(int*) * g->max_nodes);
    g->adjacency_size = (int*)malloc(sizeof(int) * g->max_nodes);
    
    for (int i = 0; i < g->max_nodes; i++) {
        g->adjacency[i] = (int*)malloc(sizeof(int) * g->max_nodes);
        g->adjacency_size[i] = 0;
    }
    
    return g;
}

int graph_add_node(Graph *g, int grid_x, int grid_y) {
    // Проверяем границы сетки
    if (grid_x < 0 || grid_x >= g->grid_width || grid_y < 0 || grid_y >= g->grid_height) {
        return -1;
    }
    
    // Пересчитываем в пиксели (с учетом парддинга)
    int pixel_x = (grid_x + g->padding) * g->chunk_size + g->chunk_size / 2;
    int pixel_y = (grid_y + g->padding) * g->chunk_size + g->chunk_size / 2;
    
    // Расширяем массив если нужно
    if (g->node_count >= g->max_nodes) {
        g->max_nodes *= 2;
        g->nodes = (Node*)realloc(g->nodes, sizeof(Node) * g->max_nodes);
        
        g->adjacency = (int**)realloc(g->adjacency, sizeof(int*) * g->max_nodes);
        g->adjacency_size = (int*)realloc(g->adjacency_size, sizeof(int) * g->max_nodes);
        
        for (int i = g->node_count; i < g->max_nodes; i++) {
            g->adjacency[i] = (int*)malloc(sizeof(int) * g->max_nodes);
            g->adjacency_size[i] = 0;
        }
    }
    
    Node *node = &g->nodes[g->node_count];
    node->id = g->node_count;
    node->grid_x = grid_x;
    node->grid_y = grid_y;
    node->pixel_x = pixel_x;
    node->pixel_y = pixel_y;
    node->road_count = 0;
    
    return g->node_count++;
}

int graph_get_or_create_node(Graph *g, int grid_x, int grid_y) {
    // Ищем существующий узел
    for (int i = 0; i < g->node_count; i++) {
        if (g->nodes[i].grid_x == grid_x && g->nodes[i].grid_y == grid_y) {
            return i;
        }
    }
    
    // Если не найден, создаем новый
    return graph_add_node(g, grid_x, grid_y);
}

void graph_add_edge(Graph *g, int from, int to, RoadType type) {
    if (from < 0 || from >= g->node_count || to < 0 || to >= g->node_count) {
        return;
    }
    
    // Расширяем массив если нужно
    if (g->edge_count >= g->max_edges) {
        g->max_edges *= 2;
        g->edges = (Edge*)realloc(g->edges, sizeof(Edge) * g->max_edges);
    }
    
    Edge *edge = &g->edges[g->edge_count];
    edge->from = from;
    edge->to = to;
    edge->type = type;
    
    // Вычисляем длину дороги
    if (type == ROAD_HORIZONTAL) {
        edge->length = abs(g->nodes[to].grid_x - g->nodes[from].grid_x);
    } else {
        edge->length = abs(g->nodes[to].grid_y - g->nodes[from].grid_y);
    }
    
    // Распределяем чанки на этой дороге
    edge->chunks = (int*)malloc(sizeof(int) * (edge->length + 1));
    
    if (type == ROAD_HORIZONTAL) {
        int start = g->nodes[from].grid_x < g->nodes[to].grid_x ? g->nodes[from].grid_x : g->nodes[to].grid_x;
        for (int i = 0; i <= edge->length; i++) {
            edge->chunks[i] = start + i;
        }
    } else {
        int start = g->nodes[from].grid_y < g->nodes[to].grid_y ? g->nodes[from].grid_y : g->nodes[to].grid_y;
        for (int i = 0; i <= edge->length; i++) {
            edge->chunks[i] = start + i;
        }
    }
    
    // Добавляем в список смежности
    g->adjacency[from][g->adjacency_size[from]++] = to;
    g->adjacency[to][g->adjacency_size[to]++] = from;
    
    // Увеличиваем счетчик дорог на узлах
    g->nodes[from].road_count++;
    g->nodes[to].road_count++;
    
    g->edge_count++;
}

int graph_count_roads_at_node(Graph *g, int node_id) {
    if (node_id < 0 || node_id >= g->node_count) {
        return 0;
    }
    return g->nodes[node_id].road_count;
}

void graph_destroy(Graph *g) {
    if (!g) return;
    
    for (int i = 0; i < g->max_nodes; i++) {
        free(g->adjacency[i]);
    }
    free(g->adjacency);
    free(g->adjacency_size);
    
    for (int i = 0; i < g->edge_count; i++) {
        free(g->edges[i].chunks);
    }
    free(g->edges);
    free(g->nodes);
    free(g);
}

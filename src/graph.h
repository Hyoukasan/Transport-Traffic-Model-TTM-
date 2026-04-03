#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>

typedef enum {
    ROAD_HORIZONTAL,
    ROAD_VERTICAL
} RoadType;

// Узел графа = перекресток
typedef struct {
    int id;
    int grid_x;      // позиция на сетке (в чанках)
    int grid_y;
    int pixel_x;     // позиция в пикселях
    int pixel_y;
    int road_count;  // сколько дорог проходит через этот перекресток
} Node;

// Ребро графа = дорога между двумя перекрестками
typedef struct {
    int from;        // id первого узла
    int to;          // id второго узла
    RoadType type;   // HORIZONTAL или VERTICAL
    int length;      // длина дороги в чанках
    int *chunks;     // массив чанков, через которые проходит дорога
} Edge;

// Весь граф
typedef struct {
    Node *nodes;
    int node_count;
    int max_nodes;
    
    Edge *edges;
    int edge_count;
    int max_edges;
    
    // Список смежности (для каждого узла - какие узлы с ним соседи)
    int **adjacency;
    int *adjacency_size;
    
    // Параметры сетки
    int grid_width;
    int grid_height;
    int chunk_size;  // размер чанка в пикселях
    
    int window_width;
    int window_height;
    int padding;     // отступ от края окна в чанках
} Graph;

// Инициализация графа
Graph* graph_create(int window_width, int window_height, int chunk_size, int padding);

// Добавить перекресток (узел)
int graph_add_node(Graph *g, int grid_x, int grid_y);

// Добавить дорогу (ребро)
void graph_add_edge(Graph *g, int from, int to, RoadType type);

// Получить или создать перекресток на координатах
int graph_get_or_create_node(Graph *g, int grid_x, int grid_y);

// Подсчитать количество дорог через перекресток
int graph_count_roads_at_node(Graph *g, int node_id);

// Очистить и освободить граф
void graph_destroy(Graph *g);

#endif

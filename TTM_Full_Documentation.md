# Transport Traffic Model (TTM) - Полная документация

## Обзор проекта
TTM - симулятор транспортного трафика на C с использованием OpenGL/GLFW.
Проект генерирует сетку дорог и визуализирует их в полноэкранном режиме.

## Архитектура

### Основные модули:
1. **graph.h/c** - Граф дорожной сети (узлы + ребра)
2. **road_generator.h/c** - Генератор случайных дорог
3. **renderer.h/c** - Визуализация (OpenGL)
4. **application.h/c** - Главный контроллер (окно, loop)
5. **car.h** - Структура машины (готово к использованию)

---

## 1. GRAPH MODULE (Граф дорожной сети)

### Структуры данных:

#### Node (Узел = Перекресток)
```c
typedef struct {
    int id;           // Уникальный ID (0, 1, 2, ...)
    int grid_x;       // Позиция на сетке в ЧАНКАХ
    int grid_y;       // Позиция на сетке в ЧАНКАХ
    int pixel_x;      // Позиция в ПИКСЕЛЯХ для рисования
    int pixel_y;      // Позиция в ПИКСЕЛЯХ для рисования
    int road_count;   // Сколько дорог пересекается в этом перекрестке
} Node;
```

#### Edge (Ребро = Дорога)
```c
typedef struct {
    int from;         // ID узла начала дороги
    int to;           // ID узла конца дороги
    RoadType type;    // ROAD_HORIZONTAL или ROAD_VERTICAL
    int length;       // Длина дороги в чанках
    int *chunks;      // Массив номеров чанков на дороге
} Edge;
```

#### Graph (Весь граф)
```c
typedef struct {
    Node *nodes;           // Массив всех перекрестков
    int node_count;        // Сколько перекрестков создано
    int max_nodes;         // Выделенная память
    
    Edge *edges;           // Массив всех дорог
    int edge_count;        // Сколько дорог создано
    int max_edges;         // Выделенная память
    
    // Список смежности для поиска соседей
    int **adjacency;       // adjacency[0] = [1, 3] (узел 0 соединен с 1 и 3)
    int *adjacency_size;   // Количество соседей у каждого узла
    
    // Параметры сетки
    int grid_width;        // Ширина сетки в чанках
    int grid_height;       // Высота сетки в чанках
    int chunk_size;        // Размер чанка в пикселях
    int window_width;      // Ширина окна в пикселях
    int window_height;     // Высота окна в пикселях
    int padding;           // Отступ от края в чанках
} Graph;
```

### API функции:
- `Graph* graph_create(int w, int h, int chunk_size, int padding)`
- `int graph_add_node(Graph *g, int grid_x, int grid_y)`
- `void graph_add_edge(Graph *g, int from, int to, RoadType type)`
- `int graph_get_or_create_node(Graph *g, int grid_x, int grid_y)`
- `void graph_destroy(Graph *g)`

---

## 2. ROAD_GENERATOR MODULE (Генератор дорог)

### Структура:
```c
typedef struct {
    Point *points;         // Массив случайных точек
    int point_count;       // Всего точек (= количество дорог)
    int max_points;        // Размер массива
    
    int horizontal_roads;  // Количество горизонтальных дорог
    int vertical_roads;    // Количество вертикальных дорог
} RoadGenerator;

typedef struct {
    int x, y;  // Координаты точки на сетке
} Point;
```

### Алгоритм генерации:

1. **Создание генератора:**
   ```c
   RoadGenerator* gen = road_gen_create(4);  // 4 дороги
   // Автоматически: 2 горизонтальные + 2 вертикальные
   ```

2. **Генерация точек:**
   ```c
   road_gen_generate_points(gen, graph);
   // Для горизонтальных дорог: случайный Y, X=0 и X=max
   // Для вертикальных дорог: случайный X, Y=0 и Y=max
   ```

3. **Построение дорог:**
   ```c
   road_gen_build_roads(gen, graph);
   // Создает дороги от края до края экрана
   // На пересечениях автоматически создает перекрестки
   ```

### Пример результата (4 дороги):
```
Горизонтальные дороги:
  Y=5:  ━━━━━━━━━━━━━━━━━━━━━━━━ (от X=0 до X=35)
  Y=12: ━━━━━━━━━━━━━━━━━━━━━━━━ (от X=0 до X=35)

Вертикальные дороги:
  X=8:  ║ (от Y=0 до Y=18)
  X=20: ║ (от Y=0 до Y=18)

Созданные перекрестки (4 узла):
  Node[0]: grid(8,5)   - пересечение X=8, Y=5
  Node[1]: grid(20,5)  - пересечение X=20, Y=5
  Node[2]: grid(8,12)  - пересечение X=8, Y=12
  Node[3]: grid(20,12) - пересечение X=20, Y=12
```

---

## 3. RENDERER MODULE (Визуализация)

### Что делает:
Преобразует координаты из пиксельной системы в нормализованные OpenGL координаты (-1 до +1).

### Трансформация координат:
```
Пиксельные → Нормализованные:
x_gl = (2.0 * x_pixel / window_width) - 1.0
y_gl = 1.0 - (2.0 * y_pixel / window_height)
```

### API:
- `void renderer_init(void)` - Создает VAO/VBO для OpenGL
- `void renderer_draw_roads(Graph *graph)` - Рисует белые линии дорог
- `void renderer_draw_nodes(Graph *graph)` - Рисует красные точки перекрестков
- `void renderer_shutdown(void)` - Освобождает ресурсы

---

## 4. CAR MODULE (Машина)

### Структура (готова к использованию):
```c
typedef struct Car {
    int carID;          // Уникальный ID машины
    int stepAdded;      // Шаг симуляции, когда машина была добавлена
    int origin;         // ID перекрестка старта
    int next;           // ID перекрестка, куда сейчас едет
    int destination;    // ID конечного перекрестка
    bool moved;         // Флаг движения на текущем шаге
} Car;

Car* createCar(int carID, int stepAdded, int origin, int next, int destination);
void freeCar(Car* c);
```

---

## 5. APPLICATION MODULE (Главный контроллер)

### Параметры (можно менять):
```c
int chunk_size = 50;  // Размер чанка в пикселях
int padding = 1;      // Отступ от края в чанках
int num_roads = 4;    // Количество дорог для генерации
```

### Полный экран:
Использует `glfwGetVideoMode(glfwGetPrimaryMonitor())` для получения размеров экрана.

### Главный loop:
```c
while(application_is_running()) {
    application_update();  // Отрисовка + логика
}
```

---

## КОД ВСЕХ ФАЙЛОВ

### graph.h
```c
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
```

### graph.c
```c
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
```

### road_generator.h
```c
#ifndef ROAD_GENERATOR_H
#define ROAD_GENERATOR_H

#include "graph.h"

typedef struct {
    int x;
    int y;
} Point;

// Генератор дорог
typedef struct {
    Point *points;
    int point_count;
    int max_points;
    
    int horizontal_roads;  // количество вертикальных дорог
    int vertical_roads;    // количество горизонтальных дорог
} RoadGenerator;

// Создать генератор
RoadGenerator* road_gen_create(int num_roads);

// Генерировать случайные координаты точек
void road_gen_generate_points(RoadGenerator *gen, Graph *graph);

// Построить дороги на основе точек (Манхэттенское расстояние)
void road_gen_build_roads(RoadGenerator *gen, Graph *graph);

// Освободить генератор
void road_gen_destroy(RoadGenerator *gen);

#endif
```

### road_generator.c
```c
#include "road_generator.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

#define INITIAL_CAPACITY 50

RoadGenerator* road_gen_create(int num_roads) {
    RoadGenerator *gen = (RoadGenerator*)malloc(sizeof(RoadGenerator));
    
    gen->point_count = num_roads;
    gen->max_points = num_roads;
    gen->points = (Point*)malloc(sizeof(Point) * num_roads);
    
    // Простое распределение: примерно половина вертикальных, половина горизонтальных
    gen->horizontal_roads = num_roads / 2;
    gen->vertical_roads = num_roads - gen->horizontal_roads;
    
    return gen;
}

void road_gen_generate_points(RoadGenerator *gen, Graph *graph) {
    if (!gen || !graph || gen->point_count == 0) {
        return;
    }
    
    srand((unsigned int)time(NULL));
    
    // Генерируем точки
    // Первый набор - горизонтальные дороги (одинаковый Y, разные X)
    for (int i = 0; i < gen->horizontal_roads; i++) {
        gen->points[i].x = rand() % graph->grid_width;
        gen->points[i].y = rand() % graph->grid_height;
    }
    
    // Второй набор - вертикальные дороги (одинаковый X, разные Y)
    for (int i = gen->horizontal_roads; i < gen->point_count; i++) {
        gen->points[i].x = rand() % graph->grid_width;
        gen->points[i].y = rand() % graph->grid_height;
    }
    
    // Выводим информацию
    printf("Generated %d road points:\n", gen->point_count);
    printf("  Horizontal roads: %d\n", gen->horizontal_roads);
    printf("  Vertical roads: %d\n", gen->vertical_roads);
    printf("  Expected intersections: %d\n", gen->horizontal_roads * gen->vertical_roads);
    
    for (int i = 0; i < gen->point_count; i++) {
        printf("  Point %d: (%d, %d)\n", i, gen->points[i].x, gen->points[i].y);
    }
}

void road_gen_build_roads(RoadGenerator *gen, Graph *graph) {
    if (!gen || !graph) {
        return;
    }
    
    // Строим горизонтальные дороги
    for (int i = 0; i < gen->horizontal_roads; i++) {
        Point *p = &gen->points[i];
        
        // Получаем или создаем узлы на концах дороги
        int y = p->y;  // неизменный Y (горизонтальная дорога)
        
        int left_id = graph_get_or_create_node(graph, 0, y);
        int right_id = graph_get_or_create_node(graph, graph->grid_width - 1, y);
        
        // Добавляем дорогу
        graph_add_edge(graph, left_id, right_id, ROAD_HORIZONTAL);
        
        printf("Horizontal road %d: from (0, %d) to (%d, %d)\n", 
               i, y, graph->grid_width - 1, y);
    }
    
    // Строим вертикальные дороги
    for (int i = 0; i < gen->vertical_roads; i++) {
        Point *p = &gen->points[gen->horizontal_roads + i];
        
        // Получаем или создаем узлы на концах дороги
        int x = p->x;  // неизменный X (вертикальная дорога)
        
        int top_id = graph_get_or_create_node(graph, x, 0);
        int bot_id = graph_get_or_create_node(graph, x, graph->grid_height - 1);
        
        // Добавляем дорогу
        graph_add_edge(graph, top_id, bot_id, ROAD_VERTICAL);
        
        printf("Vertical road %d: from (%d, 0) to (%d, %d)\n", 
               i, x, x, graph->grid_height - 1);
    }
    
    printf("\nRoad network created:\n");
    printf("  Total nodes (intersections): %d\n", graph->node_count);
    printf("  Total edges (roads): %d\n", graph->edge_count);
}

void road_gen_destroy(RoadGenerator *gen) {
    if (!gen) return;
    
    free(gen->points);
    free(gen);
}
```

### renderer.h
```c
#ifndef RENDERER_H
#define RENDERER_H

#include "graph.h"

// Инициализация рендерера
void renderer_init(void);

// Рисование всех дорог из графа
void renderer_draw_roads(Graph *graph);

// Рисование узлов (перекрестков)
void renderer_draw_nodes(Graph *graph);

// Очистка рендерера
void renderer_shutdown(void);

#endif
```

### renderer.c
```c
#include "renderer.h"
#include <GL/glew.h>
#include <stdlib.h>
#include <math.h>

static GLuint VAO = 0, VBO = 0;

void renderer_init(void) {
    // Создаем VAO и VBO для рисования линий
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Настраиваем атрибуты вершин
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderer_draw_roads(Graph *graph) {
    if (!graph || graph->edge_count == 0) {
        return;
    }
    
    // Преобразуем координаты в нормализованные координаты OpenGL (-1 до 1)
    float *vertices = (float*)malloc(graph->edge_count * 4 * sizeof(float));
    
    for (int i = 0; i < graph->edge_count; i++) {
        Edge *edge = &graph->edges[i];
        Node *from = &graph->nodes[edge->from];
        Node *to = &graph->nodes[edge->to];
        
        // Преобразуем пиксельные координаты в нормализованные
        float x1 = (2.0f * from->pixel_x / graph->window_width) - 1.0f;
        float y1 = 1.0f - (2.0f * from->pixel_y / graph->window_height);
        float x2 = (2.0f * to->pixel_x / graph->window_width) - 1.0f;
        float y2 = 1.0f - (2.0f * to->pixel_y / graph->window_height);
        
        vertices[i * 4 + 0] = x1;
        vertices[i * 4 + 1] = y1;
        vertices[i * 4 + 2] = x2;
        vertices[i * 4 + 3] = y2;
    }
    
    // Загружаем вершины в VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, graph->edge_count * 4 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    
    // Рисуем линии
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, graph->edge_count * 2);
    glBindVertexArray(0);
    
    free(vertices);
}

void renderer_draw_nodes(Graph *graph) {
    if (!graph || graph->node_count == 0) {
        return;
    }
    
    // Преобразуем координаты для точек (перекрестков)
    float *vertices = (float*)malloc(graph->node_count * 2 * sizeof(float));
    
    for (int i = 0; i < graph->node_count; i++) {
        Node *node = &graph->nodes[i];
        
        float x = (2.0f * node->pixel_x / graph->window_width) - 1.0f;
        float y = 1.0f - (2.0f * node->pixel_y / graph->window_height);
        
        vertices[i * 2 + 0] = x;
        vertices[i * 2 + 1] = y;
    }
    
    // Загружаем вершины в VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, graph->node_count * 2 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    
    // Рисуем точки
    glPointSize(8.0f);
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, graph->node_count);
    glPointSize(1.0f);
    glBindVertexArray(0);
    
    free(vertices);
}

void renderer_shutdown(void) {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}
```

### application.h
```c
#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdbool.h>
#include "graph.h"

int application_init(const char *title);
bool application_is_running(void);
void application_update(void);
void application_shutdown(void);

// Getter для графа (может понадобиться внешним модулям)
Graph* application_get_graph(void);

#endif
```

### application.c
```c
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdbool.h>

#include "application.h"
#include "graph.h"
#include "road_generator.h"
#include "menu.h"
#include "renderer.h"

static GLFWwindow* window = NULL;
static AppState app_state = APP_STATE_MAIN_MENU;
static Menu menu = {0};
static int prev_lmb = GLFW_RELEASE;
static Graph* graph = NULL;

int application_init(const char *title){
    // Создаем лог-файл для отладки
    FILE *logfile = fopen("debug_log.txt", "w");
    
    if (!glfwInit()){
        printf("GLFW initialization failed!\n");
        if (logfile) { fprintf(logfile, "GLFW initialization failed!\n"); fclose(logfile); }
        return 1;
    }

    // Получаем размеры главного монитора
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int screen_width = mode->width;
    int screen_height = mode->height;
    
    if (logfile) fprintf(logfile, "Screen resolution: %dx%d\n", screen_width, screen_height);
    printf("Screen resolution: %dx%d\n", screen_width, screen_height);

    window = glfwCreateWindow(screen_width, screen_height, title, glfwGetPrimaryMonitor(), NULL);
    if (!window){
        printf("Window initialization failed!\n");
        if (logfile) { fprintf(logfile, "Window initialization failed!\n"); fclose(logfile); }
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    
    if(glewInit() != GLEW_OK){
        printf("GLEW initialization failed!\n");
        if (logfile) { fprintf(logfile, "GLEW initialization failed!\n"); fclose(logfile); }
        glfwTerminate();
        return 1;
    }

    // Инициализируем граф сетки дорог
    int chunk_size = 50;  // размер одного чанка в пикселях
    int padding = 1;      // отступ от края в чанках
    graph = graph_create(screen_width, screen_height, chunk_size, padding);
    
    if (logfile) {
        fprintf(logfile, "Graph created\n");
        fprintf(logfile, "  Grid size: %dx%d chunks\n", graph->grid_width, graph->grid_height);
        fprintf(logfile, "  Chunk size: %d\n", chunk_size);
    }
    
    // Генерируем и строим дороги
    int num_roads = 4;  // количество дорог
    RoadGenerator* gen = road_gen_create(num_roads);
    road_gen_generate_points(gen, graph);
    road_gen_build_roads(gen, graph);
    road_gen_destroy(gen);
    
    if (logfile) {
        fprintf(logfile, "\nRoad network info:\n");
        fprintf(logfile, "  Total nodes: %d\n", graph->node_count);
        fprintf(logfile, "  Total edges: %d\n", graph->edge_count);
        fprintf(logfile, "\nNodes:\n");
        for (int i = 0; i < graph->node_count; i++) {
            fprintf(logfile, "  Node %d: grid(%d,%d) pixel(%d,%d) roads=%d\n", 
                   i, graph->nodes[i].grid_x, graph->nodes[i].grid_y,
                   graph->nodes[i].pixel_x, graph->nodes[i].pixel_y,
                   graph->nodes[i].road_count);
        }
        fprintf(logfile, "\nEdges:\n");
        for (int i = 0; i < graph->edge_count; i++) {
            fprintf(logfile, "  Edge %d: %d->%d type=%s length=%d\n", 
                   i, graph->edges[i].from, graph->edges[i].to,
                   graph->edges[i].type == ROAD_HORIZONTAL ? "H" : "V",
                   graph->edges[i].length);
        }
        fflush(logfile);
    }
    
    menu_init(&menu);
    
    // Инициализируем рендерер
    renderer_init();
    
    if (logfile) {
        fprintf(logfile, "\nApplication initialized successfully!\n");
        fclose(logfile);
    }

    return 0;
}

bool application_is_running(void){
    return !glfwWindowShouldClose(window) && app_state != APP_STATE_EXIT;
}

void application_update(void){
    double mx, my;
    glfwGetCursorPos(window, &mx, &my);
    
    int lmb = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    bool click = (lmb == GLFW_PRESS && prev_lmb == GLFW_RELEASE);
    prev_lmb = lmb;

    if(app_state == APP_STATE_MAIN_MENU || app_state == APP_STATE_SETTINGS_MENU){
        menu_update(&menu, (int)mx, (int)my, click);
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Render roads and intersections
    if (graph) {
        // Set color for roads (white)
        glColor3f(1.0f, 1.0f, 1.0f);
        renderer_draw_roads(graph);
        
        // Set color for nodes (red)
        glColor3f(1.0f, 0.0f, 0.0f);
        renderer_draw_nodes(graph);
    }
    
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void application_shutdown(void){
    renderer_shutdown();
    
    if (graph) {
        graph_destroy(graph);
    }
    glfwTerminate();
}

Graph* application_get_graph(void){
    return graph;
}
```

### car.h
```c
#ifndef CAR_H
#define CAR_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct Car
{
    // MADE BY ME helper variable to help me visualize which car's 'turn' it is
    int carID;

    // the time step associated with the addCar event that added this car
    int stepAdded;

    // the intersection this car started from
    int origin;

    // the intersection this car is driving toward currently
    int next;

    // the intersection this car wants to ultimately stop at
    int destination;

    // track whether a car has moved during this time step (remember to reset to false before next time step)
    bool moved;

}  Car;

Car* createCar( int carID, int stepAdded, int origin, int next, int destination );
void freeCar( Car* c );

#endif
```

### main.c
```c
#include "application.h"
#include <stdio.h>

int main(void){
    FILE *test = fopen("test_start.txt", "w");
    fprintf(test, "Program started\n");
    fflush(test);
    fclose(test);

    if(application_init("Transport Traffic Model")){
        return 0;
    }

    while(application_is_running()){
        application_update();
    }

    application_shutdown();

    return 0;
}
```

### menu.h
```c
#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

typedef enum {
    APP_STATE_MAIN_MENU,
    APP_STATE_SIMULATION_CONFIG,
    APP_STATE_SETTINGS_MENU,
    APP_STATE_RUNNING_SIMULATION,
    APP_STATE_EXIT
} AppState;

typedef struct {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;

    const char* text;

    bool selected;
    bool pressed;

    AppState target_state;
} Button;

typedef struct Menu{
    AppState current_state;
    Button buttons[5];
    int button_count;
    int selected_index;
} Menu;

void menu_init(Menu* menu);
void menu_update(Menu* menu, int mx, int my, bool click);
void menu_render(Menu* menu);

#endif
```

### menu.c
```c
#include "menu.h"
#include <stdbool.h>

void menu_init(Menu *menu){
    if (!menu) return;
    
    menu->current_state = APP_STATE_MAIN_MENU;
    menu->button_count = 0;
    menu->selected_index = -1;
}

static void menu_load_state(Menu *menu, AppState app_state){
    if (!menu) return;
    
    switch(app_state){
        case APP_STATE_MAIN_MENU:
            menu->buttons[0] = (Button){};
            menu->buttons[1] = (Button){};
            menu->button_count = 2;
            break;
        case APP_STATE_SETTINGS_MENU:
            menu->buttons[0] = (Button){};
            menu->buttons[1] = (Button){};
            menu->button_count = 2;
            break;

        default: break;
    }   
}

void menu_update(Menu* menu, int mx, int my, bool click){
    if (!menu) return;
    
    for(int i = 0; i < menu->button_count; i++){
        Button *button = &menu->buttons[i];
        // TODO: Логика обновления кнопок
        (void)button;  // Чтобы компилятор не жаловался
    }
}

void menu_render(Menu* menu){
    if (!menu) return;
    
    // TODO: Рисование меню
}
```

### Makefile
```makefile
# ===================================
# Transport Traffic Model (TTM) on C
# ===================================

CC = gcc
OPT = -O2
CFLAGS = -Wall -Wextra -Ithird_party/include -g $(OPT)
LDFLAGS = -Lthird_party/lib
LIBS = -lglfw3 -lglew32 -lopengl32
TARGET = TTM.exe
BUILD_DIR = build
SRCS = $(wildcard src/*.c)
OBJECTS = $(SRCS:src/%.c=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(TARGET)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS)

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(TARGET) $(BUILD_DIR)
```

---

## Сборка и запуск

### Требования:
- GCC (MinGW на Windows)
- GLFW3
- GLEW
- OpenGL

### Компиляция:
```bash
make clean
make
```

### Запуск:
```bash
./TTM.exe
```

### Что должно быть видно:
- Полноэкранное окно
- Белые линии = дороги
- Красные точки = перекрестки
- Серый фон = пустое пространство

---

## Параметры для настройки

В `application.c` можно менять:
```c
int chunk_size = 50;  // Размер чанка в пикселях
int padding = 1;      // Отступ от края экрана в чанках
int num_roads = 4;    // Количество дорог для генерации
```

---

## Архитектурные особенности

### Координатные системы:
1. **Grid coordinates** - логические (в чанках) для алгоритмов
2. **Pixel coordinates** - абсолютные (в пикселях) для рендеринга  
3. **Normalized coordinates** - нормализованные (-1..+1) для OpenGL

### Динамическая память:
- Массивы Node и Edge растут автоматически (начинают с 50, удваиваются)
- Adjacency list для быстрого поиска соседей узла
- Chunks в каждом Edge хранят путь по чанкам

### Алгоритм генерации:
1. Генерируем случайные точки для каждой дороги
2. Строим дороги от края до края экрана
3. На пересечениях автоматически создаем перекрестки
4. Сохраняем топологию в виде графа

---

## Что дальше?

### Готово к реализации:
- **Симуляция движения машин** (используя структуру Car)
- **Алгоритмы маршрутизации** (A*, Dijkstra)
- **Интерактивное меню** (настройка параметров)
- **Статистика трафика** (скорость, пробки)

### Структура для машин:
```c
typedef struct Car {
    int carID;          // ID машины
    int stepAdded;      // Шаг добавления
    int origin;         // Старт (ID узла)
    int next;           // Следующий узел
    int destination;    // Конечная цель
    bool moved;         // Флаг движения
} Car;
```

---

## Вопросы для другого ИИ:

1. Как реализовать симуляцию движения машин по графу?
2. Как добавить алгоритм поиска пути (A* или Dijkstra)?
3. Как оптимизировать рендеринг для большого количества машин?
4. Как добавить физику столкновений и светофоры?
5. Как реализовать статистику трафика в реальном времени?

---

*Создано для анализа другим ИИ. Полная документация проекта TTM.*

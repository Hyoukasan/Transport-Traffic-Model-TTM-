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

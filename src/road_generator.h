#ifndef ROAD_GENERATOR_H
#define ROAD_GENERATOR_H

struct Graph;

typedef struct {
    int x;
    int y;
} Point;

// Генератор дорог
typedef struct {
    Point *points;
    int point_count;
    int max_points;

    int horizontal_roads;  // количество горизонтальных дорог
    int vertical_roads;    // количество вертикальных дорог
} RoadGenerator;

// Создать генератор
RoadGenerator* road_gen_create(int num_roads);

// Генерировать случайные координаты точек
void road_gen_generate_points(RoadGenerator *gen, struct Graph *graph);

// Построить сегменты дорог на основе точек
void road_gen_build_roads(RoadGenerator *gen, struct Graph *graph);

// Освободить генератор
void road_gen_destroy(RoadGenerator *gen);

#endif

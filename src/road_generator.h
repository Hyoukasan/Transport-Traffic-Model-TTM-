#ifndef ROAD_GENERATOR_H
#define ROAD_GENERATOR_H

#define ROAD_SCENARIO_HIGHWAY 1
#define ROAD_SCENARIO_SINGLE_INTERSECTION 2
#define ROAD_SCENARIO_MULTI_INTERSECTION 3

typedef struct Graph Graph;

typedef struct {
    int x;
    int y;
} Point;

// Генератор дорог
typedef struct {
    Point *points;
    int point_count;
    int max_points;
    int scenario;  // Сценарий, по которому строится сеть

    int horizontal_roads;  // количество горизонтальных дорог
    int vertical_roads;    // количество вертикальных дорог
} RoadGenerator;

// Создать генератор
RoadGenerator* road_gen_create(int num_roads);

// Создать генератор для конкретного сценария
RoadGenerator* road_gen_create_with_scenario(int scenario);

// Генерировать массив координат точек с учётом выбранного сценария
void road_gen_generate_points(RoadGenerator *gen, Graph *graph);

// Построить сегменты дорог на основе точек
void road_gen_build_roads(RoadGenerator *gen, Graph *graph);

// Сгенерировать точки и построить дороги одной функцией.
// Параметр scenario позволяет функции самой выбрать нужный кейс.
void road_gen_generate_and_build(RoadGenerator *gen, Graph *graph, int scenario);

// Освободить генератор
void road_gen_destroy(RoadGenerator *gen);

#endif
#ifndef RENDERER_H
#define RENDERER_H

struct Graph;
struct Car;

// Инициализация рендерера
void renderer_init(void);

// Рисование всех сегментов дорог
void renderer_draw_roads(Graph *graph);

// Рисование вспомогательной сетки
void renderer_draw_grid(Graph *graph);

// Рисование узлов/концов сегментов
void renderer_draw_nodes(Graph *graph);

// Рисование машин на дорогах
void renderer_draw_cars(Graph *graph, Car *cars, int car_count);

// Очистка рендерера
void renderer_shutdown(void);

#endif

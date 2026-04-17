#ifndef RENDERER_H
#define RENDERER_H

struct Graph;
struct Car;

// Инициализация рендерера
void renderer_init(void);

// Рисование всех сегментов дорог
void renderer_draw_roads(struct Graph *graph);

// Рисование вспомогательной сетки
void renderer_draw_grid(struct Graph *graph);

// Рисование узлов/концов сегментов
void renderer_draw_nodes(struct Graph *graph);

// Рисование машин на дорогах
void renderer_draw_cars(struct Graph *graph, struct Car *cars, int car_count);

// Очистка рендерера
void renderer_shutdown(void);

#endif

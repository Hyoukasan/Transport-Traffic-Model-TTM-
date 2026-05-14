#ifndef RENDERER_H
#define RENDERER_H

typedef struct Graph Graph;
typedef struct Car Car;
typedef struct Menu Menu_t;

// Инициализация рендерера
void renderer_init(void);


void renderer_draw_background(unsigned int texture);


void menu_render(Menu_t* menu, int screen_width, int screen_height);

// Рисование всех сегментов дорог
void renderer_draw_roads(Graph *graph);

// Рисование вспомогательной сетки
void renderer_draw_grid(Graph *graph);

// Загружает дороги и узлы в GPU один раз
void renderer_upload_graph(Graph *graph);

// Рисование узлов/концов сегментов
// Рисование машин на дорогах
void renderer_draw_cars(Graph *graph, Car *cars, int car_count);


void renderer_draw_text(float x, float y, char* text, float scale, float r, float g, float b, int screen_width, int screen_height);

// Очистка рендерера
void renderer_shutdown(void);

#endif

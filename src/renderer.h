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

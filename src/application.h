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
#ifndef APPLICATION_HEADLESS_H
#define APPLICATION_HEADLESS_H

#include <stdbool.h>

typedef struct Graph Graph;

// Headless version for systems without graphics
int application_init_headless(void);
bool application_is_running_headless(void);
void application_update_headless(void);
void application_shutdown_headless(void);

Graph* application_get_graph_headless(void);

#endif
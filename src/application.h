#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdbool.h>

int application_init(const char *title);
bool application_is_running(void);
void application_update(void);
void application_shutdown(void);

#endif
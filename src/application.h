#ifndef APPLICATION_H
#define APPLICATION_H

void application_init(int width, int height, const char *title);
int application_is_running(void);
void application_update(void);
void application_close(void);

#endif
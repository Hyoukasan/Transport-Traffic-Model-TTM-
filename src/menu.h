#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

typedef enum {
    APP_STATE_MAIN_MENU,
    APP_STATE_SIMULATION_CONFIG,
    APP_STATE_SETTINGS_MENU,
    APP_STATE_RUNNING_SIMULATION,
    APP_STATE_EXIT
} AppState;

typedef struct {
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;

    const char* text;

    bool selected;
    bool pressed;

    AppState target_state;
} Button;

typedef struct Menu{
    AppState current_state;
    Button buttons[5];
    int button_count;
    int selected_index;
} Menu;

void menu_init(Menu* menu);
void menu_update(Menu* menu, int mx, int my, bool click);
void menu_render(Menu* menu);

#endif
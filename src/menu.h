#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

typedef enum {
    MENU_STATE_MAIN_MENU,
    MENU_STATE_CREATE_SIMULATION,
    MENU_STATE_SIMULATION_CONFIG,
    MENU_STATE_SIMULATION_PAUSE,
    MENU_STATE_RUNNING_SIMULATION,
    MENU_STATE_CLOSED,
    MENU_STATE_INFO,
    MENU_STATE_EXIT
} MenuState;

typedef struct {
    int x, y, width, height;
    unsigned int texture;

    bool selected;
    bool pressed;

    MenuState target_state;
} MenuButton_t;

typedef struct Menu {
    int x, y, width, height;
    unsigned int texture;

    MenuState current_state;
    MenuButton_t buttons[5];
    int button_count;
    int selected_index;
} Menu_t;

void menu_init(Menu_t* menu);
void menu_update(Menu_t* menu, int mx, int my, bool click);

#endif  
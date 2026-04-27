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
    MENU_STATE_EXIT,
    MENU_STATE_SCENARIO_HIGHWAY,
    MENU_STATE_SCENARIO_SINGLE_INTERSECTION,
    MENU_STATE_SCENARIO_MULTI_INTERSECTION,
} MenuState;

typedef struct {
    const char *texture_path;
    MenuState target_state;
} ButtonInfo;

typedef struct {
    int x, y, width, height;
    unsigned int texture;

    MenuState target_state;
} MenuButton_t;

typedef struct Menu {
    int x, y, width, height;
    unsigned int texture;
    unsigned int background_texture;

    MenuState current_state;
    MenuButton_t buttons[4];
    int button_count;
    int selected_index;
} Menu_t;

void menu_init(Menu_t* menu, int screen_width, int screen_height);
void menu_update(Menu_t* menu, int mx, int my, bool click);

#endif  

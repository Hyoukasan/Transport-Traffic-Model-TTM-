#ifndef MENU_H
#define MENU_H

typedef enum {
    APP_STATE_MAIN_MENU,
    APP_STATE_RUNNING_SIMULATION,
    APP_STATE_SIMULATION_CONFIG,
    APP_STATE_INFO,
    APP_STATE_EXIT
} AppState;

typedef struct {
    int x, y, width, height;
    unsigned int texture;

    bool selected;
    bool pressed;

    AppState target_state;
} MenuButton_t;

typedef struct Menu {
    int x, y, width, height;
    unsigned int texture;

    AppState current_state;
    MenuButton_t buttons[5];
    int button_count;
    int selected_index;
} Menu_t;

void menu_init(Menu_t* menu);
void menu_update(Menu_t* menu, int mx, int my, bool click);

#endif  
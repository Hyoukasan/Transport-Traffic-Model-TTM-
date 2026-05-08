#ifndef MENU_H
#define MENU_H

#include <stdbool.h>

typedef enum {
    MENU_STATE_IDLE,
    MENU_STATE_MAIN_MENU,
    MENU_STATE_SCENARIO_SELECT,
    MENU_STATE_LOAD_PROFILE,
    MENU_STATE_SIMULATION_CONFIG_SETTING,
    MENU_STATE_SIMULATION_CONFIG_PAUSE,
    MENU_STATE_SIMULATION_PAUSE,
    MENU_STATE_START_SIMULATION,
    MENU_STATE_CLOSED,
    MENU_STATE_INFO,
    MENU_STATE_EXIT,
    MENU_STATE_SCENARIO_HIGHWAY,
    MENU_STATE_SCENARIO_SINGLE_INTERSECTION,
    MENU_STATE_SCENARIO_MULTI_INTERSECTION,
} MenuState;

typedef enum {
    BUTTON_ID_NONE,

    BUTTON_ID_START,
    BUTTON_ID_CONFIG,
    BUTTON_ID_ABOUT,
    BUTTON_ID_EXIT,

    BUTTON_ID_HIGHWAY,
    BUTTON_ID_SINGLE_INTERSECTION,
    BUTTON_ID_MULTI_INTERSECTION,

    BUTTON_ID_ADD_2_LANES,
    BUTTON_ID_SUB_2_LANES,
    BUTTON_ID_ADD_5_CARS,
    BUTTON_ID_SUB_5_CARS,
    
    BUTTON_ID_RESUME,
    BUTTON_ID_BACK,
    BUTTON_ID_SAVE_PROFILE,

    BUTTON_ID_SLOT_1,
    BUTTON_ID_SLOT_2,
    BUTTON_ID_SLOT_3,
    BUTTON_ID_SLOT_4
} ButtonId;

typedef struct {
    const char *texture_path;

    MenuState target_state;
    ButtonId button_id;
} ButtonInfo;

typedef struct {
    int x, y, width, height;
    unsigned int texture;

    MenuState target_state;
    ButtonId button_id;
    char profile_text[64];
} MenuButton_t;

typedef struct Menu {
    int x, y, width, height;
    unsigned int texture;
    unsigned int background_texture;

    MenuState current_state;
    MenuButton_t buttons[8];
    
    int button_count;
    ButtonId last_pressed_button;
} Menu_t;

void menu_init(Menu_t* menu, int screen_width, int screen_height);
void menu_set_state(Menu_t* menu, MenuState state);
void menu_update(Menu_t* menu, int mx, int my, bool click);

#endif 

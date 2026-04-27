#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "menu.h"
#include "texture.h"

static void menu_load_state(Menu_t *menu);

void menu_init(Menu_t *menu, int screen_width, int screen_height){
    if (menu == NULL) {
        return;
    }   
    
    menu->width  = 420;
    menu->height = 820;    
    menu->x      = (screen_width - menu->width) / 2;
    menu->y      = (screen_height - menu->height) / 2;
    
    menu->current_state = MENU_STATE_MAIN_MENU;
    menu->button_count = 4;
    menu->selected_index = -1;

    menu->background_texture = texture_load("Data/textures/background.png", NULL, NULL);
    if(menu->background_texture == 0) {
        printf("Warning: background.png not loaded.\n");
    }

    menu->texture = texture_load("Data/textures/background_menu.png", NULL, NULL);
    if (menu->texture == 0) {
        printf("Warning: new_background.png not loaded, using fallback color.\n");
    }

    menu_load_state(menu); 
}

void menu_set_state(Menu_t* menu, MenuState state) {
    if (menu == NULL) {
        return;
    }

    menu->current_state = state;
    menu_load_state(menu);
}

static void bind_buttons(MenuButton_t *buttons, ButtonInfo* map, int count_buttons) {
    for (int i = 0; i < count_buttons; i++) {
        buttons[i].texture = texture_load(map[i].texture_path, NULL, NULL);
        buttons[i].target_state = map[i].target_state;
    }
}

static ButtonInfo main_menu_buttons[] = {
    {"Data/textures/start.png",                 MENU_STATE_CREATE_SIMULATION},
    {"Data/textures/config.png",                MENU_STATE_SIMULATION_CONFIG},
    {"Data/textures/about.png",                 MENU_STATE_INFO},
    {"Data/textures/exit.png",                  MENU_STATE_EXIT}
};

static ButtonInfo scenario_menu_buttons[] = {
    {"Data/textures/highway.png",               MENU_STATE_SCENARIO_HIGHWAY},
    {"Data/textures/single_intersection.png",   MENU_STATE_SCENARIO_SINGLE_INTERSECTION},
    {"Data/textures/multi_intersection.png",    MENU_STATE_SCENARIO_MULTI_INTERSECTION},
    {"Data/textures/back.png",                  MENU_STATE_MAIN_MENU}
};

static ButtonInfo pause_menu_buttons[] = {
    {"Data/textures/resume.png", MENU_STATE_IDLE},
    {"Data/textures/save.png",   MENU_STATE_SIMULATION_CONFIG},
    {"Data/textures/back.png",   MENU_STATE_MAIN_MENU}
};

static void set_buttons(MenuButton_t* buttons, int button_count, int menu_width, int menu_height, int gap) {
    if(buttons == NULL) {
        return;
    }

    int button_width = 320;
    int button_height = 100;

    int total_height = button_count * button_height + (button_count - 1) * gap;
    int start_y = (menu_height - total_height) / 2;

    for(size_t i = 0; i < button_count; i++) {
        buttons[i].width  = button_width;
        buttons[i].height = button_height;
        buttons[i].x = (menu_width - button_width) / 2;
        buttons[i].y = start_y + i * (button_height + gap) + 50;
    }

}

static void menu_load_state(Menu_t* menu) {
    if (menu == NULL) {
        return;
    }
    
    switch(menu->current_state){
        case MENU_STATE_MAIN_MENU:
            menu->button_count = 4;
            set_buttons(menu->buttons, menu->button_count, menu->width, menu->height, 25);
            bind_buttons(menu->buttons, main_menu_buttons, menu->button_count);
            break;
        case MENU_STATE_CREATE_SIMULATION:
            menu->button_count = 4;
            set_buttons(menu->buttons, menu->button_count, menu->width, menu->height, 30);
            bind_buttons(menu->buttons, scenario_menu_buttons, menu->button_count);
            break;
        case MENU_STATE_SIMULATION_CONFIG:
            menu->button_count = 0;
            break;
        case MENU_STATE_INFO:
            menu->button_count = 0;
            break;
        case MENU_STATE_EXIT:
            menu->button_count = 0;
            break;
        case MENU_STATE_SIMULATION_PAUSE:
            menu->button_count = 3;
            set_buttons(menu->buttons, menu->button_count, menu->width, menu->height, 30);
            bind_buttons(menu->buttons, pause_menu_buttons, menu->button_count);
            break;
        default:
            menu->button_count = 0;
            break;
    }   
}

void menu_update(Menu_t* menu, int mx, int my, bool click) {
    if (menu == NULL) {
        return;
    }

    for (int i = 0; i < menu->button_count; i++) {
        MenuButton_t *button = &menu->buttons[i];

        int x = menu->x + button->x;
        int y = menu->y + button->y;

        bool inside = mx >= x && mx <= x + button->width &&
            my >= y && my <= y + button->height;

        if (click && inside) {
            menu_set_state(menu, button->target_state);
            break;
        }
    }
}

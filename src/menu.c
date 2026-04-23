#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "menu.h"
#include "texture.h"

static uint8_t stack_size = 0;

void menu_init(Menu_t *menu){
    if (menu == NULL) {
        return;
    }   
    
    menu->x = 150;
    menu->y = 320;
    menu->width = 400;
    menu->height = 500;
    menu->current_state = APP_STATE_MAIN_MENU;
    menu->button_count = 4;
    menu->selected_index = -1;

    menu->texture = texture_load("Data/textures/menu_background.png", NULL, NULL);

    if (menu->texture == 0) {
        printf("Warning: menu_background.png not loaded, using fallback color.\n");
    }
}

static void set_button(MenuButton_t *button, int x, int y, int w, int h, unsigned int texture, AppState target_state) {
    button->x = x;
    button->y = y;
    button->width = w;
    button->height = h;
    button->texture = texture;
    button->selected = false;
    button->pressed = false;
    button->target_state = target_state;
}

static void menu_load_state(Menu_t *menu, AppState app_state) {
    if (menu == NULL) {
        return;
    }
    
    switch(app_state){
        case APP_STATE_MAIN_MENU:
        set_button(&menu->buttons[0], 150, 220, 220, 60, texture_load("Data/textures/start.png", NULL, NULL), APP_STATE_RUNNING_SIMULATION);
        set_button(&menu->buttons[1], 150, 290, 220, 60, texture_load("Data/textures/load.png", NULL, NULL), APP_STATE_SIMULATION_CONFIG);
        set_button(&menu->buttons[2], 150, 360, 220, 60, texture_load("Data/textures/about.png", NULL, NULL), APP_STATE_INFO);
        set_button(&menu->buttons[3], 150, 430, 220, 60, texture_load("Data/textures/exit.png", NULL, NULL), APP_STATE_EXIT);
            menu->button_count = 4;
            break;
        case APP_STATE_RUNNING_SIMULATION:
            break;
        case APP_STATE_SIMULATION_CONFIG:
            break;
        case APP_STATE_INFO:
            break;
        case APP_STATE_EXIT:
            break;

        default: break;
    }   
}

void menu_update(Menu_t *menu, int mx, int my, bool click) {
    if (menu == NULL) {
        return;
    }

    for (int i = 0; i < menu->button_count; i++) {
        MenuButton_t *button = &menu->buttons[i];

        bool inside =
            mx >= button->x && mx <= button->x + button->width &&
            my >= button->y && my <= button->y + button->height;

        if (click && inside) {
            state_push(button->target_state);
            break;
        }
    }
}

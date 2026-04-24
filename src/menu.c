#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "menu.h"
#include "texture.h"

static void menu_load_state(Menu_t *menu, MenuState app_state);

void menu_init(Menu_t *menu, int screen_width, int screen_height){
    if (menu == NULL) {
        return;
    }   
    
    menu->width = 420;
    menu->height = 820;    
    menu->x = (screen_width - menu->width) / 2;
    menu->y = (screen_height - menu->height) / 2;
    
    menu->current_state = MENU_STATE_MAIN_MENU;
    menu->button_count = 4;
    menu->selected_index = -1;

    menu_load_state(menu, menu->current_state); 

    menu->background_texture = texture_load("Data/textures/background.png", NULL, NULL);
    if(menu->background_texture == 0) {
        printf("Warning: background.png not loaded.\n");
    }

    menu->texture = texture_load("Data/textures/new_background.png", NULL, NULL);
    if (menu->texture == 0) {
        printf("Warning: new_background.png not loaded, using fallback color.\n");
    }
}

static void set_button(MenuButton_t *button, int x, int y, int w, int h, unsigned int texture, MenuState target_state) {
    button->x = x;
    button->y = y;
    button->width = w;
    button->height = h;
    button->texture = texture;
    button->selected = false;
    button->pressed = false;
    button->target_state = target_state;
}

static void menu_load_state(Menu_t *menu, MenuState app_state) {
    if (menu == NULL) {
        return;
    }
    
    switch(app_state){
        case MENU_STATE_MAIN_MENU:
            set_button(&menu->buttons[0], 150, 220, 135, 60, texture_load("Data/textures/start.png", NULL, NULL), MENU_STATE_CREATE_SIMULATION);
            set_button(&menu->buttons[1], 150, 290, 135, 60, texture_load("Data/textures/load.png", NULL, NULL), MENU_STATE_SIMULATION_CONFIG);
            set_button(&menu->buttons[2], 150, 360, 135, 60, texture_load("Data/textures/about.png", NULL, NULL), MENU_STATE_INFO);
            set_button(&menu->buttons[3], 150, 430, 135, 60, texture_load("Data/textures/exit.png", NULL, NULL), MENU_STATE_EXIT);
            menu->button_count = 4;
            break;
        case MENU_STATE_CREATE_SIMULATION:
            break;
        case MENU_STATE_SIMULATION_CONFIG:
            break;
        case MENU_STATE_INFO:
            break;
        case MENU_STATE_EXIT:
            break;

        default: break;
    }   
}

void menu_update(Menu_t *menu, int mx, int my, bool click) {
    if (menu == NULL) {
        return;
    }

    if (menu->current_state != MENU_STATE_MAIN_MENU) {
        return;
    }

    for (int i = 0; i < menu->button_count; i++) {
        MenuButton_t *button = &menu->buttons[i];

        bool inside =
            mx >= button->x && mx <= button->x + button->width &&
            my >= button->y && my <= button->y + button->height;

        button->selected = inside;

        if (click && inside) {
            menu->current_state = button->target_state;
            break;
        }
    }
}

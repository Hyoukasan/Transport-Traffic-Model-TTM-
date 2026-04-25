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

static void bind_buttons(MenuButton_t *buttons, ButtonInfo *info, int count) {
    for (int i = 0; i < count; i++) {
        buttons[i].texture = texture_load(info[i].texture_path, NULL, NULL);
        buttons[i].target_state = info[i].target_state;
    }
}

static void set_buttons(MenuButton_t* buttons, int button_count, int menu_width, int menu_height, int gap) {
    if(buttons == NULL) {
        return;
    }

    int button_width = 320;
    int button_height = 60;

    int total_height = button_count * button_height + (button_count - 1) * gap;
    int start_y = (menu_height - total_height) / 2;

    for (int i = 0; i < button_count; i++) {
        buttons[i].width  = button_width;
        buttons[i].height = button_height;
        buttons[i].x = (menu_width - button_width) / 2;
        buttons[i].y = start_y + i * (button_height + gap);
    }

}

static void menu_load_state(Menu_t* menu) {
    if (menu == NULL) {
        return;
    }
    
    switch(menu->current_state){
        case MENU_STATE_MAIN_MENU:
            ButtonInfo buttons_map[] = {
                { "Data/textures/start.png",  MENU_STATE_CREATE_SIMULATION },
                { "Data/textures/config.png", MENU_STATE_SIMULATION_CONFIG },
                { "Data/textures/about.png",  MENU_STATE_INFO },
                { "Data/textures/exit.png",   MENU_STATE_EXIT }
            };

            menu->button_count = 4;
            set_buttons(menu->buttons, menu->button_count, menu->width, menu->height, 10);
            bind_buttons(menu->buttons, buttons_map, menu->button_count);
            break;
        case MENU_STATE_CREATE_SIMULATION:
            break;
        case MENU_STATE_SIMULATION_CONFIG:
            break;
        case MENU_STATE_INFO:
            break;
        case MENU_STATE_EXIT:
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

    if (menu->current_state != MENU_STATE_MAIN_MENU) {
        return;
    }

    for (int i = 0; i < menu->button_count; i++) {
        MenuButton_t *button = &menu->buttons[i];

        bool inside =
            mx >= button->x && mx <= button->x + button->width &&
            my >= button->y && my <= button->y + button->height;

        if (click && inside) {
            menu->current_state = button->target_state;
            break;
        }
    }
}

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "menu.h"

#define MAX_HISTORY 10

static AppState state_stack[MAX_HISTORY];
static uint8_t stack_size = 0;

void menu_init(Menu_t *menu){
    if (menu == NULL) {
        return;
    }   
    
    menu->current_state = APP_STATE_MAIN_MENU;
    menu->button_count = 0;
    menu->selected_index = -1;
}

static bool state_push(AppState current_state) {
    if(stack_size >= MAX_HISTORY) {
        return false;
    }

    state_stack[stack_size++] = current_state;

    return true;
}

static bool state_pop(AppState* current_state) {
    if(stack_size == 0) {
        return false;
    }

    stack_size--;
    *current_state = state_stack[stack_size];

    return true;
}

void set_buttons(MenuButton_t* button)

static void menu_load_state(Menu_t *menu, AppState app_state) {
    if (menu == NULL) {
        return;
    }
    
    switch(app_state){
        case APP_STATE_MAIN_MENU:
            menu->buttons[0] = (MenuButton_t){20, 40, 130, 55,};
            menu->buttons[1] = (MenuButton_t){};
            menu->buttons[2] = (MenuButton_t){};
            menu->buttons[3] = (MenuButton_tn){}:
            menu->button_count = 2;
            break;
        case APP_STATE_RUNNING_SIMULATION:
            menu->buttons[0] = (MenuButton_t){};
            menu->buttons[1] = (MenuButton_t){};
            menu->button_count = 2;
            break;
        case APP_STATE_INFO:
            break;
        case APP_STATE_EXIT:
            break;

        default: break;
    }   
}

void menu_update(Menu_t* menu, int mx, int my, bool click){
    if (menu == NULL) {
        return;
    }
    
    for(int i = 0; i < menu->button_count; i++) {
        MenuButton_t *current_button = &menu->buttons[i];
        
        if(click && mx <= current_button->x && my == current_button->y) {
            state_push(menu->current_state);
        }
    }
}

void menu_render(Menu_t* menu) {
    if (menu == NULL) {
        return;
    }
    
}
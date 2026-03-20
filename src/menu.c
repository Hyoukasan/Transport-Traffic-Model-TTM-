#include "menu.h"

void menu_init(Menu *menu){
    menu->current_state = APP_STATE_MAIN_MENU;
    menu->button_count = 0;
    menu->selected_index = -1;
}

static void menu_load_state(Menu *menu, AppState app_state){
    switch(app_state){
        case APP_STATE_MAIN_MENU:
            menu->buttons[0] = (Button){};
            menu->buttons[1] = (Button){};
            menu->button_count =2;
            break;
        case APP_STATE_SETTINGS_MENU:
            menu->buttons[0] = (Button){};
            menu->buttons[1] = (Button){};
            menu->button_count =2;
            break;

        default: break;
    }   
}

void menu_update(Menu* menu, int mx, int my, bool click){
    
    for(int i = 0; i < menu->button_count; i++){
        Button *button = &menu->buttons[i];
    }

}
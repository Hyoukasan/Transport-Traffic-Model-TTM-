#include "menu.h"
#include <stdbool.h>

void menu_init(Menu *menu){
    if (!menu) return;
    
    menu->current_state = APP_STATE_MAIN_MENU;
    menu->button_count = 0;
    menu->selected_index = -1;
}

static void menu_load_state(Menu *menu, AppState app_state){
    if (!menu) return;
    
    switch(app_state){
        case APP_STATE_MAIN_MENU:
            menu->buttons[0] = (Button){};
            menu->buttons[1] = (Button){};
            menu->button_count = 2;
            break;
        case APP_STATE_SETTINGS_MENU:
            menu->buttons[0] = (Button){};
            menu->buttons[1] = (Button){};
            menu->button_count = 2;
            break;

        default: break;
    }   
}

void menu_update(Menu* menu, int mx, int my, bool click){
    if (!menu) return;
    
    for(int i = 0; i < menu->button_count; i++){
        Button *button = &menu->buttons[i];
        // TODO: Логика обновления кнопок
        (void)button;  // Чтобы компилятор не жаловался
    }
}

void menu_render(Menu* menu){
    if (!menu) return;
    
    // TODO: Рисование меню
}
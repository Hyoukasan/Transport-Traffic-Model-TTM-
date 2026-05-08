#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "menu.h"
#include "texture.h"
#include "config_manager.h"

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
    menu->last_pressed_button = BUTTON_ID_NONE;

    menu->background_texture = texture_load("data/textures/background.png", NULL, NULL);
    if(menu->background_texture == 0) {
        printf("Warning: background.png not loaded.\n");
    }

    menu->texture = texture_load("data/textures/background_menu.png", NULL, NULL);
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
        buttons[i].texture      = texture_load(map[i].texture_path, NULL, NULL);
        buttons[i].target_state = map[i].target_state;
        buttons[i].button_id    = map[i].button_id;
        buttons[i].profile_text[0] = '\0';
    }
}

static int slot_from_button_id(ButtonId button_id) {
    switch (button_id) {
        case BUTTON_ID_SLOT_1:
            return 1;
        case BUTTON_ID_SLOT_2:
            return 2;
        case BUTTON_ID_SLOT_3:
            return 3;
        case BUTTON_ID_SLOT_4:
            return 4;
        default:
            return 0;
    }
}

static const char* scenario_name(ScenarioType scenario) {
    switch (scenario) {
        case SCENARIO_HIGHWAY:
            return "HIGHWAY";
        case SCENARIO_SINGLE_INTERSECTION:
            return "1 CROSSROAD";
        case SCENARIO_MULTI_INTERSECTION:
            return "MULTI CROSSROAD";
        default:
            return "SCENARIO";
    }
}

static void format_profile_time(float time, char* buffer, int buffer_size) {
    if (buffer == NULL || buffer_size <= 0) {
        return;
    }

    int total_seconds = (int)(time + 0.5f);
    int hours = total_seconds / 3600;
    int minutes = (total_seconds % 3600) / 60;
    int seconds = total_seconds % 60;

    snprintf(buffer, buffer_size, "%02d.%02d.%02d", hours, minutes, seconds);
}

static const char* profile_texture_path(int slot) {
    switch (slot) {
        case 1:
            return "data/textures/profile_1.png";
        case 2:
            return "data/textures/profile_2.png";
        case 3:
            return "data/textures/profile_3.png";
        case 4:
            return "data/textures/profile_4.png";
        default:
            return "data/textures/empty.png";
    }
}

static void bind_profile_buttons(MenuButton_t *buttons, ButtonInfo* map, int count_buttons) {
    for (int i = 0; i < count_buttons; i++) {
        int slot = slot_from_button_id(map[i].button_id);
        ConfigManager profile = {0};
        float time = 0.0f;

        buttons[i].target_state = map[i].target_state;
        buttons[i].button_id = map[i].button_id;
        buttons[i].profile_text[0] = '\0';

        if (slot > 0 && config_manager_load_profile(&profile, slot, &time) == 0) {
            char time_text[16];
            format_profile_time(time, time_text, sizeof(time_text));

            buttons[i].texture = texture_load(profile_texture_path(slot), NULL, NULL);
            snprintf(buttons[i].profile_text, sizeof(buttons[i].profile_text), "%s - %d LANES - %s",
                scenario_name(profile.scenario), profile.lane_count, time_text);
        } else {
            buttons[i].texture = texture_load("data/textures/empty.png", NULL, NULL);
        }
    }
}

static ButtonInfo main_menu_buttons[] = {
    {"data/textures/start.png",                 MENU_STATE_CREATE_SIMULATION, BUTTON_ID_START},
    {"data/textures/config.png",                MENU_STATE_SIMULATION_CONFIG, BUTTON_ID_CONFIG},
    {"data/textures/about.png",                 MENU_STATE_INFO, BUTTON_ID_ABOUT},
    {"data/textures/exit.png",                  MENU_STATE_EXIT, BUTTON_ID_EXIT}
};

static ButtonInfo scenario_menu_buttons[] = {
    {"data/textures/highway.png",               MENU_STATE_SCENARIO_HIGHWAY, BUTTON_ID_HIGHWAY},
    {"data/textures/single_intersection.png",   MENU_STATE_SCENARIO_SINGLE_INTERSECTION, BUTTON_ID_SINGLE_INTERSECTION},
    {"data/textures/multi_intersection.png",    MENU_STATE_SCENARIO_MULTI_INTERSECTION, BUTTON_ID_MULTI_INTERSECTION},
    {"data/textures/back.png",                  MENU_STATE_MAIN_MENU, BUTTON_ID_BACK}
};

static ButtonInfo pause_menu_buttons[] = {
    {"data/textures/resume.png",         MENU_STATE_IDLE, BUTTON_ID_RESUME},
    {"data/textures/save_profile.png",   MENU_STATE_SIMULATION_CONFIG_PAUSE, BUTTON_ID_SAVE_PROFILE},
    {"data/textures/back.png",           MENU_STATE_MAIN_MENU, BUTTON_ID_BACK}
};

static ButtonInfo load_profile_menu_buttons[] = {
    {"data/textures/empty.png", MENU_STATE_SIMULATION_CONFIG, BUTTON_ID_SLOT_1},
    {"data/textures/empty.png", MENU_STATE_SIMULATION_CONFIG, BUTTON_ID_SLOT_2},
    {"data/textures/empty.png", MENU_STATE_SIMULATION_CONFIG, BUTTON_ID_SLOT_3},
    {"data/textures/empty.png", MENU_STATE_SIMULATION_CONFIG, BUTTON_ID_SLOT_4}
};

static ButtonInfo save_profile_menu_buttons[] = {
    {"data/textures/empty.png", MENU_STATE_SIMULATION_CONFIG_PAUSE, BUTTON_ID_SLOT_1},
    {"data/textures/empty.png", MENU_STATE_SIMULATION_CONFIG_PAUSE, BUTTON_ID_SLOT_2},
    {"data/textures/empty.png", MENU_STATE_SIMULATION_CONFIG_PAUSE, BUTTON_ID_SLOT_3},
    {"data/textures/empty.png", MENU_STATE_SIMULATION_CONFIG_PAUSE, BUTTON_ID_SLOT_4}
};

static void set_buttons(MenuButton_t* buttons, int button_count, int menu_width, int menu_height, int gap) {
    if(buttons == NULL) {
        return;
    }

    int button_width  = 320;
    int button_height = 100;

    int total_height  = button_count * button_height + (button_count - 1) * gap;
    int start_y       = (menu_height - total_height) / 2;

    for(size_t i = 0; i < (size_t)button_count; i++) {
        buttons[i].width  = button_width;
        buttons[i].height = button_height;
        buttons[i].x      = (menu_width - button_width) / 2;
        buttons[i].y      = start_y + i * (button_height + gap) + 50;
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
            menu->button_count = 4;
            set_buttons(menu->buttons, menu->button_count, menu->width, menu->height, 30);
            bind_profile_buttons(menu->buttons, load_profile_menu_buttons, menu->button_count);
            break;
        case MENU_STATE_SIMULATION_CONFIG_PAUSE:
            menu->button_count = 4;
            set_buttons(menu->buttons, menu->button_count, menu->width, menu->height, 30);
            bind_profile_buttons(menu->buttons, save_profile_menu_buttons, menu->button_count);
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

    menu->last_pressed_button = BUTTON_ID_NONE;

    for (int i = 0; i < menu->button_count; i++) {
        MenuButton_t *button = &menu->buttons[i];

        int x = menu->x + button->x;
        int y = menu->y + button->y;

        bool inside = mx >= x && mx <= x + button->width &&
            my >= y && my <= y + button->height;

        if (click && inside) {
            menu->last_pressed_button = button->button_id;
            menu_set_state(menu, button->target_state);
            break;
        }
    }
}

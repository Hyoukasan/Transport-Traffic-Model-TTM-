#ifndef APP_MANAGER_H
#define APP_MANAGER_H

typedef enum {
    APP_STATE_IDLE,
    APP_STATE_RUNNING_SIMULATION,
    APP_STATE_SIMULATION_PAUSE,
    APP_STATE_CLOSED
} AppState;

typedef enum {
    AUDIO_MAIN_MENU_AMBIENT,
    AUDIO_SIMULATION_AMBIENT,
    AUDIO_CLICK_EVENT_SOUND
} AudioType;


typedef struct AppManager{
    AppState current_state;
    AudioType current_audio;
    
    int screen_width;
    int screen_height;
}AppManager;


#endif
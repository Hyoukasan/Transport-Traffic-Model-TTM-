#ifndef APP_MANAGER_H
#define APP_MANAGER_H

typedef enum {
    APP_STATE_RUNNING_SIMULATION,
    APP_STATE_SIMULATION_PAUSE,
    APP_STATE_CLOSED
} AppState;

typedef struct AppManager{
    AppState target_state;
}AppManager;


#endif
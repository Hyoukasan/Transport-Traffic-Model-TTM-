#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>
typedef struct GLFWwindow GLFWwindow;

typedef struct {
    double mouse_x;
    double mouse_y;

    int lmb_now;
    int lmb_prev;
    bool lmb_click;  

    int key_esc_now;
    int key_esc_prev;
    bool key_esc_click;
} InputState;

void input_init(InputState *in);
void input_update(GLFWwindow *window, InputState *in);

#endif
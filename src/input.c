#include <GLFW/glfw3.h>

#include "input.h"


void input_init(InputState* input) {
    if (input == NULL) {
        return;
    }

    input->mouse_x = 0.0;
    input->mouse_y = 0.0;
    input->lmb_now = GLFW_RELEASE;
    input->lmb_prev = GLFW_RELEASE;
    input->lmb_click = false;
    input->key_esc_now = GLFW_RELEASE;
    input->key_esc_prev = GLFW_RELEASE;
    input->key_esc_click = false;
}

void input_update(GLFWwindow* window, InputState* input) {
    if (window == NULL || input == NULL) {
        return;
    }

    glfwGetCursorPos(window, &input->mouse_x, &input->mouse_y);

    input->lmb_prev = input->lmb_now;
    input->lmb_now = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    input->lmb_click = (input->lmb_now == GLFW_PRESS && input->lmb_prev == GLFW_RELEASE);

    input->key_esc_prev = input->key_esc_now;
    input->key_esc_now = glfwGetKey(window, GLFW_KEY_ESCAPE);
    input->key_esc_click = (input->key_esc_now == GLFW_PRESS && input->key_esc_prev == GLFW_RELEASE);
}
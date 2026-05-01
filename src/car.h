#ifndef CAR_H
#define CAR_H

#include <stdbool.h>

struct Graph;

typedef enum {
    CAR_STATE_NORMAL,
    CAR_STATE_BRAKING,
    CAR_STATE_OVERTAKING,
    CAR_STATE_ACCIDENT
} CarState;

typedef enum {
    CAR_COLOR_YELLOW,
    CAR_COLOR_RED,
    CAR_COLOR_BLUE,
    CAR_COLOR_GREEN,
    CAR_COLOR_BLACK
} CarColor;

typedef struct Car{
    int id;
    int road_id;          // индекс сегмента дороги
    float position;       // 0.0 .. 1.0 вдоль сегмента
    float speed;          // текущая скорость
    float desired_speed;  // желаемая скорость
    float length;         // длина машины в логических единицах
    float offset;         // смещение для полосы
    int lane;             // номер полосы
    bool at_intersection; // находится ли машина в зоне пересечения
    int last_turn_x;
    int last_turn_y;
    float angle;          // угол поворота для рендера
    CarState state;
    CarColor color;
    unsigned int texture; // OpenGL texture id
} Car;

void car_init(Car *car, int id, int road_id, float desired_speed, float length, int lane, float offset);
void car_set_texture(Car *car, unsigned int texture, float width, float height);
unsigned int car_load_texture(const char *path, int *out_width, int *out_height);
void car_update(struct Car *car, const struct Graph *graph, float dt);
void car_destroy(Car *car);

#endif
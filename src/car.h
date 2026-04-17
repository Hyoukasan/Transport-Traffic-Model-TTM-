#ifndef CAR_H
#define CAR_H

#include <stdbool.h>

typedef struct Graph Graph;


typedef enum {
    CAR_STATE_NORMAL,
    CAR_STATE_BRAKING,
    CAR_STATE_OVERTAKING,
    CAR_STATE_ACCIDENT
} CarState;

typedef struct {
    int id;
    int road_id;          // индекс сегмента дороги
    float position;       // 0.0 .. 1.0 вдоль сегмента
    float speed;          // текущая скорость
    float desired_speed;  // желаемая скорость
    float length;         // длина машины в логических единицах
    float offset;         // смещение для полосы
    int lane;             // номер полосы
    CarState state;
    unsigned int texture; // OpenGL texture id
    float tex_width;
    float tex_height;
    float color[3];
} Car;

void car_init(Car *car, int id, int road_id, float desired_speed, float length, int lane, float offset);
void car_set_texture(Car *car, unsigned int texture, float width, float height);
unsigned int car_load_texture(const char *path, int *out_width, int *out_height);
void car_update(Car *car, const Graph *graph, float dt);
void car_destroy(Car *car);

#endif
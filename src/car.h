#ifndef CAR_H
#define CAR_H

#include <stdbool.h>
#include "graph.h"  // для RoadDirection

struct Graph;

typedef enum {
    CAR_STATE_NORMAL,
    CAR_STATE_BRAKING,
    CAR_STATE_OVERTAKING,
    CAR_STATE_ACCIDENT,
    CAR_STATE_LANE_CHANGE,  
    CAR_STATE_TURNING       
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

    // Новые поля для плавных манёвров
    float lane_offset;    // d: боковое смещение (0.0 = центр полосы)
    int target_lane;      // куда перестраиваемся (-1 = нет)
    float lane_shift;     // 0.0..1.0: прогресс перестроения
    float lane_change_timer; // секунд до следующей случайной проверки перестроения
    float turn_progress;  // 0.0..1.0: прогресс поворота (для угла и дуги)
    float angle_from;     // начальный угол для плавного поворота
    float angle_to;       // целевой угол

    // Параметры дуги поворота
    float turn_center_x;
    float turn_center_y;
    float turn_radius;
    float turn_path_angle_from;
    float turn_path_angle_to;
    int turn_target_road_id;
    int turn_target_lane;
    float turn_target_position;
    float turn_start_fraction;  // позиция начала поворота
    bool turn_decided;          // решено ли поворачивать
    bool turn_made;             // решено ли делать поворот
} Car;

void car_init(Car *car, int id, int road_id, float desired_speed, float length, int lane, float offset);
void car_set_texture(Car *car, unsigned int texture);
void car_update(struct Car *car, const struct Graph *graph, float dt);
void car_destroy(Car *car);

// Новые функции для манёвров
void car_start_lane_change(Car *car, int target_lane);
void car_update_lane_change(Car *car, float dt);
void car_start_turn(Car *car, RoadDirection new_dir);
void car_update_turn(Car *car, float dt);

#endif
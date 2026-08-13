#ifndef TEST_IM_H
#define TEST_IM_H

#include "traffic_manager.h"
#include "car.h"

void testIM_init(TrafficManager* manager);

// Освобождение ресурсов
void testIM_destroy(void);

// Принять решение для машины при её входе на перекрёсток (ОДИН РАЗ, потом заблокировано)
void testIM_on_car_enter_intersection(TrafficManager* manager, Car* car, int intersection_id);

// Разблокировать решение машины при её выходе с перекрёстка
void testIM_on_car_exit_intersection(Car* car, int intersection_id);

#endif

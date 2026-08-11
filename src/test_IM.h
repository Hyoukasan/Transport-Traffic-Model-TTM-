#ifndef TEST_IM_H
#define TEST_IM_H

#include "traffic_manager.h"

// Инициализация (опционально)
void testIM_init(TrafficManager* manager);

// Обновление резервов перекрёстков каждый кадр
void testIM_update(TrafficManager* manager, float dt);

// Освобождение ресурсов
void testIM_destroy(void);

#endif

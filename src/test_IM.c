#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "test_IM.h"
#include "car.h"
#include "graph.h"

// Простые массивы для резервации перекрёстков
static int *im_reserved_ids = NULL;
static float *im_reserved_timers = NULL;
static int im_reserved_count = 0;

// Убедиться, что есть место для всех перекрёстков
static void ensure_capacity(TrafficManager* manager) {
    int needed = 0;
    if (manager && manager->graph) needed = manager->graph->intersection_count;
    if (needed <= im_reserved_count) return;

    int new_count = needed > 0 ? needed : 0;
    int *new_ids = (int*)realloc(im_reserved_ids, sizeof(int) * new_count);
    float *new_timers = (float*)realloc(im_reserved_timers, sizeof(float) * new_count);
    if (new_count > 0 && (new_ids == NULL || new_timers == NULL)) {
        return;
    }

    im_reserved_ids = new_ids;
    im_reserved_timers = new_timers;

    for (int i = im_reserved_count; i < new_count; i++) {
        im_reserved_ids[i] = -1;
        im_reserved_timers[i] = 0.0f;
    }

    im_reserved_count = new_count;
}

// Найти машину по id
static Car* find_car_by_id(TrafficManager* manager, int car_id) {
    if (manager == NULL || manager->cars == NULL || car_id < 0) return NULL;
    for (int i = 0; i < manager->car_count; i++) {
        if (manager->cars[i].id == car_id) return &manager->cars[i];
    }
    return NULL;
}

// Расчет расстояния машины до центра перекрёстка
static float car_distance_to_intersection_center(const Car* car, const Intersection* intersection, const Graph* graph) {
    if (car == NULL || intersection == NULL || graph == NULL) return 9999.0f;
    if (car->road_id < 0 || car->road_id >= graph->road_count) return 9999.0f;
    
    const RoadSegment* road = &graph->roads[car->road_id];
    
    // Вычислить текущую координату машины
    float car_x, car_y;
    if (road->type == ROAD_HORIZONTAL) {
        car_x = (float)road->x1 + car->position * (float)road->length;
        car_y = (float)road->y1;
    } else {
        car_x = (float)road->x1;
        car_y = (float)road->y1 + car->position * (float)road->length;
    }
    
    // Расстояние до центра перекрёстка
    float dx = car_x - (float)intersection->x;
    float dy = car_y - (float)intersection->y;
    
    return sqrtf(dx * dx + dy * dy);
}

void testIM_init(TrafficManager* manager) {
    ensure_capacity(manager);
}

void testIM_destroy(void) {
    free(im_reserved_ids);
    free(im_reserved_timers);
    im_reserved_ids = NULL;
    im_reserved_timers = NULL;
    im_reserved_count = 0;
}

// Принять решение для машины ОДИН РАЗ при её входе на перекрёсток
// Машина получает плана: проезжать (NORMAL) или ждать (SLOWING)
static void testIM_decide_for_car(TrafficManager* manager, Car* car, int intersection_id) {
    if (manager == NULL || car == NULL || intersection_id < 0) return;
    if (intersection_id >= manager->graph->intersection_count) return;

    const Intersection* inter = &manager->graph->intersections[intersection_id];
    
    // Машина уже приняла решение на этом перекрёстке - не менять!
    if (car->intersection_locked && car->locked_intersection_id == intersection_id) {
        return;
    }

    // Блокируем это решение
    car->intersection_locked = true;
    car->locked_intersection_id = intersection_id;

    // Находим вторую машину на перекрёстке (если есть)
    Car* other_car = NULL;
    int other_id = (car->id == inter->id_car_at_intersecction[0]) 
                   ? inter->id_car_at_intersecction[1] 
                   : inter->id_car_at_intersecction[0];
    
    if (other_id >= 0) {
        other_car = find_car_by_id(manager, other_id);
    }

    // ===== ОДНА МАШИНА =====
    if (other_car == NULL) {
        car->state = CAR_STATE_NORMAL;
        return;
    }

    // ===== ДВЕ МАШИНЫ - ПРИНИМАЕМ РЕШЕНИЕ =====
    
    // Приоритет 1: Машина которая поворачивает
    bool this_turning = (car->state == CAR_STATE_TURNING) || (car->turn_made && car->turn_decided);
    bool other_turning = (other_car->state == CAR_STATE_TURNING) || (other_car->turn_made && other_car->turn_decided);

    if (this_turning && !other_turning) {
        // Мы поворачиваем - мы едим
        car->state = CAR_STATE_NORMAL;
        return;
    }
    if (!this_turning && other_turning) {
        // Другая машина поворачивает - мы ждём
        car->state = CAR_STATE_SLOWING;
        return;
    }

    // Приоритет 2: Обе машины не поворачивают - по расстоянию до центра
    float dist_this = car_distance_to_intersection_center(car, inter, manager->graph);
    float dist_other = car_distance_to_intersection_center(other_car, inter, manager->graph);

    if (dist_this < dist_other) {
        // Мы ближе к центру - мы едим
        car->state = CAR_STATE_NORMAL;
    } else {
        // Другая ближе - мы ждём
        car->state = CAR_STATE_SLOWING;
    }
}

// Очистить блокировку машины когда она выходит с перекрёстка
static void testIM_unlock_car(Car* car, int intersection_id) {
    if (car == NULL) return;
    if (car->intersection_locked && car->locked_intersection_id == intersection_id) {
        car->intersection_locked = false;
        car->locked_intersection_id = -1;
    }
}

// Вызывается когда машина добавляется на перекрёсток
// Принимает решение один раз и блокирует его
void testIM_on_car_enter_intersection(TrafficManager* manager, Car* car, int intersection_id) {
    if (manager == NULL || car == NULL) return;
    testIM_decide_for_car(manager, car, intersection_id);
}

// Вызывается когда машина покидает перекрёсток
// Разблокирует решение
void testIM_on_car_exit_intersection(Car* car, int intersection_id) {
    if (car == NULL) return;
    testIM_unlock_car(car, intersection_id);
}

// Старая главная функция (теперь не используется в цикле)
// Оставляю на случай если нужна для совместимости
void testIM_update(TrafficManager* manager, float dt) {
    (void)manager;  // Параметр намеренно не используется
    (void)dt;       // Параметр намеренно не используется
    // Функция больше не вызывается каждый кадр
    // Решения принимаются в traffic_manager_find_cars_at_intersactions()
}

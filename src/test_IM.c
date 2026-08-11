#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

// Простая приоритетная функция: прямо>право>лево
static int car_priority(const Car* car) {
    if (car == NULL) return 0;
    if (car->turn_type == CAR_TURN_NONE) return 3;
    if (car->turn_type == CAR_TURN_RIGHT) return 2;
    return 1;
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

// Главная функция: проверяет 2 слота у каждого перекрёстка и ставит резервацию
void testIM_update(TrafficManager* manager, float dt) {
    if (manager == NULL || manager->graph == NULL) return;

    ensure_capacity(manager);

    int icount = manager->graph->intersection_count;
    for (int ix = 0; ix < icount; ix++) {
        const Intersection* inter = &manager->graph->intersections[ix];
        if (inter == NULL) continue;

        int id0 = inter->id_car_at_intersecction[0];
        int id1 = inter->id_car_at_intersecction[1];

        Car* c0 = find_car_by_id(manager, id0);
        Car* c1 = find_car_by_id(manager, id1);

        // Таймер резервации уменьшаем
        if (im_reserved_timers && ix < im_reserved_count) {
            if (im_reserved_timers[ix] > 0.0f) im_reserved_timers[ix] -= dt;
            if (im_reserved_timers[ix] <= 0.0f) im_reserved_ids[ix] = -1;
        }

        // Нет машин — пропускаем
        if (c0 == NULL && c1 == NULL) continue;

        // Одна машина — разрешаем и резервируем
        if (c0 != NULL && c1 == NULL) {
            if (im_reserved_ids[ix] != c0->id) {
                im_reserved_ids[ix] = c0->id;
                if (im_reserved_timers) im_reserved_timers[ix] = 1.0f;
            }
            if (c0->state == CAR_STATE_SLOWING) c0->state = CAR_STATE_NORMAL;
            continue;
        }

        if (c1 != NULL && c0 == NULL) {
            if (im_reserved_ids[ix] != c1->id) {
                im_reserved_ids[ix] = c1->id;
                if (im_reserved_timers) im_reserved_timers[ix] = 1.0f;
            }
            if (c1->state == CAR_STATE_SLOWING) c1->state = CAR_STATE_NORMAL;
            continue;
        }

        // Обе машины есть
        if (c0 != NULL && c1 != NULL) {
            int rid = (im_reserved_ids && ix < im_reserved_count) ? im_reserved_ids[ix] : -1;
            Car* reserved_car = find_car_by_id(manager, rid);
            if (reserved_car != NULL && (reserved_car == c0 || reserved_car == c1)) {
                Car* other = (reserved_car == c0) ? c1 : c0;
                if (reserved_car->state == CAR_STATE_SLOWING) reserved_car->state = CAR_STATE_NORMAL;
                if (other->state != CAR_STATE_TURNING) {
                    other->state = CAR_STATE_SLOWING;
                    float cap = reserved_car->speed * 0.6f;
                    if (other->speed > cap) other->speed = cap;
                }
                continue;
            }

            int p0 = car_priority(c0);
            int p1 = car_priority(c1);
            Car* winner = NULL;
            Car* loser = NULL;
            if (p0 != p1) {
                winner = (p0 > p1) ? c0 : c1;
            } else if (c0->speed != c1->speed) {
                winner = (c0->speed > c1->speed) ? c0 : c1;
            } else {
                winner = (c0->position > c1->position) ? c0 : c1;
            }
            loser = (winner == c0) ? c1 : c0;

            if (im_reserved_ids && ix < im_reserved_count) {
                im_reserved_ids[ix] = winner->id;
                im_reserved_timers[ix] = 1.0f;
            }

            if (winner->state == CAR_STATE_SLOWING) winner->state = CAR_STATE_NORMAL;
            if (loser->state != CAR_STATE_TURNING) {
                loser->state = CAR_STATE_SLOWING;
                float cap = winner->speed * 0.5f;
                if (loser->speed > cap) loser->speed = cap;
            }
        }
    }
}

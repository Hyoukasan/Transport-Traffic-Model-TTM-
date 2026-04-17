#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "car.h"
#include "graph.h"
#include "texture.h"

static bool point_in_range(int value, int a, int b) {
    if (a <= b) {
        return value >= a && value <= b;
    }
    return value >= b && value <= a;
}

static float calculate_road_fraction(const RoadSegment *road, int x, int y) {
    if (!road) {
        return 0.0f;
    }

    if (road->type == ROAD_HORIZONTAL) {
        int span = abs(road->x2 - road->x1);
        if (span == 0) {
            return 0.0f;
        }
        return (float)abs(x - road->x1) / (float)span;
    }

    if (road->type == ROAD_VERTICAL) {
        int span = abs(road->y2 - road->y1);
        if (span == 0) {
            return 0.0f;
        }
        return (float)abs(y - road->y1) / (float)span;
    }

    return 0.0f;
}

void car_init(Car *car, int id, int road_id, float desired_speed, float length, int lane, float offset) {
    if (car == NULL) {
        return;
    }

    car->id = id;
    car->road_id = road_id;
    car->position = 0.0f;
    car->speed = 0.0f;
    car->desired_speed = desired_speed;
    car->length = length;
    car->offset = offset;
    car->lane = lane;
    car->at_intersection = false;
    car->last_turn_x = -1;
    car->last_turn_y = -1;
    car->angle = 0.0f;
    car->state = CAR_STATE_NORMAL;
    car->texture = 0;
    car->tex_width = 0.0f;
    car->tex_height = 0.0f;
    car->color[0] = 0.8f;
    car->color[1] = 0.2f;
    car->color[2] = 0.2f;
}

void car_set_texture(Car *car, unsigned int texture, float width, float height) {
    if (car == NULL) {
        return;
    }

    car->texture = texture;
    car->tex_width = width;
    car->tex_height = height;
}

unsigned int car_load_texture(const char *path, int *out_width, int *out_height) {
    return texture_load(path, out_width, out_height);
}

void car_update(Car *car, const Graph *graph, float dt) {
    if (car == NULL || graph == NULL || car->road_id < 0 || car->road_id >= graph->road_count) {
        return;
    }

    const RoadSegment *road = &graph->roads[car->road_id];
    float target_speed = car->desired_speed;
    if (target_speed > road->speed_limit) {
        target_speed = road->speed_limit;
    }

    if (road->accident || car->state == CAR_STATE_ACCIDENT) {
        target_speed *= 0.2f;
    } else if (car->state == CAR_STATE_BRAKING) {
        target_speed *= 0.5f;
    } else if (car->state == CAR_STATE_OVERTAKING) {
        target_speed *= 1.1f;
        if (target_speed > road->speed_limit) {
            target_speed = road->speed_limit;
        }
    }

    float accel = 2.5f;
    car->speed += (target_speed - car->speed) * accel * dt;
    if (car->speed < 0.0f) {
        car->speed = 0.0f;
    }

    float segment_length = (float)road->length;
    if (segment_length <= 0.0f) {
        segment_length = 1.0f;
    }

    car->position += car->speed * dt / segment_length;
    if (car->position > 1.0f) {
        car->position = 1.0f;
    }

    car->at_intersection = false;
    float base_angle = (road->type == ROAD_VERTICAL) ? 90.0f : 0.0f;
    car->angle = base_angle;

    for (int i = 0; i < graph->intersection_count; i++) {
        int ix = graph->intersections[i].x;
        int iy = graph->intersections[i].y;
        if (!point_in_range(ix, road->x1, road->x2) && road->type == ROAD_HORIZONTAL) continue;
        if (!point_in_range(iy, road->y1, road->y2) && road->type == ROAD_VERTICAL) continue;
        if (road->type == ROAD_HORIZONTAL && iy != road->y1) continue;
        if (road->type == ROAD_VERTICAL && ix != road->x1) continue;

        float intersection_fraction = calculate_road_fraction(road, ix, iy);
        if (intersection_fraction < 0.0f || intersection_fraction > 1.0f) {
            continue;
        }

        if (car->position < intersection_fraction - 0.04f || car->position > intersection_fraction + 0.04f) {
            continue;
        }

        if (car->last_turn_x == ix && car->last_turn_y == iy) {
            car->at_intersection = true;
            break;
        }

        int crossing_road_id = -1;
        for (int r = 0; r < graph->road_count; r++) {
            if (r == car->road_id) {
                continue;
            }
            const RoadSegment *other = &graph->roads[r];
            if (other->type == ROAD_HORIZONTAL && ix >= other->x1 && ix <= other->x2 && iy == other->y1) {
                if (road->type == ROAD_VERTICAL) {
                    crossing_road_id = r;
                    break;
                }
            } else if (other->type == ROAD_VERTICAL && iy >= other->y1 && iy <= other->y2 && ix == other->x1) {
                if (road->type == ROAD_HORIZONTAL) {
                    crossing_road_id = r;
                    break;
                }
            }
        }

        if (crossing_road_id >= 0) {
            const RoadSegment *new_road = &graph->roads[crossing_road_id];
            float new_position = 0.0f;
            if (new_road->type == ROAD_HORIZONTAL) {
                int span = abs(new_road->x2 - new_road->x1);
                new_position = span > 0 ? (float)abs(ix - new_road->x1) / (float)span : 0.0f;
                car->angle = 0.0f;
            } else {
                int span = abs(new_road->y2 - new_road->y1);
                new_position = span > 0 ? (float)abs(iy - new_road->y1) / (float)span : 0.0f;
                car->angle = 90.0f;
            }

            car->road_id = crossing_road_id;
            car->position = new_position + 0.02f;
            if (car->position > 1.0f) {
                car->position = 1.0f;
            }
            car->at_intersection = true;
            car->last_turn_x = ix;
            car->last_turn_y = iy;
            break;
        }
    }

    if (!car->at_intersection) {
        car->last_turn_x = -1;
        car->last_turn_y = -1;
    }
}

void car_destroy(Car *car) {
    if (car == NULL) {
        return;
    }

    if (car->texture != 0) {
        GLuint tex = (GLuint)car->texture;
        glDeleteTextures(1, &tex);
        car->texture = 0;
    }
}

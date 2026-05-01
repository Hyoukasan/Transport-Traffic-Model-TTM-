#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "car.h"
#include "graph.h"

static bool point_in_range(int value, int a, int b) {
    if (a <= b) {
        return value >= a && value <= b;
    }
    return value >= b && value <= a;
}

static float calculate_road_fraction(const RoadSegment *road, int x, int y) {
    if (road == NULL) {
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

static bool is_horizontal_direction(RoadDirection dir) {
    return dir == ROAD_DIR_EAST || dir == ROAD_DIR_WEST;
}

static bool is_vertical_direction(RoadDirection dir) {
    return dir == ROAD_DIR_NORTH || dir == ROAD_DIR_SOUTH;
}

static bool is_left_turn(RoadDirection current, RoadDirection candidate) {
    switch (current) {
        case ROAD_DIR_EAST:
            return candidate == ROAD_DIR_NORTH;
        case ROAD_DIR_WEST:
            return candidate == ROAD_DIR_SOUTH;
        case ROAD_DIR_NORTH:
            return candidate == ROAD_DIR_WEST;
        case ROAD_DIR_SOUTH:
            return candidate == ROAD_DIR_EAST;
        default:
            return false;
    }
}

static bool is_right_turn(RoadDirection current, RoadDirection candidate) {
    switch (current) {
        case ROAD_DIR_EAST:
            return candidate == ROAD_DIR_SOUTH;
        case ROAD_DIR_WEST:
            return candidate == ROAD_DIR_NORTH;
        case ROAD_DIR_NORTH:
            return candidate == ROAD_DIR_EAST;
        case ROAD_DIR_SOUTH:
            return candidate == ROAD_DIR_WEST;
        default:
            return false;
    }
}

static bool is_perpendicular_direction(RoadDirection current, RoadDirection candidate) {
    return (is_horizontal_direction(current) && is_vertical_direction(candidate)) ||
           (is_vertical_direction(current) && is_horizontal_direction(candidate));
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
}

void car_set_texture(Car *car, unsigned int texture) {
    if (car == NULL) {
        return;
    }

    car->texture = texture;
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
        car->position = 1.2f;
    }

    car->at_intersection = false;
    switch (road->direction) {
        case ROAD_DIR_EAST:
            car->angle = -90.0f;
            break;
        case ROAD_DIR_WEST:
            car->angle = 90.0f;
            break;
        case ROAD_DIR_NORTH:
            car->angle = 0.0f;
            break;
        case ROAD_DIR_SOUTH:
            car->angle = 180.0f;
            break;
        default:
            car->angle = 0.0f;
            break;
    }

    for (int i = 0; i < graph->intersection_count; i++) {
        int ix = graph->intersections[i].x;
        int iy = graph->intersections[i].y;

        if (road->type == ROAD_HORIZONTAL) {
            if (!point_in_range(ix, road->x1, road->x2) || iy != road->y1) {
                continue;
            }
        } else if (road->type == ROAD_VERTICAL) {
            if (!point_in_range(iy, road->y1, road->y2) || ix != road->x1) {
                continue;
            }
        } else {
            continue;
        }

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

        int left_road_id = -1;
        int right_road_id = -1;

        Intersection *intersection = &graph->intersections[i];
        for (int j = 0; j < intersection->road_count; j++) {
            int candidate_id = intersection->roads[j];
            if (candidate_id == car->road_id) {
                continue;
            }

            const RoadSegment *candidate = &graph->roads[candidate_id];
            if (!is_perpendicular_direction(road->direction, candidate->direction)) {
                continue;
            }

            if (is_left_turn(road->direction, candidate->direction)) {
                left_road_id = candidate_id;
            } else if (is_right_turn(road->direction, candidate->direction)) {
                right_road_id = candidate_id;
            }
        }

        int chosen_road_id = -1;
        int roll = rand() % 100;
        if (left_road_id >= 0 && roll < 20) {
            chosen_road_id = left_road_id;
        } else if (right_road_id >= 0 && roll < 40) {
            chosen_road_id = right_road_id;
        }

        if (chosen_road_id >= 0) {
            const RoadSegment *new_road = &graph->roads[chosen_road_id];
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

            car->road_id = chosen_road_id;
            car->position = new_position + 0.02f;
            if (car->position > 1.0f) {
                car->position = 1.0f;
            }
            car->at_intersection = true;
            car->last_turn_x = ix;
            car->last_turn_y = iy;
            break;
        }

        car->at_intersection = true;
        car->last_turn_x = ix;
        car->last_turn_y = iy;
        break;
    }

    if (!car->at_intersection) {
        car->last_turn_x = -1;
        car->last_turn_y = -1;
    }
}


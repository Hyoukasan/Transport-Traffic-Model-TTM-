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
// мне нужно было это для определения типа поворота на перекрестке, чтобы правильно выбирать полосу при смене дороги
static bool road_supports_direction(const RoadSegment *road, RoadDirection direction) {
    if (road == NULL) {
        return false;
    }

    if (road->direction != ROAD_DIR_NONE) {
        return road->direction == direction;
    }

    if (road->type == ROAD_HORIZONTAL) {
        return direction == ROAD_DIR_EAST || direction == ROAD_DIR_WEST;
    }
    if (road->type == ROAD_VERTICAL) {
        return direction == ROAD_DIR_NORTH || direction == ROAD_DIR_SOUTH;
    }
    return false;
}
// функция определния выбора полосы при повороте на перекрестке
static int direction_lane_start(const RoadSegment *road, RoadDirection direction) {
    int lanes = road->lanes > 0 ? road->lanes : 1;
    int half = lanes / 2;

    if (road->direction != ROAD_DIR_NONE) {
        return 0;
    }

    if (road->type == ROAD_HORIZONTAL) {
        if (direction == ROAD_DIR_WEST) {
            return 0;
        }
        if (direction == ROAD_DIR_EAST) {
            return half;
        }
    }
    if (road->type == ROAD_VERTICAL) {
        if (direction == ROAD_DIR_SOUTH) {
            return 0;
        }
        if (direction == ROAD_DIR_NORTH) {
            return half;
        }
    }
    return 0;
}

static int direction_lane_count(const RoadSegment *road, RoadDirection direction) {
    int lanes = road->lanes > 0 ? road->lanes : 1;
    int half = lanes / 2;

    if (road->direction != ROAD_DIR_NONE) {
        return lanes;
    }

    if (road->type == ROAD_HORIZONTAL) {
        if (direction == ROAD_DIR_WEST) {
            return half;
        }
        if (direction == ROAD_DIR_EAST) {
            return lanes - half;
        }
    }
    if (road->type == ROAD_VERTICAL) {
        if (direction == ROAD_DIR_SOUTH) {
            return half;
        }
        if (direction == ROAD_DIR_NORTH) {
            return lanes - half;
        }
    }
    return 0;
}

static int map_lane_to_direction(const RoadSegment *source_road, RoadDirection source_direction, int source_lane,
                                 const RoadSegment *target_road, RoadDirection target_direction) {
    if (source_road == NULL || target_road == NULL) {
        return 0;
    }

    int source_start = direction_lane_start(source_road, source_direction);
    int source_count = direction_lane_count(source_road, source_direction);
    int target_start = direction_lane_start(target_road, target_direction);
    int target_count = direction_lane_count(target_road, target_direction);

    if (source_count <= 0 || target_count <= 0) {
        return target_start;
    }

    int local_index = source_lane - source_start;
    if (local_index < 0) {
        local_index = 0;
    }
    if (local_index >= source_count) {
        local_index = source_count - 1;
    }
    if (local_index >= target_count) {
        local_index = target_count - 1;
    }

    return target_start + local_index;
}

static bool is_direction_aligned_with_road(const RoadSegment *road, RoadDirection direction) {
    if (road == NULL) {
        return true;
    }

    if (road->type == ROAD_HORIZONTAL) {
        if (road->x2 >= road->x1) {
            return direction == ROAD_DIR_EAST;
        }
        return direction == ROAD_DIR_WEST;
    }

    if (road->type == ROAD_VERTICAL) {
        if (road->y2 >= road->y1) {
            return direction == ROAD_DIR_SOUTH;
        }
        return direction == ROAD_DIR_NORTH;
    }

    return true;
}

static float position_to_travel_fraction(const RoadSegment *road, RoadDirection direction, float position) {
    if (is_direction_aligned_with_road(road, direction)) {
        return position;
    }
    return 1.0f - position;
}

static float travel_fraction_to_position(const RoadSegment *road, RoadDirection direction, float travel_fraction) {
    if (is_direction_aligned_with_road(road, direction)) {
        return travel_fraction;
    }
    return 1.0f - travel_fraction;
}

static float advance_travel_position(const RoadSegment *road, RoadDirection direction, float travel_fraction, float delta) {
    float next = travel_fraction + delta;
    if (next < 0.0f) {
        next = 0.0f;
    }
    if (next > 1.0f) {
        next = 1.0f;
    }
    return travel_fraction_to_position(road, direction, next);
}

static float coordinate_fraction_along_road(const RoadSegment *road, int x, int y) {
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

static float coordinate_at_travel_position(const RoadSegment *road, RoadDirection direction, float position) {
    if (road == NULL) {
        return 0.0f;
    }

    float travel_fraction = position_to_travel_fraction(road, direction, position);
    if (road->type == ROAD_HORIZONTAL) {
        int min_x = road->x1 < road->x2 ? road->x1 : road->x2;
        int max_x = road->x1 > road->x2 ? road->x1 : road->x2;
        float span = (float)(max_x - min_x);
        if (direction == ROAD_DIR_EAST) {
            return min_x + span * travel_fraction;
        }
        return max_x - span * travel_fraction;
    }
    if (road->type == ROAD_VERTICAL) {
        int min_y = road->y1 < road->y2 ? road->y1 : road->y2;
        int max_y = road->y1 > road->y2 ? road->y1 : road->y2;
        float span = (float)(max_y - min_y);
        if (direction == ROAD_DIR_SOUTH) {
            return min_y + span * travel_fraction;
        }
        return max_y - span * travel_fraction;
    }

    return 0.0f;
}

static float coordinate_fraction_for_direction(const RoadSegment *road, RoadDirection direction, int x, int y) {
    float frac = coordinate_fraction_along_road(road, x, y);
    return position_to_travel_fraction(road, direction, frac);
}

static RoadDirection get_effective_direction(const RoadSegment *road, int lane) {
    if (road == NULL) {
        return ROAD_DIR_NONE;
    }

    if (road->direction != ROAD_DIR_NONE) {
        return road->direction;
    }

    int lanes = road->lanes > 0 ? road->lanes : 1;
    int half = lanes / 2;
    if (road->type == ROAD_HORIZONTAL) {
        return lane < half ? ROAD_DIR_WEST : ROAD_DIR_EAST;
    }
    if (road->type == ROAD_VERTICAL) {
        return lane < half ? ROAD_DIR_SOUTH : ROAD_DIR_NORTH;
    }
    return ROAD_DIR_NONE;
}

static int choose_lane_for_direction(const RoadSegment *road, RoadDirection desired, int preferred_lane) {
    if (road == NULL) {
        return 0;
    }

    int lanes = road->lanes > 0 ? road->lanes : 1;
    if (lanes == 1) {
        return 0;
    }

    int lane_start = direction_lane_start(road, desired);
    int lane_count = direction_lane_count(road, desired);
    if (lane_count <= 0) {
        return 0;
    }

    if (road->direction != ROAD_DIR_NONE) {
        int lane_index = preferred_lane >= 0 && preferred_lane < lanes ? preferred_lane : rand() % lanes;
        return lane_index;
    }

    int local_index = preferred_lane - lane_start;
    if (local_index < 0 || local_index >= lane_count) {
        local_index = 0;
    }
    if (local_index >= lane_count) {
        local_index = lane_count - 1;
    }

    return lane_start + local_index;
}

static float clampf(float value, float min_value, float max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

static float travel_fraction_to_distance(float fraction, float length) {
    return fraction * length;
}

static float distance_to_travel_fraction(float distance, float length) {
    if (length <= 0.0f) {
        return 0.0f;
    }
    return distance / length;
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
    RoadDirection current_direction = get_effective_direction(road, car->lane);
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
    float old_position = car->position;
    float old_travel_fraction = position_to_travel_fraction(road, current_direction, old_position);

    car->speed += (target_speed - car->speed) * accel * dt;
    if (car->speed < 0.0f) {
        car->speed = 0.0f;
    }

    float segment_length = (float)road->length;
    if (segment_length <= 0.0f) {
        segment_length = 1.0f;
    }

    float new_travel_fraction = old_travel_fraction + car->speed * dt / segment_length;
    new_travel_fraction = clampf(new_travel_fraction, 0.0f, 1.0f);
    car->position = travel_fraction_to_position(road, current_direction, new_travel_fraction);

    float current_coord = coordinate_at_travel_position(road, current_direction, car->position);
    float old_coord = coordinate_at_travel_position(road, current_direction, old_position);
    car->at_intersection = false;
    switch (current_direction) {
        case ROAD_DIR_EAST:
            car->angle = 90.0f;
            break;
        case ROAD_DIR_WEST:
            car->angle = -90.0f;
            break;
        case ROAD_DIR_NORTH:
            car->angle = 180.0f;
            break;
        case ROAD_DIR_SOUTH:
            car->angle = 0.0f;
            break;
        default:
            car->angle = 0.0f;
            break;
    }

    int chosen_intersection = -1;
    float chosen_fraction = 0.0f;
    int chosen_ix = -1;
    int chosen_iy = -1;
    float best_distance = INFINITY;

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

        float intersection_coord = (road->type == ROAD_HORIZONTAL) ? (float)ix : (float)iy;
        float gap = 0.0f;
        bool ahead = false;

        switch (current_direction) {
            case ROAD_DIR_EAST:
                ahead = old_coord < intersection_coord && intersection_coord <= current_coord;
                gap = intersection_coord - old_coord;
                break;
            case ROAD_DIR_WEST:
                ahead = old_coord > intersection_coord && intersection_coord >= current_coord;
                gap = old_coord - intersection_coord;
                break;
            case ROAD_DIR_SOUTH:
                ahead = old_coord < intersection_coord && intersection_coord <= current_coord;
                gap = intersection_coord - old_coord;
                break;
            case ROAD_DIR_NORTH:
                ahead = old_coord > intersection_coord && intersection_coord >= current_coord;
                gap = old_coord - intersection_coord;
                break;
            default:
                break;
        }

        if (!ahead || gap <= 0.0f) {
            continue;
        }

        if (car->last_turn_x == ix && car->last_turn_y == iy) {
            continue;
        }

        if (gap < best_distance) {
            best_distance = gap;
            chosen_intersection = i;
            chosen_ix = ix;
            chosen_iy = iy;
            chosen_fraction = coordinate_fraction_for_direction(road, current_direction, ix, iy);
        }
    }

    if (chosen_intersection < 0) {
        car->last_turn_x = -1;
        car->last_turn_y = -1;
        return;
    }

    float intersection_distance = travel_fraction_to_distance(new_travel_fraction - chosen_fraction, segment_length);
    bool crossed_intersection = new_travel_fraction > chosen_fraction;
    if (!crossed_intersection) {
        return;
    }

    int left_road_id = -1;
    int right_road_id = -1;
    RoadDirection left_target = ROAD_DIR_NONE;
    RoadDirection right_target = ROAD_DIR_NONE;

    switch (current_direction) {
        case ROAD_DIR_EAST:
            left_target = ROAD_DIR_NORTH;
            right_target = ROAD_DIR_SOUTH;
            break;
        case ROAD_DIR_WEST:
            left_target = ROAD_DIR_SOUTH;
            right_target = ROAD_DIR_NORTH;
            break;
        case ROAD_DIR_NORTH:
            left_target = ROAD_DIR_WEST;
            right_target = ROAD_DIR_EAST;
            break;
        case ROAD_DIR_SOUTH:
            left_target = ROAD_DIR_EAST;
            right_target = ROAD_DIR_WEST;
            break;
        default:
            break;
    }

    Intersection *intersection = &graph->intersections[chosen_intersection];
    for (int j = 0; j < intersection->road_count; j++) {
        int candidate_id = intersection->roads[j];
        if (candidate_id == car->road_id) {
            continue;
        }

        const RoadSegment *candidate = &graph->roads[candidate_id];
        if (road->type == ROAD_HORIZONTAL && candidate->type == ROAD_VERTICAL) {
            if (candidate->direction == left_target || candidate->direction == ROAD_DIR_NONE) {
                left_road_id = candidate_id;
            }
            if (candidate->direction == right_target || candidate->direction == ROAD_DIR_NONE) {
                right_road_id = candidate_id;
            }
        } else if (road->type == ROAD_VERTICAL && candidate->type == ROAD_HORIZONTAL) {
            if (candidate->direction == left_target || candidate->direction == ROAD_DIR_NONE) {
                left_road_id = candidate_id;
            }
            if (candidate->direction == right_target || candidate->direction == ROAD_DIR_NONE) {
                right_road_id = candidate_id;
            }
        }
    }

    int chosen_road_id = -1;
    RoadDirection chosen_target = ROAD_DIR_NONE;
    bool turn_made = false;
    int roll = rand() % 100;
    if (left_road_id >= 0 && roll < 20) {
        chosen_road_id = left_road_id;
        chosen_target = left_target;
        turn_made = true;
    } else if (right_road_id >= 0 && roll < 40) {
        chosen_road_id = right_road_id;
        chosen_target = right_target;
        turn_made = true;
    }

    car->at_intersection = true;
    car->last_turn_x = chosen_ix;
    car->last_turn_y = chosen_iy;

    if (turn_made) {
        const RoadSegment *new_road = &graph->roads[chosen_road_id];
        int new_lane = map_lane_to_direction(road, current_direction, car->lane, new_road, chosen_target);
        if (!road_supports_direction(new_road, chosen_target)) {
            new_lane = choose_lane_for_direction(new_road, chosen_target, new_lane);
        }

        float new_road_fraction = coordinate_fraction_for_direction(new_road, chosen_target, chosen_ix, chosen_iy);
        float remaining_distance = intersection_distance;
        float new_road_length = (float)(new_road->length > 0 ? new_road->length : 1.0f);
        float new_road_delta = distance_to_travel_fraction(remaining_distance, new_road_length);
        float new_road_travel_fraction = clampf(new_road_fraction + new_road_delta, 0.0f, 1.0f);
        float new_position = travel_fraction_to_position(new_road, chosen_target, new_road_travel_fraction);

        car->road_id = chosen_road_id;
        car->lane = new_lane;
        car->position = clampf(new_position, 0.0f, 1.0f);
        return;
    }

    return;
}


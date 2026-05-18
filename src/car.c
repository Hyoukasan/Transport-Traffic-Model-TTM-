#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GL/glew.h>

#include "car.h"
#include "graph.h"
#include "geometry.h"

static float speed_control(float speed_limit, float target_speed)
{
    return (target_speed > speed_limit) ? speed_limit : target_speed;
}

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

static float road_lane_center(const RoadSegment *road, int lane) {
    int lanes = road->lanes > 0 ? road->lanes : 1;
    int half = lanes / 2;

    if (road->type == ROAD_HORIZONTAL) {
        if (road->direction == ROAD_DIR_EAST) {
            return (float)(road->y1 + lane);
        }
        if (road->direction == ROAD_DIR_WEST) {
            return (float)(road->y1 - (lanes - 1) + lane);
        }
        return (float)(road->y1 - half + lane);
    }

    if (road->type == ROAD_VERTICAL) {
        if (road->direction == ROAD_DIR_NORTH) {
            return (float)(road->x1 + lane);
        }
        if (road->direction == ROAD_DIR_SOUTH) {
            return (float)(road->x1 - (lanes - 1) + lane);
        }
        return (float)(road->x1 - half + lane);
    }

    return 0.0f;
}


void car_init(Car *car, int id, int road_id, float desired_speed, int lane) {
    if (car == NULL) {
        return;
    }

    car->id = id;
    car->road_id = road_id;
    car->position = 0.0f;
    car->speed = 0.0f;
    car->desired_speed = desired_speed;
    car->lane = lane;
    car->at_intersection = false;
    car->last_turn_x = -1;
    car->last_turn_y = -1;
    car->angle = 0.0f;
    car->state = CAR_STATE_NORMAL;
    car->texture = 0;
    car->overtaking = false;
    car->original_lane = -1;

    // Инициализация новых полей
    car->lane_offset = 0.0f;
    car->target_lane = -1;
    car->lane_shift = 0.0f;
    car->lane_change_timer = (float)(rand() % 121 + 120) / 60.0f;
    car->turn_progress = 0.0f;
    car->angle_from = 0.0f;
    car->angle_to = 0.0f;
    car->turn_center_x = 0.0f;
    car->turn_center_y = 0.0f;
    car->turn_radius = 0.0f;
    car->turn_path_angle_from = 0.0f;
    car->turn_path_angle_to = 0.0f;
    car->turn_target_road_id = -1;
    car->turn_target_lane = -1;
    car->turn_target_position = 0.0f;
    car->turn_start_fraction = 0.0f;
    car->turn_decided = false;
    car->turn_made = false;
}

void car_set_texture(Car *car, unsigned int texture) {
    if (car == NULL) {
        return;
    }

    car->texture = texture;
}

void car_destroy(Car *car) {
    if (car == NULL) {
        return;
    }

    memset(car, 0, sizeof(*car));
    car->road_id = -1;
    car->target_lane = -1;
    car->overtaking = false;
    car->original_lane = -1;
    car->turn_target_road_id = -1;
    car->turn_target_lane = -1;
    car->last_turn_x = -1;
    car->last_turn_y = -1;
}

/*Функция car_speed_update изменяет текущую скорость для текущего автомобиля в зависимости от его состояния
Также учитывается ограничется скорости на линии. Формула расчета идет через коэфицент сглаживания*/

static void car_speed_update(Car* car, const RoadSegment* road, float dt) {
    float target_speed = car->desired_speed;
    float speed_limit = road->speed_limit;

    target_speed = speed_control(speed_limit, target_speed);

    CarState cur_state = car->state;

    switch (cur_state)
    {
    case CAR_STATE_ACCIDENT:
        // Состояние аварии запрещает перестроени
        break;

    case CAR_STATE_BRAKING:
        target_speed = 0.0f;
        break;

    case CAR_STATE_TRAFFIC_LIGHT:
        target_speed = 0.0f;
        break;

    case CAR_STATE_OVERTAKING:
        target_speed *= 1.1f;
        target_speed = speed_control(speed_limit, target_speed);
        break;
        
    case CAR_STATE_SLOWING:
        target_speed *= 0.5f;
        break;
    
    default:
        if(road->accident) {
            target_speed *= 0.2f;
        }

        break;
    }

    float accel = 2.5f;
    float k = accel * dt;
    if (k > 1.0f) {
        k = 1.0f;
    }

    car->speed += (target_speed - car->speed) * k;

    if(car->speed < 0.0f) {
        car->speed = 0.0f;
    }
}

static void car_angle_update(Car* car, RoadDirection current_direction)
{
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
}

typedef struct {
    int idx;
    float fraction;
    int x;
    int y;
} CrossedIntersection;

/*Функция car_find_crossed_intersection проодится по каждому перекрестку 
и по направлению дороги определяет расстояние авто до перекрестка*/

static CrossedIntersection car_find_crossed_intersection(
    const Car* car,
    const Graph* graph,
    const RoadSegment* road,
    RoadDirection direction,
    float old_coord,
    float current_coord) 

{
    CrossedIntersection crossed = {-1, 0.0f, -1, -1};
    // Если машина пересекает несколько перекрестков, ищем ближайший
    float best_distance = INFINITY;

    for(size_t i = 0; i < (size_t)graph->intersection_count; i++) {
        int ix = graph->intersections[i].x;
        int iy = graph->intersections[i].y;

        if(road->type == ROAD_HORIZONTAL) {
            if (!point_in_range(ix, road->x1, road->x2) || iy != road->y1) {
                continue;
            }
        } else if(road->type == ROAD_VERTICAL) {
            if (!point_in_range(iy, road->y1, road->y2) || ix != road->x1) {
                continue;
            }     
        } else {
            continue;
        }

        float intersection_coord = (road->type == ROAD_HORIZONTAL) ? (float)ix : (float)iy;
        // gap - вспомогательная переменная: растояние позиции со старого кадра до перекрестка
        float gap = 0.0f;
        bool ahead = false;

        switch (direction) {
            case ROAD_DIR_EAST:
            case ROAD_DIR_SOUTH:
                ahead = old_coord < intersection_coord && intersection_coord <= current_coord;
                gap = intersection_coord - old_coord;
                break;

            case ROAD_DIR_WEST:
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
            crossed.idx = (int)i;
            crossed.x = ix;
            crossed.y = iy;
            crossed.fraction = coordinate_fraction_for_direction(road, direction, ix, iy);
        }

    }

    return crossed;
}

void car_update(Car *car, const Graph *graph, float dt) {
    if (car == NULL || graph == NULL || car->road_id < 0 || car->road_id >= graph->road_count) {
        return;
    }

    // Обновление плавных манёвров
    car_update_lane_change(car, dt);

    if (car->state == CAR_STATE_TURNING) {
        car_update_turn(car, dt);
        return;
    }

    const RoadSegment *road = &graph->roads[car->road_id];
    RoadDirection current_direction = graph_get_lane_direction(road, car->lane);

    car_speed_update(car, road, dt);

    float old_position = car->position;
    float old_travel_fraction = position_to_travel_fraction(road, current_direction, old_position);

    float segment_length = (float)road->length;
    if (segment_length <= 0.0f) {
        segment_length = 1.0f;
    }

    float new_travel_fraction = old_travel_fraction + car->speed * dt / segment_length;
    new_travel_fraction = clampf(new_travel_fraction, 0.0f, 1.0f);
    float new_position = travel_fraction_to_position(road, current_direction, new_travel_fraction);

    float current_coord = coordinate_at_travel_position(road, current_direction, new_position);
    float old_coord = coordinate_at_travel_position(road, current_direction, old_position);
    car->at_intersection = false;

    car_angle_update(car, current_direction);

    if (car->state == CAR_STATE_ACCIDENT) {
        car->position = new_position;
        return;
    }

    CrossedIntersection crossed = car_find_crossed_intersection(car, graph, road, current_direction, old_coord, current_coord);

    if (crossed.idx < 0) {
        car->last_turn_x = -1;
        car->last_turn_y = -1;
        car->turn_decided = false;
        car->turn_made = false;
        car->position = new_position;
        return;
    }

    int left_road_id = -1;
    int right_road_id = -1;
    RoadDirection left_target = ROAD_DIR_NONE;
    RoadDirection right_target = ROAD_DIR_NONE;
    RoadDirection chosen_target = ROAD_DIR_NONE;

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

    const Intersection *intersection = &graph->intersections[crossed.idx];
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

    // Решение о повороте при подъезде к пересечению
    if (!car->turn_decided && new_travel_fraction >= crossed.fraction - 0.1f) {
        car->turn_decided = true;
        int roll = rand() % 100;
        if (left_road_id >= 0 && roll < 20) {
            car->turn_made = true;
            car->turn_target_road_id = left_road_id;
            chosen_target = left_target;
        } else if (right_road_id >= 0 && roll < 40) {
            car->turn_made = true;
            car->turn_target_road_id = right_road_id;
            chosen_target = right_target;
        } else {
            car->turn_made = false;
        }

        if (car->turn_made) {
            const RoadSegment *new_road = &graph->roads[car->turn_target_road_id];
            int new_lane = map_lane_to_direction(road, current_direction, car->lane, new_road, chosen_target);
            if (!road_supports_direction(new_road, chosen_target)) {
                new_lane = choose_lane_for_direction(new_road, chosen_target, new_lane);
            }

            float current_lane_center = road_lane_center(road, car->lane);
            float target_lane_center = road_lane_center(new_road, new_lane);

            float offset_current = (road->type == ROAD_HORIZONTAL)
                ? fabsf(current_lane_center - (float)crossed.y)
                : fabsf(current_lane_center - (float)crossed.x);
            float offset_target = (new_road->type == ROAD_HORIZONTAL)
                ? fabsf(target_lane_center - (float)crossed.y)
                : fabsf(target_lane_center - (float)crossed.x);

            float turn_offset = fmaxf(fmaxf(offset_current, offset_target), 1.0f);
            float turn_radius = turn_offset;

            float start_x, start_y;
            if (road->type == ROAD_HORIZONTAL) {
                start_x = (float)crossed.x;
                start_y = current_lane_center;
            } else {
                start_x = current_lane_center;
                start_y = (float)crossed.y;
            }

            float end_x, end_y;
            if (new_road->type == ROAD_HORIZONTAL) {
                end_x = (float)crossed.x;
                end_y = target_lane_center;
            } else {
                end_x = target_lane_center;
                end_y = (float)crossed.y;
            }

            car->turn_start_fraction = clampf(coordinate_fraction_for_direction(road, current_direction, start_x, start_y), 0.0f, 1.0f);
            float end_fraction = clampf(coordinate_fraction_for_direction(new_road, chosen_target, end_x, end_y), 0.0f, 1.0f);
            car->turn_target_position = clampf(travel_fraction_to_position(new_road, chosen_target, end_fraction), 0.0f, 1.0f);
            car->turn_target_lane = new_lane;
            car->turn_center_x = (float)crossed.x + ((current_direction == ROAD_DIR_EAST || chosen_target == ROAD_DIR_EAST) ? turn_radius : -turn_radius);
            car->turn_center_y = (float)crossed.y + ((current_direction == ROAD_DIR_SOUTH || chosen_target == ROAD_DIR_SOUTH) ? turn_radius : -turn_radius);
            car->turn_radius = turn_radius;
        }
    }

    // Начало поворота при достижении края
    if (car->turn_decided && car->turn_made && new_travel_fraction >= car->turn_start_fraction && car->state != CAR_STATE_TURNING) {
        const RoadSegment *new_road = &graph->roads[car->turn_target_road_id];
        float current_lane_center = road_lane_center(road, car->lane);
        float target_lane_center = road_lane_center(new_road, car->turn_target_lane);

        float start_x, start_y;
        if (road->type == ROAD_HORIZONTAL) {
            start_x = coordinate_at_travel_position(road, current_direction, car->turn_start_fraction);
            start_y = current_lane_center;
        } else {
            start_x = current_lane_center;
            start_y = coordinate_at_travel_position(road, current_direction, car->turn_start_fraction);
        }

        float end_x, end_y;
        if (new_road->type == ROAD_HORIZONTAL) {
            end_x = coordinate_at_travel_position(new_road, chosen_target, car->turn_target_position);
            end_y = target_lane_center;
        } else {
            end_x = target_lane_center;
            end_y = coordinate_at_travel_position(new_road, chosen_target, car->turn_target_position);
        }

        float start_angle = atan2f(start_y - car->turn_center_y, start_x - car->turn_center_x) * (180.0f / 3.14159265f);
        float end_angle = atan2f(end_y - car->turn_center_y, end_x - car->turn_center_x) * (180.0f / 3.14159265f);

        car->position = travel_fraction_to_position(road, current_direction, car->turn_start_fraction);
        car->turn_path_angle_from = start_angle;
        car->turn_path_angle_to = end_angle;
        car->angle_from = direction_to_angle(current_direction);
        car->angle_to = direction_to_angle(chosen_target);
        car->turn_progress = 0.0f;
        car->state = CAR_STATE_TURNING;
    }

    bool crossed_intersection = new_travel_fraction > crossed.fraction;
    if (crossed_intersection && car->turn_decided && !car->turn_made) {
        car->turn_decided = false;
        car->turn_made = false;
    }
    if (!crossed_intersection) {
        car->position = new_position;
        return;
    }

    car->position = new_position;
    return;
}

// Новые функции для манёвров
void car_start_lane_change(Car *car, int target_lane) {
    if (car == NULL || target_lane < 0 || target_lane == car->lane || car->state != CAR_STATE_NORMAL) {
        return;
    }

    car->target_lane = target_lane;
    car->lane_shift = 0.0f;
    car->lane_offset = 0.0f;
    car->state = CAR_STATE_LANE_CHANGE;
}

void car_update_lane_change(Car *car, float dt) {
    if (car == NULL || car->target_lane < 0) return;

    car->lane_shift += dt * 2.0f;  // скорость перестроения
    if (car->lane_shift > 1.0f) {
        car->lane_shift = 1.0f;
    }

    float lane_diff = (float)(car->target_lane - car->lane);
    car->lane_offset = lane_diff * smoothstep(car->lane_shift);

    if (car->lane_shift >= 1.0f) {
        car->lane += (int)lane_diff;
        car->target_lane = -1;
        car->lane_shift = 0.0f;
        car->lane_offset = 0.0f;
        car->state = CAR_STATE_NORMAL;
    }
}

void car_update_turn(Car *car, float dt) {
    if (car == NULL || car->state != CAR_STATE_TURNING) {
        return;
    }

    car->turn_progress += dt * 0.8f;  // скорость поворота по дуге
    if (car->turn_progress > 1.0f) {
        car->turn_progress = 1.0f;
    }

    car->angle = lerp_angle(car->angle_from, car->angle_to, smoothstep(car->turn_progress));
    if (car->turn_progress >= 1.0f) {
        car->angle = car->angle_to;
        car->state = CAR_STATE_NORMAL;
        if (car->turn_target_road_id >= 0) {
            car->road_id = car->turn_target_road_id;
        }
        if (car->turn_target_lane >= 0) {
            car->lane = car->turn_target_lane;
        }
        car->position = clampf(car->turn_target_position, 0.0f, 1.0f);
        car->turn_target_road_id = -1;
        car->turn_target_lane = -1;
        car->turn_center_x = 0.0f;
        car->turn_center_y = 0.0f;
        car->turn_radius = 0.0f;
        car->turn_path_angle_from = 0.0f;
        car->turn_path_angle_to = 0.0f;
        car->turn_target_position = 0.0f;
        car->turn_progress = 0.0f;
        car->turn_decided = false;
        car->turn_made = false;
    }
}


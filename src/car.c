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

static int graph_find_node_index(const Graph *g, int x, int y) {
    if (!g) {
        return -1;
    }
    for (int i = 0; i < g->intersection_count; i++) {
        if (g->intersections[i].x == x && g->intersections[i].y == y) {
            return i;
        }
    }
    return -1;
}

static float road_direction_angle(const RoadSegment *road) {
    if (!road) {
        return 0.0f;
    }

    float dx = (float)(road->x2 - road->x1);
    float dy = (float)(road->y1 - road->y2); // invert Y because screen Y is flipped
    if (dx == 0.0f && dy == 0.0f) {
        return 0.0f;
    }

    const float rad_to_deg = 180.0f / 3.14159265f;
    return atan2f(dy, dx) * rad_to_deg;
}

static float normalize_angle(float angle) {
    while (angle > 180.0f) angle -= 360.0f;
    while (angle <= -180.0f) angle += 360.0f;
    return angle;
}

static int graph_find_road_between_nodes(const Graph *g, int from_node, int to_node) {
    if (!g || from_node < 0 || to_node < 0 || from_node >= g->intersection_count || to_node >= g->intersection_count) {
        return -1;
    }

    const GridPoint *a = &g->intersections[from_node];
    const GridPoint *b = &g->intersections[to_node];

    for (int r = 0; r < g->road_count; r++) {
        const RoadSegment *road = &g->roads[r];
        if (road->type == ROAD_HORIZONTAL) {
            if (a->y == road->y1 && b->y == road->y1 && point_in_range(a->x, road->x1, road->x2) && point_in_range(b->x, road->x1, road->x2)) {
                return r;
            }
        } else if (road->type == ROAD_VERTICAL) {
            if (a->x == road->x1 && b->x == road->x1 && point_in_range(a->y, road->y1, road->y2) && point_in_range(b->y, road->y1, road->y2)) {
                return r;
            }
        }
    }
    return -1;
}

static bool car_compute_path(Car *car, const Graph *graph, int current_node) {
    if (!car || !graph || current_node < 0) {
        return false;
    }

    if (graph->intersection_count <= 1) {
        return false;
    }

    // Find neighboring nodes connected by roads
    int neighbors[MAX_CAR_PATH_NODES];
    int neighbor_count = 0;

    int cx = graph->intersections[current_node].x;
    int cy = graph->intersections[current_node].y;

    for (int i = 0; i < graph->intersection_count && neighbor_count < MAX_CAR_PATH_NODES; i++) {
        if (i == current_node) continue;

        int nx = graph->intersections[i].x;
        int ny = graph->intersections[i].y;

        // Check if there's a road connecting current node to this node
        int road_id = graph_find_road_between_nodes(graph, current_node, i);
        if (road_id >= 0) {
            neighbors[neighbor_count++] = i;
        }
    }

    if (neighbor_count == 0) {
        return false;
    }

    // Choose random neighbor as next target
    car->path_nodes[0] = current_node;
    car->path_nodes[1] = neighbors[rand() % neighbor_count];
    car->path_length = 2;
    car->path_index = 1;

    return true;
}

static int car_get_next_path_node(Car *car, const Graph *graph, int current_node) {
    if (!car || !graph || current_node < 0) {
        return -1;
    }

    if (car->path_length <= 1 || car->path_index >= car->path_length) {
        if (!car_compute_path(car, graph, current_node)) {
            return -1;
        }
    }

    while (car->path_index < car->path_length && car->path_nodes[car->path_index] == current_node) {
        car->path_index++;
    }

    if (car->path_index >= car->path_length) {
        if (!car_compute_path(car, graph, current_node)) {
            return -1;
        }
        while (car->path_index < car->path_length && car->path_nodes[car->path_index] == current_node) {
            car->path_index++;
        }
        if (car->path_index >= car->path_length) {
            return -1;
        }
    }

    return car->path_nodes[car->path_index];
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
    car->last_node_index = -1;
    car->target_node = -1;
    car->path_length = 0;
    car->path_index = 0;
    car->angle = 0.0f;
    car->target_angle = 0.0f;
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
    car->target_angle = road_direction_angle(road);

    float angle_diff = normalize_angle(car->target_angle - car->angle);
    float turn_speed = 360.0f; // degrees per second
    float max_turn = turn_speed * dt;
    if (fabsf(angle_diff) <= max_turn) {
        car->angle = car->target_angle;
    } else {
        car->angle += copysignf(max_turn, angle_diff);
    }

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

        int current_node = graph_find_node_index(graph, ix, iy);
        if (current_node < 0) {
            continue;
        }

        if (car->last_node_index == current_node) {
            car->at_intersection = true;
            break;
        }

        int next_node = car_get_next_path_node(car, graph, current_node);
        int crossing_road_id = -1;

        if (next_node >= 0) {
            crossing_road_id = graph_find_road_between_nodes(graph, current_node, next_node);
        }

        if (crossing_road_id < 0) {
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
        }

        if (crossing_road_id >= 0) {
            const RoadSegment *new_road = &graph->roads[crossing_road_id];
            float new_position = 0.0f;
            if (new_road->type == ROAD_HORIZONTAL) {
                int span = abs(new_road->x2 - new_road->x1);
                new_position = span > 0 ? (float)abs(ix - new_road->x1) / (float)span : 0.0f;
            } else {
                int span = abs(new_road->y2 - new_road->y1);
                new_position = span > 0 ? (float)abs(iy - new_road->y1) / (float)span : 0.0f;
            }

            car->road_id = crossing_road_id;
            car->position = new_position;
            if (car->position > 1.0f) {
                car->position = 1.0f;
            }
            car->target_angle = road_direction_angle(new_road);
            car->at_intersection = true;
            car->last_turn_x = ix;
            car->last_turn_y = iy;
            car->last_node_index = current_node;
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

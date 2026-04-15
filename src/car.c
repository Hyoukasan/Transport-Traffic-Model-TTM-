#include "car.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void car_init(Car *car, int id, int road_id, float desired_speed, float length, int lane, float offset) {
    if (!car) {
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
    car->state = CAR_STATE_NORMAL;
    car->texture = 0;
    car->tex_width = 0.0f;
    car->tex_height = 0.0f;
    car->color[0] = 0.8f;
    car->color[1] = 0.2f;
    car->color[2] = 0.2f;
}

void car_set_texture(Car *car, unsigned int texture, float width, float height) {
    if (!car) {
        return;
    }

    car->texture = texture;
    car->tex_width = width;
    car->tex_height = height;
}

unsigned int car_load_texture(const char *path, int *out_width, int *out_height) {
    if (!path) {
        return 0;
    }

    int width, height, channels;
    unsigned char *pixels = stbi_load(path, &width, &height, &channels, 4);
    if (!pixels) {
        fprintf(stderr, "car_load_texture: failed to load image '%s'\n", path);
        return 0;
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(pixels);

    if (out_width) {
        *out_width = width;
    }
    if (out_height) {
        *out_height = height;
    }

    return (unsigned int)texture_id;
}

void car_update(Car *car, const Graph *graph, float dt) {
    if (!car || !graph || car->road_id < 0 || car->road_id >= graph->road_count) {
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
}

void car_destroy(Car *car) {
    if (!car) {
        return;
    }

    if (car->texture != 0) {
        GLuint tex = (GLuint)car->texture;
        glDeleteTextures(1, &tex);
        car->texture = 0;
    }
}
